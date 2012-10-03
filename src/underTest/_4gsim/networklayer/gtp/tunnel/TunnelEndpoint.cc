//
// Copyright (C) 2012 Calin Cerchez
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
#include "TunnelEndpoint.h"
#include "PDNConnection.h"
#include "GTPUtils.h"
#include "LTEUtils.h"
#include "Subscriber.h"
#include "GTP.h"
#include "GTPControl.h"
#include "GTPUser.h"

TunnelEndpoint::TunnelEndpoint() {
	init();
}

TunnelEndpoint::TunnelEndpoint(GTPPath *path) {
	// TODO Auto-generated constructor stub
	init();
	this->path = path;
	module = path->getModule();
	if (module != NULL)
		localId = module->genTeids();
}

TunnelEndpoint::~TunnelEndpoint() {
	// TODO Auto-generated destructor stub
}

void TunnelEndpoint::init() {
	path = NULL;
	localId = 0;
	remoteId = 0;
	fwTunnEnd = NULL;
	module = NULL;
}

void TunnelEndpoint::sendCreateSessionRequest() {
	Subscriber *sub = NULL;
	PDNConnection *conn = NULL;
	if ((sub = dynamic_cast<Subscriber*>(ownerp))) {
		if (sub->getGTPProcedure() == EUTRANInitAttachReq)
			conn = sub->getEsmEntity()->getDefPDNConnection();
	} else if ((conn = dynamic_cast<PDNConnection*>(ownerp))) {
		sub = conn->getSubscriber();
	} else {
		return;
	}

	GTPMessage *csr = new GTPMessage("Create-Session-Request");

	/* Create Session Request */
	csr->setHeader(GTPUtils().createHeader(CreateSessionRequest, 0, 1, remoteId, uniform(0, 1000)));

	/* IMSI */
	csr->pushIe(GTPUtils().createIE(GTPV2_IMSI, IMSI_CODED_SIZE, 0, sub->getEmmEntity()->getImsi()));

	/* MSISDN */
	if (sub->getMsisdn() != NULL)
		csr->pushIe(GTPUtils().createIE(GTPV2_MSISDN, MSISDN_CODED_SIZE, 0, sub->getMsisdn()));

	/* Serving Network */
	csr->pushIe(GTPUtils().createIE(GTPV2_ServingNetwork, PLMNID_CODED_SIZE, 0, path->getModule()->getPLMNId()));

	/* RAT Type */
	csr->pushIe(GTPUtils().createIE(GTP_V2, GTPV2_RATType, 0, sub->getRatType()));

	/* Sender F-TEID */
	csr->pushIe(createFteidIE(0));

	/* PDN Connection info */
	conn->toGTPMessage(csr);

	sub->pushSeqNr(csr->getHeader()->getSeqNr());
	path->send(csr);
}

void TunnelEndpoint::processCreateSessionRequest(GTPMessage *msg) {

	/* IMSI */
	GTPInfoElem *imsiIe = msg->findIe(GTPV2_IMSI, 0);
	if (imsiIe == NULL) {
		sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Con_IE_Missing, 0, 0, 1, GTPV2_IMSI, 0));
		return;
	}
	char *imsi = GTPUtils().processIE(imsiIe);
	Subscriber *sub = dynamic_cast<GTPControl*>(module)->subT->findSubscriberForIMSI(imsi);
	if (sub == NULL) {
		sub = new Subscriber();
		dynamic_cast<GTPControl*>(module)->subT->push_back(sub);
		sub->initEntities(0);
		sub->setImsi(imsi);
		sub->setGTPProcedure(EUTRANInitAttachReq);
	}
	/* MSISDN */
	GTPInfoElem *msisdn = msg->findIe(GTPV2_MSISDN, 0);
	if (msisdn != NULL) {
		sub->setMsisdn(GTPUtils().processIE(msisdn));
	}

//	/* RAT Type */
//	int ratPos = GTPUtils().findIE(RATType, 0, msg);
//	if (ratPos == -1) {
//		return GTPUtils().createCauseIE(GTP_CAUSE_IE_MAX_SIZE, 0, Man_IE_Missing, 0, 0, 1, RATType);
//	}
//	sub->setRATType(GTPUtils().processIE(msg->getIes(ratPos)));
//
	ESMEntity *esm = sub->getEsmEntity();
	PDNConnection *conn = new PDNConnection(esm);
	if (!conn->fromGTPMessage(msg, this))
		return;
	if (sub->getGTPProcedure() == EUTRANInitAttachReq)
		esm->addPDNConnection(conn, true);
	else
		esm->addPDNConnection(conn, false);

	if (path->getType() == S11_S4_SGW_GTP_C) {	// for SGW
		sub->setS11TunnEnd(this);

		// initialize forward tunnel to PGW
		/* PGW S5/S8 Address */
		GTPInfoElem *pdnGwAddr = msg->findIe(GTPV2_F_TEID, 1);
		if (pdnGwAddr == NULL) {
			sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Con_IE_Missing, 0, 0, 1, GTPV2_F_TEID, 1));
			return;
		}
		TunnelEndpoint *pdnGw = GTPUtils().processFteidIE(pdnGwAddr);
		if (pdnGw == NULL) {
			sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, No_Res_Avail, 0, 0, 1, GTPV2_F_TEID, 1));
			return;
		}
		GTPPath *path = module->findPath(pdnGw->getRemoteAddr(), pdnGw->getType());
		if (path == NULL) {
			sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, No_Res_Avail, 0, 0, 1, GTPV2_F_TEID, 1));
			return;
		}
		pdnGw->setPath(path);
		pdnGw->setLocalId(module->genTeids());
		pdnGw->setFwTunnelEndpoint(this);
		module->addTunnelEndpoint(pdnGw);

		// set the newly created TE as S5S8 tunnel endpoint to PDN connection
		conn->setS5S8TunnEnd(pdnGw);
		conn->setPDNGwAddress(pdnGw->getRemoteAddr());

		// initialize user tunnel towards PGW
		GTPPath *userPath = module->findPath(pdnGw->getPath()->getRemoteAddr());
		if (userPath == NULL) {
			sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, No_Res_Avail, 0, 0, 1, GTPV2_F_TEID, 1));
			return;
		}
		TunnelEndpoint *userTe = new TunnelEndpoint(userPath);
		module->addTunnelEndpoint(userTe);
		BearerContext *defBearer = conn->getDefaultBearer();
		defBearer->setPGWTunnEnd(userTe);
		defBearer->tunnEnds[2] = userTe;

		// forward message to PGW
		sub->pushSeqNr(msg->getHeader()->getSeqNr());
		if (sub->getGTPProcedure() == EUTRANInitAttachReq)
			sub->setStatus(SUB_PENDING);
		pdnGw->sendCreateSessionRequest();
	} else if (path->getType() == S5_S8_PGW_GTP_C) {	// for PGW
		// set this as S5S8 tunnel endpoint to PDN connection
		conn->setS5S8TunnEnd(this);
		conn->setPDNGwAddress(this->getLocalAddr());

		// initialize user tunnel towards SGW
		BearerContext *defBearer = conn->getDefaultBearer();
		TunnelEndpoint *userTe = defBearer->tunnEnds[2];
		if (userTe == NULL) {
			sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Con_IE_Missing, 0, 0, 1, GTPV2_F_TEID, 2));
			return;
		}
		GTPPath *path = module->findPath(userTe->getRemoteAddr(), userTe->getType());
		if (path == NULL) {
			sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, No_Res_Avail, 0, 0, 1, GTPV2_F_TEID, 1));
			return;
		}
		userTe->setPath(path);
		userTe->setLocalId(module->genTeids());
		defBearer->setSGWTunnEnd(userTe);
		module->addTunnelEndpoint(userTe);

		// send response back to SGW
//		if (sub->getGTPProcedure() == EUTRANInitAttachReq)
//			sub->setStatus(SUB_ACTIVE);
		sub->setGTPProcedure(EUTRANInitAttachRes);
		sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Request_Accepted));
	}
}

void TunnelEndpoint::sendCreateSessionResponse(unsigned seqNr, GTPInfoElem *cause) {
	GTPMessage *csr = new GTPMessage("Create-Session-Response");

	/* Create Session Response */
	csr->setHeader(GTPUtils().createHeader(CreateSessionResponse, 0, 1, remoteId, seqNr));

	/* Cause */
	csr->pushIe(cause);

	if (cause->getValue(0) > 0x3F) {
		path->send(csr);
		return;
	}

	Subscriber *sub = NULL;
	PDNConnection *conn = NULL;
	if ((sub = dynamic_cast<Subscriber*>(ownerp))) {
		if (sub->getGTPProcedure() == EUTRANInitAttachRes)
			conn = sub->getEsmEntity()->getDefPDNConnection();
	} else if ((conn = dynamic_cast<PDNConnection*>(ownerp))) {
		sub = conn->getSubscriber();
	} else {
		return;
	}

	/* Sender F-TEID */
	if (path->getType() == S11_S4_SGW_GTP_C)
		csr->pushIe(this->createFteidIE(0));

	/* PGW S5/S8 Address */
	if (path->getType() == S5_S8_PGW_GTP_C)
		csr->pushIe(this->createFteidIE(1));
//
//	/* PDN Address Allocation */
//	csr->pushIe(GTPUtils().createAddrIE(GTPV2_PAA, 0, conn->getSubscriberAddress()));
//
//	/* APN Restriction */
//	csr->pushIe(GTPUtils().createIntegerIE(GTPV2_APNRestriction, 0, 0));
//
	/* PDN Connection info */
	conn->toGTPMessage(csr);
//
	path->send(csr);
}

void TunnelEndpoint::processCreateSessionResponse(GTPMessage *msg) {

	/* Cause */
	GTPInfoElem *cause = msg->findIe(GTPV2_Cause, 0);
	if (cause == NULL) {
		/* TODO */
		// send error to MME
		return;
	}

	if (GTPUtils().processCauseIE(cause) != Request_Accepted) {
		/* TODO */
		// send error to MME
		return;
	}

	if (path->getType() == S5_S8_SGW_GTP_C) {	// for SGW
		// find out remoteId from PGW
		/* PGW S5/S8 Address */
		GTPInfoElem *pdnGwAddr = msg->findIe(GTPV2_F_TEID, 1);
		if (pdnGwAddr == NULL) {
			/* TODO */
			// send error to MME
			return;
		}
		if (!processFteidIE(pdnGwAddr)) {
			/* TODO */
			// send error to MME
			return;
		}
//
//		/* PDN Address Allocation */
//		GTPInfoElem *pdnAddrAlloc = msg->findIe(GTPV2_PAA, 0);
//		if (pdnAddrAlloc == NULL) {
//			/* TODO */
//			// send error to MME
//			return;
//		}

		PDNConnection *conn = dynamic_cast<PDNConnection*>(ownerp);
		Subscriber *sub = conn->getSubscriber();

		if (conn->isDefault())
			sub->setGTPProcedure(EUTRANInitAttachRes);
		if (!conn->fromGTPMessage(msg, this)) {
			/* TODO */
			// send error to MME
			return;
		}

		// get remote id from PGW for user tunnel
		BearerContext *defBearer = conn->getDefaultBearer();
		TunnelEndpoint *userPgwTe = defBearer->getPGWTunnEnd();
		userPgwTe->setRemoteId(defBearer->tunnEnds[2]->getRemoteId());

		// initialize user tunnel towards ENB
		GTPPath *userPath = module->findPath(fwTunnEnd->getPath()->getRemoteAddr());
		if (userPath == NULL) {
			/* TODO */
			// send error to MME
			return;
		}
		TunnelEndpoint *userEnbTe = new TunnelEndpoint(userPath);
		userEnbTe->setFwTunnelEndpoint(userPgwTe);
		module->addTunnelEndpoint(userEnbTe);
		defBearer->setENBTunnEnd(userEnbTe);
		defBearer->tunnEnds[0] = userEnbTe;
		defBearer->tunnEnds[2] = NULL;
//		if (sub->getGTPProcedure() == EUTRANInitAttachRes)
//			sub->setStatus(SUB_ACTIVE);
		fwTunnEnd->sendCreateSessionResponse(sub->popSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Request_Accepted));
	} else if (path->getType() == S11_MME_GTP_C) {	// for MME
		// find out remoteId from SGW
		/* Sender F-TEID */
		GTPInfoElem *senderAddr = msg->findIe(GTPV2_F_TEID, 0);
		if (senderAddr == NULL) {
			/* TODO */
			// send error to MME
			return;
		}
		if (!processFteidIE(senderAddr)) {
			/* TODO */
			// send error to MME
			return;
		}

		// put user tunnel endpoint in table in order to send it to ENB
		Subscriber *sub = dynamic_cast<Subscriber*>(ownerp);
		if (sub->getEmmEntity()->getState() == EMM_DEREGISTERED_N)
			sub->setGTPProcedure(EUTRANInitAttachRes);

		PDNConnection *conn = NULL;
		if (sub->getGTPProcedure() == EUTRANInitAttachRes)
			conn = sub->getDefaultPDNConn();

		if (conn != NULL) {
			if (!conn->fromGTPMessage(msg, this)) {
				/* TODO */
				// send error to MME
				return;
			}
			BearerContext *defBearer = conn->getDefaultBearer();
			TunnelEndpoint *userTe = defBearer->tunnEnds[0];
			if (userTe == NULL) {
				/* TODO */
				// send error to MME
				return;
			}
			userTe->getPath()->setType(S1_U_eNodeB_GTP_U);
			defBearer->setENBTunnEnd(userTe);
			dynamic_cast<GTPControl*>(module)->fireChangeNotification(NF_SUB_PDN_ACK, conn);
		}
	}
}

void TunnelEndpoint::sendModifyBearerRequest() {
	Subscriber *sub = NULL;
	PDNConnection *conn = NULL;
	if ((sub = dynamic_cast<Subscriber*>(ownerp))) {
		if (sub->getGTPProcedure() == EUTRANInitAttachReq)
			conn = sub->getEsmEntity()->getDefPDNConnection();
	} else if ((conn = dynamic_cast<PDNConnection*>(ownerp))) {
		sub = conn->getSubscriber();
	} else {
		return;
	}

	GTPMessage *mbr = new GTPMessage("Modify-Bearer-Request");

	/* Modify Bearer Request */
	mbr->setHeader(GTPUtils().createHeader(ModifyBearerRequest, 0, 1, remoteId, uniform(0, 1000)));

	/* Bearer Contexts to be modified */
	BearerContext *bearer = sub->getDefaultPDNConn()->getDefaultBearer();
	bearer->tunnEnds[0] = bearer->getENBTunnEnd();
	mbr->pushIe(bearer->createBearerContextIE(0));

	sub->pushSeqNr(mbr->getHeader()->getSeqNr());
	path->send(mbr);
}

void TunnelEndpoint::sendModifyBearerResponse(unsigned seqNr, GTPInfoElem *cause) {
	GTPMessage *msr = new GTPMessage("Modify-Session-Response");

	/* Create Session Response */
	msr->setHeader(GTPUtils().createHeader(ModifyBearerResponse, 0, 1, remoteId, seqNr));

	/* Cause */
	msr->pushIe(cause);

	if (cause->getValue(0) > 0x3F) {
		path->send(msr);
		return;
	}

	Subscriber *sub = NULL;
	PDNConnection *conn = NULL;
	if ((sub = dynamic_cast<Subscriber*>(ownerp))) {
		if (sub->getGTPProcedure() == EUTRANInitAttachRes)
			conn = sub->getEsmEntity()->getDefPDNConnection();
	} else if ((conn = dynamic_cast<PDNConnection*>(ownerp))) {
		sub = conn->getSubscriber();
	} else {
		return;
	}

//	/* Sender F-TEID */
//	if (path->getType() == S11_S4_SGW_GTP_C)
//		csr->pushIe(this->createFteidIE(0));
//
//	/* PGW S5/S8 Address */
//	if (path->getType() == S5_S8_PGW_GTP_C)
//		csr->pushIe(this->createFteidIE(1));
////
////	/* PDN Address Allocation */
////	csr->pushIe(GTPUtils().createAddrIE(GTPV2_PAA, 0, conn->getSubscriberAddress()));
////
////	/* APN Restriction */
////	csr->pushIe(GTPUtils().createIntegerIE(GTPV2_APNRestriction, 0, 0));
////
//	/* PDN Connection info */
//	conn->toGTPMessage(csr);
////
	path->send(msr);
}

void TunnelEndpoint::processModifyBearerRequest(GTPMessage *msg) {
//	unsigned char proc = 100;

	if (path->getType() == S11_S4_SGW_GTP_C) {	// for SGW
		Subscriber *sub = dynamic_cast<Subscriber*>(ownerp);
		BearerContext *bearer = sub->getDefaultPDNConn()->getDefaultBearer();

		/* Bearer Context to be modified*/
		std::vector<GTPInfoElem*> bearerCtxts = msg->findIes(GTPV2_BearerContext, 0);
		if (bearerCtxts.size() < 1) {
			sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Man_IE_Missing, 0, 0, 1, GTPV2_BearerContext, 0));
			return;
		}
		/* TODO process all bearers */
		bearer->processBearerContextIE(bearerCtxts[0]);
		TunnelEndpoint *enbTe = bearer->getENBTunnEnd();
		enbTe->setRemoteId(bearer->tunnEnds[0]->getRemoteId());

		// send back response to MME
		sendModifyBearerResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Request_Accepted));
		if (sub->getGTPProcedure() == EUTRANInitAttachRes) {
			sub->setGTPProcedure(NoProcedure);
			sub->setStatus(SUB_ACTIVE);
		}
	} else if (path->getType() == S5_S8_PGW_GTP_C) { // for PGW
		PDNConnection *conn = dynamic_cast<PDNConnection*>(ownerp);
		Subscriber *sub = conn->getSubscriber();
		sendModifyBearerResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Request_Accepted));
		if (sub->getGTPProcedure() == EUTRANInitAttachRes) {
			sub->setGTPProcedure(NoProcedure);
			sub->setStatus(SUB_ACTIVE);
		}
	}
}

void TunnelEndpoint::processModifyBearerResponse(GTPMessage *msg) {
	if (path->getType() == S11_MME_GTP_C) { // for MME
		Subscriber *sub = dynamic_cast<Subscriber*>(ownerp);
		if (sub->getGTPProcedure() == EUTRANInitAttachRes) {
			sub->setGTPProcedure(NoProcedure);
			sub->setStatus(SUB_ACTIVE);
		}
	}

}

void TunnelEndpoint::processTunnelMessage(GTPMessage *msg) {
	EV << "GTP: Processing Tunnel Message\n";

	switch(msg->getHeader()->getType()) {
		case CreateSessionRequest:
			processCreateSessionRequest(msg);
			break;
		case CreateSessionResponse:
			processCreateSessionResponse(msg);
			break;
		case ModifyBearerRequest:
			processModifyBearerRequest(msg);
			break;
		case ModifyBearerResponse:
			processModifyBearerResponse(msg);
			break;
		default:;
	}
}

void TunnelEndpoint::setOwner(cPolymorphic *ownerp) {
	this->ownerp = ownerp;
}

cPolymorphic *TunnelEndpoint::getOwner() {
	return ownerp;
}

std::string TunnelEndpoint::info(int tabs) const {

    std::stringstream out;
    out << "lTEID:" << localId << "  ";
    out << "rTEID:" << remoteId << "  ";
    if (fwTunnEnd != NULL) {
    	out << "fwd: yes  ";
    }
    if (path != NULL) {
    	out << "path:{ " << path->info(tabs + 1);
    	out << " }";
    }
    return out.str();
}

GTPInfoElem *TunnelEndpoint::createFteidIE(unsigned char instance) {
	unsigned len = path->getLocalAddr().isIPv6() ? 21 : 9;
	char *val = (char*)calloc(len, sizeof(char));
	char *p = val;

	*((char*)p) = (path->getLocalAddr().isIPv6() ? 0x40 : 0x80) + path->getType();
	p += 1;
	*((unsigned*)p) = ntohl(localId);
	p += 4;
	if (path->getLocalAddr().isIPv6()) {	// not supported
	} else {
		*((unsigned*)p) = ntohl(path->getLocalAddr().get4().getInt());
		p += 4;
	}
	return GTPUtils().createIE(GTPV2_F_TEID, len, instance, val);
}

bool TunnelEndpoint::processFteidIE(GTPInfoElem *fteid) {
	if (module == NULL)
		return false;
	char *val = GTPUtils().processIE(fteid);
	char *p = val;
	if ((*p & 0x80)) {
		unsigned char type = *p & 31;
		p++;
		unsigned char remoteId = ntohl(*((unsigned*)(p)));
		p += 4;
		unsigned addr = ntohl(*((unsigned*)(p)));
		p += 4;
		if (path == NULL) {
			GTPPath *path = module->findPath(IPv4Address(addr), type);
			if (path == NULL)
				return false;
			this->path = path;
			this->module = path->getModule();
			this->localId = module->genTeids();
		}
		this->remoteId = remoteId;
	} else if ((*p & 0x40)) {	// ipv6 not supported

	}

	return true;
}
