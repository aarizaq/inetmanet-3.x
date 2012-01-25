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
 * @file GiaTokenFactory.h
 * @author Robert Palmer
 */

#ifndef __GIATOKENFACTORY_H_
#define __GIATOKENFACTORY_H_


#include <vector>
#include <queue>

#include <omnetpp.h>

#include <InitStages.h>

class Gia;
#include "GiaNeighbors.h"


/**
 * This class handles the token allocation.
 * It grants the next token to the node which has fewest tokens.
 * If some nodes have the same amount of granted tokens, the node with the highest capacity
 * will obtain the token.
 */
class GiaTokenFactory : public cSimpleModule
{
  public:
    // OMNeT++ methodes
    /**
     * Sets init stage
     */
    virtual int numInitStages() const
    {
        return MAX_STAGE_OVERLAY + 1;
    }

    /**
     * Initializes this class and set some WATCH(variable) for OMNeT++
     * @param stage Level of initialization (OMNeT++)
     */
    virtual void initialize( int stage );

    /**
     * This module doesn't handle OMNeT++ messages
     * @param msg OMNeT++ message
     */
    virtual void handleMessages( cMessage* msg );

    // class methodes
    /**
     * Set neighbors
     * @param neighbors pointer to our neighborlist
     */
    virtual void setNeighbors( GiaNeighbors* neighbors );

    /**
     * Set maximum hop count
     * @param maxHopCount
     */
    void setMaxHopCount( uint32_t maxHopCount);

    /**
     * Sends a token to a GiaNode
     */
    virtual void grantToken();

  protected:

    Gia* gia;

    // sort rules for priority queue
class tokenCompareGiaNode : public std::binary_function<FullGiaNodeInfo, FullGiaNodeInfo, FullGiaNodeInfo>
    {
    public:
        bool operator()(const FullGiaNodeInfo& x, const FullGiaNodeInfo& y);
    };

    typedef std::priority_queue<FullGiaNodeInfo, std::vector<FullGiaNodeInfo>, tokenCompareGiaNode> TokenQueue;
    TokenQueue tokenQueue; /**< prioriry queue of all current neighbors */
    GiaNeighbors* neighbors; /**< pointer to our current neighbors */
    std::vector<GiaNode> tokenQueueVector; /** a vector of the priority queue (to visualize current priority state)*/
    uint32_t maxHopCount; /**< maximum hop count */

    // statistics
    uint32_t stat_sentTokens; /**< number of sent tokens */

    /**
     * Creates priority queue
     */
    void createPriorityQueue();

    /**
     * Clears tokenQueue
     */
    void clearTokenQueue();

    /**
     * Update TokenQueue-Vector (for OMNeT++ WATCH)
     */
    void updateQueueVector();

    /**
     * Increase sentTokens at neighbor-node which is on top of priority queue
     */
    void updateSentTokens();

    /**
     * Sends token to node on top of priority queue
     */
    void sendToken();
};

#endif
