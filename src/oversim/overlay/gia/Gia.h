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
 * @file Gia.h
 * @author Robert Palmer
 */

#ifndef __GIA_H_
#define __GIA_H_


#include <vector>

#include <omnetpp.h>

#include <IPvXAddress.h>

#include <GlobalNodeList.h>
#include <UnderlayConfigurator.h>
#include <OverlayKey.h>
#include <NodeHandle.h>
#include <CommonMessages_m.h>
#include <BaseOverlay.h>

#include "GiaMessage_m.h"
#include "GiaKeyListModule.h"
#include "GiaKeyList.h"
#include "GiaNeighbors.h"
#include "GiaTokenFactory.h"
#include "GiaNode.h"
#include "GiaNeighborCandidateList.h"
#include "GiaMessageBookkeeping.h"


/**
 * Gia overlay module
 *
 * Implementation of the Gia overlay as described in
 * "Making Gnutella-like P2P Systems Scalable"
 * by Y. Chawathe et al. published at SIGCOMM'03
 *
 * @author Robert Palmer
 */
class Gia : public BaseOverlay
{
  public:
    /**
     * initializes base class-attributes
     *
     * @param stage the init stage
     */
    void initializeOverlay(int stage);

    /**
     * Writes statistical data and removes node from bootstrap oracle
     */
    void finishOverlay();

    /**
     * Set state to toStage
     * @param toStage the stage to change to
     */
    virtual void changeState(int toStage);

    /**
     * Marks nodes if they are ready
     */
    void updateTooltip();
    //virtual void setBootstrapedIcon();

    // Gia
    /**
     * Destructor
     */
    ~Gia();

    void handleTimerEvent(cMessage* msg);

    void handleUDPMessage(BaseOverlayMessage* msg);

    // API for structured P2P overlays

    virtual void getRoute(const OverlayKey& key, CompType destComp,
                       CompType srcComp, cPacket* msg,
                       const std::vector<TransportAddress>& sourceRoute
                           = TransportAddress::UNSPECIFIED_NODES,
                       RoutingType routingType = DEFAULT_ROUTING);

    void handleAppMessage(cMessage* msg);

    void sendToken(const GiaNode& dst);

  protected:
    // parameters from OMNeT.ini
    uint32_t maxNeighbors; /**< maximum number of neighbors */
    uint32_t minNeighbors; /**< minimum number of neighbors */
    uint32_t maxTopAdaptionInterval; /**< maximum topology adaption interval */
    uint32_t topAdaptionAggressiveness; /**< the topology adaption aggressiveness */
    double maxLevelOfSatisfaction; /**< maximum level of satisfaction */
    double updateDelay; /**< time between to update messages (in ms) */
    uint32_t maxHopCount; /**< maximum time to live for sent messages */
    uint32_t messageTimeout; /**< timeout for messages */
    uint32_t neighborTimeout; /**< timeout for neighbors */
    uint32_t sendTokenTimeout; /**< timeout for tokens */
    uint32_t tokenWaitTime; /**< delay to send a new token */
    double keyListDelay; /**< delay to send the keylist to our neighbors */
    bool outputNodeDetails; /**< output of node details? (on std::cout)*/
    bool optimizeReversePath; /**< use optimized reverse path? */

    double levelOfSatisfaction; /**< current level of statisfaction */
    unsigned int connectionDegree;
    unsigned int receivedTokens;
    unsigned int sentTokens;

    // node references
    GiaNode thisGiaNode; /**< this node */
    NodeHandle bootstrapNode; /**< next possible neighbor candidate */
    GiaMessageBookkeeping* msgBookkeepingList; /**< pointer to a message bookkeeping list */

    // statistics
    uint32_t stat_joinCount; /**< number of sent join messages */
    uint32_t stat_joinBytesSent; /**< number of sent bytes of join messages */
    uint32_t stat_joinREQ; /**< number of sent join request messages */
    uint32_t stat_joinREQBytesSent;  /**< number of sent bytes of join request messages */
    uint32_t stat_joinRSP; /**< number of sent join response messages */
    uint32_t stat_joinRSPBytesSent; /**< number of sent bytes of join response messages */
    uint32_t stat_joinACK; /**< number of sent join acknowledge messages */
    uint32_t stat_joinACKBytesSent; /**< number of sent bytes of join acknowledge messages */
    uint32_t stat_joinDNY; /**< number of sent join deny messages */
    uint32_t stat_joinDNYBytesSent; /**< number of sent bytes of join deny messages */
    uint32_t stat_disconnectMessages; /**< number of sent disconnect messages */
    uint32_t stat_disconnectMessagesBytesSent; /**< number of sent bytes of disconnect messages */
    uint32_t stat_updateMessages; /**< number of sent update messages */
    uint32_t stat_updateMessagesBytesSent; /**< number of sent bytes of update messages */
    uint32_t stat_tokenMessages; /**< number of sent token messages */
    uint32_t stat_tokenMessagesBytesSent; /**< number of sent bytes of token messages */
    uint32_t stat_keyListMessages; /**< number of sent keylist messages */
    uint32_t stat_keyListMessagesBytesSent; /**< number of sent bytes of keylist messages */
    uint32_t stat_routeMessages; /**< number of sent route messages */
    uint32_t stat_routeMessagesBytesSent; /**< number of sent bytes of route messages */
    uint32_t stat_maxNeighbors; /**< maximum number of neighbors */
    uint32_t stat_addedNeighbors; /**< number of added neighbors during life cycle of this node */
    uint32_t stat_removedNeighbors; /**< number of removed neighbors during life cycle of this node */
    uint32_t stat_numSatisfactionMessages; /**< number of satisfaction self-messages */
    double stat_sumLevelOfSatisfaction; /**< sum of level of satisfaction */
    double stat_maxLevelOfSatisfaction; /**< maximum level of satisfaction */

    // self-messages
    cMessage* satisfaction_timer; /**< timer for satisfaction self-message */
    cMessage* update_timer; /**< timer for update self-message */
    cMessage* timedoutMessages_timer; /**< timer for message timeout */
    cMessage* timedoutNeighbors_timer; /**< timer for neighbors timeout */
    cMessage* sendKeyList_timer; /**< timer for send keylist */
    cMessage* sendToken_timer; /**< timer for send token */

    // module references
    GiaKeyListModule* keyListModule; /**< pointer to KeyListModule */
    GiaNeighbors* neighbors; /**< pointer to neighbor list */
    GiaTokenFactory* tokenFactory; /**< pointer to TokenFactory */

    // internal
    GiaNeighborCandidateList neighCand; /**< list of all neighbor candidates */
    GiaNeighborCandidateList knownNodes; //!< list of known nodes in the overlay
    GiaKeyList keyList; /**< key list of this node */

    // internal methodes

    void joinOverlay();

    /**
     * Decides if Node newNode will be accepted as new neighor
     * @param newNode Node to accept or deny
     * @param degree the node's connection degree
     * @return boolean true or false
     */
    bool acceptNode(const GiaNode& newNode, unsigned int degree);

    /**
     * Adds newNode as new neighbor
     * @param newNode the node to add as a neighbor
     * @param degree the node's connection degree
     */
    void addNeighbor(GiaNode& newNode, unsigned int degree);

    /**
     * Removes newNode from our NeighborList
     * @param newNode NodeHandle of the node to remove from neighbors
     */
    void removeNeighbor(const GiaNode& newNode);

    /**
     * Calculates level of satisfaction
     * @return levelOfSatisfaction value
     */
    double calculateLevelOfSatisfaction();

    /**
     * Sends JOIN_REQ_Message from node src to node dst
     * @param dst: Destination
     */
    void sendMessage_JOIN_REQ(const NodeHandle& dst);

    /**
     * Sends JOIN_RSP_Message from node src to node dst
     * @param dst: Destination
     */
    void sendMessage_JOIN_RSP(const NodeHandle& dst);

    /**
     * Sends JOIN_ACK_Message from node src to node dst
     * @param dst: Destination
     */
    void sendMessage_JOIN_ACK(const NodeHandle& dst);

    /**
     * Sends JOIN_DNY_Message from node src to node dst
     * @param dst: Destination
     */
    void sendMessage_JOIN_DNY(const NodeHandle& dst);

    /**
     * Sends DISCONNECT_Message from node src to node dst
     * @param dst: Destination
     */
    void sendMessage_DISCONNECT(const NodeHandle& dst);

    /**
     * Sends UPDATE_Message from node src to node dst
     * @param dst: Destination
     */
    void sendMessage_UPDATE(const NodeHandle& dst);

    /**
     * Sends KeyList to node dst
     * @param dst: Destination
     */
    void sendKeyListToNeighbor(const NodeHandle& dst);

    /**
     * Updates neighborlist with new capacity and connectiondegree
     * informations from received message msg
     * @param msg Received message
     */
    void updateNeighborList(GiaMessage* msg);

    /**
     * Forwards a search response message to the next node in reverse-path
     * @param msg Message to forward to next node
     */
    void forwardSearchResponseMessage(SearchResponseMessage* msg);

    /**
     * Forwards a message to the next random selected node, biased random walk
     * @param msg Message to forward to next node
     * @param fromApplication Marks if message is from application layer
     */
    void forwardMessage(GiaIDMessage* msg, bool fromApplication);

    /**
     * Processes search message msg.
     * Generates Search_Response_Messages
     * @param msg Search message
     * @param fromApplication Marks if message is from application layer
     */
    void processSearchMessage(SearchMessage* msg, bool fromApplication);

    /**
     * Sends a response message to a received search query
     * @param srcNode Node which contains the searched key
     * @param msg SearchMessage
     */
    void sendSearchResponseMessage(const GiaNode& srcNode, SearchMessage* msg);

    /**
     * Delivers search result to application layer
     * @param msg Search response message
     */
    void deliverSearchResult(SearchResponseMessage* msg);
};

#endif
