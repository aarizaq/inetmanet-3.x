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
 * @file BaseOverlay.h
 * @author Bernhard Heep
 * @author Sebastian Mies
 */

#ifndef __BASEOVERLAY_H_
#define __BASEOVERLAY_H_

#include <oversim_mapset.h>

#include <omnetpp.h>
#include <UDPSocket.h>
#include <NodeVector.h>
#include <TopologyVis.h>
#include <INotifiable.h>
#include <BaseRpc.h>
#include <BaseTcpSupport.h>
#include <IterativeLookupConfiguration.h>
#include <RecursiveLookup.h>
#include <InitStages.h>

class GlobalNodeList;
class UnderlayConfigurator;
class BaseApp;
class NodeHandle;
class OverlayKey;
class NotificationBoard;
class AbstractLookup;
class BootstrapList;

/**
 * Base class for overlays
 *
 * Base class for overlay modules, with KBR-API, statistics and
 * pointers to the GlobalNodeList and the UnderlayConfigurator.
 * Derived classes must use BaseOverlayMessage as base class for own
 * message types.
 *
 * @author Ingmar Baumgart
 * @author Bernhard Heep
 * @author Stephan Krause
 * @author Sebastian Mies
 */
class BaseOverlay : public INotifiable,
                    public BaseRpc,
                    public BaseTcpSupport,
                    public TopologyVis
{

    friend class IterativeLookup;
    friend class IterativePathLookup;
    friend class RecursiveLookup;
    friend class BootstrapList;
    friend class SendToKeyListener;

    //------------------------------------------------------------------------
    //--- Construction / Destruction -----------------------------------------
    //------------------------------------------------------------------------

public:

    BaseOverlay();

    /** Virtual destructor */
    virtual ~BaseOverlay();

    enum States {
        INIT = 0,
        JOINING_1 = 1,
        JOINING_2 = 2,
        JOINING_3 = 3,
        READY = 4,
        REFRESH = 5,
        SHUTDOWN = 6,
        FAILED = 7,

        //some aliases for compatibility
        JOINING = JOINING_1,
        JOIN = JOINING_1,
        BOOTSTRAP = JOINING_1,
        RSET = JOINING_2,
        BSET = JOINING_3
    };

    States getState() { return state; };

    class BaseOverlayContext : public cObject
    {
    public:
        BaseOverlayContext(const OverlayKey& key, bool malicious) : key(key), malicious(malicious) {};
        OverlayKey key;
        bool malicious;
    };


    //------------------------------------------------------------------------
    //--- Statistics ---------------------------------------------------------
    //------------------------------------------------------------------------

private://fields: statistics

    int numAppDataSent;   /**< number of sent app data packets (incl.\ forwarded packets) */
    int bytesAppDataSent; /**< number of sent app data bytes (incl.\ forwarded bytes) */
    int numAppLookupSent;   /**< number of sent app loookup packets (incl.\ forwarded packets) */
    int bytesAppLookupSent; /**< number of sent app lookup bytes (incl.\ forwarded bytes) */
    int numMaintenanceSent;   /**< number of sent maintenance packets (incl.\ forwarded packets) */
    int bytesMaintenanceSent; /**< number of sent maintenance bytes (incl.\ forwarded bytes) */

    int numAppDataReceived;   /**< number of received app data packets (incl.\ packets to be forwarded ) */
    int bytesAppDataReceived; /**< number of received app data bytes (incl.\ bytes to be forwarded) */
    int numAppLookupReceived;   /**< number of received app lookup packets (incl.\ packets to be forwarded) */
    int bytesAppLookupReceived; /**< number of received app lookup bytes (incl.\ bytes to be forwarded) */
    int numMaintenanceReceived;   /**< number of received maintenance packets (incl.\ packets to be forwarded) */
    int bytesMaintenanceReceived; /**< number of received maintenance bytes (incl.\ bytes to be forwarded) */

    int numInternalSent;  /**< number of packets sent to same host but different port (SimpleMultiOverlayHost) */
    int bytesInternalSent;  /**< number of bytes sent to same host but different port (SimpleMultiOverlayHost) */
    int numInternalReceived;  /**< number of packets received from same host but different port (SimpleMultiOverlayHost) */
    int bytesInternalReceived;  /**< number of bytes received from same host but different port (SimpleMultiOverlayHost) */

    int bytesAuthBlockSent; /**< number of bytes sent for rpc message signatures */

    int joinRetries; /**< number of join retries */

protected:
    UDPSocket socket;
    /**
     * Structure for computing the average delay in one specific hop
     */
    struct HopDelayRecord
    {
        int count;
        simtime_t val;
        HopDelayRecord() : count(0), val(0) {};
    };

    int numAppDataForwarded;   /**< number of forwarded app data packets */
    int bytesAppDataForwarded; /**< number of forwarded app data bytes at out-gate */
    int numAppLookupForwarded;   /**< number of forwarded app lookup packets */
    int bytesAppLookupForwarded; /**< number of forwarded app lookup bytes at out-gate */
    int numMaintenanceForwarded;   /**< number of forwarded maintenance packets */
    int bytesMaintenanceForwarded; /**< number of forwarded maintenance bytes at out-gate */

    int numFindNodeSent; /**< */
    int bytesFindNodeSent; /**< */
    int numFindNodeResponseSent; /**< */
    int bytesFindNodeResponseSent; /**< */
    int numFailedNodeSent; /**< */
    int bytesFailedNodeSent; /**< */
    int numFailedNodeResponseSent; /**< */
    int bytesFailedNodeResponseSent; /**< */
    std::vector<HopDelayRecord*> singleHopDelays; /**< */

    simtime_t creationTime; /**< simtime when the node has been created */


    //------------------------------------------------------------------------
    //--- Common Overlay Attributes & Global Module References ---------------
    //------------------------------------------------------------------------

protected://fields: overlay attributes



    // references to global modules
    GlobalNodeList* globalNodeList;           /**< pointer to GlobalNodeList in this node  */
    NotificationBoard* notificationBoard;       /**< pointer to NotificationBoard in this node */
    UnderlayConfigurator* underlayConfigurator; /**< pointer to UnderlayConfigurator in this node */
    BootstrapList* bootstrapList; /**< pointer to the BootstrapList module */
    GlobalParameters* globalParameters; /**< pointer to the GlobalParameters module */

    // overlay common parameters
    bool debugOutput;           /**< debug output ? */
    RoutingType defaultRoutingType;
    bool useCommonAPIforward;   /**< forward messages to applications? */
    bool collectPerHopDelay;    /**< collect delay for single hops */
    bool routeMsgAcks;          /**< send ACK when receiving route message */
    uint32_t recNumRedundantNodes;  /**< numRedundantNodes for recursive routing */
    bool recordRoute;   /**< record visited hops on route */
    bool drawOverlayTopology;
    bool rejoinOnFailure;
    bool sendRpcResponseToLastHop; /**< needed by KBR protocols for NAT support */
    bool dropFindNodeAttack; /**< if node is malicious, it tries a findNode attack */
    bool isSiblingAttack; /**< if node is malicious, it tries a isSibling attack */
    bool invalidNodesAttack; /**< if node is malicious, it tries a invalidNode attack */
    bool dropRouteMessageAttack; /**< if node is malicious, it drops all received BaseRouteMessages */
    int localPort;              /**< used UDP-port */
    int hopCountMax;            /**< maximum hop count */
    bool measureAuthBlock; /**< if true, measure the overhead of signatures in rpc messages */
    bool restoreContext; /**< if true, a node rejoins with its old nodeId and malicious state */

    int numDropped;             /**< number of dropped packets */
    int bytesDropped;           /**< number of dropped bytes */

    cOutVector delayVector;     /**< statistical output vector for packet-delays */
    cOutVector hopCountVector;  /**< statistical output vector for hop-counts */

    States state;

    //------------------------------------------------------------------------
    //--- Initialization & finishing -----------------------------------------
    //------------------------------------------------------------------------

private://methods: cSimpleModule initialization

    /**
     * initializes base-class-attributes
     *
     * @param stage the init stage
     */
    void initialize(int stage);

    /**
     * collects statistical data
     */
    void finish();

protected://methods: overlay initialization

    /**
     * Sets init stage.
     *
     * @see InitStages.h for more information about the used stage numbers
     */
    int numInitStages() const;

    /**
     * Initializes derived-class-attributes.<br>
     *
     * Initializes derived-class-attributes, called by
     * BaseOverlay::initialize(). By default this method is called
     * once. If more stages are needed one can overload
     * numInitStages() and add more stages.
     *
     * @param stage the init stage
     */
    virtual void initializeOverlay( int stage );

    /**
     * collects statistical data in derived class
     */
    virtual void finishOverlay();

private:
    /**
    * Overlay implementations can overwrite this virtual
    * method to set a specific nodeID. The default implementation
    * sets a random nodeID.
    */
    virtual void setOwnNodeID();

    //------------------------------------------------------------------------
    //--- General Overlay Parameters (getter and setters) --------------------
    //------------------------------------------------------------------------

public://methods: getters and setters

    /**
     * Returns true, if node is malicious.
     *
     * @return true, if node is malicious.
     */
    bool isMalicious();

    /**
     * Returns true if overlay is one in an array, inside a SimpleMultiOverlayHost.
     *
     * @return true, if overlay is in a SimpleMultiOverlayHost
     */
    bool isInSimpleMultiOverlayHost();

    const simtime_t& getCreationTime() { return creationTime; };


    //------------------------------------------------------------------------
    //--- UDP functions copied from the INET framework .----------------------
    //------------------------------------------------------------------------

protected:
    /**
     * Tells UDP we want to get all packets arriving on the given port
     */
    void bindToPort(int port);


    //------------------------------------------------------------------------
    //--- Overlay Common API: Key-based Routing ------------------------------
    //------------------------------------------------------------------------

protected://methods: KBR

    /**
     * Routes message through overlay.
     *
     * The default implementation uses FindNode to determine next
     * hops and a generic greedy routing algorithm provides with
     * SendToKey.
     *
     * @param key destination key
     * @param destComp the destination component
     * @param srcComp the source component
     * @param msg message to route
     * @param sourceRoute If sourceRoute is given, the message gets sent via
     *                    all nodes in the list before it is routed
     *                    (nextHop is used as a proxy)
     * @param routingType specifies the routing mode (ITERATIVE_ROUTING, ...)
     */
    virtual void route(const OverlayKey& key, CompType destComp,
                       CompType srcComp, cPacket* msg,
                       const std::vector<TransportAddress>& sourceRoute
                           = TransportAddress::UNSPECIFIED_NODES,
                       RoutingType routingType = DEFAULT_ROUTING);

    /**
     * Calls deliver function in application.
     *
     * Encapsulates messages in KBRdeliver messages and sends them
     * to application.
     *
     * @param msg delivered message
     * @param destKey the destination key of the message
     */
    void callDeliver( BaseOverlayMessage* msg,
                      const OverlayKey& destKey);

    /**
     * Calls forward function in application
     *
     * Encapsulates messages in KBRforward messages and sends them
     * to application. <br> the message to be sent through the API
     * must be encapsulated in <code>msg</code>.
     *
     * @param key destination key
     * @param msg message to forward
     * @param nextHopNode next hop
     */
    void callForward( const OverlayKey& key, BaseRouteMessage* msg,
                      const NodeHandle& nextHopNode);

    /**
     * Informs application about state changes of nodes or newly joined nodes
     *
     * Creates a KBRUpdate message and sends it up to the application
     *
     * @param node the node that has joined or changed its state
     * @param joined has the node joined or changed its state?
     */
    void callUpdate(const NodeHandle& node, bool joined);

public:

    /**
     * Join the overlay with a given nodeID
     *
     * Join the overlay with a given nodeID. This method may be called
     * by an application to join the overlay with a specific nodeID. It is
     * also called if the node's IP address changes.
     *
     * @param nodeID The new nodeID for this node.
     */
    void join(const OverlayKey& nodeID = OverlayKey::UNSPECIFIED_KEY);

    /**
     * finds nodes closest to the given OverlayKey
     *
     * calls findNode() (that should be overridden in derived overlay)
     * and returns a list with (num) nodes ordered by distance to the
     * node defined by key.
     *
     * @param key the given node
     * @param num number of nodes that are returned
     * @param safe The safe parameters is not implemented yet
     */
    virtual NodeVector* local_lookup(const OverlayKey& key, int num, bool safe);


    /**
     */
    virtual NodeVector* neighborSet(int num);

    /**
     * Query if a node is among the siblings for a given key.
     *
     * Query if a node is among the siblings for a given key.
     * This means, that the nodeId of this node is among the closest
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
    virtual bool isSiblingFor(const NodeHandle& node,
                              const OverlayKey& key, int numSiblings, bool* err);

    /**
     * Query the maximum number of siblings (nodes close to a key)
     * that are maintained by this overlay protocol.
     *
     * @return int number of siblings.
     */
    virtual int getMaxNumSiblings();

    /**
     * Query the maximum number of redundant next hop nodes that
     * are returned by findNode().
     *
     * @return int number of redundant nodes returned by findNode().
     */
    virtual int getMaxNumRedundantNodes();

    //------------------------------------------------------------------------
    //--- Message Handlers ---------------------------------------------------
    //------------------------------------------------------------------------

protected://methods: message handling

    /**
     * Checks for message type and calls corresponding method.<br>
     *
     * Checks for message type (from UDP/App or selfmessage) and
     * calls corresponding method like getRoute(), get(), put(),
     * remove(), handleTimerEvent(), handleAppMessage() and
     * handleUDPMessage().
     *
     * @param msg The message to be handled
     */
    void handleMessage(cMessage* msg);


    /**
     * Handles a BaseOverlayMessage<br>
     *
     * Handles BaseOverlayMessages of type OVERLAYSIGNALING, RPC,
     * APPDATA or OVERLAYROUTE.
     *
     * @param msg The message to be handled
     * @param destKey the destination key of the message
     */
    void handleBaseOverlayMessage(BaseOverlayMessage* msg,
                                  const OverlayKey& destKey =
                                      OverlayKey::UNSPECIFIED_KEY);

protected://methods: message handling

    /**
     * Processes messages from underlay
     *
     * @param msg Message from UDP
     */
    virtual void handleUDPMessage(BaseOverlayMessage* msg);

    /**
     * Processes "timer" self-messages
     *
     * @param msg A self-message
     */
    //virtual void handleTimerEvent(cMessage* msg);

    /**
     * Processes non-commonAPI messages
     *
     * @param msg non-commonAPIMessage
     */
    virtual void handleAppMessage(cMessage* msg);

    /**
     * callback-method for events at the NotificationBoard
     *
     * @param category ... TODO ...
     * @param details ... TODO ...
     */
    virtual void receiveChangeNotification(int category,
                                           const cPolymorphic* details);

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
     * Collect overlay specific sent messages statistics
     *
     * This method is called from BaseOverlay::sendMessageToUDP()
     * for every overlay message that is sent by a node. Use this
     * to collect statistical data for overlay protocol specific
     * message types.
     *
     * @param msg The overlay message to be sent to the UDP layer
     */
    virtual void recordOverlaySentStats(BaseOverlayMessage* msg);

protected://methods: icons and ui support

    /**
     * Sets the overlay ready icon and register/deregisters the node at the GlobalNodeList
     *
     * @param ready true if the overlay changed to ready state (joined successfully)
     */
    void setOverlayReady( bool ready );

    //------------------------------------------------------------------------
    //--- Messages -----------------------------------------------------------
    //------------------------------------------------------------------------

private://methods: sending packets over udp

    void sendRouteMessage(const TransportAddress& dest,
                          BaseRouteMessage* msg,
                          bool ack);

    bool checkFindNode(BaseRouteMessage* routeMsg);

public:
    /**
     * Sends message to underlay
     *
     * @param dest destination node
     * @param msg message to send
     */
    void sendMessageToUDP(const TransportAddress& dest, cPacket* msg);

    //------------------------------------------------------------------------
    //--- Basic Routing ------------------------------------------------------
    //------------------------------------------------------------------------

protected://fields: config
    IterativeLookupConfiguration iterativeLookupConfig;
    RecursiveLookupConfiguration recursiveLookupConfig;

    class lookupHashFcn
    {
    public:
        size_t operator()( const AbstractLookup* l1 ) const
        {
            return (size_t)l1;
        }
        bool operator()(const AbstractLookup* l1,
                        const AbstractLookup* l2) const
        {
            return (l1 == l2);
        }
    };

    typedef UNORDERED_SET<AbstractLookup*, lookupHashFcn, lookupHashFcn> LookupSet;

    LookupSet lookups;

private://methods: internal routing

    /**
     * creates a LookupSet
     */
    void initLookups();

    /**
     * deletes entries in lookups
     */
    void finishLookups();

    /**
     * Hook for forwarded message in recursive lookup mode
     *
     * This hook is called just before a message is forwarded to a next hop or
     * if the message is at its destination just before it is sent to the app.
     * Default implementation just returns true. This hook
     * can for example be used to detect failed nodes and call
     * handleFailedNode() before the actual forwarding takes place.
     *
     * @param dest destination node
     * @param msg message to send
     * @returns true, if message should be forwarded;
     *  false, if message will be forwarded later by an other function
     *  or message has been discarded
     */
    virtual bool recursiveRoutingHook(const TransportAddress& dest,
                                      BaseRouteMessage* msg);

public://methods: basic message routing

    /**
     * Sends a message to an overlay node, with the generic routing
     * algorithm.
     *
     * @param key The destination key
     * @param message Message to be sent
     * @param numSiblings number of siblings to send message to
     *                     (numSiblings > 1 means multicast)
     * @param sourceRoute If sourceRoute is given, the message gets sent via
     *                    all nodes in the list before it is routed
     *                    (nextHop is used as a proxy)
     * @param routingType specifies the routing mode (ITERATIVE_ROUTING, ...)
     */
    void sendToKey(const OverlayKey& key, BaseOverlayMessage* message,
                   int numSiblings = 1,
                   const std::vector<TransportAddress>& sourceRoute
                             = TransportAddress::UNSPECIFIED_NODES,
                   RoutingType routingType = DEFAULT_ROUTING);

    /**
     * This method should implement the distance between two keys.
     * It may be overloaded to implement a new metric. The default
     * implementation uses the standard-metric d = abs(x-y).
     *
     * @param x Left-hand-side Key
     * @param y Right-hand-side key
     * @param useAlternative use an alternative distance metric
     * @return OverlayKey Distance between x and y
     */
    virtual OverlayKey distance(const OverlayKey& x,
                                const OverlayKey& y,
                                bool useAlternative = false) const;

protected://methods: routing class factory

    /**
     * Creates an abstract iterative lookup instance.
     *
     * @param routingType The routing type for this
     *                    lookup (e.g. recursive/iterative)
     * @param msg pointer to the message for which the lookup is created.
     *            Derived classes can use it to construct an object with
     *            additional info for the lookup class.
     * @param findNodeExt object that will be sent with the findNodeCalls
     * @param appLookup Set to true, if lookup is triggered by application (for statistics)
     * @return AbstractLookup* The new lookup instance.
     */
    virtual AbstractLookup* createLookup(RoutingType routingType = DEFAULT_ROUTING,
                                         const BaseOverlayMessage* msg = NULL,
                                         const cPacket* findNodeExt = NULL,
                                         bool appLookup = false);

    /**
     * Removes the abstract lookup instance.
     *
     * @param lookup the Lookup to remove
     */
    virtual void removeLookup( AbstractLookup* lookup );

    /**
     * Implements the find node call.
     *
     * This method simply returns the closest nodes known in the
     * corresponding routing topology. If the node is a sibling for
     * this key (isSiblingFor(key) = true), this method returns all
     * numSiblings siblings, with the closest neighbor to the key
     * first.
     *
     * @param key The lookup key.
     * @param numRedundantNodes Maximum number of next hop nodes to return.
     * @param numSiblings number of siblings to return
     * @param msg A pointer to the BaseRouteMessage or FindNodeCall
     *                   message of this lookup.
     * @return NodeVector with closest nodes.
     */
    virtual NodeVector* findNode( const OverlayKey& key,
                                  int numRedundantNodes,
                                  int numSiblings,
                                  BaseOverlayMessage* msg = NULL);


    /**
     * Join the overlay with a given nodeID in thisNode.key
     *
     * Join the overlay with a given nodeID in thisNode.key.  This
     * method may be called by an application to join the overlay with
     * a specific nodeID. It is also called if the node's IP address
     * changes.
     *
     */
    virtual void joinOverlay();

     /**
      * Join another overlay partition with the given node as bootstrap node
      *
      * Join another overlay partition with the given node as bootstrap node.
      * This method is called to join a foreign overlay partition and start
      * the merging process.
      *
      * @param node The foreign bootstrap node
      */
    virtual void joinForeignPartition(const NodeHandle& node);

    /**
     * Handles a failed node.
     *
     * This method is called whenever a node given by findNode() was
     * unreachable. The default implementation does nothing at all.
     *
     * @param failed the failed node
     * @return true if lookup should retry here
     */
    virtual bool handleFailedNode(const TransportAddress& failed);

    virtual void lookupRpc(LookupCall* call);

    virtual void nextHopRpc(NextHopCall* call);

protected://methods: statistic helpers for IterativeLookup

    void countFindNodeCall(const FindNodeCall* call);
    void countFailedNodeCall(const FailedNodeCall* call);


    bool internalHandleRpcCall( BaseCallMessage* msg );
    void internalHandleRpcResponse(BaseResponseMessage* msg,
                                   cPolymorphic* context, int rpcId,
                                   simtime_t rtt);
    void internalHandleRpcTimeout(BaseCallMessage* msg,
                                  const TransportAddress& dest,
                                  cPolymorphic* context,
                                  int rpcId, const OverlayKey& destKey);

    // TODO rename to internalSendRouteRpcMessage()
    void internalSendRouteRpc(BaseRpcMessage* message,
                              const OverlayKey& destKey,
                              const std::vector<TransportAddress>&
                              sourceRoute,
                              RoutingType routingType);

    /*
     * Returns the component type of this module
     *
     * @return the component type
     */
    CompType getThisCompType();

    bool kbr; /**< set this to true, if the overlay provides KBR services */

private:
    void internalSendRpcResponse(BaseCallMessage* call,
                                 BaseResponseMessage* response);

    const cGate* udpGate;
    const cGate* appGate;

public:
    /*
     * Register a new component at the overlay
     *
     * @param compType The compoment type (defined in CommonMessages.msg)
     * @param module The module pointer of the component
     */
    void registerComp(CompType compType, cModule *module);

    /*
     * Get the module pointer of a registered component
     *
     * @param compType The compoment type (defined in CommonMessages.msg)
     * @return The module pointer of the component
     */
    cModule* getCompModule(CompType compType);

    /*
     * Get the direct_in gate of a registered component
     *
     * @param compType The compoment type (defined in CommonMessages.msg)
     * @return The pointer to the direct_in gate of the component
     */
    cGate* getCompRpcGate(CompType compType);

    /*
     * Sends a message to all currently registered components, but the
     * source component
     *
     * @param msg The pointer to the message to send
     * @param srcComp The type of the originating component
     *
     */
    void sendMessageToAllComp(cMessage* msg, CompType srcComp);

    /*
     * Returns true, if the overlay is a structured overlay and
     * provides KBR services (e.g. route, lookup, ...).
     */
    bool providesKbr() { return kbr; };

    virtual uint8_t getBitsPerDigit() { return 1; };

    bool getMeasureAuthBlock() { return measureAuthBlock; }

    BootstrapList& getBootstrapList() const { return *bootstrapList;}

private:
    void findNodeRpc( FindNodeCall* call );
    void failedNodeRpc( FailedNodeCall* call );

    typedef std::map<CompType, std::pair<cModule*, cGate*> > CompModuleList;
    CompModuleList compModuleList;
    bool internalReadyState; /**< internal overlay state used for setOverlayReady() */
};

#endif
