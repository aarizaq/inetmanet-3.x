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

#include "GTPControl.h"
#include "SubscriberTableAccess.h"
#include "GatewayTableAccess.h"
#include "GTPUtils.h"

Define_Module(GTPControl);

GTPControl::GTPControl() {
	// TODO Auto-generated constructor stub
	localCounter = 1;
	tunnIds = 5 + uniform(0, 20);
	plane = GTP_CONTROL;
}

GTPControl::~GTPControl() {
	// TODO Auto-generated destructor stub
}

void GTPControl::initialize(int stage) {
	GTP::initialize(stage);
	nb->subscribe(this, NF_SUB_NEEDS_PDN);
	nb->subscribe(this, NF_SUB_MODIF_TUNN);
	subT = SubscriberTableAccess().get();
}

void GTPControl::processMessage(GTPMessage *msg, GTPPath *path) {
	TunnelEndpoint *te = teT->findTunnelEndpoint(msg->getHeader()->getTeid(), path);
	if (te == NULL) {
		te = new TunnelEndpoint();
		te->setModule(this);
		GTPInfoElem *senderIe = msg->findIe(GTPV2_F_TEID, 0);
		if (senderIe == NULL) {
			TunnelEndpoint(path).sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Man_IE_Missing, 0, 0, 1, GTPV2_F_TEID, 0));
			return;
		}
		if (!te->processFteidIE(senderIe)) {
			TunnelEndpoint(path).sendCreateSessionResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Man_IE_Incorrect, 0, 0, 1, GTPV2_F_TEID, 0));
			return;
		}
		teT->push_back(te);

	}
	te->processTunnelMessage(msg);
}

void GTPControl::receiveChangeNotification(int category, const cPolymorphic *details) {
	Enter_Method_Silent();
	if (category == NF_SUB_NEEDS_PDN) {
		EV << "GTPControl: Received NF_SUB_NEEDS_PDN notification. Processing notification.\n";
		PDNConnection *conn = check_and_cast<PDNConnection*>(details);
		Subscriber *sub = conn->getOwner()->getOwner();
		/* TODO */
		/* SGW selection */
		gT = GatewayTableAccess().get();
		Gateway *gw = gT->findGateway(sub->getTac());
		if (gw == NULL) {
			/* TODO */
			// signal back error
			return;
		}
		GTPPath *path = pT->at(gw->getPathId());
		TunnelEndpoint *te = sub->getS11TunnEnd();
		if (te == NULL) {
			te = new TunnelEndpoint(path);
			teT->push_back(te);
			sub->setS11TunnEnd(te);
		}

		if (sub->getEmmEntity()->getState() == EMM_DEREGISTERED_N) {
			if (sub->getGTPProcedure() == NoProcedure)
				sub->setGTPProcedure(EUTRANInitAttachReq);
			te->sendCreateSessionRequest();
		}
	} else if (category == NF_SUB_MODIF_TUNN) {
		EV << "GTPControl: Received NF_MODIF_TUNN notification. Processing notification.\n";
		BearerContext *bearer = check_and_cast<BearerContext*>(details);
		Subscriber *sub = bearer->getSubscriber();
		TunnelEndpoint *s11Tunn = sub->getS11TunnEnd();
		if (sub->getEmmEntity()->getState() != EMM_REGISTERED_N) {
			s11Tunn->sendModifyBearerRequest();
		}
	}
}
