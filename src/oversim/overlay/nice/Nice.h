//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file Nice.h
 * @author Christian Huebsch
 * @author Dimitar Toshev
 */

#ifndef __NICE_H_
#define __NICE_H_

namespace oversim
{

class Nice;

}

#include <BaseOverlay.h>
#include "NiceCluster.h"
#include "NiceMessage_m.h"
#include "NicePeerInfo.h"
#include <hashWatch.h>
#include <vector>
#include "combination.h"
#include <algorithm>

#include <fstream>

namespace oversim
{

/**
 * NICE overlay module
 *
 * Implementation of the NICE overlay as described in
 * "Scalable Application Layer Multicast" by
 * S. Banerjee and B. Bhattacharjee and C. Kommareddy,
 * published at SIGCOMM'02, 2002.
 *
 * @author Christian Huebsch
 */
class Nice : public BaseOverlay
{

    /* The maximum of supported layers, fix for easier handling of structures */
    const static short maxLayers = 10;

    friend class NicePeerInfo;

public:

    /* Define set of TransportAdresses and iterator */
    typedef std::set<TransportAddress> TaSet;
    typedef std::set<TransportAddress>::iterator TaSetIt;

    /* Constructor and Destructor */
    Nice();
    virtual ~Nice();

    /* See BaseOverlay.h */
    virtual void initializeOverlay( int stage );

    virtual void handleTimerEvent(cMessage* msg);

    virtual void handleUDPMessage(BaseOverlayMessage* msg);

    virtual void handleAppMessage(cMessage* msg);

    virtual void finishOverlay();


protected:

    /**
     * changes node state
     *
     * @param toState state to change to
     */
    virtual void changeState(int toState);

    /* see BaseOverlay.h */
    virtual void joinOverlay();

    virtual void handleNodeLeaveNotification();


private:

    int pimp;

    TransportAddress RendevouzPoint;

    bool isRendevouzPoint;

    /* Periodic Self-Messages and their intervals */

    /* Timer for heartbeats by every member node */
    cMessage* heartbeatTimer;
    simtime_t heartbeatInterval;

    /* Timer structure maintenance */
    cMessage* maintenanceTimer;
    simtime_t maintenanceInterval;

    /* Timer for query timeouts */
    cMessage* queryTimer;
    simtime_t queryInterval;

    /* Timer poll the Rendevouz Point */
    cMessage* rpPollTimer;
    simtime_t rpPollTimerInterval;

    double peerTimeoutHeartbeats;

    /* For Visualization only */
    cMessage* visualizationTimer;

    void updateVisualization();

    /* Double leader resolution */
    simtime_t first_HB;
    TransportAddress first_leader;
    simtime_t second_HB;
    TransportAddress second_leader;

    std::vector<std::pair<TransportAddress, simtime_t> > leaderHeartbeats;

    /* DEBUG */
    int clusterrefinement;
    int debug_heartbeats;
    int debug_visualization;
    int debug_join;
    int debug_peertimeouts;
    int debug_removes;
    int debug_queries;

    /* Cluster parameter k */
    unsigned short k;

    /* Cluster Information */
    NiceCluster clusters[maxLayers];

    /* Holds the current layer we query, if we do */
    int evalLayer;
    int joinLayer;

    /* The address of the point which we polled as RendevouzPoint */
    TransportAddress polledRendevouzPoint;

    /* Holds the query start time for RTT evaluation */
    simtime_t query_start;

    /* The current queried node */
    TransportAddress tempResolver;

    /* distance to first queried node that answers */
    simtime_t query_compare;

    /* Layer intended to join */
    short targetLayer;

    /* Map for all peer infos */
    std::map<TransportAddress, NicePeerInfo*> peerInfos;

    /* set holding temporary peered joiner nodes */
    std::map<TransportAddress, simtime_t> tempPeers;
    bool isTempPeered;

    double CLUSTERLEADERBOUND;
    double CLUSTERLEADERCOMPAREDIST;
    double SC_PROC_DISTANCE;
    double SC_MIN_OFFSET;

    // Statistics
    int numInconsistencies;

    int numQueryTimeouts;

    int numPeerTimeouts;

    int numTempPeerTimeouts;

    int numStructurePartitions;

    int numOwnMessagesReceived;

    double totalSCMinCompare;

    int numJoins;

    int numForward;

    int totalForwardBytes;

    int numReceived;

    int totalReceivedBytes;

    int numHeartbeat;

    int totalHeartbeatBytes;

    /* Find highest layer in which node is leader */
    int getHighestLeaderLayer();

    /* Find highest layer in which node resides. Returns -1 when node not yet joined. */
    int getHighestLayer();

    /* Joining the hierarchy from scratch */
    void BasicJoinLayer(short layer);

    /* Splits a cluster */
    void ClusterSplit(int layer);

    /* Request sublayer information from a cluster leader */
    void Query(const TransportAddress& node, short layer);

    /* NICE message handlers */
    void handleNiceClusterMergeRequest(NiceClusterMerge* mergeMsg);

    void handleNiceForceMerge(NiceMessage* msg);

    void handleNiceHeartbeat(NiceHeartbeat* msg);

    void handleNiceLeaderHeartbeat(NiceLeaderHeartbeat* lhbMsg);

    void handleNiceLeaderTransfer(NiceLeaderHeartbeat* transferMsg);

    void handleNiceJoinCluster(NiceMessage* joinMsg);

    void handleNiceJoineval(NiceMessage* msg);

    void handleNiceJoinevalResponse(NiceMessage* msg);

    void handleNiceLeaderHeartbeatOrTransfer(NiceMessage* msg);

    void handleNiceMulticast(NiceMulticastMessage* multicastMsg);

    void handleNicePeerTemporary(NiceMessage* msg);

    void handleNicePeerTemporaryRelease(NiceMessage* msg);

    void handleNicePingProbe(NiceMessage* msg);

    void handleNicePingProbeResponse(NiceMessage* msg);

    void handleNicePollRp(NiceMessage* msg);

    void handleNicePollRpResponse(NiceMessage* msg);

    void handleNiceQuery(NiceMessage* queryMsg);

    void handleNiceQueryResponse(NiceMemberMessage* queryRspMsg);

    void handleNiceRemove(NiceMessage* msg);


    /* Joins a cluster */
    void JoinCluster(const TransportAddress& leader, short layer);

    /* Sends heartbeats to all clusters the node resides in */
    void sendHeartbeats();

    /**
     * Cleans the tempPeers map from peers that have timed out,
     * removes timed-out peers in peerInfos and also removes them from the clusters.
     */
    void cleanPeers();

    /**
     * Splits clusters if needed.
     * Returns true if a split was made, false otherwise.
     */
    bool splitNeeded();

    /**
     * Merges clusters if needed.
     * Returns true if a merge was made, false otherwise.
     */
    bool mergeNeeded();

    /**
     * Checks if we have received conflicting leader heartbeats.
     * Returns true if a collision was detected.
     */
    bool checkLeaderHeartbeatsForCollisions(NiceLeaderHeartbeat* hbMsg);

    /* Periodic maintenance of the protocol hierarchy */
    void maintenance();

    /* Determines the max distance of a member in a cluster */
    simtime_t getMaxDistance(TransportAddress member, const std::set<TransportAddress>& neighbors);
    template <class ConstIter>
    simtime_t getMaxDistance(TransportAddress member, ConstIter neighborsBegin, ConstIter neighborsEnd);

    /* Determines the mean distance of a cluster */
    simtime_t getMeanDistance(std::set<TransportAddress> neighbors);

    /* Initiates a LeaderTransfer message */
    void LeaderTransfer(int layer, TransportAddress leader, TaSet cluster, TransportAddress sc_leader, TaSet superCluster);

    /* Transfers leadership of whole cluster to given node */
    void LeaderTransfer(int layer, TransportAddress leader);

    /* Removes node from cluster */
    void Remove(int layer);

    /* Sends Heartbeat to specific member */
    void sendHeartbeatTo(const TransportAddress& node, int layer);

    /* Sends a remove message to the given node at layer */
    void sendRemoveTo(const TransportAddress& node, int layer);

    /* Merges two clusters */
    void ClusterMerge(int layer);

    /* Sends a ClusterMerge request message */
    void ClusterMergeRequest(const TransportAddress& node, int layer);

    /* Leaves structure gracefully */
    void gracefulLeave(short bottomLayer);

    /* Determines the center of a cluster */
    std::pair<TransportAddress, simtime_t> findCenter(TaSet cluster, bool allowRandom = false);
    std::pair<TransportAddress, simtime_t> findCenter(std::vector<TransportAddress> cluster, bool allowRandom = false);
    std::pair<TransportAddress, simtime_t> findCenter(const NiceCluster& cluster, bool allowRandom = false);
    template <class ConstIter>
    std::pair<TransportAddress, simtime_t> findCenter(ConstIter begin, ConstIter end, bool allowRandom = false);

    void sendDataToOverlay(NiceMulticastMessage *appMsg);

    void pollRP(int layer);

};

}; //namespace

#endif

