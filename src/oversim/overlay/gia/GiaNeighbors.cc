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
 * @file GiaNeighbors.cc
 * @author Robert Palmer, Bernhard Heep
 */

#include <assert.h>

#include <InitStages.h>

#include "GiaNeighbors.h"


Define_Module(GiaNeighbors);

void GiaNeighbors::initialize(int stage)
{
    // wait until IPAddressResolver finished his initialization
    if(stage != MIN_STAGE_OVERLAY)
        return;

    WATCH_MAP(neighbors);
    timeout = getParentModule()->getSubmodule("gia")->par("neighborTimeout");
    //unspecNode = GiaNode::UNSPECIFIED_NODE;
}


void GiaNeighbors::handleMessages( cMessage* msg )
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

uint32_t GiaNeighbors::getSize() const
{
    return neighbors.size();
}

bool GiaNeighbors::contains(const OverlayKey& key) const
{
    NeighborsConstIterator it = neighbors.begin();

    for(it = neighbors.begin(); it != neighbors.end(); it++)
        if(it->first.getKey() == key)
            break;

    if (it != neighbors.end())
        return true;
    return false;
}

bool GiaNeighbors::contains(const GiaNode& node) const
{
    NeighborsConstIterator it = neighbors.find(node);

    if(it != neighbors.end())
        return true;
    return false;
}

void GiaNeighbors::add(const GiaNode& node, unsigned int degree)
{
    GiaNeighborInfo info = {degree,
                            5,
                            5,
                            simTime(),
                            GiaKeyList()};

    neighbors.insert(std::make_pair(node, info));
    //neighbors.insert(node);
}

// void GiaNeighbors::add(const NodeHandle& node)
// {
//     GiaNeighborInfo info = {node.getCapacity(),
// 			    node.getConnectionDegree(),
// 			    NULL}
//     neighbors.insert(std::make_pair(node, info));
// }


void GiaNeighbors::remove(const GiaNode& node)
{
    neighbors.erase(node);
}

const GiaNode& GiaNeighbors::get(unsigned int i)
{
    assert( getSize() && i <= getSize() );
    NeighborsIterator it = neighbors.begin();

    for(unsigned int j = 0; j < i; j++)
	it++;

    if (it != neighbors.end()) return it->first;
    return GiaNode::UNSPECIFIED_NODE;
}

GiaNeighborInfo* GiaNeighbors::get(const GiaNode& node)
{
    if (node.isUnspecified()) return NULL;

    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end())
        return &(it->second);
    return NULL;
}

const GiaNode& GiaNeighbors::get(const OverlayKey& key)
{
    NeighborsIterator it;

    for(it = neighbors.begin(); it != neighbors.end(); it++)
        if(it->first.getKey() == key)
            break;

    if(it != neighbors.end())
        return it->first;
    return GiaNode::UNSPECIFIED_NODE;
}

void GiaNeighbors::updateTimestamp(const GiaNode& node)
{
    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end())
        it->second.timestamp = simTime();
}

void GiaNeighbors::removeTimedoutNodes()
{
    NeighborsIterator it = neighbors.begin();

    while(it != neighbors.end()) {
        if(simTime() > (it->second.timestamp + timeout)) {
            neighbors.erase(it);
            it = neighbors.begin();//not efficient
        }
        else
            it++;
    }

}

//TODO keyList pointer
void GiaNeighbors::setNeighborKeyList(const GiaNode& node,
                                      const GiaKeyList& keyList)
{
    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end())
        it->second.keyList = keyList;
}

GiaKeyList* GiaNeighbors::getNeighborKeyList(const GiaNode& node)
{
    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end())
        return &(it->second.keyList);
    return NULL;
}

double GiaNeighbors::getCapacity(const GiaNode& node) const
{
    NeighborsConstIterator it = neighbors.find(node);

    if(it != neighbors.end())
        return it->first.getCapacity();
    return 0;
}

// void GiaNeighbors::setCapacity(const GiaNode& node, double capacity)
// {
//     NeighborsIterator it = neighbors.find(node);

//     if(it != neighbors.end())
// 	it->first.setCapacity(capacity);
// }

unsigned int GiaNeighbors::getConnectionDegree(const GiaNode& node) const
{
    NeighborsConstIterator it = neighbors.find(node);

    if(it != neighbors.end())
        return it->second.connectionDegree;
    return 0;
}

void GiaNeighbors::setConnectionDegree(const GiaNode& node,
                                       unsigned int degree)
{
    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end())
        it->second.connectionDegree = degree;
}

void GiaNeighbors::setReceivedTokens(const GiaNode& node,
                                     unsigned int tokens)
{
    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end()) {
        std::cout << "recieved: " << it->second.receivedTokens << " -> " << tokens << std::endl;
        it->second.receivedTokens = tokens;
    }
}

void GiaNeighbors::increaseReceivedTokens(const GiaNode& node)
{
    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end())
        it->second.receivedTokens++;
}

void GiaNeighbors::decreaseReceivedTokens(const GiaNode& node)
{
    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end())
        it->second.receivedTokens--;
}

unsigned int GiaNeighbors::getReceivedTokens(const GiaNode& node) const
{
    NeighborsConstIterator it = neighbors.find(node);

    if(it != neighbors.end())
        return it->second.receivedTokens;
    return 0;
}


void GiaNeighbors::setSentTokens(const GiaNode& node, unsigned int tokens)
{
    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end()) {
        std::cout << "sent: " << it->second.sentTokens << " -> " << tokens << std::endl;
        it->second.sentTokens = tokens;
    }
}

void GiaNeighbors::increaseSentTokens(const GiaNode& node)
{
    NeighborsIterator it = neighbors.find(node);

    if(it != neighbors.end() && it->second.sentTokens >= 0)
        it->second.sentTokens++;
}

unsigned int GiaNeighbors::getSentTokens(const GiaNode& node) const
{
    NeighborsConstIterator it = neighbors.find(node);

    if(it != neighbors.end())
        return it->second.sentTokens;
    return 0;
}

const GiaNode& GiaNeighbors::getDropCandidate(double capacity,
                                              unsigned int degree) const
{
    // determine node with highest capacity
    unsigned int subset = 0;
    double maxCapacity = 0;
    unsigned int dropDegree = 0;
    GiaNode dropCandidate;

    NeighborsConstIterator it, candIt;
    for(it = neighbors.begin(); it != neighbors.end(); it++) {
        if(it->first.getCapacity() <= capacity) {
            subset++;
            if(it->first.getCapacity() > maxCapacity) {
                candIt = it;
                dropDegree = it->second.connectionDegree;
                maxCapacity = it->first.getCapacity();
            }
        }
    }

    if(subset > 0 &&
            (/*subset == neighbors->getSize() || */dropDegree > degree) &&
            dropDegree > 1) {
        return candIt->first;
    }

    return GiaNode::UNSPECIFIED_NODE;
                                              }

std::ostream& operator<<(std::ostream& os, const GiaNeighborInfo& info)
{
    os //<< "C: " << info.capacity
    << ", degree: "  << info.connectionDegree
    << ", rTokens "  << info.receivedTokens
    << ", sTokens " << info.sentTokens
    << ", tStamp: " << info.timestamp;
    return os;
}
