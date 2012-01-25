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
 * @file PastryNeighborhoodSet.cc
 * @author Felix Palmen
 */

#include "PastryNeighborhoodSet.h"
#include "PastryTypes.h"

Define_Module(PastryNeighborhoodSet);

void PastryNeighborhoodSet::earlyInit(void)
{
    WATCH_VECTOR(neighbors);
}

void PastryNeighborhoodSet::initializeSet(uint32_t numberOfNeighbors,
                                          uint32_t bitsPerDigit,
                                          const NodeHandle& owner)
{
    this->owner = owner;
    this->numberOfNeighbors = numberOfNeighbors;
    this->bitsPerDigit = bitsPerDigit;
    if (!neighbors.empty()) neighbors.clear();

    // fill Set with unspecified node handles
    for (uint32_t i = numberOfNeighbors; i>0; i--)
	neighbors.push_back(unspecNode());
}

void PastryNeighborhoodSet::dumpToStateMessage(PastryStateMessage* msg) const
{
    uint32_t i = 0;
    uint32_t size = 0;
    std::vector<PastryExtendedNode>::const_iterator it;

    msg->setNeighborhoodSetArraySize(numberOfNeighbors);
    for (it = neighbors.begin(); it != neighbors.end(); it++) {
        if (!it->node.isUnspecified()) {
            ++size;
            msg->setNeighborhoodSet(i++, it->node);
        }
    }
    msg->setNeighborhoodSetArraySize(size);
}

const NodeHandle& PastryNeighborhoodSet::findCloserNode(const OverlayKey& destination,
                                                        bool optimize)
{
    std::vector<PastryExtendedNode>::const_iterator it;

    if (optimize) {
        // pointer to later return value, initialize to unspecified, so
        // the specialCloserCondition() check will be done against our own
        // node as long as no node closer to the destination than our own was
        // found.
        const NodeHandle* ret = &NodeHandle::UNSPECIFIED_NODE;

        for (it = neighbors.begin(); it != neighbors.end(); it++) {
            if (it->node.isUnspecified()) break;
            if (specialCloserCondition(it->node, destination, *ret))
                ret = &(it->node);
        }
        return *ret;
    } else {
        for (it = neighbors.begin(); it != neighbors.end(); it++) {
            if (it->node.isUnspecified()) break;
            if (specialCloserCondition(it->node, destination)) return it->node;
        }
        return NodeHandle::UNSPECIFIED_NODE;
    }
}

void PastryNeighborhoodSet::findCloserNodes(const OverlayKey& destination,
                                            NodeVector* nodes)
{
    std::vector<PastryExtendedNode>::const_iterator it;

    for (it = neighbors.begin(); it != neighbors.end(); it++)
        if (! it->node.isUnspecified())
            nodes->add(it->node);
}

bool PastryNeighborhoodSet::mergeNode(const NodeHandle& node, simtime_t prox)
{
    std::vector<PastryExtendedNode>::iterator it;

    bool nodeAlreadyInVector = false; // was the node already in the list?
    bool nodeValueWasChanged = false;  // true if the list was changed, false if the rtt was too big
    // look for node in the set, if it's there and the value was changed, erase it (since the position is no longer valid)
    for (it = neighbors.begin(); it != neighbors.end(); it++) {
        if (!it->node.isUnspecified() && it->node == node) {
            if (prox == SimTime::getMaxTime() || it->rtt == prox) return false; // nothing to do!
            neighbors.erase(it);
            nodeAlreadyInVector = true;
            break;
        }
    }
    // look for the correct position for the node
    for (it = neighbors.begin(); it != neighbors.end(); it++) {
        if (it->node.isUnspecified() || (it->rtt > prox)) {
            nodeValueWasChanged = true;
            break;
        }
    }
    neighbors.insert(it, PastryExtendedNode(node, prox)); // insert the entry there
    if (!nodeAlreadyInVector) neighbors.pop_back(); // if a new entry was inserted, erase the last entry
    return !nodeAlreadyInVector && nodeValueWasChanged; // return whether a new entry was added
}

void PastryNeighborhoodSet::dumpToVector(std::vector<TransportAddress>& affected) const
{
    std::vector<PastryExtendedNode>::const_iterator it;

    for (it = neighbors.begin(); it != neighbors.end(); it++)
        if (! it->node.isUnspecified())
            affected.push_back(it->node);
}

const TransportAddress& PastryNeighborhoodSet::failedNode(const TransportAddress& failed)
{
    std::vector<PastryExtendedNode>::iterator it;

    for (it = neighbors.begin(); it != neighbors.end(); it++) {
        if (it->node.isUnspecified()) break;
        if (it->node.getIp() == failed.getIp()) {
            neighbors.erase(it);
	    neighbors.push_back(unspecNode());
            break;
        }
    }

    // never ask for repair
    return TransportAddress::UNSPECIFIED_NODE;
}

std::ostream& operator<<(std::ostream& os, const PastryExtendedNode& n)
{
    os << n.node << ";";
    if (n.rtt != SimTime::getMaxTime())
        os << " Ping: " << n.rtt;

    return os;
}

