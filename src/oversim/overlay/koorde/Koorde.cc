//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file Koorde.cc
 * @author Jochen Schenk, Ingmar Baumgart
 */
#include <IPvXAddress.h>
#include <IInterfaceTable.h>
#include <IPv4InterfaceData.h>
#include <GlobalStatistics.h>

#include "Koorde.h"

using namespace std;

namespace oversim {

Define_Module(Koorde);

void Koorde::initializeOverlay(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces
    // are registered, address auto-assignment takes place etc.
    if (stage != MIN_STAGE_OVERLAY)
        return;

    // fetch some parameters
    deBruijnDelay = par("deBruijnDelay");
    deBruijnListSize = par("deBruijnListSize");
    shiftingBits = par("shiftingBits");
    useOtherLookup = par("useOtherLookup");
    useSucList = par("useSucList");
    setupDeBruijnBeforeJoin = par("setupDeBruijnBeforeJoin");
    setupDeBruijnAtJoin = par("setupDeBruijnAtJoin");

    // init flags
    breakLookup = false;

    // some local variables
    deBruijnNumber = 0;
    deBruijnNodes = new NodeHandle[deBruijnListSize];

    // statistics
    deBruijnCount = 0;
    deBruijnBytesSent = 0;

    // add some watches
    WATCH(deBruijnNumber);
    WATCH(deBruijnNode);

    // timer messages
    deBruijn_timer = new cMessage("deBruijn_timer");

    Chord::initializeOverlay(stage);
}

Koorde::~Koorde()
{
    cancelAndDelete(deBruijn_timer);
}

void Koorde::changeState(int toState)
{
    Chord::changeState(toState);

    switch(state) {
    case INIT:
        // init de Bruijn nodes
        deBruijnNode = NodeHandle::UNSPECIFIED_NODE;

        for (int i=0; i < deBruijnListSize; i++) {
            deBruijnNodes[i] = NodeHandle::UNSPECIFIED_NODE;
        }

        updateTooltip();
        break;
    case BOOTSTRAP:
        if (setupDeBruijnBeforeJoin) {
            // setup de bruijn node before joining the ring
            cancelEvent(join_timer);
            cancelEvent(deBruijn_timer);
            scheduleAt(simTime(), deBruijn_timer);
        } else if (setupDeBruijnAtJoin) {
            cancelEvent(deBruijn_timer);
            scheduleAt(simTime(), deBruijn_timer);
        }
        break;
    case READY:
        // init de Bruijn Protocol
        cancelEvent(deBruijn_timer);
        scheduleAt(simTime(), deBruijn_timer);

        // since we don't need the fixfingers protocol in Koorde cancel timer
        cancelEvent(fixfingers_timer);
        break;
    default:
        break;
    }

}

void Koorde::handleTimerEvent(cMessage* msg)
{
    if (msg->isName("deBruijn_timer")) {
        handleDeBruijnTimerExpired();
    } else if (msg->isName("fixfingers_timer")) {
        handleFixFingersTimerExpired(msg);
    } else {
        Chord::handleTimerEvent(msg);
    }
}

bool Koorde::handleFailedNode(const TransportAddress& failed)
{
    if (!deBruijnNode.isUnspecified()) {
        if (failed == deBruijnNode) {
            deBruijnNode = deBruijnNodes[0];
            for (int i = 0; i < deBruijnNumber - 1; i++) {
                deBruijnNodes[i] = deBruijnNodes[i+1];
            }

            if (deBruijnNumber > 0) {
                deBruijnNodes[deBruijnNumber - 1] = NodeHandle::UNSPECIFIED_NODE;
                --deBruijnNumber;
            }
        } else {
            bool removed = false;
            for (int i = 0; i < deBruijnNumber - 1; i++) {
                if ((!deBruijnNodes[i].isUnspecified()) &&
                        (failed == deBruijnNodes[i])) {
                    removed = true;
                }
                if (removed ||
                        ((!deBruijnNodes[deBruijnNumber - 1].isUnspecified())
                          && failed == deBruijnNodes[deBruijnNumber - 1])) {
                    deBruijnNodes[deBruijnNumber - 1] =
                            NodeHandle::UNSPECIFIED_NODE;
                    --deBruijnNumber;
                }
            }
        }
    }

    return Chord::handleFailedNode(failed);
}

void Koorde::handleDeBruijnTimerExpired()
{
    OverlayKey lookup = thisNode.getKey() << shiftingBits;

    if (state == READY) {
        if (successorList->getSize() > 0) {
            // look for some nodes before our actual de-bruijn key
            // to have redundancy if our de-bruijn node fails
            lookup -= (successorList->getSuccessor(successorList->getSize() /
                                              2).getKey() - thisNode.getKey());
        }

        if (lookup.isBetweenR(thisNode.getKey(),
                              successorList->getSuccessor().getKey())
                || successorList->isEmpty()) {

            int sucNum = successorList->getSize();
            if (sucNum > deBruijnListSize)
                sucNum = deBruijnListSize;

            deBruijnNode = thisNode;
            for (int i = 0; i < sucNum; i++) {
                deBruijnNodes[i] = successorList->getSuccessor(i);
                deBruijnNumber = i+1;
            }

            updateTooltip();
        } else if (lookup.isBetweenR(predecessorNode.getKey(),
                                     thisNode.getKey())) {
            int sucNum = successorList->getSize();
            if ((sucNum + 1) > deBruijnListSize)
                sucNum = deBruijnListSize - 1;

            deBruijnNode = predecessorNode;
            deBruijnNodes[0] = thisNode;
            for (int i = 0; i < sucNum; i++) {
                deBruijnNodes[i+1] = successorList->getSuccessor(i);
                deBruijnNumber = i+2;
            }

            updateTooltip();
        } else {
            DeBruijnCall* call = new DeBruijnCall("DeBruijnCall");
            call->setDestKey(lookup);
            call->setBitLength(DEBRUIJNCALL_L(call));

            sendRouteRpcCall(OVERLAY_COMP, deBruijnNode,
                             call->getDestKey(), call, NULL,
                             DEFAULT_ROUTING);
        }

        cancelEvent(deBruijn_timer);
        scheduleAt(simTime() + deBruijnDelay, deBruijn_timer);
    } else {
        if (setupDeBruijnBeforeJoin || setupDeBruijnAtJoin) {
            DeBruijnCall* call = new DeBruijnCall("DeBruijnCall");
            call->setDestKey(lookup);
            call->setBitLength(DEBRUIJNCALL_L(call));

            sendRouteRpcCall(OVERLAY_COMP, bootstrapNode, call->getDestKey(),
                             call, NULL, DEFAULT_ROUTING);

            scheduleAt(simTime() + deBruijnDelay, deBruijn_timer);
        }
    }
}

#if 0
void Koorde::handleFixFingersTimerExpired(cMessage* msg)
{
    // just in case not all timers from Chord code could be canceled
}
#endif


void Koorde::handleUDPMessage(BaseOverlayMessage* msg)
{
    Chord::handleUDPMessage(msg);
}


bool Koorde::handleRpcCall(BaseCallMessage* msg)
{
    if (state == READY) {
        // delegate messages
        RPC_SWITCH_START( msg )
        RPC_DELEGATE( DeBruijn, handleRpcDeBruijnRequest );
        RPC_SWITCH_END( )

        if (RPC_HANDLED) return true;
    } else {
        EV << "[Koorde::handleRpcCall() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Received RPC call and state != READY!"
           << endl;
    }

    return Chord::handleRpcCall(msg);
}

void Koorde::handleRpcResponse(BaseResponseMessage* msg,
                               cPolymorphic* context,
                               int rpcId, simtime_t rtt)
{
    Chord::handleRpcResponse(msg, context, rpcId, rtt);

    RPC_SWITCH_START( msg )
    RPC_ON_RESPONSE( DeBruijn ) {
        handleRpcDeBruijnResponse(_DeBruijnResponse);
        EV << "[Koorde::handleRpcResponse() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    DeBruijn RPC Response received: id=" << rpcId
           << "\n    msg=" << *_DeBruijnResponse << " rtt=" << rtt
           << endl;
        break;
    }
    RPC_SWITCH_END( )
}

void Koorde::handleRpcTimeout(BaseCallMessage* msg,
                              const TransportAddress& dest,
                              cPolymorphic* context, int rpcId,
                              const OverlayKey& destKey)
{
    Chord::handleRpcTimeout(msg, dest, context, rpcId, destKey);

    RPC_SWITCH_START( msg )
    RPC_ON_CALL( DeBruijn ) {
        handleDeBruijnTimeout(_DeBruijnCall);
        EV << "[Koorde::handleRpcTimeout() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    DeBruijn RPC Call timed out: id=" << rpcId
           << "\n    msg=" << *_DeBruijnCall
           << endl;
        break;
    }
    RPC_SWITCH_END( )
}


void Koorde::handleRpcJoinResponse(JoinResponse* joinResponse)
{
    Chord::handleRpcJoinResponse(joinResponse);

    // has to be canceled in Koorde
    cancelEvent(fixfingers_timer);

    // immediate deBruijn protocol
    cancelEvent(deBruijn_timer);
    scheduleAt(simTime(), deBruijn_timer);
}


void Koorde::rpcJoin(JoinCall* joinCall)
{
    Chord::rpcJoin(joinCall);

    if (predecessorNode == successorList->getSuccessor()) {
        // second node join -> need to setup our de bruijn node
        handleDeBruijnTimerExpired();
    }
}


void Koorde::handleRpcDeBruijnRequest(DeBruijnCall* deBruijnCall)
{
    // The key lies between thisNode and its predecessor and
    // because routing the message to the predecessor of a key
    // is near to impossible we set the deBruijnNodes here
    // and the information is as actual as the predecessor pointer.
    //
    // If this is the only node in the ring, it is the temporary de bruijn
    // node for the joining node.
    if ((predecessorNode.isUnspecified() && successorList->isEmpty())
            || deBruijnCall->getDestKey().isBetweenR(predecessorNode.getKey(),
                                              thisNode.getKey())) {
        DeBruijnResponse* deBruijnResponse =
            new DeBruijnResponse("DeBruijnResponse");

        if (predecessorNode.isUnspecified()) {
            deBruijnResponse->setDBNode(thisNode);
        } else {
            deBruijnResponse->setDBNode(predecessorNode);
        }

        int sucNum = successorList->getSize() + 1;
        deBruijnResponse->setSucNum(sucNum);
        deBruijnResponse->setSucNodeArraySize(sucNum);

        deBruijnResponse->setSucNode(0, thisNode);
        for (int k = 1; k < sucNum; k++) {
            deBruijnResponse->setSucNode(k, successorList->getSuccessor(k-1));
        }
        deBruijnResponse->setBitLength(DEBRUIJNRESPONSE_L(deBruijnResponse));

        sendRpcResponse(deBruijnCall, deBruijnResponse);
    } else if (deBruijnCall->getDestKey().isBetweenR(thisNode.getKey(),
               successorList->getSuccessor().getKey())) {
        error("Koorde::handleRpcDeBruijnRequest() - unknown error.");
    } else {
        error("Koorde::handleRpcDeBruijnRequest() - "
              "Request couldn't be delivered!");
    }
}

void Koorde::handleRpcDeBruijnResponse(DeBruijnResponse* deBruijnResponse)
{
    int sucNum = deBruijnResponse->getSucNum();
    if (sucNum > deBruijnListSize)
        sucNum = deBruijnListSize;

    for (int i = 0; i < sucNum; i++) {
        deBruijnNodes[i] = deBruijnResponse->getSucNode(i);
        deBruijnNumber = i+1;
    }

    deBruijnNode = deBruijnResponse->getDBNode();

    updateTooltip();

    if (setupDeBruijnBeforeJoin && (state == BOOTSTRAP)) {
        // now that we have a valid de bruijn node it's time to join the ring
        if (!join_timer->isScheduled()) {
            scheduleAt(simTime(), join_timer);
        }
    }
}

void Koorde::handleDeBruijnTimeout(DeBruijnCall* deBruijnCall)
{
    if (setupDeBruijnBeforeJoin && (state == BOOTSTRAP)) {
        // failed to set initial de bruijn node
        // -> get a new bootstrap node and try again
        changeState(BOOTSTRAP);
        return;
    }

    cancelEvent(deBruijn_timer);
    scheduleAt(simTime(), deBruijn_timer);
}

NodeVector* Koorde::findNode(const OverlayKey& key,
                             int numRedundantNodes,
                             int numSiblings,
                             BaseOverlayMessage* msg)
{
    // TODO: return redundant nodes for iterative routing
    // TODO: try to always calculate optimal routing key (if e.g.
    //       the originator didn't have its deBruijnNode set already, the
    //       routing key may be very far away on the ring)
    NodeVector* nextHop = new NodeVector();
    KoordeFindNodeExtMessage *findNodeExt = NULL;

    if (state != READY)
        return nextHop;

    if (msg != NULL) {
        if (!msg->hasObject("findNodeExt")) {
            findNodeExt = new KoordeFindNodeExtMessage("findNodeExt");
            findNodeExt->setRouteKey(OverlayKey::UNSPECIFIED_KEY);
            findNodeExt->setStep(1);
            findNodeExt->setBitLength(KOORDEFINDNODEEXTMESSAGE_L);
            msg->addObject( findNodeExt );
        }

        findNodeExt = (KoordeFindNodeExtMessage*) msg->getObject("findNodeExt");
    }

    if (key.isUnspecified()) {
        error("Koorde::findNode() - direct Messaging is no longer in use.");
    } else if (key.isBetweenR(predecessorNode.getKey(), thisNode.getKey())) {
        // the message is destined for this node
        nextHop->push_back(thisNode);
    } else if (key.isBetweenR(thisNode.getKey(),
                              successorList->getSuccessor().getKey())){
        // the message destined for our successor
        nextHop->push_back(successorList->getSuccessor());
    } else {
        // if useOtherLookup is enabled we try to use
        // our successor list to get to the key
        if (useOtherLookup) {
            NodeHandle tmpNode = walkSuccessorList(key);
            if (tmpNode !=
                  successorList->getSuccessor(successorList->getSize() - 1)) {
                nextHop->push_back(tmpNode);
            } else {
                NodeHandle tmpHandle = findDeBruijnHop(key, findNodeExt);
                if (tmpHandle != thisNode || breakLookup) {
                    nextHop->push_back(tmpHandle);
                    breakLookup = false;
                } else {
                    return findNode(key, numRedundantNodes, numSiblings, msg);
                }
            }
        } else {
            // find next hop using either the de Bruijn node and
            // its successors or our own successors
            NodeHandle tmpHandle = findDeBruijnHop(key, findNodeExt);
            if (tmpHandle != thisNode || breakLookup) {
                nextHop->push_back(tmpHandle);
                breakLookup = false;
            } else {
                return findNode(key, numRedundantNodes, numSiblings, msg);
            }
        }
    }
    return nextHop;
}

NodeHandle Koorde::findDeBruijnHop(const OverlayKey& destKey,
                                   KoordeFindNodeExtMessage* findNodeExt)
{
    if (findNodeExt->getRouteKey().isUnspecified()) {
        if (!deBruijnNode.isUnspecified()) {
            int step;
            findNodeExt->setRouteKey(findStartKey(thisNode.getKey(),
                              successorList->getSuccessor().getKey(), destKey,
                              step));
            findNodeExt->setStep(step);
        } else {
            breakLookup = true;
            return successorList->getSuccessor();
        }
    }

    // check if the route key falls in our responsibility or
    // else forward the message to our successor
    if (findNodeExt->getRouteKey().isBetweenR(thisNode.getKey(),
        successorList->getSuccessor().getKey())) {
        if ((unsigned int)findNodeExt->getStep() > destKey.getLength())
            error("Koorde::findDeBruijnHop - Bounding error: "
                  "trying to get non existing bit out of overlay key!");

        // update the route key
        OverlayKey add = OverlayKey(destKey.getBit(destKey.getLength() -
                                                   findNodeExt->getStep()));
        for (int i = 1; i < shiftingBits; i++) {
            add = (add << 1) + OverlayKey(destKey.getBit(destKey.getLength() -
                                          findNodeExt->getStep() - i));
        }

        OverlayKey routeKey = (findNodeExt->getRouteKey()<<shiftingBits) + add;
        findNodeExt->setRouteKey(routeKey);
        findNodeExt->setStep(findNodeExt->getStep() + shiftingBits);

        if (deBruijnNode.isUnspecified()) {
            breakLookup = true;
            if (useSucList)
                return walkSuccessorList(findNodeExt->getRouteKey());
            else
                return successorList->getSuccessor();
        }

        // check if the new route key falls between our
        // de Bruijn node and its successor
        if (deBruijnNumber > 0) {
            if (findNodeExt->getRouteKey().isBetweenR(deBruijnNode.getKey(),
                                                      deBruijnNodes[0].getKey())) {
                return deBruijnNode;
            } else {
                // otherwise check if the route key falls between
                // our de Bruijn successors
                NodeHandle nextHop = walkDeBruijnList(findNodeExt->
                                                      getRouteKey());
                return nextHop;
            }
        } else {
            return deBruijnNode;
        }
    } else {
        breakLookup = true;
        // if optimization is set search the successor list and
        // de bruijn node to find "good" next hop
        if (useSucList) {
            if (deBruijnNode.isUnspecified()) {
                return walkSuccessorList(findNodeExt->getRouteKey());
            } else {
                NodeHandle tmpHandle =
                    walkSuccessorList(findNodeExt->getRouteKey());

                // todo: optimization - check complete deBruijnList
                if (deBruijnNode.getKey().isBetween(tmpHandle.getKey(),
                                               findNodeExt->getRouteKey())) {
                    return deBruijnNode;
                } else {
                    return tmpHandle;
                }
            }
        } else
            return successorList->getSuccessor();
    }
}


const NodeHandle& Koorde::walkDeBruijnList(const OverlayKey& key)
{
    if (deBruijnNumber == 0)
        return NodeHandle::UNSPECIFIED_NODE;

    for (int i = 0; i < deBruijnNumber-1; i++) {
        if (key.isBetweenR(deBruijnNodes[i].getKey(),deBruijnNodes[i+1].getKey())) {
            return deBruijnNodes[i];
        }
    }

    return deBruijnNodes[deBruijnNumber-1];
}

const NodeHandle& Koorde::walkSuccessorList(const OverlayKey& key)
{
    for (unsigned int i = 0; i < successorList->getSize()-1; i++) {
        if (key.isBetweenR(successorList->getSuccessor(i).getKey(),
                           successorList->getSuccessor(i+1).getKey())) {
            return successorList->getSuccessor(i);
        }
    }

    return successorList->getSuccessor(successorList->getSize()-1);
}

void Koorde::updateTooltip()
{
    //
    // Updates the tooltip display strings.
    //

    if (ev.isGUI()) {
        std::stringstream ttString;

        // show our predecessor, successor and de Bruijn node in tooltip
        ttString << "Pred "<< predecessorNode << endl << "This  "
                 << thisNode << endl
                 << "Suc   " << successorList->getSuccessor() << endl
                 << "DeBr " << deBruijnNode << endl;
        ttString << "List ";

        for (unsigned int i = 0; i < successorList->getSize(); i++) {
            ttString << successorList->getSuccessor(i).getIp() << " ";
        }

        ttString << endl;
        ttString << "DList ";

        for (int i = 0; i < deBruijnNumber; i++) {
            ttString << deBruijnNodes[i].getIp() << " ";
        }

        ttString << endl;

        getParentModule()->getParentModule()->
            getDisplayString().setTagArg("tt", 0, ttString.str().c_str());
        getParentModule()->getDisplayString().setTagArg("tt", 0,
                                                  ttString.str().c_str());
        getDisplayString().setTagArg("tt", 0, ttString.str().c_str());

        // draw an arrow to our current successor
        showOverlayNeighborArrow(successorList->getSuccessor(), true,
                                 "m=m,50,0,50,0;ls=red,1");
    }
}


void Koorde::finishOverlay()
{
    // statistics
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);

    if (time >= GlobalStatistics::MIN_MEASURED) {
        globalStatistics->addStdDev("Koorde: Sent DEBRUIJN Messages/s",
                                    deBruijnCount / time);
        globalStatistics->addStdDev("Koorde: Sent DEBRUIJN Bytes/s",
                                    deBruijnBytesSent / time);
    }

    Chord::finishOverlay();
}

void Koorde::recordOverlaySentStats(BaseOverlayMessage* msg)
{
    Chord::recordOverlaySentStats(msg);

    BaseOverlayMessage* innerMsg = msg;
    while (innerMsg->getType() != APPDATA &&
           innerMsg->getEncapsulatedPacket() != NULL) {
        innerMsg =
            static_cast<BaseOverlayMessage*>(innerMsg->getEncapsulatedPacket());
    }

    switch (innerMsg->getType()) {
        case RPC: {
            if ((dynamic_cast<DeBruijnCall*>(innerMsg) != NULL) ||
                (dynamic_cast<DeBruijnResponse*>(innerMsg) != NULL)) {
                RECORD_STATS(deBruijnCount++; deBruijnBytesSent +=
                                 msg->getByteLength());
            }
        break;
        }
    }
}

OverlayKey Koorde::findStartKey(const OverlayKey& startKey,
                                const OverlayKey& endKey,
                                const OverlayKey& destKey,
                                int& step)
{
    OverlayKey diffKey, newStart, tmpDest, newKey, powKey;
    int nBits;

    if (startKey == endKey)
        return startKey;

    diffKey = endKey - startKey;
    nBits = diffKey.log_2();

    if (nBits < 0) {
        nBits = 0;
    }

    while ((startKey.getLength() - nBits) % shiftingBits != 0) {
       nBits--;
   }

    step = nBits + 1;

#if 0
    // TODO: work in progress to find better start key
    uint shared;
    for (shared = 0; shared < (startKey.getLength() - nBits); shared += shiftingBits) {
        if (destKey.sharedPrefixLength(startKey << shared) >= (startKey.getLength() - nBits - shared)) {
             break;
         }
    }

    uint nBits2 = startKey.getLength() - shared;

    newStart = (startKey >> nBits2) << nBits2;

    tmpDest = destKey >> (destKey.getLength() - nBits2);
    newKey = tmpDest + newStart;

    std::cout << "startKey: " << startKey.toString(2) << endl
              << "endKey  : " << endKey.toString(2) << endl
              << "diff    : " << (endKey-startKey).toString(2) << endl
              << "newKey  : " << newKey.toString(2) << endl
              << "destKey : " << destKey.toString(2) << endl
              << "nbits   : " << nBits << endl
              << "nbits2  : " << nBits2 << endl;

    // is the new constructed route key bigger than our start key return it
    if (newKey.isBetweenR(startKey, endKey)) {
        std::cout << "HIT" << endl;
        return newKey;
    } else {
        nBits2 -= shiftingBits;
        newStart = (startKey >> nBits2) << nBits2;

        tmpDest = destKey >> (destKey.getLength() - nBits2);
        newKey = tmpDest + newStart;

        if (newKey.isBetweenR(startKey, endKey)) {
            std::cout << "startKey: " << startKey.toString(2) << endl
                      << "endKey  : " << endKey.toString(2) << endl
                      << "diff    : " << (endKey-startKey).toString(2) << endl
                      << "newKey  : " << newKey.toString(2) << endl
                      << "destKey : " << destKey.toString(2) << endl
                      << "nbits   : " << nBits << endl
                      << "nbits2  : " << nBits2 << endl;
            std::cout << "HIT2" << endl;
            return newKey;
        }
    }

    std::cout << "MISS" << endl;
#endif

    newStart = (startKey >> nBits) << nBits;

    tmpDest = destKey >> (destKey.getLength() - nBits);
    newKey = tmpDest + newStart;

    // is the new constructed route key bigger than our start key return it
    if (newKey.isBetweenR(startKey, endKey)) {
        return newKey;
    }

    // If the part of the destination key smaller than the one of
    // the original key add pow(nBits) (this is the first bit where
    // the start key and end key differ) to the new constructed key
    // and check if it's between start and end key.
    newKey += powKey.pow2(nBits);

    if (newKey.isBetweenR(startKey, endKey)) {
        return newKey;
    } else {
        // this part should not be called
        throw cRuntimeError("Koorde::findStartKey(): Invalid start key");
        return OverlayKey::UNSPECIFIED_KEY;
    }
}

void Koorde::findFriendModules()
{
    successorList = check_and_cast<ChordSuccessorList*>
                    (getParentModule()->getSubmodule("successorList"));
}

void Koorde::initializeFriendModules()
{
    // initialize successor list
    successorList->initializeList(par("successorListSize"), thisNode, this);
}

}; //namespace

