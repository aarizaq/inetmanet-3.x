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

#include "S1AP.h"
#include "S1APPdu.h"
#include "LTEUtils.h"
#include "S1APControlInfo_m.h"
#include "RoutingTableAccess.h"
#include "PerEncoder.h"
#include "PerDecoder.h"
#include "SCTPCommand_m.h"

Define_Module(S1AP)

S1AP::S1AP() {
	// TODO Auto-generated constructor stub
	outboundStreams = 10;
	plmnId = NULL;
	cellId = NULL;
	name = "";
	pagDrx = v32;
	relMmeCapac = 5;
	sendQueue.setName("sendQueue");
	retryTimer = new cMessage("Retry-Timer");

}

S1AP::~S1AP() {
	// TODO Auto-generated destructor stub
	if (plmnId)
		delete plmnId;
	if (cellId)
		delete cellId;
	if (retryTimer != NULL) {
		if (retryTimer->getContextPointer() != NULL)
			cancelEvent(retryTimer);
		delete retryTimer;
	}
}

void S1AP::initialize(int stage) {
	if (stage == 4) {
		subT = SubscriberTableAccess().get();
		nb = NotificationBoardAccess().get();
		nb->subscribe(this, NF_SUB_TUNN_ACK);
		rT = RoutingTableAccess().get();
		const char *fileName = par("configFile");
		if (fileName == NULL || (!strcmp(fileName, "")))
			error("Error reading configuration from file %s", fileName);

		this->loadConfigFromXML(fileName);
		scheduleAt(simTime() + 2, retryTimer);
		retryTimer->setContextPointer(this);
	}
}

void S1AP::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
    	processTimer(msg);
    }
    else if (msg->arrivedOn("nasIn")) {
    	EV << "S1AP: Received message from upper layer. Inserting message in queue.\n";
    	sendQueue.insert(msg);
    }
    else { // must be from SCTP
    	handleLowerMessage(msg);
    }
}

void S1AP::handleLowerMessage(cMessage *msg) {
	EV << "S1AP: Received message from lower layer. Processing.\n";
    SCTPCommand *ind = dynamic_cast<SCTPCommand*>(msg->getControlInfo());
    S1APConnection *conn = conns.findConnectionForId(ind->getAssocId());
    if (!conn) {
        // new connection -- create new socket object and server process
    	SCTPSocket *socket = new SCTPSocket(msg);
    	socket->setOutputGate(gate("sctpOut"));
        conn = new S1APConnection(this);
        conn->setSocket(socket);
        conn->setServedGummeis(servGummeis);
        conns.push_back(conn);
    	socket->setCallbackObject(conn);
    	socket->setOutboundStreams(outboundStreams);

        //updateDisplay();
    }
    conn->processMessage(msg);
}

void S1AP::handleUpperMessage(cMessage *msg) {
	S1APControlInfo *ctrl = dynamic_cast<S1APControlInfo*>(msg->getControlInfo());
	Subscriber *sub = subT->findSubscriberForId(ctrl->getUeEnbId(), ctrl->getUeMmeId());

	if (sub != NULL) {
		S1APConnection *conn = conns.findConnectionForCellId(sub->getCellId());
		if (ctrl->getProcId() == id_initialUEMessage) {
			/* TODO */ // MME selection function + better queue
			conn = conns.findConnectionForState(S1AP_CONNECTED);
			if (conn == NULL) {
				sendQueue.insert(msg);
				return;
			} else {
				conn->sendInitialUeMessage(sub, new NasPdu(ctrl));
			}
		} else if (ctrl->getProcId() == id_InitialContextSetup) {
			if (conn != NULL) {
				conn->sendInitialContextSetupRequest(sub, new NasPdu(ctrl));
			}
		} else if (ctrl->getProcId() == id_downlinkNASTransport) {
			if (conn != NULL) {
				conn->sendDownlinkNasTransport(sub, new NasPdu(ctrl));
			}
		} else if (ctrl->getProcId() == id_uplinkNASTransport) {
			if (conn != NULL) {
				conn->sendUplinkNasTransport(sub, new NasPdu(ctrl));
			}
		}
	}
	delete msg;
}

void S1AP::sendMessageUp(Subscriber *sub, NasPdu *nasPdu) {
	cMessage *msg = new cMessage();
	S1APControlInfo *ctrl = new S1APControlInfo();
	ctrl->setUeEnbId(sub->getEnbId());
	ctrl->setUeMmeId(sub->getMmeId());
	ctrl->setValueArraySize(nasPdu->getLength());
	char *buf = nasPdu->getValue();
	for (unsigned i = 0; i < nasPdu->getLength(); i++)
		ctrl->setValue(i, buf[i]);
	msg->setControlInfo(ctrl);
	send(msg, gate("nasOut"));
	delete nasPdu;
}

void S1AP::processTimer(cMessage *msg) {
	if (msg == retryTimer) {
		if (!sendQueue.isEmpty()) {
			cMessage *msg = (cMessage*)sendQueue.pop();
			handleUpperMessage(msg);
		}
		cancelEvent(retryTimer);
		scheduleAt(simTime() + 2, retryTimer);
	}
}

void S1AP::loadOwnConfigFromXML(const cXMLElement& s1apNode) {
	if (s1apNode.getAttribute("name") != NULL)
		name = s1apNode.getAttribute("name");

	if (type == CONNECTOR) {
		std::string mcc = s1apNode.getAttribute("mcc");
		if (mcc.empty())
			error("S1AP: No configuration for mcc");
		std::string mnc = s1apNode.getAttribute("mnc");
		if (mnc.empty())
			error("S1AP: No configuration for mnc");
		this->plmnId = LTEUtils().toPLMNId(mcc, mnc);
		if (s1apNode.getAttribute("cellId") != NULL) {
			std::string cellId = s1apNode.getAttribute("cellId");
			this->cellId = LTEUtils().toByteString(cellId, CELLID_UNCODED_SIZE);
		}
		cXMLElement* suppTasNode = s1apNode.getElementByPath("SuppTas");
		if (suppTasNode != NULL) {
			loadSuppTasFromXML(*suppTasNode);
		}
	} else if (type == LISTENER) {
		cXMLElement* servGummeisNode = s1apNode.getElementByPath("ServGummeis");
		if (servGummeisNode != NULL) {
			loadServGummeisFromXML(*servGummeisNode);
		}
	}
}

void S1AP::loadListenersFromXML(const cXMLElement& s1apNode) {

	int port;
	AddressVector addresses;

	cXMLElement* listenersNode = s1apNode.getElementByPath("Listeners");
	if (listenersNode == NULL)
		error("S1AP: No configuration for S1AP listeners");

	if (!listenersNode->getAttribute("port"))
		error("S1AP: Listeners has no port attribute");
	port = atoi(listenersNode->getAttribute("port"));

    cXMLElementList listenersList = listenersNode->getChildren();
    for (cXMLElementList::iterator listenersIt = listenersList.begin(); listenersIt != listenersList.end(); listenersIt++) {

    	std::string elementName = (*listenersIt)->getTagName();
        if ((elementName == "Listener")) {
        	const char *address;

        	address = (*listenersIt)->getAttribute("address");
        	if (!address)
        		error("Diameter: Listener has no address attribute");
        	addresses.push_back(IPvXAddress(address));
        }
    }

    if (addresses.size() > 0) {
		serverSocket = new SCTPSocket();
		serverSocket.setOutputGate(gate("sctpOut"));
		serverSocket.setOutboundStreams(outboundStreams);
		serverSocket.bindx(addresses, port);
		serverSocket.listen(true);
    }
}

void S1AP::loadConnectorsFromXML(const cXMLElement& s1apNode) {
	cXMLElement* connsNode = s1apNode.getElementByPath("Connectors");
	if (connsNode != NULL) {
	    cXMLElementList connsList = connsNode->getChildren();
	    for (cXMLElementList::iterator connIt = connsList.begin(); connIt != connsList.end(); connIt++) {

	    	std::string elementName = (*connIt)->getTagName();
	        if ((elementName == "Connector")) {
	        	AddressVector remoteAddrs;
	        	AddressVector localAddrs;
	        	int port;
	        	SCTPSocket *socket = new SCTPSocket();
	        	S1APConnection *conn = new S1APConnection(this);

	        	if (!(*connIt)->getAttribute("address"))
					error("S1AP: Connector has no address attribute");
	        	IPvXAddress remoteAddr = IPvXAddress((*connIt)->getAttribute("address"));
	        	remoteAddrs.push_back(remoteAddr);
	        	const IPv4Route *route = rT->findBestMatchingRoute(remoteAddr.get4());
	        	if (route == NULL) {
	        		error("S1AP: No route to host");
	        	}
	        	localAddrs.push_back(route->getGateway());
	        	if (!(*connIt)->getAttribute("port"))
	        		error("S1AP: Connector has no port attribute");
	        	port = atoi((*connIt)->getAttribute("port"));

	           	socket->bindx(localAddrs, EPHEMERAL_PORT_MIN + uniform(EPHEMERAL_PORT_MIN, EPHEMERAL_PORT_MAX));
	           	socket->setOutputGate(gate("sctpOut"));
	           	socket->setCallbackObject(conn);
	           	socket->setOutboundStreams(outboundStreams);
	           	conn->setPlmnId(plmnId);
	           	conn->setCellId(cellId);
	           	conn->setSupportedTas(suppTas);
	           	conn->setSocket(socket);
	           	conn->setAddresses(remoteAddrs);
	           	conn->setPort(port);
	           	conns.push_back(conn);
	           	conn->connect();
	        }
	    }
	}
}

void S1AP::loadSuppTasFromXML(const cXMLElement &suppTasNode) {
    cXMLElementList suppTasList = suppTasNode.getChildren();
    for (cXMLElementList::iterator suppTaIt = suppTasList.begin(); suppTaIt != suppTasList.end(); suppTaIt++) {
    	std::string elementName = (*suppTaIt)->getTagName();
        if ((elementName == "SuppTa")) {
        	bool pres = 0;
        	SupportedTaItem supportedTaIt = SupportedTaItem();
        	if (!(*suppTaIt)->getAttribute("tac"))
        		continue;

        	supportedTaIt.tac = LTEUtils().toByteString((*suppTaIt)->getAttribute("tac"), TAC_UNCODED_SIZE);

        	cXMLElement* bplmnsNode = (*suppTaIt)->getElementByPath("Bplmns");
        	if (bplmnsNode == NULL)
        		continue;

        	cXMLElementList bplmnsList = bplmnsNode->getChildren();
            for (cXMLElementList::iterator bplmnsListIt = bplmnsList.begin(); bplmnsListIt != bplmnsList.end(); bplmnsListIt++) {
            	std::string elementName = (*bplmnsListIt)->getTagName();
            	if ((elementName == "Bplmn")) {
            		if ((*bplmnsListIt)->getAttribute("mcc") == NULL)
            			continue;
            		if ((*bplmnsListIt)->getAttribute("mnc") == NULL)
            			continue;
            		supportedTaIt.bplmns.push_back(LTEUtils().toPLMNId((*bplmnsListIt)->getAttribute("mcc"), (*bplmnsListIt)->getAttribute("mnc")));
            		pres = 1;
            	}
            }
            if (pres)
            	suppTas.push_back(supportedTaIt);
        }
    }
}

void S1AP::loadServGummeisFromXML(const cXMLElement &servGummeisNode) {
    cXMLElementList servGummeisList = servGummeisNode.getChildren();
    for (cXMLElementList::iterator servGummeiIt = servGummeisList.begin(); servGummeiIt != servGummeisList.end(); servGummeiIt++) {
    	std::string elementName = (*servGummeiIt)->getTagName();
        if ((elementName == "ServGummei")) {
        	unsigned char pres = 0;
        	ServedGummeiItem servedGummeiIt = ServedGummeiItem();

        	cXMLElement* servPlmnsNode = (*servGummeiIt)->getElementByPath("ServPlmns");
        	if (servPlmnsNode == NULL)
        		continue;

        	cXMLElementList servPlmnsList = servPlmnsNode->getChildren();
            for (cXMLElementList::iterator servPlmnIt = servPlmnsList.begin(); servPlmnIt != servPlmnsList.end(); servPlmnIt++) {
            	std::string elementName = (*servPlmnIt)->getTagName();
            	if ((elementName == "ServPlmn")) {
            		if ((*servPlmnIt)->getAttribute("mcc") == NULL)
            			continue;
            		if ((*servPlmnIt)->getAttribute("mnc") == NULL)
            			continue;
            		servedGummeiIt.servPlmns.push_back(LTEUtils().toPLMNId((*servPlmnIt)->getAttribute("mcc"), (*servPlmnIt)->getAttribute("mnc")));
            		pres += 1;
            	}
            }

        	cXMLElement* servGrIdsNode = (*servGummeiIt)->getElementByPath("ServGroupIds");
        	if (servGrIdsNode == NULL)
        		continue;

        	cXMLElementList servGrIdsList = servGrIdsNode->getChildren();
            for (cXMLElementList::iterator servGrIdIt = servGrIdsList.begin(); servGrIdIt != servGrIdsList.end(); servGrIdIt++) {
            	std::string elementName = (*servGrIdIt)->getTagName();
            	if ((elementName == "ServGroup")) {
            		if ((*servGrIdIt)->getAttribute("id") == NULL)
            			continue;
            		servedGummeiIt.servGrIds.push_back(LTEUtils().toByteString((*servGrIdIt)->getAttribute("id"), GROUPID_CODED_SIZE));
            		pres += 1;
            	}
            }

        	cXMLElement* servMmecsNode = (*servGummeiIt)->getElementByPath("ServMmecs");
        	if (servMmecsNode == NULL)
        		continue;

        	cXMLElementList servMmecsList = servMmecsNode->getChildren();
            for (cXMLElementList::iterator servMmeIt = servMmecsList.begin(); servMmeIt != servMmecsList.end(); servMmeIt++) {
            	std::string elementName = (*servMmeIt)->getTagName();
            	if ((elementName == "ServMme")) {
            		if ((*servMmeIt)->getAttribute("code") == NULL)
            			continue;
            		servedGummeiIt.servMmecs.push_back(LTEUtils().toByteString((*servMmeIt)->getAttribute("code"), MMECODE_CODED_SIZE));
            		pres += 1;
            	}
            }
            if (pres == 3)
            	servGummeis.push_back(servedGummeiIt);
        }
    }
}

void S1AP::loadConfigFromXML(const char *filename) {
	cXMLElement* config = ev.getXMLDocument(filename);
    if (config == NULL)
        error("S1AP: Cannot read configuration from file: %s", filename);

    cXMLElement* s1apNode = config->getElementByPath("S1AP");
	if (s1apNode == NULL)
		error("S1AP: No configuration for S1AP application");

	if (!s1apNode->getAttribute("type"))
		error("S1AP: Peer has no type attribute");
	type = atoi(s1apNode->getAttribute("type"));

	loadOwnConfigFromXML(*s1apNode);
	if (type == LISTENER) {
		loadListenersFromXML(*s1apNode);
	} else {
		loadConnectorsFromXML(*s1apNode);
	}
}

void S1AP::receiveChangeNotification(int category, const cPolymorphic *details) {
	Enter_Method_Silent();
	if (category == NF_SUB_TUNN_ACK) {
		EV << "S1AP: Received NF_SUB_TUNN_ACK notification. Processing notification.\n";
		BearerContext *bearer = check_and_cast<BearerContext*>(details);
		Subscriber *sub = bearer->getSubscriber();
		S1APConnection *conn = conns.findConnectionForCellId(sub->getCellId());
		if (conn == NULL) {
			return;
		}
		conn->sendInitialContextSetupResponse(sub);
//		sendInitialContextSetupResponse(sub);
	}
}


