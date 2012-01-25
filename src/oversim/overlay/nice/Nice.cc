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
 * @file Nice.cc
 * @author Christian Huebsch
 * @author Dimitar Toshev
 */

#include <stdio.h>

#include <GlobalStatistics.h>
#include "SimpleInfo.h"
#include "SimpleNodeEntry.h"
#include "SimpleUDP.h"
#include "GlobalNodeListAccess.h"
#include <BootstrapList.h>
#include "Nice.h"

namespace oversim
{

/**
 * Define colors for layers in visualization
 */
const char *clustercolors[] = { "yellow",
                                "magenta",
                                "red",
                                "orange",
                                "green",
                                "aquamarine",
                                "cyan",
                                "blue",
                                "navy",
                                "yellow"
                              };

const char *clusterarrows[] = { "m=m,50,50,50,50;ls=yellow,2",
                                "m=m,50,50,50,50;ls=magenta,3",
                                "m=m,50,50,50,50;ls=red,4",
                                "m=m,50,50,50,50;ls=orange,5",
                                "m=m,50,50,50,50;ls=green,6",
                                "m=m,50,50,50,50;ls=aquamarine,7",
                                "m=m,50,50,50,50;ls=cyan,8",
                                "m=m,50,50,50,50;ls=blue,9",
                                "m=m,50,50,50,50;ls=navy,10",
                                "m=m,50,50,50,50;ls=yellow,11"
                              };

Define_Module(Nice);

const short Nice::maxLayers;

/******************************************************************************
 * Constructor
 */
Nice::Nice() : isRendevouzPoint(false),
               isTempPeered(false),
               numInconsistencies(0),
               numQueryTimeouts(0),
               numPeerTimeouts(0),
               numTempPeerTimeouts(0),
               numStructurePartitions(0),
               numOwnMessagesReceived(0),
               totalSCMinCompare(0),
               numJoins(0),
               totalForwardBytes(0),
               numReceived(0),
               totalReceivedBytes(0),
               numHeartbeat(0),
               totalHeartbeatBytes(0)
{

    /* do nothing at this point of time, OverSim calls initializeOverlay */

} // Nice


/******************************************************************************
 * Destructor
 */
Nice::~Nice()
{

    // destroy self timer messages
    cancelAndDelete(heartbeatTimer);
    cancelAndDelete(maintenanceTimer);
    cancelAndDelete(rpPollTimer);
    cancelAndDelete(queryTimer);
    cancelAndDelete(visualizationTimer);

    std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.begin();

    for (; it != peerInfos.end(); it++) {

        delete it->second;

    }

} // ~NICE


/******************************************************************************
 * initializeOverlay
 * see BaseOverlay.h
 */
void Nice::initializeOverlay( int stage )
{

    /* Because of IPAddressResolver, we need to wait until interfaces
     * are registered, address auto-assignment takes place etc. */
    if (stage != MIN_STAGE_OVERLAY)
        return;

    /* Set appearance of node in visualization */
    getParentModule()->getParentModule()->getDisplayString().setTagArg("i", 0, "device/pc_vs");
    getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 0, "block/circle_vs");

    /* Initially clear all clusters */
    for (int i=0; i<maxLayers; i++) {

        for (TaSet::iterator itn = clusters[i].begin(); itn != clusters[i].end(); ++itn) {
            deleteOverlayNeighborArrow(*itn);
        }
        clusters[i].clear();

    }

    /* Initialize Self-Messages */

    // Periodic Heartbeat Messages
    heartbeatInterval = par("heartbeatInterval");
    heartbeatTimer = new cMessage("heartbeatTimer");

    // Periodic Protocol Maintenance
    maintenanceInterval = par("maintenanceInterval");
    maintenanceTimer = new cMessage("maintenanceTimer");

    queryInterval = par("queryInterval");
    queryTimer = new cMessage("queryTimer");

    rpPollTimer = new cMessage("rpPollTimer");
    rpPollTimerInterval = par("rpPollTimerInterval");

    peerTimeoutHeartbeats = par("peerTimeoutHeartbeats");

    pimp = par("enhancedMode");

    isRendevouzPoint = false;

    polledRendevouzPoint = TransportAddress::UNSPECIFIED_NODE;

    /* DEBUG */
    clusterrefinement = par("debug_clusterrefinement");
    debug_heartbeats = par("debug_heartbeats");
    debug_visualization = par("debug_visualization");
    debug_join = par("debug_join");
    debug_peertimeouts = par("debug_peertimeouts");
    debug_removes = par("debug_removes");
    debug_queries = par("debug_queries");

    visualizationTimer = new cMessage("visualizationTimer");

    /* Read cluster parameter k */
    k = par("k");

    CLUSTERLEADERBOUND = par("clusterLeaderBound");
    CLUSTERLEADERCOMPAREDIST = par("clusterLeaderCompareDist");
    SC_PROC_DISTANCE = par("scProcDistance");
    SC_MIN_OFFSET = par("scMinOffset");

    /* Add own node to peerInfos */
    NicePeerInfo* pi = new NicePeerInfo(this);
    pi->set_distance(0);
    peerInfos.insert(std::make_pair(thisNode, pi));

    /* Set evaluation layer to not specified */
    evalLayer = -1;
    joinLayer = -1;

    first_leader = TransportAddress::UNSPECIFIED_NODE;
    second_leader = TransportAddress::UNSPECIFIED_NODE;

    // add some watches
    WATCH(thisNode);
    WATCH_POINTER_MAP(peerInfos);
    WATCH(evalLayer);
    WATCH(query_start);
    WATCH(heartbeatTimer);
    WATCH_MAP(tempPeers);
    WATCH(RendevouzPoint);
    WATCH(isRendevouzPoint);

    WATCH(numInconsistencies);
    WATCH(numQueryTimeouts);
    WATCH(numPeerTimeouts);
    WATCH(numTempPeerTimeouts);
    WATCH(numStructurePartitions);
    WATCH(numOwnMessagesReceived);
    WATCH(totalSCMinCompare);
    WATCH(numJoins);
    WATCH(totalForwardBytes);
    WATCH(numReceived);
    WATCH(totalReceivedBytes);
    WATCH(numHeartbeat);
    WATCH(totalHeartbeatBytes);

} // initializeOverlay


/******************************************************************************
 * joinOverlay
 * see BaseOverlay.h
 */
void Nice::joinOverlay()
{
    changeState(INIT);
    changeState(BOOTSTRAP);
} // joinOverlay

void Nice::handleNodeLeaveNotification() {
    if (isRendevouzPoint) {
        opp_error("The NICE Rendevouz Point is being churned out and the simulation cannot continue. "
                "Please, check your config and make sure that the Chrun Generator's configuration is correct. "
                "Specifically, the Rendevouz Point must not get churned out during the simulation.");
    }
}

/******************************************************************************
 * changeState
 * see BaseOverlay.h
 */
void Nice::changeState( int toState )
{
    switch (toState) {

    case INIT:

        state = INIT;

        getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, "red");

        scheduleAt(simTime() + 1, visualizationTimer);

        break;

    case BOOTSTRAP:

        state = BOOTSTRAP;

        /* get rendevouz point */
        RendevouzPoint = bootstrapList->getBootstrapNode();
        if (RendevouzPoint.isUnspecified()) {

            RendevouzPoint = thisNode;
            isRendevouzPoint = true;

            /* join cluster layer 0 as first node */
            clusters[0].add(thisNode);
            clusters[0].setLeader(thisNode);

            changeState(READY);

            return;

        }
        else {

            pollRP(-1);

            /* initiate NICE structure joining */
            BasicJoinLayer(-1);

        }

        break;

    case READY:

        state = READY;

        cancelEvent(heartbeatTimer);
        scheduleAt(simTime() + heartbeatInterval, heartbeatTimer);
        cancelEvent(maintenanceTimer);
        scheduleAt(simTime() + maintenanceInterval, maintenanceTimer);

        getParentModule()->getParentModule()->getDisplayString().setTagArg
        ("i2", 1, clustercolors[getHighestLayer()]);

        setOverlayReady(true);
        /* allow only rendevouz point to be bootstrap node */
        if (!isRendevouzPoint) bootstrapList->removeBootstrapNode(thisNode);

        break;

    }

} // changeState


/******************************************************************************
 * changeState
 * see BaseOverlay.h
 */
void Nice::handleTimerEvent( cMessage* msg )
{

    if (msg->isName("visualizationTimer")) {

        updateVisualization();
        scheduleAt(simTime() + 1, visualizationTimer);

    }
    else if (msg->isName("heartbeatTimer")) {

        sendHeartbeats();
        scheduleAt(simTime() + heartbeatInterval, heartbeatTimer);

    }
    else if (msg->isName("maintenanceTimer")) {

        maintenance();
        cancelEvent(maintenanceTimer);
        scheduleAt(simTime() + maintenanceInterval, maintenanceTimer);

    }
    else if (msg->isName("queryTimer")) {

        RECORD_STATS(++numInconsistencies; ++numQueryTimeouts);
        if (!tempResolver.isUnspecified() && 
                (tempResolver == RendevouzPoint ||
                 (!polledRendevouzPoint.isUnspecified() && tempResolver == polledRendevouzPoint))) {
            Query(RendevouzPoint, joinLayer);
        }
        else {
            Query(tempResolver, joinLayer);
        }

    }
    else if (msg->isName("rpPollTimer")) {

        pollRP(-1);

    }

} // handleTimerEvent


/******************************************************************************
 * handleUDPMessage
 * see BaseOverlay.h
 */
void Nice::handleUDPMessage(BaseOverlayMessage* msg)
{

    // try message cast to NICE base message
    if (dynamic_cast<NiceMessage*>(msg) != NULL) {

        NiceMessage* niceMsg = check_and_cast<NiceMessage*>(msg);

        // First of all, update activity information for sourcenode
        std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(niceMsg->getSrcNode());

        if (it != peerInfos.end()) {

            it->second->touch();

        }

        /* Dispatch message, possibly downcasting to a more concrete type */
        switch (niceMsg->getCommand()) {

            /* More concrete types, to which the message is cast if needed */
            NiceMemberMessage* queryRspMsg;
            NiceClusterMerge* mergeMsg;
            NiceMulticastMessage* multicastMsg;

            case NICE_QUERY:

                handleNiceQuery(niceMsg);

                break;

            case NICE_QUERY_RESPONSE:

                queryRspMsg = check_and_cast<NiceMemberMessage*>(niceMsg);
                handleNiceQueryResponse(queryRspMsg);

                break;

            case NICE_JOIN_CLUSTER:

                handleNiceJoinCluster(niceMsg);

                break;

            case NICE_POLL_RP:

                handleNicePollRp(niceMsg);

                break;

            case NICE_POLL_RP_RESPONSE:

                handleNicePollRpResponse(niceMsg);

                break;

            case NICE_HEARTBEAT:

                handleNiceHeartbeat(check_and_cast<NiceHeartbeat*>(niceMsg));

                break;

            case NICE_LEADERHEARTBEAT:

                handleNiceLeaderHeartbeat(check_and_cast<NiceLeaderHeartbeat*>(niceMsg));

                break;

            case NICE_LEADERTRANSFER:

                handleNiceLeaderTransfer(check_and_cast<NiceLeaderHeartbeat*>(niceMsg));

                break;

            case NICE_JOINEVAL:

                handleNiceJoineval(niceMsg);

                break;

            case NICE_JOINEVAL_RESPONSE:

                handleNiceJoinevalResponse(niceMsg);

                break;

            case NICE_REMOVE:

                handleNiceRemove(niceMsg);

                break;

            case NICE_PEER_TEMPORARY:

                handleNicePeerTemporary(niceMsg);

                break;

            case NICE_PEER_TEMPORARY_RELEASE:

                handleNicePeerTemporaryRelease(niceMsg);

                break;

            case NICE_PING_PROBE:

                handleNicePingProbe(niceMsg);

                break;

            case NICE_PING_PROBE_RESPONSE:

                handleNicePingProbeResponse(niceMsg);

                break;

            case NICE_FORCE_MERGE:

                handleNiceForceMerge(niceMsg);

                break;

            case NICE_CLUSTER_MERGE_REQUEST:

                mergeMsg = check_and_cast<NiceClusterMerge*>(niceMsg);

                handleNiceClusterMergeRequest(mergeMsg);

                break;

            case NICE_MULTICAST:

                multicastMsg = check_and_cast<NiceMulticastMessage*>(msg);

                handleNiceMulticast(multicastMsg);

                break;

            default:
                
                delete niceMsg;
        }
    }
    else {
        delete msg;
    }
} // handleUDPMessage


/******************************************************************************
 * finishOverlay
 * see BaseOverlay.h
 */
void Nice::finishOverlay()
{

    if (isRendevouzPoint) {
        RendevouzPoint = TransportAddress::UNSPECIFIED_NODE;
    }

    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;

    globalStatistics->addStdDev("Nice: Inconsistencies/s", (double)numInconsistencies / time);
    globalStatistics->addStdDev("Nice: Query Timeouts/s", (double)numQueryTimeouts / time);
    globalStatistics->addStdDev("Nice: Peer Timeouts/s", (double)numPeerTimeouts / time);
    globalStatistics->addStdDev("Nice: Temporary Peer Timeouts/s", (double)numTempPeerTimeouts / time);
    globalStatistics->addStdDev("Nice: Structure Partitions/s", (double)numStructurePartitions / time);
    globalStatistics->addStdDev("Nice: Own Messages Received/s", (double)numOwnMessagesReceived / time);
    globalStatistics->addStdDev("Nice: SC Minimum Compare/s", (double)totalSCMinCompare / time);
    globalStatistics->addStdDev("Nice: Received JOIN Messages/s", (double)numJoins / time);
    globalStatistics->addStdDev("Nice: Forwarded Multicast Messages/s", (double)numForward / time);
    globalStatistics->addStdDev("Nice: Forwarded Multicast Bytes/s", (double)totalForwardBytes / time);
    globalStatistics->addStdDev("Nice: Received Multicast Messages/s (subscribed groups only)", (double)numReceived / time);
    globalStatistics->addStdDev("Nice: Received Multicast Bytes/s (subscribed groups only)", (double)totalReceivedBytes / time);
    globalStatistics->addStdDev("Nice: Send Heartbeat Messages/s", (double)numHeartbeat / time);
    globalStatistics->addStdDev("Nice: Send Heartbeat Bytes/s", (double)totalHeartbeatBytes / time);
    if( debug_join ) recordScalar("Nice: Total joins", (double)numJoins);

} // finishOverlay


/******************************************************************************
 * BasicJoinLayer
 * Queries RendevouzPoint, sets targetLayer to the given layer
 * and peers temporarily with the RendevouzPoint.
 */
void Nice::BasicJoinLayer(short layer)
{

    // Cancel timers involved in structure refinement
    /*
    if (layer == -1 || layer == 0) {
        cancelEvent(maintenanceTimer);
        cancelEvent(heartbeatTimer);
    }
    */

    Query(RendevouzPoint, -1);

    if (layer > -1)
        targetLayer = layer;
    else
        targetLayer = 0;

    // Temporary peer with RP for faster data reception
    NiceMessage* msg = new NiceMessage("NICE_PEER_TEMPORARY");
    msg->setSrcNode(thisNode);
    msg->setCommand(NICE_PEER_TEMPORARY);
    msg->setLayer(-1);
    msg->setBitLength(NICEMESSAGE_L(msg));

    sendMessageToUDP(RendevouzPoint, msg);

    isTempPeered = true;

} // BasicJoinLayer


/******************************************************************************
 * Query
 * Sends a query message to destination.
 * Records query_start time.
 * Sets tempResolver to destination.
 * Sets joinLayer to layer.
 */
void Nice::Query(const TransportAddress& destination, short layer)
{
    if (debug_queries)
        EV << simTime() << " : " << thisNode.getIp() << " : Query()" << endl;


    NiceMessage* msg = new NiceMessage("NICE_QUERY");
    msg->setSrcNode(thisNode);
    msg->setCommand(NICE_QUERY);
    msg->setLayer(layer);
    msg->setBitLength(NICEMESSAGE_L(msg));

    query_start = simTime();
    tempResolver = destination;

    cancelEvent(queryTimer);
    scheduleAt(simTime() + queryInterval, queryTimer);

    joinLayer = layer;

    sendMessageToUDP(destination, msg);

    if (debug_queries)
        EV << simTime() << " : " << thisNode.getIp() << " : Query() finished." << endl;

} // Query

/* Functions handling NICE messages */

void Nice::handleNiceQuery(NiceMessage* queryMsg)
{

    if (debug_queries)
        EV << simTime() << " : " << thisNode.getIp() << " : handleNiceQuery()" << endl;

    short layer = queryMsg->getLayer();
    if (layer >= maxLayers) {
        if (debug_queries)
            EV << "Layer " << layer << " >=  max layer " << maxLayers << " ! Returning." << endl;
        delete queryMsg;
        return;
    }

    if (debug_queries)
        EV << " layer before: " << layer << endl;

    if (layer > getHighestLeaderLayer()) {
        if (isRendevouzPoint) {
            if (debug_queries)
                EV << " getHighestLeaderLayer(): " << getHighestLeaderLayer() << " ! RP self-promoting." << endl;

            for (int i = getHighestLeaderLayer() + 1; i <= layer; ++i) {
                clusters[i].add(thisNode);
                clusters[i].setLeader(thisNode);
            }
        }
        else {
            if (debug_queries)
                EV << " getHighestLeaderLayer(): " << getHighestLeaderLayer() << " ! Returning." << endl;

            delete queryMsg;
            return;
        }
    }

    if (layer < 0) {

        if (isRendevouzPoint) {

            /* If layer is < 0, response with highest layer I am leader of */
            if (debug_queries)
                EV << " I am RP." << endl;
            layer = getHighestLeaderLayer();

        }
        else {

            if (debug_queries)
                EV << " I am not RP. Return." << endl;

            if (pimp) {

                /* forward to Rendevouz Point */
                NiceMessage* dup = static_cast<NiceMessage*>(queryMsg->dup());
                sendMessageToUDP(RendevouzPoint, dup);

            }

            delete queryMsg;
            return;

        }

    }

    if (debug_queries)
        EV << " layer after: " << layer << endl;

    if (!clusters[layer].getLeader().isUnspecified()) {

        if (clusters[layer].getLeader() != thisNode) {

            if (pimp) {

                NiceMessage* dup = static_cast<NiceMessage*>(queryMsg->dup());
                sendMessageToUDP(clusters[layer].getLeader(), dup);

            }

            if (debug_queries)
                EV << " I am not leader of this cluster. return." << endl;

            delete queryMsg;
            return;

        }

    }
    else {

        delete queryMsg;
        return;

    }

    NiceMemberMessage* response = new NiceMemberMessage("NICE_QUERY_RESPONSE");
    response->setSrcNode(thisNode);
    response->setCommand(NICE_QUERY_RESPONSE);
    response->setLayer(layer);

    /* Fill in current cluster members except me */
    response->setMembersArraySize(clusters[layer].getSize()-1);

    int j=0;

    for (int i = 0; i < clusters[layer].getSize(); i++) {

        if (clusters[layer].get(i) != thisNode) {

            response->setMembers(j, clusters[layer].get(i));
            if (debug_queries)
                EV << " Response: " << i << " : " << clusters[layer].get(i) << endl;
            j++;

        }

    }

    response->setBitLength(NICEMEMBERMESSAGE_L(response));

    sendMessageToUDP(queryMsg->getSrcNode(), response);

    if (debug_queries)
        EV << " Sent response to: " << queryMsg->getSrcNode() << endl;

    if (debug_queries)
        EV << simTime() << " : " << thisNode.getIp() << " : handleNiceQuery() finished." << endl;

    delete queryMsg;
} // handleNiceQuery

void Nice::handleNiceClusterMergeRequest(NiceClusterMerge* mergeMsg)
{
    EV << simTime() << " : " << thisNode.getIp() << " : NICE_CLUSTER_MERGE_REQUEST" << endl;

    short layer = mergeMsg->getLayer();

    // Only react if I am a leader of this cluster layer

    if (clusters[layer].getLeader().isUnspecified()) {

        EV << simTime() << " : " << thisNode.getIp() << " : NO LEADER! BREAK. NICE_CLUSTER_MERGE_REQUEST finished" << endl;

        delete mergeMsg;

        return;

    }

    if (clusters[layer].getLeader() == thisNode) {

        clusters[layer+1].remove(mergeMsg->getSrcNode());
        deleteOverlayNeighborArrow(mergeMsg->getSrcNode());

        TransportAddress oldLeader = clusters[layer+1].getLeader();
        if (oldLeader.isUnspecified() || oldLeader != thisNode) {
            clusters[layer+1].add(mergeMsg->getNewClusterLeader());
            clusters[layer+1].setLeader(mergeMsg->getNewClusterLeader());
        }

        for (unsigned int i=0; i<mergeMsg->getMembersArraySize(); i++) {

            /* Add new node to cluster */
            clusters[layer].add(mergeMsg->getMembers(i));

            /* Create peer context to joining node */
            /* Check if peer info already exists */
            std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(mergeMsg->getMembers(i));

            if (it != peerInfos.end()) { /* We already know this node */

            }
            else { /* Create PeerInfo object */

                NicePeerInfo* pi = new NicePeerInfo(this);

                pi->set_last_HB_arrival(simTime().dbl());

                peerInfos.insert(std::make_pair(mergeMsg->getMembers(i), pi));

            }

            /* Draw arrow to new member */
            showOverlayNeighborArrow(mergeMsg->getMembers(i), false, clusterarrows[layer]);

            EV << "getHighestLeaderLayer()].getSize(): " << clusters[getHighestLeaderLayer()].getSize() << endl;

#if 0
            if (clusters[getHighestLeaderLayer()].getSize() < 2) {

                // cancel layer
                for (TaSet::iterator itn = clusters[i].begin(); itn != clusters[i].end(); ++itn) {
                    deleteOverlayNeighborArrow(*itn);
                }
                clusters[getHighestLeaderLayer()].clear();

                for (short i=0; i<maxLayers; i++) {

                    if (clusters[i].getSize() > 0) {

                        if (clusters[i].contains(thisNode)) {

                            getParentModule()->getParentModule()->getDisplayString().setTagArg
                                ("i2", 1, clustercolors[i]);

                        }

                    }

                }

            }
#endif

        }

    }
    else { // Forward to new cluster leader

        if (pimp) {

            NiceMemberMessage* dup = static_cast<NiceMemberMessage*>(mergeMsg->dup());
            sendMessageToUDP(clusters[layer].getLeader(), dup);
            delete mergeMsg;
            return;

        }

    }

    if (pimp)
        sendHeartbeats();

    delete mergeMsg;

    EV << simTime() << " : " << thisNode.getIp() << " : NICE_CLUSTER_MERGE_REQUEST finished" << endl;
}

void Nice::handleNiceForceMerge(NiceMessage* msg)
{
    ClusterMergeRequest(msg->getSrcNode(), msg->getLayer());

    delete msg;
}

void Nice::handleNiceHeartbeat(NiceHeartbeat* hbMsg)
{
    if (debug_heartbeats)
        EV << simTime() << " : " << thisNode.getIp() << " : NICE_HEARTBEAT from  " << hbMsg->getSrcNode() << endl;

    /* Update sequence number information and evaluate distance */
    std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(hbMsg->getSrcNode());

    if (it != peerInfos.end()) {

        /* We already know this node */
        // Collect subcluster infos
        it->second->setSubClusterMembers(hbMsg->getSublayermembers());

        it->second->set_last_HB_arrival(simTime().dbl());

        if (it->second->get_backHB(hbMsg->getSeqRspNo()) > 0) {

            /* Valid distance measurement, get value */
            double oldDistance = it->second->get_distance();

            /* Use Exponential Moving Average with factor 0.1 */
            double newDistance = (simTime().dbl() - it->second->get_backHB(hbMsg->getSeqRspNo()) - hbMsg->getHb_delay())/2.0;

            if (oldDistance > 0) {

                it->second->set_distance((0.1 * newDistance) + (0.9 * oldDistance));

            }
            else {

                it->second->set_distance(newDistance);

            }

        }

        it->second->set_last_recv_HB(hbMsg->getSeqNo());

    }

    it = peerInfos.find(hbMsg->getSrcNode());

    if (it != peerInfos.end()) {

        for (unsigned int i=0; i<hbMsg->getMembersArraySize(); i++) {

            it->second->updateDistance(hbMsg->getMembers(i), hbMsg->getDistances(i));

        }

    }

    delete hbMsg;

    if (debug_heartbeats)
        EV << simTime() << " : " << thisNode.getIp() << " : handleHeartbeat() finished.  " << endl;
}

void Nice::handleNiceLeaderHeartbeat(NiceLeaderHeartbeat* lhbMsg)
{
    if (debug_heartbeats)
        EV << simTime() << " : " << thisNode.getIp() << " : NICE_LEADERHEARTBEAT from  " << lhbMsg->getSrcNode() << endl;

    ASSERT(lhbMsg->getSrcNode() != thisNode);

    /* Update sequence number information and evaluate distance */
    std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(lhbMsg->getSrcNode());

    if (it != peerInfos.end()) { /* We already know this node */

        it->second->set_last_HB_arrival(simTime().dbl());

        if (it->second->get_backHB(lhbMsg->getSeqRspNo()) > 0) {

            /* Valid distance measurement, get value */
            it->second->set_distance((simTime().dbl() - it->second->get_backHB(lhbMsg->getSeqRspNo()) - lhbMsg->getHb_delay())/2);

        }

        it->second->set_last_recv_HB(lhbMsg->getSeqNo());

    }

    it = peerInfos.find(lhbMsg->getSrcNode());

    if (it != peerInfos.end()) {

        for (unsigned int i=0; i<lhbMsg->getMembersArraySize(); i++) {

            it->second->updateDistance(lhbMsg->getMembers(i), lhbMsg->getDistances(i));

        }

    }

    // Maintain cluster memberships

    if (lhbMsg->getLayer() > getHighestLayer()) {

        /* Node is not part of this cluster, remove it */
        sendRemoveTo(lhbMsg->getSrcNode(), lhbMsg->getLayer());

        if (debug_heartbeats)
            EV << "Node is not part of this cluster (" << lhbMsg->getLayer() << "), removing it..." << endl;

        delete lhbMsg;
        return;

    }

    if (!clusters[lhbMsg->getLayer()].getLeader().isUnspecified() && 
            clusters[lhbMsg->getLayer()].getLeader() == thisNode) {

        if (debug_heartbeats)
            EV << "Leader collision...";

        if (lhbMsg->getSrcNode() < thisNode) {

            if (debug_heartbeats)
                EV << "...making other leader." << endl;

            if (lhbMsg->getLayer() + 1 <= getHighestLayer()) {
                gracefulLeave(lhbMsg->getLayer() + 1);
            }

            /* Fix visualisation - remove arrows */
            int hbLayer = lhbMsg->getLayer();
            for (TaSet::iterator itn = clusters[hbLayer].begin(); itn != clusters[hbLayer].end(); ++itn) {
                deleteOverlayNeighborArrow(*itn);
            }

            clusters[lhbMsg->getLayer()].add(lhbMsg->getSrcNode());
            clusters[lhbMsg->getLayer()].setLeader(lhbMsg->getSrcNode());
            LeaderTransfer(lhbMsg->getLayer(), lhbMsg->getSrcNode());
        }
        else {
            if (debug_heartbeats)
                EV << "...remaining leader." << endl;
            delete lhbMsg;
            return;
        }
    }
    else if (!clusters[lhbMsg->getLayer()].getLeader().isUnspecified() &&
            clusters[lhbMsg->getLayer()].getLeader() != lhbMsg->getSrcNode()) {
        if (debug_heartbeats)
            EV << "Possible multiple leaders detected... sending remove to " << clusters[lhbMsg->getLayer()].getLeader() << " leader.\n";
        sendRemoveTo(clusters[lhbMsg->getLayer()].getLeader(), lhbMsg->getLayer());
        
    }

    /* Everything is in order. Process HB */
    bool leaderChanged = clusters[lhbMsg->getLayer()].getLeader().isUnspecified() || clusters[lhbMsg->getLayer()].getLeader() != lhbMsg->getSrcNode();

    for (int m=lhbMsg->getLayer(); m<maxLayers; m++) {
        for (TaSet::iterator itn = clusters[m].begin(); itn != clusters[m].end(); ++itn) {
            deleteOverlayNeighborArrow(*itn);
        }
        clusters[m].clear();
    }

    for (unsigned int i=0; i<lhbMsg->getMembersArraySize(); i++) {

        //Check if member is already part of cluster

        /* Check if peer info already exists */
        std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(lhbMsg->getMembers(i));

        if (it != peerInfos.end()) { /* We already know this node */

        }
        else { /* Create PeerInfo object */

            NicePeerInfo* pi = new NicePeerInfo(this);

            pi->set_last_HB_arrival(simTime().dbl());

            peerInfos.insert(std::make_pair(lhbMsg->getMembers(i), pi));

        }

        clusters[lhbMsg->getLayer()].add(lhbMsg->getMembers(i));

    }

    clusters[lhbMsg->getLayer()].setLeader(lhbMsg->getSrcNode());
    if (!leaderChanged)
        clusters[lhbMsg->getLayer()].confirmLeader();

    if (lhbMsg->getSupercluster_membersArraySize() > 0) {

        for (TaSet::iterator itn = clusters[lhbMsg->getLayer() + 1].begin(); itn != clusters[lhbMsg->getLayer() + 1].end(); ++itn) {
            deleteOverlayNeighborArrow(*itn);
        }
        clusters[lhbMsg->getLayer()+1].clear();

        for (unsigned int i=0; i<lhbMsg->getSupercluster_membersArraySize(); i++) {

            /* Check if peer info already exists */
            std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(lhbMsg->getSupercluster_members(i));

            if (it != peerInfos.end()) { /* We already know this node */

            }
            else { /* Create PeerInfo object */

                NicePeerInfo* pi = new NicePeerInfo(this);

                pi->set_last_HB_arrival(simTime().dbl());

                peerInfos.insert(std::make_pair(lhbMsg->getSupercluster_members(i), pi));

            }

            clusters[lhbMsg->getLayer()+1].add(lhbMsg->getSupercluster_members(i));

        }

        clusters[lhbMsg->getLayer()+1].setLeader(lhbMsg->getSupercluster_leader());

        it = peerInfos.find(lhbMsg->getSrcNode());

        if (it != peerInfos.end()) {

            for (unsigned int k=0; k<lhbMsg->getMembersArraySize(); k++) {

                it->second->updateDistance(lhbMsg->getMembers(k), lhbMsg->getDistances(k));

            }

        }
        else {

            NicePeerInfo* pi = new NicePeerInfo(this);

            pi->set_last_HB_arrival(simTime().dbl());

            peerInfos.insert(std::make_pair(lhbMsg->getSrcNode(), pi));

        }
    }

    if (debug_heartbeats)
        EV << simTime() << " : " << thisNode.getIp() << " : handleNiceLeaderHeartbeat() finished.  " << endl;

    delete lhbMsg;
}

void Nice::handleNiceLeaderTransfer(NiceLeaderHeartbeat* transferMsg)
{

    if (debug_heartbeats)
        EV << simTime() << " : " << thisNode.getIp() << " : NICE_LEADERTRANSFER from " << transferMsg->getSrcNode() << " for " << transferMsg->getLayer() << endl;

    if (!clusters[transferMsg->getLayer()].getLeader().isUnspecified()) {

        /* React only if I am not already leader */
        if (clusters[transferMsg->getLayer()].getLeader() != thisNode) {

            if (debug_heartbeats)
                EV << "I am not already leader of this cluster layer." << endl;

            for (TaSet::iterator itn = clusters[transferMsg->getLayer()].begin(); itn != clusters[transferMsg->getLayer()].end(); ++itn) {
                deleteOverlayNeighborArrow(*itn);
            }
            clusters[transferMsg->getLayer()].clear();
            clusters[transferMsg->getLayer()].add(thisNode);
            clusters[transferMsg->getLayer()].setLeader(thisNode);

            for (unsigned int i=0; i<transferMsg->getMembersArraySize(); i++) {

                if (debug_heartbeats)
                    EV << "Adding: " << transferMsg->getMembers(i) << endl;

                clusters[transferMsg->getLayer()].add(transferMsg->getMembers(i));
                showOverlayNeighborArrow(transferMsg->getMembers(i), false, clusterarrows[transferMsg->getLayer()]);

                std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(transferMsg->getMembers(i));

                if (it != peerInfos.end()) {

                    /* We already know this node */
                    it->second->touch();

                }
                else {

                    //We don't know him yet
                    NicePeerInfo* pi = new NicePeerInfo(this);

                    pi->set_last_HB_arrival(simTime().dbl());

                    peerInfos.insert(std::make_pair(transferMsg->getMembers(i), pi));

                }

            }

            if (transferMsg->getSupercluster_membersArraySize() > 0) {

                for (TaSet::iterator itn = clusters[transferMsg->getLayer() + 1].begin(); itn != clusters[transferMsg->getLayer() + 1].end(); ++itn) {
                    deleteOverlayNeighborArrow(*itn);
                }
                clusters[transferMsg->getLayer()+1].clear();

                for (unsigned int i=0; i<transferMsg->getSupercluster_membersArraySize(); i++) {

                    clusters[transferMsg->getLayer()+1].add(transferMsg->getSupercluster_members(i));

                    std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(transferMsg->getSupercluster_members(i));

                    if (it != peerInfos.end()) {

                        /* We already know this node */
                        it->second->touch();

                    }
                    else {

                        //We don't know him yet
                        NicePeerInfo* pi = new NicePeerInfo(this);

                        pi->set_last_HB_arrival(simTime().dbl());

                        peerInfos.insert(std::make_pair(transferMsg->getSupercluster_members(i), pi));

                    }

                }

                // experimental
                clusters[transferMsg->getLayer()+1].add(thisNode);

                if (!transferMsg->getSupercluster_leader().isUnspecified()) {

                    clusters[transferMsg->getLayer()+1].setLeader(transferMsg->getSupercluster_leader());

                    if ((clusters[transferMsg->getLayer()+1].getLeader() == thisNode) &&
                            (clusters[transferMsg->getLayer()+2].getSize() == 0)) {

                        for (unsigned int i=0; i<transferMsg->getSupercluster_membersArraySize(); i++) {

                            showOverlayNeighborArrow(transferMsg->getSupercluster_members(i), false, clusterarrows[transferMsg->getLayer()+1]);

                        }

                    }
                    else {

                        JoinCluster(transferMsg->getSupercluster_leader(), transferMsg->getLayer()+1);

                    }

                }

            }
            else {

                if (!isRendevouzPoint && RendevouzPoint != transferMsg->getSrcNode()) {
                    BasicJoinLayer(transferMsg->getLayer() + 1);
                }

            }

            for (int i=0; i<maxLayers; i++) {

                if (clusters[i].contains(thisNode))
                    getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, clustercolors[i]);

            }

            clusters[transferMsg->getLayer()].setLastLT();


        }
        else {

            for (unsigned int i=0; i<transferMsg->getMembersArraySize(); i++) {

                if (debug_heartbeats)
                    EV << "Adding: " << transferMsg->getMembers(i) << endl;

                clusters[transferMsg->getLayer()].add(transferMsg->getMembers(i));
                showOverlayNeighborArrow(transferMsg->getMembers(i), false, clusterarrows[transferMsg->getLayer()]);

                std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(transferMsg->getMembers(i));

                if (it == peerInfos.end()) {

                    //We don't know him yet
                    NicePeerInfo* pi = new NicePeerInfo(this);

                    pi->set_last_HB_arrival(simTime().dbl());

                    peerInfos.insert(std::make_pair(transferMsg->getMembers(i), pi));

                }

            }

        }

    }

    if (pimp)
        sendHeartbeats();

    if (debug_heartbeats)
        EV << simTime() << " : " << thisNode.getIp() << " : handleNiceLeaderTransfer() finished.  " << endl;

    delete transferMsg;
}

void Nice::handleNiceJoinCluster(NiceMessage* joinMsg)
{
    if (debug_join)
        EV << simTime() << " : " << thisNode.getIp() << " : handleNiceJoinCluster()" << endl;

    short layer = joinMsg->getLayer();

    if (debug_join)
        std::cout << " From : " << joinMsg->getSrcNode() << ", Layer:  " << layer << endl;

    if (!clusters[layer].getLeader().isUnspecified()) {

        if (clusters[layer].getLeader() != thisNode) {

            if (pimp) {

                NiceMessage* dup = static_cast<NiceMessage*>(joinMsg->dup());
                sendMessageToUDP(clusters[layer].getLeader(), dup);

            }

        }
        else {

            RECORD_STATS(++numJoins);

            /* Add new node to cluster */
            clusters[layer].add(joinMsg->getSrcNode());

            /* Create peer context to joining node */
            /* Check if peer info already exists */
            std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(joinMsg->getSrcNode());

            if (it != peerInfos.end()) { /* We already know this node */


            }
            else { /* Create PeerInfo object */

                NicePeerInfo* pi = new NicePeerInfo(this);

                peerInfos.insert(std::make_pair(joinMsg->getSrcNode(), pi));

            }

            /* Draw arrow to new member */
            showOverlayNeighborArrow(joinMsg->getSrcNode(), false, clusterarrows[layer]);

            if (pimp)
                sendHeartbeatTo(joinMsg->getSrcNode(), layer);
        }

    }
    else {

        if (debug_join)
            EV << "Leader unspecified. Ignoring request." << endl;

    }

    if (debug_join)
        EV << simTime() << " : " << thisNode.getIp() << " : handleNiceJoinCluster() finished." << endl;

    delete joinMsg;
} // handleNiceJoinCluster

void Nice::handleNiceJoineval(NiceMessage* msg)
{
    NiceMessage* responseMsg = new NiceMessage("NICE_JOINEVAL_RESPONSE");
    responseMsg->setSrcNode(thisNode);
    responseMsg->setCommand(NICE_JOINEVAL_RESPONSE);
    responseMsg->setLayer(msg->getLayer());

    responseMsg->setBitLength(NICEMESSAGE_L(responseMsg));

    sendMessageToUDP(msg->getSrcNode(), responseMsg);

    delete msg;
}

void Nice::handleNiceJoinevalResponse(NiceMessage* msg)
{
    if (evalLayer > 0 && evalLayer == msg->getLayer()) {

        query_compare = simTime() - query_compare;

        if (query_compare < query_start) {

            Query(msg->getSrcNode(), msg->getLayer()-1);

        }
        else {

            Query(tempResolver, msg->getLayer() - 1);

        }

        evalLayer = -1;
    }

    delete msg;
}

void Nice::handleNiceMulticast(NiceMulticastMessage* multicastMsg)
{
    RECORD_STATS(++numReceived; totalReceivedBytes += multicastMsg->getByteLength());

    /* If it is mine, count */
    if (multicastMsg->getSrcNode() == thisNode) {

        RECORD_STATS(++numOwnMessagesReceived);

    }
    else {

        unsigned int hopCount = multicastMsg->getHopCount();
        hopCount++;

        if (hopCount < 8) {

            RECORD_STATS(++numForward; totalForwardBytes += multicastMsg->getByteLength());

            NiceMulticastMessage* forOverlay = static_cast<NiceMulticastMessage*>(multicastMsg->dup());
            forOverlay->setHopCount(hopCount);
            sendDataToOverlay(forOverlay);

            send(multicastMsg->decapsulate(), "appOut");

        }
    }

    delete multicastMsg;
}

void Nice::handleNicePeerTemporary(NiceMessage* msg)
{
    // Add node to tempPeers
    tempPeers.insert(std::make_pair(msg->getSrcNode(), simTime()));

    delete msg;
}

void Nice::handleNicePeerTemporaryRelease(NiceMessage* msg)
{
    // Remove node from tempPeers
    tempPeers.erase(msg->getSrcNode());
    deleteOverlayNeighborArrow(msg->getSrcNode());

    delete msg;
}

void Nice::handleNicePingProbe(NiceMessage* msg)
{
    // Only answer if I am part of requested layer
    if (clusters[msg->getLayer()].contains(thisNode)) {

        NiceMessage* probe = new NiceMessage("NICE_PING_PROBE");
        probe->setSrcNode(thisNode);
        probe->setCommand(NICE_PING_PROBE_RESPONSE);
        probe->setLayer(msg->getLayer());

        probe->setBitLength(NICEMESSAGE_L(probe));

        sendMessageToUDP(msg->getSrcNode(), probe);

    }
    else {

        //Do nothing

    }

    delete msg;
}

void Nice::handleNicePingProbeResponse(NiceMessage* msg)
{
    //Only react if still in same cluster as when asked
    if (msg->getLayer() == getHighestLayer()+1) {

        std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(msg->getSrcNode());

        if (it != peerInfos.end()) {

            double distance = simTime().dbl() - it->second->getDES();

            it->second->set_distance(distance);
            it->second->touch();

        }

    }

    delete msg;
}

void Nice::handleNicePollRp(NiceMessage* msg)
{
    if (isRendevouzPoint) {

        NiceMessage* response = new NiceMessage("NICE_POLL_RP_RESPONSE");
        response->setSrcNode(thisNode);
        response->setCommand(NICE_POLL_RP_RESPONSE);
        response->setLayer(getHighestLeaderLayer());
        response->setBitLength(NICEMESSAGE_L(response));

        sendMessageToUDP(msg->getSrcNode(), response);

    }
    delete msg;
}

void Nice::handleNicePollRpResponse(NiceMessage* msg)
{
    if (!polledRendevouzPoint.isUnspecified() && polledRendevouzPoint == RendevouzPoint) {

        polledRendevouzPoint = TransportAddress::UNSPECIFIED_NODE;
        cancelEvent(rpPollTimer);

    }

    delete msg;
}

void Nice::handleNiceQueryResponse(NiceMemberMessage* queryRspMsg)
{

    cancelEvent(queryTimer);

    short layer = queryRspMsg->getLayer();

    /* Check layer response */
    if (layer == targetLayer) {

        /* Use member information for own cluster update */
        for (unsigned int i = 0; i < queryRspMsg->getMembersArraySize(); i++) {

            clusters[layer].add(queryRspMsg->getMembers(i));

        }

        clusters[layer].add(queryRspMsg->getSrcNode());

        /* Initiate joining of lowest layer */
        JoinCluster(queryRspMsg->getSrcNode(), layer);

        changeState(READY);

    }
    else {

        /* Evaluate RTT to queried node */
        query_start = simTime() - query_start;

        /* Find out who is nearest cluster member in response, if nodes are given */
        if (queryRspMsg->getMembersArraySize() > 0) {

            NiceMessage* joineval = new NiceMessage("NICE_JOINEVAL");
            joineval->setSrcNode(thisNode);
            joineval->setCommand(NICE_JOINEVAL);
            joineval->setLayer(layer);

            joineval->setBitLength(NICEMESSAGE_L(joineval));

            /* Initiate evaluation with all cluster members */
            for (unsigned int i = 0; i < queryRspMsg->getMembersArraySize(); i++) {

                NiceMessage* dup = static_cast<NiceMessage*>(joineval->dup());

                sendMessageToUDP(queryRspMsg->getMembers(i), dup);

            }

            delete joineval;

        }
        else { // Directly query same node again for lower layer

            Query(queryRspMsg->getSrcNode(), queryRspMsg->getLayer()-1);

        }

        evalLayer = layer;
        query_compare = simTime();

    }

    delete queryRspMsg;
} // handleNiceQueryResponse

void Nice::handleNiceRemove(NiceMessage* msg)
{
    if (debug_removes)
        EV << simTime() << " : " << thisNode.getIp() << " : NICE_REMOVE" << endl;

    if (msg->getSrcNode() == thisNode) {
        if (debug_removes)
            EV << simTime() << " : " << thisNode.getIp() << " : received remove from self. Disregard.";
        delete msg;
        return;
    }

    short layer = msg->getLayer();

    if (pimp) {
        if (!clusters[layer].getLeader().isUnspecified()) {
            if (clusters[layer].getLeader() != thisNode && (clusters[layer].getLeader() != msg->getSrcNode())) {

                NiceMessage* dup = static_cast<NiceMessage*>(msg->dup());
                sendMessageToUDP(clusters[layer].getLeader(), dup);
                delete msg;
                return;
            }
        }
    }

    if (debug_removes)
        EV << simTime() << " : " << thisNode.getIp() << " : removing " << msg->getSrcNode() << " from layer " << layer << endl;

    if (!clusters[msg->getLayer()].getLeader().isUnspecified()) {

        if (clusters[msg->getLayer()].getLeader() == thisNode) {

            // check prevents visualization arrows to be deleted by error
            if (clusters[msg->getLayer()].contains(msg->getSrcNode())) {

                clusters[msg->getLayer()].remove(msg->getSrcNode());
                deleteOverlayNeighborArrow(msg->getSrcNode());
                updateVisualization();

            }

        }
    }

    if (debug_removes)
        EV << simTime() << " : " << thisNode.getIp() << " : NICE_REMOVE finished." << endl;

    delete msg;
}

/* End handlers for NICE messages */

/******************************************************************************
 * getHighestLeaderLayer
 */
int Nice::getHighestLeaderLayer()
{
    int layer = getHighestLayer();
    if (!clusters[layer].getLeader().isUnspecified() && clusters[layer].getLeader() == thisNode) {
        return layer;
    }
    else if (layer == -1) {
        return -1;
    }
    else {
        return layer - 1;
    }
} // getHighestLeaderLayer

int Nice::getHighestLayer()
{
    if (clusters[0].getSize() == 0) {
        // Not yet joined to overlay
        return -1;
    }

    int highest = 0;

    while (highest < maxLayers &&
           !clusters[highest].getLeader().isUnspecified() &&
           clusters[highest].getLeader() == thisNode) {
        ++highest;
    }

    if (highest == maxLayers) {
        // we are top leader
        return maxLayers - 1;
    }
    else if (!clusters[highest].contains(thisNode)) {
        // we are top leader. highest is one plus our layer.
        return highest - 1;
    }
    else {
        return highest;
    }
} // getHighestLayer

void Nice::JoinCluster(const TransportAddress& leader, short layer)
{

    if (debug_join)
        EV << simTime() << " : " << thisNode.getIp() << " : JoinCluster()" << endl;

    NiceMessage* msg = new NiceMessage("NICE_JOIN_CLUSTER");
    msg->setSrcNode(thisNode);
    msg->setCommand(NICE_JOIN_CLUSTER);
    msg->setLayer(layer);
    msg->setBitLength(NICEMESSAGE_L(msg));

    sendMessageToUDP(leader, msg);

    /* Create peer context to leader */
    /* Check if peer info already exists */
    std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(leader);

    if (it != peerInfos.end()) { /* We already know this node */

    }
    else { /* Create PeerInfo object */

        NicePeerInfo* pi = new NicePeerInfo(this);

        peerInfos.insert(std::make_pair(leader, pi));

    }

    /* Locally add thisNode, too */
    clusters[layer].add(thisNode);

    /* Set leader for cluster */
    clusters[layer].add(leader);
    clusters[layer].setLeader(leader);

    for (short i=0; i<maxLayers; i++) {

        if (clusters[i].getSize() > 0) {

            if (clusters[i].contains(thisNode)) {

                getParentModule()->getParentModule()->getDisplayString().setTagArg
                ("i2", 1, clustercolors[i]);

            }

        }

    }

    // If not already running, schedule some timers
    if (!heartbeatTimer->isScheduled()) {

        scheduleAt(simTime() + heartbeatInterval, heartbeatTimer);

    }
    if (!maintenanceTimer->isScheduled()) {

        scheduleAt(simTime() + heartbeatInterval, maintenanceTimer);

    }

    if (isTempPeered) {

        // Release temporary peering
        NiceMessage* msg = new NiceMessage("NICE_PEER_TEMPORARY_RELEASE");
        msg->setSrcNode(thisNode);
        msg->setCommand(NICE_PEER_TEMPORARY_RELEASE);
        msg->setLayer(-1);
        msg->setBitLength(NICEMESSAGE_L(msg));

        sendMessageToUDP(RendevouzPoint, msg);

        isTempPeered = false;

    }

    if (debug_join)
        EV << simTime() << " : " << thisNode.getIp() << " : JoinCluster() finished." << endl;

} // JoinCluster


/******************************************************************************
 * sendHeartbeats
 */
void Nice::sendHeartbeats()
{

    /* Go through all cluster layers from top to bottom */

    for (int i=getHighestLayer(); i >= 0; i--) {

        /* Determine if node is cluster leader in this layer */
        if (!clusters[i].getLeader().isUnspecified()) {

            if (clusters[i].getLeader() == thisNode) {
                clusters[i].confirmLeader();

                /* Build heartbeat message with info on all current members */
                NiceLeaderHeartbeat* msg = new NiceLeaderHeartbeat("NICE_LEADERHEARTBEAT");
                msg->setSrcNode(thisNode);
                msg->setCommand(NICE_LEADERHEARTBEAT);
                msg->setLayer(i);
                msg->setOne_hop_distance(simTime().dbl());
                msg->setK(k);
                msg->setSc_tolerance(SC_PROC_DISTANCE);

                msg->setMembersArraySize(clusters[i].getSize());

                /* Fill in members */
                for (int j = 0; j < clusters[i].getSize(); j++) {

                    msg->setMembers(j, clusters[i].get(j));

                }

                /* Fill in distances to members */
                msg->setDistancesArraySize(clusters[i].getSize());

                for (int j = 0; j < clusters[i].getSize(); j++) {

                    std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(clusters[i].get(j));

                    if (it != peerInfos.end()) {

                        msg->setDistances(j, it->second->get_distance());

                    }
                    else {

                        msg->setDistances(j, -1);

                    }

                }

                /* Fill in Supercluster members, if existent */
                if (clusters[i+1].getSize() > 0) {

                    msg->setSupercluster_leader(clusters[i+1].getLeader());

                    msg->setSupercluster_membersArraySize(clusters[i+1].getSize());

                    for (int j = 0; j < clusters[i+1].getSize(); j++) {

                        msg->setSupercluster_members(j, clusters[i+1].get(j));

                    }

                }

                /* Send Heartbeat to all members in cluster, except me */
                for (int j = 0; j < clusters[i].getSize(); j++) {

                    if (clusters[i].get(j) != thisNode) {

                        NiceLeaderHeartbeat *copy = static_cast<NiceLeaderHeartbeat*>(msg->dup());

                        /* Get corresponding sequence numbers out of peerInfo */
                        std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(clusters[i].get(j));

                        if (it != peerInfos.end()) {

                            unsigned int seqNo = it->second->get_last_sent_HB();

                            copy->setSeqNo(++seqNo);

                            it->second->set_backHB(it->second->get_backHBPointer(), seqNo, simTime().dbl());
                            it->second->set_last_sent_HB(seqNo);
                            it->second->set_backHBPointer(!it->second->get_backHBPointer());

                            copy->setSeqRspNo(it->second->get_last_recv_HB());

                            if (it->second->get_last_HB_arrival() > 0) {

                                copy->setHb_delay(simTime().dbl() - it->second->get_last_HB_arrival());

                            }
                            else {

                                copy->setHb_delay(0.0);

                            }

                        }

                        copy->setBitLength(NICELEADERHEARTBEAT_L(msg));

                        RECORD_STATS(++numHeartbeat; totalHeartbeatBytes += copy->getByteLength());

                        sendMessageToUDP(clusters[i].get(j), copy);

                    }

                }

                delete msg;

            }
            else { // I am normal cluster member

                /* Build heartbeat message with info on all current members */
                NiceHeartbeat* msg = new NiceHeartbeat("NICE_HEARTBEAT");
                msg->setSrcNode(thisNode);
                msg->setCommand(NICE_HEARTBEAT);
                msg->setLayer(i);
                msg->setOne_hop_distance(simTime().dbl());

                msg->setSublayermembers(0);
                if (i>0) {
                    if (clusters[i-1].getLeader() == thisNode)
                        msg->setSublayermembers(clusters[i-1].getSize());

                }

                msg->setMembersArraySize(clusters[i].getSize());

                /* Fill in members */
                for (int j = 0; j < clusters[i].getSize(); j++) {

                    msg->setMembers(j, clusters[i].get(j));

                }

                /* Fill in distances to members */
                msg->setDistancesArraySize(clusters[i].getSize());

                for (int j = 0; j < clusters[i].getSize(); j++) {

                    std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(clusters[i].get(j));

                    if (it != peerInfos.end()) {

                        msg->setDistances(j, it->second->get_distance());

                    }
                    else {

                        msg->setDistances(j, -1);

                    }

                }

                /* Send Heartbeat to all members in cluster, except me */
                for (int j = 0; j < clusters[i].getSize(); j++) {

                    if (clusters[i].get(j) != thisNode) {

                        NiceHeartbeat *copy = static_cast<NiceHeartbeat*>(msg->dup());

                        /* Get corresponding sequence number out of peerInfo */
                        std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(clusters[i].get(j));

                        if (it != peerInfos.end()) {

                            unsigned int seqNo = it->second->get_last_sent_HB();

                            copy->setSeqNo(++seqNo);

                            it->second->set_backHB(it->second->get_backHBPointer(), seqNo, simTime().dbl());
                            it->second->set_backHBPointer(!it->second->get_backHBPointer());
                            it->second->set_last_sent_HB(seqNo);

                            copy->setSeqRspNo(it->second->get_last_recv_HB());

                            copy->setHb_delay(simTime().dbl() - it->second->get_last_HB_arrival());

                        }

                        copy->setBitLength(NICEHEARTBEAT_L(msg));

                        RECORD_STATS(++numHeartbeat; totalHeartbeatBytes += copy->getByteLength());

                        sendMessageToUDP(clusters[i].get(j), copy);

                    }

                }

                delete msg;

            }
        }

    }

    // Additionally, ping all supercluster members, if existent
    if (clusters[getHighestLayer()+1].getSize() > 0 && !clusters[getHighestLayer()].getLeader().isUnspecified()) {

        NiceMessage* msg = new NiceMessage("NICE_PING_PROBE");
        msg->setSrcNode(thisNode);
        msg->setCommand(NICE_PING_PROBE);
        msg->setLayer(getHighestLayer()+1);

        msg->setBitLength(NICEMESSAGE_L(msg));

        for (int i=0; i<clusters[getHighestLayer()+1].getSize(); i++) {

            if (clusters[getHighestLayer()+1].get(i) != clusters[getHighestLayer()].getLeader()) {

                NiceMessage* dup = static_cast<NiceMessage*>(msg->dup());

                sendMessageToUDP(clusters[getHighestLayer()+1].get(i), dup);

                std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(clusters[getHighestLayer()+1].get(i));

                if (it != peerInfos.end()) {

                    it->second->set_distance_estimation_start(simTime().dbl());

                }

            }

        }

        delete msg;

    }

} // sendHeartbeats


/******************************************************************************
 * sendHeartbeatTo
 */
void Nice::sendHeartbeatTo(const TransportAddress& node, int layer)
{

    if (clusters[layer].getLeader() == thisNode) {

        /* Build heartbeat message with info on all current members */
        NiceLeaderHeartbeat* msg = new NiceLeaderHeartbeat("NICE_LEADERHEARTBEAT");
        msg->setSrcNode(thisNode);
        msg->setCommand(NICE_LEADERHEARTBEAT);
        msg->setLayer(layer);

        msg->setMembersArraySize(clusters[layer].getSize());

        /* Fill in members */
        for (int j = 0; j < clusters[layer].getSize(); j++) {

            msg->setMembers(j, clusters[layer].get(j));

        }

        /* Fill in distances to members */
        msg->setDistancesArraySize(clusters[layer].getSize());

        for (int j = 0; j < clusters[layer].getSize(); j++) {

            std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(clusters[layer].get(j));

            if (it != peerInfos.end()) {

                msg->setDistances(j, it->second->get_distance());

            }
            else {

                msg->setDistances(j, -1);

            }

        }

        /* Fill in Supercluster members, if existent */
        if (clusters[layer+1].getSize() > 0) {

            msg->setSupercluster_leader(clusters[layer+1].getLeader());

            msg->setSupercluster_membersArraySize(clusters[layer+1].getSize());

            for (int j = 0; j < clusters[layer+1].getSize(); j++) {

                msg->setSupercluster_members(j, clusters[layer+1].get(j));

            }

        }

        /* Get corresponding sequence numbers out of peerInfo */
        std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(node);

        if (it != peerInfos.end()) {

            unsigned int seqNo = it->second->get_last_sent_HB();

            msg->setSeqNo(++seqNo);

            it->second->set_backHB(it->second->get_backHBPointer(), seqNo, simTime().dbl());
            it->second->set_last_sent_HB(seqNo);
            it->second->set_backHBPointer(!it->second->get_backHBPointer());

            msg->setSeqRspNo(it->second->get_last_recv_HB());

            msg->setHb_delay(simTime().dbl() - it->second->get_last_HB_arrival());

        }

        msg->setBitLength(NICELEADERHEARTBEAT_L(msg));

        RECORD_STATS(++numHeartbeat; totalHeartbeatBytes += msg->getByteLength());

        sendMessageToUDP(node, msg);

    }
    else {

        // build heartbeat message with info on all current members
        NiceHeartbeat* msg = new NiceHeartbeat("NICE_HEARTBEAT");
        msg->setSrcNode(thisNode);
        msg->setCommand(NICE_HEARTBEAT);
        msg->setLayer(layer);

        msg->setMembersArraySize(clusters[layer].getSize());

        // fill in members
        for (int j = 0; j < clusters[layer].getSize(); j++) {

            msg->setMembers(j, clusters[layer].get(j));

        }

        // fill in distances to members
        msg->setDistancesArraySize(clusters[layer].getSize());

        for (int j = 0; j < clusters[layer].getSize(); j++) {

            std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(clusters[layer].get(j));

            if (it != peerInfos.end()) {

                msg->setDistances(j, it->second->get_distance());

            }
            else if (clusters[layer].get(j) == thisNode) {

                msg->setDistances(j, 0);

            }
            else {

                msg->setDistances(j, -1);

            }

        }

        msg->setBitLength(NICEHEARTBEAT_L(msg));

        RECORD_STATS(++numHeartbeat; totalHeartbeatBytes += msg->getByteLength());

        sendMessageToUDP(node, msg);

    }

} // sendHeartbeatTo

/**
 * sendRemoveTo
 */
void Nice::sendRemoveTo(const TransportAddress& node, int layer)
{
    NiceMessage* msg = new NiceMessage("NICE_REMOVE");
    msg->setSrcNode(thisNode);
    msg->setCommand(NICE_REMOVE);
    msg->setLayer(layer);

    msg->setBitLength(NICEMESSAGE_L(msg));

    sendMessageToUDP(node, msg);
} // sendRemoveTo

/**
 * cleanPeers
 */
void Nice::cleanPeers()
{
    // Clean tempPeers
    std::vector<TransportAddress> deadTempPeers;
    
    std::map<TransportAddress, simtime_t>::iterator itTempPeer;
    for (itTempPeer = tempPeers.begin(); itTempPeer != tempPeers.end(); ++itTempPeer) {
        if (simTime() > (itTempPeer->second + 3 * heartbeatInterval)) {
            RECORD_STATS(++numTempPeerTimeouts);
            deadTempPeers.push_back(itTempPeer->first);
        }
    }

    std::vector<TransportAddress>::iterator itDead;
    for (itDead = deadTempPeers.begin(); itDead != deadTempPeers.end(); ++itDead) {
        tempPeers.erase(*itDead);
        deleteOverlayNeighborArrow(*itDead);
    }

    /* Delete nodes that haven't been active for too long autonomously */
    std::vector<TransportAddress> deadPeers;

    std::map<TransportAddress, NicePeerInfo*>::iterator itPeer = peerInfos.begin();
    while (itPeer != peerInfos.end()) {

        double offset = peerTimeoutHeartbeats * heartbeatInterval.dbl();
        if (itPeer->first != thisNode && simTime() > (itPeer->second->getActivity() + offset)) {

            if (debug_peertimeouts) {
                EV << simTime() << " : " << thisNode.getIp() << " : PEER TIMED OUT! : " << itPeer->first << endl;
                EV << "Activity : " << itPeer->second->getActivity() << endl;
            }

            RECORD_STATS(++numPeerTimeouts);

            deadPeers.push_back(itPeer->first);

        }

        ++itPeer;

    }

    for (itDead = deadPeers.begin(); itDead != deadPeers.end(); ++itDead) {
        delete peerInfos[*itDead];
        peerInfos.erase(*itDead);

        // Delete nodes from all layer clusters
        for (int i = 0; i < maxLayers; i++) {
            clusters[i].remove(*itDead);
            deleteOverlayNeighborArrow(*itDead);
        }
    }
} // cleanPeers

/**
 * splitNeeded
 */
bool Nice::splitNeeded()
{
    bool splitMade = false;
    // Check if cluster split is necessary
    // Find lowest layer that needs splitting and split it. If we're still cluster leader, continue up.
    for (int i = 0, highest = std::min(getHighestLeaderLayer(), maxLayers - 2);
            i <= highest && !clusters[i].getLeader().isUnspecified() && clusters[i].getLeader() == thisNode;
            ++i) {
        if (clusters[i].getSize() > 3 * k + 1 &&
                clusters[i].isLeaderConfirmed() &&
                (i == maxLayers - 1 || clusters[i + 1].getSize() == 0 || clusters[i + 1].isLeaderConfirmed())) {
            splitMade = true;
            ClusterSplit(i);
        }
    }
    return splitMade;
} // splitNeeded

/**
 * mergeNeeded
 */
bool Nice::mergeNeeded()
{
    if (isRendevouzPoint) {
        // The Rendevouz Point can't initiate a merge, since that would
        // compromise its status as a Rendevouz Point.
        return false;
    }

    // The layer at which we must merge
    int mergeLayer;
    int highestLeaderLayer = getHighestLeaderLayer();
    
    // Find lowest layer that needs merging.
    // The node will disappear from all higher layers.
    for (mergeLayer= 0; mergeLayer <= highestLeaderLayer && mergeLayer < maxLayers - 1; ++mergeLayer) {
        /* Do not attempt merging if we're not sure we belong as leader */
        if (clusters[mergeLayer].getSize() < k && clusters[mergeLayer].isLeaderConfirmed() &&
                clusters[mergeLayer + 1].isLeaderConfirmed()) {
            ClusterMerge(mergeLayer);

            // The merge may fail, check if it did.
            // If it really did, we should try to see if there's some other layer that we may merge.
            if (!clusters[mergeLayer + 1].contains(thisNode)) {
                return true;
            }
        }
    }
    
    return false;
}

/**
 * checkLeaderHeartbeatsForCollisions
 */
bool Nice::checkLeaderHeartbeatsForCollisions(NiceLeaderHeartbeat* hbMsg)
{

    bool collisionDetected = false;

    //Alternative Detection
    leaderHeartbeats.push_back(std::make_pair(hbMsg->getSrcNode(), simTime()));

    if (leaderHeartbeats.size() > 3) {

        if (debug_heartbeats)
            EV << simTime() << "leaderHeartbeats.size() > 3 :  " << leaderHeartbeats.size() << endl;

        simtime_t predecessor =  leaderHeartbeats.at(leaderHeartbeats.size()-2).second;

        if (debug_heartbeats)
            EV << simTime() << "predecessor :  " << predecessor << endl;


        if (simTime() < (predecessor + heartbeatInterval)) {

            if (debug_heartbeats)
                EV << simTime() << "simTime() < (predecessor + heartbeatInterval)" << endl;

            if (leaderHeartbeats.at(leaderHeartbeats.size()-2).first != hbMsg->getSrcNode()) {

                if (debug_heartbeats) {
                    EV << simTime() << "(leaderHeartbeats.at(leaderHeartbeats.size()-2).first != hbMsg->getSrcNode())" << endl;
                    EV << "leaderHeartbeats.at(leaderHeartbeats.size()-2).first: " << leaderHeartbeats.at(leaderHeartbeats.size()-2).first << endl;
                }

                if (leaderHeartbeats.at(leaderHeartbeats.size()-3).first == hbMsg->getSrcNode()) {

                    if (debug_heartbeats) {
                        EV << simTime() << "(leaderHeartbeats.at(leaderHeartbeats.size()-3).first == hbMsg->getSrcNode())" << endl;
                        EV << "leaderHeartbeats.at(leaderHeartbeats.size()-3).first: " << leaderHeartbeats.at(leaderHeartbeats.size()-3).first << endl;
                        EV << "timestamp: " << leaderHeartbeats.at(leaderHeartbeats.size()-3).second << endl;
                    }

                    if (leaderHeartbeats.at(leaderHeartbeats.size()-4).first == leaderHeartbeats.at(leaderHeartbeats.size()-2).first) {

                        if (debug_heartbeats) {
                            EV << simTime() << "(leaderHeartbeats.at(leaderHeartbeats.size()-4).first == leaderHeartbeats.at(leaderHeartbeats.size()-2).first" << endl;
                            EV << "leaderHeartbeats.at(leaderHeartbeats.size()-4).first: " << leaderHeartbeats.at(leaderHeartbeats.size()-4).first << endl;
                            EV << "timestamp: " << leaderHeartbeats.at(leaderHeartbeats.size()-4).second << endl;

                        }

                        if (debug_heartbeats)
                            EV << simTime() << " : " << thisNode.getIp() << " : CONFLICTING LEADERS!" << endl;

                        NiceMessage* removeMsg = new NiceMessage("NICE_REMOVE");
                        removeMsg->setSrcNode(thisNode);
                        removeMsg->setCommand(NICE_REMOVE);
                        removeMsg->setLayer(hbMsg->getLayer());

                        removeMsg->setBitLength(NICEMESSAGE_L(removeMsg));

                        sendMessageToUDP(leaderHeartbeats.at(leaderHeartbeats.size()-2).first, removeMsg);

                        collisionDetected = true;

                    }

                }

            }
        }

    }


    /* Tidy up leaderheartbeats */
    if (leaderHeartbeats.size() > 4) {

        for (unsigned int i=0; i<(leaderHeartbeats.size()-4); i++) {

            leaderHeartbeats.erase(leaderHeartbeats.begin());

        }

    }

    return collisionDetected;
}

/******************************************************************************
 * maintenance
 */
void Nice::maintenance()
{
    // care for structure connection timer
    if (RendevouzPoint.isUnspecified()) {

        EV << "No RendevouzPoint! " << endl;

    }

    int highestLayer = getHighestLayer();
    bool leaderDied = false;
    clusters[highestLayer].isLeaderConfirmed();
    TransportAddress newLeader;
    TransportAddress oldLeader = clusters[highestLayer].getLeader();
    cleanPeers();
    // Cluster Leader died. Select new. If the old leader is unspecified, then we're
    // probably waiting to join the layer.
    if (clusters[highestLayer].getLeader().isUnspecified() && !oldLeader.isUnspecified()) {
        leaderDied = true;
        RECORD_STATS(++numStructurePartitions; ++numInconsistencies);
        newLeader = findCenter(clusters[highestLayer]).first;
    }

    splitNeeded();
    mergeNeeded();

    if (leaderDied && RendevouzPoint != oldLeader) {
        clusters[highestLayer].setLeader(newLeader);
        if (newLeader == thisNode) {
            clusters[highestLayer + 1].add(newLeader);
            BasicJoinLayer(highestLayer + 1);
        }
        else {
            LeaderTransfer(highestLayer, newLeader);
        }
    }
    else if (getHighestLayer() == getHighestLeaderLayer()) {
        if (!isRendevouzPoint) {
            pollRP(-1);
            BasicJoinLayer(getHighestLayer() + 1);
        }
    }

    highestLayer = getHighestLayer();
    for (int i = highestLayer + 1; i < maxLayers; ++i) {
        if (clusters[i].getSize() != 0) {
            EV << "Stale data for cluster " << i << endl;
            for (TaSet::iterator itn = clusters[i].begin(); itn != clusters[i].end(); ++itn) {
                deleteOverlayNeighborArrow(*itn);
            }
            clusters[i].clear();
        }
    }

    // if highest super cluster has more than one member, try to find a closer leader.
    // However, the Rendevouz Point must can't move between clusters.
    if (!isRendevouzPoint && highestLayer < maxLayers - 1 && clusters[highestLayer + 1].getSize() > 1) {

        if (clusterrefinement)
            EV << simTime() << " : " << thisNode.getIp() << " : Look for better parent node in cluster : " << highestLayer + 1 << " ..."<< endl;

        TransportAddress highestLeader = clusters[highestLayer].getLeader();
        std::map<TransportAddress, NicePeerInfo*>::iterator it;
        if (!highestLeader.isUnspecified()) {
            it = peerInfos.find(highestLeader);
        }
        else {
            it = peerInfos.end();
        }

        if (it != peerInfos.end() && it->second->get_distance() > 0) {

            double distance = it->second->get_distance() - ((it->second->get_distance()/100.0) * SC_PROC_DISTANCE);

            double smallest = 10000.0;
            TransportAddress candidate = TransportAddress::UNSPECIFIED_NODE;

            for (int i=0; i < clusters[highestLayer+1].getSize(); i++) {

                if (clusters[highestLayer+1].get(i) != clusters[highestLayer].getLeader()) {

                    std::map<TransportAddress, NicePeerInfo*>::iterator it2 = peerInfos.find(clusters[highestLayer+1].get(i));

                    if (it2 != peerInfos.end()) {

                        if ((it2->second->get_distance() < smallest) && (it2->second->get_distance() > 0)) {
                            smallest = it2->second->get_distance();
                            candidate = it2->first;
                        }

                    }

                }

            }

            std::set<TransportAddress> clusterset;

            for (int m=0; m<clusters[getHighestLayer()+1].getSize(); m++) {

                clusterset.insert(clusters[getHighestLayer()+1].get(m));

            }

            simtime_t meanDistance = getMeanDistance(clusterset);

            simtime_t minCompare = (meanDistance/100.0)*SC_MIN_OFFSET;

            RECORD_STATS(totalSCMinCompare += minCompare.dbl());

            if (minCompare < 0.005)
                minCompare = 0.005;

            if ((smallest < distance) && ((distance - smallest) > minCompare.dbl())) { // change supercluster


                if (clusterrefinement) {
                    EV << simTime() <<" : " << thisNode.getIp() << ": Change SuperCluster! to " << candidate.getIp() << endl;
                    EV << "Old distance ():  " << it->second->get_distance() << endl;
                    EV << "SC_PROC_DISTANCE:  " << SC_PROC_DISTANCE << endl;
                    EV << "Compare distance:  " << distance << endl;
                    EV << "New distance:  " << smallest << endl;
                    EV << "New SC_MIN_OFFSET:  " << SC_MIN_OFFSET << endl;
                }

                // leave old
                Remove(highestLayer);

                // join new
                JoinCluster(candidate, highestLayer);

                return;

            }
        }
        else {

            //Do nothing

        }

    }

    if (!isRendevouzPoint) {

        // Try to find better leader.
        for (int i = getHighestLeaderLayer(); i >= 0; i--) {

            if (clusters[i].getSize() > 1 && clusters[i].isLeaderConfirmed()) {

                bool allDistancesKnown = true;

                if (clusterrefinement)
                    EV << simTime() << " : " << thisNode.getIp() << " : Find better cluster leader in ..." << i << endl;

                /* Only make decisions if node has total distance knowledge in this cluster */
                for (int j = 0; j < clusters[i].getSize() && allDistancesKnown; j++) {

                    /* Check if peer info already exists */
                    std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(clusters[i].get(j));

                    if (it != peerInfos.end()) {

                        simtime_t distance = it->second->get_distance();

                        //EV << "My distance to " << it->first << " : " << distance << endl;

                        if (distance < 0) {
                            allDistancesKnown = false;
                            continue;
                        }

                        for (int k = 0; k < clusters[i].getSize(); k++) {

                            if ((it->first != thisNode) && (clusters[i].get(k) != it->first)) {

                                if (it->second->getDistanceTo(clusters[i].get(k)) < 0) {
                                    allDistancesKnown = false;
                                    break;
                                }
                            }

                        }

                    }
                    else {

                        allDistancesKnown = false;

                    }

                }

                if (allDistancesKnown) {

                    if (clusterrefinement)
                        EV << "Complete distance knowledge available." << endl;

                    // Perform check for better cluster leader
                    TransportAddress new_leader = findCenter(clusters[i]).first;

                    if (clusterrefinement)
                        EV << "NEW LEADER laut " << thisNode.getIp() << " --> " << new_leader.getIp() << endl;

                    std::set<TransportAddress> clusterset;

                    for (int m=0; m<clusters[i].getSize(); m++) {

                        clusterset.insert(clusters[i].get(m));

                    }


                    simtime_t meanDistance = getMeanDistance(clusterset);
                    simtime_t oldDistance = getMaxDistance(clusters[i].getLeader(), clusterset);
                    simtime_t newDistance = getMaxDistance(new_leader, clusterset);
                    simtime_t compareDistance = (oldDistance - ((oldDistance/100.0)*CLUSTERLEADERCOMPAREDIST));

                    simtime_t minCompare = (meanDistance/100.0)*CLUSTERLEADERBOUND;

                    if (minCompare < 0.005)
                        minCompare = 0.005;

                    if ((newDistance.dbl() < compareDistance.dbl()) && ((compareDistance.dbl() - newDistance.dbl()) > minCompare.dbl())) {

                        if (clusterrefinement)
                            EV << "CHANGE " << CLUSTERLEADERCOMPAREDIST << endl;

                        if (new_leader != thisNode) {

                            // Set new leader for this cluster
                            clusters[i].setLeader(new_leader);

                            for (int j=0; j<clusters[i].getSize(); j++) {

                                deleteOverlayNeighborArrow(clusters[i].get(j));

                            }

                            gracefulLeave(i);
                            LeaderTransfer(i, new_leader);

                            getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 0, "block/circle_vs");

                        }

                    }

                    if (clusterrefinement) {
                        EV << "MaxDistance " << new_leader.getIp() << " : " << getMaxDistance(new_leader, clusterset) << endl;
                        EV << "MaxDistance " << clusters[i].getLeader() << " : " << getMaxDistance(clusters[i].getLeader(), clusterset) << endl;
                        EV << "MaxDistance " << thisNode.getIp() << " : " << getMaxDistance(thisNode, clusterset) << endl;
                    }


                }

            } // if cluster i has other members

        } // for i from highest leader layer to 0

    } // if this is not the rendevouz point

} // maintenance


/******************************************************************************
 * ClusterSplit
 */
void Nice::ClusterSplit(int layer)
{

    EV << simTime() << " : " << thisNode.getIp() << " : ClusterSplit in Layer " << layer << endl;

    /* Get cluster to be splitted */
    NiceCluster cluster = clusters[layer];

    /* Introduce some helper structures */
    std::vector<TransportAddress> vec1;
    std::vector<TransportAddress> vec2;
    std::vector<TransportAddress> cl1;
    std::vector<TransportAddress> cl2;
    TaSet cl1set, cl2set;
    TransportAddress cl1_center = TransportAddress::UNSPECIFIED_NODE;
    TransportAddress cl2_center = TransportAddress::UNSPECIFIED_NODE;
    simtime_t min_delay = 999;

    for (int i=0; i<cluster.getSize(); i++) {

        /* Delete all arrows in visualization */
        deleteOverlayNeighborArrow(cluster.get(i));

        /* Put all members to first vector */
        vec1.push_back(cluster.get(i));
        //EV << "vec1.push_back(cluster.get(i)): " << cluster.get(i).getIp() << endl;

        /* Put first half of members to second vector */
        if (i < cluster.getSize()/2) {
            vec2.push_back(cluster.get(i));
            //EV << "vec2.push_back(cluster.get(i)): " << cluster.get(i).getIp() << endl;
        }

    }

    int combinations = 0;

    TaSet::iterator sit;

    if (cluster.getSize() < 18) {

        /* Go through all combinations of clusters */
        do {

            combinations++;

            //EV << "combinations: " << combinations << endl;

            /* Introduce some helper structures */
            TransportAddress q1_center;
            TransportAddress q2_center;
            std::vector<TransportAddress> vec3;

            /* Determine elements that are in first set but not in second */
            std::set_difference(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), inserter(vec3, vec3.begin()));

            simtime_t min_q1_delay = 999;
            simtime_t min_q2_delay = 999;
            simtime_t max_delay = 0;

            q1_center = findCenter(vec2).first;

            //EV << "q1_center: " << q1_center.getIp() << endl;

            std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(q1_center);

            if (it != peerInfos.end()) {

                min_q1_delay = it->second->get_distance();

            }
            else {

                min_q1_delay = 0;

            }

            q2_center = findCenter(vec3).first;

            //EV << "q2_center: " << q2_center.getIp() << endl;

            it = peerInfos.find(q2_center);

            if (it != peerInfos.end()) {

                min_q2_delay = it->second->get_distance();

            }
            else {

                min_q2_delay = 0;

            }

            max_delay = std::max(min_q1_delay, min_q2_delay);

            if (min_delay == 0) min_delay = max_delay;

            if ((max_delay < min_delay) && !q1_center.isUnspecified() && !q2_center.isUnspecified()) {

                min_delay = max_delay;
                cl1 = vec2;
                cl2 = vec3;
                cl1_center = q1_center;
                cl2_center = q2_center;
            }

        } while (next_combination(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()));

        //build sets
        cl1set.insert(cl1.begin(), cl1.end());
        cl2set.insert(cl2.begin(), cl2.end());

    }
    else {
        EV << thisNode.getIp() << " RANDOM SPLIT" << endl;
        cl1set.clear();
        cl2set.clear();
        for (int i=0; i<cluster.getSize(); i++) {
            if (i < cluster.getSize()/2) {
                cl1set.insert(cluster.get(i));
            }
            else {
                cl2set.insert(cluster.get(i));
            }
        }
        cl1_center = findCenter(cl1set,true).first;
        cl2_center = findCenter(cl2set,true).first;
    }

    if (isRendevouzPoint) {
        // Make certain that we remain leader
        if (cl1set.count(thisNode) > 0) {
            cl1_center = thisNode;
        }
        else {
            cl2_center = thisNode;
        }
    }

    // Cluster split accomplished, now handling consequences

    // CASE 1: We lost all cluster leaderships
    // repair all cluster layer, top down
    if ((cl1_center != thisNode) && (cl2_center != thisNode)) {

        clusters[layer+1].add(cl1_center);
        clusters[layer+1].add(cl2_center);
        TaSet superCluster = TaSet(clusters[layer + 1].begin(), clusters[layer + 1].end());
        TransportAddress scLeader;

        if (layer < getHighestLayer()) {
            gracefulLeave(layer+1);
            scLeader = clusters[layer + 1].getLeader();
        }
        else {
            scLeader = cl1_center;
            if (isRendevouzPoint) {
                opp_error("Something went wrong in Nice::ClusterSplit and the RendevouzPoint is trying to give up leadership. This is a bug in OverSim's implementation of Nice. Please fix it, or file a bug.");
            }
        }

        // Leaving the upper layer before sending LTs leads to wrong information in the packets.
        LeaderTransfer(layer, cl1_center, cl1set, scLeader, superCluster);
        LeaderTransfer(layer, cl2_center, cl2set, scLeader, superCluster);

        if (layer < maxLayers - 1) {
            for (TaSet::iterator itn = clusters[layer + 1].begin(); itn != clusters[layer + 1].end(); ++itn) {
                deleteOverlayNeighborArrow(*itn);
            }
            clusters[layer + 1].clear();
        }

        getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 0, "block/circle_vs");

    }

    // CASE 2: We stay leader in one of the new clusters
    if ((cl1_center == thisNode) || (cl2_center == thisNode)) {

        if (clusters[layer + 1].getSize() == 0) {

            clusters[layer + 1].add(cl1_center);
            clusters[layer + 1].add(cl2_center);

            clusters[layer + 1].setLeader(thisNode);
            clusters[layer + 1].confirmLeader();

        }

        clusters[layer + 1].add(cl2_center);
        clusters[layer + 1].add(cl1_center);
        TaSet superCluster = TaSet(clusters[layer + 1].begin(), clusters[layer + 1].end());
        if (cl1_center == thisNode) {

            LeaderTransfer(layer, cl2_center, cl2set, clusters[layer + 1].getLeader(), superCluster);

        }
        else {

            LeaderTransfer(layer, cl1_center, cl1set, clusters[layer + 1].getLeader(), superCluster);

        }


    }

    // Set local cluster info
    clusters[layer].clear();

    // Depends on in which of the two clusters this node is
    if (cl1set.count(thisNode)) {

        TaSet::iterator cit = cl1set.begin();
        while (cit != cl1set.end()) {
            clusters[layer].add(*cit);
            cit++;
        }

        clusters[layer].setLeader(cl1_center);

    }
    else {

        TaSet::iterator cit = cl2set.begin();
        while (cit != cl2set.end()) {
            clusters[layer].add(*cit);
            cit++;
        }

        clusters[layer].setLeader(cl2_center);

    }
    clusters[layer].confirmLeader();

    //update arrows
    updateVisualization();

    if (pimp)
        sendHeartbeats();

} // ClusterSplit


/******************************************************************************
 * ClusterMerge
 */
void Nice::ClusterMerge(int layer)
{

    ASSERT(layer < maxLayers - 1);
    ASSERT(clusters[layer].getLeader() == thisNode);
    simtime_t min_delay = 999;

    TransportAddress min_node = TransportAddress::UNSPECIFIED_NODE;

    for (int i=0; i<clusters[layer+1].getSize(); i++) {

        TransportAddress node = clusters[layer+1].get(i);

        if (node != thisNode) {

            std::map<TransportAddress, NicePeerInfo*>::iterator it = peerInfos.find(node);

            if (it != peerInfos.end()) {
                simtime_t delay = it->second->get_distance();

                if ((delay > 0) && (delay < min_delay)) {

                    min_delay = delay;
                    min_node = node;

                }
            }
        }

    }

    if (!min_node.isUnspecified()) {

        // send merge request
        ClusterMergeRequest(min_node, layer);

        // leave above layer, we are no longer the leader of this cluster.
        gracefulLeave(layer + 1);

        clusters[layer].add(min_node);
        clusters[layer].setLeader(min_node);
        clusters[layer].confirmLeader();

        for (int j=0; j<clusters[layer+1].getSize(); j++) {

            deleteOverlayNeighborArrow(clusters[layer+1].get(j));

        }

        for (int i = 0, highest = getHighestLayer(); i < highest; i++) {

            getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, clustercolors[i]);

        }

    }
    else {

        EV << thisNode.getIp() << " no suitable cluster found";

    }

} // ClusterMerge


/******************************************************************************
 * ClusterMergeRequest
 */
void Nice::ClusterMergeRequest(const TransportAddress& node, int layer)
{
    ASSERT(clusters[layer+1].contains(thisNode));
    ASSERT(!clusters[layer+1].getLeader().isUnspecified());

    NiceClusterMerge* msg = new NiceClusterMerge("NICE_CLUSTER_MERGE_REQUEST");
    msg->setSrcNode(thisNode);
    msg->setCommand(NICE_CLUSTER_MERGE_REQUEST);
    msg->setLayer(layer);

    msg->setMembersArraySize(clusters[layer].getSize());

    /* Fill in members */
    for (int j = 0; j < clusters[layer].getSize(); j++) {

        msg->setMembers(j, clusters[layer].get(j));

        deleteOverlayNeighborArrow(clusters[layer].get(j));

    }

    msg->setNewClusterLeader(clusters[layer+1].getLeader());

    msg->setBitLength(NICECLUSTERMERGE_L(msg));

    getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 0, "block/circle_vs");

    sendMessageToUDP(node, msg);

} // ClusterMergeRequest


/******************************************************************************
 * findCenter
 */
std::pair<TransportAddress,simtime_t> Nice::findCenter(TaSet cluster, bool allowRandom)
{
    return findCenter(cluster.begin(), cluster.end(), allowRandom);
}


/******************************************************************************
 * findCenter
 */
std::pair<TransportAddress, simtime_t> Nice::findCenter(std::vector<TransportAddress> cluster, bool allowRandom)
{
    return findCenter(cluster.begin(), cluster.end(), allowRandom);
}


/******************************************************************************
 * findCenter
 */
std::pair<TransportAddress, simtime_t> Nice::findCenter(const NiceCluster& cluster, bool allowRandom)
{
    return findCenter(cluster.begin(), cluster.end(), allowRandom);
}

/******************************************************************************
 * findCenter
 */
template <class ConstIter>
std::pair<TransportAddress, simtime_t> Nice::findCenter(ConstIter begin, ConstIter end, bool allowRandom)
{

    TransportAddress center = TransportAddress::UNSPECIFIED_NODE;
    simtime_t min_delay = 1000;

    for (ConstIter it = begin; it != end; ++it) {

        simtime_t delay = getMaxDistance(*it, begin, end);

        if ((delay > 0) && (delay < min_delay)) {

            min_delay = delay;
            center = *it;

        }

    }

    if (center.isUnspecified()) {
        center = *begin;
    }

    //EV << "center: " << center << endl;
    return std::make_pair(center, min_delay);

} // findCenter


/******************************************************************************
 * getMaxDistance
 */
template <class ConstIter>
simtime_t Nice::getMaxDistance(TransportAddress member, ConstIter neighborsBegin, ConstIter neighborsEnd)
{
    simtime_t maxDelay = 0;
    simtime_t delay = 0;

    if (member == thisNode) {

        for (ConstIter it = neighborsBegin; it != neighborsEnd; ++it) {

            std::map<TransportAddress, NicePeerInfo*>::iterator itInfo = peerInfos.find(*it);

            if (itInfo != peerInfos.end()) {

                delay = itInfo->second->get_distance();
                maxDelay = std::max(delay, maxDelay);

            }

        }

    }
    else {

        std::map<TransportAddress, NicePeerInfo*>::iterator itInfo = peerInfos.find(member);

        if (itInfo != peerInfos.end()) {

            for (ConstIter it = neighborsBegin; it != neighborsEnd; ++it) {

                //EV << "getDistanceTo " << *it2 << endl;
                delay = itInfo->second->getDistanceTo(*it);
                //EV << thisNode.getIp() << " : Distance to " << it2->getIp() << " : " << delay << endl;
                maxDelay = std::max(delay, maxDelay);

            }

        }

    }

    return maxDelay;

} // getMaxDistance

simtime_t Nice::getMaxDistance(TransportAddress member, const std::set<TransportAddress>& neighbors)
{
    return getMaxDistance(member, neighbors.begin(), neighbors.end());
} // getMaxDistance

/******************************************************************************
 * getMeanDistance
 */
simtime_t Nice::getMeanDistance(std::set<TransportAddress> neighbors)
{
    simtime_t meanDelay = 0;
    simtime_t delay = 0;
    unsigned int number = 0;

    std::set<TransportAddress>::iterator it = neighbors.begin();

    while (it != neighbors.end()) {

        if (*it != thisNode) {

            std::map<TransportAddress, NicePeerInfo*>::iterator it2 = peerInfos.find(*it);

            if (it2 != peerInfos.end()) {

                delay = it2->second->get_distance();
                //EV << "delay to " << *it << " : " << delay << endl;

                if  (delay > 0.0) {

                    meanDelay += delay;
                    number++;

                }

            }

        }

        it++;

    }

    if (number > 0) {

        return meanDelay/number;

    }
    else {

        return 0;

    }

} // getMeanDistance


/******************************************************************************
 * LeaderTransfer
 * Does not actually change stored cluster information.
 */
void Nice::LeaderTransfer(int layer, TransportAddress leader, TaSet cluster, TransportAddress sc_leader, TaSet superCluster)
{

    NiceLeaderHeartbeat* msg = new NiceLeaderHeartbeat("NICE_LEADERTRANSFER");
    msg->setSrcNode(thisNode);
    msg->setCommand(NICE_LEADERTRANSFER);
    msg->setLayer(layer);

    msg->setMembersArraySize(cluster.size());

    // fill in members
    TaSet::iterator it = cluster.begin();
    int i = 0;
    while (it != cluster.end()) {
        msg->setMembers(i++, *it);
        it++;
    }

    // fill in supercluster members, if existent
    msg->setSupercluster_leader(sc_leader);

    msg->setSupercluster_membersArraySize(superCluster.size());

    it = superCluster.begin();
    i = 0;
    while (it != superCluster.end()) {
        msg->setSupercluster_members(i++, *it);
        ++it;
    }

    msg->setBitLength(NICELEADERHEARTBEAT_L(msg));

    sendMessageToUDP(leader, msg);

} // LeaderTransfer

void Nice::LeaderTransfer(int layer, TransportAddress leader)
{
    ASSERT(clusters[layer].contains(leader));

    if (isRendevouzPoint) {
        opp_error("The RendevouzPoint is handing off leadership and the simulation cannot continue. This is a bug in the Nice implementation in OverSim, please check the backtrace and fix it or submit a bug report.");
    }

    TaSet cluster(clusters[layer].begin(), clusters[layer].end());

    if (layer == maxLayers - 1)
        LeaderTransfer(layer, leader, cluster, TransportAddress::UNSPECIFIED_NODE, TaSet());
    else
        LeaderTransfer(layer, leader, cluster, clusters[layer + 1].getLeader(), TaSet(clusters[layer + 1].begin(), clusters[layer+1].end()));
}

/******************************************************************************
 * Remove
 */
void Nice::Remove(int layer)
{
    if (debug_removes)
        EV << simTime() << " : " << thisNode.getIp() << " : Remove()" << endl;

    int highestLayer = getHighestLayer();
    ASSERT(layer <= highestLayer);

    NiceMessage* msg = new NiceMessage("NICE_REMOVE");
    msg->setSrcNode(thisNode);
    msg->setCommand(NICE_REMOVE);
    msg->setLayer(layer);

    msg->setBitLength(NICEMESSAGE_L(msg));

    sendMessageToUDP(clusters[layer].getLeader(), msg);

    clusters[layer].remove(thisNode);

    for (short i=0; i<maxLayers; i++) {

        if (clusters[i].getSize() > 0) {

            if (clusters[i].contains(thisNode)) {

                getParentModule()->getParentModule()->getDisplayString().setTagArg
                ("i2", 1, clustercolors[i]);

            }

        }

    }

    if (debug_removes)
        EV << simTime() << " : " << thisNode.getIp() << " : Remove() finished." << endl;


} // Remove


/******************************************************************************
 * gracefulLeave
 */
void Nice::gracefulLeave(short bottomLayer)
{
    EV << simTime() << " : " << thisNode.getIp() << " : gracefulLeave()" << endl;

    int layer = getHighestLayer();

    ASSERT(layer >= bottomLayer);

    if (isRendevouzPoint) {
        opp_error("The RendevouzPoint is trying to leave a layer and the simulation cannot continue. This is a bug in the Nice implementation in OverSim, please check the backtrace and fix it or submit a bug report.");
    }

    if (!clusters[layer].getLeader().isUnspecified() && clusters[layer].getLeader() != thisNode) {
        // simply leave cluster
        EV << "removing " << thisNode.getIp() << " from " << layer << endl;
        if (!clusters[layer].getLeader().isUnspecified()) {
            Remove(layer);
        }
        clusters[layer].remove(thisNode);
    }

    for (layer = getHighestLeaderLayer(); layer >= bottomLayer; layer--) {

        EV << "REPAIR: " << layer << endl;

        for (TaSet::const_iterator itNode = clusters[layer].begin(); itNode != clusters[layer].end(); ++itNode) {

            EV << "rest: " << itNode->getIp() << endl;

            deleteOverlayNeighborArrow(*itNode);

        }

        EV << "remove from: " << layer << endl;
        Remove(layer);

        if (clusters[layer].getSize() > 0) {
            TransportAddress new_sc_center = findCenter(clusters[layer]).first;

            EV << "NEW LEADER (GL): " << layer << " --> " << new_sc_center.getIp() << endl;

            if (new_sc_center.isUnspecified()) {

                new_sc_center = clusters[layer].get(0);

                EV << "UNSPECIFIED! instead choose: " << new_sc_center.getIp() << endl;

            }

            clusters[layer].setLeader(new_sc_center);

            LeaderTransfer(layer, new_sc_center);

        }

    }

    EV << simTime() << " : " << thisNode.getIp() << " : gracefulLeave() finished." << endl;


} // gracefulLeave


/******************************************************************************
 * handleAppMessage
 */
void Nice::handleAppMessage(cMessage* msg)
{
    if ( ALMAnycastMessage* anycastMsg = dynamic_cast<ALMAnycastMessage*>(msg) ) {
        EV << "[Nice::handleAppMessage() @ " << overlay->getThisNode().getIp()
           << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
           << "    Anycast message for group " << anycastMsg->getGroupId() << "\n"
           << "    ignored: Not implemented yet!"
           << endl;
    }
    else if ( ALMCreateMessage* createMsg = dynamic_cast<ALMCreateMessage*>(msg) ) {
        EV << "[Nice::handleAppMessage() @ " << overlay->getThisNode().getIp()
           << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
           << "    Create message for group " << createMsg->getGroupId() << "\n"
           << "    ignored: Not implemented yet!"
           << endl;
    }
    else if ( ALMDeleteMessage* deleteMsg = dynamic_cast<ALMDeleteMessage*>(msg) ) {
        EV << "[Nice::handleAppMessage() @ " << overlay->getThisNode().getIp()
           << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
           << "    Delete message for group " << deleteMsg->getGroupId() << "\n"
           << "    ignored: Not implemented yet!"
           << endl;
    }
    else if ( ALMLeaveMessage* leaveMsg = dynamic_cast<ALMLeaveMessage*>(msg) ) {
        EV << "[Nice::handleAppMessage() @ " << overlay->getThisNode().getIp()
           << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
           << "    Leave message for group " << leaveMsg->getGroupId() << "\n"
           << "    ignored: Not implemented yet!"
           << endl;
    }
    else if ( ALMMulticastMessage* multicastMsg = dynamic_cast<ALMMulticastMessage*>(msg) ) {
        NiceMulticastMessage *niceMsg = new NiceMulticastMessage("NICE_MULTICAST");
        niceMsg->setCommand(NICE_MULTICAST);
        niceMsg->setLayer(-1);
        niceMsg->setSrcNode(thisNode);
        niceMsg->setLastHop(thisNode);
        niceMsg->setHopCount(0);

        niceMsg->setBitLength(NICEMULTICAST_L(niceMsg));

        niceMsg->encapsulate(multicastMsg);
        sendDataToOverlay(niceMsg);

        // otherwise msg gets deleted later
        msg = NULL;
    }
    else if ( ALMSubscribeMessage* subscribeMsg = dynamic_cast<ALMSubscribeMessage*>(msg) ) {
        EV << "[Nice::handleAppMessage() @ " << overlay->getThisNode().getIp()
           << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
           << "    Subscribe message for group " << subscribeMsg->getGroupId() << "\n"
           << "    ignored: Not implemented yet!"
           << endl;
    }

    // Delete msg if not already deleted
    if ( msg ) {
        delete msg;
    }

} // handleAppMessage


/******************************************************************************
 * sendDataToOverlay
 */
void Nice::sendDataToOverlay(NiceMulticastMessage *appMsg)
{

    for (int layer=0; layer <= getHighestLayer(); layer++) {

        if ( appMsg->getLayer() != layer ) {

            for (int j=0; j<clusters[layer].getSize(); j++) {

                if (!(clusters[layer].contains(appMsg->getLastHop())) || appMsg->getSrcNode() == thisNode) {

                    const TransportAddress& member = clusters[layer].get(j);

                    if (!(member == thisNode)) {

                        NiceMulticastMessage* dup = static_cast<NiceMulticastMessage*>(appMsg->dup());

                        dup->setLayer( layer );
                        dup->setLastHop(thisNode);

                        sendMessageToUDP(member, dup);

                    }

                }

            } // for

        }

    }

    // Also forward data to temporary peers
    std::map<TransportAddress, simtime_t>::iterator it = tempPeers.begin();

    while (it != tempPeers.end()) {

        NiceMulticastMessage* dup = static_cast<NiceMulticastMessage*>(appMsg->dup());

        dup->setSrcNode(thisNode);

        sendMessageToUDP(it->first, dup);

        it++;

    }

    delete appMsg;

} // sendDataToOverlay


/******************************************************************************
 * updateVisualization
 */
void Nice::updateVisualization()
{

    if (debug_visualization)
        EV << simTime() << " : " << thisNode.getIp() << " : updateVisualization" << endl;

    /* Update node symbol */
    getParentModule()->getParentModule()
    ->getDisplayString().setTagArg("i2", 0, "block/circle_vs");

    if (isRendevouzPoint) {

        getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 0, "block/star_vs");

    }

    /* Update node color */
    if (debug_visualization)
        EV << "getHighestLayer(): " << getHighestLayer() << endl;

    getParentModule()->getParentModule()
    ->getDisplayString().setTagArg("i2", 1, clustercolors[getHighestLayer()]);

    //redraw
    for (int i=0; clusters[i].contains(thisNode); i++) {

        if (!(clusters[i].getLeader().isUnspecified())) {

            if (clusters[i].getLeader() == thisNode) {

                for (int j=0; j<clusters[i].getSize();j++) {

                    if (debug_visualization)
                        EV << "draw to: " << clusters[i].get(j) << endl;

                    showOverlayNeighborArrow(clusters[i].get(j), false, clusterarrows[i]);

                }

            }

        }
    }


} // updateVisualization

void Nice::pollRP(int layer)
{

    if (debug_queries)
        EV << simTime() << " : " << thisNode.getIp() << " : pollRP()" << endl;

    NiceMessage* msg = new NiceMessage("NICE_POLL_RP");
    msg->setSrcNode(thisNode);
    msg->setCommand(NICE_POLL_RP);
    msg->setLayer(layer);
    msg->setBitLength(NICEMESSAGE_L(msg));

    cancelEvent(rpPollTimer);
    scheduleAt(simTime() + rpPollTimerInterval, rpPollTimer);

    sendMessageToUDP(RendevouzPoint, msg);

    polledRendevouzPoint = RendevouzPoint;

    if (debug_queries)
        EV << simTime() << " : " << thisNode.getIp() << " : pollRP() finished." << endl;

} // pollRP



}; //namespace
