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
 * @file ChordFingerTable.h
 * @author Markus Mauch, Ingmar Baumgart
 */

#ifndef __CHORDFINGERTABLE_H_
#define __CHORDFINGERTABLE_H_

#include <deque>
#include <map>

#include <omnetpp.h>

#include <NodeVector.h>
#include <InitStages.h>

class BaseOverlay;

namespace oversim {

typedef std::multimap<simtime_t, NodeHandle> Successors;
typedef std::pair<NodeHandle, Successors> FingerEntry;

class Chord;

/**
 * Chord's finger table module
 *
 * This modul contains the finger table of the Chord implementation.
 *
 * @author Markus Mauch, Ingmar Baumgart
 * @see Chord
 */
class ChordFingerTable : public cSimpleModule
{
  public:

    virtual int numInitStages() const
    {
        return MAX_STAGE_OVERLAY + 1;
    }

    virtual void initialize(int stage);
    virtual void handleMessage(cMessage* msg);

    /**
     * Sets up the finger table
     *
     * Sets up the finger table and makes all fingers pointing
     * to the node itself. Should be called on startup to initialize
     * the finger table.
     *
     * @param size number of fingers
     * @param owner set all fingers to the key of node handle owner
     * @param overlay pointer to the main chord module
     */
    virtual void initializeTable(uint32_t size, const NodeHandle& owner,
                                 Chord* overlay);

    /**
     * Sets a particular finger to point to node
     *
     * @param pos number of the finger to set
     * @param node set finger to this node
     * @param sucNodes optional pointer containing a list of successors
     *                 of this finger
     */
    virtual void setFinger(uint32_t pos, const NodeHandle& node,
                           Successors const* sucNodes = NULL);
    virtual void setFinger(uint32_t pos, const Successors& nodes);

    virtual bool updateFinger(uint32_t pos, const NodeHandle& node, simtime_t rtt);

    /**
     * Returns the NodeVector of a particular finger
     *
     * @param pos number of the finger to get
     * @return NodeVector of the particular finger(s)
     */
    virtual const NodeHandle& getFinger(uint32_t pos);

    virtual NodeVector* getFinger(uint32_t pos, const OverlayKey& key);

    bool handleFailedNode(const TransportAddress& failed);

    /**
     * Deletes a particular finger
     *
     * @param pos number of the finger to get
     */
    void removeFinger(uint32_t pos);

    /**
     * Returns the size of the finger table
     *
     * @return number of fingers
     */
    virtual uint32_t getSize();

private:

    uint32_t maxSize; /**< maximum size of the finger table */
    std::deque<FingerEntry> fingerTable; /**< the finger table vector */
    Chord* overlay; /**< pointer to the main chord module */
};

}; //namespace

std::ostream& operator<<(std::ostream& os, const oversim::Successors& suc);
std::ostream& operator<<(std::ostream& os, const oversim::FingerEntry& entry);

#endif
