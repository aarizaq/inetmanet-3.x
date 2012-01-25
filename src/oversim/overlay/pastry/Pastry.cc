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
 * @file Pastry.cc
 * @author Felix Palmen
 * @author Gerhard Petruschat
 * @author Bernhard Heep
 */

#include <cassert>

#include <IPvXAddress.h>
#include <IInterfaceTable.h>
#include <IPv4InterfaceData.h>
#include <RpcMacros.h>
#include <InitStages.h>
#include <GlobalStatistics.h>

#include "Pastry.h"


Define_Module(Pastry);

Pastry::~Pastry()
{
    // destroy self timer messages
    cancelAndDelete(readyWait);
    cancelAndDelete(joinUpdateWait);
    cancelAndDelete(secondStageWait);
    if (useDiscovery) cancelAndDelete(discoveryTimeout);
    if (routingTableMaintenanceInterval > 0) cancelAndDelete(repairTaskTimeout);

    clearVectors();
}

void Pastry::clearVectors()
{
    // purge pending state messages
    if (!stReceived.empty()) {
        for (std::vector<PastryStateMsgHandle>::iterator it =
            stReceived.begin(); it != stReceived.end(); it++) {
            // check whether one of the pointers is a duplicate of stateCache
            if (it->msg == stateCache.msg) stateCache.msg = NULL;
            if (it->prox == stateCache.prox) stateCache.prox = NULL;
            delete it->msg;
            delete it->prox;
        }
        stReceived.clear();
        stReceivedPos = stReceived.end();
    }

    // purge notify list:
    notifyList.clear();
}

void Pastry::purgeVectors(void)
{
    clearVectors();

    BasePastry::purgeVectors();
}

void Pastry::initializeOverlay(int stage)
{
    if ( stage != MIN_STAGE_OVERLAY )
        return;

    // Pastry provides KBR services
    kbr = true;

    baseInit();

    useDiscovery = par("useDiscovery");
    pingBeforeSecondStage = par("pingBeforeSecondStage");
    secondStageInterval = par("secondStageWait");
    discoveryTimeoutAmount = par("discoveryTimeoutAmount");
    routingTableMaintenanceInterval = par("routingTableMaintenanceInterval");
    sendStateAtLeafsetRepair = par("sendStateAtLeafsetRepair");
    partialJoinPath = par("partialJoinPath");
    readyWaitAmount = par("readyWait");
    minimalJoinState = par("minimalJoinState");

    overrideOldPastry = par("overrideOldPastry");
    overrideNewPastry = par("overrideNewPastry");

    if (overrideOldPastry) {
        //useSecondStage = true;
        //secondStageInterval = ???;
        useDiscovery = false;
        sendStateAtLeafsetRepair = true;
        routingTableMaintenanceInterval = 0;
    }

    if (overrideNewPastry) {
        //useSecondStage = false;
        secondStageInterval = 0;
        useDiscovery = true;
        discoveryTimeoutAmount = 0.4;
        routingTableMaintenanceInterval = 60;
        sendStateAtLeafsetRepair = false;
    }

    joinTimeout = new cMessage("joinTimeout");
    readyWait = new cMessage("readyWait");
    secondStageWait = new cMessage("secondStageWait");
    joinUpdateWait = new cMessage("joinUpdateWait");

    discoveryTimeout =
        (useDiscovery ? new cMessage("discoveryTimeout") : NULL);
    repairTaskTimeout =
        ((routingTableMaintenanceInterval > 0) ?
                new cMessage("repairTaskTimeout") : NULL);

    updateCounter = 0;
}

void Pastry::joinOverlay()
{
    changeState(INIT);

    if (bootstrapNode.isUnspecified()) {
        // no existing pastry network -> first node of a new one
        changeState(READY);
    } else {
        // join existing pastry network
        nearNode = bootstrapNode;
        if (useDiscovery) changeState(DISCOVERY);
        else changeState(JOINING_2);
    }
}

void Pastry::changeState(int toState)
{
    if (readyWait->isScheduled()) cancelEvent(readyWait);
    baseChangeState(toState);

    switch (toState) {
    case INIT:
        cancelAllRpcs();
        purgeVectors();
        break;

    case DISCOVERY:
        state = DISCOVERY;
        //nearNode = bootstrapNode;
        nearNodeRtt = MAXTIME;
        pingNode(bootstrapNode, discoveryTimeoutAmount, 0,
                 NULL, "PING bootstrapNode in discovery mode",
                 NULL, PING_DISCOVERY, UDP_TRANSPORT); //TODO
        sendRequest(bootstrapNode, PASTRY_REQ_LEAFSET); //TODO should be an RPC
        depth = -1;

        // schedule join timer for discovery algorithm
        cancelEvent(joinTimeout);
        scheduleAt(simTime() + joinTimeoutAmount, joinTimeout);

        break;

    case JOINING_2: {
        joinHopCount = 0;
        PastryJoinMessage* msg = new PastryJoinMessage("JOIN-Request");
        //TODO add timestamp to join msg
        msg->setPastryMsgType(PASTRY_MSG_JOIN);
        msg->setStatType(MAINTENANCE_STAT);
        msg->setSendStateTo(thisNode);
        msg->setBitLength(PASTRYJOIN_L(msg));
        RECORD_STATS(joinSent++; joinBytesSent += msg->getByteLength());
        std::vector<TransportAddress> sourceRoute;
        sourceRoute.push_back(nearNode);
        sendToKey(thisNode.getKey(), msg, 0/*1*/, sourceRoute);
    }
    break;

    case READY:
        // determine list of all known nodes as notifyList
        notifyList.clear();
        leafSet->dumpToVector(notifyList);
        routingTable->dumpToVector(notifyList);
        sort(notifyList.begin(), notifyList.end());
        notifyList.erase(unique(notifyList.begin(), notifyList.end()),
                         notifyList.end());

        // schedule update
        cancelEvent(joinUpdateWait);
        scheduleAt(simTime() + 0.0001, joinUpdateWait);

        // schedule second stage
        if (secondStageInterval > 0) {
            cancelEvent(secondStageWait);
            scheduleAt(simTime() + secondStageInterval, secondStageWait);
        }

        // schedule routing table maintenance task
        if (routingTableMaintenanceInterval > 0) {
            cancelEvent(repairTaskTimeout);
            scheduleAt(simTime() + routingTableMaintenanceInterval, repairTaskTimeout);
        }

        break;
    }
}


void Pastry::pingResponse(PingResponse* pingResponse,
                          cPolymorphic* context, int rpcId,
                          simtime_t rtt)
{
    if (state == DISCOVERY) {
        EV << "[Pastry::pingResponse() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Pong (or Ping-context from NeighborCache) received (from "
           << pingResponse->getSrcNode().getIp() << ") in DISCOVERY mode"
           << endl;

        if (nearNodeRtt > rtt) {
            nearNode = pingResponse->getSrcNode();
            nearNodeRtt = rtt;
            nearNodeImproved = true;
        }
    } else {
        BasePastry::pingResponse(pingResponse, context, rpcId, rtt);
    }
}


void Pastry::handleTimerEvent(cMessage* msg)
{

    if (msg == joinTimeout) {
        EV << "[Pastry::handleTimerEvent() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    timeout expired, restarting..."
           << endl;
        join();
    } else if (msg == readyWait) {
        if (partialJoinPath) {
            RECORD_STATS(joinPartial++);
            sort(stReceived.begin(), stReceived.end(), stateMsgIsSmaller);

            // start pinging the nodes found in the first state message:
            stReceivedPos = stReceived.begin();
            stateCache = *stReceivedPos;
            EV << "[Pastry::handleTimerEvent() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    joining despite some missing STATE messages."
               << endl;
            processState();
        } else {
            EV << "[Pastry::handleTimerEvent() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    timeout waiting for missing state messages"
               << " in JOIN state, restarting..."
               << endl;
            joinOverlay();
        }
    } else if (msg == joinUpdateWait) {
        EV << "[Pastry::handleTimerEvent() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    sending state updates to all nodes."
           << endl;
        doJoinUpdate();
    } else if (msg == secondStageWait) {
        EV << "[Pastry::handleTimerEvent() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    sending STATE requests to all nodes in"
           << " second stage of initialization."
           << endl;
        doSecondStage();
    } else if (msg->isName("sendStateWait")) {
        PastrySendState* sendStateMsg = check_and_cast<PastrySendState*>(msg);

        std::vector<PastrySendState*>::iterator pos =
            std::find(sendStateWait.begin(), sendStateWait.end(),
                      sendStateMsg);
        if (pos != sendStateWait.end()) sendStateWait.erase(pos);

        sendStateTables(sendStateMsg->getDest());
        delete sendStateMsg;
    } else if (msg == discoveryTimeout) {
        if ((depth == 0) && (nearNodeImproved)) {
            depth++; //repeat last step if closer node was found
        }
        if ((depth == 0) || (pingedNodes < 1)) {
            changeState(JOINING_2);
        } else {
            PastryRoutingRowRequestMessage* msg =
                new PastryRoutingRowRequestMessage("ROWREQ");
            msg->setPastryMsgType(PASTRY_MSG_ROWREQ);
            msg->setStatType(MAINTENANCE_STAT);
            msg->setSendStateTo(thisNode);
            msg->setRow(depth);
            msg->setBitLength(PASTRYRTREQ_L(msg));
            RECORD_STATS(routingTableReqSent++;
                         routingTableReqBytesSent += msg->getByteLength());
            sendMessageToUDP(nearNode, msg);
        }
    } else if (msg == repairTaskTimeout) {
        EV << "[Pastry::handleTimerEvent() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    starting routing table maintenance"
           << endl;
        doRoutingTableMaintenance();
        scheduleAt(simTime() + routingTableMaintenanceInterval,
                   repairTaskTimeout);
    }
}

void Pastry::handleUDPMessage(BaseOverlayMessage* msg)
{
    PastryMessage* pastryMsg = check_and_cast<PastryMessage*>(msg);
    uint32_t type = pastryMsg->getPastryMsgType();

    if (debugOutput) {
        EV << "[Pastry::handleUDPMessage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    incoming message of type ";
        switch(type) {
        case PASTRY_MSG_STD:
            EV << "PASTRY_MSG_STD";
            break;
        case PASTRY_MSG_JOIN:
            EV << "PASTRY_MSG_JOIN";
            break;
        case PASTRY_MSG_STATE:
            EV << "PASTRY_MSG_STATE";
            break;
        case PASTRY_MSG_LEAFSET:
            EV << "PASTRY_MSG_LEAFSET";
            break;
        case PASTRY_MSG_ROWREQ:
            EV << "PASTRY_MSG_ROWREQ";
            break;
        case PASTRY_MSG_RROW:
            EV << "PASTRY_MSG_RROW";
            break;
        case PASTRY_MSG_REQ:
            EV << "PASTRY_MSG_REQ";
            break;
        default:
            EV << "UNKNOWN (" << type <<")";
            break;
        }
        EV << endl;
    }

    switch (type) {
    case PASTRY_MSG_STD:
        opp_error("Pastry received PastryMessage of unknown type!");
        break;

    case PASTRY_MSG_JOIN: {
        PastryJoinMessage* jmsg =
            check_and_cast<PastryJoinMessage*>(pastryMsg);
        RECORD_STATS(joinReceived++; joinBytesReceived +=
                     jmsg->getByteLength());
        if (state != READY) {
            if (jmsg->getSendStateTo() == thisNode) {
                EV << "[Pastry::handleUDPMessage() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    PastryJoinMessage received by originator!"
                   << endl;
            } else {
                EV << "[Pastry::handleUDPMessage() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    received join message before reaching "
                   << "READY state, dropping message!"
                   << endl;
            }
        }
        else if (jmsg->getSendStateTo() == thisNode) {
            EV << "[Pastry::handleUDPMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    PastryJoinMessage gets dropped because it is "
               << "outdated and has been received by originator!"
               << endl;
        } else {
            OverlayCtrlInfo* overlayCtrlInfo
                = check_and_cast<OverlayCtrlInfo*>(jmsg->getControlInfo());

            uint32_t joinHopCount =  overlayCtrlInfo->getHopCount();
            if ((joinHopCount > 1) &&
                ((defaultRoutingType == ITERATIVE_ROUTING) ||
                 (defaultRoutingType == EXHAUSTIVE_ITERATIVE_ROUTING)))
                 joinHopCount--;

            // remove node from state if it is rejoining
            handleFailedNode(jmsg->getSendStateTo());

            sendStateTables(jmsg->getSendStateTo(),
                            (minimalJoinState ?
                              PASTRY_STATE_MINJOIN : PASTRY_STATE_JOIN),
                             joinHopCount, true);
        }

        delete jmsg;
    }
    break;

    case PASTRY_MSG_LEAFSET: {
        PastryLeafsetMessage* lmsg =
            check_and_cast<PastryLeafsetMessage*>(pastryMsg);
        RECORD_STATS(leafsetReceived++; leafsetBytesReceived +=
            lmsg->getByteLength());

        if (state == DISCOVERY) {
            uint32_t lsSize = lmsg->getLeafSetArraySize();
            const NodeHandle* node;
            pingedNodes = 0;

            for (uint32_t i = 0; i < lsSize; i++) {
                node = &(lmsg->getLeafSet(i));
                // unspecified nodes not considered
                if ( !(node->isUnspecified()) ) {
                    pingNode(*node, discoveryTimeoutAmount, 0,
                             NULL, "PING received leafs for nearest node",
                             NULL, -1, UDP_TRANSPORT);//TODO
                    pingedNodes++;
               }
            }

            EV << "[Pastry::handleUDPMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    received leafset, waiting for pings"
              << endl;

            if (discoveryTimeout->isScheduled()) cancelEvent(discoveryTimeout);
            scheduleAt(simTime() + discoveryTimeoutAmount, discoveryTimeout);
            delete lmsg;
        } else if (state == READY) {
            handleLeafsetMessage(lmsg, false);
        } else {
            delete lmsg;
        }
    }
    break;

    case PASTRY_MSG_ROWREQ: {
        PastryRoutingRowRequestMessage* rtrmsg =
            check_and_cast<PastryRoutingRowRequestMessage*>(pastryMsg);
        RECORD_STATS(routingTableReqReceived++; routingTableReqBytesReceived +=
            rtrmsg->getByteLength());
        if (state == READY)
            if (rtrmsg->getRow() == -1)
                sendRoutingRow(rtrmsg->getSendStateTo(), routingTable->getLastRow());
                else if (rtrmsg->getRow() > routingTable->getLastRow())
                    EV << "[Pastry::handleUDPMessage() @ " << thisNode.getIp()
                       << " (" << thisNode.getKey().toString(16) << ")]\n"
                       << "    received request for nonexistent routing"
                       << "table row, dropping message!"
                       << endl;
                else sendRoutingRow(rtrmsg->getSendStateTo(), rtrmsg->getRow());
        else
            EV << "[Pastry::handleUDPMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    received routing table request before reaching "
               << "READY state, dropping message!"
               << endl;
        delete rtrmsg;
    }
    break;

    case PASTRY_MSG_RROW: {
        PastryRoutingRowMessage* rtmsg =
            check_and_cast<PastryRoutingRowMessage*>(pastryMsg);
        RECORD_STATS(routingTableReceived++; routingTableBytesReceived +=
            rtmsg->getByteLength());

        if (state == DISCOVERY) {
            uint32_t nodesPerRow = rtmsg->getRoutingTableArraySize();
            const NodeHandle* node;
            if (depth == -1) {
                depth = rtmsg->getRow();
            }
            pingedNodes = 0;
            nearNodeImproved = false;

            if (depth > 0) {
                for (uint32_t i = 0; i < nodesPerRow; i++) {
                    node = &(rtmsg->getRoutingTable(i));
                    // unspecified nodes not considered
                    if ( !(node->isUnspecified()) ) {
                        // we look for best connection here, so Timeout is short and there are no retries
                        pingNode(*node, discoveryTimeoutAmount, 0, NULL,
                                 "PING received routing table for nearest node",
                                 NULL, -1, UDP_TRANSPORT); //TODO
                        pingedNodes++;
                    }
                }
                depth--;
            }
            EV << "[Pastry::handleUDPMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    received routing table, waiting for pings"
               << endl;
            if (discoveryTimeout->isScheduled()) {
                cancelEvent(discoveryTimeout);
            }
            scheduleAt(simTime() + discoveryTimeoutAmount, discoveryTimeout);
        }

        else if (state == READY) {

            uint32_t nodesPerRow = rtmsg->getRoutingTableArraySize();
            PastryStateMessage* stateMsg;

            stateMsg = new PastryStateMessage("STATE");
            stateMsg->setTimestamp(rtmsg->getTimestamp());
            stateMsg->setPastryMsgType(PASTRY_MSG_STATE);
            stateMsg->setStatType(MAINTENANCE_STAT);
            stateMsg->setSender(rtmsg->getSender());
            stateMsg->setLeafSetArraySize(0);
            stateMsg->setNeighborhoodSetArraySize(0);
            stateMsg->setRoutingTableArraySize(nodesPerRow);

            for (uint32_t i = 0; i < nodesPerRow; i++) {
                stateMsg->setRoutingTable(i, rtmsg->getRoutingTable(i));
            }

            handleStateMessage(stateMsg);
        }

        delete rtmsg;
    }
    break;

    case PASTRY_MSG_REQ: {
        PastryRequestMessage* lrmsg =
            check_and_cast<PastryRequestMessage*>(pastryMsg);
        handleRequestMessage(lrmsg);
    }
    break;

    case PASTRY_MSG_STATE: {
        PastryStateMessage* stateMsg =
            check_and_cast<PastryStateMessage*>(msg);
        RECORD_STATS(stateReceived++; stateBytesReceived +=
                     stateMsg->getByteLength());
        handleStateMessage(stateMsg);
    }
    break;
    }
}


bool Pastry::recursiveRoutingHook(const TransportAddress& dest,
                                  BaseRouteMessage* msg)
{
    if (dest == thisNode) {
        return true;
    }

    PastryMessage* pmsg =
        dynamic_cast<PastryMessage*>(msg->getEncapsulatedPacket());

    if (pmsg && pmsg->getPastryMsgType() == PASTRY_MSG_JOIN) {
        PastryJoinMessage* jmsg = static_cast<PastryJoinMessage*>(pmsg);
        if (jmsg->getSendStateTo() != thisNode) {
            RECORD_STATS(joinSeen++; joinBytesSeen += jmsg->getByteLength());
            // remove node from state if it is rejoining
            handleFailedNode(jmsg->getSendStateTo());

            sendStateTables(jmsg->getSendStateTo(),
                            minimalJoinState ?
                            PASTRY_STATE_MINJOIN : PASTRY_STATE_JOIN,
                            check_and_cast<OverlayCtrlInfo*>(msg->getControlInfo())
                            ->getHopCount(), false);
        }
    }

    // forward now:
    return true;
}

void Pastry::iterativeJoinHook(BaseOverlayMessage* msg, bool incrHopCount)
{
    PastryFindNodeExtData* findNodeExt = NULL;
    if (msg && msg->hasObject("findNodeExt")) {
        findNodeExt =
            check_and_cast<PastryFindNodeExtData*>(msg->
                    getObject("findNodeExt"));
    }
    // Send state tables on any JOIN message we see:
    if (findNodeExt) {
        const TransportAddress& stateRecipient =
            findNodeExt->getSendStateTo();
        if (!stateRecipient.isUnspecified()) {
            RECORD_STATS(joinSeen++);
            sendStateTables(stateRecipient,
                            minimalJoinState ?
                            PASTRY_STATE_MINJOIN : PASTRY_STATE_JOIN,
                            findNodeExt->getJoinHopCount(), false);
        }
        if (incrHopCount) {
            findNodeExt->setJoinHopCount(findNodeExt->getJoinHopCount() + 1);
        }
    }
}


void Pastry::doJoinUpdate(void)
{
    // send "update" state message to all nodes who sent us their state
    // during INIT, remove these from notifyList so they don't get our
    // state twice
    std::vector<TransportAddress>::iterator nListPos;
    if (!stReceived.empty()) {
        for (std::vector<PastryStateMsgHandle>::iterator it =
                 stReceived.begin(); it != stReceived.end(); ++it) {
            simtime_t timestamp = it->msg->getTimestamp();
            sendStateTables(it->msg->getSender(), PASTRY_STATE_UPDATE,
                            &timestamp);
            nListPos = find(notifyList.begin(), notifyList.end(),
                            it->msg->getSender());
            if (nListPos != notifyList.end()) {
                notifyList.erase(nListPos);
            }
            delete it->msg;
            delete it->prox;
        }
        stReceived.clear();
    }

    // send a normal STATE message to all remaining known nodes
    for (std::vector<TransportAddress>::iterator it =
             notifyList.begin(); it != notifyList.end(); it++) {
        if (*it != thisNode) sendStateTables(*it, PASTRY_STATE_JOINUPDATE);
    }
    notifyList.clear();

    updateTooltip();
}

void Pastry::doSecondStage(void)
{
    getParentModule()->getParentModule()->bubble("entering SECOND STAGE");

    // probe nodes in local state
    if (leafSet->isValid()) {
        PastryStateMessage* stateMsg = new PastryStateMessage("STATE");
        stateMsg->setPastryMsgType(PASTRY_MSG_STATE);
        stateMsg->setStatType(MAINTENANCE_STAT);
        stateMsg->setPastryStateMsgType(PASTRY_STATE_STD);
        stateMsg->setSender(thisNode);
        routingTable->dumpToStateMessage(stateMsg);
        leafSet->dumpToStateMessage(stateMsg);
        neighborhoodSet->dumpToStateMessage(stateMsg);
        //stateMsg->setBitLength(PASTRYSTATE_L(stateMsg));
        PastryStateMsgHandle handle(stateMsg);

        if (!stateCache.msg) {
            stateCache = handle;
            processState();
        } else {
            stateCacheQueue.push(handle);
            prePing(stateMsg);
        }
    }

    // "second stage" for locality:
    notifyList.clear();
    routingTable->dumpToVector(notifyList);
    neighborhoodSet->dumpToVector(notifyList);
    sort(notifyList.begin(), notifyList.end());
    notifyList.erase(unique(notifyList.begin(), notifyList.end()),
                     notifyList.end());
    for (std::vector<TransportAddress>::iterator it = notifyList.begin();
         it != notifyList.end(); it++) {
        if (*it == thisNode) continue;
        EV << "[Pastry::doSecondStage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    second stage: requesting state from " << *it
           << endl;
        sendRequest(*it, PASTRY_REQ_STATE);
    }
    notifyList.clear();
}


void Pastry::doRoutingTableMaintenance()
{
    for (int i = 0; i < routingTable->getLastRow(); i++) {
        const TransportAddress& ask4row = routingTable->getRandomNode(i);

        if ((!ask4row.isUnspecified()) && (ask4row != thisNode)) {
            PastryRoutingRowRequestMessage* msg =
                new PastryRoutingRowRequestMessage("ROWREQ");
            msg->setPastryMsgType(PASTRY_MSG_ROWREQ);
            msg->setStatType(MAINTENANCE_STAT);
            msg->setSendStateTo(thisNode);
            msg->setRow(i + 1);
            msg->setBitLength(PASTRYRTREQ_L(msg));

            RECORD_STATS(routingTableReqSent++;
                         routingTableReqBytesSent += msg->getByteLength());

            sendMessageToUDP(ask4row, msg);
        } else {
            EV << "[Pastry::doRoutingTableMaintenance() @ "
               << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    could not send Message to Node in Row" << i
               << endl;
        }
    }
}


bool Pastry::handleFailedNode(const TransportAddress& failed)
{
    if (state != READY) {
        return false;
    }
    bool wasValid = leafSet->isValid();

    //std::cout << thisNode.getIp() << " is handling failed node: "
    //          << failed.getIp() << std::endl;
    if (failed.isUnspecified())
        opp_error("Pastry::handleFailedNode(): failed is unspecified!");

    const TransportAddress& lsAsk = leafSet->failedNode(failed);
    const TransportAddress& rtAsk = routingTable->failedNode(failed);
    neighborhoodSet->failedNode(failed);

    if (! lsAsk.isUnspecified()) {
        newLeafs();
        if (sendStateAtLeafsetRepair) sendRequest(lsAsk, PASTRY_REQ_REPAIR);
        else sendRequest(lsAsk, PASTRY_REQ_LEAFSET);
    }
    if (! rtAsk.isUnspecified() &&
        (lsAsk.isUnspecified() ||
         lsAsk != rtAsk)) sendRequest(rtAsk, PASTRY_REQ_REPAIR);

    if (wasValid && lsAsk.isUnspecified() && (! leafSet->isValid())) {
        EV << "[Pastry::handleFailedNode() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    lost connection to the network, trying to re-join."
           << endl;
        //std::cout << thisNode.getIp()
        //          << " Pastry: lost connection to the network, trying to re-join."
        //          << std::endl;
        join();
        return false;
    }

    return true;
}

void Pastry::checkProxCache(void)
{
    EV << "[Pastry::checkProxCache() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]"
       << endl;

    // no cached STATE message?
    assert(stateCache.msg || !stateCache.prox);
    if (!stateCache.msg) return;

    // no entries in stateCache.prox?
    if (stateCache.prox->pr_rt.empty() &&
        stateCache.prox->pr_ls.empty() &&
        stateCache.prox->pr_ns.empty())
        throw new cRuntimeError("ERROR in Pastry: stateCache.prox empty!");

    /*
    //debug
    for (uint i = 0; i < stateCache.prox->pr_rt.size(); ++i) {
        if (stateCache.prox->pr_rt[i] == -3)
            EV << stateCache.msg->getRoutingTable(i).getIp() << " ";
    }
    for (uint i = 0; i < stateCache.prox->pr_ls.size(); ++i) {
        if (stateCache.prox->pr_ls[i] == -3)
            EV << stateCache.msg->getLeafSet(i).getIp() << " ";
    }
    for (uint i = 0; i < stateCache.prox->pr_ns.size(); ++i) {
        if (stateCache.prox->pr_ns[i] == -3)
            EV << stateCache.msg->getNeighborhoodSet(i).getIp() << " ";
    }
    EV << endl;
     */

    // some entries not yet determined?
    if ((find(stateCache.prox->pr_rt.begin(), stateCache.prox->pr_rt.end(),
        PASTRY_PROX_PENDING) != stateCache.prox->pr_rt.end()) ||
        (find(stateCache.prox->pr_ls.begin(), stateCache.prox->pr_ls.end(),
         PASTRY_PROX_PENDING) != stateCache.prox->pr_ls.end()) ||
        (find(stateCache.prox->pr_ns.begin(), stateCache.prox->pr_ns.end(),
         PASTRY_PROX_PENDING) != stateCache.prox->pr_ns.end())) {
        //std::cout << "pending" << std::endl;
        return;
    }

    EV << "[Pastry::checkProxCache() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]\n"
       << "    all proximities for current STATE message from "
       << stateCache.msg->getSender().getIp()
       << " collected!"
       << endl;
    /*
    //debug
    if (stateCache.prox != NULL) {
        std::vector<PastryStateMsgHandle>::iterator it;
        for (it = stReceived.begin(); it != stReceived.end(); ++it) {
            if (it->prox == NULL) {
                EV << ". " << endl;
                continue;
            }
            for (uint i = 0; i < it->prox->pr_rt.size(); ++i) {
                EV << it->prox->pr_rt[i] << " ";
            }
            for (uint i = 0; i < it->prox->pr_ls.size(); ++i) {
                EV << it->prox->pr_ls[i] << " ";
            }
            for (uint i = 0; i < it->prox->pr_ns.size(); ++i) {
                EV << it->prox->pr_ns[i] << " ";
            }
            EV << endl;
        }
        EV << endl;
    } else EV << "NULL" << endl;
*/

    simtime_t now = simTime();

    if (state == JOINING_2) {
        // save pointer to proximity vectors (it is NULL until now):
        stReceivedPos->prox = stateCache.prox;

        // collected proximities for all STATE messages?
        if (++stReceivedPos == stReceived.end()) {
            EV << "[Pastry::checkProxCache() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    proximities for all STATE messages collected!"
               << endl;
            stateCache.msg = NULL;
            stateCache.prox = NULL;
            if (debugOutput) {
                EV << "[Pastry::checkProxCache() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    [JOIN] starting to build own state from "
                   << stReceived.size() << " received state messages..."
                   << endl;
            }
            if (mergeState()) {
                changeState(READY);
                EV << "[Pastry::checkProxCache() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    changeState(READY) called"
                   << endl;
            } else {
                EV << "[Pastry::checkProxCache() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Error initializing while joining! Restarting ..."
                   << endl;
                joinOverlay();
            }

        } else {
            EV << "[Pastry::checkProxCache() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    NOT all proximities for all STATE messages collected!"
               << endl;
            /*
            for (uint32_t i = 0; i < stReceived.size(); ++i) {
                EV << ((i == 0) ? "    " : " | ");
                std::cout << ((i == 0) ? "    " : " | ");
                if (stReceived[i].msg == stReceivedPos->msg) {
                    EV << "*";
                    std::cout << "*";
                }
                EV << stReceived[i].msg << " " << stReceived[i].prox;
                std::cout << stReceived[i].msg << " " << stReceived[i].prox;
            }
            EV << endl;
            std::cout << std::endl;
             */

            // process next state message in vector:
            if (stReceivedPos->msg == NULL)
                throw cRuntimeError("stReceivedPos->msg = NULL");
            stateCache = *stReceivedPos;
            if (stateCache.msg == NULL)
                throw cRuntimeError("msg = NULL");
            processState();
        }
    } else {
        // state == READY
        if (stateCache.msg->getPastryStateMsgType() == PASTRY_STATE_REPAIR) {
            // try to repair routingtable based on repair message:
            const TransportAddress& askRt =
                routingTable->repair(stateCache.msg, stateCache.prox);
            if (! askRt.isUnspecified()) {
                sendRequest(askRt, PASTRY_REQ_REPAIR);
            }

            // while not really known, it's safe to assume that a repair
            // message changed our state:
            lastStateChange = now;
        } else {
            if (stateCache.outdatedUpdate) {
                // send another STATE message on outdated state update:
                updateCounter++;
                sendStateDelayed(stateCache.msg->getSender());
            } else {
                // merge info in own state tables
                // except leafset (was already handled in handleStateMessage)
                if (neighborhoodSet->mergeState(stateCache.msg, stateCache.prox))
                    lastStateChange = now;
                EV << "[Pastry::checkProxCache() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Merging nodes into routing table"
                   << endl;
                if (routingTable->mergeState(stateCache.msg, stateCache.prox)) {
                    lastStateChange = now;
                    EV << "[Pastry::checkProxCache() @ " << thisNode.getIp()
                       << " (" << thisNode.getKey().toString(16) << ")]\n"
                       << "    Merged nodes into routing table"
                       << endl;
                }
            }
        }
        updateTooltip();

        endProcessingState();
    }
}

void Pastry::endProcessingState(void)
{
    // if state message was not an update, send one back:
    if (stateCache.msg &&
        stateCache.msg->getPastryStateMsgType() != PASTRY_STATE_UPDATE &&
        (alwaysSendUpdate || lastStateChange == simTime()) &&
        thisNode != stateCache.msg->getSender()) {//hack
        simtime_t timestamp = stateCache.msg->getTimestamp();
        sendStateTables(stateCache.msg->getSender(), PASTRY_STATE_UPDATE,
                        &timestamp);
    }

    delete stateCache.msg;
    stateCache.msg = NULL;
    delete stateCache.prox;
    stateCache.prox = NULL;

    // process next queued message:
    if (! stateCacheQueue.empty()) {
        stateCache = stateCacheQueue.front();
        stateCacheQueue.pop();
        processState();
    } //TODO get rid of the delayed update messages...
    /*else {
        std::cout << thisNode.getIp() << "\t" << simTime()
                  << " all states processed ("
                  << updateCounter << ")" << std::endl;
        updateCounter = 0;
    }*/
}

bool Pastry::mergeState(void)
{
    bool ret = true;

    if (state == JOINING_2) {
        // building initial state
        if (debugOutput) {
            EV << "[Pastry::mergeState() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    [JOIN] starting to build own state from "
               << stReceived.size() << " received state messages..."
               << endl;
        }
        if (stateCache.msg &&
            stateCache.msg->getNeighborhoodSetArraySize() > 0) {
            if (debugOutput) {
                EV << "[Pastry::mergeState() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    [JOIN] initializing NeighborhoodSet from "
                   << stReceived.front().msg->getJoinHopCount() << ". hop"
                   << endl;
            }
            if (!neighborhoodSet->mergeState(stReceived.front().msg,
                                             stReceived.front().prox )) {
                EV << "[Pastry::mergeState() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Error initializing own neighborhoodSet"
                   << " while joining! Restarting ..."
                   << endl;
                ret = false;
            }
        }
        if (debugOutput) {
            EV << "[Pastry::mergeState() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    [JOIN] initializing LeafSet from "
               << stReceived.back().msg->getJoinHopCount() << ". hop"
               << endl;
        }

        //assert(!stateCache.msg || stateCache.msg->getLeafSetArraySize() > 0);

        if (!leafSet->mergeState(stReceived.back().msg,
                                 stReceived.back().prox )) {
            EV << "[Pastry::mergeState() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    Error initializing own leafSet while joining!"
               << " Restarting ..."
               << endl;
            //std::cout << "Pastry: Error initializing own leafSet while "
            //                    "joining! Restarting ..." << std::endl;
            ret = false;
        } else {
            newLeafs();
        }
        if (debugOutput) {
            EV << "[Pastry::mergeState() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    [JOIN] initializing RoutingTable from all hops"
               << endl;
        }

        assert(!stateCache.msg ||
               stateCache.msg->getRoutingTableArraySize() > 0);

        if (!routingTable->initStateFromHandleVector(stReceived)) {
            EV << "[Pastry::mergeState() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    Error initializing own routingTable while joining!"
               << " Restarting ..."
               << endl;
            //std::cout << "Pastry: Error initializing own routingTable "
            //             "while joining! Restarting ..." << std::endl;

            ret = false;
        }
    } else if (state == READY) {
        // merging single state (stateCache.msg)
        if ((stateCache.msg->getNeighborhoodSetArraySize() > 0) &&
            (!neighborhoodSet->mergeState(stateCache.msg, NULL))) {
            ret = false;
        }
        if (!leafSet->mergeState(stateCache.msg, NULL)) {
            ret = false;
        } else {
            newLeafs();
        }
        if (!routingTable->mergeState(stateCache.msg, NULL)) {
            ret = false;
        }
    }

    if (ret) lastStateChange = simTime();
    return ret;
}

void Pastry::handleStateMessage(PastryStateMessage* msg)
{
    if (debugOutput) {
        EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    new STATE message to process "
           << static_cast<void*>(msg) << " in state " <<
            ((state == READY)?"READY":((state == JOINING_2)?"JOIN":"INIT"))
           << endl;
        if (state == JOINING_2) {
            EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    ***   own joinHopCount:  " << joinHopCount << endl
               << "    ***   already received:  " << stReceived.size() << endl
               << "    ***   last-hop flag:     "
               << (msg->getLastHop() ? "true" : "false") << endl
               << "    ***   msg joinHopCount:  "
               << msg->getJoinHopCount() << endl;
        }
    }
    if (state == INIT || state == DISCOVERY) {
        EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    can't handle state messages until at least reaching JOIN state."
           << endl;
        delete msg;
        return;
    }

    PastryStateMsgHandle handle(msg);

    // in JOIN state, store all received state Messages, need them later:
    if (state == JOINING_2) {
        //std::cout << simTime() << " " << thisNode.getIp() << " "
        //          << msg->getJoinHopCount()
        //          << (msg->getLastHop() ? " *" : "") << std::endl;

        if (msg->getPastryStateMsgType() != PASTRY_STATE_JOIN) {
            delete msg;
            return;
        }

        if (joinHopCount && stReceived.size() == joinHopCount) {
            EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    Warning: dropping state message received after "
               << "all needed state messages were collected in JOIN state."
               << endl;
            delete msg;
            return;
        }

        stReceived.push_back(handle);
        if (pingBeforeSecondStage && proximityNeighborSelection) prePing(msg);

        if (msg->getLastHop()) {
            if (joinTimeout->isScheduled()) {
                //std::cout << simTime() << " " << thisNode.getIp()
                //<< " cancelEvent(joinTimeout), received:"
                //<< stReceived.size() << ", hopcount:" << joinHopCount << std::endl;
                cancelEvent(joinTimeout);
            }
            /*if (msg->getSender().getKey() == thisNode.getKey()) {
                EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Error: OverlayKey already in use, restarting!"
                   << endl;
                //std::cout << "Pastry: Error: OverlayKey already in use, restarting!"
                //                   << std::endl;
                joinOverlay();
                return;
            }*/

            if (joinHopCount) {
                EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Error: received a second `last' state message! Restarting ..."
                   << endl;
                //std::cout << thisNode.getIp() << "Pastry: Error: received a second `last' state message! "
                //                    "Restarting ..." << std::endl;
                joinOverlay();
                return;
            }

            joinHopCount = msg->getJoinHopCount();
            //std::cout << stReceived.size() << " " << joinHopCount << std::endl;
            if (stReceived.size() < joinHopCount) {
                // some states still missing:
                cancelEvent(readyWait);
                scheduleAt(simTime() + readyWaitAmount, readyWait);
                //std::cout << simTime() << " " << thisNode.getIp() << " readyWait scheduled!" << std::endl;
            }
        }

        if (joinHopCount) {
            if (stReceived.size() > joinHopCount) {
                EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Error: too many state messages received in JOIN state! ("
                   << stReceived.size() << " > " << joinHopCount << ") Restarting ..."
                   << endl;
                //std::cout << " failed!" << std::endl;
                joinOverlay();
                return;
            }
            if (stReceived.size() == joinHopCount) {
                // all state messages are here, sort by hopcount:
                sort(stReceived.begin(), stReceived.end(),
                     stateMsgIsSmaller);

                // start pinging the nodes found in the first state message:
                stReceivedPos = stReceived.begin();
                stateCache = *stReceivedPos;
                EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    have all STATE messages, now pinging nodes."
                   << endl;
                if (pingBeforeSecondStage && proximityNeighborSelection) {
                    pingNodes();
                } else {
                    mergeState(); // JOINING / stateCache
                    //endProcessingState(); //no way
                    stateCache.msg = NULL;
                    changeState(READY);
                    EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
                       << " (" << thisNode.getKey().toString(16) << ")]\n"
                       << "    changeState(READY) called"
                       << endl;
                }

                // cancel timeout:
                if (readyWait->isScheduled()) cancelEvent(readyWait);
            } else {
                //TODO occasionally, here we got a wrong hop count in
                // iterative mode due to more than one it. lookup during join
                // procedure
                EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Still need some STATE messages."
                   << endl;
            }

        }
        return;
    }

    if (debugOutput) {
        EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    handling STATE message"
           << endl;
        EV << "        type: " << ((msg->getPastryStateMsgType()
                                    == PASTRY_STATE_UPDATE) ? "update"
                                                            :"standard")
           << endl;
        if (msg->getPastryStateMsgType() == PASTRY_STATE_UPDATE) {
            EV << "        msg timestamp:      " <<
                msg->getTimestamp() << endl;
            EV << "        last state change:  " <<
                lastStateChange << endl;
        }
    }

    if (((msg->getPastryStateMsgType() == PASTRY_STATE_UPDATE))
            && (msg->getTimestamp() <= lastStateChange)) {
        // if we received an update based on our outdated state,
        // mark handle for retrying later:
        EV << "[Pastry::handleStateMessage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    outdated state from " << msg->getSender()
           << endl;
        handle.outdatedUpdate = true;
    }

    // determine aliveTable to prevent leafSet from merging nodes that are
    // known to be dead:
    determineAliveTable(msg);

    if (msg->getPastryStateMsgType() == PASTRY_STATE_REPAIR) {
        // try to repair leafset based on repair message right now
        const TransportAddress& askLs = leafSet->repair(msg, &aliveTable);
        if (! askLs.isUnspecified()) {
            sendRequest(askLs, PASTRY_REQ_REPAIR);
        }

        // while not really known, it's safe to assume that a repair
        // message changed our state:
        lastStateChange = simTime();
        newLeafs();
    } else if (leafSet->mergeState(msg, &aliveTable)) {
        // merged state into leafset right now
        lastStateChange = simTime();
        newLeafs();
        updateTooltip();
    }
    // in READY state, only ping nodes to get proximity metric:
    if (!stateCache.msg) {
        // no state message is processed right now, start immediately:
        stateCache = handle;
        processState();
    } else {
        if (proximityNeighborSelection && (pingBeforeSecondStage ||
            msg->getPastryStateMsgType() == PASTRY_STATE_STD)) {
            // enqueue message for later processing:
            stateCacheQueue.push(handle);
            prePing(msg);
        } else {
            bool temp = true;
            if (!neighborhoodSet->mergeState(msg, NULL)) {
                temp = false;
            }
            if (!leafSet->mergeState(msg, NULL)) {
                temp = false;
            } else {
                newLeafs();
            }
            if (!routingTable->mergeState(msg, NULL)) {
                temp = false;
            }
            if (temp) lastStateChange = simTime();
            delete msg;
        }
    }
}

void Pastry::processState(void)
{
    if (proximityNeighborSelection && (pingBeforeSecondStage ||
        stateCache.msg->getPastryStateMsgType() == PASTRY_STATE_STD)) {
        pingNodes();
    } else {
        mergeState();
        endProcessingState();
    }
}
