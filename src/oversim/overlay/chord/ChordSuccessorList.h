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
 * @file ChordSuccessorList.h
 * @author Markus Mauch, Ingmar Baumgart
 */

#ifndef __CHORDSUCCESSORLIST_H_
#define __CHORDSUCCESSORLIST_H_

#include <map>

#include <omnetpp.h>

#include <InitStages.h>
#include <NodeHandle.h>

class OverlayKey;
class NotifyResponse;

namespace oversim {

class Chord;

struct SuccessorListEntry
{
    NodeHandle nodeHandle ;//*< the nodehandle
    bool newEntry;  //*< true, if this entry has just been added
};

std::ostream& operator<<(std::ostream& os, const SuccessorListEntry& e);


/**
 * Chord's successor list module
 *
 * This modul contains the successor list of the Chord implementation.
 *
 * @author Markus Mauch, Ingmar Baumgart
 * @see Chord
 */
class ChordSuccessorList : public cSimpleModule
{
  public:
    virtual int numInitStages() const
    {
        return MAX_STAGE_OVERLAY + 1;
    }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage* msg);

    /**
     * Initializes the successor list. This should be called on startup
     *
     * @param size maximum number of neighbors in the successor list
     * @param owner the node owner is added to the successor list
     * @param overlay pointer to the main chord module
     */
    virtual void initializeList(uint32_t size, NodeHandle owner,
	Chord* overlay);

    /**
     * Returns number of neighbors in the successor list
     *
     * @return number of neighbors
     */
    virtual uint32_t getSize();

    /**
     * Checks if the successor list is empty
     *
     * @return returns false if the successor list contains other nodes
     *         than this node, true otherwise.
     */
    virtual bool isEmpty();

    /**
     * Returns a particular successor
     *
     * @param pos position in the successor list
     * @return successor at position pos
     */
    virtual const NodeHandle& getSuccessor(uint32_t pos = 0);

    /**
     * Adds new successor nodes to the successor list
     *
     * Adds new successor nodes to the successor list and sorts the
     * list using the corresponding chord keys. If the list size exceeds
     * the maximum size nodes at the end of the list will be removed.
     *
     * @param successor the node handle of the successor to be added
     * @param resize if true, shrink the list to successorListSize
     */
    virtual void addSuccessor(NodeHandle successor, bool resize = true);

    virtual void updateList(NotifyResponse* notify);

    bool handleFailedNode(const TransportAddress& failed);

    void display ();


  protected:
    NodeHandle thisNode; /**< own node handle */
    std::map<OverlayKey, SuccessorListEntry> successorMap; /**< internal representation of the successor list */

    uint32_t successorListSize; /**< maximum size of the successor list */

    Chord* overlay; /**< pointer to the main chord module */

    void removeOldSuccessors();

    /**
     * Displays the current number of successors in the list
     */
    void updateDisplayString();

    /**
     * Displays the first 4 successor nodes as tooltip.
     */
    void updateTooltip();
};

}; //namespace
#endif
