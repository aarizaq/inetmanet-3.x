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

#ifndef DIAMETERBASE_H_
#define DIAMETERBASE_H_

#include <omnetpp.h>
#include <string>
#include "SCTPSocket.h"
#include "DiameterConnection.h"
#include "DiameterSession.h"
#include "DiameterPeer.h"
#include "RoutingTableAccess.h"

/*
 * Module for Diameter base protocol. This section describes the internal architecture
 * of the Diameter model.
 *
 * The Diameter base protocol implementation is composed of several classes
 * (discussion follows below):
 *   - DiameterBase: the module class
 *   - DiameterConnection: manages a connection between two peers
 *   - DiameterPeer: manages a Diameter peer
 *   - DiameterSession: manages a session between end to end peers
 *   - DiameterApplication: manages a Diameter application
 *
 * Diameter base class holds the server socket from which each Diameter connection
 * is forked. Also it manages the connection table, peer table and session table.
 *
 * Diameter base protocol model is implemented according to RFC 3588.
 */
class DiameterBase : public cSimpleModule {
protected:
    std::string fqdn;
    std::string realm;
    unsigned vendorId;
    std::string productName;
    unsigned sessionIds;
	int outboundStreams;
    SCTPSocket serverSocket;
    DiameterConnectionMap conns;
    DiameterSessionTable sessions;
    DiameterPeerTable peers;
    IRoutingTable *rT;

    /*
     * Methods for loading the configuration from the XML file
     * ex.
     *   <DNS>
     *       <Entry fqdn="mme.lte.test" address="192.168.3.2"/>
     *   </DNS>
     *   <Diameter
     *           fqdn="hss.lte.test"
     *           realm="lte.test"
     *           vendor="0"
     *           product="HSSDiameter">
     *       <Listeners port="3868">
     *           <Listener address="192.168.3.1"/>
     *       </Listeners>
     *       <Peers>
     *       </Peers>
     *   </Diameter>
     */
    void loadOwnConfigFromXML(const cXMLElement & diameterNode);
    void loadPeersFromXML(const cXMLElement & diameterNode);
    void loadListenersFromXML(const cXMLElement & diameterNode);
    void loadConfigFromXML(const char *filename);
    AddressVector queryDNS(const cXMLElement& diameterNode, const std::string fqdnIn, int port);
    AddressVector findLocalAddresses(AddressVector remoteAddrs);

public:
    DiameterBase();
    virtual ~DiameterBase();

    virtual int numInitStages() const  { return 5; }

    /*
     * Method for reading and processing the configuration file. This includes
     * loading the Diameter peers, creating for each a initiating or responding
     * Diameter connection and loading the Diameter application.
     */
    virtual void initialize(int stage);

    /*
     * Method for handling incoming messages. If the message is a timer, it sends it
     * to the appropriate peer. If the message is a SCTP message, it finds the
     * appropriate socket and sends the message to it. If no socket is available,
     * it creates a new socket together with a new Diameter connection.
     */
    virtual void handleMessage(cMessage *msg);

    /*
     * Method for processing application messages. This method should be overridden
     * by Diameter application implementations.
     */
    virtual DiameterMessage *processMessage(DiameterMessage *msg) { return NULL; }

    /*
     * Getter methods.
     */
    int getOutboundStreams() { return outboundStreams; }
	SCTPSocket getServerSocket() { return serverSocket; }
	std::string getFqdn() { return fqdn; }
	std::string getRealm() { return realm; }
	unsigned getVendorId() { return vendorId; }
	std::string getProductName() { return productName; }

	/*
	 * Utility methods.
	 */
    DiameterConnection *createConnection(AddressVector addresses, int port);
	DiameterPeer *createPeer(std::string dFQDN, std::string dRealm, DiameterConnection *conn, DiameterApplication *appl = NULL);
	DiameterSession *createSession(bool stType);
	unsigned genSessionId() { return ++sessionIds; }

	/*
	 * Wrapper methods.
	 */
    DiameterPeer *findPeer(std::string dFQDN, std::string dRealm = "") { return peers.findPeer(dFQDN, dRealm); }
    DiameterPeer *findPeer(unsigned applId) { return peers.findPeer(applId); }
    void removeConnection(DiameterConnection *conn) { conns.erase(conn); }

};

#endif /* DIAMETERBASE_H_ */
