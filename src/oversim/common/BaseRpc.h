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
 * @file BaseRpc.h
 * @author Bernhard Heep
 * @author Ingmar Baumgart
 * @author Sebastian Mies
 * @author Gregoire Menuel
 */

#ifndef __BASERPC_H_
#define __BASERPC_H_

#include <oversim_mapset.h>

#include <omnetpp.h>

#include <RpcState.h>
#include <RpcListener.h>
#include <RpcMacros.h>

#include <ProxNodeHandle.h>

class UnderlayConfigurator;
class GlobalStatistics;
class GlobalParameters;
class NeighborCache;
class CryptoModule;
class WaitingContexts;
class BaseOverlay;

class ProxListener;

/**
 * Base class for RPCs
 *
 * Base class for RPCs.
 *
 * @author Sebastian Mies
 * @author Ingmar Baumgart
 * @author Bernhard Heep
 * @author Gregoire Menuel
 */
class BaseRpc : public RpcListener,
                public cSimpleModule
{
public:

    BaseRpc();

    /**
     * Returns the NodeHandle of this node.
     *
     * @return the NodeHandle of this node.
     */
    const NodeHandle& getThisNode() { return thisNode; };

    simtime_t getUdpTimeout() { return rpcUdpTimeout; };

protected:

    // overlay identity
    NodeHandle thisNode;   /**< NodeHandle to this node */

    BaseOverlay* overlay;

    // overlay common parameters
    bool debugOutput;           /**< debug output ? */

    // references to global modules
    GlobalStatistics* globalStatistics;  /**< pointer to GlobalStatistics module in this node */

    /**
     * Handles internal rpc requests.<br>
     *
     * This method is used to implement basic functionality in
     * the BaseRpc.
     *
     * @param msg The call message
     * @return bool true, if call has been handled.
     */
    virtual bool internalHandleRpcCall(BaseCallMessage* msg);

    /**
    * Handles rpc responses internal in base classes<br>
    *
    * This method is used to implement basic functionality in
    * the BaseRpc.
    *
    * @param msg The call message
    * @param context Pointer to an optional state object. The object
    *                has to be handled/deleted by the
    *                internalHandleRpcResponse() code
    * @param rpcId The ID of the call
    * @param rtt the time between sending the call and receiving the response
    */
    virtual void internalHandleRpcResponse(BaseResponseMessage* msg,
                                           cPolymorphic* context, int rpcId,
                                           simtime_t rtt);

    /**
    * Handles rpc timeouts internal in base classes<br>
    *
    * This method is used to implement basic functionality in
    * the BaseRpc.
    *
    * @param msg The call message
    * @param dest The node that did not response
    * @param context Pointer to an optional state object. The object
    *                has to be handled/deleted by the
    *                internalHandleRpcResponse() code
    * @param rpcId The ID of the call
    * @param destKey The key of the call if used
    * @return bool true, if call has been handled.
    * @todo return bool?
    */
    virtual void internalHandleRpcTimeout(BaseCallMessage* msg,
                                          const TransportAddress& dest,
                                          cPolymorphic* context,
                                          int rpcId, const OverlayKey& destKey);
    /**
     * Initializes Remote-Procedure state.
     */
    void initRpcs();

    /**
     * Deinitializes Remote-Procedure state.
     */
    void finishRpcs();

    /**
     * Handles incoming rpc messages and delegates them to the
     * corresponding listeners or handlers.
     *
     * @param msg The message to handle.
     */
    virtual void internalHandleRpcMessage(BaseRpcMessage* msg);

    /**
     * Routes a Remote-Procedure-Call message to an OverlayKey.<br>
     *
     * If no timeout is provided, a default value of
     * globalParameters.rpcUdpTimeout for
     * underlay and globalParameters.rpcKeyTimeout for a overlay rpc
     * is used. After a timeout the message gets retransmitted for
     * at maximum retries times. The destKey
     * attribute is kept untouched.
     *
     * @param destComp The destination component
     * @param dest Destination node handle (if specified, used as first hop)
     * @param destKey Destination OverlayKey (if unspecified, the message will
     *                be sent to dest using the overlay's UDP port)
     * @param msg RPC Call Message
     * @param context a pointer to an arbitrary cPolymorphic object,
     *        which can be used to store additional state
     * @param routingType KBR routing type
     * @param timeout RPC timeout in seconds (-1=use default value, 0=no timeout)
     * @param retries How often we try to resent rpc call, if it gets lost
     * @param rpcId RPC id
     * @param rpcListener RPC Listener
     * @return The nonce of the RPC
     */
    inline uint32_t sendRouteRpcCall(CompType destComp,
                                     const TransportAddress& dest,
                                     const OverlayKey& destKey,
                                     BaseCallMessage* msg,
                                     cPolymorphic* context = NULL,
                                     RoutingType routingType = DEFAULT_ROUTING,
                                     simtime_t timeout = -1,
                                     int retries = 0,
                                     int rpcId = -1,
                                     RpcListener* rpcListener = NULL)
    {
        if (dest.isUnspecified() && destKey.isUnspecified())
            opp_error("BaseRpc::sendRouteRpcCall() with both key and "
                      "transportAddress unspecified!");
        return sendRpcCall(ROUTE_TRANSPORT, destComp, dest, destKey, msg,
                           context, routingType, timeout, retries,
                           rpcId, rpcListener);
    }

    /**
     * Routes a Remote-Procedure-Call message to an OverlayKey.<br>
     *
     * If no timeout is provided, a default value of
     * globalParameters.rpcUdpTimeout for
     * underlay and globalParameters.rpcKeyTimeout for a overlay rpc
     * is used. After a timeout the message gets retransmitted for
     * at maximum retries times. The destKey
     * attribute is kept untouched.
     *
     * @param destComp The destination component
     * @param destKey Destination OverlayKey
     * @param msg RPC Call Message
     * @param context a pointer to an arbitrary cPolymorphic object,
     *        which can be used to store additional state
     * @param routingType KBR routing type
     * @param timeout RPC timeout in seconds (-1=use default value, 0=no timeout)
     * @param retries How often we try to resent rpc call, if it gets lost
     * @param rpcId RPC id
     * @param rpcListener RPC Listener
     * @return The nonce of the RPC
     */
    inline uint32_t sendRouteRpcCall(CompType destComp,
                                     const OverlayKey& destKey,
                                     BaseCallMessage* msg,
                                     cPolymorphic* context = NULL,
                                     RoutingType routingType = DEFAULT_ROUTING,
                                     simtime_t timeout = -1,
                                     int retries = 0,
                                     int rpcId = -1,
                                     RpcListener* rpcListener = NULL)
    {
        return sendRpcCall(ROUTE_TRANSPORT, destComp,
                           TransportAddress::UNSPECIFIED_NODE,
                           destKey, msg, context, routingType, timeout,
                           retries, rpcId, rpcListener);
    }

    /**
     * Sends a Remote-Procedure-Call message using the overlay's UDP port<br>
     * This replaces ROUTE_DIRECT calls!
     *
     * If no timeout is provided, a default value of
     * globalParameters.rpcUdpTimeout for
     * underlay and globalParameters.rpcKeyTimeout for a overlay rpc
     * is used. After a timeout the message gets retransmitted for
     * at maximum retries times. The destKey
     * attribute is kept untouched.
     *
     * @param destComp The destination component
     * @param dest Destination node handle (may contain a source route)
     * @param msg RPC Call Message
     * @param context a pointer to an arbitrary cPolymorphic object,
     *        which can be used to store additional state
     * @param routingType KBR routing type
     * @param timeout RPC timeout
     * @param retries How often we try to resent rpc call, if it gets lost
     * @param rpcId RPC id
     * @param rpcListener RPC Listener
     * @return The nonce of the RPC
     */
    inline uint32_t sendRouteRpcCall(CompType destComp,
                                     const TransportAddress& dest,
                                     BaseCallMessage* msg,
                                     cPolymorphic* context = NULL,
                                     RoutingType routingType = DEFAULT_ROUTING,
                                     simtime_t timeout = -1,
                                     int retries = 0,
                                     int rpcId = -1,
                                     RpcListener* rpcListener = NULL)
    {
        return sendRpcCall(ROUTE_TRANSPORT, destComp, dest,
                           OverlayKey::UNSPECIFIED_KEY, msg, context,
                           routingType, timeout, retries, rpcId, rpcListener);
    }

    /**
     * Sends a Remote-Procedure-Call message to the underlay<br>
     *
     * If no timeout is provided, a default value of
     * globalParameters.rpcUdpTimeout for
     * underlay and globalParameters.rpcKeyTimeout for a overlay rpc
     * is used. After a timeout the message gets retransmitted for
     * at maximum retries times. The destKey
     * attribute is kept untouched.
     *
     * @param dest Destination node handle (may contain a source route)
     * @param msg RPC Call Message
     * @param context a pointer to an arbitrary cPolymorphic object,
     *        which can be used to store additional state
     * @param timeout RPC timeout in seconds (-1=use default value, 0=no timeout)
     * @param retries How often we try to resent rpc call, if it gets lost
     * @param rpcId RPC id
     * @param rpcListener RPC Listener
     * @return The nonce of the RPC
     */
    inline uint32_t sendUdpRpcCall(const TransportAddress& dest,
                                   BaseCallMessage* msg,
                                   cPolymorphic* context = NULL,
                                   simtime_t timeout = -1,
                                   int retries = 0, int rpcId = -1,
                                   RpcListener* rpcListener = NULL)
    {
        return sendRpcCall(UDP_TRANSPORT, INVALID_COMP, dest,
                           OverlayKey::UNSPECIFIED_KEY, msg, context,
                           NO_OVERLAY_ROUTING, timeout, retries, rpcId,
                           rpcListener);
    }

    /**
     * Sends an internal Remote-Procedure-Call between two tiers<br>
     *
     * If no timeout is provided, a default value of
     * globalParameters.rpcUdpTimeout for
     * underlay and globalParameters.rpcKeyTimeout for a overlay rpc
     * is used. After a timeout the message gets retransmitted for
     * at maximum retries times. The destKey
     * attribute is kept untouched.
     *
     * @param destComp Destination component
     * @param msg RPC Call Message
     * @param context a pointer to an arbitrary cPolymorphic object,
     *        which can be used to store additional state
     * @param timeout RPC timeout in seconds (-1=use default value, 0=no timeout)
     * @param retries How often we try to resent rpc call, if it gets lost
     * @param rpcId RPC id
     * @param rpcListener RPC Listener
     * @return The nonce of the RPC
     */
    inline uint32_t sendInternalRpcCall(CompType destComp,
                                        BaseCallMessage* msg,
                                        cPolymorphic* context = NULL,
                                        simtime_t timeout = -1,
                                        int retries = 0,
                                        int rpcId = -1,
                                        RpcListener* rpcListener = NULL)
    {
        return sendRpcCall(INTERNAL_TRANSPORT, destComp,
                           TransportAddress::UNSPECIFIED_NODE,
                           OverlayKey::UNSPECIFIED_KEY, msg, context,
                           NO_OVERLAY_ROUTING, timeout, retries, rpcId,
                           rpcListener);
    }

    /**
     * Cancels a Remote-Procedure-Call.
     *
     * @param nonce The nonce of the RPC
     */
    void cancelRpcMessage(uint32_t nonce);

    /**
     * Cancels all RPCs.
     */
    void cancelAllRpcs();

    /**
     * Send Remote-Procedure response message and deletes call
     * message.
     *
     * @param transportType The transport used for this RPC
     * @param destComp Destination component
     * @param dest The TransportAddress of the destination
     *             (hint for ROUTE_TRANSPORT)
     * @param destKey The destination key for a ROUTE_TRANSPORT
     * @param call The corresponding call message to the response
     * @param response The response message
     */
    void sendRpcResponse(TransportType transportType,
                         CompType destComp,
                         const TransportAddress &dest,
                         const OverlayKey &destKey,
                         BaseCallMessage* call,
                         BaseResponseMessage* response);

    /**
     * Send Remote-Procedure response message to UDP and deletes call
     * message.
     *
     * @param call The corresponding call message to the response
     * @param response The response message
     */
    void sendRpcResponse(BaseCallMessage* call,
                         BaseResponseMessage* response);

    /**
     * ping a node by its TransportAddress
     *
     * Statistics are collected by this method.
     *
     * @param dest the node to ping
     * @param timeout RPC timeout
     * @param retries how often to retry after timeout
     * @param context a pointer to an arbitrary cPolymorphic object,
     *        which can be used to store additional state
     * @param caption special name for the ping call (instead of "PING")
     * @param rpcListener RPC Listener
     * @param rpcId RPC id
     * @param transportType The transport used for this RPC
     * @return the nonce of the sent ping call
     */
    int pingNode(const TransportAddress& dest,
                  simtime_t timeout = -1,
                  int retries = 0,
                  cPolymorphic* context = NULL,
                  const char* caption = "PING",
                  RpcListener* rpcListener = NULL,
                  int rpcId = -1,
                  TransportType transportType = INVALID_TRANSPORT);

    /**
     * Processes Remote-Procedure-Call invocation messages.<br>
     *
     * This method should be overloaded when the overlay provides
     * RPC functionality.
     *
     * @return true, if rpc has been handled
     */
    virtual bool handleRpcCall(BaseCallMessage* msg);

    /**
     * Return the component type of this module
     *
     * This method is overloaded by BaseOverlay/BaseApp and returns
     * the appropriate component type of this module.
     *
     * @return the component type of this module
     */
    virtual CompType getThisCompType() = 0;

    CompType thisCompType;

    virtual void sendMessageToUDP(const TransportAddress& addr,
                                  cPacket* message)
    {
        throw cRuntimeError("sendMessageToUDP() not implemented");
    }

    virtual void pingResponse(PingResponse* pingResponse,
                              cPolymorphic* context, int rpcId,
                              simtime_t rtt);

    virtual void pingTimeout(PingCall* pingCall,
                             const TransportAddress& dest,
                             cPolymorphic* context,
                             int rpcId);

    NeighborCache *neighborCache; /**< pointer to the neighbor cache */
    CryptoModule *cryptoModule; /**<  pointer to CryptoModule */

    int numPingSent;
    int bytesPingSent;
    int numPingResponseSent;
    int bytesPingResponseSent;

    bool internalHandleMessage(cMessage* msg);


private:

    virtual void handleTimerEvent(cMessage* msg);

    /**
     * Sends a Remote-Procedure-Call message to the underlay.<br>
     * USE ONE OF THE WRAPPER FUNCTIONS INSTEAD: <br>
     * sendRouteRpcCall(), sendInternalRpcCall(), or sendUdpRpcCall()
     *
     * If no timeout is provided, a default value of
     * globalParameters.rpcUdpTimeout for
     * underlay and globalParameters.rpcKeyTimeout for a overlay rpc
     * is used. Internal RPCs don't have a default timeout.
     * After a timeout the message gets retransmitted for
     * at maximum retries times. The destKey
     * attribute is kept untouched.
     *
     * @param transportType The type of transport
     * @param destComp The destination component
     * @param dest Destination node handle (may contain a source route)
     * @param destKey route the RPC to the node that is
     *        responsible for destkey
     * @param msg RPC Call Message
     * @param context a pointer to an arbitrary cPolymorphic object,
     *        which can be used to store additional state
     * @param routingType KBR routing type
     * @param timeout RPC timeout in seconds (-1=use default value, 0=no timeout)
     * @param retries How often we try to resent rpc call, if it gets lost
     * @param rpcId RPC id
     * @param rpcListener RPC Listener (callback handler)
     * @return The nonce of the RPC
     */
    uint32_t sendRpcCall(TransportType transportType,
                         CompType destComp,
                         const TransportAddress& dest,
                         const OverlayKey& destKey,
                         BaseCallMessage* msg,
                         cPolymorphic* context,
                         RoutingType routingType,
                         simtime_t timeout, int retries,
                         int rpcId, RpcListener* rpcListener);

    void sendRpcMessageWithTransport(TransportType transportType,
                                     CompType destComp,
                                     RoutingType routingType,
                                     const std::vector<TransportAddress>& sourceRoute,
                                     const OverlayKey& destKey,
                                     BaseRpcMessage* message);

    virtual void internalSendRouteRpc(BaseRpcMessage* message,
                                      const OverlayKey& destKey,
                                      const std::vector<TransportAddress>&
                                      sourceRoute,
                                      RoutingType routingType) = 0;

    virtual void internalSendRpcResponse(BaseCallMessage* call,
                                         BaseResponseMessage* response) = 0;

    void pingRpcCall(PingCall* call);
    void pingRpcResponse(PingResponse* response, cPolymorphic* context,
                         int rpcId, simtime_t rtt);
    void pingRpcTimeout(PingCall* pingCall, const TransportAddress& dest,
                        cPolymorphic* context, int rpcId);

    typedef UNORDERED_MAP<int,RpcState> RpcStates;

    int rpcsPending;
    RpcListener* defaultRpcListener;
    RpcStates rpcStates;
    simtime_t rpcUdpTimeout, rpcKeyTimeout;
    bool optimizeTimeouts;
    bool rpcExponentialBackoff;
};

#endif
