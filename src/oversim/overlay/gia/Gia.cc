//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file Gia.cc
 * @author Robert Palmer
 */

#include <assert.h>

#include <GlobalStatistics.h>
#include <CommonMessages_m.h>
#include <ExtAPIMessages_m.h>
#include <InitStages.h>
#include <BootstrapList.h>

#include "Gia.h"


Define_Module(Gia);

void Gia::initializeOverlay(int stage)
{
    // wait until IPAddressResolver initialized all interfaces and assigns addresses
    if(stage != MIN_STAGE_OVERLAY)
        return;

    // Get parameters from omnetpp.ini
    maxNeighbors = par("maxNeighbors");
    minNeighbors = par("minNeighbors");
    maxTopAdaptionInterval = par("maxTopAdaptionInterval");
    topAdaptionAggressiveness = par("topAdaptionAggressiveness");
    maxLevelOfSatisfaction = par("maxLevelOfSatisfaction");
    updateDelay = par("updateDelay");
    maxHopCount = par("maxHopCount"); //obsolete
    messageTimeout = par("messageTimeout");
    neighborTimeout = par("neighborTimeout");
    sendTokenTimeout = par("sendTokenTimeout");
    tokenWaitTime = par("tokenWaitTime");
    keyListDelay = par("keyListDelay");
    outputNodeDetails = par("outputNodeDetails");
    optimizeReversePath = par("optimizeReversePath");

    // get references on necessary modules
    keyListModule = check_and_cast<GiaKeyListModule*>
        (getParentModule()->getSubmodule("keyListModule"));
    neighbors = check_and_cast<GiaNeighbors*>
        (getParentModule()->getSubmodule("neighbors"));
    tokenFactory = check_and_cast<GiaTokenFactory*>
        (getParentModule()->getSubmodule("tokenFactory"));

    msgBookkeepingList = new GiaMessageBookkeeping(neighbors, messageTimeout);

    // clear neighbor candidate list
    neighCand.clear();
    knownNodes.clear();

    WATCH(thisGiaNode);
    WATCH(bootstrapNode);
    WATCH(levelOfSatisfaction);

    // self-messages
    satisfaction_timer = new cMessage("satisfaction_timer");
    update_timer = new cMessage("update_timer");
    timedoutMessages_timer = new cMessage("timedoutMessages_timer");
    timedoutNeighbors_timer = new cMessage("timedoutNeighbors_timer");
    sendKeyList_timer = new cMessage("sendKeyList_timer");
    sendToken_timer = new cMessage("sendToken_timer");

    // statistics
    stat_joinCount = 0;
    stat_joinBytesSent = 0;
    stat_joinREQ = 0;
    stat_joinREQBytesSent = 0;
    stat_joinRSP = 0;
    stat_joinRSPBytesSent = 0;
    stat_joinACK = 0;
    stat_joinACKBytesSent = 0;
    stat_joinDNY = 0;
    stat_joinDNYBytesSent = 0;
    stat_disconnectMessages = 0;
    stat_disconnectMessagesBytesSent = 0;
    stat_updateMessages = 0;
    stat_updateMessagesBytesSent = 0;
    stat_tokenMessages = 0;
    stat_tokenMessagesBytesSent = 0;
    stat_keyListMessages = 0;
    stat_keyListMessagesBytesSent = 0;
    stat_routeMessages = 0;
    stat_routeMessagesBytesSent = 0;
    stat_maxNeighbors = 0;
    stat_addedNeighbors = 0;
    stat_removedNeighbors = 0;
    stat_numSatisfactionMessages = 0;
    stat_sumLevelOfSatisfaction = 0.0;
    stat_maxLevelOfSatisfaction = 0.0;
}

void Gia::joinOverlay()
{
    changeState(INIT);

    if (bootstrapNode.isUnspecified())
        changeState(READY);
}

void Gia::changeState(int toState)
{
    switch (toState) {
    case INIT: {
        state = INIT;

        setOverlayReady(false);
        showOverlayNeighborArrow(thisGiaNode);

        // set up local nodehandle
        //thisGiaNode.setNodeHandle(thisNode);
        thisGiaNode = thisNode;

        callUpdate(thisNode, true);

        // get possible bandwidth from ppp-Module
        double capacity = 0;

        cModule* nodeModule = getParentModule()->getParentModule();

        if(!nodeModule->hasGate("pppg$i"))
            capacity += uniform(1,800000);
        else {
            // this relies on IPv4Underlay
        	int gateSize = nodeModule->gateSize("pppg$i");
            for (int i=0; i<gateSize; i++) {
                cGate* currentGate = nodeModule->gate("pppg$i",i);
                if (currentGate->isConnected())
                    capacity += check_and_cast<cDatarateChannel *>
                        (currentGate->getPreviousGate()->getChannel())->getDatarate()
                        - uniform(0,800000);
            }
        }

        thisGiaNode.setCapacity(capacity);
        //thisGiaNode.setConnectionDegree(0);
        //thisGiaNode.setReceivedTokens(0);
        //thisGiaNode.setSentTokens(0);

        connectionDegree = 0;
        receivedTokens = 0;
        sentTokens = 0;

        if (outputNodeDetails)
            EV << "(Gia) Node details: " << thisGiaNode << endl;

        // get our entry point to GIA-Overlay
        bootstrapNode = bootstrapList->getBootstrapNode();
        if(!(bootstrapNode.isUnspecified()))
            knownNodes.add(bootstrapNode);
        //else {
        //assert(!(thisGiaNode.getNodeHandle().isUnspecified()));
        //globalNodeList->registerPeer(thisGiaNode.getNodeHandle());
        //}

        // register node at TokenFactory
        //tokenFactory->setNode(&thisGiaNode);
        tokenFactory->setNeighbors(neighbors);
        tokenFactory->setMaxHopCount(maxHopCount);

        if (debugOutput)
            EV << "(Gia) Node " << thisGiaNode.getKey()
               << " (" << thisGiaNode.getIp() << ":"
               << thisGiaNode.getPort() << ") with capacity: "
               << thisGiaNode.getCapacity()  << " entered INIT state." << endl;

        getParentModule()->getParentModule()->bubble("Enter INIT state.");

        // schedule satisfaction_timer
        cancelEvent(satisfaction_timer);
        scheduleAt(simTime(), satisfaction_timer);

        // schedule timedoutMessages_timer
        cancelEvent(timedoutMessages_timer);
        scheduleAt(simTime() + messageTimeout,
                   timedoutMessages_timer);

        cancelEvent(timedoutNeighbors_timer);
        scheduleAt(simTime() + neighborTimeout,
                   timedoutNeighbors_timer);

        cancelEvent(sendToken_timer);
        scheduleAt(simTime() + sendTokenTimeout,
                   sendToken_timer);

        break;
    }
    case READY:
        state = READY;
        setOverlayReady(true);
        break;
    }
    updateTooltip();
}

// void Gia::setBootstrapedIcon()
// {
//     if (ev.isGUI()) {
//         if (state == READY) {
//             getParentModule()->getParentModule()->getDisplayString().
//                 setTagArg("i2", 1, "");
//             getDisplayString().setTagArg("i", 1, "");
//         } else {
//             getParentModule()->getParentModule()->getDisplayString().
//                 setTagArg("i2", 1, "red");
//             getDisplayString().setTagArg("i", 1, "red");
//         }
//     }
// }

void Gia::updateTooltip()
{
    if (ev.isGUI()) {
//        if (state == READY) {
//            getParentModule()->getParentModule()->getDisplayString().
//                setTagArg("i2", 1, "");
//            getDisplayString().setTagArg("i", 1, "");
//        } else {
//            getParentModule()->getParentModule()->getDisplayString().
//                setTagArg("i2", 1, "red");
//            getDisplayString().setTagArg("i", 1, "red");
//        }

        std::stringstream ttString;

        // show our predecessor and successor in tooltip
        ttString << thisNode << "\n# Neighbors: "
                 << neighbors->getSize();

        getParentModule()->getParentModule()->getDisplayString().
            setTagArg("tt", 0, ttString.str().c_str());
        getParentModule()->getDisplayString().
            setTagArg("tt", 0, ttString.str().c_str());
        getDisplayString().setTagArg("tt", 0, ttString.str().c_str());
    }
}

void Gia::handleTimerEvent(cMessage* msg)
{
    if (msg == sendToken_timer) {
        tokenFactory->grantToken();
    } else if (msg->isName("satisfaction_timer")) {
        // calculate level of satisfaction and select new neighbor candidate

        levelOfSatisfaction = calculateLevelOfSatisfaction();
        stat_numSatisfactionMessages++;
        stat_sumLevelOfSatisfaction += levelOfSatisfaction;
        if (levelOfSatisfaction > stat_maxLevelOfSatisfaction)
            stat_maxLevelOfSatisfaction = levelOfSatisfaction;

        // start again satisfaction_timer
        scheduleAt(simTime() + (maxTopAdaptionInterval *
                                           pow(topAdaptionAggressiveness,
                                               -(1 - levelOfSatisfaction))),
                   satisfaction_timer);

        // Only search a new neighbor if level of satisfaction is
        // under level of maxLevelOfSatisfaction
        if(levelOfSatisfaction < maxLevelOfSatisfaction) {
            if(knownNodes.getSize() == 0) {
                if(neighbors->getSize() == 0 && neighCand.getSize() == 0)
                    knownNodes.add(globalNodeList->getBootstrapNode()/*bootstrapList->getBootstrapNode()*/);
                else
                    return;
            }

            NodeHandle possibleNeighbor = knownNodes.getRandomCandidate();

            if(!(possibleNeighbor.isUnspecified()) &&
               thisGiaNode != possibleNeighbor &&
               !neighbors->contains(possibleNeighbor) &&
               !neighCand.contains(possibleNeighbor)) {
                // try to add new neighbor
                neighCand.add(possibleNeighbor);
                sendMessage_JOIN_REQ(possibleNeighbor);
            }
        }
    } else if (msg->isName("update_timer")) {
        // send current capacity and degree to all neighbors
        for (uint32_t i=0; i<neighbors->getSize(); i++) {
            sendMessage_UPDATE(neighbors->get(i));
        }
    } else if (msg->isName("timedoutMessages_timer")) {
        // remove timedout messages
        msgBookkeepingList->removeTimedoutMessages();
        scheduleAt(simTime() + messageTimeout,
                   timedoutMessages_timer);
    } else if (msg->isName("timedoutNeighbors_timer")) {
        // remove timedout neighbors
        neighbors->removeTimedoutNodes();
        if (neighbors->getSize() == 0) {
            changeState(INIT);
        }
        cancelEvent(timedoutNeighbors_timer);
        scheduleAt(simTime() + neighborTimeout,
                   timedoutNeighbors_timer);
    } else if (msg->isName("sendKeyList_timer")) {
        if (keyList.getSize() > 0) {
            // send keyList to all of our neighbors
            for (uint32_t i=0; i<neighbors->getSize(); i++)
                sendKeyListToNeighbor(neighbors->get(i));
        }
    } else {
        // other self-messages are notoken-self-messages
        // with an encapsulated message
        const std::string id = msg->getName();
        if (id.substr(0, 16) == std::string("wait-for-token: ")) {
        	cPacket* packet = check_and_cast<cPacket*>(msg);
            cMessage* decapsulatedMessage = packet->decapsulate();
            if (dynamic_cast<GiaIDMessage*>(decapsulatedMessage) != NULL) {
                GiaIDMessage* message = check_and_cast<GiaIDMessage*>
                    (decapsulatedMessage);
                forwardMessage(message, false);
            }
        } else if (id.substr(0, 24) == std::string("wait-for-token-fromapp: ")) {
            cPacket* packet = check_and_cast<cPacket*>(msg);
            cMessage* decapsulatedMessage = packet->decapsulate();
            if (dynamic_cast<GiaIDMessage*>(decapsulatedMessage) != NULL) {
                GiaIDMessage* message = check_and_cast<GiaIDMessage*>
                    (decapsulatedMessage);
                forwardMessage(message, true);
            }
        }
        delete msg;
    }
}

void Gia::handleUDPMessage(BaseOverlayMessage* msg)
{
    if(debugOutput)
        EV << "(Gia) " << thisGiaNode << " received udp message" << endl;

    cObject* ctrlInfo = msg->removeControlInfo();
    if(ctrlInfo != NULL)
        delete ctrlInfo;

    // Parse TokenMessages
    if (dynamic_cast<TokenMessage*>(msg) != NULL) {
        TokenMessage* giaMsg = check_and_cast<TokenMessage*>(msg);

        // Process TOKEN-Message
        if ((giaMsg->getCommand() == TOKEN)) {
            if(debugOutput)
                EV << "(Gia) Received Tokenmessage from "
		           << giaMsg->getSrcNode() << endl;

	    //neighbors->setReceivedTokens(giaMsg->getSrcNode(), giaMsg->getDstTokenNr());
            neighbors->increaseReceivedTokens(giaMsg->getSrcNode());
	    updateNeighborList(giaMsg);
        }
        delete msg;
    }

    // Process Route messages
    else if (dynamic_cast<GiaRouteMessage*>(msg) != NULL) {
        GiaRouteMessage* giaMsg = check_and_cast<GiaRouteMessage*>(msg);
        GiaNode oppositeNode(giaMsg->getSrcNode(), giaMsg->getSrcCapacity(),
                             giaMsg->getSrcDegree());

        if((giaMsg->getCommand() == ROUTE)) {
            if(debugOutput)
                EV << "(Gia) Received ROUTE::IND from " << oppositeNode << endl;

            if(state == READY) {
		//neighbors->decreaseReceivedTokens(giaMsg->getSrcNode());
		updateNeighborList(giaMsg);
		forwardMessage(giaMsg, false);
            }
        }
    }

    // Process KeyList-Messages
    else if (dynamic_cast<KeyListMessage*>(msg) != NULL) {
        KeyListMessage* giaMsg = check_and_cast<KeyListMessage*>(msg);
        GiaNode oppositeNode(giaMsg->getSrcNode(), giaMsg->getSrcCapacity(),
                             giaMsg->getSrcDegree());

        if (giaMsg->getCommand() == KEYLIST) {
            if (debugOutput)
                EV << "(Gia) " << thisGiaNode
                   << " received KEYLIST:IND message" << endl;
            // update KeyList in neighborList
            uint32_t keyListSize = giaMsg->getKeysArraySize();
            GiaKeyList neighborKeyList;
            for (uint32_t k = 0; k < keyListSize; k++)
                neighborKeyList.addKeyItem(giaMsg->getKeys(k));
            neighbors->setNeighborKeyList(giaMsg->getSrcNode(), neighborKeyList);
        }
        delete giaMsg;
    }

    // Process Search-Messages
    else if (dynamic_cast<SearchMessage*>(msg) != NULL) {
        SearchMessage* giaMsg = check_and_cast<SearchMessage*>(msg);
        GiaNode oppositeNode(giaMsg->getSrcNode(), giaMsg->getSrcCapacity(),
                             giaMsg->getSrcDegree());

	//neighbors->decreaseSentTokens(giaMsg->getSrcNode());
        updateNeighborList(giaMsg);
        processSearchMessage(giaMsg, false);
        // } else {
        //             EV << "(Gia) Message " << msg << " dropped!" << endl;
        //             RECORD_STATS(numDropped++; bytesDropped += msg->getByteLength());
        //             delete msg;
        //         }
    }

    // Process Search-Response-Messages
    else if (dynamic_cast<SearchResponseMessage*>(msg) != NULL) {
        SearchResponseMessage* responseMsg =
            check_and_cast<SearchResponseMessage*>(msg);
        forwardSearchResponseMessage(responseMsg);
    }

    // Process Gia messages
    else if (dynamic_cast<GiaMessage*>(msg) != NULL) {
        GiaMessage* giaMsg = check_and_cast<GiaMessage*>(msg);

        //assert(giaMsg->getSrcNode().moduleId != -1);
        GiaNode oppositeNode(giaMsg->getSrcNode(), giaMsg->getSrcCapacity(),
                             giaMsg->getSrcDegree());

        if (debugOutput)
            EV << "(Gia) " << thisGiaNode << " received GIA- message from "
               << oppositeNode << endl;
        updateNeighborList(giaMsg);

        // Process JOIN:REQ messages
        if (giaMsg->getCommand() == JOIN_REQUEST) {
            if (debugOutput)
                EV << "(Gia) " << thisGiaNode
                   << " received JOIN:REQ message" << endl;
            if (acceptNode(oppositeNode, giaMsg->getSrcDegree())) {
                neighCand.add(oppositeNode);
                sendMessage_JOIN_RSP(oppositeNode);
            } else {
                if (debugOutput)
                    EV << "(Gia) " << thisGiaNode << " denies node "
                       << oppositeNode << endl;
                sendMessage_JOIN_DNY(oppositeNode);
            }
        }

        // Process JOIN:RSP messages
        else if (giaMsg->getCommand() == JOIN_RESPONSE) {
            if(debugOutput)
                EV << "(Gia) " << thisGiaNode << " received JOIN:RSP message"
                   << endl;
            if(neighCand.contains(oppositeNode)) {
                neighCand.remove(oppositeNode);
                if(acceptNode(oppositeNode, giaMsg->getSrcDegree())) {
                    addNeighbor(oppositeNode, giaMsg->getSrcDegree());

                    GiaNeighborMessage* msg =
                        check_and_cast<GiaNeighborMessage*>(giaMsg);
                    for(uint32_t i = 0; i < msg->getNeighborsArraySize(); i++) {
                        GiaNode temp = msg->getNeighbors(i);
                        if(temp != thisGiaNode && temp != oppositeNode)
                            knownNodes.add(temp);
                    }

                    sendMessage_JOIN_ACK(oppositeNode);
                } else {
                    if (debugOutput)
                        EV << "(Gia) " << thisGiaNode << " denies node "
                           << oppositeNode << endl;
                    sendMessage_JOIN_DNY(oppositeNode);
                }
            }
        }

        // Process JOIN:ACK messages
        else if (giaMsg->getCommand() == JOIN_ACK) {
            if (debugOutput)
                EV << "(Gia) " << thisGiaNode << " received JOIN:ACK message"
                   << endl;
            if (neighCand.contains(oppositeNode) &&
                neighbors->getSize() < maxNeighbors) {
                neighCand.remove(oppositeNode);
                addNeighbor(oppositeNode, giaMsg->getSrcDegree());

                GiaNeighborMessage* msg =
                    check_and_cast<GiaNeighborMessage*>(giaMsg);

                for(uint32_t i = 0; i < msg->getNeighborsArraySize(); i++) {
                    GiaNode temp = msg->getNeighbors(i);
                    if(temp != thisGiaNode && temp != oppositeNode)
                        knownNodes.add(msg->getNeighbors(i));
                }
            } else {
                sendMessage_DISCONNECT(oppositeNode);
            }

        }

        // Process JOIN:DNY messages
        else if (giaMsg->getCommand() == JOIN_DENY) {
            if (debugOutput)
                EV << "(Gia) " << thisGiaNode << " received JOIN:DNY message"
                   << endl;

            if (neighCand.contains(oppositeNode))
                neighCand.remove(oppositeNode);
            knownNodes.remove(oppositeNode);

        }


        // Process DISCONNECT-Message
        else if (giaMsg->getCommand() == DISCONNECT) {
            if (debugOutput)
                EV << "(Gia) " << thisGiaNode << " received DISCONNECT:IND message" << endl;
	    removeNeighbor(giaMsg->getSrcNode());
        }

        // Process UPDATE-Message
        else if (giaMsg->getCommand() == UPDATE) {
            if (debugOutput)
                EV << "(Gia) " << thisGiaNode << " received UPDATE:IND message"
		   << endl;

	    neighbors->setConnectionDegree(giaMsg->getSrcNode(),
					   giaMsg->getSrcDegree());
	    //neighbors->setCapacity(giaMsg->getSrcNode(),
            //giaMsg->getSrcCapacity());
        } else {
            // Show unknown gia-messages
            if (debugOutput) {
                EV << "(Gia) NODE: "<< thisGiaNode << endl
                   << "       Command: " << giaMsg->getCommand() << endl
                   << "       HopCount: " << giaMsg->getHopCount() << endl
                   << "       SrcKey: " << giaMsg->getSrcNode().getKey() << endl
                   << "       SrcIP: " << giaMsg->getSrcNode().getKey() << endl
                   << "       SrcPort: " << giaMsg->getSrcNode().getPort() << endl
                   << "       SrcCapacity: " << giaMsg->getSrcCapacity() << endl
                   << "       SrcDegree: " << giaMsg->getSrcDegree() << endl;

                RECORD_STATS(numDropped++;bytesDropped += giaMsg->getByteLength());
            }
        }
        delete giaMsg;
    } else // PROCESS other messages than GiaMessages
        delete msg; // delete them all
}

bool Gia::acceptNode(const GiaNode& nNode, unsigned int degree)
{
    if (neighbors->contains(nNode))
        return false;

    if (neighbors->getSize() < maxNeighbors) {
        // we have room for new node: accept node
        return true;
    }
    // we need to drop a neighbor
    NodeHandle dropCandidate = neighbors->getDropCandidate(nNode.getCapacity(), degree);
    if(!dropCandidate.isUnspecified()) {
	if (debugOutput)
	    EV << "(Gia) " << thisGiaNode << " drops "
	       << dropCandidate <<  endl;
	neighbors->remove(dropCandidate);
	sendMessage_DISCONNECT(dropCandidate);
	return true;
    }
    return false;
}


void Gia::addNeighbor(GiaNode& node, unsigned int degree)
{
    assert(neighbors->getSize() < maxNeighbors);

    stat_addedNeighbors++;
    EV << "(Gia) " << thisGiaNode << " accepted new neighbor " << node << endl;
    getParentModule()->getParentModule()->bubble("New neighbor");
    connectionDegree = neighbors->getSize();
    changeState(READY);
    if (stat_maxNeighbors < neighbors->getSize()) {
        stat_maxNeighbors = neighbors->getSize();
    }

    cancelEvent(update_timer);
    scheduleAt(simTime() + updateDelay, update_timer);

    //node.setSentTokens(5);
    //node.setReceivedTokens(5);

    // send keyList to new Neighbor
    if (keyList.getSize() > 0)
        sendKeyListToNeighbor(node);
    neighbors->add(node, degree);

    showOverlayNeighborArrow(node, false,
                             "m=m,50,0,50,0;ls=red,1");
    updateTooltip();
}

void Gia::removeNeighbor(const GiaNode& node)
{
    stat_removedNeighbors++;

    if (debugOutput)
	EV << "(Gia) " << thisGiaNode << " removes " << node
	   << " from neighborlist." << endl;
    neighbors->remove(node);

    connectionDegree = neighbors->getSize();

    if (neighbors->getSize() == 0) {
	changeState(INIT);
    }

    deleteOverlayNeighborArrow(node);
    updateTooltip();

    cancelEvent(update_timer);
    scheduleAt(simTime() + updateDelay, update_timer);
}

double Gia::calculateLevelOfSatisfaction()
{
    uint32_t neighborsCount = neighbors->getSize();
    if(neighborsCount < minNeighbors)
        return 0.0;

    double total = 0.0;
    double levelOfSatisfaction = 0.0;

    for (uint32_t i=0; i < neighborsCount; i++)
        total += neighbors->get(i).getCapacity() / neighborsCount;

    assert(thisGiaNode.getCapacity() != 0);
    levelOfSatisfaction = total / thisGiaNode.getCapacity();

    if ((levelOfSatisfaction > 1.0) || (neighborsCount >= maxNeighbors))
        return 1.0;
    return levelOfSatisfaction;
}


void Gia::sendMessage_JOIN_REQ(const NodeHandle& dst)
{
    // send JOIN:REQ message
    GiaMessage* msg = new GiaMessage("JOIN_REQ");
    msg->setCommand(JOIN_REQUEST);
    msg->setSrcNode(thisGiaNode);
    msg->setSrcCapacity(thisGiaNode.getCapacity());
    msg->setSrcDegree(connectionDegree);
    msg->setBitLength(GIA_L(msg));

    stat_joinCount += 1;
    stat_joinBytesSent += msg->getByteLength();
    stat_joinREQ += 1;
    stat_joinREQBytesSent += msg->getByteLength();

    sendMessageToUDP(dst, msg);
}

void Gia::sendMessage_JOIN_RSP(const NodeHandle& dst)
{
    // send JOIN:RSP message
    GiaNeighborMessage* msg = new GiaNeighborMessage("JOIN_RSP");
    msg->setCommand(JOIN_RESPONSE);
    msg->setSrcNode(thisGiaNode);
    msg->setSrcCapacity(thisGiaNode.getCapacity());
    msg->setSrcDegree(connectionDegree);

    msg->setNeighborsArraySize(neighbors->getSize());
    //TODO: add parameter maxSendNeighbors
    for(uint32_t i = 0; i < neighbors->getSize(); i++)
        msg->setNeighbors(i, neighbors->get(i));

    msg->setBitLength(GIANEIGHBOR_L(msg));

    stat_joinCount += 1;
    stat_joinBytesSent += msg->getByteLength();
    stat_joinRSP += 1;
    stat_joinRSPBytesSent += msg->getByteLength();

    sendMessageToUDP(dst, msg);
}

void Gia::sendMessage_JOIN_ACK(const NodeHandle& dst)
{
    // send JOIN:ACK message
    GiaNeighborMessage* msg = new GiaNeighborMessage("JOIN_ACK");
    msg->setCommand(JOIN_ACK);
    msg->setSrcNode(thisGiaNode);
    msg->setSrcCapacity(thisGiaNode.getCapacity());
    msg->setSrcDegree(connectionDegree);

    msg->setNeighborsArraySize(neighbors->getSize());
    //TODO: add parameter maxSendNeighbors
    for(uint32_t i = 0; i < neighbors->getSize(); i++)
        msg->setNeighbors(i, neighbors->get(i));

    msg->setBitLength(GIANEIGHBOR_L(msg));

    stat_joinCount += 1;
    stat_joinBytesSent += msg->getByteLength();
    stat_joinACK += 1;
    stat_joinACKBytesSent += msg->getByteLength();

    sendMessageToUDP(dst, msg);
}

void Gia::sendMessage_JOIN_DNY(const NodeHandle& dst)
{
    // send JOIN:DNY message
    GiaMessage* msg = new GiaMessage("JOIN_DENY");
    msg->setCommand(JOIN_DENY);
    msg->setSrcNode(thisGiaNode);
    msg->setSrcCapacity(thisGiaNode.getCapacity());
    msg->setSrcDegree(connectionDegree);
    msg->setBitLength(GIA_L(msg));

    stat_joinCount += 1;
    stat_joinBytesSent += msg->getByteLength();
    stat_joinDNY += 1;
    stat_joinDNYBytesSent += msg->getByteLength();

    sendMessageToUDP(dst, msg);
}

void Gia::sendMessage_DISCONNECT(const NodeHandle& dst)
{
    // send DISCONNECT:IND message
    GiaMessage* msg = new GiaMessage("DISCONNECT");
    msg->setCommand(DISCONNECT);
    msg->setSrcNode(thisGiaNode);
    msg->setSrcCapacity(thisGiaNode.getCapacity());
    msg->setSrcDegree(connectionDegree);
    msg->setBitLength(GIA_L(msg));

    stat_disconnectMessagesBytesSent += msg->getByteLength();
    stat_disconnectMessages += 1;

    sendMessageToUDP(dst, msg);
}

void Gia::sendMessage_UPDATE(const NodeHandle& dst)
{
    // send UPDATE:IND message
    GiaMessage* msg = new GiaMessage("UPDATE");
    msg->setCommand(UPDATE);
    msg->setSrcNode(thisGiaNode);
    msg->setSrcCapacity(thisGiaNode.getCapacity());
    msg->setSrcDegree(connectionDegree);
    msg->setBitLength(GIA_L(msg));

    stat_updateMessagesBytesSent += msg->getByteLength();
    stat_updateMessages += 1;

    sendMessageToUDP(dst, msg);
}

void Gia::sendKeyListToNeighbor(const NodeHandle& dst)
{
    // send KEYLIST:IND message
    KeyListMessage* msg = new KeyListMessage("KEYLIST");
    msg->setCommand(KEYLIST);
    msg->setSrcNode(thisGiaNode);
    msg->setSrcCapacity(thisGiaNode.getCapacity());
    msg->setSrcDegree(connectionDegree);

    msg->setKeysArraySize(keyList.getSize());
    for (uint32_t i=0; i<keyList.getSize(); i++)
        msg->setKeys(i, keyList.get(i));

    msg->setBitLength(KEYLIST_L(msg));

    stat_keyListMessagesBytesSent += msg->getByteLength();
    stat_keyListMessages += 1;

    sendMessageToUDP(dst, msg);
}

void Gia::sendToken(const GiaNode& dst)
{
    TokenMessage* tokenMsg = new TokenMessage("TOKEN");
    tokenMsg->setCommand(TOKEN);
    tokenMsg->setHopCount(maxHopCount);
    tokenMsg->setSrcNode(thisGiaNode);
    tokenMsg->setSrcCapacity(thisGiaNode.getCapacity());
    tokenMsg->setSrcDegree(connectionDegree);
    tokenMsg->setSrcTokenNr(0/*dst.getReceivedTokens()*/);//???
    tokenMsg->setDstTokenNr(/*dst.getSentTokens()+*/1);
    tokenMsg->setBitLength(TOKEN_L(tokenMsg));

    stat_tokenMessagesBytesSent += tokenMsg->getByteLength();
    stat_tokenMessages += 1;

    sendMessageToUDP(dst, tokenMsg);
}

void Gia::updateNeighborList(GiaMessage* msg)
{
    if(neighbors->contains(msg->getSrcNode().getKey())) {
        neighbors->setConnectionDegree(msg->getSrcNode(),msg->getSrcDegree());
        //neighbors->setCapacity(msg->getSrcNode(), msg->getSrcCapacity());
        neighbors->updateTimestamp(msg->getSrcNode());
    }
}

void Gia::forwardSearchResponseMessage(SearchResponseMessage* responseMsg)
{
    if (responseMsg->getHopCount() != 0) {
        uint32_t reversePathArraySize = responseMsg->getReversePathArraySize();

        // reached node which started search?
        if (reversePathArraySize == 0) {
            deliverSearchResult(responseMsg);
        }
        // forward message to next node in reverse path list
        else {
            NodeHandle targetNode = neighbors->get
                (responseMsg->getReversePath(
                                             reversePathArraySize-1));

            if(!(targetNode.isUnspecified())) {
                responseMsg->setDestKey(targetNode.getKey());
                responseMsg->setReversePathArraySize(reversePathArraySize-1);
                for (uint32_t i=0; i<reversePathArraySize-1; i++)
                    responseMsg->setReversePath(i, responseMsg->getReversePath(i));
                responseMsg->setBitLength(responseMsg->getBitLength() - KEY_L);

                stat_routeMessagesBytesSent += responseMsg->getByteLength();
                stat_routeMessages += 1;

                if(responseMsg->getHopCount() > 0)
                    RECORD_STATS(numAppDataForwarded++; bytesAppDataForwarded +=
                                 responseMsg->getByteLength());

                sendMessageToUDP(targetNode, responseMsg);
            } else {
                EV << "(Gia) wrong reverse path in " << *responseMsg
		   << " ... deleted!" << endl;
                RECORD_STATS(numDropped++;
                             bytesDropped += responseMsg->getByteLength());
                delete responseMsg;
            }
        }
    }
    // max hopcount reached. delete message
    else
        delete responseMsg;
}

void Gia::forwardMessage(GiaIDMessage* msg , bool fromApplication)
{
    if (msg->getHopCount() == 0) {
        // Max-Hopcount reached. Discard message
        if (!fromApplication)
            tokenFactory->grantToken();
        RECORD_STATS(numDropped++; bytesDropped += msg->getByteLength());
        delete msg;
    } else {
        // local delivery?
        if (msg->getDestKey() == thisGiaNode.getKey()) {
            if(!fromApplication)
                tokenFactory->grantToken();

            if(debugOutput)
                EV << "(Gia) Deliver messsage " << msg
                   << " to application at " << thisGiaNode << endl;

            OverlayCtrlInfo* overlayCtrlInfo =
                new OverlayCtrlInfo();

            overlayCtrlInfo->setHopCount(msg->getHopCount());
            overlayCtrlInfo->setSrcNode(msg->getSrcNode());
            overlayCtrlInfo->setTransportType(ROUTE_TRANSPORT);
            overlayCtrlInfo->setDestComp(TIER1_COMP); // workaround
            overlayCtrlInfo->setSrcComp(TIER1_COMP);

            msg->setControlInfo(overlayCtrlInfo);
            callDeliver(msg, msg->getDestKey());
        } else {
            // forward message to next hop

            // do we know the destination-node?
            if (neighbors->contains(msg->getDestKey())) {
                // yes, destination-Node is our neighbor
                NodeHandle targetNode = neighbors->get(msg->getDestKey());
                GiaNeighborInfo* targetInfo= neighbors->get(targetNode);

                if (targetInfo->receivedTokens == 0) {
                    // wait for free token
                    if (debugOutput)
                        EV << "(Gia) No free Node, wait for free token" << endl;

                    //bad code
                    std::string id;
                    if (!fromApplication)
                        id = "wait-for-token: " + msg->getID().toString();
                    else
                        id = "wait-for-token-fromapp: " +
                            msg->getID().toString();
                    cPacket* wait_timer = new cPacket(id.c_str());
                    wait_timer->encapsulate(msg);
                    scheduleAt(simTime() + tokenWaitTime,wait_timer);
                    return;
                } else {
                    // decrease nextHop-tokencount
                    neighbors->decreaseReceivedTokens(targetNode);
                    //targetInfo->receivedTokens = targetInfo->receivedTokens - 1;

                    // forward msg to nextHop
                    msg->setHopCount(msg->getHopCount()-1);
                    msg->setSrcNode(thisGiaNode);
                    msg->setSrcCapacity(thisGiaNode.getCapacity());
                    msg->setSrcDegree(connectionDegree);
                    if (debugOutput)
                        EV << "(Gia) Forwarding message " << msg
                           << " to neighbor " << targetNode << endl;
                    if (!fromApplication)
                        tokenFactory->grantToken();

                    stat_routeMessagesBytesSent += msg->getByteLength();
                    stat_routeMessages += 1;

                    if(msg->getHopCount() > 0)
                        RECORD_STATS(numAppDataForwarded++; bytesAppDataForwarded +=
                                     msg->getByteLength());

                    sendMessageToUDP(targetNode, msg);
                }
            } else {
                // no => pick node with at least one token and highest capacity
                if (!msgBookkeepingList->contains(msg))
                    msgBookkeepingList->addMessage(msg);
                NodeHandle nextHop = msgBookkeepingList->getNextHop(msg);
                // do we have a neighbor who send us a token?
                if(nextHop.isUnspecified()) {
                    // wait for free token
                    if (debugOutput)
                        EV << "(Gia) No free Node, wait for free token" << endl;
                    std::string id;
                    if (!fromApplication)
                        id = "wait-for-token: " + msg->getID().toString();
                    else
                        id = "wait-for-token-fromapp: " +
                            msg->getID().toString();
                    cPacket* wait_timer = new cPacket(id.c_str());
                    wait_timer->encapsulate(msg);
                    scheduleAt(simTime() + tokenWaitTime,wait_timer);
                    return;
                } else {
		    GiaNeighborInfo* nextHopInfo = neighbors->get(nextHop);
		    if(nextHopInfo == NULL) {
			delete msg;
			return; //???
		    }
                    // decrease nextHop-tokencount
                    neighbors->decreaseReceivedTokens(nextHop);
                    //nextHopInfo->receivedTokens--;

                    // forward msg to nextHop
                    msg->setHopCount(msg->getHopCount()-1);
                    msg->setSrcNode(thisGiaNode);
                    msg->setSrcCapacity(thisGiaNode.getCapacity());
                    msg->setSrcDegree(connectionDegree);
                    if (debugOutput)
                        EV << "(Gia) Forwarding message " << msg
                           << " to " << nextHop << endl;
                    if (!fromApplication)
                        tokenFactory->grantToken();

                    stat_routeMessagesBytesSent += msg->getByteLength();
                    stat_routeMessages += 1;

                    if(msg->getHopCount() > 0)
                        RECORD_STATS(numAppDataForwarded++; bytesAppDataForwarded +=
                                     msg->getByteLength());

                    sendMessageToUDP(nextHop, msg);
                }
            }
        }
    }
}

void Gia::getRoute(const OverlayKey& key, CompType destComp,
                CompType srcComp, cPacket* msg,
                const std::vector<TransportAddress>& sourceRoute,
                RoutingType routingType)
{
    if ((destComp != TIER1_COMP) || (srcComp != TIER1_COMP)) {
        throw cRuntimeError("Gia::getRoute(): Works currently "
                             "only with srcComp=destComp=TIER1_COMP!");
    }

    if (state == READY) {
        GiaRouteMessage* routeMsg = new GiaRouteMessage("ROUTE");
        routeMsg->setStatType(APP_DATA_STAT);
        routeMsg->setCommand(ROUTE);
        routeMsg->setHopCount(maxHopCount);
        routeMsg->setDestKey(key);
        routeMsg->setSrcNode(thisGiaNode);
        routeMsg->setSrcCapacity(thisGiaNode.getCapacity());
        routeMsg->setSrcDegree(connectionDegree);
        routeMsg->setOriginatorKey(thisGiaNode.getKey());
        routeMsg->setOriginatorIP(thisGiaNode.getIp());
        routeMsg->setOriginatorPort(thisGiaNode.getPort());
        routeMsg->setID(OverlayKey::random());
        routeMsg->setBitLength(GIAROUTE_L(routeMsg));
        routeMsg->encapsulate(msg);


        forwardMessage(routeMsg, true);
    } else {
        RECORD_STATS(numDropped++; bytesDropped += msg->getByteLength());
        delete msg;
    }
}

void Gia::handleAppMessage(cMessage* msg)
{
    // do we receive a keylist?
    if (dynamic_cast<GIAput*>(msg) != NULL) {
        GIAput* putMsg = check_and_cast<GIAput*>(msg);
        uint32_t keyListSize = putMsg->getKeysArraySize();
        for (uint32_t k=0; k<keyListSize; k++)
            keyList.addKeyItem(putMsg->getKeys(k));

        // actualize vector in KeyListModule
        keyListModule->setKeyListVector(keyList.getVector());

        scheduleAt(simTime() + keyListDelay, sendKeyList_timer);

        delete putMsg;
    } else if (dynamic_cast<GIAsearch*>(msg) != NULL) {
        if (state == READY) {
            GIAsearch* getMsg = check_and_cast<GIAsearch*>(msg);
            SearchMessage* searchMsg = new SearchMessage("SEARCH");
            searchMsg->setCommand(SEARCH);
            searchMsg->setStatType(APP_DATA_STAT);
            searchMsg->setHopCount(maxHopCount);
            searchMsg->setDestKey(getMsg->getSearchKey());
            searchMsg->setSrcNode(thisGiaNode);
            searchMsg->setSrcCapacity(thisGiaNode.getCapacity());
            searchMsg->setSrcDegree(connectionDegree);
            searchMsg->setSearchKey(getMsg->getSearchKey());
            searchMsg->setMaxResponses(getMsg->getMaxResponses());
            searchMsg->setReversePathArraySize(0);
            searchMsg->setID(OverlayKey::random());
            searchMsg->setBitLength(SEARCH_L(searchMsg));

            processSearchMessage(searchMsg, true);

            delete getMsg;
        } else
            delete msg;
    } else {
        delete msg;
        EV << "(Gia) unkown message from app deleted!" << endl;
    }
}


void Gia::sendSearchResponseMessage(const GiaNode& srcNode, SearchMessage* msg)
{
    // does SearchMessage->foundNode[] already contain this node
    uint32_t foundNodeArraySize = msg->getFoundNodeArraySize();
    bool containsNode = false;
    for (uint32_t i=0; i<foundNodeArraySize; i++)
        if (srcNode.getKey() == msg->getFoundNode(i))
            containsNode = true;

    if (!containsNode && msg->getMaxResponses() > 0) {
        // add this node to SearchMessage->foundNode[]
        msg->setFoundNodeArraySize(foundNodeArraySize+1);
        msg->setFoundNode(foundNodeArraySize, srcNode.getKey());

        // decrease SearchMessage->maxResponses
        msg->setMaxResponses(msg->getMaxResponses()-1);

        // get first node in reverse-path
        uint32_t reversePathArraySize = msg->getReversePathArraySize();

        if (reversePathArraySize == 0) {
            // we have the key
            // deliver response to application
            SearchResponseMessage* responseMsg =
                new SearchResponseMessage("ANSWER");
            responseMsg->setCommand(ANSWER);
            responseMsg->setStatType(APP_DATA_STAT);
            responseMsg->setHopCount(maxHopCount);
            responseMsg->setSrcNode(thisGiaNode);
            responseMsg->setSrcCapacity(thisGiaNode.getCapacity());
            responseMsg->setSrcDegree(connectionDegree);
            responseMsg->setSearchKey(msg->getSearchKey());
            responseMsg->setFoundNode(srcNode);
            responseMsg->setID(OverlayKey::random());
            responseMsg->setSearchHopCount(0);

            responseMsg->setBitLength(SEARCHRESPONSE_L(responseMsg));

            deliverSearchResult(responseMsg);
        } else {
            uint32_t reversePathArraySize(msg->getReversePathArraySize());
            SearchResponseMessage* responseMsg =
                new SearchResponseMessage("ANSWER");
            responseMsg->setCommand(ANSWER);
            responseMsg->setHopCount(maxHopCount);
            responseMsg->setSrcNode(srcNode);
            responseMsg->setSrcCapacity(srcNode.getCapacity());
            responseMsg->setSrcDegree(connectionDegree);
            responseMsg->setSearchKey(msg->getSearchKey());
            responseMsg->setFoundNode(srcNode);
            responseMsg->setReversePathArraySize(reversePathArraySize);
            for (uint32_t i=0; i<reversePathArraySize; i++)
                responseMsg->setReversePath(i, msg->getReversePath(i));
            responseMsg->setID(OverlayKey::random());
            responseMsg->setSearchHopCount(reversePathArraySize);
            responseMsg->setBitLength(SEARCHRESPONSE_L(responseMsg));

            forwardSearchResponseMessage(responseMsg);
        }
    }
}


void Gia::processSearchMessage(SearchMessage* msg, bool fromApplication)
{
    OverlayKey searchKey = msg->getSearchKey();

    if (keyList.contains(searchKey)) {
        // this node contains search key
        sendSearchResponseMessage(thisGiaNode, msg);
    }

    // check if neighbors contain search key
    for (uint32_t i = 0; i < neighbors->getSize(); i++) {
        GiaKeyList* keyList = neighbors->getNeighborKeyList(neighbors->get(i));
        if (keyList->contains(searchKey))
            sendSearchResponseMessage(neighbors->get(i), msg);
    }

    // forward search-message to next hop
    if (msg->getMaxResponses() > 0) {
        // actualize reverse path
        uint32_t reversePathSize = msg->getReversePathArraySize();

        if (optimizeReversePath)
            for (uint32_t i=0; i<reversePathSize; i++) {
                if (msg->getReversePath(i) == thisGiaNode.getKey()) {
                    // Our node already in ReversePath.
                    // Delete successor nodes from ReversePath
                    msg->setBitLength(msg->getBitLength() - (reversePathSize - i)*KEY_L);
                    reversePathSize = i; // set new array size
                    break;
                }
            }

        msg->setReversePathArraySize(reversePathSize+1);
        msg->setReversePath(reversePathSize, thisGiaNode.getKey());
        msg->setBitLength(msg->getBitLength() + KEY_L);

        forwardMessage(msg, fromApplication);
    } else {
        tokenFactory->grantToken();
        delete msg;
    }
}

void Gia::deliverSearchResult(SearchResponseMessage* msg)
{
    OverlayCtrlInfo* overlayCtrlInfo = new OverlayCtrlInfo();
    overlayCtrlInfo->setHopCount(msg->getSearchHopCount());
    overlayCtrlInfo->setSrcNode(msg->getSrcNode());
    overlayCtrlInfo->setTransportType(ROUTE_TRANSPORT);

    GIAanswer* deliverMsg = new GIAanswer("GIA-Answer");
    deliverMsg->setCommand(GIA_ANSWER);
    deliverMsg->setControlInfo(overlayCtrlInfo);
    deliverMsg->setSearchKey(msg->getSearchKey());
    deliverMsg->setNode(msg->getFoundNode());
    deliverMsg->setBitLength(GIAGETRSP_L(msg));

    send(deliverMsg, "appOut");
    if (debugOutput)
        EV << "(Gia) Deliver search response " << msg
           << " to application at " << thisGiaNode << endl;

    delete msg;
}

void Gia::finishOverlay()
{
    // statistics

    globalStatistics->addStdDev("GIA: JOIN-Messages Count", stat_joinCount);
    globalStatistics->addStdDev("GIA: JOIN Bytes sent", stat_joinBytesSent);
    globalStatistics->addStdDev("GIA: JOIN::REQ Messages", stat_joinREQ);
    globalStatistics->addStdDev("GIA: JOIN::REQ Bytes sent", stat_joinREQBytesSent);
    globalStatistics->addStdDev("GIA: JOIN::RSP Messages", stat_joinRSP);
    globalStatistics->addStdDev("GIA: JOIN::RSP Bytes sent", stat_joinRSPBytesSent);
    globalStatistics->addStdDev("GIA: JOIN::ACK Messages", stat_joinACK);
    globalStatistics->addStdDev("GIA: JOIN::ACK Bytes sent", stat_joinACKBytesSent);
    globalStatistics->addStdDev("GIA: JOIN::DNY Messages", stat_joinDNY);
    globalStatistics->addStdDev("GIA: JOIN::DNY Bytes sent", stat_joinDNYBytesSent);
    globalStatistics->addStdDev("GIA: DISCONNECT:IND Messages", stat_disconnectMessages);
    globalStatistics->addStdDev("GIA: DISCONNECT:IND Bytes sent",
                                stat_disconnectMessagesBytesSent);
    globalStatistics->addStdDev("GIA: UPDATE:IND Messages", stat_updateMessages);
    globalStatistics->addStdDev("GIA: UPDATE:IND Bytes sent", stat_updateMessagesBytesSent);
    globalStatistics->addStdDev("GIA: TOKEN:IND Messages", stat_tokenMessages);
    globalStatistics->addStdDev("GIA: TOKEN:IND Bytes sent", stat_tokenMessagesBytesSent);
    globalStatistics->addStdDev("GIA: KEYLIST:IND Messages", stat_keyListMessages);
    globalStatistics->addStdDev("GIA: KEYLIST:IND Bytes sent", stat_keyListMessagesBytesSent);
    globalStatistics->addStdDev("GIA: ROUTE:IND Messages", stat_routeMessages);
    globalStatistics->addStdDev("GIA: ROUTE:IND Bytes sent", stat_routeMessagesBytesSent);
    globalStatistics->addStdDev("GIA: Neighbors max", stat_maxNeighbors);
    globalStatistics->addStdDev("GIA: Neighbors added", stat_addedNeighbors);
    globalStatistics->addStdDev("GIA: Neighbors removed", stat_removedNeighbors);
    globalStatistics->addStdDev("GIA: Level of satisfaction messages ",
                                stat_numSatisfactionMessages);
    globalStatistics->addStdDev("GIA: Level of satisfaction max ",stat_maxLevelOfSatisfaction);
    globalStatistics->addStdDev("GIA: Level of satisfaction avg ",
                                stat_sumLevelOfSatisfaction / stat_numSatisfactionMessages);
    bootstrapList->removeBootstrapNode(thisGiaNode);
}

Gia::~Gia()
{
    cancelAndDelete(satisfaction_timer);
    cancelAndDelete(update_timer);
    cancelAndDelete(timedoutMessages_timer);
    cancelAndDelete(timedoutNeighbors_timer);
    cancelAndDelete(sendKeyList_timer);
    cancelAndDelete(sendToken_timer);
    delete msgBookkeepingList;
}

