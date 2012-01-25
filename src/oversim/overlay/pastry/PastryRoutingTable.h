#ifndef __PASTRYROUTINGTABLE_H_
#define __PASTRYROUTINGTABLE_H_

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
 * @file PastryRoutingTable.h
 * @author Felix Palmen
 */

#include <vector>

#include <omnetpp.h>

#include <NodeHandle.h>

#include "PastryStateObject.h"
#include "PastryTypes.h"
#include "PastryMessage_m.h"

/**
 * Vector-type of a line in Pastry IRoutingTable
 */
typedef std::vector<PastryExtendedNode> PRTRow;

/**
 * Struct for tracking attempts to repair a routing table entry
 */
struct PRTTrackRepair
{
    TransportAddress node;//*< the node last asked for repair
    simtime_t timestamp;  //*< the time when it was asked
    uint32_t failedRow;       //*< row of the failed node
    uint32_t failedCol;	  //*< col of the failed node
    uint32_t askedRow;	  //*< row of the node last asked
    uint32_t askedCol;	  //*< col of the node last asked
};

/**
 * Routing table module
 *
 * This module contains the routing table of the Chord implementation.
 *
 * @author Felix Palmen
 * @see Pastry
 */
class PastryRoutingTable : public PastryStateObject
{

  public:
    /**
     * Initializes the routing table. This should be called on startup
     *
     * @param bitsPerDigit Pastry configuration parameter
     * @param repairTimeout Pastry configuration parameter
     * @param owner the node this table belongs to
     */
    void initializeTable(uint32_t bitsPerDigit, double repairTimeout,
                         const NodeHandle& owner);

    /**
     * gets the next hop according to the Pastry routing scheme.
     *
     * @param destination the destination key
     * @return the NodeHandle of the next Node or NodeHandle::UNSPECIFIED_NODE
     *	  if no next hop could be determined
     */
    const NodeHandle& lookupNextHop(const OverlayKey& destination);

    /**
     * try to find a node numerically closer to a given key with the same
     * shared prefix as the current node in the routing table. this method
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
     * tells the routing table that a node has failed
     *
     * @param failed the failed node
     * @return a node to ask for REPAIR or TransportAddress::UNSPECIFIED_NODE
     */
    virtual const TransportAddress& failedNode(const TransportAddress& failed);

    /**
     * attempt to repair the routing using a received REPAIR message
     *
     * @param msg the state message of type REPAIR
     * @param prox record of proximity values matching the state message
     * @return another node to ask for REPAIR or
     *    TransportAddress::UNSPECIFIED_NODE
     */
    virtual const TransportAddress& repair(const PastryStateMessage* msg,
                                           const PastryStateMsgProximity* prox);

    /**
     * dump content of the table to a PastryStateMessage
     *
     * @param msg the PastryStateMessage to be filled with entries
     */
    virtual void dumpToStateMessage(PastryStateMessage* msg) const;

    /**
     * dump content of a single row of the routing table to a message
     * @param msg the message to be filled with entries
     * @param row the number of the row
     */
    virtual void dumpRowToMessage(PastryRoutingRowMessage* msg, int row) const;

    /**
     * dump content of a single row of the routing table to a state message
     * @param msg the state message to be filled with entries
     * @param row the number of the row
     */
    virtual void dumpRowToMessage(PastryStateMessage* msg, int row) const;

    /**
     * gets the number of rows in the routing table
     * @return the number of rows in the routing table
     */
    int getLastRow();

     /**
     * returns a random node from the routing table
     *
     * @param row the row to choose a random node from
     * @return a random node or TransportAddress::UNSPECIFIED_NODE
     */
    virtual const TransportAddress& getRandomNode(int row);


    /**
     * merge a node in the IRoutingTable
     *
     * @param node the node to merge
     * @param prox proximity value of the node
     * @return true if node was merged
     */
    bool mergeNode(const NodeHandle& node, simtime_t prox);

    /**
     * initialize table from vector of PastryStateMsgHandles with STATE
     * messages received during JOIN phase. The vector has to be sorted by
     * JoinHopCount of the messages
     *
     * @param handles the vector of PastryStateMsgHandles
     * @return true on success
     */
    bool initStateFromHandleVector(const std::vector<PastryStateMsgHandle>& handles);

    /**
     * appends all routing table entries to a given vector of
     * TransportAddresses, needed to find all Nodes to be notified after
     * joining.
     *
     * @param affected the vector to fill with routing table entries
     */
    virtual void dumpToVector(std::vector<TransportAddress>& affected) const;

    uint32_t nodesPerRow; //TODO getter/setter + private

  private:
    double repairTimeout;
    std::vector<PRTRow> rows;
    std::vector<PRTTrackRepair> awaitingRepair;

    virtual void earlyInit(void);

    /**
     * adds a new line to the routing table
     */
    void addRow(void);

    /**
     * returns n'th pastry digit from a key
     *
     * @param n which digit to return
     * @param key extract digit from this key
     * @return a pastry digit
     */
    uint32_t digitAt(uint32_t n, const OverlayKey& key) const;

    /**
     * returns routing table entry at specified position
     *
     * @param row the number of the row
     * @param col the number of the column
     */
    const PastryExtendedNode& nodeAt(uint32_t row, uint32_t col) const;

    /**
     * helper function, updates a PRTTrackRepair structure to point to the next
     * node that can be asked for repair
     *
     * @param track the PRTTrackRepair structure
     */
    void findNextNodeToAsk(PRTTrackRepair& track) const;

};

/**
 * Stream output operator to make WATCH() do something useful with the
 * routing table
 */
std::ostream& operator<<(std::ostream& os, const PRTRow& row);

#endif
