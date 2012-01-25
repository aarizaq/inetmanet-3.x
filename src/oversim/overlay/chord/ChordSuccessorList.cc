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
 * @file ChordSuccessorList.cc
 * @author Markus Mauch, Ingmar Baumgart
 */

#include <cassert>

#include "ChordSuccessorList.h"

#include "Chord.h"

namespace oversim {

Define_Module(ChordSuccessorList);

using namespace std;

std::ostream& operator<<(std::ostream& os, const SuccessorListEntry& e)
{
    os << e.nodeHandle << " " << e.newEntry;
    return os;
};

void ChordSuccessorList::initialize(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces
    // are registered, address auto-assignment takes place etc.
    if (stage != MIN_STAGE_OVERLAY)
        return;

    WATCH_MAP(successorMap);
}

void ChordSuccessorList::handleMessage(cMessage* msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

void ChordSuccessorList::initializeList(uint32_t size, NodeHandle owner,
                                        Chord *overlay)
{
    successorMap.clear();
    successorListSize = size;
    thisNode = owner;
    this->overlay = overlay;
    addSuccessor(thisNode);
}

uint32_t ChordSuccessorList::getSize()
{
    return successorMap.size();
}

bool ChordSuccessorList::isEmpty()
{
    if (successorMap.size() == 1 && getSuccessor() == thisNode)
        return true;
    else
        return false;
}

const NodeHandle& ChordSuccessorList::getSuccessor(uint32_t pos)
{
    // check boundaries
    if (pos == 0 && successorMap.size() == 0)
        return NodeHandle::UNSPECIFIED_NODE;

    if (pos >= successorMap.size()) {
        error("Index out of bound (ChordSuccessorList, getSuccessor())");
    }

    std::map<OverlayKey, SuccessorListEntry>::iterator it =
        successorMap.begin();

    for (uint32_t i= 0; i < pos; i++) {
        it++;
        if (i == (pos-1))
            return it->second.nodeHandle;
    }
    return it->second.nodeHandle;
}

void ChordSuccessorList::updateList(NotifyResponse* notifyResponse)
{
    addSuccessor(notifyResponse->getSrcNode(), false);

    for (uint32_t k = 0; ((k < static_cast<uint32_t>(notifyResponse->getSucNum()))
                     && (k < (successorListSize - 1))); k++) {
        NodeHandle successor = notifyResponse->getSucNode(k);

        // don't add nodes, if this would change our successor
        if (successor.getKey().isBetweenLR(thisNode.getKey(),
                                      notifyResponse->getSrcNode().getKey()))
            continue;

        addSuccessor(successor, false);
    }

    removeOldSuccessors();
    assert(!isEmpty());
}


void ChordSuccessorList::addSuccessor(NodeHandle successor, bool resize)
{
    OverlayKey sum = successor.getKey() - (thisNode.getKey() + OverlayKey::ONE);

    std::map<OverlayKey, SuccessorListEntry>::iterator it =
        successorMap.find(sum);

    // Make a CommonAPI update() upcall to inform application
    // about our new neighbor in the successor list

    if (it == successorMap.end()) {
        // TODO: first add node and than call update()
        overlay->callUpdate(successor, true);
    } else {
        successorMap.erase(it);
    }

    SuccessorListEntry entry;
    entry.nodeHandle = successor;
    entry.newEntry = true;

    successorMap.insert(make_pair(sum, entry));

    if ((resize == true) && (successorMap.size() > (uint32_t)successorListSize)) {
        it = successorMap.end();
        it--;
        overlay->callUpdate(it->second.nodeHandle, false);
        successorMap.erase(it);
    }
}

bool ChordSuccessorList::handleFailedNode(const TransportAddress& failed)
{
    assert(failed != thisNode);
    for (std::map<OverlayKey, SuccessorListEntry>::iterator iter =
         successorMap.begin(); iter != successorMap.end(); ++iter) {
        if (failed == iter->second.nodeHandle) {
            successorMap.erase(iter);
            overlay->callUpdate(failed, false);
            // ensure that thisNode is always in the successor list
            if (getSize() == 0)
                addSuccessor(thisNode);
            return true;
        }
    }
    return false;
}

void ChordSuccessorList::removeOldSuccessors()
{
    std::map<OverlayKey,SuccessorListEntry>::iterator it;

    for (it = successorMap.begin(); it != successorMap.end();) {

        if (it->second.newEntry == false) {
            overlay->callUpdate(it->second.nodeHandle, false);
            successorMap.erase(it++);
        } else {
            it->second.newEntry = false;
            it++;
        }
    }

    it = successorMap.end();
    it--;

    while (successorMap.size() > successorListSize) {
        successorMap.erase(it--);
    }

    if (getSize() == 0)
        addSuccessor(thisNode);
}


void ChordSuccessorList::updateDisplayString()
{
    // FIXME: doesn't work without tcl/tk
    //    	if (ev.isGUI()) {
    if (1) {
        char buf[80];

        if (successorMap.size() == 1) {
            sprintf(buf, "1 successor");
        } else {
            sprintf(buf, "%zi successors", successorMap.size());
        }

        getDisplayString().setTagArg("t", 0, buf);
        getDisplayString().setTagArg("t", 2, "blue");
    }

}

void ChordSuccessorList::updateTooltip()
{
    if (ev.isGUI()) {
        std::stringstream str;
        for (uint32_t i = 0; i < successorMap.size(); i++)	{
            str << getSuccessor(i);
            if ( i != successorMap.size() - 1 )
                str << endl;
        }


        char buf[1024];
        sprintf(buf, "%s", str.str().c_str());
        getDisplayString().setTagArg("tt", 0, buf);
    }
}

void ChordSuccessorList::display()
{
    cout << "Content of ChordSuccessorList:" << endl;
    for (std::map<OverlayKey,SuccessorListEntry>::iterator it =
        successorMap.begin(); it != successorMap.end(); it++)
        cout << it->first << " with Node: " << it->second.nodeHandle << endl;
}

}; //namespace
