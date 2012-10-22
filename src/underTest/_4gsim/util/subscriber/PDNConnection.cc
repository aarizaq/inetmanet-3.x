//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <platdep/sockets.h>
#include "PDNConnection.h"
#include "Subscriber.h"
#include "DiameterApplication.h"
#include <algorithm>
#include "GTPUtils.h"
#include "NASUtils.h"
#include "NASSerializer.h"

PDNConnection::PDNConnection() {
	// TODO Auto-generated constructor stub
	init();
}

PDNConnection::PDNConnection(ESMEntity *ownerp) {
	init();
	this->ownerp = ownerp;
	this->id = ownerp->genPDNConnectionId();
	BearerContext *bearer = new BearerContext(this);
	addBearerContext(bearer, true);
}

PDNConnection::~PDNConnection() {
	// TODO Auto-generated destructor stub
	delBearerContext(0, bearers.size());
	// s5s8Tunn will be deleted in TunnelEndpointTable
}

void PDNConnection::init() {
	s5s8Tunn = NULL;
	ownerp = NULL;
	defBearer = NULL;
	id = 0;
	type = GTP_IPv4;
}

BearerContext *PDNConnection::findBearerContextForId(unsigned char id) {
	for (unsigned i = 0; i < bearers.size(); i++) {
		BearerContext *bearer = bearers[i];
		if (bearer->getId() == id)
			return bearer;
	}
	return NULL;
}

BearerContext *PDNConnection::findBearerContextForProcId(unsigned char procId) {
	for (unsigned i = 0; i < bearers.size(); i++) {
		BearerContext *bearer = bearers[i];
		if (bearer->getProcId() == procId)
			return bearer;
	}
	return NULL;
}

void PDNConnection::addBearerContext(BearerContext *bearer, bool def) {
	bearer->setOwner(this);
	if (def == true)
		defBearer = bearer;
	bearers.push_back(bearer);
}

void PDNConnection::delBearerContext(unsigned start, unsigned end) {
	BearerContexts::iterator first = bearers.begin() + start;
	BearerContexts::iterator last = bearers.begin() + end;
	BearerContexts::iterator i = first;
	for (;i != last; ++i) {
		if (*i == defBearer)
			defBearer = NULL;
		delete *i;
	}
	bearers.erase(first, last);
}

void PDNConnection::setSubscriberAddress(IPvXAddress subAddr) {
	this->subAddr = subAddr;
	type = subAddr.isIPv6() ? GTP_IPv6 : GTP_IPv4;
}

void PDNConnection::setOwner(ESMEntity *ownerp) {
	this->ownerp = ownerp;
}

ESMEntity *PDNConnection::getOwner() {
	return ownerp;
}

Subscriber *PDNConnection::getSubscriber() {
	return ownerp->getOwner();
}

bool PDNConnection::isDefault() {
	return (ownerp->getDefPDNConnection() == this);
}

AVP *PDNConnection::createAPNConfigAVP() {
	std::vector<AVP*> mip6AgentInfoVec;
	mip6AgentInfoVec.push_back(DiameterUtils().createAddressAVP(AVP_MIPHomeAgentAddr, 0, 0, 0, 0, pdnAddr));

	std::vector<AVP*> apnConfigVec;
	apnConfigVec.push_back(DiameterUtils().createUnsigned32AVP(AVP_ContextIdentifier, 1, 0, 0, TGPP, id));
	apnConfigVec.push_back(DiameterUtils().createAddressAVP(AVP_ServPartyIPAddr, 1, 0, 0, TGPP, subAddr));
	apnConfigVec.push_back(DiameterUtils().createInteger32AVP(AVP_PDNType, 1, 0, 0, TGPP, type == GTP_IPv4 ? DIAMETER_IPv4 : DIAMETER_IPv6));
	apnConfigVec.push_back(DiameterUtils().createUTF8StringAVP(AVP_ServiceSelection, 0, 0, 0, 0, apn));
	apnConfigVec.push_back(DiameterUtils().createInteger32AVP(AVP_PDNGWAllocType, 1, 0, 0, TGPP, STATIC));
	apnConfigVec.push_back(DiameterUtils().createGroupedAVP(AVP_MIP6AgentInfo, 0, 0, 0, 0, mip6AgentInfoVec));
	DiameterUtils().deleteGroupedAVP(mip6AgentInfoVec);
	AVP *apnConfig = DiameterUtils().createGroupedAVP(AVP_APNConfig, 1, 0, 0, TGPP, apnConfigVec);
	DiameterUtils().deleteGroupedAVP(apnConfigVec);
	return apnConfig;
}

bool PDNConnection::processAPNConfigAVP(std::vector<AVP*> apnConfigVec) {
	/* ServiceSelection */
	AVP *servSel = DiameterUtils().findAVP(AVP_ServiceSelection, apnConfigVec);
	if (servSel == NULL) {
		EV << "DiameterS6a: Missing ServiceSelection AVP.\n";
		return false;
	}
	apn = DiameterUtils().processUTF8String(servSel);

	/* ServPartyIPAddr */
	AVP *servPartyIPAddr = DiameterUtils().findAVP(AVP_ServPartyIPAddr, apnConfigVec);
	if (servPartyIPAddr == NULL) {
		EV << "DiameterS6a: Missing ServPartyIPAddr AVP.\n";
		return false;
	}
	subAddr = DiameterUtils().processAddressAVP(servPartyIPAddr);

	/* ContextIdentifier */
	AVP *ctxtId = DiameterUtils().findAVP(AVP_ContextIdentifier, apnConfigVec);
	if (ctxtId == NULL) {
		EV << "DiameterS6a: Missing ContextIdentifier AVP.\n";
		return false;
	}
	id = DiameterUtils().processUnsigned32AVP(ctxtId);

	/* MIP6AgentInfo */
	AVP *mip6AgentInfo = DiameterUtils().findAVP(AVP_MIP6AgentInfo, apnConfigVec);
	if (mip6AgentInfo == NULL) {
		EV << "DiameterS6a: Missing MIP6AgentInfo AVP.\n";
		return false;
	}
	std::vector<AVP*> mip6AgentInfoVec = DiameterUtils().processGroupedAVP(mip6AgentInfo);

	/* MIPHomeAgenAddr */
	AVP *mipHomeAgentAddr = DiameterUtils().findAVP(AVP_MIPHomeAgentAddr, mip6AgentInfoVec);
	if (mipHomeAgentAddr == NULL) {
		EV << "DiameterS6a: Missing MIPHomeAgentAddr AVP.\n";
		return false;
	}
	pdnAddr = DiameterUtils().processAddressAVP(mipHomeAgentAddr);
	DiameterUtils().deleteGroupedAVP(mip6AgentInfoVec);
	DiameterUtils().deleteGroupedAVP(apnConfigVec);
	return true;
}

void PDNConnection::toGTPMessage(GTPMessage *msg) {
	/* PGW S5/S8 Address */
	if (getSubscriber()->getGTPProcedure() == EUTRANInitAttachReq)
		msg->pushIe(GTPUtils().createFteidIE(1, 0, S5_S8_PGW_GTP_C, pdnAddr));

	/* APN */
	if (getSubscriber()->getGTPProcedure() == EUTRANInitAttachReq)
		msg->pushIe(GTPUtils().createIE(GTPV2_APN, 0, apn));

	/* PDN Type */
	if (getSubscriber()->getGTPProcedure() == EUTRANInitAttachReq)
		msg->pushIe(GTPUtils().createIE(GTP_V2, GTPV2_PDNType, 0, type));

	/* PDN Address Allocation */
	msg->pushIe(GTPUtils().createAddrIE(GTPV2_PAA, 0, subAddr));

	/* Bearer Contexts to be created */
	if (((getSubscriber()->getGTPProcedure() / 2) * 2) == EUTRANInitAttachReq)
		msg->pushIe(defBearer->createBearerContextIE(0));

}

bool PDNConnection::fromGTPMessage(GTPMessage *msg, TunnelEndpoint *te) {
	/* APN */
	if (getSubscriber()->getGTPProcedure() == EUTRANInitAttachReq) {
		GTPInfoElem *apn = msg->findIe(GTPV2_APN, 0);
		if (apn == NULL) {
			te->sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Man_IE_Missing, 0, 0, 1, GTPV2_APN, 0));
			return false;
		}
		this->apn = GTPUtils().processIE(apn);
	}

	/* PDN Address Allocation */
	GTPInfoElem *pdnAddrAlloc = msg->findIe(GTPV2_PAA, 0);
	if (pdnAddrAlloc == NULL) {
		te->sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Con_IE_Missing, 0, 0, 1, GTPV2_PAA, 0));
		return false;
	}
	subAddr = GTPUtils().processAddrIE(pdnAddrAlloc);

	/* Bearer Context to be created*/
	std::vector<GTPInfoElem*> bearerCtxts = msg->findIes(GTPV2_BearerContext, 0);
	if (bearerCtxts.size() < 1) {
		te->sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Man_IE_Missing, 0, 0, 1, GTPV2_BearerContext, 0));
		return false;
	}

	for (unsigned i = 0; i < bearerCtxts.size(); i++) {
		if (i == 0) {
			defBearer->processBearerContextIE(bearerCtxts.at(i));
		} else {
			BearerContext *bearer = new BearerContext(this);
			this->addBearerContext(bearer, false);
			bearer->processBearerContextIE(bearerCtxts.at(i));
		}
	}
	return true;
}

NASPlainMessage *PDNConnection::createPDNConnectivityRequest() {
	NASPlainMessage *msg = new NASPlainMessage("PDN-Connectivity-Request");
	defBearer->performStateTransition(UEInitESMProcReq);
	msg->setHdr(NASUtils().createHeader(0, 0, ESMMessage, defBearer->getProcId(), PDNConnectivityRequest));
	msg->setIesArraySize(2);

	/* PDN type */
	msg->setIes(0, NASUtils().createIE(IE_V, IEType1, 0, type));

	/* Request type */
	msg->setIes(1, NASUtils().createIE(IE_V, IEType1, 0, InitialRequest));

	return msg;
}

void PDNConnection::processPDNConnectivityRequest(NASPlainMessage *msg) {
	defBearer->setProcId(msg->getHdr().getProcTransId());

	/* PDN type */
	type = msg->getIes(0).getValue(0);
	delete msg;
}

NASPlainMessage *PDNConnection::createActDefBearerRequest() {
	NASPlainMessage *msg = new NASPlainMessage("Activate-Default-Bearer-Request");
	defBearer->performStateTransition(ActBearerCtxtReq);
	msg->setHdr(NASUtils().createHeader(defBearer->getId(), 0, ESMMessage, defBearer->getProcId(), ActDefEPSBearerCtxtReq));
	msg->setIesArraySize(3);

	/* EPS QoS */
	char epsQos[9];
	epsQos[0] = defBearer->getQci();
	msg->setIes(0, NASUtils().createIE(IE_LV, IEType4, 0, 1, epsQos));

	/* APN */
	msg->setIes(1, NASUtils().createIE(IE_LV, IEType4, 0, apn.size(), apn.data()));

	/* PDN address */
	unsigned len = subAddr.isIPv6() ? 8 : 5;
	char *pdnAddr = (char*)calloc(len, sizeof(char));
	char *p = pdnAddr;
	*((unsigned char*)(p++)) = type;
	if (type == GTP_IPv4)
		*((unsigned*)p) = ntohl(subAddr.get4().getInt());
	msg->setIes(2, NASUtils().createIE(IE_LV, IEType4, 0, len, pdnAddr));

	return msg;
}

void PDNConnection::processActDefBearerRequest(NASPlainMessage *msg) {
	/* APN */
	apn = NASUtils().processIE(msg->getIes(1));

	/* PDN address */
	char *val = NASUtils().processIE(msg->getIes(2));
	char *p = val;
	if (*((char*)p++) == GTP_IPv4)
		subAddr = IPvXAddress(IPv4Address(ntohl(*((unsigned*)p))));

	BearerContext *bearer = findBearerContextForProcId(msg->getHdr().getProcTransId());
	if (bearer == NULL)
		return;

	bearer->setId(msg->getHdr().getEpsBearerId());
	bearer->performStateTransition(ActBearerCtxtReq);
	delete msg;
}

NASPlainMessage *PDNConnection::createActDefBearerAccept() {
	NASPlainMessage *msg = new NASPlainMessage("Activate-Default-Bearer-Accept");
	defBearer->performStateTransition(ActBearerCtxtAcc);
	msg->setHdr(NASUtils().createHeader(defBearer->getId(), 0, ESMMessage, defBearer->getProcId(), ActDefEPSBearerCtxtAcc));

	return msg;
}

void PDNConnection::processActDefBearerAccept(NASPlainMessage *msg) {
	defBearer->performStateTransition(ActBearerCtxtAcc);
	delete msg;
}

std::string PDNConnection::info(int tabs) const {
    std::stringstream out;
	for (int i = 0; i < tabs; i++) out << "\t";
	out << "def:" << ((this == ownerp->getDefPDNConnection()) ? "true\n" : "false\n");
    for (int i = 0; i < tabs; i++) out << "\t";
    out << "apn:" << apn << "\n";
    for (int i = 0; i < tabs; i++) out << "\t";
    out << "pdnAddr:" << pdnAddr.str() << "\n";
    for (int i = 0; i < tabs; i++) out << "\t";
    out << "subAddr:" << subAddr.str() << "\n";
    if (s5s8Tunn != NULL) {
    	for (int i = 0; i < tabs; i++) out << "\t";
    	out << "s5s8Tunn:{ " << s5s8Tunn->info(tabs + 1);
    	out << " }\n";
    }
    if (bearers.size() > 0) {
    	for (int i = 0; i < tabs; i++) out << "\t";
		out << "bearers:{\n";
		for (unsigned j = 0; j < bearers.size(); j++) {
			for (int i = 0; i < tabs + 1; i++) out << "\t";
			out << "bearer:{\n";
			for (int i = 0; i < tabs + 2; i++) out << "\t";
			out << "def:" << ((this->defBearer == bearers[j]) ? "true\n" : "false\n");
			out << bearers[j]->info(tabs + 2);
			for (int i = 0; i < tabs + 1; i++) out << "\t";
			out << "}\n";
		}
		for (int i = 0; i < tabs; i++) out << "\t";
		out << "}\n";
    }
    return out.str();
}
