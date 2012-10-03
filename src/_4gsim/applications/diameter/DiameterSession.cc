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

#include "DiameterSession.h"
#include "DiameterBase.h"
#include "DiameterUtils.h"

DiameterSession::DiameterSession(DiameterBase *module, bool state) {
	// TODO Auto-generated constructor stub
	this->type = type;
	this->module = module;

	char time[10];
	sprintf(time, "%d", (unsigned)simTime().raw());
	char id[10];
	sprintf(id, "%d", module->genSessionId());
	this->id = module->getFqdn() + ";" + time + ";" + id;

	fsm = new cFSM();
	fsm->setName("sessionState");
	fsm->setState(IDLE);
}

DiameterSession::~DiameterSession() {
	// TODO Auto-generated destructor stub
	delete fsm;
}

void DiameterSession::performStateTransition(SessionEvent &event, unsigned applId, DiameterPeer *peer, DiameterMessage *msg) {
	int oldState = fsm->getState();

	switch(oldState) {
		case IDLE:
			switch(event) {
				case RequestsAccess: {
					DiameterPeer *peer = module->findPeer(applId);
					// here queue
					if (peer) {
						sendDiameterMessage(applId, peer, msg);
					}
					FSM_Goto(*fsm, PENDING);
					break;
				}
				case RequestReceived:
					processDiameterMessage(applId, peer, msg);
					// remains in this state
					break;
				default:
					EV << "DiameterSession: Received unexpected event\n";
					if (msg != NULL)
						delete msg;
					break;
			}
			break;
		default:
			EV << "DiameterSession: Unknown state\n";
			if (msg != NULL)
				delete msg;
			break;
	}

    if (oldState != fsm->getState())
        EV << "DiameterSession: PSM-Transition: " << stateName(oldState) << " --> " << stateName(fsm->getState()) << "  (event was: " << eventName(event) << ")\n";
    else
        EV << "DiameterSession: Staying in state: " << stateName(fsm->getState()) << " (event was: " << eventName(event) << ")\n";
}

void DiameterSession::sendDiameterMessage(unsigned applId, DiameterPeer *peer, DiameterMessage *msg) {
	msg->insertAvp(0, DiameterUtils().createUTF8StringAVP(AVP_SessionId, 0, 1, 0, 0, id));
	msg->insertAvp(1, DiameterUtils().createInteger32AVP(AVP_AuthSessionState, 0, 1, 0, 0, type ? NO_STATE_MAINTAINED : STATE_MAINTAINED));
	PeerEvent event = SEND_MESSAGE;
	peer->performStateTransition(event, msg);
}

void DiameterSession::processDiameterMessage(unsigned applId, DiameterPeer *peer, DiameterMessage *msg) {
	if (applId == S6a) {
		DiameterMessage *resp = module->processMessage(msg);
		if (resp != NULL)
			sendDiameterMessage(applId, peer, resp);
	}
}

const char *DiameterSession::stateName(int state) {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (state) {
        CASE(IDLE);
        CASE(PENDING);
        CASE(OPEN);
        CASE(DISCON);
        CASE(NOT_DISCON);
        CASE(ANY);
    }
    return s;
#undef CASE
}

const char *DiameterSession::eventName(int event) {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (event) {
        CASE(RequestsAccess);
        CASE(RequestReceived);
    }
    return s;
#undef CASE
}

DiameterSessionTable::DiameterSessionTable() {
    // TODO Auto-generated constructor stub

}

DiameterSessionTable::~DiameterSessionTable() {
    // TODO Auto-generated destructor stub
}

DiameterSession *DiameterSessionTable::findSession(std::string id) {
    for (unsigned i = 0; i < sessions.size(); i++) {
        if (sessions.at(i)->getId() == id)
            return sessions.at(i);
    }
    return NULL;
}

void DiameterSessionTable::erase(unsigned start, unsigned end) {
    DiameterSessions::iterator first = sessions.begin() + start;
    DiameterSessions::iterator last = sessions.begin() + end;
    DiameterSessions::iterator i = first;
    for (;i != last; ++i)
        delete *i;
    sessions.erase(first, last);
}

