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
 * @file IterativeLookup.cc
 * @author Sebastian Mies, Ingmar Baumgart
 */

#include <iostream>
#include <assert.h>

#include <UnderlayConfigurator.h>
#include <LookupListener.h>
#include <BaseOverlay.h>
#include <GlobalStatistics.h>

#include "IterativeLookup.h"

using namespace std;

//----------------------------------------------------------------------------

LookupListener::~LookupListener()
{}

//----------------------------------------------------------------------------

AbstractLookup::~AbstractLookup()
{}

std::ostream& operator<<(std::ostream& os, const LookupEntry& n)
{
    os << "handle: " << n.handle << " source: " << n.source
       << " alreadyUsed: " << n.alreadyUsed;

    return os;
};

std::ostream& operator<<(std::ostream& os, const LookupVector& n)
{
    for (LookupVector::const_iterator i=n.begin(); i !=n.end(); i++) {
        os << *i;
        if (i+1 != n.end()) {
            os << endl;
        }
    }

    return os;
};

//----------------------------------------------------------------------------
//- Construction & Destruction -----------------------------------------------
//----------------------------------------------------------------------------
IterativeLookup::IterativeLookup(BaseOverlay* overlay,
                                 RoutingType routingType,
                                 const IterativeLookupConfiguration& config,
                                 const cPacket* findNodeExt,
                                 bool appLookup) :
overlay(overlay),
routingType(routingType),
config(config),
firstCallExt(NULL),
finished(false),
success(false),
running(false),
appLookup(appLookup)
{
    if (findNodeExt) firstCallExt = static_cast<cPacket*>(findNodeExt->dup());

    if ((config.parallelPaths > 1) && (!config.merge)) {
        throw cRuntimeError("IterativeLookup::IterativeLookup(): "
                                "config.merge must be enabled for "
                                "using parallel paths!");
    }

    if (config.verifySiblings && (!config.merge)) {
        throw cRuntimeError("IterativeLookup::IterativeLookup(): "
                                "config.merge must be enabled for "
                                "using secure lookups!");
    }


    if (config.majoritySiblings && (!config.merge)) {
        throw cRuntimeError("IterativeLookup::IterativeLookup(): "
                                "config.merge must be enabled for "
                                "using majority decision for sibling selection!");
    }

    if (config.useAllParallelResponses && (!config.merge)) {
        throw cRuntimeError("IterativeLookup::IterativeLookup(): "
                                "config.merge must be enabled if "
                                "config.useAllParallelResponses is true!");
    }
}

IterativeLookup::~IterativeLookup()
{
    stop();
    delete firstCallExt;
    overlay->removeLookup(this);

//    std::cout << "time: " << simTime() << "deleting " << this << endl;
}

void IterativeLookup::abortLookup()
{
    if (listener != NULL) {
        delete listener;
        listener = NULL;
    }
    delete this;
}

void IterativeLookup::start()
{
//    std::cout << "time: " << simTime() << " start(): node: " << overlay->getThisNode() << " this: " << this  << " key: " << key << endl;

    // init params
    successfulPaths = 0;
    finishedPaths   = 0;
    accumulatedHops = 0;

    // init flags
    finished = false;
    success  = false;
    running  = true;

    // init siblings vector
    siblings = NodeVector(numSiblings == 0 ? 1 : numSiblings, this);
    visited.clear();
    dead.clear();
    pinged.clear();

    startTime = simTime();

    // get local closest nodes
    FindNodeCall* call = createFindNodeCall(firstCallExt);
    NodeVector* nextHops = overlay->findNode(key, overlay->getMaxNumRedundantNodes(),
        (routingType == EXHAUSTIVE_ITERATIVE_ROUTING) ? -1 : numSiblings, call);

    bool err;

    setVisited(overlay->getThisNode());

    // if this node is new and no nodes are known -> stop lookup
    if (nextHops->size() == 0) {
        //std::cout << "IterativeLookup: No next hops known" << endl;
        finished = true;
        success = false;
    } else if ((numSiblings == 0)
            && overlay->isSiblingFor(overlay->getThisNode(),
                                     key, numSiblings,
                                     &err)) {
        if (overlay->getThisNode().getKey() == key) {
            addSibling(overlay->getThisNode(), true);
            success = true;
        } else {
            std::cout << "IterativeLookup: numSiblings==0 and no node with this id"
                      << endl;
            success = false;
        }
        finished = true;
    }
    // finish lookup if the key is local and siblings are needed
    else if (numSiblings != 0 && routingType != EXHAUSTIVE_ITERATIVE_ROUTING &&
             overlay->isSiblingFor(overlay->getThisNode(), key,
                                   numSiblings, &err)) {

        for (uint32_t i=0; i<nextHops->size(); i++) {
            addSibling(nextHops->at(i), true);
        }

        success = finished = true;
    }

    // if the key was local or belongs to one of our siblings we are finished
    if (finished) {
        // calls stop and finishes the lookup
        delete nextHops;
        delete call;
        delete this;
        return;
    }

    // extract find node extensions
    cPacket* findNodeExt = NULL;
    if (call->hasObject("findNodeExt"))
        findNodeExt = (cPacket*)call->removeObject("findNodeExt");
    delete call;

    // not enough nodes for all paths? -> reduce number of parallel paths
    if ((uint32_t)config.parallelPaths > nextHops->size()) {
        config.parallelPaths = nextHops->size();
    }

    // create parallel paths and distribute nodes to paths
    for (int i = 0; i < config.parallelPaths; i++) {

        // create state
        IterativePathLookup* pathLookup = new IterativePathLookup(this);
        paths.push_back(pathLookup);

        // populate next hops
        for (uint32_t k=0; (k * config.parallelPaths + i) < nextHops->size(); k++) {
            pathLookup->add(nextHops->at(k * config.parallelPaths + i));
        }

        // send initial rpcs
        pathLookup->sendRpc(config.parallelRpcs, findNodeExt);
    }


    //std::cout << "nextHops size: " << nextHops->size()
    //<< " numSiblings: " << numSiblings
    //<< " redundantNodes: " << config.redundantNodes
    //<< " thisNode " << overlay->getThisNode().ip
    //<< " nextHop " << nextHops->at(0).ip << endl;

    delete nextHops;
    delete findNodeExt;

    checkStop();
}

void IterativeLookup::stop()
{
    // only stop if running
    if (!running)
        return;

    for (uint32_t i=0; i<paths.size(); i++) {
        success |= paths[i]->success;
    }

    // cancel pending rpcs
    for (RpcInfoMap::iterator i = rpcs.begin(); i != rpcs.end(); i++) {
//	std::cout << "time: " << simTime()     << " node: " << overlay->thisNode 	  << " this: " << this << " first: " << i->first  << " nonce: " << i->second.nonce << endl;
        overlay->cancelRpcMessage(i->second.nonce);
    }
    rpcs.clear();

    // cancel pending ping rpcs
    for (PendingPings::iterator i = pendingPings.begin(); i != pendingPings.end(); i++) {
        overlay->cancelRpcMessage(i->second);
    }
    pendingPings.clear();

    // delete path lookups
    for (uint32_t i=0; i<paths.size(); i++) {
        delete paths[i];
    }
    paths.clear();

    // reset running flag
    running  = false;
    finished = true;

#if 0
    EV << "[IterativeLookup::stop() @ " << overlay->getThisNode().getIp()
                   << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
                   << "    Lookup " << (isValid() ? "succeeded\n" : "failed\n")
                   << "    (" << successfulPaths << " successful paths)\n"
                   << "    (" << finishedPaths << " finished paths)"
                   << endl;
#endif

    // inform listener
    if (listener != NULL) {
        listener->lookupFinished(this);
        listener = NULL;
    }
}

inline void IterativeLookup::checkStop()
{
    bool finishLookup = false;

    if (config.majoritySiblings && (numSiblings > 0) &&
            finishedPaths == (uint32_t)config.parallelPaths) {
        if (majoritySiblings.size() <= (uint)numSiblings) {
            // TODO: better check that all nodes have sent the same siblings
            MajoritySiblings::iterator it;
            for (it = majoritySiblings.begin(); it != majoritySiblings.end(); it++) {
                siblings.add(*it);
            }
            success = true;
            finishLookup = true;
        }
    }

    // check if there are rpcs pending or lookup finished
    if (((successfulPaths >= 1) && (numSiblings == 0) && (siblings.size() >= 1)) ||
        ((finishedPaths == (uint32_t)config.parallelPaths) &&
                (numSiblings > 0) && (pendingPings.size() == 0))) {

        for (uint32_t i=0; i<paths.size(); i++) {
            success |= paths[i]->success;
        }
        finishLookup = true;

    } else if ((rpcs.size() == 0) && (pendingPings.size() == 0)) {
        finishLookup = true;
    }

    if (finishLookup == true) {
        // TODO: should not be needed, but sometimes finishedPaths seems
        //       to be smaller than config.parallelPaths
        if (successfulPaths >= 1) {
            success = true;
        }

        if (success == false) {
            //cout << "failed: hops :" << accumulatedHops << endl;
        }

        if (success == false && retries > 0) {
            // std::cout << "IterativeLookup::checkStop(): Retry..." << endl;
            retries--;
            LookupListener* oldListener = listener;
            listener = NULL;
            stop();
            listener = oldListener;
            start();
        } else {
            delete this;
        }
    }
}

//----------------------------------------------------------------------------
//- Enhanceable methods ------------------------------------------------------
//----------------------------------------------------------------------------
IterativePathLookup* IterativeLookup::createPathLookup()
{
    return new IterativePathLookup(this);
}

FindNodeCall* IterativeLookup::createFindNodeCall(cPacket* findNodeExt)
{
    FindNodeCall* call = new FindNodeCall("FindNodeCall");
    if (appLookup) {
        call->setStatType(APP_LOOKUP_STAT);
    } else {
        call->setStatType(MAINTENANCE_STAT);
    }

    if (routingType == EXHAUSTIVE_ITERATIVE_ROUTING) {
        call->setExhaustiveIterative(true);
    } else {
        call->setExhaustiveIterative(false);
    }

    call->setLookupKey(key);
    call->setNumRedundantNodes(config.redundantNodes);
    call->setNumSiblings(numSiblings);
    if (routingType == EXHAUSTIVE_ITERATIVE_ROUTING) {
        call->setExhaustiveIterative(true);
    } else {
        call->setExhaustiveIterative(false);
    }
    call->setBitLength(FINDNODECALL_L(call));

    // duplicate extension object
    if (findNodeExt) {
        call->addObject(static_cast<cObject*>(findNodeExt->dup()));
        call->addBitLength(findNodeExt->getBitLength());
    }

    return call;
}

//----------------------------------------------------------------------------
//- Base configuration and state ---------------------------------------------
//----------------------------------------------------------------------------
//virtual public
int IterativeLookup::compare(const OverlayKey& lhs, const OverlayKey& rhs) const
{
    return overlay->distance(lhs, key).compareTo(overlay->distance(rhs, key));
}


//----------------------------------------------------------------------------
//- Siblings and visited nodes management -----------------------------------
//----------------------------------------------------------------------------
bool IterativeLookup::addSibling(const NodeHandle& handle, bool assured)
{
    bool result = false;

    EV << "[IterativeLookup::addSibling() @ " << overlay->getThisNode().getIp()
                   << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
                   << "    Adding potential sibling " << handle
                   << endl;

    majoritySiblings.insert(handle);

    if (config.verifySiblings && !assured && !getVisited(handle)) {
        // ping potential sibling for authentication
        if (siblings.isAddable(handle) && !getPinged(handle)) {
            int id = intuniform(1, 2147483647);
            int nonce = overlay->pingNode(handle, -1, 0, NULL, NULL, this, id);
            pendingPings.insert(make_pair(id, nonce));
            setPinged(handle);
        }

        return false;
    }

    if (numSiblings == 0) {
        if (handle.getKey() == key) {
            siblings.clear();
            siblings.push_back( handle );
            result = true;
        }
    } else {
        if (config.parallelPaths == 1 && !config.verifySiblings) {
            result = true;
            if (!siblings.isFull()) {
                siblings.push_back(handle);
            }
        } else {
            if (siblings.add(handle) >= 0) {
                result = true;
            }
        }
    }

    return result;
}

void IterativeLookup::setVisited(const TransportAddress& addr, bool visitedFlag)
{
    if (visitedFlag)
        this->visited.insert(addr);
    else
        this->visited.erase(addr);
}

bool IterativeLookup::getVisited(const TransportAddress& addr)
{
    return (visited.count(addr) != 0);
}

void IterativeLookup::setDead(const TransportAddress& addr)
{
    dead.insert(addr);
}

bool IterativeLookup::getDead(const TransportAddress& addr)
{
    return (dead.count(addr) != 0);
}

void IterativeLookup::setPinged(const TransportAddress& addr)
{
    pinged.insert(addr);
}

bool IterativeLookup::getPinged(const TransportAddress& addr)
{
    return (pinged.count(addr) != 0);
}


//----------------------------------------------------------------------------
//- Parallel RPC distribution ------------------------------------------------
//----------------------------------------------------------------------------
void IterativeLookup::handleRpcResponse(BaseResponseMessage* msg,
                                   cPolymorphic* context,
                                   int rpcId, simtime_t rtt)
{
    // check flags
    if (finished || !running)
        return;

    // get source, cast messages and mark node as visited
    const TransportAddress& src = msg->getSrcNode();
    FindNodeResponse* findNodeResponse = dynamic_cast<FindNodeResponse*>(msg);
    PingResponse* pingResponse = dynamic_cast<PingResponse*>(msg);
    FailedNodeResponse* failedNodeResponse =
    dynamic_cast<FailedNodeResponse*>(msg);

    if (pingResponse != NULL) {
        pendingPings.erase(rpcId);
        // add authenticated sibling
        addSibling(pingResponse->getSrcNode(), true);
    }

    // handle find node response
    if (findNodeResponse != NULL) {
        // std::cout << "time: " << simTime() << " node: " << overlay->thisNode << " this: " << this << " received rpc with nonce: " << findNodeResponse->getNonce() << " from: " << findNodeResponse->getSrcNode() << endl;

        // check if rpc info is available, no -> exit
        if (rpcs.count(src) == 0)
            return;

        // get info
        RpcInfoVector infos = rpcs[src];
        rpcs.erase(src);

        // iterate
        bool rpcHandled = false;

        for (uint32_t i=0; i<infos.size(); i++) {
            // get info
            const RpcInfo& info = infos[i];

            // do not handle finished paths
            if (info.path->finished)
                continue;

            // check if path accepts the message
            // make an exception for responses with siblings==true
            if (!rpcHandled &&
                    (info.path->accepts(info.vrpcId) ||
                    ((routingType == EXHAUSTIVE_ITERATIVE_ROUTING)
                            || (findNodeResponse->getSiblings() &&
                                config.acceptLateSiblings)))) {
                info.path->handleResponse(findNodeResponse);
                rpcHandled = true;
            } else {
                EV << "[IterativeLookup::handleRpcResponse()]\n"
                   << "    Path does not accept message with id " << info.vrpcId
                   << endl;

                info.path->handleTimeout(NULL, findNodeResponse->getSrcNode(),
                                         info.vrpcId);
            }

            // count finished and successful paths
            if (info.path->finished) {
                finishedPaths++;

                // count total number of hops
                accumulatedHops += info.path->hops;

                if (info.path->success)
                    successfulPaths++;
            }

        }
    }


    // handle failed node response
    if (failedNodeResponse != NULL) {
        cPacket* findNodeExt = NULL;
        if (failedNodeResponse->hasObject("findNodeExt")) {
            findNodeExt =
                (cPacket*)failedNodeResponse->removeObject("findNodeExt");
        }

        for (std::vector<IterativePathLookup*>::iterator i = paths.begin();
            i != paths.end(); i++) {

            (*i)->handleFailedNodeResponse(failedNodeResponse->getSrcNode(),
                                           findNodeExt,
                                           failedNodeResponse->getTryAgain());
        }
    }

    checkStop();
}


void IterativeLookup::handleRpcTimeout(BaseCallMessage* msg,
                                  const TransportAddress& dest,
                                  cPolymorphic* context, int rpcId,
                                  const OverlayKey& destKey)
{
    // check flags
    if (finished || !running)
        return;

    if (dynamic_cast<PingCall*>(msg) != NULL) {
        pendingPings.erase(rpcId);
        checkStop();
        return;
    }

    // check if rpc info is available
    if (rpcs.count(dest)==0) {
        cout << "IterativeLookup::handleRpcTimeout(): RPC Timeout, but node"
	         << " is not in rpcs structure!" << endl;
        return;
    }

    // mark the node as dead
    setDead(dest);

    RpcInfoVector infos = rpcs[dest];
    rpcs.erase(dest);

    // iterate
    for (uint32_t i=0; i < infos.size(); i++) {

        const RpcInfo& info = infos[i];

        // do not handle finished paths
        if (info.path->finished)
            continue;

        // delegate timeout
        info.path->handleTimeout(msg, dest, info.vrpcId);

        // count finished and successful paths
        if (info.path->finished) {
            finishedPaths++;

            // count total number of hops
            accumulatedHops += info.path->hops;

            if (info.path->success)
                successfulPaths++;
        }
    }
    checkStop();
}

void IterativeLookup::sendRpc(const NodeHandle& handle, FindNodeCall* call,
                              IterativePathLookup* listener, int rpcId)
{
    // check flags
    if (finished || !running) {
        delete call;
        return;
    }

    // create rpc info
    RpcInfo info;
    info.path = listener;
    info.vrpcId = rpcId;

    // send new message
    if (rpcs.count(handle) == 0) {
        RpcInfoVector newVector;

        overlay->countFindNodeCall(call);
        newVector.nonce = overlay->sendUdpRpcCall(handle, call, NULL,
                                                  -1, 0, -1, this);

        // std::cout << "time: " << simTime() << " node: " << overlay->thisNode << " new rpc with nonce: " << newVector.nonce << " to: " << handle << endl;
        rpcs[handle] = newVector;
    } else {
        EV << "[IterativeLookup::sendRpc()]\n"
           << "    RPC already sent...not sent again"
           << endl;
        delete call;
    }

    // register info
    rpcs[handle].push_back(info);
}

//----------------------------------------------------------------------------
//- AbstractLookup implementation --------------------------------------------
//----------------------------------------------------------------------------

void IterativeLookup::lookup(const OverlayKey& key, int numSiblings,
                        int hopCountMax, int retries, LookupListener* listener)
{
    EV << "[IterativeLookup::lookup() @ " << overlay->overlay->getThisNode().getIp()
       << " (" << overlay->overlay->getThisNode().getKey().toString(16) << ")]\n"
       << "    Lookup of key " << key
       << endl;

    // check flags
    if (finished || running)
        return;

    // set params
    this->key = key;
    this->numSiblings = numSiblings;
    this->hopCountMax = hopCountMax;
    this->listener = listener;
    this->retries = retries;

    if ((routingType == EXHAUSTIVE_ITERATIVE_ROUTING)
        && (numSiblings > config.redundantNodes)) {
        throw cRuntimeError("IterativeLookup::lookup(): "
                  "With EXHAUSTIVE_ITERATIVE_ROUTING numRedundantNodes "
                  "must be >= numSiblings!");
    }

    // start lookup
    start();
}

const NodeVector& IterativeLookup::getResult() const
{
    // return sibling vector
    return siblings;
}

bool IterativeLookup::isValid() const
{
    return success && finished;
}

uint32_t IterativeLookup::getAccumulatedHops() const
{
    return accumulatedHops;
}

IterativePathLookup::IterativePathLookup(IterativeLookup* lookup)
{
    this->lookup = lookup;
    this->hops = 0;
    this->step = 0;
    this->pendingRpcs = 0;
    this->finished = false;
    this->success = false;
    this->overlay = lookup->overlay;

    if (lookup->routingType == EXHAUSTIVE_ITERATIVE_ROUTING) {
        // need to add some extra space for backup nodes, if we have to
        // remove failed nodes from the nextHops vector
        this->nextHops = LookupVector(2*(lookup->config.redundantNodes),
                                      lookup);
    } else {
        this->nextHops = LookupVector((lookup->config.redundantNodes),
                                      lookup);
    }
}

IterativePathLookup::~IterativePathLookup()
{}

bool IterativePathLookup::accepts(int rpcId)
{
    if (finished) {
        return false;
    }

    // shall we use all responses, or only
    // the first one (rpcId == step)?
    if (lookup->config.useAllParallelResponses
        && lookup->config.merge) {

        return true;
    }

    return (rpcId == step);
}

void IterativePathLookup::handleResponse(FindNodeResponse* msg)
{
    if (finished)
        return;

    if (simTime() > (lookup->startTime + LOOKUP_TIMEOUT)) {
        EV << "[IterativePathLookup::handleResponse()]\n"
           << "    Iterative lookup path timed out!"
           << endl;
        finished = true;
        success = false;
        return;
    }

    const NodeHandle& source = msg->getSrcNode();
    std::map<TransportAddress, NodeHandle>::iterator oldPos;
    oldPos = oldNextHops.find(source);
    if (oldPos != oldNextHops.end()) oldNextHops.erase(oldPos);

    // don't count local hops
    if (lookup->overlay->getThisNode() != source) {
        hops++;
    }

    lookup->setVisited(source);

//  if (source.getKey() == lookup->key) {
//      cout << "received response from destination for key " << lookup->key
//           << " with isSibling = " << msg->getSiblings() << endl;
//  }

    step++;

    // decrease pending rpcs
    pendingRpcs--;

    if (msg->getClosestNodesArraySize() != 0) {
        // mode: merge or replace
        if (!lookup->config.merge) {
            nextHops.clear();
        }
    } else {
        //cout << "findNode() returned 0 nodes!" << endl;
    }

    int numNewRpcs = 0;

    // add new next hops
    for (uint32_t i=0; i < msg->getClosestNodesArraySize(); i++) {
        const NodeHandle& handle = msg->getClosestNodes(i);

        // add NodeHandle to next hops and siblings
        int pos = add(handle, source);

        // only send new rpcs if we've learned about new nodes
        if ((pos >= 0) && (pos < lookup->config.redundantNodes)) {
            numNewRpcs++;
        }

        // check if node was found
        if ((lookup->numSiblings == 0) && (handle.getKey() == lookup->key)) {

            lookup->addSibling(handle);

            // TODO: how do we resume, if the potential sibling doesn't authenticate?
            finished = true;
            success = true;
            return;
        } else {
            if (lookup->numSiblings != 0 &&
                (lookup->routingType != EXHAUSTIVE_ITERATIVE_ROUTING) &&
                msg->getSiblings()) {

                //cout << "adding sibling " << handle << endl;
                lookup->addSibling(handle);
            }
        }
    }

#if 0
    cout << "nextHops.size " << nextHops.size()
         << " find node response " << msg->getClosestNodesArraySize()
         << " config " << lookup->config.redundantNodes << endl;

    cout << "looking for " << lookup->key << endl;

    for (uint32_t i=0; i < msg->getClosestNodesArraySize(); i++) {
        cout << "find node " << msg->getClosestNodes(i) << endl;
    }

    cout << "next Hops " << nextHops << endl;
#endif

    // check if sibling lookup is finished
    if ((lookup->routingType != EXHAUSTIVE_ITERATIVE_ROUTING)
            && msg->getSiblings()
            && msg->getClosestNodesArraySize() != 0 &&
            lookup->numSiblings != 0) {

        finished = true;
        success = true;
        return;
    }

    // extract find node extension object
    cPacket* findNodeExt = NULL;
    if (msg->hasObject("findNodeExt")) {
        findNodeExt = (cPacket*)msg->removeObject("findNodeExt");
    }

    // If config.newRpcOnEveryResponse is true, send a new RPC
    // even if there was no lookup progress
    if ((numNewRpcs == 0) && lookup->config.newRpcOnEveryResponse) {
        numNewRpcs = 1;
    }

    // send next rpcs
    sendRpc(min(numNewRpcs, lookup->config.parallelRpcs), findNodeExt);

    delete findNodeExt;
}

void IterativePathLookup::sendNewRpcAfterTimeout(cPacket* findNodeExt)
{
    // two alternatives to react on a timeout
    if (lookup->config.newRpcOnEveryTimeout) {
        // always send one new RPC for every timeout
        sendRpc(1, findNodeExt);
    } else if (pendingRpcs == 0) {
        // wait until all RPCs have timed out and then send alpha new RPCs
        sendRpc(lookup->config.parallelRpcs, findNodeExt);
    }
}

void IterativePathLookup::handleTimeout(BaseCallMessage* msg,
                                   const TransportAddress& dest, int rpcId)
{
    if (finished)
        return;

    EV << "[IterativePathLookup::handleTimeout()]\n"
       << "    Timeout of RPC " << rpcId
       << endl;

    //std::cout << lookup->overlay->getThisNode() << ": Path timeout for node"
    //          << dest << endl;

    // For exhaustive-iterative remove dead nodes from nextHops vector
    // (which is also our results vector)
    if ((lookup->routingType == EXHAUSTIVE_ITERATIVE_ROUTING)
            && lookup->getDead(dest)) {
        LookupVector::iterator it = nextHops.findIterator(
                            (dynamic_cast<const NodeHandle&>(dest)).getKey());
        if (it != nextHops.end()) {
            nextHops.erase(it);
        }
    }

    std::map<TransportAddress, NodeHandle>::iterator oldPos;
    oldPos = oldNextHops.find(dest);

    // decrease pending rpcs
    pendingRpcs--;

    cPacket* findNodeExt = NULL;
    if (msg && msg->hasObject("findNodeExt")) {
        findNodeExt = static_cast<cPacket*>(
                msg->removeObject("findNodeExt"));
    }

    if (simTime() > (lookup->startTime + LOOKUP_TIMEOUT)) {
        EV << "[IterativePathLookup::handleTimeout()]\n"
           << "    Iterative lookup path timed out!"
           << endl;
        delete findNodeExt;
        finished = true;
        success = false;
        return;
    }

    if (oldPos == oldNextHops.end() || (!lookup->config.failedNodeRpcs)) {
        sendNewRpcAfterTimeout(findNodeExt);
        delete findNodeExt;
    } else {
        if (oldPos->second.isUnspecified()) {
            // TODO: handleFailedNode should always be called for local
            // nodes, independent of config.failedNodeRpcs
            // Attention: currently this method is also called,
            // if a node responded and the path doesn't accept a message
            FindNodeCall* findNodeCall = dynamic_cast<FindNodeCall*>(msg);
            // answer was from local findNode()

            if (findNodeCall && lookup->overlay->handleFailedNode(dest)) {
                NodeVector* retry = lookup->overlay->findNode(
                   findNodeCall->getLookupKey(), -1, lookup->numSiblings, msg);

                for (NodeVector::iterator i = retry->begin(); i != retry->end(); i++) {
                    nextHops.add(LookupEntry(*i, NodeHandle::UNSPECIFIED_NODE, false));
                }

                delete retry;
            }

            sendNewRpcAfterTimeout(findNodeExt);
            delete findNodeExt;

        } else {
            FailedNodeCall* call = new FailedNodeCall("FailedNodeCall");
            call->setFailedNode(dest);
            call->setBitLength(FAILEDNODECALL_L(call));
            if (findNodeExt) {
                call->addObject(findNodeExt);
                call->addBitLength(findNodeExt->getBitLength());
            }
            lookup->overlay->countFailedNodeCall(call);
            lookup->overlay->sendUdpRpcCall(oldPos->second, call, NULL,
                                            -1, 0, -1, lookup);
        }
    }
}

void IterativePathLookup::handleFailedNodeResponse(const NodeHandle& src,
                                              cPacket* findNodeExt, bool retry)
{
    if (finished) {
        return;
    }

    std::map<TransportAddress, NodeHandle>::iterator oldPos;
    for (oldPos = oldNextHops.begin(); oldPos != oldNextHops.end(); oldPos++) {
        if ((! oldPos->second.isUnspecified()) &&
            (oldPos->second == src)) break;
    }

    if (oldPos == oldNextHops.end()) {
        return;
    }

    std::map<TransportAddress, NodeHandle>::iterator oldSrcPos =
        oldNextHops.find(src);
    const NodeHandle* oldSrc = &NodeHandle::UNSPECIFIED_NODE;

    if (oldSrcPos != oldNextHops.end()) {
        oldSrc = &(oldSrcPos->second);
    }

    if (retry) {
        // FIXME: This is needed for a node to be asked again when detecting
        // a failed node. It could pose problems when parallel lookups and
        // failed node recovery are both needed at the same time!
        lookup->setVisited(src, false);

        nextHops.add(LookupEntry(src, *oldSrc, false));
    }

    oldNextHops.erase(oldPos);

    sendNewRpcAfterTimeout(findNodeExt);
}

void IterativePathLookup::sendRpc(int num, cPacket* findNodeExt)
{
    // path finished? yes -> quit
    if (finished)
        return;

    // check for maximum hop count
    if (lookup->hopCountMax && (hops >= lookup->hopCountMax)) {
        EV << "[IterativePathLookup::sendRpc()]\n"
           << "    Max hop count exceeded - lookup failed!"
           << endl;
        //cout << "[IterativePathLookup::sendRpc()]\n"
        //     << "    Max hop count exceeded - lookup failed!"
        //     << endl;

        finished = true;
        success = false;

        return;
    }

    // if strictParallelRpcs is true, limit concurrent in-flight requests
    // to config.parallelRpcs
    if (lookup->config.strictParallelRpcs) {
        num = min(num, lookup->config.parallelRpcs - pendingRpcs);
    }

    // try all remaining nodes
    if ((num == 0) && (pendingRpcs == 0)
            && !lookup->config.finishOnFirstUnchanged) {
        num = lookup->config.parallelRpcs;
        //cout << "trying all remaining nodes ("
        //     << lookup->numSiblings << ")" << endl;
    }

    // send rpc messages
    LookupVector::iterator it = nextHops.begin();
    int i = 0;
    for (LookupVector::iterator it = nextHops.begin();
         ((num > 0) && (i < lookup->config.redundantNodes)
          && (it != nextHops.end())); it++, i++)  {

        // ignore nodes to which we've already sent an RPC
        if (it->alreadyUsed || lookup->getDead(it->handle)) continue;

        // check if node has already been visited? no ->
        // TODO: doesn't work with Broose
        if ((!lookup->config.visitOnlyOnce) || (!lookup->getVisited(it->handle))) {
            // send rpc to node increase pending rpcs
            pendingRpcs++;
            num--;
            FindNodeCall* call = lookup->createFindNodeCall(findNodeExt);
            lookup->sendRpc(it->handle, call, this, step);
            oldNextHops[it->handle] = it->source;

            //cout << "Sending RPC to " << it->handle
            //     << " ( " << num << " more )"
            //     << " thisNode = " << lookup->overlay->getThisNode().getKey() << endl;

            // mark node as already used
            it->alreadyUsed = true;
        } else {
            //EV << "[IterativePathLookup::sendRpc()]\n"
            //   << "    Last next hop ("
            //   << it->handle
            //   << ") already visited."
            //   << endl;

//            std::cout << "visited:" << std::endl;
//            for (TransportAddress::Set::iterator it = lookup->visited.begin();
//            it != lookup->visited.end(); it++)
//                std::cout << *it << std::endl;
//
//            std::cout << "nextHops:" << std::endl;
//            for (NodePairVector::iterator it = nextHops.begin();
//                 it != nextHops.end(); it++)
//                std::cout << it->first << std::endl;
        }
    }

    // no rpc sent, no pending rpcs?
    // -> failed for normal lookups
    // -> exhaustive lookups are always successful
    if (pendingRpcs == 0) {
        if (lookup->routingType == EXHAUSTIVE_ITERATIVE_ROUTING) {
            int i = 0;
            for (LookupVector::iterator it = nextHops.begin();
                 ((i < lookup->config.redundantNodes)
                         && (it != nextHops.end())); it++, i++)  {
                lookup->addSibling(it->handle);
            }

            success = true;
        } else {
            success = false;
            //cout << "failed nextHops for key " << lookup->key << endl;
            //cout << nextHops << endl;
            EV << "[IterativePathLookup::sendRpc() @ " << overlay->getThisNode().getIp()
               << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
               << "    Path failed (no further nodes to query) "
               << endl;
        }

        finished = true;
    }
    //cout << endl;
}

int IterativePathLookup::add(const NodeHandle& handle, const NodeHandle& source)
{
    if (lookup->config.merge) {
        return nextHops.add(LookupEntry(handle, source, false));
    } else {
        nextHops.push_back(LookupEntry(handle, source, false));
        return (nextHops.size() - 1);
    }
}

