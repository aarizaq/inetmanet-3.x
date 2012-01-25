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
 * @file PastryStateObject.cc
 * @author Felix Palmen
 */

#include <InitStages.h>

#include "PastryStateObject.h"
#include "PastryTypes.h"

const PastryExtendedNode* PastryStateObject::_unspecNode = NULL;

int PastryStateObject::numInitStages() const
{
    return MAX_STAGE_OVERLAY;
}

void PastryStateObject::initialize(int stage)
{
    if (stage != MIN_STAGE_OVERLAY)
        return;
    earlyInit();
}

void PastryStateObject::handleMessage(cMessage* msg)
{
    throw "a PastryStateObject should never receive a message.";
}

const NodeHandle& PastryStateObject::getDestinationNode(const OverlayKey&
                                                            destination)
{
    return NodeHandle::UNSPECIFIED_NODE;
}

const TransportAddress& PastryStateObject::repair(const PastryStateMessage* msg,
                                                  const PastryStateMsgProximity&
                                                      prox)
{
    return TransportAddress::UNSPECIFIED_NODE;
}

bool PastryStateObject::mergeState(const PastryStateMessage* msg,
                                   const PastryStateMsgProximity* prox)
{
    bool ret = false;
    int lsSize = msg->getLeafSetArraySize();
    int rtSize = msg->getRoutingTableArraySize();
    int nsSize = msg->getNeighborhoodSetArraySize();
    const NodeHandle* node;
    simtime_t rtt;

    // walk through msg's LeafSet
    for (int i = 0; i < lsSize; i++) {
        node = &(msg->getLeafSet(i));
        rtt = prox ? (*(prox->pr_ls.begin() + i)) : SimTime::getMaxTime();

        // unspecified nodes, own node and dead nodes not considered
        if (!(rtt < 0 || node->isUnspecified() || *node == owner)) {
            if (mergeNode(*node, rtt)) ret = true;
        }
    }

    // walk through msg's IRoutingTable
    for (int i = 0; i < rtSize; i++) {
        node = &(msg->getRoutingTable(i));
        rtt = prox ? (*(prox->pr_rt.begin() + i)) : SimTime::getMaxTime();

        // unspecified nodes, own node and dead nodes not considered
        if (!(rtt < 0 || node->isUnspecified() || *node == owner)) {
            if (mergeNode(*node, rtt)) ret = true;
        }
    }

    // walk through msg's NeighborhoodSet
    for (int i = 0; i < nsSize; i++) {
        node = &(msg->getNeighborhoodSet(i));
        rtt = prox ? (*(prox->pr_ns.begin() + i)) : SimTime::getMaxTime();

        // unspecified nodes, own node and dead nodes not considered
        if (!(rtt < 0 || node->isUnspecified() || *node == owner)) {
            if (mergeNode(*node, rtt)) ret = true;
        }
    }

    return ret;
}

const OverlayKey* PastryStateObject::keyDist(const OverlayKey& a,
                                             const OverlayKey& b) const
{
    const OverlayKey* smaller;
    const OverlayKey* bigger;

    if (a > b) {
        smaller = &b;
        bigger = &a;
    } else {
        smaller = &a;
        bigger = &b;
    }

    OverlayKey diff1(*bigger - *smaller);
    OverlayKey diff2(*smaller + (OverlayKey::getMax() - *bigger) + 1);

    const OverlayKey* dist;
    if (diff1 > diff2) {
        dist = new OverlayKey(diff2);
    } else {
        dist = new OverlayKey(diff1);
    }

    return dist;
}

bool PastryStateObject::isCloser(const NodeHandle& test,
                                 const OverlayKey& destination,
                                 const NodeHandle& reference) const
{
    // assert: (! test.isUnspecified()) && (! owner.isUnspecified())

    const NodeHandle* ref = &reference;
    if (ref->isUnspecified()) ref = &owner;

    if ((ref->getKey() == destination) || (test == *ref)) {
        return false;
    }

    bool closer = false;
    const OverlayKey* refDist = keyDist(ref->getKey(), destination);
    const OverlayKey* testDist = keyDist(test.getKey(), destination);
    if (*testDist < *refDist)
        closer = true;
    delete refDist;
    delete testDist;
    return closer;
}

bool PastryStateObject::specialCloserCondition(const NodeHandle& test,
                                               const OverlayKey& destination,
                                               const NodeHandle& reference)
                                               const
{
//    std::cout << ((test.getKey().sharedPrefixLength(destination, bitsPerDigit)
//                  < owner.getKey().sharedPrefixLength(destination, bitsPerDigit))
//                  != (test.getKey().sharedPrefixLength(destination)
//                      < owner.getKey().sharedPrefixLength(destination)) ? "X\n" : "");

    if (test.getKey().sharedPrefixLength(destination, bitsPerDigit)
            < owner.getKey().sharedPrefixLength(destination, bitsPerDigit)) {
        return false;
    }

    return isCloser(test, destination, reference);
}
