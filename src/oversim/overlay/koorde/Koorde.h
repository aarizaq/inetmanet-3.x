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
 * @file Koorde.h
 * @author Jochen Schenk, Ingmar Baumgart
 */

#ifndef __KOORDE_H_
#define __KOORDE_H_

#include <omnetpp.h>

#include <IPvXAddress.h>

#include <OverlayKey.h>
#include <NodeHandle.h>
#include <BaseOverlay.h>

#include "../chord/ChordSuccessorList.h"
#include "../chord/Chord.h"

namespace oversim {

/**
 * Koorde overlay module
 *
 * Implementation of the Koorde KBR overlay as described in
 * "Koorde: A simple degree-optimal distributed hash table" by
 * M. Kaashoek and D. Karger
 *
 * @author Jochen Schenk
 * @see Chord, ChordSuccessorList
 */
class Koorde : public Chord
{
  public:
    virtual ~Koorde();

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

    /**
     * updates information shown in tk-environment
     */
    virtual void updateTooltip ();

  protected:
    //parameters
    int deBruijnDelay; /**< number of seconds between two de bruijn calls */
    int deBruijnNumber; /**< number of current nodes in de bruijn list; depend on number of nodes in successor list */
    int deBruijnListSize; /**< maximal number of nodes in de bruijn list */
    int shiftingBits; /**< number of bits concurrently shifted in one routing step */
    bool useOtherLookup; /**< flag which is indicating that the optimization other lookup is enabled */
    bool useSucList; /**< flag which is indicating that the optimization using the successorlist is enabled */
    bool breakLookup; /**< flag is used during the recursive step when returning this node */
    bool setupDeBruijnBeforeJoin; /**< if true, first setup the de bruijn node using the bootstrap node and than join the ring */
    bool setupDeBruijnAtJoin; /**< if true, join the ring and setup the de bruijn node using the bootstrap node in parallel */

    //statistics
    int deBruijnCount; /**< number of de bruijn calls */
    int deBruijnBytesSent; /**< number of bytes sent during de bruijn calls*/

    //Node handles
    NodeHandle* deBruijnNodes; /**< List of de Bruijn nodes */
    NodeHandle deBruijnNode; /**< Handle to our de Bruijn node */

    //Timer Messages
    cMessage* deBruijn_timer; /**< timer for periodic de bruijn stabilization */

    /**
     * changes node state
     *
     * @param state state to change to
     */
    virtual void changeState(int state);

    /**
     * handle an expired de bruijn timer
     *
     */
    virtual void handleDeBruijnTimerExpired();

    /**
     * handle an expired fix_fingers timer (dummy function)
     *
     * @param msg the timer self-message
     */
    //virtual void handleFixFingersTimerExpired(cMessage* msg);

    // see BaseOverlay.h
    virtual bool handleRpcCall(BaseCallMessage* msg);

    // see BaseOverlay.h
    virtual void handleRpcResponse(BaseResponseMessage* msg,
                                   cPolymorphic* context, int rpcId,
                                   simtime_t rtt );

    // see BaseOverlay.h
    virtual void handleRpcTimeout(BaseCallMessage* msg,
                                  const TransportAddress& dest,
                                  cPolymorphic* context,
                                  int rpcId, const OverlayKey& destKey);

    /**
     * handle a received JOIN response
     *
     * @param joinResponse the message to process
     */
    virtual void handleRpcJoinResponse(JoinResponse* joinResponse);

    /**
     * handle a received DEBRUIJN request
     *
     * @param deBruinCall the message to process
     */
    virtual void handleRpcDeBruijnRequest(DeBruijnCall* deBruinCall);

    /**
     * handle a received DEBRUIJN response
     *
     * @param deBruijnResponse the message to process
     */
    virtual void handleRpcDeBruijnResponse(DeBruijnResponse* deBruijnResponse);

    /**
     * handle a DEBRUIJN timeout
     *
     * @param deBruijnCall the message which timed out
     */
    virtual void handleDeBruijnTimeout(DeBruijnCall* deBruijnCall);

    /**
     * returns the NodeHandle of the next hop to destination key
     * using the de Bruijn list
     *
     * @param destKey The destination key
     * @param findNodeExt The FindNodeExtensionMessage
     * @return next hop to destination key
     */
    virtual NodeHandle findDeBruijnHop(const OverlayKey& destKey,
                                       KoordeFindNodeExtMessage* findNodeExt);

    // see BaseOverlay.h
    NodeVector* findNode(const OverlayKey& key,
                         int numRedundantNodes,
                         int numSiblings,
                         BaseOverlayMessage* msg);
    /**
     * find a "good" routing key to destKey between startingKey and
     * endKey with the longest matching prefix possible
     *
     * @param startKey begin of the (key) interval
     * @param endKey end of the (key) interval
     * @param destKey destination key - should be matched as good as possible
     * @param step reference to return the bit position
     * @return a "good" routing key to start from
     */
    virtual OverlayKey findStartKey(const OverlayKey& startKey,
                                    const OverlayKey& endKey,
                                    const OverlayKey& destKey,
                                    int& step);

    /**
     * Given a key the function checks if the key falls between two
     * nodes in the de Bruijn list. If no match is found the last node
     * in the de Bruijn list is returned.
     *
     * @param key the key to check
     * @return either the node directly preceding key or the node
     * which has the shortest distance to the key
     */
    virtual const NodeHandle& walkDeBruijnList(const OverlayKey& key);

    /**
     * Given a key the function checks if the key falls between two
     * nodes in the de successor list. If no match is found the last
     * node in the de successor list is returned.
     *
     * @param key the key to check
     * @return either the node directly preceding key or the node which has the
     * shortest distance to the key
     */
    virtual const NodeHandle& walkSuccessorList(const OverlayKey& key);

    // see BaseOverlay.h
    virtual bool handleFailedNode(const TransportAddress& failed);

    // see Chord.h
    virtual void rpcJoin(JoinCall* call);

    // see Chord.h
    virtual void findFriendModules();

    // see Chord.h
    virtual void initializeFriendModules();

};

}; //namespace

#endif

