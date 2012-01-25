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
 * @file Chord.h
 * @author Markus Mauch, Ingmar Baumgart
 */

#ifndef __CHORD_H_
#define __CHORD_H_

#include <BaseOverlay.h>
#include <NeighborCache.h>

#include "ChordMessage_m.h"

namespace oversim {

class ChordSuccessorList;
class ChordFingerTable;

/**
 * Chord overlay module
 *
 * Implementation of the Chord KBR overlay as described in
 * "Chord: A Scalable Peer-to-Peer Lookup Protocol for Inetnet
 * Applications" by I. Stoica et al. published in Transactions on Networking.
 *
 * @author Markus Mauch, Ingmar Baumgart
 * @see BaseOverlay, ChordFingerTable, ChordSuccessorList
 */
class Chord : public BaseOverlay, public ProxListener
{
public:
    Chord();
    virtual ~Chord();

    // see BaseOverlay.h
    virtual void initializeOverlay(int stage);

    // see BaseOverlay.h
    virtual void handleTimerEvent(cMessage* msg);

    // see BaseOverlay.h
    virtual void handleUDPMessage(BaseOverlayMessage* msg);

    // see BaseOverlay.h
    virtual void recordOverlaySentStats(BaseOverlayMessage* msg);

    // see BaseOverlay.h
    virtual void finishOverlay();

    // see BaseOverlay.h
    OverlayKey distance(const OverlayKey& x,
                        const OverlayKey& y,
                        bool useAlternative = false) const;

    /**
     * updates information shown in tk-environment
     */
    virtual void updateTooltip();

    void proxCallback(const TransportAddress &node, int rpcId,
                      cPolymorphic *contextPointer, Prox prox);

protected:
    int joinRetry; /**< */
    int stabilizeRetry; /**< // retries before neighbor considered failed */
    double joinDelay; /**< */
    double stabilizeDelay; /**< stabilize interval (secs) */
    double fixfingersDelay; /**< */
    double checkPredecessorDelay;
    int successorListSize; /**< */
    bool aggressiveJoinMode; /**< use modified (faster) JOIN protocol */
    bool extendedFingerTable;
    unsigned int numFingerCandidates;
    bool proximityRouting;
    bool memorizeFailedSuccessor;
    bool newChordFingerTable;
    bool mergeOptimizationL1;
    bool mergeOptimizationL2;
    bool mergeOptimizationL3;
    bool mergeOptimizationL4;


    // timer messages
    cMessage* join_timer; /**< */
    cMessage* stabilize_timer; /**< */
    cMessage* fixfingers_timer; /**< */
    cMessage* checkPredecessor_timer;

    // statistics
    int joinCount; /**< */
    int stabilizeCount; /**< */
    int fixfingersCount; /**< */
    int notifyCount; /**< */
    int newsuccessorhintCount; /**< */
    int joinBytesSent; /**< */
    int stabilizeBytesSent; /**< */
    int notifyBytesSent; /**< */
    int fixfingersBytesSent; /**< */
    int newsuccessorhintBytesSent; /**< */

    int keyLength; /**< length of an overlay key in bits */
    int missingPredecessorStabRequests; /**< missing StabilizeCall msgs */

    /**
     * changes node state
     *
     * @param toState state to change to
     */
    virtual void changeState(int toState);

    // node references
    NodeHandle predecessorNode; /**< predecessor of this node */
    TransportAddress bootstrapNode; /**< node used to bootstrap */

    // module references
    ChordFingerTable* fingerTable; /**< pointer to this node's finger table */
    ChordSuccessorList* successorList; /**< pointer to this node's successor list */

    // chord routines

    /**
     * handle a expired join timer
     *
     * @param msg the timer self-message
     */
    virtual void handleJoinTimerExpired(cMessage* msg);

    /**
     * handle a expired stabilize timer
     *
     * @param msg the timer self-message
     */
    virtual void handleStabilizeTimerExpired(cMessage* msg);

    /**
     * handle a expired fix_fingers timer
     *
     * @param msg the timer self-message
     */
    virtual void handleFixFingersTimerExpired(cMessage* msg);

    /**
     * handle a received NEWSUCCESSORHINT message
     *
     * @param chordMsg the message to process
     */
    virtual void handleNewSuccessorHint(ChordMessage* chordMsg);

    /**
     * looks up the finger table and returns the closest preceeding node.
     *
     * @param key key to find the closest preceeding node for
     * @return node vector of the closest preceeding nodes to key
     */
    virtual NodeVector* closestPreceedingNode(const OverlayKey& key);


    /**
     * Assigns the finger table and successor list module to our reference
     */
    virtual void findFriendModules();

    /**
     * initializes finger table and successor list
     */
    virtual void initializeFriendModules();

    // see BaseOverlay.h
    virtual bool handleRpcCall(BaseCallMessage* msg);

    // see BaseOverlay.h
    NodeVector* findNode(const OverlayKey& key,
                         int numRedundantNodes,
                         int numSiblings,
                         BaseOverlayMessage* msg);

    // see BaseOverlay.h
    virtual void joinOverlay();

    // see BaseOverlay.h
    virtual void joinForeignPartition(const NodeHandle &node);

    // see BaseOverlay.h
    virtual bool isSiblingFor(const NodeHandle& node,
                              const OverlayKey& key,
                              int numSiblings, bool* err);

    // see BaseOverlay.h
    int getMaxNumSiblings();

    // see BaseOverlay.h
    int getMaxNumRedundantNodes();

    /**
     * Fixfingers Remote-Procedure-Call
     *
     * @param call RPC Parameter Message
     */
    void rpcFixfingers(FixfingersCall* call);

    /**
     * Join Remote-Procedure-Call
     *
     * @param call RPC Parameter Message
     */
    virtual void rpcJoin(JoinCall* call);

    /**
     * NOTIFY Remote-Procedure-Call
     *
     * @param call RPC Parameter Message
     */
    virtual void rpcNotify(NotifyCall* call);

    /**
     * STABILIZE Remote-Procedure-Call
     *
     * @param call RPC Parameter Message
     */
    void rpcStabilize(StabilizeCall* call);

    // see BaseOverlay.h
    virtual void handleRpcResponse(BaseResponseMessage* msg,
                                   cPolymorphic* context, int rpcId,
                                   simtime_t rtt);

    // see BaseOverlay.h
    virtual void handleRpcTimeout(BaseCallMessage* msg,
                                  const TransportAddress& dest,
                                  cPolymorphic* context,
                                  int rpcId, const OverlayKey& destKey);

    // see BaseRpc.h
    virtual void pingResponse(PingResponse* pingResponse,
                              cPolymorphic* context, int rpcId,
                              simtime_t rtt);

    // see BaseRpc.h
    virtual void pingTimeout(PingCall* pingCall,
                             const TransportAddress& dest,
                             cPolymorphic* context,
                             int rpcId);

    virtual void handleRpcJoinResponse(JoinResponse* joinResponse);
    virtual void handleRpcNotifyResponse(NotifyResponse* notifyResponse);
    virtual void handleRpcStabilizeResponse(StabilizeResponse* stabilizeResponse);
    virtual void handleRpcFixfingersResponse(FixfingersResponse* fixfingersResponse,
                                             double rtt = -1);

    virtual bool handleFailedNode(const TransportAddress& failed);

    friend class ChordSuccessorList;
    friend class ChordFingerTable;

private:
    TransportAddress failedSuccessor;
};

}; //namespace

#endif
