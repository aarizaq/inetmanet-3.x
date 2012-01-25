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
 * @file BaseApp.h
 * @author Bernhard Heep
 */

#ifndef __BASEAPP_H_
#define __BASEAPP_H_

//class GlobalNodeList;
class GlobalStatistics;
class UnderlayConfigurator;
class NodeHandle;
class OverlayKey;
class CommonAPIMessage;

#include <omnetpp.h>
#include <UDPSocket.h>
#include "NodeVector.h"
#include <BaseRpc.h>
#include <BaseTcpSupport.h>
#include <INotifiable.h>
#include <BaseOverlay.h>


/**
 * Base class for applications
 *
 * Base class for applications (Tier 1-3) that use overlay functionality.
 * provides common API for structured overlays (KBR), RPC and UDP
 *
 * BaseApp provides the following API calls for derived classes:
 *
 * <table rules=groups>
 * <tr><th>Method name</th><th>Call direction</th><th>available on Tier</th><th>implemented as</th></tr>
 * <tr><td colspan=4>Modified CommonAPI for structured P2P overlays:</td></tr>
 * <tr><td>callRoute()</td><td>CALL</td><td>1</td><td>OMNeT++ messages</td></tr>
 * <tr><td>deliver()</td><td>CALLBACK</td><td>1</td><td>OMNeT++ messages</td></tr>
 * <tr><td>forward()</td><td>CALLBACK</td><td>1</td><td>OMNeT++ messages</td></tr>
 * <tr><td>update()</td><td>CALLBACK</td><td>1</td><td>OMNeT++ messages</td></tr>
 * <tr><td>callLocalLookup()</td><td>CALL</td><td>1-3</td><td>direct C++-method calls</td></tr>
 * <tr><td>callNeighborSet()</td><td>CALL</td><td>1-3</td><td>direct C++-method calls</td></tr>
 * <tr><td>isSiblingFor()</td><td>CALL</td><td>1-3</td><td>direct C++-method calls</td></tr>
 * <tr><td colspan=4>Remote Procedure Call API (RPC):</td></tr>
 * <tr><td>sendUdpRpcCall()</td><td>CALL</td><td>1-3*</td><td>OMNeT++ messages</td></tr>
 * <tr><td>sendRouteRpcCall()</td><td>CALL</td><td>1-3*</td><td>OMNeT++ messages</td></tr>
 * <tr><td>sendInternalRpcCall()</td><td>CALL</td><td>1-3*</td><td>OMNeT++ messages</td></tr>
 * <tr><td colspan=4>UDP API:</td></tr>
 * <tr><td>handleUDPMessage()</td><td>CALLBACK</td><td>1-3</td><td>OMNeT++ messages</td></tr>
 * <tr><td>sendMessageToUDP()</td><td>CALL</td><td>1-3</td><td>OMNeT++ messages</td></tr>
 * <tr><td>bindToPort()</td><td>CALL</td><td>1-3</td><td>OMNeT++ messages</td></tr>
 * </table>
 *
 * Callback functions have to be implemented in derived classes!
 *
 * @see KBRTestApp
 * @author Bernhard Heep
 */
class BaseApp : public INotifiable, public BaseRpc, public BaseTcpSupport
{
private:

    /**
     * sends msg encapsulated in a KBRforward message
     * to the overlay with destination key
     *
     * @param key the destination OverlayKey
     * @param msg the message to forward
     * @param nextHopNode the considered next hop node on the route to
     *        the destination
     */
    void forwardResponse(const OverlayKey& key, cPacket* msg,
                         const NodeHandle& nextHopNode);

    /**
     * handles CommonAPIMessages
     *
     * This method gets called from BaseApp::handleMessage
     * if message arrived from_lowerTier. It determines type of
     * msg (KBR_DELIVER, KBR_FORWARD, KBR_UPDATE) and calls
     * corresponding methods. All other messages are deleted.
     *
     * @param commonAPIMsg CommonAPIMessage
     */
    void handleCommonAPIMessage(CommonAPIMessage* commonAPIMsg);

protected:
    UDPSocket socket;
    /**
     * method to set InitStage
     */
    int numInitStages() const;

    /**
     * initializes base class-attributes
     *
     * @param stage the init stage
     */
    void initialize(int stage);

    /**
     * initializes derived class-attributes
     *
     * @param stage the init stage
     */
    virtual void initializeApp(int stage);

    /**
     * checks for message type and calls corresponding method
     *
     * checks for message type (from overlay or selfmessage) and calls
     * corresponding method
     * like deliver(), forward(), and timer()
     * @param msg the handled message
     */
    void handleMessage(cMessage* msg);

    /**
     * callback-method for events at the NotificationBoard
     *
     * @param category ...
     * @param details ...
     */
    virtual void receiveChangeNotification(int category, const cPolymorphic * details);

    /**
     * This method gets call if the node has a new TransportAddress (IP address)
     * because he changed his access network
     */
    virtual void handleTransportAddressChangedNotification();

    /**
     * This method gets call **.gracefulLeaveDelay seconds before it is killed
     */
    virtual void handleNodeLeaveNotification();

    /**
     * This method gets call **.gracefulLeaveDelay seconds before it is killed
     * if this node is among the gracefulLeaveProbability nodes
     */
    virtual void handleNodeGracefulLeaveNotification();

    /**
     * collects statistical data
     */
    void finish();

    /**
     * collects statistical data of derived app
     */
    virtual void finishApp();

    // common API for structured p2p-overlays
    /**
     * Common API function: calls route-method in overlay
     *
     * encapsulates msg into KBRroute message and sends it to the overlay
     * module
     *
     * @param key destination key
     * @param msg message to route
     * @param hint next hop (usually unused)
     * @param routingType specifies the routing mode (ITERATIVE_ROUTING, ...)
     */
    inline void callRoute(const OverlayKey& key, cPacket* msg,
                          const TransportAddress& hint
                              = TransportAddress::UNSPECIFIED_NODE,
                          RoutingType routingType = DEFAULT_ROUTING)
    {
        std::vector<TransportAddress> sourceRoute;
        sourceRoute.push_back(hint);
        callRoute(key, msg, sourceRoute, routingType);
    }

    void callRoute(const OverlayKey& key, cPacket* msg,
                   const std::vector<TransportAddress>& sourceRoute,
                   RoutingType routingType = DEFAULT_ROUTING);

    /**
     * Common API function: handles delivered messages from overlay
     *
     * method to handle decapsulated KBRdeliver messages from overlay module,
     * should be overwritten in derived application
     * @param key destination key
     * @param msg delivered message
     */
    virtual void deliver(OverlayKey& key, cMessage* msg);

    /**
     * Common API function: handles messages from overlay to be forwarded
     *
     * method to handle decapsulated KBRdeliver messages from overlay module,
     * should be overwritten in derived application if needed
     * @param key destination key
     * @param msg message to forward
     * @param nextHopNode next hop
     */
    virtual void forward(OverlayKey* key, cPacket** msg,
                         NodeHandle* nextHopNode);

    /**
     * Common API function: informs application about neighbors and own nodeID
     *
     * @param node new or lost neighbor
     * @param joined new or lost?
     */
    virtual void update(const NodeHandle& node, bool joined);

    /**
     * Common API function: produces a list of nodes that can be used as next
     * hops towards key
     *
     * @param key the destination key
     * @param num maximal number of nodes in answer
     * @param safe fraction of faulty nodes is not higher than in the overlay?
     */
    inline NodeVector* callLocalLookup(const OverlayKey& key, int num,
                                       bool safe)
    {
        return overlay->local_lookup(key, num, safe);
    };

    /**
     * Common API function: produces a list of neighbor nodes
     *
     * @param num maximal number of nodes in answer
     */
    inline NodeVector* callNeighborSet(int num)
    {
        return overlay->neighborSet(num);
    };

    /**
     * Query if a node is among the siblings for a given key.
     *
     * Query if a node is among the siblings for a given key.
     * This means, that the nodeId of this node among the close
     * numSiblings nodes to the key and that by a local findNode() call
     * all other siblings to this key can be retrieved.
     *
     * @param node the NodeHandle
     * @param key destination key
     * @param numSiblings The nodes knows all numSiblings nodes close
     *                    to this key
     * @param err return false if the range could not be determined
     * @return bool true, if the node is responsible for the key.
     */
    inline bool isSiblingFor(const NodeHandle& node, const OverlayKey& key,
                             int numSiblings, bool* err)
    {
        return overlay->isSiblingFor(node, key, numSiblings, err);
    };

    /**
     * processes self-messages
     *
     * method to handle self-messages
     * should be overwritten in derived application if needed
     * @param msg self-message
     */
    //virtual void handleTimerEvent(cMessage* msg);

    /**
     * method to handle non-commonAPI messages from the overlay
     *
     * @param msg message to handle
     */
    virtual void handleLowerMessage(cMessage* msg);

    /**
     * handleUpperMessage gets called of handleMessage(cMessage* msg)
     * if msg arrivedOn from_upperTier (currently msg gets deleted in
     * this function)
     *
     * @param msg the message to handle
     */
    virtual void handleUpperMessage(cMessage* msg);

    /**
     * method to handle messages that come directly from the UDP gate
     *
     * @param msg message to handle
     */
    virtual void handleUDPMessage(cMessage* msg);

     /**
     * method to handle ready messages from the overlay
     *
     * @param msg message to handle
     */
    virtual void handleReadyMessage(CompReadyMessage* msg);

    /**
     * Tells UDP we want to get all packets arriving on the given port
     */
    virtual void bindToPort(int port);

    /**
     * Sends a packet over UDP
     */
    virtual void sendMessageToUDP(const TransportAddress& destAddr, cPacket *msg);

    /**
     * handleTraceMessage gets called of handleMessage(cMessage* msg)
     * if a message arrives at trace_in. The command included in this
     * message should be parsed and handled.
     *
     * @param msg the command message to handle
     */
    virtual void handleTraceMessage(cMessage* msg);

    /**
     * sends non-commonAPI message to the lower tier
     *
     * @param msg message to send
     */
    void sendMessageToLowerTier(cPacket* msg);

    UnderlayConfigurator* underlayConfigurator; /**< pointer to
						   UnderlayConfigurator
						   in this node*/
    GlobalNodeList* globalNodeList; /**< pointer to GlobalNodeList
					 in this node*/

    GlobalStatistics* globalStatistics; /**< pointer to GlobalStatistics module
					 in this node*/

    NotificationBoard* notificationBoard; /**< pointer to
                                               NotificationBoard in this node */

    // parameters
    bool debugOutput; /**< debug output yes/no?*/

    // statistics
    int numOverlaySent; /**< number of sent packets to overlay*/
    int bytesOverlaySent; /**< number of sent bytes to overlay*/
    int numOverlayReceived; /**< number of received packets from overlay*/
    int bytesOverlayReceived; /**< number of received bytes from overlay*/
    int numUdpSent; /**< number of sent packets to UDP*/
    int bytesUdpSent; /**< number of sent bytes to UDP*/
    int numUdpReceived; /**< number of received packets from UDP*/
    int bytesUdpReceived; /**< number of received bytes from UDP*/

    simtime_t creationTime; /**< simTime when the App has been created*/

public:

    BaseApp();

    /**
     * virtual destructor
     */
    virtual ~BaseApp();



protected://methods: rpc handling
    bool internalHandleRpcCall(BaseCallMessage* msg);
    void internalHandleRpcResponse(BaseResponseMessage* msg,
                                   cPolymorphic* context, int rpcId,
                                   simtime_t rtt);

    void internalSendRouteRpc(BaseRpcMessage* message,
                              const OverlayKey& destKey,
                              const std::vector<TransportAddress>&
                                  sourceRoute,
                              RoutingType routingType);

    virtual CompType getThisCompType();
    void sendReadyMessage(bool ready = true);

private:
    void internalSendRpcResponse(BaseCallMessage* call,
                                 BaseResponseMessage* response);
};

#endif
