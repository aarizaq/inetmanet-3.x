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
 * @file ChordFingerTable.cc
 * @author Markus Mauch, Ingmar Baumgart
 */

#include <cfloat>

#include "hashWatch.h"

#include "Chord.h"
#include "ChordSuccessorList.h"
#include "ChordFingerTable.h"

namespace oversim {

Define_Module(ChordFingerTable);

void ChordFingerTable::initialize(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces
    // are registered, address auto-assignment takes place etc.
    if(stage != MIN_STAGE_OVERLAY)
        return;

    maxSize = 0;

    WATCH_DEQUE(fingerTable);
}

void ChordFingerTable::handleMessage(cMessage* msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

void ChordFingerTable::initializeTable(uint32_t size, const NodeHandle& owner,
                                       Chord* overlay)
{
    maxSize = size;
    this->overlay = overlay;
    fingerTable.clear();
}

uint32_t ChordFingerTable::getSize()
{
    return maxSize;
}

void ChordFingerTable::setFinger(uint32_t pos, const NodeHandle& node,
                                 Successors const* sucNodes)
{
    if (pos >= maxSize) {
        throw new cRuntimeError("ChordFingerTable::setFinger(): "
                                "Index out of bound");
    }

    uint32_t p = maxSize - pos - 1;
    Successors tempSuccessors;

    while (fingerTable.size() <= p) {
        fingerTable.push_back(FingerEntry(NodeHandle::UNSPECIFIED_NODE,
                                          tempSuccessors));
    }

    if (sucNodes != NULL) {
        fingerTable[p] = FingerEntry(node, *sucNodes);
    } else {
        fingerTable[p] = FingerEntry(node, tempSuccessors);
    }
}

void ChordFingerTable::setFinger(uint32_t pos, const Successors& nodes)
{
    setFinger(pos, nodes.begin()->second, &nodes);
}

bool ChordFingerTable::updateFinger(uint32_t pos, const NodeHandle& node,
                                    simtime_t rtt)
{
    if (rtt < 0)
        return false;

    if (pos >= maxSize) {
         throw new cRuntimeError("ChordFingerTable::updateFinger(): "
                                 "Index out of bound");
     }

    uint32_t p = maxSize - pos - 1;

    while (fingerTable.size() <= p) {
        Successors tempSuccessors;
        fingerTable.push_back(FingerEntry(NodeHandle::UNSPECIFIED_NODE,
                                          tempSuccessors));
    }

    Successors::iterator it;
    for (it = fingerTable[p].second.begin(); it != fingerTable[p].second.end();
         it++) {

        if (it->second == node) {
            break;
        }
    }

    if (it == fingerTable[p].second.end()) {
        return false;
    }

    fingerTable[p].second.erase(it);
    fingerTable[p].second.insert(std::make_pair(rtt, node));

    return true;
}

bool ChordFingerTable::handleFailedNode(const TransportAddress& failed)
{
    bool ret = false;
    for (int p = fingerTable.size() - 1; p >= 0; p--) {
        if (!fingerTable[p].first.isUnspecified() &&
            failed == fingerTable[p].first) {
            fingerTable[p].first = NodeHandle::UNSPECIFIED_NODE;
            ret = true;
        }
        for (std::multimap<simtime_t, NodeHandle>::iterator it =
             fingerTable[p].second.begin(); it != fingerTable[p].second.end();
             ++it) {
            if (failed == it->second) {
                fingerTable[p].second.erase(it);
                break;
            }
        }
    }

    return ret;
}

void ChordFingerTable::removeFinger(uint32_t pos)
{
    if (pos >= maxSize) {
        throw new cRuntimeError("ChordFingerTable::removeFinger(): "
                                "Index out of bound");
    }

    uint32_t p = maxSize - pos - 1;

    if (p >= fingerTable.size()) {
        return;
    } else if (p == (fingerTable.size() - 1)) {
        fingerTable.pop_back();
    } else {
        Successors tempSuccessors;
        fingerTable[p] = FingerEntry(NodeHandle::UNSPECIFIED_NODE,
                                     tempSuccessors);
    }
}

const NodeHandle& ChordFingerTable::getFinger(uint32_t pos)
{
    if (pos >= maxSize) {
        throw new cRuntimeError("ChordFingerTable::getFinger(): "
                                "Index out of bound");
    }

    uint32_t p = maxSize - pos - 1;

    if (p >= fingerTable.size()) {
        return overlay->successorList->getSuccessor();
    }
    while (fingerTable[p].first.isUnspecified() &&
            (p < (fingerTable.size() - 1))) {
        ++p;
    }
    if (fingerTable[p].first.isUnspecified())
        return overlay->successorList->getSuccessor();
    return fingerTable[p].first;
}

NodeVector* ChordFingerTable::getFinger(uint32_t pos, const OverlayKey& key)
{
    if (pos >= maxSize) {
        throw new cRuntimeError("ChordFingerTable::getFinger(): "
                                "Index out of bound");
    }

    NodeVector* nextHop = new NodeVector();
    uint32_t p = maxSize - pos - 1;

    if (p < fingerTable.size()) {
        for (Successors::const_iterator it = fingerTable[p].second.begin();
             it != fingerTable[p].second.end(); it++) {

            if(!key.isBetweenLR(fingerTable[p].first.getKey(), it->second.getKey())) {
                nextHop->push_back(it->second);
            }
        }
    } else {
        nextHop->push_back(overlay->successorList->getSuccessor());
        return nextHop;
    }

    if (nextHop->size() == 0) {
        if (fingerTable[p].first.isUnspecified()) {
            //TODO use other finger
            nextHop->push_back(overlay->successorList->getSuccessor());
        } else {
            nextHop->push_back(fingerTable[p].first);
        }
    }

    return nextHop;
}


}; //namespace

using namespace oversim;

std::ostream& operator<<(std::ostream& os, const Successors& suc)
{
    for (Successors::const_iterator i = suc.begin(); i != suc.end(); i++) {
        if (i != suc.begin()) {
            os << endl;
        }

        os << i->second;

        if (i->first == -1) {
            continue;
        } else if (i->first == MAXTIME) {
            os << "; RTT:  --- ";
        } else {
            os << "; RTT: " << i->first;
        }
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const FingerEntry& entry)
{
    if (entry.second.size() > 0) {
        os << "[ " << entry.first << " ]\n" << entry.second;
    } else {
        os << entry.first;
    }

    return os;
}
