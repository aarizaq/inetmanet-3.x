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

#ifndef DIAMETERPEER_H_
#define DIAMETERPEER_H_

#include "DiameterMessage.h"
#include "DiameterConnection.h"
#include "DiameterApplication.h"

#define TC_TIMER_TIMEOUT	1
#define TW_TIMER_TIMEOUT	10
#define TS_TIMER_TIMEOUT	10
#define TE_TIMER_TIMEOUT	3

#define EPHEMERAL_PORT_MIN	32768
#define EPHEMERAL_PORT_MAX	61000

#define STATIC	0
#define DYNAMIC	1

enum PeerState {
	CLOSED 					= FSM_Steady(0),
	WAIT_CONN_ACK 			= FSM_Steady(1),
	WAIT_I_CEA 				= FSM_Steady(2),
	WAIT_CONN_ACK_ELECT 	= FSM_Steady(3),
	WAIT_RETURNS 			= FSM_Steady(4),
	R_OPEN 					= FSM_Steady(5),
	I_OPEN 					= FSM_Steady(6),
	CLOSING 				= FSM_Steady(7)
};

enum PeerEvent {
    START,
    R_CONN_CER,
    I_RCV_CONN_ACK,
    I_RCV_CONN_NACK,
    TIMEOUT,
    R_RCV_CER,
    I_RCV_CER,
    I_RCV_CEA,
    R_RCV_CEA,
    I_RCV_NON_CEA,
    I_PEER_DISC,
    R_PEER_DISC,
    R_RCV_DPR,
    I_RCV_DPR,
    R_RCV_DPA,
    I_RCV_DPA,
    WIN_ELECTION,
    SEND_MESSAGE,
    R_RCV_MESSAGE,
    I_RCV_MESSAGE,
    R_RCV_DWR,
    I_RCV_DWR,
    I_RCV_DWA,
    R_RCV_DWA,
    STOP
};

/*
 * Class for Diameter peer. A Diameter peer can have a initiating or a responding
 * Diameter connection which is basically just the SCTP socket. It can have both for
 * a limited period of time during the election process. All the message processing
 * (base protocol messages and application messages) is done during the state machine
 * changes, described in the specification.
 * For more info contact RFC 3588.
 */
class DiameterPeer : public cPolymorphic {
private:
	bool type;                      // dummy
	bool isTLSEnabled;              // dummy
	DiameterBase *module;
	cFSM fsm;
	DiameterMessage *msg;           // for inter state message handling

    cMessage *tcTimer;  // connection initiation timer
    cMessage *twTimer;  // watchdog
    cMessage *tsTimer;  // test timer
    cMessage *teTimer;  // connection expire timer

    /*
     * Methods for base protocol message processing. CER and CEA are for connection
     * setup, DWR and DWA are the watchdog messages, DPR and DPA are termination
     * messages.
     */
    void sendCER(DiameterConnection *conn);
    unsigned processCER(DiameterMessage *msg);
    void sendCEA(DiameterConnection *conn, DiameterMessage *cer, unsigned resCode);
    unsigned processCEA(DiameterMessage *msg);
    void sendDWR(DiameterConnection *conn);
    unsigned processDWR(DiameterMessage *msg);
    void sendDWA(DiameterConnection *conn, DiameterMessage *dwr, unsigned resCode);
    unsigned processDWA(DiameterMessage *msg);
    void sendDPR(DiameterConnection *conn);
    void sendDPA(DiameterConnection *conn, DiameterMessage *dpr);

    /*
     * Methods for cleaning up the Diameter connections.
     */
    void iDisconnect();
    void rDisconnect();
    void cleanup();
    void error() { cleanup(); }

    /*
     * Method for electing the Diameter connection which will be used by the peer.
     * Election is done by comparing destination FQDN with origin FQDN and is
     * described in more detail in the standard.
     */
    void elect();
public:
	std::string dFQDN;
	std::string oFQDN;
	std::string dRealm;
	std::string oRealm;
	DiameterApplication *appl;
	DiameterConnection *iConn;
	DiameterConnection *rConn;

	DiameterPeer(DiameterBase *module);
	virtual ~DiameterPeer();

	/*
	 * Getter methods.
	 */
 	bool getType() { return type; }
 	const char *getTypeName();
 	int getConnectionType();
 	int getConnectionId();
 	int getState() { return fsm.getState(); }

 	/*
 	 * Setter methods.
 	 */
 	void setType(bool type) { this->type = type; }

 	/*
 	 * Utility methods for timer.
 	 */
 	void startTimer(cMessage *&timer, const char *name, simtime_t timeout);
 	void processTimer(cMessage *timer);

 	/*
 	 * Utility methods for state processing and printing.
 	 */
 	void performStateTransition(PeerEvent &event, DiameterMessage *msg);
 	void stateEntered();
    const char *stateName(int state);
    const char *eventName(int event);

    /*
     * Methods for processing the application messages. The application messages
     * are forwarded and received from the appropriate session.
     */
 	void sendApplMessage(DiameterConnection *conn, DiameterMessage *msg);
 	void processApplMessage(DiameterConnection *conn, DiameterMessage *msg);
};

/*
 * Class for Diameter peer table. This table will hold all the peers owned by the
 * Diameter base protocol model implementation.
 */
class DiameterPeerTable {
private:
    typedef std::vector<DiameterPeer*> DiameterPeers;
    DiameterPeers peers;
public:
    DiameterPeerTable();
    virtual ~DiameterPeerTable();

    /*
     * Method for finding a Diameter peer for a given fqdn and realm string. The
     * method returns the peer, if it is found, or NULL otherwise.
     */
    DiameterPeer *findPeer(std::string dFQDN, std::string dRealm = "");

    /*
     * Method for finding a Diameter peer for a given application id. The method
     * returns the peer, if it is found, or NULL otherwise.
     */
    DiameterPeer *findPeer(unsigned applId);

    /*
     * Method for deleting a Diameter peer. The method calls first the destructor
     * for each peer between start and end position and removes them afterwards.
     */
    void erase(unsigned start, unsigned end);

    /*
     * Method for printing the peer table.
     */
    void print();

    /*
     * Wrapper methods.
     */
    void push_back(DiameterPeer *peer) { peers.push_back(peer); }
    unsigned size() { return peers.size(); }
    DiameterPeer *at(unsigned i) { return peers[i]; }
};

#endif /* DIAMETERPEER_H_ */
