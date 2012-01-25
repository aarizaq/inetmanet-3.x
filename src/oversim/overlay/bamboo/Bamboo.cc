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
 * @file Bamboo.cc
 * @author Gerhard Petruschat, Bernhard heep
 */

#include <cassert>

#include <IPvXAddress.h>
#include <IInterfaceTable.h>
#include <IPv4InterfaceData.h>
#include <RpcMacros.h>
#include <InitStages.h>
#include <GlobalStatistics.h>
#include <LookupListener.h>
#include <AbstractLookup.h>

#include "Bamboo.h"

Define_Module(Bamboo);

Bamboo::~Bamboo()
{
    // destroy self timer messages
    cancelAndDelete(localTuningTimer);
    cancelAndDelete(leafsetMaintenanceTimer);
    cancelAndDelete(globalTuningTimer);
}

void Bamboo::initializeOverlay(int stage)
{
    if ( stage != MIN_STAGE_OVERLAY )
        return;

    // Bamboo provides KBR services
    kbr = true;

    baseInit();

    localTuningInterval = par("localTuningInterval");
    leafsetMaintenanceInterval = par("leafsetMaintenanceInterval");
    globalTuningInterval = par("globalTuningInterval");

    joinTimeout = new cMessage("joinTimeout");

    localTuningTimer = new cMessage("repairTaskTimer");
    leafsetMaintenanceTimer = new cMessage("leafsetMaintenanceTimer");
    globalTuningTimer = new cMessage("globalTuningTimer");
}

void Bamboo::joinOverlay()
{
    changeState(INIT);

    if (bootstrapNode.isUnspecified()) {
        // no existing pastry network -> first node of a new one
        changeState(READY);
    } else {
        // join existing pastry network
        changeState(JOINING_2);
    }
}

void Bamboo::changeState(int toState)
{
    baseChangeState(toState);

    switch (toState) {
    case INIT:

        break;

    case DISCOVERY:

        break;

    case JOINING_2: {
        PastryLeafsetMessage* msg = new PastryLeafsetMessage("Leafset");
        msg->setPastryMsgType(PASTRY_MSG_LEAFSET_PULL);
        msg->setStatType(MAINTENANCE_STAT);
        msg->setSender(thisNode);
        msg->setSendStateTo(thisNode);
        leafSet->dumpToStateMessage(msg);
        msg->setBitLength(PASTRYLEAFSET_L(msg));
        RECORD_STATS(leafsetSent++; leafsetBytesSent += msg->getByteLength());
        std::vector<TransportAddress> sourceRoute;
        sourceRoute.push_back(bootstrapNode);
        sendToKey(thisNode.getKey(), msg, 0/*1*/, sourceRoute);

        if (joinTimeout->isScheduled()) cancelEvent(joinTimeout);
        scheduleAt(simTime() + joinTimeoutAmount, joinTimeout);
    }

    break;

    case READY:

        // schedule routing table maintenance task
        cancelEvent(localTuningTimer);
        scheduleAt(simTime() + localTuningInterval, localTuningTimer);

        cancelEvent(leafsetMaintenanceTimer);
        //scheduleAt(simTime() + leafsetMaintenanceInterval, leafsetMaintenanceTimer);
        scheduleAt(simTime() + 0.2 /* 200ms */, leafsetMaintenanceTimer);

        cancelEvent(globalTuningTimer);
        scheduleAt(simTime() + globalTuningInterval, globalTuningTimer);

        break;
    }
}

void Bamboo::handleTimerEvent(cMessage* msg)
{
    if (msg == joinTimeout) {
        EV << "[Bamboo::handleTimerEvent() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    join timeout expired, restarting..."
           << endl;
        joinOverlay();
    } else if (msg == localTuningTimer) {
        EV << "[Bamboo::handleTimerEvent() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    starting local tuning "
           << "(aka neighbor's neighbors / routing table maintenance)"
           << endl;
        doLocalTuning();
        scheduleAt(simTime() + localTuningInterval, localTuningTimer);
    } else if (msg == leafsetMaintenanceTimer) {
        EV << "[Bamboo::handleTimerEvent() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    starting leafset maintenance"
           << endl;
        doLeafsetMaintenance();
        scheduleAt(simTime() + leafsetMaintenanceInterval,
                   leafsetMaintenanceTimer);
    } else if (msg == globalTuningTimer) {
        EV << "[Bamboo::handleTimerEvent() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    starting global tuning"
           << endl;
        doGlobalTuning();
        scheduleAt(simTime() + globalTuningInterval, globalTuningTimer);
    }
}

void Bamboo::handleUDPMessage(BaseOverlayMessage* msg)
{
    PastryMessage* pastryMsg = check_and_cast<PastryMessage*>(msg);
    uint32_t type = pastryMsg->getPastryMsgType();

    if (debugOutput) {
        EV << "[Bamboo::handleUDPMessage() @ " << thisNode.getIp()
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
        case PASTRY_MSG_LEAFSET_PULL:
            EV << "PASTRY_MSG_LEAFSET_PULL";
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
    case PASTRY_MSG_JOIN:

        break;

    case PASTRY_MSG_LEAFSET: {
        PastryLeafsetMessage* lmsg =
            check_and_cast<PastryLeafsetMessage*>(pastryMsg);
        RECORD_STATS(leafsetReceived++; leafsetBytesReceived +=
            lmsg->getByteLength());

        if (state == JOINING_2) {
            cancelEvent(joinTimeout);
        }

        if ((state == JOINING_2) || (state == READY)) {
            handleLeafsetMessage(lmsg, true);
        } else {
            delete lmsg;
        }
    }
        break;

    case PASTRY_MSG_LEAFSET_PULL: {
        PastryLeafsetMessage* lmsg =
            check_and_cast<PastryLeafsetMessage*>(pastryMsg);
        RECORD_STATS(leafsetReceived++; leafsetBytesReceived +=
            lmsg->getByteLength());

        if (state == READY) {
            sendLeafset(lmsg->getSendStateTo());
            handleLeafsetMessage(lmsg, true);

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
                    EV << "[Bamboo::handleUDPMessage() @ " << thisNode.getIp()
                       << " (" << thisNode.getKey().toString(16) << ")]\n"
                       << "    received request for nonexistent routing"
                       << "table row, dropping message!" << endl;
                else sendRoutingRow(rtrmsg->getSendStateTo(), rtrmsg->getRow());
        else
            EV << "[Bamboo::handleUDPMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    received routing table request before reaching "
               << "READY state, dropping message!" << endl;
       delete rtrmsg;
    }
        break;

    case PASTRY_MSG_RROW: {
        PastryRoutingRowMessage* rtmsg =
            check_and_cast<PastryRoutingRowMessage*>(pastryMsg);
        RECORD_STATS(routingTableReceived++; routingTableBytesReceived +=
            rtmsg->getByteLength());

        if (state == READY) {
            // create state message to probe all nodes from row message
            PastryStateMessage* stateMsg = new PastryStateMessage("STATE");
            stateMsg->setTimestamp(rtmsg->getTimestamp());
            stateMsg->setPastryMsgType(PASTRY_MSG_STATE);
            stateMsg->setStatType(MAINTENANCE_STAT);
            stateMsg->setSender(rtmsg->getSender());
            stateMsg->setLeafSetArraySize(0);
            stateMsg->setNeighborhoodSetArraySize(0);
            stateMsg->setRoutingTableArraySize(rtmsg->getRoutingTableArraySize());

            for (uint32_t i = 0; i < rtmsg->getRoutingTableArraySize(); i++) {
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

void Bamboo::doLeafsetMaintenance(void)
{
    const TransportAddress& ask = leafSet->getRandomNode();
    if (!ask.isUnspecified()) {
        sendLeafset(ask, true);
        EV << "[Bamboo::doLeafsetMaintenance()]\n"
           << "    leafset maintenance: pulling leafset from "
           << ask << endl;
    }
}


int Bamboo::getNextRowToMaintain()
{
    int digit = 0;
    int lastRow = routingTable->getLastRow();

    int* choices = new int[lastRow + 1];
    int sum = 0;

    for (int i = 0; i < lastRow; ++i) {
        sum += (choices[i] = lastRow - i);
    }

    int rval = intuniform(0, sum);

    while (true) {
        rval -= choices [digit];
        if (rval <= 0)
            break;
        ++digit;
    }
    delete[] choices;

    return digit;
}


void Bamboo::doLocalTuning()
{
    rowToAsk = getNextRowToMaintain();

    const TransportAddress& ask4row = routingTable->getRandomNode(rowToAsk);

    if ((!ask4row.isUnspecified()) && (ask4row != thisNode)) {
        PastryRoutingRowRequestMessage* msg =
            new PastryRoutingRowRequestMessage("ROWREQ");
        msg->setPastryMsgType(PASTRY_MSG_ROWREQ);
        msg->setStatType(MAINTENANCE_STAT);
        msg->setSendStateTo(thisNode);
        msg->setRow(rowToAsk + 1);
        msg->setBitLength(PASTRYRTREQ_L(msg));

        RECORD_STATS(routingTableReqSent++;
                     routingTableReqBytesSent += msg->getByteLength());

        EV << "[Bamboo::doLocalTuning() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Sending  Message to Node in Row" << rowToAsk
           << endl;

        sendMessageToUDP(ask4row, msg);
    }
}


void Bamboo::doGlobalTuning(void)
{
    int digit = getNextRowToMaintain();

    // would be a better alternative
    //OverlayKey OverlayKey::randomSuffix(uint pos) const;

    uint32_t maxDigitIndex = OverlayKey::getLength() - bitsPerDigit;
    uint32_t maxKeyIndex = OverlayKey::getLength() - 1;
    OverlayKey newKey = OverlayKey::random();
    while (newKey.getBitRange(maxDigitIndex - digit * bitsPerDigit, bitsPerDigit) ==
           thisNode.getKey().getBitRange(maxDigitIndex - digit * bitsPerDigit, bitsPerDigit)) {
        newKey = OverlayKey::random();
    }

    assert(digit * bitsPerDigit < OverlayKey::getLength());
    for (uint16_t i = 0; i < digit * bitsPerDigit; ++i) {
        newKey[maxKeyIndex - i] = thisNode.getKey().getBit(maxKeyIndex - i);
    }

    createLookup()->lookup(newKey, 1, 0, 0, new BambooLookupListener(this));
}

bool Bamboo::handleFailedNode(const TransportAddress& failed)
{
    if (state != READY) {
        return false;
    }

    if (failed.isUnspecified()) {
        throw cRuntimeError("Bamboo::handleFailedNode(): failed is unspecified!");
    }

    const TransportAddress& lsAsk = leafSet->failedNode(failed);
    routingTable->failedNode(failed);
    neighborhoodSet->failedNode(failed);

    if (lsAsk.isUnspecified() && (! leafSet->isValid())) {
        EV << "[Bamboo::handleFailedNode()]\n"
           << "    lost connection to the network, trying to re-join."
           << endl;
        join();
        return false;
    }

    return true;
}

void Bamboo::checkProxCache(void)
{
    if (state == JOINING_2) {
        changeState(READY);
        return;
    }

    // state == READY
    simtime_t now = simTime();

    // no cached STATE message?
    if (!(stateCache.msg && stateCache.prox)) return;

    // some entries not yet determined?
    if (find(stateCache.prox->pr_rt.begin(), stateCache.prox->pr_rt.end(),
             PASTRY_PROX_PENDING) != stateCache.prox->pr_rt.end()) return;
    if (find(stateCache.prox->pr_ls.begin(), stateCache.prox->pr_ls.end(),
             PASTRY_PROX_PENDING) != stateCache.prox->pr_ls.end()) return;
    if (find(stateCache.prox->pr_ns.begin(), stateCache.prox->pr_ns.end(),
             PASTRY_PROX_PENDING) != stateCache.prox->pr_ns.end()) return;

    // merge info in own state tables
    // except leafset (was already handled in handleStateMessage)
    if (neighborhoodSet->mergeState(stateCache.msg, stateCache.prox)) {
        lastStateChange = now;
    }

    if (routingTable->mergeState(stateCache.msg, stateCache.prox)) {
        lastStateChange = now;
        EV << "[Bamboo::checkProxCache()]\n"
           << "    Merged nodes into routing table."
           << endl;
    }

    updateTooltip();

    delete stateCache.msg;
    stateCache.msg = NULL;
    delete stateCache.prox;
    stateCache.prox = NULL;

    // process next queued message:
    if (! stateCacheQueue.empty()) {
        stateCache = stateCacheQueue.front();
	      stateCacheQueue.pop();
        pingNodes();
    }
}

void Bamboo::handleStateMessage(PastryStateMessage* msg)
{
    if (debugOutput) {
        EV << "[Bamboo::handleStateMessage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    new STATE message to process "
           << static_cast<void*>(msg) << " in state "
           << ((state == READY)?"READY":((state == JOINING_2)?"JOIN":"INIT"))
           << endl;
    }

    if (state == INIT) {
        EV << "[Bamboo::handleStateMessage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    can't handle state messages until at least reaching JOIN state."
           << endl;
        delete msg;
        return;
    }

    PastryStateMsgHandle handle(msg);

    if (state == JOINING_2) {
        determineAliveTable(msg);
        leafSet->mergeState(msg, &aliveTable);
        // merged state into leafset right now
        lastStateChange = simTime();
        newLeafs();
        updateTooltip();

        // no state message is processed right now, start immediately:
       stateCache = handle;
       pingNodes();

        return;
    }

    // determine aliveTable to prevent leafSet from merging nodes that are
    // known to be dead:
    determineAliveTable(msg);
    if (leafSet->mergeState(msg, &aliveTable)) {
        // merged state into leafset right now
        lastStateChange = simTime();
        newLeafs();
        updateTooltip();
    }
    // in READY state, only ping nodes to get proximity metric:
    if (!stateCache.msg) {
        // no state message is processed right now, start immediately:
        stateCache = handle;
        if (proximityNeighborSelection) {
            pingNodes();
        } else {
            simtime_t now = simTime();
            if (neighborhoodSet->mergeState(stateCache.msg, NULL)) {
                lastStateChange = now;
            }
            if (routingTable->mergeState(stateCache.msg, NULL)) {
                lastStateChange = now;
                EV << "[Bamboo::checkProxCache()]\n"
                   << "    Merged nodes into routing table."
                   << endl;
            }
        }
    } else {
        // enqueue message for later processing:
        stateCacheQueue.push(handle);
        if (proximityNeighborSelection) prePing(msg);
    }
}

void Bamboo::lookupFinished(AbstractLookup *lookup)
{
    EV << "[Bamboo::lookupFinished()]\n";
    if (lookup->isValid()) {
        EV  << "    Lookup successful" << endl;
        const NodeVector& result = lookup->getResult();
        if (result[0] != thisNode) {
            if (proximityNeighborSelection) {
                // Global Tuning PING
                Prox prox = neighborCache->getProx(result[0],
                                                   NEIGHBORCACHE_DEFAULT,
                                                   PING_SINGLE_NODE,
                                                   this, NULL);
                if (prox != Prox::PROX_UNKNOWN) {
                    routingTable->mergeNode(result[0], prox.proximity);
                }
            } else {
                routingTable->mergeNode(result[0], -1.0);
            }
        }
    } else {
        EV << "    Lookup failed" << endl;
    }
}

