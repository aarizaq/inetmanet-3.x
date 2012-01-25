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
 * @file IterativeLookup.h
 * @author Sebastian Mies, Ingmar Baumgart
 */

#ifndef __ITERATIVE_LOOKUP_H
#define __ITERATIVE_LOOKUP_H

#include <vector>
#include <oversim_mapset.h>

#include <IterativeLookupConfiguration.h>
#include <AbstractLookup.h>
#include <RpcListener.h>

#include <NodeVector.h>
#include <Comparator.h>

class NodeHandle;
class OverlayKey;
class LookupListener;
class IterativeLookup;
class IterativePathLookup;
class BaseOverlay;

static const double LOOKUP_TIMEOUT = 10.0;

class LookupEntry {
public:
    NodeHandle handle;
    NodeHandle source;
    bool alreadyUsed;

    LookupEntry(const NodeHandle& handle, const NodeHandle& source,
                bool alreadyUsed) : handle(handle), source(source),
                                    alreadyUsed(alreadyUsed) {};

    LookupEntry() : handle(NodeHandle::UNSPECIFIED_NODE),
                    source(NodeHandle::UNSPECIFIED_NODE), alreadyUsed(false) {};


};

typedef BaseKeySortedVector< LookupEntry > LookupVector;

template <>
struct KeyExtractor<LookupEntry> {
    static const OverlayKey& key(const LookupEntry& nodes)
    {
        return nodes.handle.getKey();
    };
};

/**
 * This class implements a basic greedy lookup strategy.
 *
 * It uses the standard metric for greedy behaviour. If another
 * metric is needed, the distance function can be replaced by
 * overriding the distance method.
 *
 * @author Sebastian Mies
 */
class IterativeLookup : public RpcListener,
                        public AbstractLookup,
                        public Comparator<OverlayKey>
{
    friend class IterativePathLookup;
    friend class BaseOverlay;

protected:
    /**
     * This method creates a new path lookup. It may be overloaded
     * to enhance IterativePathLookup with some new information/features.
     *
     * @return The new path lookup
     */
    virtual IterativePathLookup* createPathLookup();

    /**
     * Creates a find node call message. This method can be
     * overridden to add some additional state information to the
     * FindNodeCall message.
     *
     * @param findNodeExt Pointer to a optional cMessage, that may
     *                    contain overlay specific data to be attached
     *                    to FindNode RPCs and BaseRouteMessages
     *
     * @returns pointer to a new FindNodeCall message.
     */
    virtual FindNodeCall* createFindNodeCall(cPacket *findNodeExt = NULL);

    //-------------------------------------------------------------------------
    //- Base configuration and state ------------------------------------------
    //-------------------------------------------------------------------------
protected:
    OverlayKey key;                 /**< key to lookup */
    BaseOverlay* overlay;           /**< ptr to overlay */
    LookupListener* listener;       /**< lookup listener */
    std::vector<IterativePathLookup*> paths;  /**< parallel paths */
    RoutingType routingType;        /**< RoutingType for this lookup */
    IterativeLookupConfiguration config; /**< lookup configuration */
    cPacket* firstCallExt;          /**< additional info for first findNode() */
    uint32_t finishedPaths;             /**< number of finished paths */
    uint32_t successfulPaths;           /**< number of successful paths */
    uint32_t accumulatedHops;           /**< total number of hops (for all paths) */
    bool finished;                  /**< true, if lookup is finished */
    bool success;                   /**< true, if lookup was successful */
    bool running;                   /**< true, if lookup is running */
    int retries;                    /**< number of retries, if lookup fails */
    bool appLookup;
    SimTime startTime;              /**< time at which the lookup was started */

public://virtual methods: comparator induced by distance in BaseOverlay
    /**
     * compares two OverlayKeys and indicates which one is
     * closer to the key to look up
     *
     * @param lhs the first OverlayKey
     * @param rhs the second OverlayKey
     * @return -1 if rhs is closer, 0 if lhs and rhs are
     *         equal and 1 if rhs is farther away to the key to lookup
     */
    int compare( const OverlayKey& lhs, const OverlayKey& rhs ) const;

    //-------------------------------------------------------------------------
    //- Siblings and visited nodes management---------------------------------
    //-------------------------------------------------------------------------
protected://fields
    NodeVector siblings;           /**< closest nodes */
    TransportAddress::Set visited; /**< nodes already visited */
    TransportAddress::Set dead;    /**< nodes which seem to be dead */
    TransportAddress::Set pinged;  /**< nodes already pinged */
    typedef std::map<int,int> PendingPings;
    typedef std::set<NodeHandle> MajoritySiblings;
    MajoritySiblings majoritySiblings; /**< map for majority decision on correct siblings */
    int numSiblings;               /**< number of siblings */
    int hopCountMax;               /**< maximum hop count */
    PendingPings pendingPings;              /**< number of pending ping calls */

protected://methods
    /**
     * adds a node to the siblings NodeVector
     *
     * @param handle NodeHandle of the node to add
     * @param assured true, if this node was already authenticated
     * @return true if operation was succesful, false otherwise
     */
    bool addSibling(const NodeHandle& handle, bool assured = false);

    /**
     * adds/deletes visited nodes to/from the visited TransportAddress::Set
     *
     * @param addr TransportAddress of the node to add
     * @param visitedFlag if true node is added, else node is erased
     */
    void setVisited(const TransportAddress& addr, bool visitedFlag = true);

    /**
     * indicates if the specified node has been visited before
     *
     * @param addr TransportAddress of the node
     * @return false if addr is not in visited, true otherwise
     */
    bool getVisited( const TransportAddress& addr);

    /**
     * marks a node as already pinged for authentication
     *
     * @param addr TransportAddress of the node to mark as already pinged
     */
    void setPinged(const TransportAddress& addr);

    /**
     * verify if this node has already been pinged for authentication
     *
     * @param addr TransportAddress of the node
     * @return false if addr was not already pinged, true otherwise
     */
    bool getPinged(const TransportAddress& addr);

    /**
     * add a dead node to the dead node list
     *
     * @param addr TransportAddress of the node to add
     */
    void setDead(const TransportAddress& addr);

    /**
     * check if a node seems to be dead
     *
     * @param addr TransportAddress of the node
     * @return true, if the node seems to be dead
     */
    bool getDead(const TransportAddress& addr);

    //-------------------------------------------------------------------------
    //- Parallel RPC distribution ---------------------------------------------
    //-------------------------------------------------------------------------
protected://fields and classes: rpc distribution

    class RpcInfo
    {
    public:
        int vrpcId;
        uint8_t proxVectorId;
        IterativePathLookup* path;
    };

    class RpcInfoVector : public std::vector<RpcInfo>
    {
    public:
        uint32_t nonce;
    };

    typedef UNORDERED_MAP<TransportAddress, RpcInfoVector, TransportAddress::hashFcn> RpcInfoMap;
    RpcInfoMap rpcs;

protected://methods: rpcListener
    virtual void handleRpcResponse(BaseResponseMessage* msg,
                           cPolymorphic* context,
                           int rpcId, simtime_t rtt);

    virtual void handleRpcTimeout(BaseCallMessage* msg,
                          const TransportAddress& dest,
                          cPolymorphic* context, int rpcId,
                          const OverlayKey& destKey = OverlayKey::UNSPECIFIED_KEY);

protected://methods: rpc distribution

    void sendRpc(const NodeHandle& handle, FindNodeCall* call,
                 IterativePathLookup* listener, int rpcId);

    //-------------------------------------------------------------------------
    //- Construction & Destruction --------------------------------------------
    //-------------------------------------------------------------------------
public://construction & destruction
    IterativeLookup(BaseOverlay* overlay, RoutingType routingType,
               const IterativeLookupConfiguration& config,
               const cPacket* findNodeExt = NULL, bool appLookup = false);

    virtual ~IterativeLookup();

protected:
    void start();
    void stop();
    void checkStop();

    //-------------------------------------------------------------------------
    //- AbstractLookup implementation -----------------------------------------
    //-------------------------------------------------------------------------
public://methods
    void lookup(const OverlayKey& key, int numSiblings = 1,
                int hopCountMax = 0, int retries = 0,
                LookupListener* listener = NULL);

    const NodeVector& getResult() const;

    bool isValid() const;
    void abortLookup();

    uint32_t getAccumulatedHops() const;
};

/**
 * This class implements a path lookup.
 *
 * @author Sebastian Mies
 */
class IterativePathLookup
{
    friend class IterativeLookup;

protected://fields:
    IterativeLookup* lookup;
    BaseOverlay* overlay;

protected://fields: state
    int  hops;
    int  step;
    int  pendingRpcs;
    bool finished;
    bool success;
    LookupVector nextHops;
    std::map<TransportAddress, NodeHandle> oldNextHops;

protected://methods: rpc handling
    bool accepts(int rpcId);
    void handleResponse(FindNodeResponse* msg);
    void handleTimeout(BaseCallMessage* msg, const TransportAddress& dest,
                       int rpcId);
    void handleFailedNodeResponse(const NodeHandle& src,
                                  cPacket* findNodeExt, bool retry);

private:
    void sendRpc(int num, cPacket* FindNodeExt = NULL);

    void sendNewRpcAfterTimeout(cPacket* findNodeExt);

protected:
    IterativePathLookup(IterativeLookup* lookup);
    virtual ~IterativePathLookup();

    /**
     * Adds a NodeHandle to next hops
     */
    int add(const NodeHandle& handle,
            const NodeHandle& source = NodeHandle::UNSPECIFIED_NODE);
};

#endif

