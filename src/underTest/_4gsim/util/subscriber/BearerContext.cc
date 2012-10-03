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

#include "BearerContext.h"
#include "PDNConnection.h"
#include "GTPUtils.h"
#include "ESMEntity.h"
#include "NAS.h"
#include "GTPUtils.h"
#include "S1APIe.h"
#include "GTPSerializer.h"

BearerContext::BearerContext() {
	// TODO Auto-generated constructor stub
	init();
}

BearerContext::BearerContext(PDNConnection *ownerp) {
	init();
	this->ownerp = ownerp;
	id = ownerp->getOwner()->genBearerId();
	fsm = new cFSM();
	fsm->setName("BearerContextFSM");
	take(fsm);
	if (ownerp->getOwner()->getAppType() == UE_APPL_TYPE) {
		fsm->setState(BEARER_CONTEXT_INACTIVE_U);
	} else if (ownerp->getOwner()->getAppType() == MME_APPL_TYPE) {
		fsm->setState(BEARER_CONTEXT_INACTIVE_N);
	}
}

BearerContext::~BearerContext() {
	// TODO Auto-generated destructor stub
	// all tunnel endpoints will be deleted in TunnelEndpointTable
	// pdu should be deleted in S1AP module
	if (fsm != NULL)
		dropAndDelete(fsm);

}

void BearerContext::init() {
	id = 0;
	enbTunn = NULL;
	sgwTunn = NULL;
	pgwTunn = NULL;
	fsm = NULL;
	procId = 0;
	ownerp = NULL;
	t3485 = NULL;
	qci = 1;
	pdu = NULL;
}

void BearerContext::performStateTransition(ESMSublayerEvent event) {
	int oldState = fsm->getState();
	switch(oldState) {
		case BEARER_CONTEXT_INACTIVE_U:
			switch(event) {
				case UEInitESMProcReq:
					FSM_Goto(*fsm, PROCEDURE_TRANSACTION_PENDING_U);
					break;
				default:
					EV << "NAS-ESM: Received unexpected event\n";
					break;
			}
			break;
		case BEARER_CONTEXT_INACTIVE_N:
			switch(event) {
				case ActBearerCtxtReq: {
					/* TODO start T3485 */
					ESMEntity *esm = ownerp->getOwner();
					esm->getPeer()->performStateTransition(AttachAccepted);
					FSM_Goto(*fsm, BEARER_CONTEXT_ACTIVE_PENDING_N);
					break;
				}
				default:
					EV << "NAS-ESM: Received unexpected event\n";
					break;
			}
			break;
		case PROCEDURE_TRANSACTION_PENDING_U:
			switch(event) {
				case ActBearerCtxtReq:
					FSM_Goto(*fsm, BEARER_CONTEXT_ACTIVE_U);
					break;
				default:
					EV << "NAS-ESM: Received unexpected event\n";
					break;
			}
			break;
		case BEARER_CONTEXT_ACTIVE_U:
			switch(event) {
				case ActBearerCtxtAcc:
					FSM_Goto(*fsm, PROCEDURE_TRANSACTION_PENDING_U);
					break;
				default:
					EV << "NAS-ESM: Received unexpected event\n";
					break;
			}
			break;
		case BEARER_CONTEXT_ACTIVE_PENDING_N: {
			switch(event) {
				case ActBearerCtxtAcc:
					/* TODO stop T3485 timer */
					FSM_Goto(*fsm, BEARER_CONTEXT_ACTIVE_N);
					break;
				default:
					EV << "NAS-ESM: Received unexpected event\n";
			}
			break;
			}
		default:
			EV << "NAS-ESM: Unknown state\n";
			break;
	}

	if (oldState != fsm->getState())
	    EV << "NAS-ESM: PSM-Transition: " << stateName(oldState) << " --> " << stateName(fsm->getState()) << "  (event was: " << eventName(event) << ")\n";
	else
	    EV << "NAS-ESM: Staying in state: " << stateName(fsm->getState()) << " (event was: " << eventName(event) << ")\n";

	stateEntered();
}

void BearerContext::stateEntered() {
	switch(fsm->getState()) {
		case PROCEDURE_TRANSACTION_PENDING_U: {
			procId = uniform(1, 254);
			break;
		}
		default:
		    break;
	}
}

const char *BearerContext::stateName(int state) const {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (state) {
        CASE(BEARER_CONTEXT_INACTIVE_U);
        CASE(BEARER_CONTEXT_ACTIVE_U);
        CASE(PROCEDURE_TRANSACTION_INACTIVE_U);
        CASE(PROCEDURE_TRANSACTION_PENDING_U);
        CASE(BEARER_CONTEXT_INACTIVE_N);
        CASE(BEARER_CONTEXT_ACTIVE_PENDING_N);
        CASE(BEARER_CONTEXT_ACTIVE_N);
        CASE(BEARER_CONTEXT_INACTIVE_PENDING_N);
        CASE(BEARER_CONTEXT_MODIFY_PENDING_N);
        CASE(PROCEDURE_TRANSACTION_INACTIVE_N);
        CASE(PROCEDURE_TRANSACTION_PENDING_N);
    }
    return s;
#undef CASE
}

const char *BearerContext::eventName(int event) {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (event) {
        CASE(UEInitESMProcReq);
        CASE(ActBearerCtxtAcc);
        CASE(ActBearerCtxtReq);
    }
    return s;
#undef CASE
}

void BearerContext::setOwner(PDNConnection *ownerp) {
	this->ownerp = ownerp;
}

PDNConnection *BearerContext::getOwner() {
	return ownerp;
}

Subscriber *BearerContext::getSubscriber() {
	return ownerp->getOwner()->getOwner();
}

std::string BearerContext::info(int tabs) const {
    std::stringstream out;
    for (int i = 0; i < tabs; i++) out << "\t";
    out << "ebi:" << (unsigned)id << "\n";
    if (fsm != NULL) {
		for (int i = 0; i < tabs; i++) out << "\t";
		out << "st:" << stateName(fsm->getState()) << "\n";
    }
    if (enbTunn != NULL) {
    	for (int i = 0; i < tabs; i++) out << "\t";
    	out << "enbTunn:{ " << enbTunn->info(tabs + 1);
    	out << " }\n";
    }
    if (sgwTunn != NULL) {
    	for (int i = 0; i < tabs; i++) out << "\t";
    	out << "sgwTunn:{ " << sgwTunn->info(tabs + 1);
    	out << " }\n";
    }
    if (pgwTunn != NULL) {
    	for (int i = 0; i < tabs; i++) out << "\t";
    	out << "pgwTunn:{ " << pgwTunn->info(tabs + 1);
    	out << " }\n";
    }
    if (qci != 0) {
		for (int i = 0; i < tabs; i++) out << "\t";
		out << "qci:" << (unsigned)qci << "\n";
    }
    return out.str();
}

GTPInfoElem *BearerContext::createBearerContextIE(unsigned char instance) {
	std::vector<GTPInfoElem*>ies;
	GTPInfoElem *ebi = GTPUtils().createIE(GTP_V2, GTPV2_EBI, 0, id);
	ies.push_back(ebi);
	unsigned len = ebi->getValueArraySize() + GTPV2_IE_HEADER_SIZE;

	std::map<unsigned char, TunnelEndpoint*>::iterator it;
	for (it = tunnEnds.begin(); it != tunnEnds.end(); ++it) {
		TunnelEndpoint *te = (it->second);
		if (te != NULL) {
			GTPInfoElem *teid = (it->second)->createFteidIE(it->first);
			ies.push_back(teid);
			len += teid->getValueArraySize() + GTPV2_IE_HEADER_SIZE;
		}
	}
	return GTPUtils().createGroupedIE(GTPV2_BearerContext, instance, len, ies);
}

bool BearerContext::processBearerContextIE(GTPInfoElem *bearerContext) {
	unsigned manIECount = 1;

	std::vector<GTPInfoElem*> ies = GTPUtils().processGroupedIE(bearerContext);
	for (unsigned i = 0; i < ies.size(); i++) {
		GTPInfoElem *ie = ies.at(i);
		switch(ie->getType()) {
			case GTPV2_EBI:
				id = (*((char*)GTPUtils().processIE(ie)));
				manIECount--;
				break;
			case GTPV2_F_TEID: {
				TunnelEndpoint *te = GTPUtils().processFteidIE(ie);
				tunnEnds[ie->getInstance()] = te;
				break;
			}
			default:
			    break;
		}
	}
	if (manIECount)
		return false;
	else
		return true;
}

void BearerContext::setNasPdu(NasPdu *pdu) {
	this->pdu = pdu;
}

NasPdu *BearerContext::getNasPdu() {
	return pdu;
}
