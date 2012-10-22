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

#include "DiameterBase.h"
#include "IPvXAddress.h"
#include "IPvXAddressResolver.h"
#include "DiameterApplication.h"
#include <algorithm>

Define_Module(DiameterBase);

DiameterBase::DiameterBase() {
	// TODO Auto-generated constructor stub
	outboundStreams = 10;
}

DiameterBase::~DiameterBase() {
	// TODO Auto-generated destructor stub
	peers.erase(0, peers.size());
    sessions.erase(0, sessions.size());
}

void DiameterBase::initialize(int stage) {
	if (stage == 4) {
		rT = RoutingTableAccess().get();
		const char *fileName = par("configFile");
		if (fileName == NULL || (!strcmp(fileName, "")))
			error("DiameterBase: Error reading configuration from file %s", fileName);
		this->loadConfigFromXML(fileName);
	}
}

void DiameterBase::handleMessage(cMessage *msg) {

    if (msg->isSelfMessage()) { // handles timers
        DiameterPeer *peer = (DiameterPeer*)msg->getContextPointer();
        peer->processTimer(msg);
    } else {    // handles SCTP messages
        DiameterConnection *conn = conns.findConnection(msg);
        if (!conn) {
            // new connection -- create new socket object and server process
        	SCTPSocket *socket = new SCTPSocket(msg);
        	socket->setOutputGate(gate("sctpOut"));
            conn = new DiameterConnection(this);
            conn->setType(RESPONDER);
            conn->setSocket(socket);
            conns.insert(conn);
        	socket->setCallbackObject(conn);
        	socket->setOutboundStreams(outboundStreams);

            //updateDisplay();
        }
        conn->processMessage(msg);
    }
}

void DiameterBase::loadOwnConfigFromXML(const cXMLElement& diameterNode) {
	fqdn = diameterNode.getAttribute("fqdn");
	if (fqdn.empty())
		error("DiameterBase: Own peer has no fqdn attribute");
	realm = diameterNode.getAttribute("realm");
	if (realm.empty())
		error("DiameterBase: Own peer has no realm attribute");
	if (!diameterNode.getAttribute("vendor"))
		error("DiameterBase: Own peer has no vendor attribute");
	vendorId = atoi(diameterNode.getAttribute("vendor"));
	productName = diameterNode.getAttribute("product");
	if (productName.empty())
		error("DiameterBase: Own peer has no product attribute");
}

void DiameterBase::loadListenersFromXML(const cXMLElement& diameterNode) {

	int port;
	AddressVector addresses;

	cXMLElement* listenersNode = diameterNode.getElementByPath("Listeners");
	if (listenersNode == NULL)
		error("DiameterBase: No configuration for Diameter listeners");

	if (!listenersNode->getAttribute("port"))
		error("Diameter: Listeners has no port attribute");
	port = atoi(listenersNode->getAttribute("port"));

    cXMLElementList listenersList = listenersNode->getChildren();
    for (cXMLElementList::iterator listenersIt = listenersList.begin(); listenersIt != listenersList.end(); listenersIt++) {

    	std::string elementName = (*listenersIt)->getTagName();
        if ((elementName == "Listener")) {
        	const char *address;

        	address = (*listenersIt)->getAttribute("address");
        	if (!address)
        		error("DiameterBase: Listener has no address attribute");
        	addresses.push_back(IPvXAddress(address));
//        	err = false;
        }
    }

    //if (err)
    //	error("Diameter: Not even one Diameter listener found");

    if (addresses.size() > 0) {
		serverSocket = new SCTPSocket();
		serverSocket.setOutputGate(gate("sctpOut"));
		serverSocket.setOutboundStreams(outboundStreams);
		serverSocket.bindx(addresses, port);
		serverSocket.listen(true);
    }
}

void DiameterBase::loadPeersFromXML(const cXMLElement& diameterNode) {
	cXMLElement* peersNode = diameterNode.getElementByPath("Peers");
	if (peersNode != NULL) {
	    cXMLElementList peersList = peersNode->getChildren();
	    for (cXMLElementList::iterator peersIt = peersList.begin(); peersIt != peersList.end(); peersIt++) {

	    	std::string elementName = (*peersIt)->getTagName();
	        if ((elementName == "Peer")) {
	        	const char *fqdn;
	        	AddressVector addresses;
	        	const char *realm;
	        	int port;

	        	fqdn = (*peersIt)->getAttribute("fqdn");
	        	if (!fqdn)
	        		error("DiameterBase: Peer has no fqdn attribute");
	        	realm = (*peersIt)->getAttribute("realm");
	        	if (!realm)
	        		error("DiameterBase: Peer has no realm attribute");
	        	if (!(*peersIt)->getAttribute("port"))
	        		error("DiameterBase: Peer has no port attribute");
	        	port = atoi((*peersIt)->getAttribute("port"));

	        	addresses = queryDNS(diameterNode, fqdn, port);
	        	if (addresses.empty())
	        		error("DiameterBase: Peer FQDN not found in DNS");

	        	cXMLElement* applNode = (*peersIt)->getElementByPath("Appl");
	        	DiameterApplication *appl = NULL;
	        	if (applNode) {
	        		if (!applNode->getAttribute("id"))
	        			error("DiameterBase: Appl has no id attribute");
	        		unsigned applId = atoi(applNode->getAttribute("id"));
	        		if (!applNode->getAttribute("vendor"))
	        			error("DiameterBase: Appl has no vendor attribute");
	        		unsigned vendorId = atoi(applNode->getAttribute("vendor"));
	        		appl = new DiameterApplication(applId, vendorId);
	        	}

	        	DiameterConnection *conn = createConnection(addresses, port);
	        	DiameterPeer *peer = createPeer(fqdn, realm, conn, appl);
	        	peer->setType(STATIC);
	        	conn->setPeer(peer);
	        }
	    }
	}
}

DiameterPeer *DiameterBase::createPeer(std::string dFQDN, std::string dRealm, DiameterConnection *conn, DiameterApplication *appl) {
	DiameterPeer *peer = new DiameterPeer(this);
	peer->dFQDN = dFQDN;
	peer->dRealm = dRealm;
	peer->appl = appl;
	conn->getType() == INITIATOR ? peer->iConn = conn : peer->rConn = conn;
	peers.push_back(peer);
	return peer;
}

DiameterConnection *DiameterBase::createConnection(AddressVector addresses, int port) {
	SCTPSocket *socket = new SCTPSocket();
	DiameterConnection *conn = new DiameterConnection(this);

	AddressVector localAddrs = findLocalAddresses(addresses);
   	socket->bindx(localAddrs, EPHEMERAL_PORT_MIN + uniform(EPHEMERAL_PORT_MIN, EPHEMERAL_PORT_MAX));
   	socket->setOutputGate(gate("sctpOut"));
   	socket->setCallbackObject(conn);
   	socket->setOutboundStreams(outboundStreams);
   	conn->setSocket(socket);
   	conn->setAddresses(addresses);
   	conn->setPort(port);
   	conn->setType(INITIATOR);
   	conns.insert(conn);

   	return conn;
}

DiameterSession *DiameterBase::createSession(bool stType) {
	DiameterSession *session = new DiameterSession(this, stType);
	sessions.push_back(session);
	return session;
}

AddressVector DiameterBase::findLocalAddresses(AddressVector remoteAddrs) {
	AddressVector localAddrs;
	for (unsigned i = 0; i < remoteAddrs.size(); i++) {
		IPv4Address addr = (remoteAddrs.at(i)).get4();
		const IPv4Route *route = rT->findBestMatchingRoute(addr);
		if (route == NULL)
			error("DiameterBase: No route for remote address");
		localAddrs.push_back(IPvXAddress(route->getGateway()));
	}
	return localAddrs;
}

AddressVector DiameterBase::queryDNS(const cXMLElement& diameterNode, const std::string fqdnIn, int port) {
	AddressVector addresses;
	cXMLElement* config = diameterNode.getParentNode();

    cXMLElement* dnsNode = config->getElementByPath("DNS");
	if (dnsNode != NULL) {
	    cXMLElementList entriesList = dnsNode->getChildren();
	    for (cXMLElementList::iterator entriesIt = entriesList.begin(); entriesIt != entriesList.end(); entriesIt++) {

	    	std::string elementName = (*entriesIt)->getTagName();
	        if ((elementName == "Entry")) {
	        	const char *fqdn;
	        	IPvXAddress address;

	        	fqdn = (*entriesIt)->getAttribute("fqdn");
	        	if (!fqdn)
	        		error("Diameter: DNS Entry no fqdn attribute");
	        	if (!fqdnIn.compare(fqdn)) {
					if (!(*entriesIt)->getAttribute("address"))
						error("Diameter: DNS Entry has no address attribute");
					address = IPvXAddress((*entriesIt)->getAttribute("address"));
					addresses.push_back(address);
	        	}
	        }
	    }
	}
	return addresses;
}

void DiameterBase::loadConfigFromXML(const char *filename) {
    cXMLElement* config = ev.getXMLDocument(filename);
    if (config == NULL)
        error("DiameterBase: Cannot read configuration from file: %s", filename);

    cXMLElement* diameterNode = config->getElementByPath("Diameter");
	if (diameterNode == NULL)
		error("DiameterBase: No configuration for Diameter base application");

	loadOwnConfigFromXML(*diameterNode);
	loadListenersFromXML(*diameterNode);
	loadPeersFromXML(*diameterNode);
}
