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
 * @file Broose.h
 * @author Jochen Schenk
 */

#ifndef __BROOSE_H_
#define __BROOSE_H_

#include <omnetpp.h>
#include <BaseOverlay.h>
#include <RpcListener.h>
#include <OverlayKey.h>
#include "BrooseHandle.h"
#include "BrooseBucket.h"
#include "BrooseMessage_m.h"

#include <map>
#include <vector>

class BrooseBucket;

/**
 * Broose overlay module
 *
 * Implementation of the Broose KBR overlay as described in
 * "Broose: A Practical Distributed Hashtable Based on the
 * De-Bruijn Topology" by Anh-Tuan Gai and Laurent Viennot
 *
 * @author Jochen Schenk
 * @see Bucket
 */


class Broose : public BaseOverlay
{
  public:
    Broose();
    ~Broose();

    // see BaseOverlay.h
    virtual void initializeOverlay(int stage);

    // see BaseOverlay.h
    virtual void finishOverlay();

    // see BaseOverlay.h
    virtual bool isSiblingFor(const NodeHandle& node,
                              const OverlayKey& key,
                              int numSiblings,
                              bool* err);

    // see BaseOverlay.h
    virtual void joinOverlay();

    // see BaseOverlay.h
    virtual void recordOverlaySentStats(BaseOverlayMessage* msg);

    // see BaseOverlay.h
    virtual bool handleRpcCall(BaseCallMessage* msg);

    // see BaseOverlay.h
    virtual void handleTimerEvent(cMessage* msg);

    /**
     * updates information shown in tk-environment
     */
    void updateTooltip();

  protected:
    //parameter
    int chooseLookup; /**< decides which kind of lookup (right/left shifting) is used */
    simtime_t joinDelay; /**< time interval between two join tries */
    int receivedJoinResponse; /**< number of received join response messages */
    int receivedBBucketLookup; /**< number of received lookup responses for the B bucket */
    int numberBBucketLookup; /**< maximal number of lookup responses for the B bucket */
    int receivedLBucketLookup; /**< number of received lookup responses for the L bucket */
    int numberLBucketLookup;  /**< maximal number of lookup responses for the L bucket */
    int shiftingBits; /**< number of bits shifted in/out each step */
    int powShiftingBits; /**< 2^{variable shiftingBits} */
    uint32_t bucketSize;  /**< maximal number of bucket entries */
    uint32_t rBucketSize; /**< maximal number of entries in the r buckets */
    int keyLength; /**< length of the node and data IDs */
    simtime_t refreshTime; /**< idle time after which a node is pinged */
    uint32_t userDist; /**< how many hops are added to the estimated hop count */
    int numberRetries; /**< number of retries in case of timeout */
    int bucketRetries; /**< number of bucket retries for a successful join */
    bool stab1;
    bool stab2;

    //statistics
    int bucketCount; /**< number of Bucket messages */
    int bucketBytesSent; /**< length of all Bucket messages */
    int numFailedPackets; /**< number of packets which couldn't be routed correctly */

    //module references
    BrooseBucket *lBucket, *bBucket;  /**< */
    BrooseBucket **rBucket;  /**< */

    std::vector<BrooseBucket*> bucketVector; /**< vector of all Broose buckets */

    // timer
    cMessage* join_timer;  /**< */
    cMessage* bucket_timer; /**< timer to reconstruct all buckets */

    //node handles
    TransportAddress bootstrapNode;  /**< node handle holding the bootstrap node */

    //functions

    /**
     * handles a expired join timer
     *
     * @param msg the timer self-message
     */
    void handleJoinTimerExpired(cMessage* msg);

    /**
     * handles a expired bucket refresh timer
     *
     * @param msg the bucket refresh self-message
     */
    void handleBucketTimerExpired(cMessage* msg);

    /**
     * calculates the de-buijn distance between a key and a nodeId
     *
     * @param key the overlay key
     * @param node the nodeId
     * @param dist the estimated maximum distance based on the number of
     *             nodes in the system
     * @return the number of routing steps to the destination (negative for
     *         left shifting lookups)
     */
    int getRoutingDistance(const OverlayKey& key, const OverlayKey& node,
                           int dist);

    /**
     * Adds a node to the routing table
     *
     * @param node NodeHandle to add
     * @param isAlive true, if it is known that the node is alive
     * @param rtt measured round-trip-time to node
     * @return true, if the node was known or has been added
     */
    bool routingAdd(const NodeHandle& node, bool isAlive,
                    simtime_t rtt = MAXTIME);

    /**
     * changes the node's state
     *
     * @param state the state to which a node is changing
     */
    void changeState(int state);

    // see BaseOverlay.h
    NodeVector* findNode(const OverlayKey& key,
                         int numRedundantNodes,
                         int numSiblings,
                         BaseOverlayMessage* msg);

    // see BaseOverlay.h
    int getMaxNumSiblings();

    // see BaseOverlay.h
    int getMaxNumRedundantNodes();

    /**
     * debug function which output the content of the node's buckets
     */
    void displayBucketState();

    // see BaseOverlay.h
    void handleRpcResponse(BaseResponseMessage* msg,
                           const RpcState& rpcState,
                           simtime_t rtt);

    // see BaseOverlay.h
    void handleRpcTimeout(const RpcState& rpcState);

    /**
     * This method is called if an Find Node Call timeout has been reached.
     *
     * @param findNode The original FindNodeCall
     * @param dest the destination node
     * @param destKey the destination OverlayKey
     */
    void handleFindNodeTimeout(FindNodeCall* findNode,
			       const TransportAddress& dest,
			       const OverlayKey& destKey);

    /**
     * handles a received Bucket request
     *
     * @param msg the message to process
     */
    void handleBucketRequestRpc(BucketCall* msg);

    /**
     * handles a received Bucket response
     *
     * @param msg the message to process
     * @param rpcState the state object for the received RPC
     */
    void handleBucketResponseRpc(BucketResponse* msg, const RpcState& rpcState);

    /**
     * handles a received Bucket timeout
     *
     * @param msg the message to process
     */
    void handleBucketTimeout(BucketCall* msg);

    void routingTimeout(const BrooseHandle& handle);

    // see BaseRpc.h
    virtual void pingResponse(PingResponse* pingResponse,
                              cPolymorphic* context, int rpcId,
                              simtime_t rtt);

    // see BaseRpc.h
    virtual void pingTimeout(PingCall* pingCall,
                             const TransportAddress& dest,
                             cPolymorphic* context,
                             int rpcId);

    /**
     * updates the timestamp of a node in all buckets
     *
     * @param node node handle which should be updated
     */
    void setLastSeen(const NodeHandle& node);

    /**
     * adds a node to all buckets
     *
     * @param node node handle which should be added
     */
    void addNode(const NodeHandle& node);

    /**
     * resets the counter of failed responses
     *
     * @param node node handle of the responding node
     */
    void resetFailedResponses(const NodeHandle& node);

    /**
     * sets the rtt to a node in all buckets
     *
     * @param node node handle to which a rtt is added/updated
     * @param rtt round trip time to the node
     */
    void setRTT(const NodeHandle& node, simtime_t rtt);

    friend class BrooseBucket;
};



#endif
