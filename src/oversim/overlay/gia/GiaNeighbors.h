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
 * @file GiaNeighbors.h
 * @author Robert Palmer, Bernhard Heep
 */

#ifndef __GIANEIGHBORS_H_
#define __GIANEIGHBORS_H_

#include <vector>

#include <omnetpp.h>

#include <OverlayKey.h>
#include <InitStages.h>

#include "GiaNode.h"
#include "GiaKeyList.h"


struct GiaNeighborInfo
{
    unsigned int connectionDegree;
    unsigned int receivedTokens;
    unsigned int sentTokens;
    simtime_t timestamp;
    GiaKeyList keyList;
};

struct FullGiaNodeInfo
{
    GiaNode node;
    GiaNeighborInfo* info;
};


std::ostream& operator<<(std::ostream& os, const GiaNeighborInfo& info);

/**
 * This class is for managing all neighbor nodes
 */
class GiaNeighbors : public cSimpleModule
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
     * @return Number of neighbors
     */
    virtual unsigned int getSize() const;

    /**
     * @param node GiaNode to check
     * @return true if node is our neighbor
     */
    virtual bool contains(const GiaNode& node) const;
    //virtual bool contains(const NodeHandle& node) const;

    /**
     * @param key to check
     * @return true if node with corresponding key is our neighbor
     */
    virtual bool contains(const OverlayKey& key) const;

    /**
     * Adds a new neighbor to our neighbor list
     * @param node New neighbor to add
     * @param degree The new neighbor's connection degree
     */
    virtual void add(const GiaNode& node, unsigned int degree);
    //virtual void add(const NodeHandle& node);

    /**
     * Removes neighbor from our neighbor list
     * @param node Node to remove to
     */
    virtual void remove(const GiaNode& node);

    /**
     * Get neighbor at position
     * @param position
     * @return GiaNode
     */
    virtual const GiaNode& get(unsigned int position);

    /**
     * Get node from neighborlist
     * @param key the node's key
     * @return the node
     */
    virtual const GiaNode& get(const OverlayKey& key);
    
    //bullshit
    GiaNeighborInfo* get(const GiaNode& node);

    /**
     * Update timestamp
     */
    void updateTimestamp(const GiaNode& node);

    /**
     * Removes timedout nodes
     */
    void removeTimedoutNodes();

    /**
     * Sets the keyList of neighbor at position pos
     * @param node the node the keylist belongs to
     * @param keyList KeyList to set
     */
    void setNeighborKeyList(const GiaNode& node, const GiaKeyList& keyList);
    GiaKeyList* getNeighborKeyList(const GiaNode& node);

    double getCapacity(const GiaNode& node) const;
    //void setCapacity(const GiaNode& node, double capacity);

    void setConnectionDegree(const GiaNode& node, unsigned int degree);
    unsigned int getConnectionDegree(const GiaNode& node) const;

    void setReceivedTokens(const GiaNode& node, unsigned int tokens);
    void increaseReceivedTokens(const GiaNode& node);
    void decreaseReceivedTokens(const GiaNode& node);
    unsigned int getReceivedTokens(const GiaNode& node) const;

    void setSentTokens(const GiaNode& node, unsigned int tokens);
    void increaseSentTokens(const GiaNode& node);
    unsigned int getSentTokens(const GiaNode& node) const;

    const GiaNode& getDropCandidate(double capacity,
				       unsigned int degree) const;

  protected:
    std::map<GiaNode, GiaNeighborInfo> neighbors; /**< contains all current neighbors */
    typedef std::map<GiaNode, GiaNeighborInfo>::iterator NeighborsIterator;
    typedef std::map<GiaNode, GiaNeighborInfo>::const_iterator NeighborsConstIterator;
    GiaNode thisNode; /** this node */
    simtime_t timeout; /** timeout for neighbors */
};

#endif
