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

#ifndef DIAMETERSESSION_H_
#define DIAMETERSESSION_H_

#include <omnetpp.h>
#include "DiameterMessage.h"
#include "DiameterPeer.h"

class DiameterBase;

//#define STATELESS	0
//#define STATEFUL	1

enum SessionState {
	IDLE 					= FSM_Steady(0),
	PENDING 				= FSM_Steady(1),
	OPEN	 				= FSM_Steady(2),
	DISCON				 	= FSM_Steady(3),
	NOT_DISCON				= FSM_Steady(4),
	ANY						= FSM_Steady(5)
};

enum SessionEvent {
	RequestsAccess,
	RequestReceived,
};

/*
 * Class for Diameter session. The class implements the session state machine and
 * processes Diameter application messages which are not meant for Diameter base
 * protocol.
 */
class DiameterSession : public cPolymorphic {
private:
	bool type;  // stateful or stateless
	std::string id;
	cFSM *fsm;
//	cMessage *sTimer;
	DiameterBase *module;

	/*
	 * Methods for sending and processing Diameter messages. Message handling will
	 * not be done directly, it will be done during a state transition.
	 */
	void sendDiameterMessage(unsigned applId, DiameterPeer *peer, DiameterMessage *req);
	void processDiameterMessage(unsigned applId, DiameterPeer *peer, DiameterMessage *msg);
public:
	DiameterSession(DiameterBase *module, bool state);
	virtual ~DiameterSession();

	/*
	 * Getter methods.
	 */
	std::string getId() { return id; }

	/*
	 * Methods for managing the session state machine
	 */
	void performStateTransition(SessionEvent &event, unsigned applId, DiameterPeer *peer, DiameterMessage *msg);
    const char *stateName(int state);
    const char *eventName(int event);
};

/*
 * Class for Diameter session table. This table will hold all the sessions owned by
 * the Diameter base protocol model implementation.
 */
class DiameterSessionTable {
private:
    typedef std::vector<DiameterSession*> DiameterSessions;
    DiameterSessions sessions;
public:
    DiameterSessionTable();
    virtual ~DiameterSessionTable();

    /*
     * Method for finding a Diameter session with a specific id. The method returns
     * the session if it is found, or NULL otherwise.
     */
    DiameterSession *findSession(std::string id);

    /*
     * Method for deleting S1AP connections. The method calls first the destructor
     * for each connection between start and end position and removes them afterwards.
     */
    void erase(unsigned start, unsigned end);

    /*
     * Wrapper methods.
     */
    unsigned size() {return sessions.size();}
    DiameterSession *at(unsigned i) { return sessions.at(i); }
    void push_back(DiameterSession *session) { sessions.push_back(session); }
//  void deleteSessions();
};

#endif /* DIAMETERSESSION_H_ */
