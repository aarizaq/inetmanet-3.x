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

#include "DiameterS6a.h"
#include "DiameterUtils.h"
#include "LTEUtils.h"

Define_Module(DiameterS6a);

DiameterS6a::DiameterS6a() {
	// TODO Auto-generated constructor stub
	sessionIds = 1;
}

DiameterS6a::~DiameterS6a() {
	// TODO Auto-generated destructor stub
}

void DiameterS6a::initialize(int stage){
	DiameterBase::initialize(stage);
	subT = SubscriberTableAccess().get();
	nb = NotificationBoardAccess().get();
	nb->subscribe(this, NF_SUB_NEEDS_AUTH);
}

DiameterMessage *DiameterS6a::createULR(Subscriber *sub) {
	DiameterMessage *ulr = new DiameterMessage("Update-Location-Request");
	ulr->setHdr(DiameterUtils().createHeader(UpdateLocation, 1, 1, 0, 0, 0, uniform(0, 1000), uniform(0, 1000)));

	ulr->pushAvp(DiameterUtils().createUTF8StringAVP(AVP_UserName, 1, 1, 0, TGPP, LTEUtils().toASCIIString(sub->getEmmEntity()->getImsi(), IMSI_CODED_SIZE, TBCD_TYPE)));
	ulr->pushAvp(DiameterUtils().createInteger32AVP(AVP_RATType, 1, 1, 0, TGPP, DIAMETER_EUTRAN));
	ulr->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_ULRFlags, 1, 1, 0, TGPP, 34));	// bit 5 for initial attach and bit 1 for S6a intf
	ulr->pushAvp(DiameterUtils().createOctetStringAVP(AVP_VisitedPLMNId, 1, 1, 0, TGPP, PLMNID_CODED_SIZE, sub->getPlmnId()));

	sub->pushSeqNr(ulr->getHdr().getEndToEndId());
	return ulr;
}

DiameterMessage *DiameterS6a::createULA(DiameterMessage *ulr) {
	DiameterMessage *ula = new DiameterMessage("Update-Location-Answer");
	DiameterHeader hdr = ulr->getHdr();
	hdr.setReqFlag(false);
	ula->setHdr(hdr);

	AVP *userName = ulr->findAvp(AVP_UserName);
	if (userName == NULL) {
		ula->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_ResultCode, 0, 1, 0, 0, DIAMETER_MISSING_AVP));
		return ula;
	}

	Subscriber *sub = subT->findSubscriberForIMSI(LTEUtils().toIMSI(DiameterUtils().processUTF8String(userName)));
	if (sub == NULL) {
		ula->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_ExpResultCode, 0, 1, 0, 0, DIAMETER_ERROR_USER_UNKNOWN));
		return ula;
	}

	ula->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_ULAFlags, 1, 0, 0, TGPP, 1));	// HSS stores MME and SGSN number in separate memory

	AVP *apnConfigProf = sub->getEsmEntity()->createAPNConfigProfAVP();
	std::vector<AVP*> subscrData;
	subscrData.push_back(DiameterUtils().createOctetStringAVP(AVP_MSISDN, 1, 0, 0, TGPP, MSISDN_CODED_SIZE, sub->getMsisdn()));
	if (apnConfigProf != NULL) {
		subscrData.push_back(apnConfigProf);
	}
	ula->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_ResultCode, 0, 1, 0, 0, DIAMETER_SUCCESS));
	ula->pushAvp(DiameterUtils().createGroupedAVP(AVP_SubscriptionData, 1, 0, 0, TGPP, subscrData));
	DiameterUtils().deleteGroupedAVP(subscrData);
	return ula;
}

void DiameterS6a::processULA(DiameterMessage *ula) {
	Subscriber *sub = subT->findSubscriberForSeqNr(ula->getHdr().getEndToEndId());
	if (sub == NULL) {
		EV << "DiameterS6a: Response received for unknown subscriber. Discarding.\n";
		return;
	}
	EMMEntity *emm = sub->getEmmEntity();
	{
		AVP *expResCode = ula->findAvp(AVP_ExpResultCode);
		if (expResCode) {
			EV << "DiameterS6a: Response received with experimental ResultCode.\n";
			unsigned resCode = DiameterUtils().processUnsigned32AVP(expResCode);
			if (resCode == DIAMETER_ERROR_USER_UNKNOWN)
				emm->setEmmCause(EPSServNonEPSServNotAllowed);
			goto error;
		}

		AVP *subscrData = ula->findAvp(AVP_SubscriptionData);
		if (subscrData != NULL) {
			std::vector<AVP*> subscrDataVec = DiameterUtils().processGroupedAVP(subscrData);
			AVP *msisdn = DiameterUtils().findAVP(AVP_MSISDN, subscrDataVec);
			if (msisdn == NULL) {
				EV << "DiameterS6a: Missing MSISDN AVP.\n";
				goto error;
			}
			sub->setMsisdn(DiameterUtils().processOctetStringAVP(msisdn));
			AVP *apnConfigProf = DiameterUtils().findAVP(AVP_APNConfigProfile, subscrDataVec);
			if (apnConfigProf == NULL) {
				EV << "DiameterS6a: Missing APNConfigProfile AVP.\n";
				goto error;
			}
			if (!sub->getEsmEntity()->processAPNConfigProfAVP(apnConfigProf)) {
				goto error;
			}
			nb->fireChangeNotification(NF_SUB_AUTH_ACK, sub);
			DiameterUtils().deleteGroupedAVP(subscrDataVec);
		}
		return;
	}
	error:
		nb->fireChangeNotification(NF_SUB_AUTH_NACK, sub);
		return;
}

DiameterMessage *DiameterS6a::processMessage(DiameterMessage *msg) {
	DiameterMessage *resp = NULL;
	DiameterHeader hdr = msg->getHdr();
	switch(hdr.getCommandCode()) {
		case UpdateLocation:
			if (hdr.getReqFlag())
				resp = createULA(msg);
			else
				processULA(msg);
			break;
		default:
		    break;	// TODO should send some "unknown command" result back
	}
	delete msg;
	return resp;
}

void DiameterS6a::receiveChangeNotification(int category, const cPolymorphic *details) {
	Enter_Method_Silent();

	if (category == NF_SUB_NEEDS_AUTH) {
		EV << "DiameterS6a: Received NF_SUB_NEEDS_AUTH notification. Processing notification.\n";
		Subscriber *sub = check_and_cast<Subscriber*>(details);
		DiameterSession *session = new DiameterSession(this, NO_STATE_MAINTAINED);
		sessions.push_back(session);
		SessionEvent event = RequestsAccess;
		session->performStateTransition(event, S6a, NULL, createULR(sub));
	}
}
