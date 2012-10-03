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

#ifndef S1APCONNECTION_H_
#define S1APCONNECTION_H_

#include <omnetpp.h>
#include "SCTPSocket.h"
#include "S1APPdu.h"
#include "Subscriber.h"

#define EPHEMERAL_PORT_MIN	32768
#define EPHEMERAL_PORT_MAX	61000

enum S1APConnectionState {
	S1AP_CONNECTED,
	S1AP_PENDING,
	S1AP_DISCONNECTED
};

enum S1APConnectionEvent {
	SCTPEstablished,
	S1APRcvSetupRequest,
	S1APRcvSetupResponse,
	S1APRcvSetupFailure,
};

class S1AP;

/*
 * Class for S1AP connection. A S1AP connection is built around a
 * SCTP socket. Besides the socket, the connection will hold specific
 * information for S1AP protocol, PLMN id, Cell Id etc.
 * S1AP connection implements also a state machine which is not defined
 * in the specification and it helps setting up a connection between the
 * entities.
 *
 *                +-------------------+
 *                | S1AP_DISCONNECTED |-----------+
 *                +-------------------+           |
 *                     ^        |                 |
 *                     |        |                 |
 * S1APRcvSetupFailure |        | SCTPEstablished |
 * /InvalidResponse    |        |                 |
 *                     |        |                 |
 *                     |        v                 |
 *                  +--------------+              | S1APRcvSetupRequest
 *                  | S1AP_PENDING |              | /CorrectRequest
 *                  +--------------+              |
 *                             |                  |
 *                             |                  |
 *        S1APRcvSetupResponse |                  |
 *        /CorrectResponse     |                  |
 *                             |                  |
 *                             |                  |
 *                             v                  |
 *                 +----------------+             |
 *                 | S1AP_CONNECTED |<------------+
 *                 +----------------+
 */
class S1APConnection : public SCTPSocket::CallbackInterface {
private:
	char *plmnId;
	char *cellId;
	std::vector<SupportedTaItem> suppTas;
	std::vector<ServedGummeiItem> servGummeis;

	cFSM *fsm;

	S1AP *module;

	SCTPSocket *socket;
	AddressVector addresses;
	int port;

    /*
     * Callback methods for SCTP socket. Message handling is done in the
     * data arrived method.
     */
	void socketEstablished(int32 connId, void *yourPtr, uint64 buffer);
	void socketDataArrived(int32 connId, void *yourPtr, cPacket *msg, bool urgent);
	void socketDataNotificationArrived(int32 connId, void *yourPtr, cPacket *msg);
	void socketPeerClosed(int32 connId, void *yourPtr);
	void socketClosed(int32 connId, void *yourPtr);
	void socketFailure(int32 connId, void *yourPtr, int32 code);
	void socketStatusArrived(int32 connId, void *yourPtr, SCTPStatusInfo *status) { delete status; }

	/*
	 * Methods for sending and processing S1AP connection setup messages.
	 * These methods will be used internally between two S1AP connections.
	 */
	void sendS1SetupRequest();
	ProtocolIeField *processS1SetupRequest(OpenType *val);
	void sendS1SetupResponse();
	ProtocolIeField *processS1SetupResponse(OpenType *val);
	void sendS1SetupFailure(ProtocolIeField *cause);

public:
	S1APConnection(S1AP *module);
	virtual ~S1APConnection();

	/*
	 * Method for sending S1AP messages. It creates and sends a SCTPSimpleMessage
	 * after encoding S1APPdu according to rules described in the ASN.1
	 * implementation and S1AP messages description.
	 */
	void send(S1APPdu *msg);

    /*
     * Methods for sending and processing S1AP messages that are not
     * related to S1AP connection management.
     */
    void sendInitialUeMessage(Subscriber *sub, NasPdu *nasPdu);
    ProtocolIeField *processInitialUeMessage(OpenType *val);
    void sendInitialContextSetupRequest(Subscriber *sub, NasPdu *nasPdu);
    ProtocolIeField *processInitialContextSetupRequest(OpenType *va);
    void sendInitialContextSetupResponse(Subscriber *sub);
    ProtocolIeField *processInitialContextSetupResponse(OpenType *val);
    void sendDownlinkNasTransport(Subscriber *sub, NasPdu *pdu);
    ProtocolIeField *processDownlinkNasTransport(OpenType *val);
    void sendUplinkNasTransport(Subscriber *sub, NasPdu *nasPdu);
    ProtocolIeField *processUplinkNasTransport(OpenType *val);

	/*
	 * Utility methods for state processing and printing.
	 */
    const char *stateName(int state);
    const char *eventName(int event);
	void performStateTransition(S1APConnectionEvent &event, OpenType *val = NULL);

	/*
	 * Setter methods.
	 */
    void setPlmnId(char *plmnId) { this->plmnId = plmnId; }
    void setCellId(char *cellId) { this->cellId = cellId; }
    void setSupportedTas(std::vector<SupportedTaItem> suppTas) { this->suppTas = suppTas; }
    void setServedGummeis(std::vector<ServedGummeiItem> servGummeis) { this->servGummeis =servGummeis; }
    void setSocket(SCTPSocket *socket) { this->socket = socket; }
    void setAddresses(AddressVector addresses) { this->addresses = addresses; }
    void setPort(int port) { this->port = port; }

    /*
     * Getter methods.
     */
    int getConnectionId() { return socket->getConnectionId(); }
    int getState() { return fsm->getState(); }
    char *getCellId() { return cellId; }
    char *getPlmnId() { return plmnId; }
    std::vector<SupportedTaItem> getSupportedTas() { return suppTas; }

    /*
     * Wrapper methods.
     */
    void connect() { socket->connectx(addresses, port); }
    void processMessage(cMessage *msg) { socket->processMessage(PK(msg)); }

};

/*
 * Class for S1AP connection table. This table will hold all the connections
 * owned by the S1AP protocol model implementation.
 */
class S1APConnectionTable {
private:
    typedef std::vector<S1APConnection*> S1APConnections;
    S1APConnections conns;
public:
    S1APConnectionTable();
    virtual ~S1APConnectionTable();

    /*
     * Method for finding a S1AP connection for a given connection id.
     * The method returns the connection, if it is found, or NULL otherwise.
     */
    S1APConnection *findConnectionForId(int connId);

    /*
     * Method for finding a S1AP connection for a given cell id.
     * The method returns the connection, if it is found, or NULL otherwise.
     */
    S1APConnection *findConnectionForCellId(char *cellId);

    /*
     * Method for finding a S1AP connection for a given state.
     * The method returns the connection, if it is found, or NULL otherwise.
     */
    S1APConnection *findConnectionForState(int state);

    /*
     * Method for deleting a S1AP connection. The method calls first
     * the destructor for the connection and removes it afterwards.
     */
    void erase(unsigned start, unsigned end);

    /*
     * Wrapper methods.
     */
    unsigned int size() {return conns.size();}
    void push_back(S1APConnection *conn) { conns.push_back(conn); }
    S1APConnection *at(unsigned i) { return conns.at(i); }

};

#endif /* S1APCONNECTION_H_ */
