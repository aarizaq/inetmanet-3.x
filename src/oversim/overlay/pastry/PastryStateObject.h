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


#ifndef __PASTRYSTATEOBJECT_H
#define __PASTRYSTATEOBJECT_H

/**
 * @file PastryStateObject.h
 * @author Felix Palmen
 */

#include <vector>

#include <omnetpp.h>

#include <NodeHandle.h>
#include <NodeVector.h>

#include "PastryTypes.h"
#include "PastryMessage_m.h"

/**
 * PastryStateObject Module
 *
 * This module class describes the common interface of all Pastry State Objects
 * and implements what all have in common
 *
 * @author Felix Palmen
 * @see PastryRoutingTable, LeafSet, NeighborhoodSet
 */
class PastryStateObject : public cSimpleModule
{
  public:

    void handleMessage(cMessage* msg);

    int numInitStages(void) const;

    void initialize(int stage);

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
     * shared prefix as the current node in the state table. this method
     * is to be called, when a regular next hop couldn't be found or wasn't
     * reachable.
     *
     * @param destination the destination key
     * @param optimize if set, check all nodes and return the best/closest one
     * @return a closer NodeHandle or NodeHandle::UNSPECIFIED_NODE if none was
     *	  found
     */
    virtual const NodeHandle& findCloserNode(const OverlayKey& destination,
                                             bool optimize = false) = 0;

    virtual void findCloserNodes(const OverlayKey& destination,
                                 NodeVector* nodes) = 0;

    /**
     * do something about a failed node
     *
     * @param failed the failed node
     * @return a node to ask for REPAIR or TransportAddress::UNSPECIFIED_NODE
     */
    virtual const TransportAddress& failedNode(const TransportAddress& failed) = 0;

    /**
     * attempt to repair state using a received REPAIR message
     *
     * @param msg the state message of type REPAIR
     * @param prox record of proximity values matching the state message
     * @return another node to ask for REPAIR or
     * TransportAddress::UNSPECIFIED_NODE
     */
    virtual const TransportAddress& repair(const PastryStateMessage* msg,
                                           const PastryStateMsgProximity& prox);

    /**
     * dump content of the set to a PastryStateMessage
     *
     * @param msg the PastryStateMessage to be filled with entries
     */
    virtual void dumpToStateMessage(PastryStateMessage* msg) const = 0;

    /**
     * update own state based on a received PastryStateMessage
     *
     * @param msg the PastryStateMessage to use as source for update
     * @param prox record of proximity values matching the state message
     * @return true if leafSet was actually changed
     */
    bool mergeState(const PastryStateMessage* msg,
                    const PastryStateMsgProximity* prox);

    /**
     * append all entries to a given vector of TransportAddresses,
     * needed to find all Nodes to be notified after joining.
     *
     * @param affected the vector to fill with entries
     */
    virtual void dumpToVector(std::vector<TransportAddress>& affected)
        const = 0;

    /**
     * test a given NodeHandle if it is closer to a given destination
     *
     * @param test the NodeHandle to test
     * @param destination the destination Key
     * @param reference NodeHandle to compare to, own node if unset
     * @return true if test is closer to destination than owner
     */
    bool isCloser(const NodeHandle& test, const OverlayKey& destination,
                  const NodeHandle& reference =
                      NodeHandle::UNSPECIFIED_NODE) const;

    /**
     * test a given NodeHandle if it is closer to a given destination, but
     * only if the shared prefix length with the destination is at least equal
     * to the shared prefix length with our own node
     *
     * This is needed for the "rare case" in the Pastry routing algorithm.
     *
     * @param test the NodeHandle to test
     * @param destination the destination Key
     * @param reference NodeHandle to compare to, own node if unset
     * @return true if test is closer to destination than owner
     */
    bool specialCloserCondition(const NodeHandle& test,
                                const OverlayKey& destination,
                                const NodeHandle& reference =
                                    NodeHandle::UNSPECIFIED_NODE) const;

  protected:

    /**
     * stores the NodeHandle of the owner of this PastryStateObject.
     * Derived classes have to initialize it.
     */
    NodeHandle owner;

    uint32_t bitsPerDigit;

    /**
     * unspecified Node with proximity
     */
  private:
    static const PastryExtendedNode* _unspecNode;

  protected:
    static const PastryExtendedNode& unspecNode()
    {
        if (_unspecNode == NULL)
            _unspecNode = new PastryExtendedNode();
        return *_unspecNode;
    }

  private:

    /**
     * initialize watches etc.
     */
    virtual void earlyInit(void) = 0;

    /**
     * try to merge a single node in the state table
     *
     * @param node handle of the node
     * @param prox proximity value of the node
     * @return true if node was inserted
     */
    virtual bool mergeNode(const NodeHandle& node, simtime_t prox) = 0;

    /**
     * compute the distance of two keys on the ring
     *
     * @param a one key
     * @param b another key
     * @return pointer to distance (must be deleted by caller)
     */
    const OverlayKey* keyDist(const OverlayKey& a, const OverlayKey& b) const;
};

#endif
