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

#ifndef DIAMETERCONNECTION_H_
#define DIAMETERCONNECTION_H_

#include <omnetpp.h>
#include "SCTPSocket.h"
#include "INETDefs.h"
#include "DiameterMessage.h"

#define INITIATOR	0
#define RESPONDER	1

class DiameterPeer;
class DiameterBase;

/*
 * Class for Diameter connection. The class is basically just a SCTP socket, that
 * sends and receives messages for a Diameter peer (owner).
 */
class DiameterConnection : public SCTPSocket::CallbackInterface {
private:
	bool type;	// initiator or responder
	bool ignore;    // used because socketClosed is called twice
	DiameterBase *module;
	DiameterPeer *peer;

	AddressVector addresses;

    SCTPSocket socket;
    bool ordered;

	int port;

	/*
	 * Callback methods for SCTP socket. Message handling is done in the data
	 * arrived method.
	 */
	void socketEstablished(int32 connId, void *yourPtr, uint64 buffer);
	void socketDataArrived(int32 connId, void *yourPtr, cPacket *msg, bool urgent);
	void socketDataNotificationArrived(int32 connId, void *yourPtr, cPacket *msg);
	void socketPeerClosed(int32 connId, void *yourPtr);
	void socketClosed(int32 connId, void *yourPtr);
	void socketFailure(int32 connId, void *yourPtr, int32 code);
	void socketStatusArrived(int32 connId, void *yourPtr, SCTPStatusInfo *status) { delete status; }

public:
	DiameterConnection(DiameterBase *module);
	virtual ~DiameterConnection();

	void setOrdered (bool v) {ordered = v;}
	bool getOrdered () {return ordered;}
	/*
	 * Getter methods.
	 */
	bool getType() { return type; }
	int getConnectionId() { return socket.getConnectionId(); }

	/*
	 * Setter methods.
	 */
    void setPeer(DiameterPeer *peer) { this->peer = peer; }
    void setType(bool type) { this->type = type; }
    void setSocket(SCTPSocket *socket) { this->socket = socket; }
    void setAddresses(AddressVector addresses) { this->addresses = addresses; }
    void setPort(int port) { this->port = port; }

    /*
     * Method for processing the origin of each message. It returns a Diameter result
     * code, resulted from the origin processing.
     */
	unsigned processOrigin(DiameterPeer *&peer, DiameterMessage *msg);

	/*
	 * Utility methods for the socket.
	 */
    void shutdown();
    void close();
    void connect();

    /*
     * Method for sending Diameter messages. It will be used for all messages coming
     * from the peer and it will add information regarding source identification.
     */
	void send(DiameterMessage *msg, unsigned fqdnPos, unsigned realmPos);

	/*
	 * Wrapper methods.
	 */
	void processMessage(cMessage *msg) { socket.processMessage(PK(msg)); }
};

/*
 * Class for Diameter connection map. This class will hold all the connection for
 * the Diameter base protocol model.
 */
class DiameterConnectionMap {
private:
    typedef std::map<int,DiameterConnection*> DiameterConnections;
    DiameterConnections conns;
public:
    DiameterConnectionMap();
    virtual ~DiameterConnectionMap();

    /*
     * Method for finding a Diameter connection with a specific message based on the
     * association id specified in the control info. The method returns the connection
     * if it is found, or NULL otherwise.
     */
    DiameterConnection *findConnection(cMessage *msg);

    /*
     * Method for inserting a new connection in the map.
     */
    void insert(DiameterConnection *conn);

    /*
     * Method for deleting a Diameter connection. The method calls first the
     * destructor for the connection and removes it afterwards.
     */
    void erase(DiameterConnection *conn);

    /*
     * Wrapper method.
     */
    unsigned int size() {return conns.size();}

    /*
     * Method for printing the connection map.
     */
    void print();
};

#endif /* DIAMETERCONNECTION_H_ */
