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
 * @file Chord.cc
 * @author Markus Mauch, Ingmar Baumgart
 */

#include <GlobalStatistics.h>
#include <Comparator.h>
#include <BootstrapList.h>
#include <GlobalParameters.h>
#include <NeighborCache.h>

#include <ChordFingerTable.h>
#include <ChordSuccessorList.h>

#include "Chord.h"

namespace oversim {

Define_Module(Chord);

Chord::Chord()
{
    stabilize_timer = fixfingers_timer = join_timer = NULL;
    fingerTable = NULL;
}


void Chord::initializeOverlay(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces
    // are registered, address auto-assignment takes place etc.
    if (stage != MIN_STAGE_OVERLAY)
        return;

    if (iterativeLookupConfig.merge == true) {
        throw cRuntimeError("Chord::initializeOverlay(): "
              "Chord doesn't work with iterativeLookupConfig.merge = true!");
    }

    // Chord provides KBR services
    kbr = true;

    // fetch some parameters
    useCommonAPIforward = par("useCommonAPIforward");
    successorListSize = par("successorListSize");
    joinRetry = par("joinRetry");
    stabilizeRetry = par("stabilizeRetry");
    joinDelay = par("joinDelay");
    stabilizeDelay = par("stabilizeDelay");
    fixfingersDelay = par("fixfingersDelay");
    checkPredecessorDelay = par("checkPredecessorDelay");
    aggressiveJoinMode = par("aggressiveJoinMode");
    extendedFingerTable = par("extendedFingerTable");
    numFingerCandidates = par("numFingerCandidates");
    proximityRouting = par("proximityRouting");
    memorizeFailedSuccessor = par("memorizeFailedSuccessor");

    // merging optimizations
    mergeOptimizationL1 = par("mergeOptimizationL1");
    mergeOptimizationL2 = par("mergeOptimizationL2");
    mergeOptimizationL3 = par("mergeOptimizationL3");
    mergeOptimizationL4 = par("mergeOptimizationL4");

    keyLength = OverlayKey::getLength();
    missingPredecessorStabRequests = 0;

    // statistics
    joinCount = 0;
    stabilizeCount = 0;
    fixfingersCount = 0;
    notifyCount = 0;
    newsuccessorhintCount = 0;
    joinBytesSent = 0;
    stabilizeBytesSent = 0;
    notifyBytesSent = 0;
    fixfingersBytesSent = 0;
    newsuccessorhintBytesSent = 0;

    failedSuccessor = TransportAddress::UNSPECIFIED_NODE;

    // find friend modules
    findFriendModules();

    // add some watches
    WATCH(predecessorNode);
    WATCH(thisNode);
    WATCH(bootstrapNode);
    WATCH(joinRetry);
    WATCH(missingPredecessorStabRequests);

    // self-messages
    join_timer = new cMessage("join_timer");
    stabilize_timer = new cMessage("stabilize_timer");
    fixfingers_timer = new cMessage("fixfingers_timer");
    checkPredecessor_timer = new cMessage("checkPredecessor_timer");
}


Chord::~Chord()
{
    // destroy self timer messages
    cancelAndDelete(join_timer);
    cancelAndDelete(stabilize_timer);
    cancelAndDelete(fixfingers_timer);
    cancelAndDelete(checkPredecessor_timer);
}



void Chord::joinOverlay()
{
    changeState(INIT);
    changeState(BOOTSTRAP);
}


void Chord::joinForeignPartition(const NodeHandle &node)
{
    Enter_Method_Silent();

    // create a join call and sent to the bootstrap node.
    JoinCall *call = new JoinCall("JoinCall");
    call->setBitLength(JOINCALL_L(call));

    RoutingType routingType = (defaultRoutingType == FULL_RECURSIVE_ROUTING ||
                               defaultRoutingType == RECURSIVE_SOURCE_ROUTING) ?
                              SEMI_RECURSIVE_ROUTING : defaultRoutingType;

    sendRouteRpcCall(OVERLAY_COMP, node, thisNode.getKey(),
                     call, NULL, routingType, joinDelay);
}


void Chord::changeState(int toState)
{
    //
    // Defines tasks to be executed when a state change occurs.
    //

    switch (toState) {
    case INIT:
        state = INIT;

        setOverlayReady(false);

        // initialize predecessor pointer
        predecessorNode = NodeHandle::UNSPECIFIED_NODE;

        // initialize finger table and successor list
        initializeFriendModules();

        updateTooltip();

        // debug message
        if (debugOutput) {
            EV << "[Chord::changeState() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Entered INIT stage"
            << endl;
        }

        getParentModule()->getParentModule()->bubble("Enter INIT state.");
        break;

    case BOOTSTRAP:
        state = BOOTSTRAP;

        // initiate bootstrap process
        cancelEvent(join_timer);
        // workaround: prevent notificationBoard from taking
        // ownership of join_timer message
        take(join_timer);
        scheduleAt(simTime(), join_timer);

        // debug message
        if (debugOutput) {
            EV << "[Chord::changeState() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Entered BOOTSTRAP stage"
            << endl;
        }
        getParentModule()->getParentModule()->bubble("Enter BOOTSTRAP state.");

        // find a new bootstrap node and enroll to the bootstrap list
        bootstrapNode = bootstrapList->getBootstrapNode();

        // is this the first node?
        if (bootstrapNode.isUnspecified()) {
            // create new cord ring
            assert(predecessorNode.isUnspecified());
            bootstrapNode = thisNode;
            changeState(READY);
            updateTooltip();
        }
        break;

    case READY:
        state = READY;

        setOverlayReady(true);

        // initiate stabilization protocol
        cancelEvent(stabilize_timer);
        scheduleAt(simTime() + stabilizeDelay, stabilize_timer);

        // initiate finger repair protocol
        cancelEvent(fixfingers_timer);
        scheduleAt(simTime() + fixfingersDelay,
                   fixfingers_timer);

        // initiate predecessor check
        cancelEvent(checkPredecessor_timer);
        if (checkPredecessorDelay > 0) {
            scheduleAt(simTime() + checkPredecessorDelay,
                       checkPredecessor_timer);
        }

        // debug message
        if (debugOutput) {
            EV << "[Chord::changeState() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Entered READY stage"
            << endl;
        }
        getParentModule()->getParentModule()->bubble("Enter READY state.");
        break;
    }
}


void Chord::handleTimerEvent(cMessage* msg)
{
    // catch JOIN timer
    if (msg == join_timer) {
        handleJoinTimerExpired(msg);
    }
    // catch STABILIZE timer
    else if (msg == stabilize_timer) {
        handleStabilizeTimerExpired(msg);
    }
    // catch FIX_FINGERS timer
    else if (msg == fixfingers_timer) {
        handleFixFingersTimerExpired(msg);
    }
    // catch CHECK_PREDECESSOR timer
    else if (msg == checkPredecessor_timer) {
        cancelEvent(checkPredecessor_timer);
        scheduleAt(simTime() + checkPredecessorDelay,
                   checkPredecessor_timer);
        if (!predecessorNode.isUnspecified()) pingNode(predecessorNode);
    }
    // unknown self message
    else {
        error("Chord::handleTimerEvent(): received self message of "
              "unknown type!");
    }
}


void Chord::handleUDPMessage(BaseOverlayMessage* msg)
{
    ChordMessage* chordMsg = check_and_cast<ChordMessage*>(msg);
    switch(chordMsg->getCommand()) {
    case NEWSUCCESSORHINT:
        handleNewSuccessorHint(chordMsg);
        break;
    default:
        error("handleUDPMessage(): Unknown message type!");
        break;
    }

    delete chordMsg;
}


bool Chord::handleRpcCall(BaseCallMessage* msg)
{
    if (state != READY) {
        EV << "[Chord::handleRpcCall() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Received RPC call and state != READY"
           << endl;
        return false;
    }

    // delegate messages
    RPC_SWITCH_START( msg )
    // RPC_DELEGATE( <messageName>[Call|Response], <methodToCall> )
    RPC_DELEGATE( Join, rpcJoin );
    RPC_DELEGATE( Notify, rpcNotify );
    RPC_DELEGATE( Stabilize, rpcStabilize );
    RPC_DELEGATE( Fixfingers, rpcFixfingers );
    RPC_SWITCH_END( )

    return RPC_HANDLED;
}

void Chord::handleRpcResponse(BaseResponseMessage* msg,
                              cPolymorphic* context, int rpcId,
                              simtime_t rtt)
{
    RPC_SWITCH_START(msg)
    RPC_ON_RESPONSE( Join ) {
        handleRpcJoinResponse(_JoinResponse);
        EV << "[Chord::handleRpcResponse() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Received a Join RPC Response: id=" << rpcId << "\n"
        << "    msg=" << *_JoinResponse << " rtt=" << rtt
        << endl;
        break;
    }
    RPC_ON_RESPONSE( Notify ) {
        handleRpcNotifyResponse(_NotifyResponse);
        EV << "[Chord::handleRpcResponse() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Received a Notify RPC Response: id=" << rpcId << "\n"
        << "    msg=" << *_NotifyResponse << " rtt=" << rtt
        << endl;
        break;
    }
    RPC_ON_RESPONSE( Stabilize ) {
        handleRpcStabilizeResponse(_StabilizeResponse);
        EV << "[Chord::handleRpcResponse() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Received a Stabilize RPC Response: id=" << rpcId << "\n"
        << "    msg=" << *_StabilizeResponse << " rtt=" << rtt
        << endl;
        break;
    }
    RPC_ON_RESPONSE( Fixfingers ) {
        handleRpcFixfingersResponse(_FixfingersResponse, SIMTIME_DBL(rtt));
        EV << "[Chord::handleRpcResponse() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Received a Fixfingers RPC Response: id=" << rpcId << "\n"
        << "    msg=" << *_FixfingersResponse << " rtt=" << rtt
        << endl;
        break;
    }
    RPC_SWITCH_END( )
}

void Chord::handleRpcTimeout(BaseCallMessage* msg,
                             const TransportAddress& dest,
                             cPolymorphic* context, int rpcId,
                             const OverlayKey&)
{
    RPC_SWITCH_START(msg)
    RPC_ON_CALL( FindNode ) {
        EV << "[Chord::handleRpcTimeout() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    FindNode RPC Call timed out: id=" << rpcId << "\n"
        << "    msg=" << *_FindNodeCall
        << endl;
        break;
    }
    RPC_ON_CALL( Join ) {
        EV << "[Chord::handleRpcTimeout() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Join RPC Call timed out: id=" << rpcId << "\n"
        << "    msg=" << *_JoinCall
        << endl;
        break;
    }
    RPC_ON_CALL( Notify ) {
        EV << "[Chord::handleRpcTimeout() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Notify RPC Call timed out: id=" << rpcId << "\n"
        << "    msg=" << *_NotifyCall
        << endl;
        if (!handleFailedNode(dest)) join();
        break;
    }
    RPC_ON_CALL( Stabilize ) {
        EV << "[Chord::handleRpcTimeout() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Stabilize RPC Call timed out: id=" << rpcId << "\n"
        << "    msg=" << *_StabilizeCall
        << endl;
        if (!handleFailedNode(dest)) join();
        break;
    }
    RPC_ON_CALL( Fixfingers ) {
        EV << "[Chord::handleRpcTimeout() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Fixfingers RPC Call timed out: id=" << rpcId << "\n"
        << "    msg=" << *_FixfingersCall
        << endl;
        break;
    }
    RPC_SWITCH_END( )
}

int Chord::getMaxNumSiblings()
{
    return successorListSize;
}

int Chord::getMaxNumRedundantNodes()
{
    return extendedFingerTable ? numFingerCandidates : 1;
}


bool Chord::isSiblingFor(const NodeHandle& node,
                         const OverlayKey& key,
                         int numSiblings,
                         bool* err)
{
    if (key.isUnspecified())
        error("Chord::isSiblingFor(): key is unspecified!");

    if (state != READY) {
        *err = true;
        return false;
    }

    if (numSiblings > getMaxNumSiblings()) {
        opp_error("Chord::isSiblingFor(): numSiblings too big!");
    }
    // set default number of siblings to consider
    if (numSiblings == -1) numSiblings = getMaxNumSiblings();

    // if this is the first and only node on the ring, it is responsible
    if ((predecessorNode.isUnspecified()) && (node == thisNode)) {
        if (successorList->isEmpty() || (node.getKey() == key)) {
            *err = false;
            return true;
        } else {
            *err = true;
            return false;
        }
    }

    if ((node == thisNode)
         && (key.isBetweenR(predecessorNode.getKey(), thisNode.getKey()))) {

        *err = false;
        return true;
    }

    NodeHandle prevNode = predecessorNode;
    NodeHandle curNode;

    for (int i = -1; i < (int)successorList->getSize();
         i++, prevNode = curNode) {

        if (i < 0) {
            curNode = thisNode;
        } else {
            curNode = successorList->getSuccessor(i);
        }

        if (node == curNode) {
            // is the message destined for curNode?
            if (key.isBetweenR(prevNode.getKey(), curNode.getKey())) {
                if (numSiblings <= ((int)successorList->getSize() - i)) {
                    *err = false;
                    return true;
                } else {
                    *err = true;
                    return false;
                }
            } else {
                // the key doesn't directly belong to this node, but
                // the node could be a sibling for this key
                if (numSiblings <= 1) {
                    *err = false;
                    return false;
                } else {
                    // In Chord we don't know if we belong to the
                    // replicaSet of one of our predecessors
                    *err = true;
                    return false;
                }
            }
        }
    }

    // node is not in our neighborSet
    *err = true;
    return false;
}

bool Chord::handleFailedNode(const TransportAddress& failed)
{
    Enter_Method_Silent();

    if (!predecessorNode.isUnspecified() && failed == predecessorNode)
        predecessorNode = NodeHandle::UNSPECIFIED_NODE;

    //TODO const reference -> trying to compare unspec NH
    TransportAddress oldSuccessor = successorList->getSuccessor();

    if (successorList->handleFailedNode(failed))
        updateTooltip();
    // check pointer for koorde
    if (fingerTable != NULL)
        fingerTable->handleFailedNode(failed);

    // if we had a ring consisting of 2 nodes and our successor seems
    // to be dead. Remove also predecessor because the successor
    // and predecessor are the same node
    if ((!predecessorNode.isUnspecified()) &&
        oldSuccessor == predecessorNode) {
        predecessorNode = NodeHandle::UNSPECIFIED_NODE;
        callUpdate(predecessorNode, false);
    }

    if (failed == oldSuccessor) {
        // schedule next stabilization process
        if (memorizeFailedSuccessor) {
            failedSuccessor = oldSuccessor;
        }
        cancelEvent(stabilize_timer);
        scheduleAt(simTime(), stabilize_timer);
    }

    if (state != READY) return true;

    if (successorList->isEmpty()) {
        // lost our last successor - cancel periodic stabilize tasks
        // and wait for rejoin
        cancelEvent(stabilize_timer);
        cancelEvent(fixfingers_timer);
    }

    return !(successorList->isEmpty());
}

NodeVector* Chord::findNode(const OverlayKey& key,
                            int numRedundantNodes,
                            int numSiblings,
                            BaseOverlayMessage* msg)
{
    bool err;
    NodeVector* nextHop;

    if (state != READY)
        return new NodeVector();

    if (successorList->isEmpty() && !predecessorNode.isUnspecified()) {
        throw new cRuntimeError("Chord: Node is READY, has a "
                                "predecessor but no successor!");
        join();
        return new NodeVector();
    }

    // if key is unspecified, the message is for this node
    if (key.isUnspecified()) {
        nextHop = new NodeVector();
        nextHop->push_back(thisNode);
    }

    // the message is destined for this node
    else if (isSiblingFor(thisNode, key, 1, &err)) {
        nextHop = new NodeVector();
        nextHop->push_back(thisNode);
        for (uint32_t i = 0; i < successorList->getSize(); i++) {
            nextHop->push_back(successorList->getSuccessor(i));
        }
        nextHop->downsizeTo(numSiblings);
    }

    // the message destined for our successor
    else if (key.isBetweenR(thisNode.getKey(),
                            successorList->getSuccessor().getKey())) {
        nextHop = new NodeVector();
        for (uint32_t i = 0; i < successorList->getSize(); i++) {
            nextHop->push_back(successorList->getSuccessor(i));
        }
        nextHop->downsizeTo(numRedundantNodes);
    }

    // find next hop with finger table and/or successor list
    else {
        nextHop = closestPreceedingNode(key);
        nextHop->downsizeTo(numRedundantNodes);
    }

    return nextHop;
}


NodeVector* Chord::closestPreceedingNode(const OverlayKey& key)
{
    NodeHandle tempHandle = NodeHandle::UNSPECIFIED_NODE;

    // find the closest preceding node in the successor list
    for (int j = successorList->getSize() - 1; j >= 0; j--) {
        // return a predecessor of the key, unless we know a node with an Id = destKey
        if (successorList->getSuccessor(j).getKey().isBetweenR(thisNode.getKey(), key)) {
            tempHandle = successorList->getSuccessor(j);
            break;
        }
    }

    if(tempHandle.isUnspecified()) {
        std::stringstream temp;
        temp << "Chord::closestPreceedingNode(): Successor list broken "
             << thisNode.getKey() << " " << key;
        throw cRuntimeError(temp.str().c_str());
    }

    NodeVector* nextHop = NULL;

    for (int i = fingerTable->getSize() - 1; i >= 0; i--) {
        // return a predecessor of the key, unless we know a node with an Id = destKey
        if (fingerTable->getFinger(i).getKey().isBetweenLR(tempHandle.getKey(), key)) {
            if(!extendedFingerTable) {
                nextHop = new NodeVector();
                nextHop->push_back(fingerTable->getFinger(i));

                EV << "[Chord::closestPreceedingNode() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    ClosestPreceedingNode: node " << thisNode
                   << " for key " << key << "\n"
                   << "    finger " << fingerTable->getFinger(i).getKey()
                   << " better than \n"
                   << "    " << tempHandle.getKey()
                   << endl;
                return nextHop;
            } else {
                return fingerTable->getFinger(i, key);
            }
        }
    }

    nextHop = new NodeVector();
    EV << "[Chord::closestPreceedingNode() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]\n"
       << "    No finger found"
       << endl;

    // if no finger is found lookup the rest of the successor list
    for (int i = successorList->getSize() - 1; i >= 0
        && nextHop->size() <= numFingerCandidates ; i--) {
        if (successorList->getSuccessor(i).getKey().isBetween(thisNode.getKey(), key)) {
            nextHop->push_back(successorList->getSuccessor(i));
        }
    }

    if (nextHop->size() != 0) {
        return nextHop;
    }

    // if this is the first and only node on the ring, it is responsible
    if ((predecessorNode.isUnspecified()) &&
        (successorList->getSuccessor() == thisNode)) {
        nextHop->push_back(thisNode);
        return nextHop;
    }

    // if there is still no node found throw an exception
    throw cRuntimeError("Error in Chord::closestPreceedingNode()!");
    return nextHop;
}

void Chord::recordOverlaySentStats(BaseOverlayMessage* msg)
{
    BaseOverlayMessage* innerMsg = msg;
    while (innerMsg->getType() != APPDATA &&
           innerMsg->getEncapsulatedPacket() != NULL) {
        innerMsg =
            static_cast<BaseOverlayMessage*>(innerMsg->getEncapsulatedPacket());
    }

    switch (innerMsg->getType()) {
        case OVERLAYSIGNALING: {
            ChordMessage* chordMsg = dynamic_cast<ChordMessage*>(innerMsg);
            switch(chordMsg->getCommand()) {
            case NEWSUCCESSORHINT:
                RECORD_STATS(newsuccessorhintCount++;
                             newsuccessorhintBytesSent += msg->getByteLength());
                break;
            }
            break;
        }

        case RPC: {
            if ((dynamic_cast<StabilizeCall*>(innerMsg) != NULL) ||
                    (dynamic_cast<StabilizeResponse*>(innerMsg) != NULL)) {
                RECORD_STATS(stabilizeCount++; stabilizeBytesSent +=
                             msg->getByteLength());
            } else if ((dynamic_cast<NotifyCall*>(innerMsg) != NULL) ||
                    (dynamic_cast<NotifyResponse*>(innerMsg) != NULL)) {
                RECORD_STATS(notifyCount++; notifyBytesSent +=
                             msg->getByteLength());
            } else if ((dynamic_cast<FixfingersCall*>(innerMsg) != NULL) ||
                    (dynamic_cast<FixfingersResponse*>(innerMsg) != NULL)) {
                RECORD_STATS(fixfingersCount++; fixfingersBytesSent +=
                             msg->getByteLength());
            } else if ((dynamic_cast<JoinCall*>(innerMsg) != NULL) ||
                    (dynamic_cast<JoinResponse*>(innerMsg) != NULL)) {
                RECORD_STATS(joinCount++; joinBytesSent += msg->getByteLength());
            }
            break;
        }

        case APPDATA:
            break;

        default:
            throw cRuntimeError("Unknown message type!");
    }
}


void Chord::finishOverlay()
{
    // remove this node from the bootstrap list
    bootstrapList->removeBootstrapNode(thisNode);

    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;

    globalStatistics->addStdDev("Chord: Sent JOIN Messages/s",
                                joinCount / time);
    globalStatistics->addStdDev("Chord: Sent NEWSUCCESSORHINT Messages/s",
                                newsuccessorhintCount / time);
    globalStatistics->addStdDev("Chord: Sent STABILIZE Messages/s",
                                stabilizeCount / time);
    globalStatistics->addStdDev("Chord: Sent NOTIFY Messages/s",
                                notifyCount / time);
    globalStatistics->addStdDev("Chord: Sent FIX_FINGERS Messages/s",
                                fixfingersCount / time);
    globalStatistics->addStdDev("Chord: Sent JOIN Bytes/s",
                                joinBytesSent / time);
    globalStatistics->addStdDev("Chord: Sent NEWSUCCESSORHINT Bytes/s",
                                newsuccessorhintBytesSent / time);
    globalStatistics->addStdDev("Chord: Sent STABILIZE Bytes/s",
                                stabilizeBytesSent / time);
    globalStatistics->addStdDev("Chord: Sent NOTIFY Bytes/s",
                                notifyBytesSent / time);
    globalStatistics->addStdDev("Chord: Sent FIX_FINGERS Bytes/s",
                                fixfingersBytesSent / time);
}



void Chord::handleJoinTimerExpired(cMessage* msg)
{
    // only process timer, if node is not bootstrapped yet
    if (state == READY)
        return;

    // enter state BOOTSTRAP
    if (state != BOOTSTRAP)
        changeState(BOOTSTRAP);

    // change bootstrap node from time to time
    joinRetry--;
    if (joinRetry == 0) {
        joinRetry = par("joinRetry");
        changeState(BOOTSTRAP);
        return;
    }

    // call JOIN RPC
    JoinCall* call = new JoinCall("JoinCall");
    call->setBitLength(JOINCALL_L(call));

    RoutingType routingType = (defaultRoutingType == FULL_RECURSIVE_ROUTING ||
                               defaultRoutingType == RECURSIVE_SOURCE_ROUTING) ?
                              SEMI_RECURSIVE_ROUTING : defaultRoutingType;

    sendRouteRpcCall(OVERLAY_COMP, bootstrapNode, thisNode.getKey(),
                     call, NULL, routingType, joinDelay);

    // schedule next bootstrap process in the case this one fails
    cancelEvent(join_timer);
    scheduleAt(simTime() + joinDelay, msg);
}


void Chord::handleStabilizeTimerExpired(cMessage* msg)
{
    if (state != READY)
        return;

    // alternative predecessor check
    if ((checkPredecessorDelay == 0) &&
        (missingPredecessorStabRequests >= stabilizeRetry)) {
        // predecessor node seems to be dead
        // remove it from the predecessor / successor lists
        //successorList->removeSuccessor(predecessorNode);
        predecessorNode = NodeHandle::UNSPECIFIED_NODE;
        missingPredecessorStabRequests = 0;
        updateTooltip();
        callUpdate(predecessorNode, false);
    }

    if (!successorList->isEmpty()) {
        // call STABILIZE RPC
        StabilizeCall* call = new StabilizeCall("StabilizeCall");
        call->setBitLength(STABILIZECALL_L(call));

        sendUdpRpcCall(successorList->getSuccessor(), call);

        missingPredecessorStabRequests++;
    }

    // check if fingers are still alive and remove unreachable finger nodes
    if (mergeOptimizationL4) {
        OverlayKey offset;
        for (uint32_t nextFinger = 0; nextFinger < thisNode.getKey().getLength();
             nextFinger++) {
            offset = OverlayKey::pow2(nextFinger);

            // send message only for non-trivial fingers
            if (offset > successorList->getSuccessor().getKey() - thisNode.getKey()) {
                if ((fingerTable->getFinger(nextFinger)).isUnspecified()) {
                    continue;
                } else {
                    pingNode(fingerTable->getFinger(nextFinger), -1, 0, NULL,
                             NULL, NULL, nextFinger);
                }
            }
        }
    }

    // schedule next stabilization process
    cancelEvent(stabilize_timer);
    scheduleAt(simTime() + stabilizeDelay, msg);
}


void Chord::handleFixFingersTimerExpired(cMessage* msg)
{
    if ((state != READY) || successorList->isEmpty())
        return;

    OverlayKey offset, lookupKey;
    for (uint32_t nextFinger = 0; nextFinger < thisNode.getKey().getLength();
         nextFinger++) {
        // calculate "n + 2^(i - 1)"
        offset = OverlayKey::pow2(nextFinger);
        lookupKey = thisNode.getKey() + offset;

        // send message only for non-trivial fingers
        if (offset > successorList->getSuccessor().getKey() - thisNode.getKey()) {
            // call FIXFINGER RPC
            FixfingersCall* call = new FixfingersCall("FixfingersCall");
            call->setFinger(nextFinger);
            call->setBitLength(FIXFINGERSCALL_L(call));

            sendRouteRpcCall(OVERLAY_COMP, lookupKey, call, NULL,
                             DEFAULT_ROUTING, fixfingersDelay);
        } else {
            // delete trivial fingers (points to the successor node)
            fingerTable->removeFinger(nextFinger);
        }
    }

    // schedule next finger repair process
    cancelEvent(fixfingers_timer);
    scheduleAt(simTime() + fixfingersDelay, msg);
}


void Chord::handleNewSuccessorHint(ChordMessage* chordMsg)
{
    NewSuccessorHintMessage* newSuccessorHintMsg =
        check_and_cast<NewSuccessorHintMessage*>(chordMsg);

    // fetch the successor's predecessor
    NodeHandle predecessor = newSuccessorHintMsg->getPreNode();

    // is the successor's predecessor a new successor for this node?
    if (predecessor.getKey().isBetween(thisNode.getKey(),
                                  successorList->getSuccessor().getKey())
        || (thisNode.getKey() == successorList->getSuccessor().getKey())) {
        // add the successor's predecessor to the successor list
        successorList->addSuccessor(predecessor);
        updateTooltip();
    }

    // if the successor node reports a new successor, put it into the
    // successor list and start stabilizing
    if (mergeOptimizationL3) {
        if (successorList->getSuccessor() == predecessor) {
            StabilizeCall *call = new StabilizeCall("StabilizeCall");
            call->setBitLength(STABILIZECALL_L(call));

            sendUdpRpcCall(predecessor, call);
        } else {
            if (successorList->getSuccessor() == newSuccessorHintMsg->
                                                               getSrcNode()) {

                StabilizeCall *call = new StabilizeCall("StabilizeCall");
                call->setBitLength(STABILIZECALL_L(call));

                sendUdpRpcCall(predecessor, call);
            }
        }
    }
}


void Chord::rpcJoin(JoinCall* joinCall)
{
    NodeHandle requestor = joinCall->getSrcNode();

    // compile successor list
    JoinResponse* joinResponse =
        new JoinResponse("JoinResponse");

    int sucNum = successorList->getSize();
    joinResponse->setSucNum(sucNum);
    joinResponse->setSucNodeArraySize(sucNum);

    for (int k = 0; k < sucNum; k++) {
        joinResponse->setSucNode(k, successorList->getSuccessor(k));
    }

    // sent our predecessor as hint to the joining node
    if (predecessorNode.isUnspecified() && successorList->isEmpty()) {
        // we are the only node in the ring
        joinResponse->setPreNode(thisNode);
    } else {
        joinResponse->setPreNode(predecessorNode);
    }

    joinResponse->setBitLength(JOINRESPONSE_L(joinResponse));

    sendRpcResponse(joinCall, joinResponse);

    if (aggressiveJoinMode) {
        // aggressiveJoinMode differs from standard join operations:
        // 1. set our predecessor pointer to the joining node
        // 2. send our old predecessor as hint in JoinResponse msgs
        // 3. send a NEWSUCCESSORHINT to our old predecessor to update
        //    its successor pointer

        // send NEWSUCCESSORHINT to our old predecessor

        if (!predecessorNode.isUnspecified()) {
            NewSuccessorHintMessage* newSuccessorHintMsg =
                new NewSuccessorHintMessage("NEWSUCCESSORHINT");
            newSuccessorHintMsg->setCommand(NEWSUCCESSORHINT);

            newSuccessorHintMsg->setSrcNode(thisNode);
            newSuccessorHintMsg->setPreNode(requestor);
            newSuccessorHintMsg->
            setBitLength(NEWSUCCESSORHINT_L(newSuccessorHintMsg));

            sendMessageToUDP(predecessorNode, newSuccessorHintMsg);
        }

        if (predecessorNode.isUnspecified() || (predecessorNode != requestor)) {
            // the requestor is our new predecessor
            NodeHandle oldPredecessor = predecessorNode;
            predecessorNode = requestor;

            // send update to application if we've got a new predecessor
            if (!oldPredecessor.isUnspecified()) {
                callUpdate(oldPredecessor, false);
            }
            callUpdate(predecessorNode, true);

        }
    }

    // if we don't have a successor, the requestor is also our new successor
    if (successorList->isEmpty())
        successorList->addSuccessor(requestor);

    updateTooltip();
}

void Chord::handleRpcJoinResponse(JoinResponse* joinResponse)
{
    // determine the numer of successor nodes to add
    int sucNum = successorListSize - 1;

    if (joinResponse->getSucNum() < successorListSize - 1) {
        sucNum = joinResponse->getSucNum();
    }

    // add successor getNode(s)
    for (int k = 0; k < sucNum; k++) {
        NodeHandle successor = joinResponse->getSucNode(k);
        successorList->addSuccessor(successor);
    }

    // the sender of this message is our new successor
    successorList->addSuccessor(joinResponse->getSrcNode());

    // in aggressiveJoinMode: use hint in JoinResponse
    // to set our new predecessor
    if (aggressiveJoinMode) {
        // it is possible that the joinResponse doesn't contain a valid
        // predecessor especially when merging two partitions
        if (!joinResponse->getPreNode().isUnspecified()) {
            if (!predecessorNode.isUnspecified()) {


                // inform the original predecessor about the new predecessor
                if (mergeOptimizationL2) {
                    NewSuccessorHintMessage* newSuccessorHintMsg =
                        new NewSuccessorHintMessage("NEWSUCCESSORHINT");
                    newSuccessorHintMsg->setCommand(NEWSUCCESSORHINT);
                    newSuccessorHintMsg->setSrcNode(thisNode);
                    newSuccessorHintMsg->setPreNode(joinResponse->getPreNode());
                    newSuccessorHintMsg->
                        setBitLength(NEWSUCCESSORHINT_L(newSuccessorHintMsg));

                    sendMessageToUDP(predecessorNode, newSuccessorHintMsg);
                }
            }

            NodeHandle oldPredecessor = predecessorNode;
            predecessorNode = joinResponse->getPreNode();

            if (!oldPredecessor.isUnspecified()
                && !joinResponse->getPreNode().isUnspecified()
                && oldPredecessor != joinResponse->getPreNode()) {
                callUpdate(oldPredecessor, false);
            }
            callUpdate(predecessorNode, true);
        }
    }

    updateTooltip();

    changeState(READY);

    // immediate stabilization protocol
    cancelEvent(stabilize_timer);
    scheduleAt(simTime(), stabilize_timer);

    // immediate finger repair protocol
    cancelEvent(fixfingers_timer);
    scheduleAt(simTime(), fixfingers_timer);
}


void Chord::rpcStabilize(StabilizeCall* call)
{
    // our predecessor seems to be alive
    if (!predecessorNode.isUnspecified() &&
        call->getSrcNode() == predecessorNode) {
        missingPredecessorStabRequests = 0;
    }

    // reply with StabilizeResponse message
    StabilizeResponse* stabilizeResponse =
        new StabilizeResponse("StabilizeResponse");
    stabilizeResponse->setPreNode(predecessorNode);
    stabilizeResponse->setBitLength(STABILIZERESPONSE_L(stabilizeResponse));

    sendRpcResponse(call, stabilizeResponse);
}

void Chord::handleRpcStabilizeResponse(StabilizeResponse* stabilizeResponse)
{
    if (state != READY) {
        return;
    }

    // fetch the successor's predecessor
    const NodeHandle& predecessor = stabilizeResponse->getPreNode();

    // is the successor's predecessor a new successor for this node?
    if ((successorList->isEmpty() ||
         predecessor.getKey().isBetween(thisNode.getKey(),
                                  successorList->getSuccessor().getKey())) &&
        (failedSuccessor.isUnspecified() || failedSuccessor != predecessor)) {
        if (successorList->isEmpty() && predecessor.isUnspecified()) {
            // successor is emptry and the sender of the response has
            // no predecessor => take the sender as new successor
            successorList->addSuccessor(stabilizeResponse->getSrcNode());
        } else {
            // add the successor's predecessor to the successor list
            successorList->addSuccessor(predecessor);
        }
        updateTooltip();
    }

    // compile NOTIFY RPC
    NotifyCall* notifyCall = new NotifyCall("NotifyCall");
    notifyCall->setBitLength(NOTIFYCALL_L(notifyCall));
    notifyCall->setFailed(failedSuccessor);
    failedSuccessor = TransportAddress::UNSPECIFIED_NODE;

    sendUdpRpcCall(successorList->getSuccessor(), notifyCall);
}

void Chord::rpcNotify(NotifyCall* call)
{
    // our predecessor seems to be alive
    if (!predecessorNode.isUnspecified() &&
        call->getSrcNode() == predecessorNode) {
        missingPredecessorStabRequests = 0;
    }

    bool newPredecessorSet = false;

    NodeHandle newPredecessor = call->getSrcNode();

    // is the new predecessor closer than the current one?
    if (predecessorNode.isUnspecified() ||
        newPredecessor.getKey().isBetween(predecessorNode.getKey(), thisNode.getKey()) ||
        (!call->getFailed().isUnspecified() &&
         call->getFailed() == predecessorNode)) {

        if ((predecessorNode.isUnspecified()) ||
            (newPredecessor != predecessorNode)) {

            // set up new predecessor
            NodeHandle oldPredecessor = predecessorNode;
            predecessorNode = newPredecessor;

            if (successorList->isEmpty()) {
                successorList->addSuccessor(newPredecessor);
            }

            newPredecessorSet = true;
            updateTooltip();

            // send update to application if we've got a new predecessor
            if (!oldPredecessor.isUnspecified()) {
                callUpdate(oldPredecessor, false);
            }
            callUpdate(predecessorNode, true);

            // inform the original predecessor about the new predecessor
            if (mergeOptimizationL1) {
                if (!oldPredecessor.isUnspecified()) {
                    NewSuccessorHintMessage *newSuccessorHintMsg =
                        new NewSuccessorHintMessage("NEWSUCCESSORHINT");
                    newSuccessorHintMsg->setCommand(NEWSUCCESSORHINT);

                    newSuccessorHintMsg->setSrcNode(thisNode);
                    newSuccessorHintMsg->setPreNode(predecessorNode);
                    newSuccessorHintMsg->
                        setBitLength(NEWSUCCESSORHINT_L(newSuccessorHintMsg));
                    sendMessageToUDP(oldPredecessor, newSuccessorHintMsg);
                }
            }


        }
    }

    // compile NOTIFY response
    NotifyResponse* notifyResponse = new NotifyResponse("NotifyResponse");

    int sucNum = successorList->getSize();
    notifyResponse->setSucNum(sucNum);
    notifyResponse->setSucNodeArraySize(sucNum);

    // can't accept the notify sender as predecessor,
    // tell it about my correct predecessor
    if (mergeOptimizationL3) {
        if (!newPredecessorSet && (predecessorNode != newPredecessor)) {

            notifyResponse->setPreNode(predecessorNode);
            notifyResponse->setPreNodeSet(false);
        } else {
            notifyResponse->setPreNodeSet(true);
        }
    }

    for (int k = 0; k < sucNum; k++) {
        notifyResponse->setSucNode(k, successorList->getSuccessor(k));
    }

    notifyResponse->setBitLength(NOTIFYRESPONSE_L(notifyResponse));

    sendRpcResponse(call, notifyResponse);
}


void Chord::handleRpcNotifyResponse(NotifyResponse* notifyResponse)
{
    if (state != READY) {
        return;
    }

    if (successorList->getSuccessor() != notifyResponse->getSrcNode()) {
        EV << "[Chord::handleRpcNotifyResponse() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    The srcNode of the received NotifyResponse is not our "
           << " current successor"
           << endl;
        return;
    }

    // if the NotifyResponse sender couldn't accept me as predecessor,
    // put its predecessor into the successor list and starts stabilizing
    if (mergeOptimizationL3) {
        if (!notifyResponse->getPreNodeSet()) {
            StabilizeCall *call = new StabilizeCall("StabilizeCall");
            call->setBitLength(STABILIZECALL_L(call));

            successorList->addSuccessor(notifyResponse->getPreNode());
            if (successorList->getSuccessor() == notifyResponse->getPreNode())
                sendUdpRpcCall(notifyResponse->getPreNode(), call);
            return;
        }
    }

    // replace our successor list by our successor's successor list
    successorList->updateList(notifyResponse);

    updateTooltip();
}


void Chord::rpcFixfingers(FixfingersCall* call)
{
    FixfingersResponse* fixfingersResponse =
        new FixfingersResponse("FixfingersResponse");

    fixfingersResponse->setSucNodeArraySize(1);
    fixfingersResponse->setSucNode(0, thisNode);

    if (extendedFingerTable) {
        fixfingersResponse->setSucNodeArraySize(((successorList->getSize() + 1
                                                < numFingerCandidates + 1)
                                                ? successorList->getSize() + 1
                                                : numFingerCandidates + 1));
        for (unsigned int i = 0;
            i < (((successorList->getSize()) < numFingerCandidates)
                 ? (successorList->getSize()) : numFingerCandidates); i++) {

            assert(!successorList->getSuccessor(i).isUnspecified());
            fixfingersResponse->setSucNode(i + 1,
                                           successorList->getSuccessor(i));
        }
    }
    fixfingersResponse->setFinger(call->getFinger());
    fixfingersResponse->setBitLength(FIXFINGERSRESPONSE_L(fixfingersResponse));

    sendRpcResponse(call, fixfingersResponse);
}


void Chord::handleRpcFixfingersResponse(FixfingersResponse* fixfingersResponse,
                                        double rtt)
{
    /*
    OverlayCtrlInfo* ctrlInfo =
        check_and_cast<OverlayCtrlInfo*>(fixfingersResponse->getControlInfo());

    RECORD_STATS(globalStatistics->recordOutVector("Chord: FIX_FINGERS response Hop Count", ctrlInfo->getHopCount()));
     */

    // set new finger pointer#
    if (!extendedFingerTable) {
        fingerTable->setFinger(fixfingersResponse->getFinger(),
                               fixfingersResponse->getSucNode(0));
    } else {
        Successors successors;
        for (unsigned int i = 0; i < fixfingersResponse->getSucNodeArraySize();
             i++) {
            if (fixfingersResponse->getSucNode(i).isUnspecified())
                continue;
            if (fixfingersResponse->getSucNode(i) == thisNode)
                break;
            successors.insert(std::make_pair(MAXTIME,
                                             fixfingersResponse->getSucNode(i)));
        }

        if (successors.size() == 0) {
            return;
        }

        fingerTable->setFinger(fixfingersResponse->getFinger(), successors);

#if 0
        if (proximityRouting || globalParameters->getTopologyAdaptation()) {
#else
        if (proximityRouting) {
#endif
            for (unsigned int i = 0;
                 i < fixfingersResponse->getSucNodeArraySize();
                 i++) {
                if (fixfingersResponse->getSucNode(i).isUnspecified())
                    continue;
                if (fixfingersResponse->getSucNode(i) == thisNode)
                    break;
                //pingNode(fixfingersResponse->getSucNode(i), -1, 0, NULL,
                //         NULL, NULL, fixfingersResponse->getFinger(),
                //         INVALID_TRANSPORT);
                Prox prox =
                    neighborCache->getProx(fixfingersResponse->getSucNode(i),
                                           NEIGHBORCACHE_DEFAULT,
                                           fixfingersResponse->getFinger(),
                                           this, NULL);
                if (prox == Prox::PROX_TIMEOUT) {
                    fingerTable->removeFinger(fixfingersResponse->getFinger());
                } else if (prox != Prox::PROX_UNKNOWN &&
                           prox != Prox::PROX_SELF) {
                    fingerTable->updateFinger(fixfingersResponse->getFinger(),
                                              fixfingersResponse->getSucNode(i),
                                              prox.proximity);
                }
            }
        }
    }
}

void Chord::proxCallback(const TransportAddress &node, int rpcId,
                         cPolymorphic *contextPointer, Prox prox)
{
    if (prox == Prox::PROX_TIMEOUT) {
        // call join dependant on return value?
        handleFailedNode(node);
        return;
    }

    fingerTable->updateFinger(rpcId, (NodeHandle&)node, prox.proximity);
}

void Chord::pingResponse(PingResponse* pingResponse, cPolymorphic* context,
                         int rpcId, simtime_t rtt)
{
    EV << "[Chord::pingResponse() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]\n"
       << "    Received a Ping RPC Response: id=" << rpcId << "\n"
       << "    msg=" << *pingResponse << " rtt=" << rtt
       << endl;

    if (rpcId != -1)
        fingerTable->updateFinger(rpcId, pingResponse->getSrcNode(), rtt);
}

void Chord::pingTimeout(PingCall* pingCall,
                        const TransportAddress& dest,
                        cPolymorphic* context, int rpcId)
{
    EV << "[Chord::pingTimeout() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]\n"
       << "    Ping RPC timeout: id=" << rpcId << endl;

    // call join dependant on return value?
    handleFailedNode(dest);
}

void Chord::findFriendModules()
{
    fingerTable = check_and_cast<ChordFingerTable*>
                  (getParentModule()->getSubmodule("fingerTable"));

    successorList = check_and_cast<ChordSuccessorList*>
                    (getParentModule()->getSubmodule("successorList"));
}


void Chord::initializeFriendModules()
{
    // initialize finger table
    fingerTable->initializeTable(thisNode.getKey().getLength(), thisNode, this);

    // initialize successor list
    successorList->initializeList(par("successorListSize"), thisNode, this);
}


void Chord::updateTooltip()
{
    if (ev.isGUI()) {
        std::stringstream ttString;

        // show our predecessor and successor in tooltip
        ttString << predecessorNode << endl << thisNode << endl
                 << successorList->getSuccessor();

        getParentModule()->getParentModule()->getDisplayString().
        setTagArg("tt", 0, ttString.str().c_str());
        getParentModule()->getDisplayString().
        setTagArg("tt", 0, ttString.str().c_str());
        getDisplayString().setTagArg("tt", 0, ttString.str().c_str());

        // draw an arrow to our current successor
        showOverlayNeighborArrow(successorList->getSuccessor(), true,
                                 "m=m,50,0,50,0;ls=red,1");
        showOverlayNeighborArrow(predecessorNode, false,
                                 "m=m,50,100,50,100;ls=green,1");
    }
}

// TODO: The following should be removed, since Chord doesn't have a simple metric
OverlayKey Chord::distance(const OverlayKey& x,
                           const OverlayKey& y,
                           bool useAlternative) const
{
    return KeyUniRingMetric().distance(x, y);
}

}; //namespace
