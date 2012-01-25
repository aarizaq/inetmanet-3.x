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
 * @file BasePastry.cc
 * @author Felix Palmen, Gerhard Petruschat, Bernhard Heep
 */

#include <sstream>

#include <IPvXAddress.h>
#include <IInterfaceTable.h>
#include <IPv4InterfaceData.h>
#include <RpcMacros.h>
#include <InitStages.h>
#include <NeighborCache.h>
#include <GlobalStatistics.h>
#include <BootstrapList.h>
#include <assert.h>

#include "BasePastry.h"


void BasePastry::purgeVectors(void)
{
    // purge Queue for processing multiple STATE messages:
    while (! stateCacheQueue.empty()) {
        delete stateCacheQueue.front().msg;
        delete stateCacheQueue.front().prox;
        stateCacheQueue.pop();
    }

    // delete cached state message:
    delete stateCache.msg;
    stateCache.msg = NULL;
    delete stateCache.prox;
    stateCache.prox = NULL;

    // purge vector of waiting sendState messages:
    if (! sendStateWait.empty()) {
        for (std::vector<PastrySendState*>::iterator it =
                 sendStateWait.begin(); it != sendStateWait.end(); it++) {
            if ( (*it)->isScheduled() ) cancelEvent(*it);
            delete *it;
        }
        sendStateWait.clear();
    }
}

void BasePastry::baseInit()
{
    bitsPerDigit = par("bitsPerDigit");
    numberOfLeaves = par("numberOfLeaves");
    numberOfNeighbors = par("numberOfNeighbors");
    joinTimeoutAmount = par("joinTimeout");
    repairTimeout = par("repairTimeout");
    enableNewLeafs = par("enableNewLeafs");
    optimizeLookup = par("optimizeLookup");
    useRegularNextHop = par("useRegularNextHop");
    alwaysSendUpdate = par("alwaysSendUpdate");
    proximityNeighborSelection = par("proximityNeighborSelection");

    if (!neighborCache->isEnabled()) {
        throw cRuntimeError("NeighborCache is disabled, which is mandatory "
                                "for Pastry/Bamboo. Activate it by setting "
                                "\"**.neighborCache.enableNeighborCache "
                                "= true\" in your omnetpp.ini!");
    }

    if (numberOfLeaves % 2) {
        EV << "[BasePastry::baseInit() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Warning: numberOfLeaves must be even - adding 1."
           << endl;
        numberOfLeaves++;
    }

    routingTable = check_and_cast<PastryRoutingTable*>
        (getParentModule()->getSubmodule("pastryRoutingTable"));
    leafSet = check_and_cast<PastryLeafSet*>
        (getParentModule()->getSubmodule("pastryLeafSet"));
    neighborhoodSet = check_and_cast<PastryNeighborhoodSet*>
        (getParentModule()->getSubmodule("pastryNeighborhoodSet"));

    stateCache.msg = NULL;
    stateCache.prox = NULL;

    rowToAsk = 0;

    // initialize statistics
    joins = 0;
    joinTries = 0;
    joinPartial = 0;
    joinSeen = 0;
    joinReceived = 0;
    joinSent = 0;
    stateSent = 0;
    stateReceived = 0;
    repairReqSent = 0;
    repairReqReceived = 0;
    stateReqSent = 0;
    stateReqReceived = 0;

    joinBytesSeen = 0;
    joinBytesReceived = 0;
    joinBytesSent = 0;
    stateBytesSent = 0;
    stateBytesReceived = 0;
    repairReqBytesSent = 0;
    repairReqBytesReceived = 0;
    stateReqBytesSent = 0;
    stateReqBytesReceived = 0;

    totalLookups = 0;
    responsibleLookups = 0;
    routingTableLookups = 0;
    closerNodeLookups = 0;
    closerNodeLookupsFromNeighborhood = 0;

    leafsetReqSent = 0;
    leafsetReqBytesSent = 0;
    leafsetReqReceived = 0;
    leafsetReqBytesReceived = 0;
    leafsetSent = 0;
    leafsetBytesSent = 0;
    leafsetReceived = 0;
    leafsetBytesReceived = 0;

    routingTableReqSent = 0;
    routingTableReqBytesSent = 0;
    routingTableReqReceived = 0;
    routingTableReqBytesReceived = 0;
    routingTableSent = 0;
    routingTableBytesSent = 0;
    routingTableReceived = 0;
    routingTableBytesReceived = 0;

    WATCH(joins);
    WATCH(joinTries);
    WATCH(joinSeen);
    WATCH(joinBytesSeen);
    WATCH(joinReceived);
    WATCH(joinBytesReceived);
    WATCH(joinSent);
    WATCH(joinBytesSent);
    WATCH(stateSent);
    WATCH(stateBytesSent);
    WATCH(stateReceived);
    WATCH(stateBytesReceived);
    WATCH(repairReqSent);
    WATCH(repairReqBytesSent);
    WATCH(repairReqReceived);
    WATCH(repairReqBytesReceived);
    WATCH(stateReqSent);
    WATCH(stateReqBytesSent);
    WATCH(stateReqReceived);
    WATCH(stateReqBytesReceived);
    WATCH(lastStateChange);

    WATCH(leafsetReqSent);
    WATCH(leafsetReqBytesSent);
    WATCH(leafsetReqReceived);
    WATCH(leafsetReqBytesReceived);
    WATCH(leafsetSent);
    WATCH(leafsetBytesSent);
    WATCH(leafsetReceived);
    WATCH(leafsetBytesReceived);

    WATCH(routingTableReqSent);
    WATCH(routingTableReqBytesSent);
    WATCH(routingTableReqReceived);
    WATCH(routingTableReqBytesReceived);
    WATCH(routingTableSent);
    WATCH(routingTableBytesSent);
    WATCH(routingTableReceived);
    WATCH(routingTableBytesReceived);
}


void BasePastry::baseChangeState(int toState)
{
    switch (toState) {
    case INIT:
        state = INIT;

        if (!thisNode.getKey().isUnspecified())
            bootstrapList->removeBootstrapNode(thisNode);

        if (joinTimeout->isScheduled()) cancelEvent(joinTimeout);

        purgeVectors();

        bootstrapNode = bootstrapList->getBootstrapNode();

        routingTable->initializeTable(bitsPerDigit, repairTimeout, thisNode);
        leafSet->initializeSet(numberOfLeaves, bitsPerDigit,
                               repairTimeout, thisNode, this);
        neighborhoodSet->initializeSet(numberOfNeighbors, bitsPerDigit,
                                       thisNode);

        updateTooltip();
        lastStateChange = simTime();

        getParentModule()->getParentModule()->bubble("entering INIT state");

        break;

    case JOINING_2:
        state = JOINING_2;

        // bootstrapNode must be obtained before calling this method,
        // for example by calling changeState(INIT)

        if (bootstrapNode.isUnspecified()) {
            // no existing pastry network -> first node of a new one
            changeState(READY);
            return;
        }

        cancelEvent(joinTimeout);
        scheduleAt(simTime() + joinTimeoutAmount, joinTimeout);

        updateTooltip();
        getParentModule()->getParentModule()->bubble("entering JOIN state");

        RECORD_STATS(joinTries++);

        break;

    case READY:
        assert(state != READY);
        state = READY;

         //bootstrapList->registerBootstrapNode(thisNode);

        // if we are the first node in the network, there's nothing else
        // to do
        if (bootstrapNode.isUnspecified()) {
            RECORD_STATS(joinTries++);
            RECORD_STATS(joins++);
            setOverlayReady(true);
            return;
        }

        getParentModule()->getParentModule()->bubble("entering READY state");
        updateTooltip();
        RECORD_STATS(joins++);

        break;

    default: // discovery
        break;
    }
    setOverlayReady(state == READY);
}


void BasePastry::newLeafs(void)
{
    if (! enableNewLeafs) return;

    PastryNewLeafsMessage* msg = leafSet->getNewLeafsMessage();
    if (msg) {
        send(msg, "appOut");
        EV << "[BasePastry::newLeafs() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    newLeafs() called."
           << endl;
    }
}


void BasePastry::changeState(int toState)
{

}


void BasePastry::pingResponse(PingResponse* msg, cPolymorphic* context,
                              int rpcId, simtime_t rtt)
{
    EV << "[BasePastry::pingResponse() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]\n"
       << "    Pong (or Ping-context from NeighborCache) received (from "
       << msg->getSrcNode().getIp() << ")"
       << endl;

    const NodeHandle& src = msg->getSrcNode();
    assert(!src.isUnspecified());

    // merge single pinged nodes (bamboo global tuning)
    if (rpcId == PING_SINGLE_NODE) {
        routingTable->mergeNode(src, proximityNeighborSelection ?
                                     rtt : SimTime::getMaxTime());
        return;
    }

    /*// a node with the an equal ID has responded
    if ((src.getKey() == thisNode.getKey()) && (src.getIp() != thisNode.getIp())) {
        EV << "[BasePastry::pingResponse() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    a node with the an equal ID has responded, rejoining" << endl;
        delete context;
        //joinOverlay();
        return;
    }*/

    if (context != NULL && stateCache.msg && stateCache.prox) {
        PingContext* pingContext = check_and_cast<PingContext*>(context);
        if (pingContext->nonce != stateCache.nonce) {
            delete context;
            return;
            //throw cRuntimeError("response doesn't fit stateCache");
        }
        switch (pingContext->stateObject) {
            case ROUTINGTABLE: {
                /*node = &(stateCache.msg->getRoutingTable(pingContext->index));
                if((node->isUnspecified()) || (*node != src)) {
                    std::cout << simTime() << " " << thisNode.getIp() << " rt: state from "
                              << stateCache.msg->getSender().getIp() << " *** failed: node "
                              << node->ip << " src " << src.getIp() << std::endl;
                    break;
                }*/
                *(stateCache.prox->pr_rt.begin() + pingContext->index) = rtt;
                break;
            }
            case LEAFSET: {
                /*node = &(stateCache.msg->getLeafSet(pingContext->index));
                if ((node->isUnspecified()) || (*node != src)) {
                    std::cout << simTime() << " " << thisNode.getIp() << " ls: state from "
                              << stateCache.msg->getSender().getIp() << " *** failed: node "
                              << node->ip << " src " << src.getIp() << std::endl;
                    break;
                }*/
                *(stateCache.prox->pr_ls.begin() + pingContext->index) = rtt;
                break;
            }
            case NEIGHBORHOODSET: {
                /*node = &(stateCache.msg->getNeighborhoodSet(pingContext->index));
                if((node->isUnspecified()) || (*node != src)) {
                    std::cout << simTime() << " " << thisNode.getIp() << " ns: state from "
                              << stateCache.msg->getSender().getIp() << " *** failed: node "
                              << node->ip << " src " << src.getIp() << std::endl;
                    break;
                }*/
                *(stateCache.prox->pr_ns.begin() + pingContext->index) = rtt;
                break;
            }
            default: {
                throw cRuntimeError("wrong state object type!");
            }
        }
        checkProxCache();
    }
    delete context;
}


void BasePastry::proxCallback(const TransportAddress& node, int rpcId,
                              cPolymorphic *contextPointer, Prox prox)
{
    Enter_Method("proxCallback()");

    EV << "[BasePastry::proxCallback() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Pong received (from "
           << node.getIp() << ")"
           << endl;

    double rtt = ((prox == Prox::PROX_TIMEOUT) ? PASTRY_PROX_INFINITE
                                               : prox.proximity);

    // merge single pinged nodes (bamboo global tuning)
    if (rpcId == PING_SINGLE_NODE) {
        routingTable->mergeNode((const NodeHandle&)node,
                                proximityNeighborSelection ?
                                rtt : SimTime::getMaxTime());
        delete contextPointer;
        return;
    }

    if (contextPointer != NULL && stateCache.msg && stateCache.prox) {
        PingContext* pingContext = check_and_cast<PingContext*>(contextPointer);

        if (pingContext->nonce != stateCache.nonce) {
            delete contextPointer;
            return;
        }
        // handle failed node
        if (rtt == PASTRY_PROX_INFINITE && state== READY) {
            handleFailedNode(node); // TODO
            updateTooltip();

            // this could initiate a re-join, exit the handler in that
            // case because all local data was erased:
            if (state != READY) {
                delete contextPointer;
                return;
            }
        }
        switch (pingContext->stateObject) {
        case ROUTINGTABLE:
            *(stateCache.prox->pr_rt.begin() + pingContext->index) = rtt;
            break;

        case LEAFSET:
            *(stateCache.prox->pr_ls.begin() + pingContext->index) = rtt;
            break;

        case NEIGHBORHOODSET:
            *(stateCache.prox->pr_ns.begin() + pingContext->index) = rtt;
            break;

        default:
            throw cRuntimeError("wrong state object type!");
        }
        checkProxCache();
    }
    delete contextPointer;
}


void BasePastry::prePing(const PastryStateMessage* stateMsg)
{
    uint32_t rt_size = stateMsg->getRoutingTableArraySize();
    uint32_t ls_size = stateMsg->getLeafSetArraySize();
    uint32_t ns_size = stateMsg->getNeighborhoodSetArraySize();

    for (uint32_t i = 0; i < rt_size + ls_size + ns_size; i++) {
        const NodeHandle* node;
        if (i < rt_size) {
            node = &(stateMsg->getRoutingTable(i));
        }
        else if (i < (rt_size + ls_size) ) {
            node = &(stateMsg->getLeafSet(i - rt_size));
        }
        else {
            node = &(stateMsg->getNeighborhoodSet(i - rt_size - ls_size));
        }
        if ((node->isUnspecified()) || (*node == thisNode)) {
            continue;
        }
        /*if (node->key == thisNode.getKey()) {
            cerr << "Pastry Warning: Other node with same key found, "
                "restarting!" << endl;
            opp_error("TODO: Other node with same key found...");
            joinOverlay(); //segfault
            //return;
            continue;
        }*/

        neighborCache->getProx(*node, NEIGHBORCACHE_DEFAULT, PING_RECEIVED_STATE, this, NULL);
    }
}

void BasePastry::pingNodes(void)
{
    EV << "[BasePastry::pingNodes() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]" << endl;

    if (stateCache.msg == NULL) throw cRuntimeError("no state msg");

    assert(stateCache.prox == NULL);
    stateCache.prox = new PastryStateMsgProximity();

    uint32_t rt_size = stateCache.msg->getRoutingTableArraySize();
    stateCache.prox->pr_rt.resize(rt_size, PASTRY_PROX_UNDEF);

    uint32_t ls_size = stateCache.msg->getLeafSetArraySize();
    stateCache.prox->pr_ls.resize(ls_size, PASTRY_PROX_UNDEF);

    uint32_t ns_size = stateCache.msg->getNeighborhoodSetArraySize();
    stateCache.prox->pr_ns.resize(ns_size, PASTRY_PROX_UNDEF);

    std::vector< std::pair<const NodeHandle*, PingContext*> > nodesToPing;
    // set prox state
    for (uint32_t i = 0; i < rt_size + ls_size + ns_size; i++) {
        const NodeHandle* node;
        std::vector<simtime_t>::iterator proxPos;
        PingContext* pingContext = NULL;
        StateObject stateObject;
        uint32_t index;
        if (stateCache.msg == NULL) break;
        if (i < rt_size) {
            node = &(stateCache.msg->getRoutingTable(i));
            proxPos = stateCache.prox->pr_rt.begin() + i;
            stateObject = ROUTINGTABLE;
            index = i;
        } else if ( i < (rt_size + ls_size) ) {
            node = &(stateCache.msg->getLeafSet(i - rt_size));
            proxPos = stateCache.prox->pr_ls.begin() + (i - rt_size);
            stateObject = LEAFSET;
            index = i - rt_size;
        } else {
            node = &(stateCache.msg->getNeighborhoodSet(i - rt_size - ls_size));
            proxPos = stateCache.prox->pr_ns.begin() + (i - rt_size - ls_size);
            stateObject = NEIGHBORHOODSET;
            index = i - rt_size - ls_size;
        }
        // proximity is undefined for unspecified nodes:
        if (!node->isUnspecified()) {
            pingContext = new PingContext(stateObject, index,
                                          stateCache.nonce);

            Prox prox = neighborCache->getProx(*node, NEIGHBORCACHE_DEFAULT, -1,
                                               this, pingContext);
            if (prox == Prox::PROX_SELF) {
                *proxPos = 0;
            } else if (prox == Prox::PROX_TIMEOUT) {
                *proxPos = PASTRY_PROX_INFINITE;
            } else if (prox == Prox::PROX_UNKNOWN) {
                *proxPos = PASTRY_PROX_PENDING;
            } else {
                *proxPos = prox.proximity;
            }
        }
    }
    checkProxCache();
}

void BasePastry::determineAliveTable(const PastryStateMessage* stateMsg)
{
    uint32_t rt_size = stateMsg->getRoutingTableArraySize();
    aliveTable.pr_rt.clear();
    aliveTable.pr_rt.resize(rt_size, 1);

    uint32_t ls_size = stateMsg->getLeafSetArraySize();
    aliveTable.pr_ls.clear();
    aliveTable.pr_ls.resize(ls_size, 1);

    uint32_t ns_size = stateMsg->getNeighborhoodSetArraySize();
    aliveTable.pr_ns.clear();
    aliveTable.pr_ns.resize(ns_size, 1);

    for (uint32_t i = 0; i < rt_size + ls_size + ns_size; i++) {
        const TransportAddress* node;
        std::vector<simtime_t>::iterator tblPos;
        if (i < rt_size) {
            node = &(stateMsg->getRoutingTable(i));
            tblPos = aliveTable.pr_rt.begin() + i;
        } else if ( i < (rt_size + ls_size) ) {
            node = &(stateMsg->getLeafSet(i - rt_size));
            tblPos = aliveTable.pr_ls.begin() + (i - rt_size);
        } else {
            node = &(stateMsg->getNeighborhoodSet(i - rt_size - ls_size));
            tblPos = aliveTable.pr_ns.begin() + (i - rt_size - ls_size);
        }
        if (neighborCache->getProx(*node, NEIGHBORCACHE_DEFAULT_IMMEDIATELY) ==
                Prox::PROX_TIMEOUT) {
            *tblPos = PASTRY_PROX_INFINITE;
        }
    }
}

void BasePastry::sendStateTables(const TransportAddress& destination,
                                 int type, ...)
{
    if (destination.getIp() == thisNode.getIp())
        opp_error("Pastry: trying to send state to self!");

    int hops = 0;
    bool last = false;
    simtime_t timestamp = 0;

    if ((type == PASTRY_STATE_JOIN) ||
        (type == PASTRY_STATE_MINJOIN) ||
        (type == PASTRY_STATE_UPDATE)) {
        // additional parameters needed:
        va_list ap;
        va_start(ap, type);
        if (type == PASTRY_STATE_JOIN || type == PASTRY_STATE_MINJOIN) {
            hops = va_arg(ap, int);
            last = static_cast<bool>(va_arg(ap, int));
        } else {
            timestamp = *va_arg(ap, simtime_t*);
        }
        va_end(ap);
    }

    // create new state msg and set special fields for some types:
    PastryStateMessage* stateMsg;
    if (type == PASTRY_STATE_JOIN || type == PASTRY_STATE_MINJOIN) {
        stateMsg = new PastryStateMessage("STATE (Join)");
        stateMsg->setJoinHopCount(hops);
        stateMsg->setLastHop(last);
        stateMsg->setTimestamp(simTime());
    } else if (type == PASTRY_STATE_UPDATE) {
        stateMsg = new PastryStateMessage("STATE (Update)");
        EV << "[BasePastry::sendStateTables() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    sending state (update) to " << destination
           << endl;
        stateMsg->setTimestamp(timestamp);
    } else if (type == PASTRY_STATE_REPAIR) {
        stateMsg = new PastryStateMessage("STATE (Repair)");
        stateMsg->setTimestamp(timestamp);
        EV << "[BasePastry::sendStateTables() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    sending state (repair) to " << destination
           << endl;
    } else {
        stateMsg = new PastryStateMessage("STATE");
        EV << "[BasePastry::sendStateTables() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    sending state (standard) to " << destination
           << endl;
    }

    // fill in standard content:
    stateMsg->setPastryMsgType(PASTRY_MSG_STATE);
    stateMsg->setStatType(MAINTENANCE_STAT);
    stateMsg->setPastryStateMsgType(type);
    stateMsg->setSender(thisNode);

    // the following part of the new join works on the assumption, that the node
    // routing the join message is close to the joining node
    // therefore its switched on together with the discovery algorithm
    if (type == PASTRY_STATE_MINJOIN) {
        //send just the needed row for new join protocol
        routingTable->dumpRowToMessage(stateMsg, hops);
        if (last) leafSet->dumpToStateMessage(stateMsg);
        else stateMsg->setLeafSetArraySize(0);
        if (hops == 1) neighborhoodSet->dumpToStateMessage(stateMsg);
        else stateMsg->setNeighborhoodSetArraySize(0);
    } else {
        routingTable->dumpToStateMessage(stateMsg);
        leafSet->dumpToStateMessage(stateMsg);
        neighborhoodSet->dumpToStateMessage(stateMsg);
    }

    // send...
    stateMsg->setBitLength(PASTRYSTATE_L(stateMsg));
    RECORD_STATS(stateSent++; stateBytesSent += stateMsg->getByteLength());
    sendMessageToUDP(destination, stateMsg);
}

void BasePastry::sendStateDelayed(const TransportAddress& destination)
{
    PastrySendState* selfMsg = new PastrySendState("sendStateWait");
    selfMsg->setDest(destination);
    sendStateWait.push_back(selfMsg);
    scheduleAt(simTime() + 0.0001, selfMsg);
}

void BasePastry::pingTimeout(PingCall* msg,
                             const TransportAddress& dest,
                             cPolymorphic* context,
                             int rpcId)
{
    EV << "[BasePastry::sendStateDelayed() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]\n"
       << "    Ping timeout occurred (" << dest.getIp() << ")"
       << endl;

    // handle failed node
    if (state == READY) {
        handleFailedNode(dest); // TODO
        updateTooltip();

        // this could initiate a re-join, exit the handler in that
        // case because all local data was erased:
        if (state != READY) {
            delete context;
            return;
        }
    }

    //TODO must be removed
    if (context && stateCache.msg && stateCache.prox &&
        rpcId == PING_RECEIVED_STATE) {
        PingContext* pingContext = check_and_cast<PingContext*>(context);
        if (pingContext->nonce != stateCache.nonce) {
            delete context;
            return;
            //std::stringstream temp;
            //temp << thisNode << " timeout/call doesn't fit stateCache";
            //throw cRuntimeError(temp.str().c_str());
        }
        //const NodeHandle* node;
        switch (pingContext->stateObject) {
            case ROUTINGTABLE: {
                /*if (pingContext->index >=
                    stateCache.msg->getRoutingTableArraySize()) {
                    std::cout << "*** FAILED ***" << std::endl;
                    break;
                }
                node = &(stateCache.msg->getRoutingTable(pingContext->index));
                if((node->isUnspecified()) || (dest != *node)) {
                    std::cout << msg->getNonce() << " " << simTime() << " " << thisNode.getIp() << " rt: state from "
                    << stateCache.msg->getSender().getIp() << " *** failed: node "
                    << node->ip << " failed dest " << dest.getIp() << std::endl;
                    break;
                }*/
                *(stateCache.prox->pr_rt.begin() + pingContext->index) =
                    PASTRY_PROX_INFINITE;
                break;
            }
            case LEAFSET: {
                /*if (pingContext->index >=
                    stateCache.msg->getLeafSetArraySize()) {
                    std::cout << "*** FAILED ***" << std::endl;
                    break;
                }
                node = &(stateCache.msg->getLeafSet(pingContext->index));
                if((node->isUnspecified()) || (dest != *node)) {
                    std::cout << msg->getNonce() << " " << simTime() << " " << thisNode.getIp() << " ls: state from "
                    << stateCache.msg->getSender().getIp() << " *** failed: node "
                    << node->ip << " failed dest " << dest.getIp() << std::endl;
                    break;
                }*/
                *(stateCache.prox->pr_ls.begin() + pingContext->index) =
                    PASTRY_PROX_INFINITE;
                break;
            }
            case NEIGHBORHOODSET: {
                /*if (pingContext->index >=
                    stateCache.msg->getNeighborhoodSetArraySize()) {
                    std::cout << "*** FAILED ***" << std::endl;
                    break;
                }
                node = &(stateCache.msg->getNeighborhoodSet(pingContext->index));
                if((node->isUnspecified()) || (dest != *node)) {
                    std::cout << msg->getNonce() << " " << simTime() << " " << thisNode.getIp() << " ns: state from "
                    << stateCache.msg->getSender().getIp() << " *** failed: node "
                    << node->ip << " failed dest " << dest.getIp() << std::endl;
                    break;
                }*/
                *(stateCache.prox->pr_ns.begin() + pingContext->index) =
                    PASTRY_PROX_INFINITE;
                break;
            }
        }
        checkProxCache();
    }

    delete context;
}

void BasePastry::sendRequest(const TransportAddress& ask, int type)
{
    assert(ask != thisNode);
    std::string msgName("Req: ");
    switch (type) {
    case PASTRY_REQ_REPAIR:
        if (ask.isUnspecified())
            throw cRuntimeError("Pastry::sendRequest(): asked for repair from "
                                "unspecified node!");
        msgName += "Repair";
        break;

    case PASTRY_REQ_STATE:
        if (ask.isUnspecified())
            throw cRuntimeError("Pastry::sendRequest(): asked for state from "
                                "unspecified node!");
        msgName += "State";
        break;

    case PASTRY_REQ_LEAFSET:
        if (ask.isUnspecified())
            throw cRuntimeError("Pastry::sendRequest(): asked for leafset from "
                  "unspecified node!");
        msgName += "Leafset";
        break;
    }
    PastryRequestMessage* msg = new PastryRequestMessage(msgName.c_str());
    msg->setPastryMsgType(PASTRY_MSG_REQ);
    msg->setPastryReqType(type);
    msg->setStatType(MAINTENANCE_STAT);
    msg->setSendStateTo(thisNode);
    msg->setBitLength(PASTRYREQ_L(msg));
    sendMessageToUDP(ask, msg); //TODO RPCs

    switch (type) {
    case PASTRY_REQ_REPAIR:
        RECORD_STATS(repairReqSent++; repairReqBytesSent += msg->getByteLength());
        break;

    case PASTRY_REQ_STATE:
        RECORD_STATS(stateReqSent++; stateReqBytesSent += msg->getByteLength());
        break;

    case PASTRY_REQ_LEAFSET:
        RECORD_STATS(leafsetReqSent++; leafsetReqBytesSent += msg->getByteLength());
        break;
    }
}


void BasePastry::sendLeafset(const TransportAddress& tell, bool pull)
{
    if (tell.isUnspecified())
        opp_error("Pastry::sendLeafset(): send leafset to "
                  "unspecified node!");

    PastryLeafsetMessage* msg = new PastryLeafsetMessage("Leafset");
    if (pull) msg->setPastryMsgType(PASTRY_MSG_LEAFSET_PULL);
    else msg->setPastryMsgType(PASTRY_MSG_LEAFSET);
    msg->setTimestamp(simTime());
    msg->setStatType(MAINTENANCE_STAT);
    msg->setSender(thisNode);
    msg->setSendStateTo(thisNode);
    leafSet->dumpToStateMessage(msg);
    msg->setBitLength(PASTRYLEAFSET_L(msg));
    RECORD_STATS(leafsetSent++; leafsetBytesSent += msg->getByteLength());
    sendMessageToUDP(tell, msg);


}

void BasePastry::sendRoutingRow(const TransportAddress& tell, int row)
{
    if (tell.isUnspecified())
        opp_error("Pastry::sendRoutingTable(): asked for routing Table from "
                  "unspecified node!");

    PastryRoutingRowMessage* msg = new PastryRoutingRowMessage("Routing Row");
    msg->setPastryMsgType(PASTRY_MSG_RROW);
    msg->setStatType(MAINTENANCE_STAT);
    //msg->setSendStateTo(thisNode);
    msg->setSender(thisNode);
    msg->setRow(row);
    routingTable->dumpRowToMessage(msg, row);
    msg->setBitLength(PASTRYRTABLE_L(msg));
    RECORD_STATS(routingTableSent++; routingTableBytesSent += msg->getByteLength());
    sendMessageToUDP(tell, msg);
}

void BasePastry::handleRequestMessage(PastryRequestMessage* msg)
{
    assert(msg->getSendStateTo() != thisNode);
    uint32_t reqtype = msg->getPastryReqType();
    if (reqtype == PASTRY_REQ_REPAIR) {
        RECORD_STATS(repairReqReceived++; repairReqBytesReceived +=
            msg->getByteLength());
        if (state == READY)
            sendStateTables(msg->getSendStateTo(),
                            PASTRY_STATE_REPAIR);
        else
            EV << "[BasePastry::handleRequestMessage() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    received repair request before reaching"
            << " READY state, dropping message!"
            << endl;
        delete msg;
    }
    else if (reqtype == PASTRY_REQ_STATE) {
        RECORD_STATS(stateReqReceived++; stateReqBytesReceived +=
            msg->getByteLength());
        if (state == READY)
            sendStateTables(msg->getSendStateTo());
        else
            EV << "[BasePastry::handleRequestMessage() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    received state request before reaching"
            << " READY state, dropping message!"
            << endl;
        delete msg;
    }
    else if (PASTRY_REQ_LEAFSET) {
        RECORD_STATS(leafsetReqReceived++; leafsetReqBytesReceived +=
            msg->getByteLength());
        if (state == READY) {
            sendLeafset(msg->getSendStateTo());
        }
        else
            EV << "[BasePastry::handleRequestMessage() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    received leafset request before reaching"
            << " READY state, dropping message!"
            << endl;
        delete msg;
    }

}

void BasePastry::handleLeafsetMessage(PastryLeafsetMessage* msg, bool mergeSender)
{
    uint32_t lsSize = msg->getLeafSetArraySize();
    PastryStateMessage* stateMsg;

    stateMsg = new PastryStateMessage("STATE");
    stateMsg->setTimestamp(msg->getTimestamp());
    stateMsg->setPastryMsgType(PASTRY_MSG_STATE);
    stateMsg->setStatType(MAINTENANCE_STAT);
    stateMsg->setSender(msg->getSender());
    stateMsg->setLeafSetArraySize(lsSize);
    stateMsg->setNeighborhoodSetArraySize(0);
    stateMsg->setRoutingTableArraySize(0);

    for (uint32_t i = 0; i < lsSize; i++) {
        stateMsg->setLeafSet(i, msg->getLeafSet(i));
    }

    if (mergeSender) {
        stateMsg->setLeafSetArraySize(lsSize+1);
        stateMsg->setLeafSet(lsSize, msg->getSender());
    }

    handleStateMessage(stateMsg);
    delete msg;
}

bool BasePastry::isSiblingFor(const NodeHandle& node,
                              const OverlayKey& key,
                              int numSiblings,
                              bool* err)
{
    if (key.isUnspecified())
        error("Pastry::isSiblingFor(): key is unspecified!");

    if ((numSiblings == 1) && (node == thisNode)) {
        if (leafSet->isClosestNode(key)) {
            *err = false;
            return true;
        } else {
            *err = false;
            return false;
        }
    }

    NodeVector* result =  leafSet->createSiblingVector(key, numSiblings);

    if (result == NULL) {
        *err = true;
        return false;
    }

    if (result->contains(node.getKey())) {
        delete result;
        *err = false;
        return true;
    } else {
        delete result;
        *err = true;
        return false;
    }

    /*
      const NodeHandle& dest = leafSet->getDestinationNode(key);
      if (!dest.isUnspecified()) {
      *err = false;
      return true;
      } else {

      *err = true;
      return false;
      }
    */
}


void BasePastry::handleAppMessage(BaseOverlayMessage* msg)
{
    delete msg;
}

void BasePastry::updateTooltip()
{
    if (ev.isGUI()) {
        std::stringstream ttString;

        // show our predecessor and successor in tooltip
        ttString << leafSet->getPredecessor() << endl << thisNode << endl
                 << leafSet->getSuccessor();

        getParentModule()->getParentModule()->getDisplayString().
            setTagArg("tt", 0, ttString.str().c_str());
        getParentModule()->getDisplayString().
            setTagArg("tt", 0, ttString.str().c_str());
        getDisplayString().setTagArg("tt", 0, ttString.str().c_str());

        // draw arrows:
        showOverlayNeighborArrow(leafSet->getSuccessor(), true,
                                 "m=m,50,0,50,0;ls=red,1");
        showOverlayNeighborArrow(leafSet->getPredecessor(), false,
                                 "m=m,50,100,50,100;ls=green,1");

    }
}

BasePastry::~BasePastry()
{
    cancelAndDelete(joinTimeout);

    purgeVectors();
}

void BasePastry::finishOverlay()
{
    // remove this node from the bootstrap list
    if (!thisNode.getKey().isUnspecified()) bootstrapList->removeBootstrapNode(thisNode);

    // collect statistics
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;

    // join statistics
    //if (joinTries > joins)
        //std::cout << thisNode << " jt:" << joinTries << " j:" << joins << " jts:"
        //          << joinTimeout->isScheduled() << " rws:" << readyWait->isScheduled()
        //          << " state:" << state << " time:" << time << std::endl;
    // join is on the way...
    if (joinTries > 0 && joinTimeout->isScheduled()) joinTries--;
    if (joinTries > 0) {
        globalStatistics->addStdDev("Pastry: join success ratio", (double)joins / (double)joinTries);
        globalStatistics->addStdDev("Pastry: join tries", joinTries);
    } else if (state == READY) {
        // nodes has joined in init-/transition-phase
        globalStatistics->addStdDev("Pastry: join success ratio", 1);
        globalStatistics->addStdDev("Pastry: join tries", 1);
    } else {
        globalStatistics->addStdDev("Pastry: join success ratio", 0);
        globalStatistics->addStdDev("Pastry: join tries", 1);
    }

    globalStatistics->addStdDev("Pastry: joins with missing replies from routing path/s",
                                joinPartial / time);
    globalStatistics->addStdDev("Pastry: JOIN Messages seen/s", joinSeen / time);
    globalStatistics->addStdDev("Pastry: bytes of JOIN Messages seen/s", joinBytesSeen / time);
    globalStatistics->addStdDev("Pastry: JOIN Messages received/s", joinReceived / time);
    globalStatistics->addStdDev("Pastry: bytes of JOIN Messages received/s",
                                joinBytesReceived / time);
    globalStatistics->addStdDev("Pastry: JOIN Messages sent/s", joinSent / time);
    globalStatistics->addStdDev("Pastry: bytes of JOIN Messages sent/s", joinBytesSent / time);
    globalStatistics->addStdDev("Pastry: STATE Messages sent/s", stateSent / time);
    globalStatistics->addStdDev("Pastry: bytes of STATE Messages sent/s", stateBytesSent / time);
    globalStatistics->addStdDev("Pastry: STATE Messages received/s", stateReceived / time);
    globalStatistics->addStdDev("Pastry: bytes of STATE Messages received/s",
                                stateBytesReceived / time);
    globalStatistics->addStdDev("Pastry: REPAIR Requests sent/s", repairReqSent / time);
    globalStatistics->addStdDev("Pastry: bytes of REPAIR Requests sent/s",
                                repairReqBytesSent / time);
    globalStatistics->addStdDev("Pastry: REPAIR Requests received/s", repairReqReceived / time);
    globalStatistics->addStdDev("Pastry: bytes of REPAIR Requests received/s",
                                repairReqBytesReceived / time);
    globalStatistics->addStdDev("Pastry: STATE Requests sent/s", stateReqSent / time);
    globalStatistics->addStdDev("Pastry: bytes of STATE Requests sent/s", stateReqBytesSent / time);
    globalStatistics->addStdDev("Pastry: STATE Requests received/s", stateReqReceived / time);
    globalStatistics->addStdDev("Pastry: bytes of STATE Requests received/s",
                                stateReqBytesReceived / time);

    globalStatistics->addStdDev("Pastry: bytes of STATE Requests received/s",
                                stateReqBytesReceived / time);

    globalStatistics->addStdDev("Pastry: total number of lookups", totalLookups);
    globalStatistics->addStdDev("Pastry: responsible lookups", responsibleLookups);
    globalStatistics->addStdDev("Pastry: lookups in routing table", routingTableLookups);
    globalStatistics->addStdDev("Pastry: lookups using closerNode()", closerNodeLookups);
    globalStatistics->addStdDev("Pastry: lookups using closerNode() with result from "
                                "neighborhood set", closerNodeLookupsFromNeighborhood);
    globalStatistics->addStdDev("Pastry: LEAFSET Requests sent/s", leafsetReqSent / time);
    globalStatistics->addStdDev("Pastry: bytes of LEAFSET Requests sent/s", leafsetReqBytesSent / time);
    globalStatistics->addStdDev("Pastry: LEAFSET Requests received/s", leafsetReqReceived / time);
    globalStatistics->addStdDev("Pastry: bytes of LEAFSET Requests received/s",
                                leafsetReqBytesReceived / time);
    globalStatistics->addStdDev("Pastry: LEAFSET Messages sent/s", leafsetSent / time);
    globalStatistics->addStdDev("Pastry: bytes of LEAFSET Messages sent/s", leafsetBytesSent / time);
    globalStatistics->addStdDev("Pastry: LEAFSET Messages received/s", leafsetReceived / time);
    globalStatistics->addStdDev("Pastry: bytes of LEAFSET Messages received/s",
                                leafsetBytesReceived / time);
    globalStatistics->addStdDev("Pastry: ROUTING TABLE Requests sent/s", routingTableReqSent / time);
    globalStatistics->addStdDev("Pastry: bytes of ROUTING TABLE Requests sent/s", routingTableReqBytesSent / time);
    globalStatistics->addStdDev("Pastry: ROUTING TABLE Requests received/s", routingTableReqReceived / time);
    globalStatistics->addStdDev("Pastry: bytes of ROUTING TABLE Requests received/s",
                                routingTableReqBytesReceived / time);
    globalStatistics->addStdDev("Pastry: ROUTING TABLE Messages sent/s", routingTableSent / time);
    globalStatistics->addStdDev("Pastry: bytes of ROUTING TABLE Messages sent/s", routingTableBytesSent / time);
    globalStatistics->addStdDev("Pastry: ROUTING TABLE Messages received/s", routingTableReceived / time);
    globalStatistics->addStdDev("Pastry: bytes of ROUTING TABLE Messages received/s",
                                routingTableBytesReceived / time);

}

int BasePastry::getMaxNumSiblings()
{
    return (int)floor(numberOfLeaves / 2.0);
}

int BasePastry::getMaxNumRedundantNodes()
{
    return (int)floor(numberOfLeaves);
}

NodeVector* BasePastry::findNode(const OverlayKey& key,
                                 int numRedundantNodes,
                                 int numSiblings,
                                 BaseOverlayMessage* msg)
{
    if ((numRedundantNodes > getMaxNumRedundantNodes()) ||
        (numSiblings > getMaxNumSiblings())) {

        opp_error("(Pastry::findNode()) numRedundantNodes or numSiblings "
                  "too big!");
    }
    RECORD_STATS(totalLookups++);

    NodeVector* nextHops = new NodeVector(numRedundantNodes);

    if (state != READY) {
        return nextHops;
    } else if (key.isUnspecified() || leafSet->isClosestNode(key)) {
        RECORD_STATS(responsibleLookups++);
        nextHops->add(thisNode);
    } else {
        const NodeHandle* next = &(leafSet->getDestinationNode(key));

        if (next->isUnspecified()) {
            next = &(routingTable->lookupNextHop(key));
            if (!next->isUnspecified()) {
                RECORD_STATS(routingTableLookups++);
            }
        } else {
            RECORD_STATS(responsibleLookups++);
        }

        if (next->isUnspecified()) {
            RECORD_STATS(closerNodeLookups++);
            // call findCloserNode() on all state objects
            if (optimizeLookup) {
                const NodeHandle* tmp;
                next = &(routingTable->findCloserNode(key, true));
                tmp = &(neighborhoodSet->findCloserNode(key, true));

                if ((! tmp->isUnspecified()) &&
                    (leafSet->isCloser(*tmp, key, *next))) {
                    RECORD_STATS(closerNodeLookupsFromNeighborhood++);
                    next = tmp;
                }

                tmp = &(leafSet->findCloserNode(key, true));
                if ((! tmp->isUnspecified()) &&
                    (leafSet->isCloser(*tmp, key, *next))) {
                    RECORD_STATS(closerNodeLookupsFromNeighborhood--);
                    next = tmp;
                }
            } else {
                next = &(routingTable->findCloserNode(key));

                if (next->isUnspecified()) {
                    RECORD_STATS(closerNodeLookupsFromNeighborhood++);
                    next = &(neighborhoodSet->findCloserNode(key));
                }

                if (next->isUnspecified()) {
                    RECORD_STATS(closerNodeLookupsFromNeighborhood--);
                    next = &(leafSet->findCloserNode(key));
                }
            }
        }

        iterativeJoinHook(msg, !next->isUnspecified());

        if (!next->isUnspecified()) {
            nextHops->add(*next);
        }
    }

    bool err;

    // if we're a sibling, return all numSiblings
    if ((numSiblings >= 0) && isSiblingFor(thisNode, key, numSiblings, &err)) {
        if (err == false) {
            delete nextHops;
            return  leafSet->createSiblingVector(key, numSiblings);
        }
    }

    if (/*(nextHops->size() > 0) &&*/ (numRedundantNodes > 1)) {

        //memleak... comp should be a ptr and deleted in NodeVector::~NodeVector()...
        //KeyDistanceComparator<KeyRingMetric>* comp =
        //    new KeyDistanceComparator<KeyRingMetric>( key );

        KeyDistanceComparator<KeyRingMetric> comp(key);
        //KeyDistanceComparator<KeyPrefixMetric> comp(key);
        NodeVector* additionalHops = new NodeVector( numRedundantNodes, &comp );

        routingTable->findCloserNodes(key, additionalHops);
        leafSet->findCloserNodes(key, additionalHops);
        neighborhoodSet->findCloserNodes(key, additionalHops);

        if (useRegularNextHop && (nextHops->size() > 0) &&
            (*additionalHops)[0] != (*nextHops)[0]) {
            for (uint32_t i = 0; i < additionalHops->size(); i++) {
                if ((*additionalHops)[i] != (*nextHops)[0])
                    nextHops->push_back((*additionalHops)[i]);
            }
            delete additionalHops;
        } else {
            delete nextHops;
            return additionalHops;
        }
    }
    return nextHops;
}
AbstractLookup* BasePastry::createLookup(RoutingType routingType,
                                         const BaseOverlayMessage* msg,
                                         const cObject* dummy,
                                         bool appLookup)
{
    assert(dummy == NULL);
    PastryFindNodeExtData* findNodeExt =
        new PastryFindNodeExtData("findNodeExt");

    if (msg) {
        const PastryMessage* pmsg =
            dynamic_cast<const PastryMessage*>(msg->getEncapsulatedPacket());
        if ((pmsg) && (pmsg->getPastryMsgType() == PASTRY_MSG_JOIN)) {
            const PastryJoinMessage* jmsg =
                check_and_cast<const PastryJoinMessage*>(pmsg);
            findNodeExt->setSendStateTo(jmsg->getSendStateTo());
            findNodeExt->setJoinHopCount(1);
        }
    }
    findNodeExt->setBitLength(PASTRYFINDNODEEXTDATA_L);

    AbstractLookup* newLookup = BaseOverlay::createLookup(routingType,
                                                          msg, findNodeExt,
                                                          appLookup);

    delete findNodeExt;
    return newLookup;
}

bool stateMsgIsSmaller(const PastryStateMsgHandle& hnd1,
                       const PastryStateMsgHandle& hnd2)
{
    return (hnd1.msg->getJoinHopCount() < hnd2.msg->getJoinHopCount());
}


std::ostream& operator<<(std::ostream& os, const PastryStateMsgProximity& pr)
{
    os << "PastryStateMsgProximity {" << endl;
    os << "  pr_rt {" << endl;
    for (std::vector<simtime_t>::const_iterator i = pr.pr_rt.begin();
         i != pr.pr_rt.end(); ++i) {
        os << "    " << *i << endl;
    }
    os << "  }" << endl;
    os << "  pr_ls {" << endl;
    for (std::vector<simtime_t>::const_iterator i = pr.pr_ls.begin();
         i != pr.pr_ls.end(); ++i) {
        os << "    " << *i << endl;
    }
    os << "  }" << endl;
    os << "  pr_ns {" << endl;
    for (std::vector<simtime_t>::const_iterator i = pr.pr_ns.begin();
         i != pr.pr_ns.end(); ++i) {
        os << "    " << *i << endl;
    }
    os << "  }" << endl;
    os << "}" << endl;
    return os;
}


//virtual public: distance metric
OverlayKey BasePastry::distance(const OverlayKey& x,
                                const OverlayKey& y,
                                bool useAlternative) const
{
    if (!useAlternative) return KeyRingMetric().distance(x, y);
    return KeyPrefixMetric().distance(x, y);
}
