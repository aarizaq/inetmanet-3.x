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
 * @file GlobalNodeList.h
 * @author Markus Mauch, Robert Palmer
 */

#ifndef __GLOBALNODELIST_H__
#define __GLOBALNODELIST_H__

#include <map>
#include <vector>
#include <oversim_mapset.h>

#include <omnetpp.h>

#include <ChurnGenerator.h>

#include <BinaryValue.h>
#include <NodeHandle.h>
#include <PeerStorage.h>

class BootstrapList;
class TransportAddress;
class OverlayKey;
class GlobalStatistics;


/**
 * Global module (formerly known as GlobalNodeList) that supports the node
 * bootstrap process and contains node specific underlay parameters,
 * malicious node states, etc...
 *
 * @author Markus Mauch, Robert Palmer
 */
class GlobalNodeList : public cSimpleModule
{
public:
    /**
     * holds all OverlayKeys
     */
    typedef std::vector<OverlayKey> KeyList;

    /**
     * Adds new peers to the peer set.
     *
     * Called automatically by the underlay,
     * when new peers are created.
     *
     * @param ip IPvXAddress of the peer to add
     * @param info underlay specific info of the peer to add
     */
    void addPeer(const IPvXAddress& ip, PeerInfo* info);

    /**
     * Sends a NotificationBoard message to all registered peers.
     *
     * @param category Type of notification
     */
    void sendNotificationToAllPeers(int category);

    /**
     * Removes a peer from the peerSet.
     *
     * Called automatically by the underlay,
     * when peers are removed.
     *
     * @param ip IPvXAddress of the peer to remove
     */
    virtual void killPeer(const IPvXAddress& ip);

    /**
     * Returns a random NodeHandle
     *
     * Returns a random NodeHandle from the peerSet if at least one peer has
     * been registered, an empty TransportAddress otherwise.
     *
     * @param nodeType If != -1, return a node of that type
     * @param bootstrappedNeeded does the node need to be bootstrapped?
     * @param inoffensiveNeeded does the node need to be inoffensive?
     *
     * @return NodeHandle of the node
     */
    virtual const NodeHandle& getRandomNode(int32_t nodeType = -1,
                                            bool bootstrappedNeeded = true,
                                            bool inoffensiveNeeded = false);

    /**
     * Returns a random NodeHandle
     *
     * Returns a random NodeHandle of an already bootstrapped node from the
     * peerSet if at least one peer has been registered, an empty
     * TransportAddress otherwise. If the optional node parameter is given,
     * try to return a bootstrap node with the same TypeID.
     *
     * @param node Find a bootstrap node with the same TypeID (partition) as node
     * @return NodeHandle of the bootstrap node
     */
    virtual const NodeHandle& getBootstrapNode(const NodeHandle &node =
                                               NodeHandle::UNSPECIFIED_NODE);

    /**
     * Bootstraps peers in the peer set.
     *
     * @param peer node to register
     */
    virtual void registerPeer(const TransportAddress& peer);

    /**
     * Bootstraps peers in the peer set.
     *
     * @param peer node to register
     */
    virtual void registerPeer(const NodeHandle& peer);


    /**
     * Update entry to real port without having bootstrapped
     *
     * @param peer node to refresh
     */
    virtual void refreshEntry(const TransportAddress& peer);

    /**
     * Debootstraps peers in the peer set
     *
     * @param peer node to remove
     */
    virtual void removePeer(const TransportAddress& peer);

    /**
     * Returns a keylist
     *
     * @param maximumKeys maximum number of keys in new keylist
     * @return pointer to new keylist
     */
    virtual KeyList* getKeyList(uint32_t maximumKeys);

    /**
     * Returns random key from list
     *
     * @return the key
     */
    virtual const OverlayKey& getRandomKeyListItem();

    /**
     * Colors module-icon blue (ready), green (ready, malicious) or red (not ready)
     *
     * @param address TransportAddress of the specified node
     * @param ready state to visualize
     */
    virtual void setOverlayReadyIcon(const TransportAddress& address, bool ready);

    /**
     * Searches the peerSet for the specified node
     *
     * @param peer TransportAddress of the specified node
     * @return PeerInfo of the node or NULL if node is not in peerSet
     */
    virtual PeerInfo* getPeerInfo(const TransportAddress& peer);

    /**
     * Set a node to be malicious
     *
     * @param address TransportAddress of the node
     * @param malicious state to set
     */
    virtual void setMalicious(const TransportAddress& address, bool malicious);

    /**
     * Check if a node is malicious
     *
     * @param address TransportAddress of the node
     * @return if the node is malicious
     */
    virtual bool isMalicious(const TransportAddress& address);

    virtual cObject** getContext(const TransportAddress& address);

    /**
    * Mark a node for deletion
    *
    * @param address TransportAddress of the node
    */
    void setPreKilled(const TransportAddress& address);

    /**
     * Selects a random node from the peerSet, which is not
     * already marked for deletion
     *
     * @param nodeType If != -1, return a node of that type
     * @returns A pointer to the TransportAddress of a random alive node
     */
     TransportAddress* getRandomAliveNode(int32_t nodeType = -1);

    /**
     * Selects a random node from the peerSet
     *
     * @param nodeType If != -1, return a node of that type
     * @param bootstrapNeeded does the node need to be bootstrapped?
     * @returns The peerInfo of a random node
     */
    virtual PeerInfo* getRandomPeerInfo(int32_t nodeType = -1,
                                        bool bootstrapNeeded = false);

    /**
     * Searches the peerSet for the specified node
     *
     * @param ip IPvXAddress of the specified node
     * @return PeerInfo of the node or NULL if node is not in peerSet
     */
    virtual PeerInfo* getPeerInfo(const IPvXAddress& ip);

    size_t getNumNodes() { return peerStorage.size(); };

    bool areNodeTypesConnected(int32_t a, int32_t b);
    void connectNodeTypes(int32_t a, int32_t b);
    void disconnectNodeTypes(int32_t a, int32_t b);
    void mergeBootstrapNodes(int toPartition, int fromPartition, int numNodes);

    inline void incLandmarkPeerSize() { landmarkPeerSize++; }
    inline uint16_t getLandmarkPeerSize() { return landmarkPeerSize; }
    inline void incLandmarkPeerSizePerType(uint16_t type) { landmarkPeerSizePerType[type]++; }

protected:
    /**
     * Init member function of module
     */
    virtual void initialize();

    /**
     * HandleMessage member function of module
     *
     * @param msg messag to handle
     */
    virtual void handleMessage(cMessage* msg);

    /**
     * Member function to create keylist
     *
     * @param size size of new keylist
     */
    virtual void createKeyList(uint32_t size);

    KeyList keyList; /**< the keylist */
    uint16_t landmarkPeerSize;
    uint16_t landmarkPeerSizePerType[MAX_NODETYPES];
    uint32_t preKilledNodes; /**< number of nodes marked for deletion in the peer set */
    double maliciousNodeRatio; /**< ratio of current malicious nodes when changing the ratio dynamically */
    cOutVector maliciousNodesVector; /**< vector that records the cange of malicious node rate */
    PeerStorage peerStorage; /**< Set of nodes participating in the overlay */

    // key distribution parameters TODO should be put into an other module
    uint32_t maxNumberOfKeys; /**< parameter used by createKeyList() */
    double keyProbability; /**< probability of keys to be owned by nodes */
    bool isKeyListInitialized;

private:
    GlobalStatistics* globalStatistics; /**< pointer to GlobalStatistics module in this node */
    bool connectionMatrix[MAX_NODETYPES][MAX_NODETYPES]; /**< matrix specifices with node types (partitions) can communication */
};

#endif
