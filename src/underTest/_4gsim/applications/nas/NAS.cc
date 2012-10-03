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

#include "NAS.h"
#include "LTEUtils.h"
#include "PhyControlInfo_m.h"
#include "S1APControlInfo_m.h"
#include "S1APConstant.h"
#include "NASSerializer.h"
#include "PDNConnection.h"
#include "NASUtils.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "RoutingTableAccess.h"

Define_Module(NAS)

NAS::NAS() {
	// TODO Auto-generated constructor stub
	appType = RELAY_APPL_TYPE;
}

NAS::~NAS() {
	// TODO Auto-generated destructor stub
//	if (sub != NULL)
//		delete emm;
}

void NAS::initialize(int stage) {
	if (stage == 4) {
		subT = SubscriberTableAccess().get();
		const char *fileName = par("configFile");
		if (fileName != NULL && (strcmp(fileName, "")))
			this->loadConfigFromXML(fileName);
		ift=InterfaceTableAccess().get();
		rt=RoutingTableAccess().get();
    	nb = NotificationBoardAccess().get();
    	nb->subscribe(this, NF_SUB_AUTH_ACK);
    	nb->subscribe(this, NF_SUB_AUTH_NACK);
    	nb->subscribe(this, NF_SUB_PDN_ACK);
	}
}

void NAS::handleMessage(cMessage *msg) {
	if (appType == RELAY_APPL_TYPE) {
		if (msg->isSelfMessage()) {
			msg->setName("Relay-Message");
//	        NASRelay *relay = (NASRelay*)msg->getContextPointer();
//	        relay->processTimer(msg);
		} else if (msg->arrivedOn("radioIn")) {
			EV << "NAS-RELAY: Received message from radio. Relaying message.\n";
			handleMessageFromRadio(msg);
		} else {
			EV << "NAS-RELAY: Received message from S1AP. Relaying message.\n";
			handleMessageFromS1AP(msg);
		}
	} else if (appType == MME_APPL_TYPE) {
		msg->setName("MME-Message");
		if (msg->isSelfMessage()) {

		} else {
			EV << "NAS-MME: Received message from S1AP. Processing message.\n";
			handleMessageFromS1AP(msg);
		}
	} else if (appType == UE_APPL_TYPE) {
		msg->setName("UE-Message");
		if (msg->isSelfMessage()) {

		} else {
			EV << "NAS-UE: Received message from radio. Processing message.\n";
			handleMessageFromRadio(msg);
		}
	}
}

void NAS::handleMessageFromS1AP(cMessage *msg) {
	S1APControlInfo *ctrl = dynamic_cast<S1APControlInfo *>(msg->removeControlInfo());
	char *buf = (char*)calloc(ctrl->getValueArraySize(), sizeof(char));
	char *p = buf;
	for (unsigned i = 0; i < ctrl->getValueArraySize(); i++)
		buf[i] = ctrl->getValue(i);
	Subscriber *sub = subT->findSubscriberForId(ctrl->getUeEnbId(), ctrl->getUeMmeId());
	if (sub == NULL) {
		EV << "NAS: Unknown subscriber. Dropping message.\n";
		goto end;
	}
	{
		NASPlainMessage *nmsg = new NASPlainMessage();
		NASHeader hdr = NASSerializer().parseHeader(p);
		if (!hdr.getLength()) {
			EV << "NAS: Message decoding error. Dropping message.\n";
			goto end;
		}
		p += hdr.getLength();
		nmsg->setHdr(hdr);
		switch(hdr.getMsgType()) {
			case AttachRequest: {
				nmsg->setName("Attach-Request");
				if (!NASSerializer().parseAttachRequest(nmsg, p)) {
					EV << "NAS: Message processing error. Dropping message.\n";
					goto end;
				}
				sub->setMmeId(subT->genMmeId());
				sub->initEntities(appType);
				EMMEntity *emm = sub->getEmmEntity();
				emm->setModule(this);
				ESMEntity *esm = sub->getEsmEntity();
				esm->setModule(this);
				emm->processAttachRequest(nmsg);
				sub->setStatus(SUB_PENDING);
				nb->fireChangeNotification(NF_SUB_NEEDS_AUTH, sub);
				break;
			}
			case AttachAccept: {
				nmsg->setName("Attach-Accept");
				if (!NASSerializer().parseAttachAccept(nmsg, p)) {
					EV << "NAS: Message parsing error. Dropping message.\n";
					goto end;
				}
				sub->setStatus(SUB_ACTIVE);
				sendToRadio(nmsg, sub->getChannelNr());
				break;
			}
			case AttachReject: {
				nmsg->setName("Attach-Reject");
				if (!NASSerializer().parseAttachReject(nmsg, p)) {
					EV << "NAS: Message parsing error. Dropping message.\n";
					goto end;
				}
				sub->setStatus(SUB_INACTIVE);
				sendToRadio(nmsg, sub->getChannelNr());
				break;
			}
			case AttachComplete: {
				nmsg->setName("Attach-Complete");
				if (!NASSerializer().parseAttachComplete(nmsg, p)) {
					EV << "NAS: Message parsing error. Dropping message.\n";
					goto end;
				}
				sub->getEmmEntity()->processAttachComplete(nmsg);
				break;
			}
			default:
			    break;
		}
	}
	end:
		delete msg;
		delete ctrl;
}

void NAS::handleMessageFromRadio(cMessage *msg) {
	NASPlainMessage *nmsg = check_and_cast<NASPlainMessage*>(msg);
	PhyControlInfo *ctrl = dynamic_cast<PhyControlInfo *>(nmsg->removeControlInfo());
	Subscriber *sub = subT->findSubscriberForChannel(ctrl->getChannelNumber());
	if (appType == RELAY_APPL_TYPE) {
		if (sub == NULL) {
			sub = new Subscriber();
			sub->initEntities(appType);
			// only dummy PDN connection to store the UE bearer contexts
			PDNConnection *conn = new PDNConnection();
			conn->setOwner(sub->getEsmEntity());
			sub->getEsmEntity()->addPDNConnection(conn, true);
			sub->setChannelNr(ctrl->getChannelNumber());
			sub->setEnbId(subT->genEnbId());
			sub->setMmeId(0);
			sub->setStatus(SUB_PENDING);
			subT->push_back(sub);
		}
		sendToS1AP(nmsg, sub->getEnbId(), sub->getMmeId());
	} else {
		if (sub == NULL) {
			delete nmsg;
			return;
		}
		switch(nmsg->getHdr().getMsgType()) {
			case AttachAccept: {
				EMMEntity *emm = sub->getEmmEntity();
				ESMEntity *esm = sub->getEsmEntity();
				emm->processAttachAccept(nmsg);
				InterfaceEntry *entry = ift->getInterfaceByName("UERadioInterface");
				if(entry)
				{
					entry->ipv4Data()->setIPAddress(esm->getDefPDNConnection()->getSubscriberAddress().get4());
					entry->setMACAddress(MACAddress::generateAutoAddress());
					entry->setMtu(1500);
					IPv4Route *route=new IPv4Route();
					route->setInterface(entry);
					rt->addRoute(route);
				}
				break;
			}
			case AttachReject: {
				EMMEntity *emm = sub->getEmmEntity();
				emm->processAttachReject(nmsg);
				break;
			}
			default:
				delete nmsg;
				break;
		}
	}
	delete ctrl;
}

void NAS::sendToS1AP(NASPlainMessage *nmsg, unsigned subEnbId, unsigned subMmeId) {
	cMessage *msg = new cMessage(nmsg->getName());
	S1APControlInfo *ctrl = new S1APControlInfo();
	ctrl->setUeEnbId(subEnbId);
	ctrl->setUeMmeId(subMmeId);
	switch(nmsg->getHdr().getMsgType()) {
		case AttachRequest:
			ctrl->setProcId(id_initialUEMessage);
			break;
		case AttachAccept:
			ctrl->setProcId(id_InitialContextSetup);
			break;
		case AttachReject:
			ctrl->setProcId(id_downlinkNASTransport);
			break;
		case AttachComplete:
			ctrl->setProcId(id_uplinkNASTransport);
			break;
		default:
		    break;
	}
	char *nasPdu;
	unsigned nasPduLen = NASSerializer().serialize(nmsg, nasPdu);
	ctrl->setValueArraySize(nasPduLen);
	for (unsigned i = 0; i < ctrl->getValueArraySize(); i++)
		ctrl->setValue(i, nasPdu[i]);
	msg->setControlInfo(ctrl);

	delete nmsg;
	send(msg, gate("s1apOut"));
}

void NAS::sendToRadio(NASPlainMessage *nmsg, int channelNr) {
	PhyControlInfo *ctrl = new PhyControlInfo();
	ctrl->setChannelNumber(channelNr);
	ctrl->setBitrate(2e6);
	nmsg->setControlInfo(ctrl);
	send(nmsg, gate("radioOut"));
}

void NAS::loadConfigFromXML(const char *filename) {
    cXMLElement* config = ev.getXMLDocument(filename);
    if (config == NULL)
        error("NAS: Cannot read configuration from file: %s", filename);

    cXMLElement* nasNode = config->getElementByPath("NAS");
	if (nasNode == NULL)
		error("NAS: No configuration for NAS");

	if (!nasNode->getAttribute("appType"))
		error("NAS: No <appType> for NAS");
	appType = atoi(nasNode->getAttribute("appType"));
    if (appType == UE_APPL_TYPE) {
    	/* TODO */
    	// no cell available
    	channelNumber = par("channelNumber");

    	Subscriber *sub = new Subscriber();
    	sub->setChannelNr(channelNumber);
    	sub->initEntities(appType);
    	subT->push_back(sub);
    	loadESMConfigFromXML(*nasNode);
    	loadEMMConfigFromXML(*nasNode);
    } else if (appType == RELAY_APPL_TYPE) {

    } else {

    }
}

void NAS::loadEMMConfigFromXML(const cXMLElement &nasNode) {
	EMMEntity *emm = subT->at(0)->getEmmEntity();
	emm->setModule(this);

	// go to EMM_DEREGISTERED_PLMN_SEARCH
	emm->performStateTransition(SwitchOn);

	const char *imsi = nasNode.getAttribute("imsi");
	if (!imsi) {
		emm->performStateTransition(NoUSIM);
		return;
	}
	emm->setImsi(LTEUtils().toIMSI(imsi));
	// go to EMM_DEREGISTERED_NORMAL_SERVICE
	emm->performStateTransition(CellFoundPermittedPLMN);
	/* TODO */
	// limited-service
	/* TODO */
	// manual network mode
}

void NAS::loadESMConfigFromXML(const cXMLElement &nasNode) {
	ESMEntity *esm = subT->at(0)->getEsmEntity();
	esm->setModule(this);
	PDNConnection *conn = new PDNConnection(esm);
	esm->addPDNConnection(conn, true);
}

void NAS::receiveChangeNotification(int category, const cPolymorphic *details) {
	Enter_Method_Silent();
	if (category == NF_SUB_AUTH_ACK) {
		EV << "NAS: Received NF_SUB_AUTH_ACK notification. Processing notification.\n";
		Subscriber *sub = check_and_cast<Subscriber*>(details);
		nb->fireChangeNotification(NF_SUB_NEEDS_PDN, sub->getDefaultPDNConn());
	} else if (category == NF_SUB_AUTH_NACK) {
		EV << "NAS: Received NF_SUB_AUTH_NACK notification. Processing notification.\n";
		Subscriber *sub = check_and_cast<Subscriber*>(details);
		EMMEntity *emm = sub->getEmmEntity();
		emm->performStateTransition(AttachRejected);
	} else if (category == NF_SUB_PDN_ACK) {
		EV << "NAS: Received NF_SUB_PDN_ACK notification. Processing notification.\n";
		PDNConnection *conn = check_and_cast<PDNConnection*>(details);
		EMMEntity *emm = conn->getOwner()->getPeer();
		if (emm->getState() == EMM_DEREGISTERED_N) {
			emm->performStateTransition(AttachAccepted);
		}
	}
}
