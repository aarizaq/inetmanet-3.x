#ifndef __PASTRYLEAFSET_H
#define __PASTRYLEAFSET_H

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
 * @file PastryLeafSet.h
 * @author Felix Palmen
 */

#include <vector>

#include <omnetpp.h>

#include <NodeHandle.h>
#include <NodeVector.h>

#include "PastryStateObject.h"
#include "PastryTypes.h"
#include "PastryMessage_m.h"

#include "BasePastry.h"

class BasePastry;

/**
 * struct for tracking repair requests
 */
struct PLSRepairData
{
    simtime_t ts;
    bool left;
    PLSRepairData(simtime_t ts = 0, bool left = true) : ts(ts), left(left) {};
};

/**
 * PastryLeafSet module
 *
 * This module contains the LeafSet of the Pastry implementation.
 *
 * @author Felix Palmen
 * @see Pastry
 */
class PastryLeafSet : public PastryStateObject
{
  public:

    /**
     * Initializes the leaf set. This should be called on startup
     *
     * @param numberOfLeaves Pastry configuration parameter
     * @param bitsPerDigit number of bits per digits
     * @param repairTimeout Pastry configuration parameter
     * @param owner the node this table belongs to
     * @param overlay pointer to the pastry main module
     */
    void initializeSet(uint32_t numberOfLeaves,
                       uint32_t bitsPerDigit,
                       simtime_t repairTimeout,
                       const NodeHandle& owner,
                       BasePastry* overlay);

    /**
     * gets the final node according to the Pastry routing scheme.
     *
     * @param destination the destination key
     * @return the NodeHandle of the final node or NodeHandle::UNSPECIFIED_NODE
     *	  if given destination key is outside the leaf set
     */
    virtual const NodeHandle& getDestinationNode(const OverlayKey& destination);

    /**
     * try to find a node numerically closer to a given key with the same
     * shared prefix as the current node in the leaf set. this method
     * is to be called, when a regular next hop couldn't be found or wasn't
     * reachable.
     *
     * @param destination the destination key
     * @param optimize if set, check all nodes and return the best/closest one
     * @return a closer NodeHandle or NodeHandle::UNSPECIFIED_NODE if none was
     *	  found
     */
    virtual const NodeHandle& findCloserNode(const OverlayKey& destination,
                                             bool optimize = false);

    void findCloserNodes(const OverlayKey& destination,
                         NodeVector* nodes);
    /**
     * tells the leafset that a node has failed
     *
     * @param failed the failed node
     * @return a node to ask for REPAIR or TransportAddress::UNSPECIFIED_NODE
     */
    virtual const TransportAddress& failedNode(const TransportAddress& failed);

    /**
     * attempt to repair the leafset using a received REPAIR message
     *
     * @param msg the state message of type REPAIR
     * @param prox record of proximity values matching the state message
     * @return another node to ask for REPAIR or
     *    TransportAddress::UNSPECIFIED_NODE
     */
    virtual const TransportAddress& repair(const PastryStateMessage* msg,
                                           const PastryStateMsgProximity* prox);

    /**
     * checks if we are the closest node to key destination in the overlay
     *
     * @param destination the key to check
     * @return true if we are closest to given key
     */
    bool isClosestNode(const OverlayKey& destination) const;

    /**
     * dump content of the set to a PastryStateMessage
     *
     * @param msg the PastryStateMessage to be filled with entries
     */
    virtual void dumpToStateMessage(PastryStateMessage* msg) const;

     /**
     * dump content of the set to a PastryLeafsetMessage
     *
     * @param msg the PastryLeafsetMessage to be filled with entries
     */
    virtual void dumpToStateMessage(PastryLeafsetMessage* msg) const;

    /**
     * returns a random node from the leafset
     *
     * @return a random node or TransportAddress::UNSPECIFIED_NODE
     */
    virtual const TransportAddress& getRandomNode();

    /**
     * merge a node into LeafSet
     *
     * @param node the node to merge
     * @param prox the proximity value of the node
     * @return true if node was merged
     */
    bool mergeNode(const NodeHandle& node, simtime_t prox);

    /**
     * return predecessor node for visualizing
     */
    const NodeHandle& getPredecessor(void) const;

    /**
     * return successor node for visualizing
     */
    const NodeHandle& getSuccessor(void) const;

    /**
     * check if LeafSet knows at least one node to the left and to the right
     */
    bool isValid(void) const;

    /**
     * appends all leaf set entries to a given vector of TransportAddresses,
     * needed to find all Nodes to be notified after joining.
     *
     * @param affected the vector to fill with leaf set entries
     */
    virtual void dumpToVector(std::vector<TransportAddress>& affected) const;

    NodeVector* createSiblingVector(const OverlayKey& key, int numSiblings) const;


    /**
     * generates a newLeafs-message if LeafSet changed since last call
     * to this method.
     *
     * @return pointer to newLeafs-message or NULL
     */
    PastryNewLeafsMessage* getNewLeafsMessage(void);

  private:
    uint32_t numberOfLeaves;
    simtime_t repairTimeout;
    BasePastry* overlay; /**< pointer to the main pastry module */
    std::vector<NodeHandle> leaves;
    std::vector<NodeHandle>::iterator smaller;
    std::vector<NodeHandle>::iterator bigger;

    std::map<TransportAddress, PLSRepairData> awaitingRepair;

    bool newLeafs;

    bool isFull;
    bool wasFull;

    virtual void earlyInit(void);

    /**
     * return the node with the biggest key in the LeafSet or
     * NodeHandle::UNSPECIFIED_NODE if LeafSet is empty
     *
     * @return biggest node in the set
     */
    const NodeHandle& getBiggestNode(void) const;

    /**
     * return the biggest key in the LeafSet or OverlayKey::UNSPECIFIED_KEY
     * if LeafSet is empty
     *
     * @return biggest key in the set
     */
    const OverlayKey& getBiggestKey(void) const;

    /**
     * return the node with the smallest key in the LeafSet or
     * NodeHandle::UNSPECIFIED_NODE if LeafSet is empty
     *
     * @return smallest node in the set
     */
    const NodeHandle& getSmallestNode(void) const;

    /**
     * return the smallest key in the LeafSet or OverlayKey::UNSPECIFIED_KEY
     * if LeafSet is empty
     *
     * @return smallest key in the set
     */
    const OverlayKey& getSmallestKey(void) const;

    /**
     * test if a given key should be placed on the left or on the right side
     * of the leaf set
     *
     * @param key key to test
     * @return true if key belongs to the left
     */
    bool isLeft(const OverlayKey& key) const;

    /**
     * insert a leaf at a given position
     *
     * @param it iterator where to insert the new leaf
     * @param node NodeHandle of new leaf
     */
    void insertLeaf(std::vector<NodeHandle>::iterator& it,
	    const NodeHandle& node);

    bool balanceLeafSet();
};

#endif
