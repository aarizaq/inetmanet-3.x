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
 * @file BasePastry.h
 * @author Felix Palmen, Gerhard Petruschat, Bernhard Heep
 */

#ifndef __BASEPASTRY_H_
#define __BASEPASTRY_H_

#include <vector>
#include <map>
#include <queue>
#include <algorithm>

#include <omnetpp.h>
#include <IPvXAddress.h>

#include <OverlayKey.h>
#include <NodeHandle.h>
#include <BaseOverlay.h>
#include <NeighborCache.h>

#include "PastryTypes.h"
#include "PastryMessage_m.h"
#include "PastryRoutingTable.h"
#include "PastryLeafSet.h"
#include "PastryNeighborhoodSet.h"

class PastryLeafSet;

#define DISCOVERY JOINING_1

class BasePastry : public BaseOverlay, public ProxListener
{
public:

    virtual ~BasePastry();

    virtual void pingResponse(PingResponse* pingResponse,
                              cPolymorphic* context, int rpcId,
                              simtime_t rtt);

    void pingTimeout(PingCall* call, const TransportAddress& dest,
                     cPolymorphic* context, int rpcId);

    // see BaseOverlay.h
    int getMaxNumSiblings();

    // see BaseOverlay.h
    int getMaxNumRedundantNodes();

     /**
     * processes messages from application
     *
     * @param msg message from application
     */
    virtual void handleAppMessage(BaseOverlayMessage* msg);

    /**
     * updates information shown in tk-environment
     */
    virtual void updateTooltip();

    // see BaseOverlay.h
    virtual NodeVector* findNode(const OverlayKey& key,
                                 int numRedundantNodes,
                                 int numSiblings,
                                 BaseOverlayMessage* msg);

    /**
     * processes state messages, merging with own state tables
     *
     * @param msg the pastry state message
     */
    virtual void handleStateMessage(PastryStateMessage* msg) = 0;

    // see BaseOverlay.h
    virtual void finishOverlay();

    /**
     * send a PastryStateMessage directly to a node
     *
     * @param destination destination node
     * @param type the type of the state message to be sent
     * @param ... additional arguments for some types: @n
     *        PASTRY_STATE_JOIN: @c int @a hops number of hops to
     *        destination node @n
     *        PASTRY_STATE_JOIN: @c bool @a last mark the state
     *        message to originate from closest node found @n
     *        PASTRY_STATE_UPDATE: @c simtime_t* @a timestamp pointer use
     *        this timestamp for the uptade message
     */
    // Note: simtime_t cannot be passes as '...' param, it's non-POD type.
    void sendStateTables(const TransportAddress& destination,
                         int type = PASTRY_STATE_STD, ...);

    /**
     * send a standard state message with a small delay
     *
     * @param destination destination node
     */
    void sendStateDelayed(const TransportAddress& destination);


    // see BaseOverlay.h
    virtual bool isSiblingFor(const NodeHandle& node,
                              const OverlayKey& key,
                              int numSiblings,
                              bool* err);

    // see BaseOverlay.h
    virtual AbstractLookup* createLookup(RoutingType routingType = DEFAULT_ROUTING,
                                         const BaseOverlayMessage* msg = NULL,
                                         const cObject* dummy = NULL,
                                         bool appLookup = false);

    uint8_t getBitsPerDigit() { return bitsPerDigit; };

    // statistics
    int joins;
    int joinTries;
    int joinPartial;
    int joinSeen;
    int joinBytesSeen;
    int joinReceived;
    int joinBytesReceived;
    int joinSent;
    int joinBytesSent;
    int stateSent;
    int stateBytesSent;
    int stateReceived;
    int stateBytesReceived;
    int repairReqSent;
    int repairReqBytesSent;
    int repairReqReceived;
    int repairReqBytesReceived;
    int stateReqSent;
    int stateReqBytesSent;
    int stateReqReceived;
    int stateReqBytesReceived;
    int totalLookups;
    int responsibleLookups;
    int routingTableLookups;
    int closerNodeLookups;
    int closerNodeLookupsFromNeighborhood;

    int leafsetReqSent;
    int leafsetReqBytesSent;
    int leafsetReqReceived;
    int leafsetReqBytesReceived;
    int leafsetSent;
    int leafsetBytesSent;
    int leafsetReceived;
    int leafsetBytesReceived;

    int routingTableReqSent;
    int routingTableReqBytesSent;
    int routingTableReqReceived;
    int routingTableReqBytesReceived;
    int routingTableSent;
    int routingTableBytesSent;
    int routingTableReceived;
    int routingTableBytesReceived;
    uint32_t rowToAsk;

    void proxCallback(const TransportAddress& node, int rpcId,
                      cPolymorphic *contextPointer, Prox prox);

  protected:

     /**
     * changes node state
     *
     * @param toState state to change to
     */
    virtual void changeState(int toState);

     // parameters
    uint32_t bitsPerDigit;
    uint32_t numberOfLeaves;
    uint32_t numberOfNeighbors;
    double readyWaitAmount;
    double joinTimeoutAmount;
    double repairTimeout;
    bool enableNewLeafs;
    bool useRegularNextHop;
    bool alwaysSendUpdate;
    bool optimizeLookup;
    bool proximityNeighborSelection;

    simtime_t nearNodeRtt;

    uint32_t pingedNodes;
    bool nearNodeImproved;

    bool periodicMaintenance;

    TransportAddress* leaf2ask;

    TransportAddress bootstrapNode;
    NodeHandle nearNode;

    simtime_t lastStateChange;

    /**
     * Handle for processing a single state message
     */
    PastryStateMsgHandle stateCache;

    /**
     * Queue of state messages waiting to be processed in READY state
     */
    std::queue<PastryStateMsgHandle> stateCacheQueue;

    /**
     * Early update of leaf set: helper structure for marking known-dead nodes
     */
    PastryStateMsgProximity aliveTable;

     /**
     * checks whether proxCache is complete, takes appropriate actions
     * depending on the protocol state
     */
    virtual void checkProxCache(void) = 0;

    uint32_t joinHopCount;
    cMessage* joinTimeout;
    cMessage* readyWait;
    cMessage* joinUpdateWait;
    std::vector<PastrySendState*> sendStateWait;

    PastryRoutingTable* routingTable;
    PastryLeafSet* leafSet;
    PastryNeighborhoodSet* neighborhoodSet;

    /**
     * delete all information/messages caching vectors, used for restarting
     * overlay or finish()
     */
    virtual void purgeVectors(void);

    /**
     * initializes parameters and variables used in both Bamboo and Pastry
     */
    void baseInit(void);

    /**
     * changes node state, but leaves specific behavour,
     * scheduling tasks in particular, to the inheriting protocols
     */
    void baseChangeState(int);

    OverlayKey distance(const OverlayKey& x,
                        const OverlayKey& y,
                        bool useAlternative = false) const;

    virtual void iterativeJoinHook(BaseOverlayMessage* msg,
                                   bool incrHopCount) { };

    enum StateObject
    {
        ROUTINGTABLE,
        LEAFSET,
        NEIGHBORHOODSET
    };

    struct PingContext : public cPolymorphic
    {
        PingContext(StateObject stateObject, uint32_t index, uint32_t nonce)
            : stateObject(stateObject), index(index), nonce(nonce)
              {};
        virtual ~PingContext() {};
        StateObject stateObject;
        uint32_t index;
        uint32_t nonce;
    };

    enum
    {
        PING_RECEIVED_STATE = 1,
        PING_NEXT_HOP = 2,
        PING_SINGLE_NODE = 3,
        PING_DISCOVERY
    };

     /**
     * ping all nodes in a given state message. this is called when a state
     * message arrives while another one is still being processed.
     */
    void prePing(const PastryStateMessage* stateMsg);

    /**
     * ping all nodes in the pastry state message pointed to by
     * private member stateCache
     */
    void pingNodes(void);

     /**
     * change the aliveTable to match the given stateMsg.
     *
     * each node that's knowm to be dead from our neighborCache gets a
     * value of PASTRY_PROX_INFINITE, all other nodes just get a
     * value of 1
     */
    void determineAliveTable(const PastryStateMessage* stateMsg);

     /**
     * send a request to a given node
     * @param ask request from this node
     * @param type specifies the data requested
     */
    void sendRequest(const TransportAddress& ask, int type);

    /**
     *	send the leafset to a node
     * @param tell the node to send to
     * @param pull true requests his leafset
     */
    void sendLeafset(const TransportAddress& tell, bool pull = false);

    /**
     *	send a row of the routing table to a node
     * @param tell the node to send to
     * @param row the number of the row
     */
    void sendRoutingRow(const TransportAddress& tell, int row);


     /**
     * processes state messages, merging with own state tables
     *
     * @param msg the pastry state message
     */
    void handleRequestMessage(PastryRequestMessage* msg);

     /**
      * processes leafset messages, merging with own state tables
      *
      * @param msg the pastry state message
      * @param mergeSender should the sender also be merged
     */
    void handleLeafsetMessage(PastryLeafsetMessage* msg, bool mergeSender = false);

    /**
     * Pastry API: send newLeafs() to application if enabled
     */
    void newLeafs(void);

    friend class PastryLeafSet;
};

/**
 * predicate for comparing two pointers to PastryStateMessages based on their
 * joinHopCount. Needed for sorting the received PastryStateMessages.
 */
bool stateMsgIsSmaller(const PastryStateMsgHandle& hnd1,
                       const PastryStateMsgHandle& hnd2);

std::ostream& operator<<(std::ostream& os, const PastryStateMsgProximity& pr);

#endif

