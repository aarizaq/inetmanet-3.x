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

#include "EMMEntity.h"
#include "NAS.h"
#include "NASSerializer.h"
#include "LTEUtils.h"
#include "NASUtils.h"
#include "PhyControlInfo_m.h"

EMMEntity::EMMEntity() {
	init();
}

EMMEntity::EMMEntity(unsigned char appType) {
	// TODO Auto-generated constructor stub
	init();
	this->appType = appType;
	nasKey = uniform(0, 6);

	if (appType == UE_APPL_TYPE) {
		fsm = new cFSM("fsm-EMM-UE");
		fsm->setState(EMM_NULL);
		take(fsm);
	} else if (appType == MME_APPL_TYPE) {
		fsm = new cFSM("fsm-EMM-MME");
		fsm->setState(EMM_DEREGISTERED_N);
		take(fsm);
	}
}

EMMEntity::~EMMEntity() {
	// TODO Auto-generated destructor stub
	if (fsm != NULL)
		dropAndDelete(fsm);

	if (imsi != NULL)
		delete imsi;

	delete [] ueNetworkCapability;

	if (t3410 != NULL) {
		if (t3410->getContextPointer() != NULL)
			module->cancelEvent(t3410);
		delete t3410;
	}
	if (t3402 != NULL) {
		if (t3402->getContextPointer() != NULL)
			module->cancelEvent(t3402);
		delete t3402;
	}
	if (t3411 != NULL) {
		if (t3411->getContextPointer() != NULL)
			module->cancelEvent(t3411);
		delete t3411;
	}
	if (t3450 != NULL) {
		if (t3450->getContextPointer() != NULL)
			module->cancelEvent(t3450);
		delete t3450;
	}
}

void EMMEntity::init() {
	attCounter = 0;
	tracAreaUpdCounter = 0;
	nasKey = 0;
	appType = 0;
	module = NULL;
	epsUpdSt = -1;
	fsm = NULL;
	ueNetworkCapability = new char[3];
	ueNetworkCapability[0] = 0x02;
	ueNetworkCapability[1] = 0x80;
	ueNetworkCapability[2] = 0x40;
	emmCause = -1;
	t3402 = NULL;
	t3410 = NULL;
	t3411 = NULL;
	t3450 = NULL;
	imsi = NULL;
	tmsi = 0;
	peer = NULL;
}

void EMMEntity::performStateTransition(EMMSublayerEvent event) {

	int oldState = fsm->getState();
	switch(oldState) {
		case EMM_NULL:
			switch(event) {
				case SwitchOn:	// switch on
					FSM_Goto(*fsm, EMM_DEREGISTERED_PLMN_SEARCH);
					break;
				default:
					EV << "NAS-EMM: Received unexpected event\n";
					break;
			}
			break;
		case EMM_DEREGISTERED_PLMN_SEARCH:
			switch(event) {
				case NoUSIM:
					FSM_Goto(*fsm, EMM_DEREGISTERED_NO_IMSI);
					break;
				case CellFoundPermittedPLMN:
					FSM_Goto(*fsm, EMM_DEREGISTERED_NORMAL_SERVICE);
					break;
				default:
					EV << "NAS-EMM: Received unexpected event\n";
					break;
			}
			break;
		case EMM_DEREGISTERED_NORMAL_SERVICE:
			switch(event) {
				case AttachRequested:
					FSM_Goto(*fsm, EMM_REGISTERED_INITIATED);
					break;
				default:
					EV << "NAS-EMM: Received unexpected event\n";
					break;
			}
			break;
		case EMM_DEREGISTERED_N:
			switch(event) {
				case AttachAccepted: {
					if (tmsi == 0) {
						tmsi = uniform(1, 1000);
						FSM_Goto(*fsm, EMM_COMMON_PROCEDURE_INITIATED);
					}
					NASPlainMessage *msg = createAttachAccept();
					module->sendToS1AP(msg, ownerp->getEnbId(), ownerp->getMmeId());
					// else remain in this state
					break;
				}
				case AttachRejected: {
					NASPlainMessage *msg = createAttachReject();
					module->sendToS1AP(msg, ownerp->getEnbId(), ownerp->getMmeId());
					break;
				}
				case AttachCompleted: {
					/* TODO stop T3450 */
					FSM_Goto(*fsm, EMM_REGISTERED_N);
					break;
				}
				default:
					EV << "NAS-EMM: Received unexpected event\n";
					break;
			}
			break;
		case EMM_REGISTERED_INITIATED:
			switch(event) {
				case AttachAccepted: {
					attCounter = 0;
					tracAreaUpdCounter = 0;
					module->cancelEvent(t3410);
					epsUpdSt = EU1_UPDATED;
					NASPlainMessage *msg = createAttachComplete();
					module->sendToRadio(msg, module->getChannelNumber());
					FSM_Goto(*fsm, EMM_REGISTERED_U);
					break;
				}
				case AttachRejected: {
					if (emmCause == EPSServNonEPSServNotAllowed) {
						epsUpdSt = EU3_ROAMING_NOT_ALLOWED;
						FSM_Goto(*fsm, EMM_DEREGISTERED_U);
					}
					break;
				}
				default:
					EV << "NAS-EMM: Received unexpected event\n";
					break;
			}
			break;
		case EMM_COMMON_PROCEDURE_INITIATED:
			switch(event) {
				case AttachCompleted:
					/* TODO stop T3450 */
					FSM_Goto(*fsm, EMM_REGISTERED_N);
					break;
				default:
					EV << "NAS-EMM: Received unexpected event\n";
					break;
			}
			break;
		default:
			EV << "NAS-EMM: Unknown state\n";
			break;
	}

    if (oldState != fsm->getState())
        EV << "NAS-EMM: PSM-Transition: " << stateName(oldState) << " --> " << stateName(fsm->getState()) << "  (event was: " << eventName(event) << ")\n";
    else
        EV << "NAS-EMM: Staying in state: " << stateName(fsm->getState()) << " (event was: " << eventName(event) << ")\n";

    stateEntered();
}

void EMMEntity::stateEntered() {
	switch(fsm->getState()) {
		case EMM_DEREGISTERED_NORMAL_SERVICE: {
			attCounter = 0;
			// send attach request with default pdn connection
			NASPlainMessage *msg = createAttachRequest();
			module->sendToRadio(msg, module->getChannelNumber());
			// start t3410, reset t3402 and t3411
			startT3410();
			// go to EMM_REGISTERED_INITIATED
			ownerp->setStatus(SUB_PENDING);
			performStateTransition(AttachRequested);
			break;
		}
		case EMM_DEREGISTERED_U:
		case EMM_DEREGISTERED_N:
			ownerp->setStatus(SUB_INACTIVE);
			break;
		case EMM_REGISTERED_NORMAL_SERVICE:
		case EMM_REGISTERED_N:
			ownerp->setStatus(SUB_ACTIVE);
			break;
		default:
		    break;
	}
}

NASPlainMessage *EMMEntity::createAttachRequest() {
	NASPlainMessage *msg = new NASPlainMessage("Attach-Request");
	msg->setHdr(NASUtils().createHeader(0, PlainNASMessage, EMMMessage, 0, AttachRequest));
	msg->setIesArraySize(5);

	/* NAS key set identifier */
	msg->setIes(0, NASUtils().createIE(IE_V, IEType1, 0, nasKey));

	/* EPS attach type */
	msg->setIes(1, NASUtils().createIE(IE_V, IEType1, 0, EPSAttach));

	/* EPS mobile identity */
	msg->setIes(2, NASUtils().createEPSMobileIdIE(IE_LV, IMSI_ID, imsi));

	/* UE network capability */
	msg->setIes(3, NASUtils().createIE(IE_LV, IEType4, 0, 3, ueNetworkCapability));

	/* ESM message container */
	NASPlainMessage *pdnConnReq = peer->getDefPDNConnection()->createPDNConnectivityRequest();
	msg->encapsulate(pdnConnReq);
	msg->setEncapPos(4);

	return msg;
}

void EMMEntity::processAttachRequest(NASPlainMessage *msg) {
	/* EPS mobile identity */
	char *id;
	unsigned len = NASUtils().processEPSMobileIdIE(msg->getIes(2), id);
	if (len == IMSI_CODED_SIZE)
		imsi = id;

	/* ESM message container */
	NASPlainMessage *smsg = check_and_cast<NASPlainMessage*>(msg->decapsulate());
	if (smsg->getHdr().getMsgType() == PDNConnectivityRequest) {
		// initialize PDN connection for this UE
		PDNConnection *conn = new PDNConnection(peer);
		conn->processPDNConnectivityRequest(smsg);
		if (fsm->getState() == EMM_DEREGISTERED_N)
			peer->addPDNConnection(conn, true);
		else
			peer->addPDNConnection(conn, false);
	}
	delete msg;
}

NASPlainMessage *EMMEntity::createAttachAccept() {
	NASPlainMessage *msg = new NASPlainMessage("Attach-Accept");
	msg->setHdr(NASUtils().createHeader(0, PlainNASMessage, EMMMessage, 0, AttachAccept));
	msg->setIesArraySize(5);

	/* EPS attach result */
	msg->setIes(0, NASUtils().createIE(IE_V, IEType1, 0, 2));	// 2 = combined EPS/IMSI attach

	/* Spare half octet */
	msg->setIes(1, NASUtils().createIE(IE_V, IEType1, 0, 0));

	/* T3412 value */
	// 111 - timer deactivated, 00000 - timer deactivated (for tracking area update)
	msg->setIes(2, NASUtils().createIE(IE_V, IEType3, 0, 224));

	/* TAI list */
	char *taiList = (char*)calloc(6, sizeof(char));
	taiList[0] = 0; // 00 - type list, 00000 number of elements
	memcpy(taiList + 1, ownerp->getPlmnId(), PLMNID_CODED_SIZE);
	memcpy(taiList + 1 + PLMNID_CODED_SIZE, ownerp->getTac(), TAC_CODED_SIZE);
	msg->setIes(3, NASUtils().createIE(IE_LV, IEType4, 0, 6, taiList));

	/* ESM message container */
	NASPlainMessage *actDefBearerReq = peer->getDefPDNConnection()->createActDefBearerRequest();
	msg->encapsulate(actDefBearerReq);
	msg->setEncapPos(4);

	return msg;
}

void EMMEntity::processAttachAccept(NASPlainMessage *msg) {
	/* ESM message container */
	PDNConnection *conn = peer->getDefPDNConnection();
	NASPlainMessage *smsg = check_and_cast<NASPlainMessage*>(msg->decapsulate());
	if (smsg->getHdr().getMsgType() == ActDefEPSBearerCtxtReq)
		conn->processActDefBearerRequest(smsg);

	performStateTransition(AttachAccepted);
	delete msg;
}

NASPlainMessage *EMMEntity::createAttachReject() {
	NASPlainMessage *msg = new NASPlainMessage("Attach-Reject");
	msg->setHdr(NASUtils().createHeader(0, PlainNASMessage, EMMMessage, 0, AttachReject));
	msg->setIesArraySize(1);

	/* EMM Cause */
	msg->setIes(0, NASUtils().createIE(IE_V, IEType3, 0, (unsigned char)emmCause));

	/* ESM message container */
//	NASPlainMessage *actDefBearerReq = peer->getDefPDNConnection()->createActDefBearerRequest();
//	msg->encapsulate(actDefBearerReq);
//	msg->setEncapPos(4);

	return msg;
}

void EMMEntity::processAttachReject(NASPlainMessage *msg) {
	/* EMM Cause */
	emmCause = msg->getIes(0).getValue(0);

	/* ESM message container */
//	PDNConnection *conn = peer->getDefPDNConnection();
//	NASPlainMessage *smsg = check_and_cast<NASPlainMessage*>(msg->decapsulate());
//	if (smsg->getHdr().getMsgType() == ActDefEPSBearerCtxtReq)
//		conn->processActDefBearerRequest(smsg);

	performStateTransition(AttachRejected);
	delete msg;
}


NASPlainMessage *EMMEntity::createAttachComplete() {
	NASPlainMessage *msg = new NASPlainMessage("Attach-Complete");
	msg->setHdr(NASUtils().createHeader(0, PlainNASMessage, EMMMessage, 0, AttachComplete));
	msg->setIesArraySize(1);

	/* ESM message container */
	NASPlainMessage *actDefBearerReq = peer->getDefPDNConnection()->createActDefBearerAccept();
	msg->encapsulate(actDefBearerReq);
	msg->setEncapPos(0);

	return msg;
}

void EMMEntity::processAttachComplete(NASPlainMessage *msg) {
	/* ESM message container */
	PDNConnection *conn = peer->getDefPDNConnection();
	NASPlainMessage *smsg = check_and_cast<NASPlainMessage*>(msg->decapsulate());
	if (smsg->getHdr().getMsgType() == ActDefEPSBearerCtxtAcc)
		conn->processActDefBearerAccept(smsg);

	performStateTransition(AttachCompleted);
	delete msg;
}

const char *EMMEntity::stateName(int state) const {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (state) {
        CASE(EMM_NULL);
        CASE(EMM_DEREGISTERED_U);
        CASE(EMM_DEREGISTERED_NORMAL_SERVICE);
        CASE(EMM_DEREGISTERED_LIMITED_SERVICE);
        CASE(EMM_DEREGISTERED_ATTEMPTING_TO_ATTACH);
        CASE(EMM_DEREGISTERED_PLMN_SEARCH);
        CASE(EMM_DEREGISTERED_NO_IMSI);
        CASE(EMM_DEREGISTERED_ATTACH_NEEDED);
        CASE(EMM_DEREGISTERED_NO_CELL_AVAILABLE);
        CASE(EMM_REGISTERED_U);
        CASE(EMM_REGISTERED_INITIATED);
        CASE(EMM_REGISTERED_NORMAL_SERVICE);
        CASE(EMM_REGISTERED_ATTEMPTING_TO_UPDATE);
        CASE(EMM_REGISTERED_LIMITED_SERVICE);
        CASE(EMM_REGISTERED_PLMN_SEARCH);
        CASE(EMM_REGISTERED_UPDATE_NEEDED);
        CASE(EMM_REGISTERED_ATTEMPTING_TO_UPDATE_MM);
        CASE(EMM_REGISTERED_IMSI_DETACH_INITIATED);
        CASE(EMM_DEREGISTERED_INITIATED_U);
        CASE(EMM_TRACKING_AREA_UPDATING_INITIATED);
        CASE(EMM_SERVICE_REQUEST_INITIATED);
        CASE(EMM_DEREGISTERED_N);
        CASE(EMM_COMMON_PROCEDURE_INITIATED);
        CASE(EMM_REGISTERED_N);
        CASE(EMM_DEREGISTERED_INITIATED_M);
    }
    return s;
#undef CASE
}

const char *EMMEntity::statusName() const {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (epsUpdSt) {
        CASE(EU1_UPDATED);
        CASE(EU2_NOT_UPDATED);
        CASE(EU3_ROAMING_NOT_ALLOWED);
    }
    return s;
#undef CASE
}

const char *EMMEntity::eventName(int event) {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (event) {
        CASE(EnableS1Mode);
        CASE(NoUSIM);
        CASE(CellFoundPermittedPLMN);
        CASE(SwitchOn);
        CASE(AttachRequested);
        CASE(AttachAccepted);
        CASE(AttachRejected);
        CASE(AttachCompleted);
    }
    return s;
#undef CASE
}

void EMMEntity::setModule(NAS *module) {
	this->module = module;
}

NAS *EMMEntity::getModule() {
	return module;
}

void EMMEntity::setOwner(Subscriber *ownerp) {
	ownerp->setEmmEntity(this);
	this->ownerp = ownerp;
}

Subscriber *EMMEntity::getOwner() {
	return ownerp;
}

void EMMEntity::startT3410() {
	if (t3410 == NULL) {
		t3410 = new cMessage("T3410");
		t3410->setContextPointer(this);
	}
	module->scheduleAt(simTime() + T3410_TIMEOUT, t3410);
	if (t3402 != NULL)
		module->cancelEvent(t3402);
	if (t3411 != NULL)
		module->cancelEvent(t3411);
}

void EMMEntity::setPeer(ESMEntity *peer) {
	this->peer = peer;
}

ESMEntity *EMMEntity::getPeer() {
	return peer;
}

std::string EMMEntity::info(int tabs) const {
	std::stringstream out;
    if (imsi != NULL) {
    	for (int i = 0; i < tabs; i++) out << "\t";
    	out << "imsi:" << LTEUtils().toASCIIString(imsi, IMSI_CODED_SIZE) << "\n";
    }
    if (epsUpdSt != -1) {
    	for (int i = 0; i < tabs; i++) out << "\t";
    	out << "epsUpdSt:" << statusName() << "\n";
    }
    if (fsm != NULL) {
    	for (int i = 0; i < tabs; i++) out << "\t";
    	out << "emmSt:" << stateName(fsm->getState()) << "\n";
    }

    return out.str();
}
