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
 * @file BaseRpc.cc
 * @author Sebastian Mies
 * @author Ingmar Baumgart
 * @author Bernhard Heep
 * @author Gregoire Menuel
 */

#include <vector>
#include <string>
#include <cassert>

#include <CommonMessages_m.h>
#include <UnderlayConfiguratorAccess.h>
#include <GlobalStatisticsAccess.h>
#include <NeighborCache.h>
#include <CryptoModule.h>
#include <Vivaldi.h>
#include <OverlayAccess.h>

#include "BaseRpc.h"
#include "RpcMacros.h"


//------------------------------------------------------------------------
//--- Initialization & finishing -----------------------------------------
//------------------------------------------------------------------------


//------------------------------------------------------------------------
//--- RPC Handling -------------------------------------------------------
//------------------------------------------------------------------------

BaseRpc::BaseRpc()
{
    defaultRpcListener = NULL;
    neighborCache = NULL;
    cryptoModule = NULL;
}

bool BaseRpc::internalHandleMessage(cMessage* msg)
{
    // process self-messages and RPC-timeouts
    if (msg->isSelfMessage()) {
        // process rpc self-messages
        BaseRpcMessage* rpcMessage = dynamic_cast<BaseRpcMessage*>(msg);
        if (rpcMessage != NULL) {
            internalHandleRpcMessage(rpcMessage);
            return true;
        }
        // process all other self-messages
        handleTimerEvent(msg);
        return true;
    }

    // process RPC messages
    BaseRpcMessage* rpcMessage = dynamic_cast<BaseRpcMessage*>(msg);
    if (rpcMessage != NULL) {
        internalHandleRpcMessage(rpcMessage);
        return true;
    }

    // other messages are processed by derived classes
    // (e.g. BaseOverlay / BaseApp)
    return false;
}

void BaseRpc::handleTimerEvent(cMessage* msg)
{
    // ...
}

//private
void BaseRpc::initRpcs()
{
    // set friend modules
    globalStatistics = GlobalStatisticsAccess().get();

    rpcUdpTimeout = par("rpcUdpTimeout");
    rpcKeyTimeout = par("rpcKeyTimeout");
    optimizeTimeouts = par("optimizeTimeouts");
    rpcExponentialBackoff = par("rpcExponentialBackoff");

    rpcsPending = 0;
    rpcStates.clear();

    defaultRpcListener = new RpcListener();

    //set ping cache
    numPingSent = 0;
    bytesPingSent = 0;
    numPingResponseSent = 0;
    bytesPingResponseSent = 0;

    WATCH(numPingSent);
    WATCH(bytesPingSent);
    WATCH(numPingResponseSent);
    WATCH(bytesPingResponseSent);

    // set overlay pointer
    overlay = OverlayAccess().get(this);

    // register component
    thisCompType = getThisCompType();
    overlay->registerComp(thisCompType, this);

    // get pointer to the neighborCache
    cModule *mod = getParentModule();
    while (neighborCache == NULL) {
        neighborCache = (NeighborCache*)mod->getSubmodule("neighborCache");
        mod = mod->getParentModule();
        if (!mod)
            throw cRuntimeError("BaseRpc::initRpc: "
                                "Module type contains no NeighborCache!");
    }

    // get pointer to the cryptoModule
    mod = getParentModule();
    cryptoModule = NULL;
    while (cryptoModule == NULL) {
        cryptoModule = (CryptoModule*)mod->getSubmodule("cryptoModule");
        mod = mod->getParentModule();
        if (!mod)
            throw cRuntimeError("BaseRpc::initRpc: CryptoModule not found!");
    }
}

//private
void BaseRpc::finishRpcs()
{
    cancelAllRpcs();

    // delete default rpc listener
    if (defaultRpcListener != NULL) {
        delete defaultRpcListener;
        defaultRpcListener = NULL;
    }
}

void BaseRpc::cancelAllRpcs()
{
    // stop all rpcs
    for (RpcStates::iterator i = rpcStates.begin();
        i != rpcStates.end(); i++) {
        cancelAndDelete(i->second.callMsg);
        cancelAndDelete(i->second.timeoutMsg);
        delete i->second.dest;
        i->second.dest = NULL;
        delete i->second.context;
        i->second.context = NULL;
    }
    rpcStates.clear();
}

uint32_t BaseRpc::sendRpcCall(TransportType transportType,
                              CompType destComp,
                              const TransportAddress& dest,
                              const OverlayKey& destKey,
                              BaseCallMessage* msg,
                              cPolymorphic* context,
                              RoutingType routingType,
                              simtime_t timeout,
                              int retries,
                              int rpcId,
                              RpcListener* rpcListener)
{
    // create nonce, timeout and set default parameters
    uint32_t nonce;
    do {
        nonce = intuniform(1, 2147483647);
    } while (rpcStates.count(nonce) > 0);

    if (timeout == -1) {
        switch (transportType) {
        case INTERNAL_TRANSPORT:
            timeout = 0;
            break;
        case UDP_TRANSPORT:
            if (optimizeTimeouts) {
                timeout = neighborCache->getNodeTimeout(dest);
                if (timeout == -1) timeout = rpcUdpTimeout;
            } else timeout = rpcUdpTimeout;
            break;
        case ROUTE_TRANSPORT:
            timeout = (destKey.isUnspecified() ?
                       rpcUdpTimeout :
                       rpcKeyTimeout);
            break;
        default:
            throw cRuntimeError("BaseRpc::sendRpcMessage(): "
                                "Unknown RpcTransportType!");
        }
    }

    if (rpcListener == NULL)
        rpcListener = defaultRpcListener;

    // create state
    RpcState state;
    state.id = rpcId;
    state.timeSent = simTime();
    state.dest = dest.dup();
    state.destKey = destKey;
    state.srcComp = getThisCompType();
    state.destComp = destComp;
    state.listener = rpcListener;
    state.timeoutMsg = new RpcTimeoutMessage();
    state.timeoutMsg->setNonce(nonce);
    state.retries = retries;
    state.rto = timeout;
    state.transportType = transportType;
    //state.transportType = (destKey.isUnspecified() && (dest.getSourceRouteSize() == 0)
    //        ? UDP_TRANSPORT : transportType); //test
    state.routingType = routingType;
    state.context = context;

    if (rpcStates.count(nonce) > 0)
        throw cRuntimeError("RPC nonce collision");

    // set message parameters
    msg->setNonce(nonce);
    if (transportType == ROUTE_TRANSPORT)
        msg->setSrcNode(overlay->getThisNode());
    else
        msg->setSrcNode(thisNode);
    msg->setType(RPC);

    // sign the message
    // if (transportType != INTERNAL_TRANSPORT) cryptoModule->signMessage(msg);

    // save copy of call message in RpcState
    state.callMsg = dynamic_cast<BaseCallMessage*>(msg->dup());
    assert(!msg->getEncapsulatedPacket() || !msg->getEncapsulatedPacket()->getControlInfo());

    // register state
    rpcStates[nonce] = state;

    // schedule timeout message
    if (state.rto != 0)
        scheduleAt(simTime() + state.rto, state.timeoutMsg);

    // TODO: cleanup code to have only one type for source routes
    std::vector<TransportAddress> sourceRoute;
    sourceRoute.push_back(dest);
    if (dest.getSourceRouteSize() > 0) {
        state.transportType = transportType = ROUTE_TRANSPORT;
        sourceRoute.insert(sourceRoute.begin(), dest.getSourceRoute().rend(),
                          dest.getSourceRoute().rbegin());
        // remove the original source route from the destination
        sourceRoute.back().clearSourceRoute();
    }
    sendRpcMessageWithTransport(transportType, destComp, routingType,
                                sourceRoute, destKey, msg);

    return nonce;
}


//public
void BaseRpc::cancelRpcMessage(uint32_t nonce)
{
    if (rpcStates.count(nonce)==0)
        return;
    RpcState state = rpcStates[nonce];
    rpcStates.erase(nonce);
    cancelAndDelete(state.callMsg);
    cancelAndDelete(state.timeoutMsg);
    delete state.dest;
    state.dest = NULL;
    delete state.context;
    state.context = NULL;
}

//protected
void BaseRpc::internalHandleRpcMessage(BaseRpcMessage* msg)
{
    // check if this is a rpc call message
    BaseCallMessage* rpCall = dynamic_cast<BaseCallMessage*>(msg);
    if (rpCall != NULL) {
        // verify the message signature
        //cryptoModule->verifyMessage(msg);

        OverlayCtrlInfo* overlayCtrlInfo =
            dynamic_cast<OverlayCtrlInfo*>(msg->getControlInfo());

        if (overlayCtrlInfo && overlayCtrlInfo->getSrcRoute().isUnspecified() &&
            (!overlayCtrlInfo->getLastHop().isUnspecified())) {
            overlayCtrlInfo->setSrcRoute(NodeHandle(msg->getSrcNode().getKey(),
                                               overlayCtrlInfo->getLastHop()));
        }

        bool rpcHandled = true;
        if (!handleRpcCall(rpCall)) rpcHandled = internalHandleRpcCall(rpCall);
        if (!rpcHandled) {
            EV << "[BaseRpc::internalHandleRpcMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    Error: RPC '" << msg->getFullName()<< "' was not handled"
               << endl;
            delete msg;
        }
        return;
    }

    // get nonce
    int nonce = msg->getNonce();

    // nonce known? no -> delete message and return
    if (rpcStates.count(nonce)==0) {
        EV << "[BaseRpc::internalHandleRpcMessage() @ " << thisNode.getIp()
           << " " << thisNode.getKey().toString(16) << ")]\n"
           << "    RPC: Nonce Unknown"
           << endl;
        delete msg;
        return;
    }

    // get state and remove from map
    RpcState state = rpcStates[nonce];
    rpcStates.erase(nonce);

    // is timeout message?
    if (msg->isSelfMessage() &&
        (dynamic_cast<RpcTimeoutMessage*>(msg) != NULL)) {
        // yes-> inform listener

        // retry?
        state.retries--;
        if (state.retries>=0) {
            // TODO: cleanup code to have only one type for source routes
            std::vector<TransportAddress> sourceRoute;
            sourceRoute.push_back(*state.dest);
            if (state.dest->getSourceRouteSize() > 0) {
                sourceRoute.insert(sourceRoute.begin(),
                                   state.dest->getSourceRoute().rend(),
                                   state.dest->getSourceRoute().rbegin());
                // remove the original source route from the destination
                sourceRoute.back().clearSourceRoute();
            }

            sendRpcMessageWithTransport(state.transportType, state.destComp,
                                        state.routingType,
                                        sourceRoute,
                                        state.destKey,
                                        dynamic_cast<BaseCallMessage*>
                                        (state.callMsg->dup()));

            if (rpcExponentialBackoff) {
                state.rto *= 2;
            }

            if (state.rto!=0)
                scheduleAt(simTime() + state.rto, msg);

            state.timeSent = simTime();
            rpcStates[nonce] = state;
            return;
        }
        EV << "[BaseRpc::internalHandleRpcMessage() @ " << thisNode.getIp()
           << " " << thisNode.getKey().toString(16) << ")]\n"
           << "    RPC timeout (" << state.callMsg->getName() << ")"
           << endl;

        // inform neighborcache
        if (state.transportType == UDP_TRANSPORT ||
            (!state.dest->isUnspecified() && state.destKey.isUnspecified())) {
            neighborCache->setNodeTimeout(*state.dest);
        }

        // inform listener
        if (state.listener != NULL)
            state.listener->handleRpcTimeout(state);

        // inform overlay
        internalHandleRpcTimeout(state.callMsg, *state.dest, state.context,
                                 state.id, state.destKey);
        handleRpcTimeout(state);

    } else { // no-> handle rpc response

        // verify the message signature
        //cryptoModule->verifyMessage(msg);

        OverlayCtrlInfo* overlayCtrlInfo =
            dynamic_cast<OverlayCtrlInfo*>(msg->getControlInfo());

        if (overlayCtrlInfo && overlayCtrlInfo->getSrcRoute().isUnspecified() &&
                 (!overlayCtrlInfo->getLastHop().isUnspecified())) {
             overlayCtrlInfo->setSrcRoute(NodeHandle(msg->getSrcNode().getKey(),
                                                overlayCtrlInfo->getLastHop()));
        }

        // drop responses with wrong source key
        if (state.destKey.isUnspecified()) {
            const NodeHandle* stateHandle =
                dynamic_cast<const NodeHandle*>(state.dest);
                if (stateHandle != NULL &&
                    stateHandle->getKey() != msg->getSrcNode().getKey()) {

                    EV << "[BaseRpc::internalHandleRpcMessage() @ "
                       << thisNode.getIp()
                       << " " << thisNode.getKey().toString(16) << ")]\n"
                       << "    Dropping RPC: Invalid source key"
                       << endl;

                    // restore state to trigger timeout message
                    rpcStates[nonce] = state;
                    delete msg;
                    return;
                }
        }

        // get parameters
        simtime_t rtt = simTime() - state.timeSent;
        BaseResponseMessage* response
            = dynamic_cast<BaseResponseMessage*>(msg);

        //if (state.transportType == UDP_TRANSPORT)
        //    globalStatistics->recordOutVector("BaseRpc: UDP Round Trip Time",
        //                                      rtt);

        // neighborCache/ncs stuff
        if (state.transportType == UDP_TRANSPORT ||
            (state.transportType != INTERNAL_TRANSPORT &&
             response->getCallHopCount() == 1)) {
            unsigned int ncsArraySize = response->getNcsInfoArraySize();
            if (ncsArraySize > 0) {
                std::vector<double> tempCoords(ncsArraySize);
                for (uint8_t i = 0; i < ncsArraySize; i++) {
                    tempCoords[i] = response->getNcsInfo(i);
                }
                AbstractNcsNodeInfo* coords =
                    neighborCache->getNcsAccess().createNcsInfo(tempCoords);

                OverlayCtrlInfo* ctrlInfo =
                    dynamic_cast<OverlayCtrlInfo*>(response->getControlInfo());

                neighborCache->updateNode(response->getSrcNode(), rtt,
                                          (ctrlInfo ?
                                           ctrlInfo->getSrcRoute() :
                                           NodeHandle::UNSPECIFIED_NODE),
                                           coords);
            } else {
                neighborCache->updateNode(response->getSrcNode(), rtt);
            }
        }

        // inform listener
        if (state.listener != NULL)
            state.listener->handleRpcResponse(response, state, rtt);

        // inform overlay
        internalHandleRpcResponse(response, state.context, state.id, rtt);
        handleRpcResponse(response, state, rtt);

        // delete response
        delete response->removeControlInfo();
        delete response;
    }

    // delete messages
    delete state.callMsg;
    cancelAndDelete(state.timeoutMsg);
    delete state.dest;

    // clean up pointers
    state.dest = NULL;
    state.context = NULL;
    state.callMsg = NULL;
    state.timeoutMsg = NULL;
}

//private
bool BaseRpc::internalHandleRpcCall(BaseCallMessage* msg)
{
    RPC_SWITCH_START( msg );
    RPC_DELEGATE( Ping, pingRpcCall );
    RPC_SWITCH_END( );

    return RPC_HANDLED;
}

void BaseRpc::internalHandleRpcResponse(BaseResponseMessage* msg,
                                        cPolymorphic* context,
                                        int rpcId, simtime_t rtt)
{
    // call rpc stubs
    RPC_SWITCH_START( msg );
    RPC_ON_RESPONSE( Ping ) {
        pingRpcResponse(_PingResponse, context, rpcId, rtt);
    }
    RPC_SWITCH_END( );
}

void BaseRpc::internalHandleRpcTimeout(BaseCallMessage* msg,
                                       const TransportAddress& dest,
                                       cPolymorphic* context,
                                       int rpcId, const OverlayKey& destKey)
{
    RPC_SWITCH_START( msg ) {
        RPC_ON_CALL( Ping ) {
            pingRpcTimeout(_PingCall, dest, context, rpcId);
        }
    }
    RPC_SWITCH_END( )
}

//virtual protected
bool BaseRpc::handleRpcCall(BaseCallMessage* msg)
{
    return false;
}

void BaseRpc::sendRpcResponse(TransportType transportType,
                              CompType compType,
                              const TransportAddress& dest,
                              const OverlayKey& destKey,
                              BaseCallMessage* call,
                              BaseResponseMessage* response)
{
    if (call == NULL || response == NULL) {
        throw cRuntimeError("call or response = NULL!");
    }

    // vivaldi: set coordinates and error estimation in response
    if (neighborCache->sendBackOwnCoords()) { //TODO only for directly sent msgs
        std::vector<double> nodeCoord =
            neighborCache->getNcsAccess().getOwnNcsInfo();

        response->setNcsInfoArraySize(nodeCoord.size());
        for (uint32_t i = 0; i < nodeCoord.size(); i++) {
            response->setNcsInfo(i, nodeCoord[i]);
        }
    }

    assert(transportType == INTERNAL_TRANSPORT ||
           !dest.isUnspecified() ||
           !destKey.isUnspecified());

    if (transportType == ROUTE_TRANSPORT)
        response->setSrcNode(overlay->getThisNode());
    else
        response->setSrcNode(thisNode);
    response->setType(RPC);
    response->setNonce(call->getNonce());
    response->setStatType(call->getStatType());

    RoutingType routingType = NO_OVERLAY_ROUTING;
    OverlayCtrlInfo* overlayCtrlInfo = NULL;
    if (dynamic_cast<OverlayCtrlInfo*>(call->getControlInfo())) {
        overlayCtrlInfo =
            static_cast<OverlayCtrlInfo*>(call->removeControlInfo());
        response->setCallHopCount(overlayCtrlInfo->getHopCount());
    } else {
        delete call->removeControlInfo();
        response->setCallHopCount(1); // one udp hop (?)
    }

    // source routing
    std::vector<TransportAddress> sourceRoute;
    if (overlayCtrlInfo && transportType == ROUTE_TRANSPORT) {
        routingType =
            static_cast<RoutingType>(overlayCtrlInfo->getRoutingType());
        for (uint32_t i = overlayCtrlInfo->getVisitedHopsArraySize(); i > 0; --i) {
            sourceRoute.push_back(overlayCtrlInfo->getVisitedHops(i - 1));
        }
    }

    if (sourceRoute.size() == 0) {
        // empty visited hops list => direct response
        sourceRoute.push_back(dest);
    }

    sendRpcMessageWithTransport(transportType, compType,
                                routingType, sourceRoute,
                                destKey, response);
    delete overlayCtrlInfo;
    delete call;
}

//protected
void BaseRpc::sendRpcResponse(BaseCallMessage* call,
                              BaseResponseMessage* response)
{
    const TransportAddress* destNode = &(call->getSrcNode());
    const OverlayKey* destKey = &(call->getSrcNode().getKey());

    OverlayCtrlInfo* overlayCtrlInfo =
        dynamic_cast<OverlayCtrlInfo*>(call->getControlInfo());

    // "magic" transportType selection: internal
    if (overlayCtrlInfo &&
        overlayCtrlInfo->getTransportType() == INTERNAL_TRANSPORT) {
        sendRpcResponse(INTERNAL_TRANSPORT,
                        static_cast<CompType>(overlayCtrlInfo->getSrcComp()),
                        *destNode, *destKey, call, response);
    } else {
        internalSendRpcResponse(call, response);
    }
}

void BaseRpc::sendRpcMessageWithTransport(TransportType transportType,
                                          CompType destComp,
                                          RoutingType routingType,
                                          const std::vector<TransportAddress>& sourceRoute,
                                          const OverlayKey& destKey,
                                          BaseRpcMessage* message)
{
    switch (transportType) {
    case UDP_TRANSPORT: {
        sendMessageToUDP(sourceRoute[0], message);
        break;
    }
    case ROUTE_TRANSPORT: {
        internalSendRouteRpc(message, destKey,
                             sourceRoute, routingType);
        break;
    }
    case INTERNAL_TRANSPORT: {
        cGate *destCompGate = overlay->getCompRpcGate(destComp);
        if (destCompGate == NULL) {
            throw cRuntimeError("BaseRpc::sendRpcMessageWithTransport():"
                                    " INTERNAL_RPC to unknown RpcCompType!");
        }
        OverlayCtrlInfo *overlayCtrlInfo = new OverlayCtrlInfo();
        overlayCtrlInfo->setSrcComp(getThisCompType());
        overlayCtrlInfo->setDestComp(destComp);
        overlayCtrlInfo->setTransportType(INTERNAL_TRANSPORT);
        message->setControlInfo(overlayCtrlInfo);
        sendDirect(message, destCompGate);
        break;
    }
    default:
        throw cRuntimeError("BaseRpc::sendRpcMessageWithTransport: "
                                "invalid transportType!");
        break;
    }
}

// ping RPC stuff
void BaseRpc::pingResponse(PingResponse* response, cPolymorphic* context,
                           int rpcId, simtime_t rtt)
{
}

void BaseRpc::pingTimeout(PingCall* call, const TransportAddress& dest,
                          cPolymorphic* context, int rpcId)
{
}

void BaseRpc::pingRpcCall(PingCall* call)
{
    std::string pongName(call->getName());
    if (pongName == "PING")
        pongName = "PONG";
    else {
        pongName = "PONG: [ ";
        pongName += call->getName();
        pongName += " ]";
    }

    PingResponse* response = new PingResponse(pongName.c_str());
    response->setBitLength(PINGRESPONSE_L(response));
    RECORD_STATS(numPingResponseSent++; bytesPingResponseSent +=
        response->getByteLength());

    sendRpcResponse(call, response );
}

void BaseRpc::pingRpcResponse(PingResponse* response,
                              cPolymorphic* context, int rpcId, simtime_t rtt)
{
    pingResponse(response, context, rpcId, rtt);
}

void BaseRpc::pingRpcTimeout(PingCall* pingCall,
                             const TransportAddress& dest,
                             cPolymorphic* context,
                             int rpcId)
{
    pingTimeout(pingCall, dest, context, rpcId);
}

int BaseRpc::pingNode(const TransportAddress& dest, simtime_t timeout,
                       int retries, cPolymorphic* context,
                       const char* caption, RpcListener* rpcListener,
                       int rpcId, TransportType transportType)
{
    PingCall* call = new PingCall(caption);
    call->setBitLength(PINGCALL_L(call));
    RECORD_STATS(numPingSent++; bytesPingSent += call->getByteLength());

    if (transportType == UDP_TRANSPORT ||
        (transportType != ROUTE_TRANSPORT &&
         getThisCompType() == OVERLAY_COMP)) {
        return sendUdpRpcCall(dest, call, context, timeout, retries, rpcId,
                       rpcListener);
    } else {
        return sendRouteRpcCall(getThisCompType(), dest, call, context,
                         DEFAULT_ROUTING, timeout, retries, rpcId,
                         rpcListener);
    }
}


