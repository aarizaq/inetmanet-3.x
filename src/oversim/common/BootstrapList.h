//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file BootstrapList.h
 * @author Bin Zheng, Ingmar Baumgart
 */


#ifndef __BOOTSTRAPLIST_H_
#define __BOOTSTRAPLIST_H_

#include <omnetpp.h>
#include <BaseApp.h>
#include <BootstrapNodeHandle.h>
#include <NodeVector.h>
#include <oversim_mapset.h>

class BaseOverlay;
class ZeroconfConnector;

typedef std::pair<TransportAddress, BootstrapNodeHandle*> NodePair;
// hash_map for accommodating bootstrap nodes
typedef UNORDERED_MAP<TransportAddress, BootstrapNodeHandle*,
                      TransportAddress::hashFcn> BootstrapNodeSet;

/**
 * The BootstrapList module maintains a list of bootstrap node candidates.
 *
 * The BootstrapList module maintains a list of bootstrap node
 * candidates received from various sources (GlobalNodeList for
 * simulations and ZeroconfConnector for SingleHostUnderlay). This list is also
 * used to detect overlay partitions and triggers the merging process.
 *
 * @author Bin Zheng, Ingmar Baumgart
 * @see BootstrapList
 */
class BootstrapList : public BaseApp
{

public:
    BootstrapList();
    ~BootstrapList();

   /**
    * Get a bootstrap node from the bootstrap list.
    * If not using SingleHostUnderlayConfigurator and the list is empty,
    * return a node by asking the GlobalNodeList.
    */
    const TransportAddress getBootstrapNode();

   /**
    * Determine locality of a bootstrap node.
    *
    * @param node the newly discovered bootstrap node
    */
   void locateBootstrapNode(const NodeHandle& node);

    /**
     * Inserts a new bootstrap candidate into the bootstrap list.
     *
     * @param node the bootstrap candidate
     * @param prio priority of the bootstrap node
     * @return true, if bootstrap node is already in the list
     */
    bool insertBootstrapCandidate(const NodeHandle& node,
                                  BootstrapNodePrioType prio = DNSSD);

    bool insertBootstrapCandidate(BootstrapNodeHandle& node);

    /**
     * Remove an unavailable bootstrap candidate from the bootstraplist.
     *
     * @param addr the address of the bootstrap candidate
     */
    void removeBootstrapCandidate(const TransportAddress &addr);

    void removeBootstrapNode(const NodeHandle& node);

    void registerBootstrapNode(const NodeHandle& node);

protected:
    // see BaseRpc.h
    virtual void pingResponse(PingResponse* pingResponse,
                              cPolymorphic* context, int rpcId,
                              simtime_t rtt);

    // see BaseRpc.h
    virtual void pingTimeout(PingCall* pingCall,
                             const TransportAddress& dest,
                             cPolymorphic* context,
                             int rpcId);

    // see BaseOverlay.h
    virtual CompType getThisCompType() { return BOOTSTRAPLIST_COMP; };

private:
    BootstrapNodeSet bootstrapList;

    // see BaseApp.h
    virtual void initializeApp(int stage);

    // see BaseApp.h
    virtual void finishApp();

    // see BaseApp.h
    void handleTimerEvent(cMessage *msg);

    /**
     * Periodic maintenance method for the bootstrap list.
     */
    void handleBootstrapListTimerExpired();

    // see BaseRpc.h
    void handleRpcResponse(BaseResponseMessage* msg,
                           cPolymorphic* context, int rpcId,
                           simtime_t rtt);

    /**
     * Handle the response for a lookup rpc. This is used
     * to detected foreign overlay partitions
     *
     * @param msg The lookup response message
     */
    void handleLookupResponse(LookupResponse* msg);

    static const int timerInterval = 10; /**< the interval of the maintenance timer in seconds */

    cMessage* timerMsg; /**< self-message for periodic maintenance */
    ZeroconfConnector* zeroconfConnector; /**<pointer to the ZeroconfConnector module */
    bool mergeOverlayPartitions; /**< if true, detect and merge overlay partitions */
    bool maintainList; /**< maintain a list of bootstrap candidates and check them periodically */
};

#endif
