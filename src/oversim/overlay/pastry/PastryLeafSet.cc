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
 * @file PastryLeafSet.cc
 * @author Felix Palmen
 */

#include "PastryLeafSet.h"
#include "PastryTypes.h"
#include "NodeVector.h"

#if 0
#define LEAF_TEST() \
    do {\
        uint32_t count = 0;\
        uint32_t i = 0, j = leaves.size() - 1;\
        while (leaves[i].isUnspecified() && i < j) ++i;\
        while (leaves[j].isUnspecified() && i < j) --j;\
        if (i == j) break;\
        if (!owner.getKey().isBetween(leaves[(leaves.size() / 2) - 1].getKey(),\
                                 leaves[leaves.size() / 2].getKey()))\
                ASSERT2(false, "leafset broken!");\
        for (uint32_t ci = i + 1; ci < j; ++ci) {\
            if (leaves[ci - 1].getKey() >= leaves[ci].getKey()) {\
                count++;\
            }\
        }\
        if (leaves[j].getKey() >= leaves[i].getKey()) count++;\
        ASSERT2(count <= 1, "leafset broken!");\
    } while (false);
#else
#define LEAF_TEST() {}
#endif

Define_Module(PastryLeafSet);

void PastryLeafSet::earlyInit(void)
{
    WATCH_VECTOR(leaves);
}

void PastryLeafSet::initializeSet(uint32_t numberOfLeaves,
                                  uint32_t bitsPerDigit,
                                  simtime_t repairTimeout,
                                  const NodeHandle& owner,
                                  BasePastry *overlay)
{
    if (numberOfLeaves % 2) throw "numberOfLeaves must be even.";

    this->owner = owner;
    this->numberOfLeaves = numberOfLeaves;
    this->repairTimeout = repairTimeout;
    this->bitsPerDigit = bitsPerDigit;
    this->overlay = overlay;

    if (!leaves.empty()) leaves.clear();

    // fill Set with unspecified node handles
    for (uint32_t i = numberOfLeaves; i>0; i--)
        leaves.push_back(NodeHandle::UNSPECIFIED_NODE);

    // initialize iterators to mark the beginning of bigger/smaller keys
    // in the set
    bigger = leaves.begin() + (numberOfLeaves >> 1);
    smaller = bigger - 1;

    // reset repair marker:
    if (!awaitingRepair.empty()) awaitingRepair.clear();

    newLeafs = false;
    isFull = false;
    wasFull = false;
}

const NodeHandle& PastryLeafSet::getSuccessor(void) const
{
    return *bigger;
}

const NodeHandle& PastryLeafSet::getPredecessor(void) const
{
    return *smaller;
}

bool PastryLeafSet::isValid(void) const
{
    return (!(smaller->isUnspecified() || bigger->isUnspecified()));
}

const NodeHandle& PastryLeafSet::getDestinationNode(
    const OverlayKey& destination)
{
    std::vector<NodeHandle>::const_iterator i;
    const OverlayKey* smallest;
    const OverlayKey* biggest;
    const NodeHandle* ret = &NodeHandle::UNSPECIFIED_NODE;

    // check whether destination is inside leafSet:

    smallest = &(getSmallestKey());
    biggest = &(getBiggestKey());
    if (smallest->isUnspecified()) smallest = &(owner.getKey());
    if (biggest->isUnspecified()) biggest = &(owner.getKey());

    if (!destination.isBetweenLR(*smallest, *biggest)) return *ret;

    // find the closest node:

    for (i = leaves.begin(); i != leaves.end(); i++) {
        if (i->isUnspecified()) continue;

        // note for next line:
        // * dereferences iterator, & gets address of element.
        if (isCloser(*i, destination, *ret)) ret = &(*i);
    }

    return *ret;
}

bool PastryLeafSet::isClosestNode(const OverlayKey& destination) const
{
    // check for simple cases first
    if (owner.getKey() == destination) {
        return true;
    }

    if (bigger->isUnspecified() && smaller->isUnspecified()) {
        return true;
    }

    // check if the next bigger or smaller node in the set is closer
    // than own node
    bool biggerIsCloser = false;
    bool smallerIsCloser = false;

    if (! bigger->isUnspecified()) {
        biggerIsCloser = isCloser(*bigger, destination);
    }
    if (! smaller->isUnspecified()) {
        smallerIsCloser = isCloser(*smaller, destination);
    }

    // return true if both are not closer
    return ((!biggerIsCloser) && (!smallerIsCloser));
}

void PastryLeafSet::dumpToStateMessage(PastryStateMessage* msg) const
{
    uint32_t i = 0;
    uint32_t size = 0;
    std::vector<NodeHandle>::const_iterator it;

    msg->setLeafSetArraySize(numberOfLeaves);
    for (it = leaves.begin(); it != leaves.end(); it++) {
        if (!it->isUnspecified()) {
            ++size;
            msg->setLeafSet(i++, *it);
        }
    }
    msg->setLeafSetArraySize(size);
}

//TODO ugly duplication of code
void PastryLeafSet::dumpToStateMessage(PastryLeafsetMessage* msg) const
{
    uint32_t i = 0;
    uint32_t size = 0;
    std::vector<NodeHandle>::const_iterator it;

    msg->setLeafSetArraySize(numberOfLeaves);
    for (it = leaves.begin(); it != leaves.end(); it++) {
        if (!it->isUnspecified()) {
            ++size;
            msg->setLeafSet(i++, *it);
        }
    }
    msg->setLeafSetArraySize(size);
}

const TransportAddress& PastryLeafSet::getRandomNode()
{
    //std::vector<NodeHandle>::iterator i;
    uint32_t rnd;
    int i;

    rnd = intuniform(0, numberOfLeaves - 1, 0);
    i = rnd;
    //while (rnd < leaves.size()) {
    while (i < (int)leaves.size()) {
        if (!leaves[i].isUnspecified()) return leaves[i];
        else i++;//rnd++;
    }
    i = rnd;
    while (i >= 0) {
        if (!leaves[i].isUnspecified()) return leaves[i];
        else i--;
    }
    EV << "Leafset::getRandomNode() returns UNSPECIFIED_NODE"
          "Leafset empty??" << endl;
    return TransportAddress::UNSPECIFIED_NODE;

}

NodeVector* PastryLeafSet::createSiblingVector(const OverlayKey& key,
        int numSiblings) const
{
    std::vector<NodeHandle>::const_iterator it;

    // create temporary comparator
    KeyDistanceComparator<KeyRingMetric>* comp =
        new KeyDistanceComparator<KeyRingMetric>( key );

    // create result vector
    NodeVector* result = new NodeVector( numSiblings, comp );

    result->add(owner);

    for (it = leaves.begin(); it != leaves.end(); it++) {
        if (!it->isUnspecified()) {
            result->add(*it);
        }
    }

    delete comp;

    if (!isValid()) {
        return result;
    }

    // if the leafset is not full, we could have a very small network
    // => return true (FIXME hack)
    if (leaves.front().isUnspecified() || leaves.back().isUnspecified()) {
        return result;
    }

    if ((result->contains(getBiggestKey())) ||
        (result->contains(getSmallestKey()))) {
        delete result;
        return NULL;
    }

    return result;
}

bool PastryLeafSet::mergeNode(const NodeHandle& node, simtime_t prox)
{
    assert(node != overlay->getThisNode());

    std::vector<NodeHandle>::iterator it, it_left, it_right;
    const OverlayKey* last_left = &(owner.getKey());
    const OverlayKey* last_right = &(owner.getKey());

    it_left = smaller;
    it_right = bigger;

    // avoid duplicates
    for (it = leaves.begin(); it != leaves.end(); ++it) {
        if (it->isUnspecified()) {
            isFull = false;
            continue;
        }
        if (it->getKey() == node.getKey()) return false;
    }

    // look for correct position in left and right half of leafset
    while (true) {
        if(!isFull) {
            // both sides free
            if(it_left->getKey().isUnspecified() &&
               it_right->getKey().isUnspecified()) {
                insertLeaf(it_left, node);
                return true;
            }
            if (it_left->getKey().isUnspecified() &&
                !node.getKey().isBetween(*last_right, it_right->getKey())) {
                // end of smaller entries found
                insertLeaf(it_left, node);
                return true;
            }
            if (it_right->getKey().isUnspecified() &&
                !node.getKey().isBetween(it_left->getKey(), *last_left)) {
                // end of bigger entries found
                insertLeaf(it_right, node);
                return true;
            }
        }
        // left side
        if (node.getKey().isBetween(it_left->getKey(), *last_left)) {
            // found correct position for inserting the new entry between
            // existing ones
            insertLeaf(it_left, node);
            return true;
        }
        // right side
        if (node.getKey().isBetween(*last_right, it_right->getKey())) {
            // found correct position for inserting the new entry between
            // existing ones
            insertLeaf(it_right, node);
            return true;
        }

        last_right = &(it_right->getKey());
        ++it_right;

        if (it_right == leaves.end()) break;

        last_left = &(it_left->getKey());
        --it_left;
    }
    return false;
}

void PastryLeafSet::insertLeaf(std::vector<NodeHandle>::iterator& it,
                               const NodeHandle& node)
{
    assert(node != overlay->getThisNode());

    LEAF_TEST();
    bool issmaller = (it <= smaller);
    if (issmaller) {
        if (!leaves.front().isUnspecified())  {
            overlay->callUpdate(leaves.front(), false);
        }
        overlay->callUpdate(node, true);

        leaves.insert(++it, node);
        NodeHandle& temp = leaves.front();
        if (!temp.isUnspecified() && leaves.back().isUnspecified()) {
            leaves.back() = temp;
        }
        leaves.erase(leaves.begin());


    } else {
        if (!leaves.back().isUnspecified())  {
            overlay->callUpdate(leaves.back(), false);
        }
        overlay->callUpdate(node, true);

        leaves.insert(it, node);
        NodeHandle& temp = leaves.back();
        if (!temp.isUnspecified() && leaves.front().isUnspecified()) {
            leaves.front() = temp;
        }
        leaves.pop_back();
    }

    if (!leaves.front().isUnspecified() &&
        !leaves.back().isUnspecified()) {
        isFull = true;
    } else isFull = false;

    newLeafs = true;
    bigger = leaves.begin() + (numberOfLeaves >> 1);
    smaller = bigger - 1;

    // ensure balance in leafset
    if (!isFull) {
        balanceLeafSet();
    }
    LEAF_TEST();
}

bool PastryLeafSet::balanceLeafSet()
{
    if (isFull ||
        (!leaves.front().isUnspecified() &&
         !(leaves.end() - 2)->isUnspecified()) ||
        (!leaves.back().isUnspecified() &&
         !(leaves.begin() + 1)->isUnspecified()))
        return false;

    std::vector<NodeHandle>::iterator it_left, it_right;

    for (it_left = smaller, it_right = bigger;
         it_right != leaves.end(); --it_left, ++it_right) {
        if (it_left->isUnspecified()) {
            if (it_right->isUnspecified() ||
                (it_right + 1) == leaves.end() ||
                (it_right + 1)->isUnspecified()) return false;
            *it_left = *(it_right + 1);
            *(it_right + 1) = NodeHandle::UNSPECIFIED_NODE;
            return true;
        } else if (it_right->isUnspecified()) {

            if (it_left == leaves.begin() ||
                (it_left - 1)->isUnspecified()) return false;
            *it_right = *(it_left - 1);
            *(it_left - 1) = NodeHandle::UNSPECIFIED_NODE;
            return true;
        }
    }
    return false; // should not happen
}

void PastryLeafSet::dumpToVector(std::vector<TransportAddress>& affected) const
{
    std::vector<NodeHandle>::const_iterator it;

    for (it = leaves.begin(); it != leaves.end(); it++)
        if (!it->isUnspecified())
            affected.push_back(*it);
}

const NodeHandle& PastryLeafSet::getSmallestNode(void) const
{
    std::vector<NodeHandle>::const_iterator i = leaves.begin();
    while ((i->isUnspecified()) && (i != smaller)) i++;
    assert(i->isUnspecified() || *i != overlay->getThisNode());
    return *i;
}

const OverlayKey& PastryLeafSet::getSmallestKey(void) const
{
    return getSmallestNode().getKey();
}

const NodeHandle& PastryLeafSet::getBiggestNode(void) const
{
    std::vector<NodeHandle>::const_iterator i = leaves.end()-1;
    while ((i->isUnspecified()) && (i != bigger)) i--;
    assert(i->isUnspecified() || *i != overlay->getThisNode());
    return *i;
}

const OverlayKey& PastryLeafSet::getBiggestKey(void) const
{
    return getBiggestNode().getKey();
}

void PastryLeafSet::findCloserNodes(const OverlayKey& destination,
                                    NodeVector* nodes)
{
    std::vector<NodeHandle>::const_iterator it;

    for (it = leaves.begin(); it != leaves.end(); it++)
        if (!it->isUnspecified())
            nodes->add(*it);

}

const NodeHandle& PastryLeafSet::findCloserNode(const OverlayKey& destination,
                                                bool optimize)
{
    std::vector<NodeHandle>::const_iterator i;
    const NodeHandle* ret = &NodeHandle::UNSPECIFIED_NODE;

    // this will only be called after getDestinationNode() returned
    // NodeHandle::UNSPECIFIED_NODE, so a closer Node can only be the biggest
    // or the smallest node in the LeafSet.

    const NodeHandle& smallest = getSmallestNode();
    const NodeHandle& biggest = getBiggestNode();

    if ((!smallest.isUnspecified()) &&
            (specialCloserCondition(smallest, destination, *ret))) {
        if (optimize) ret = &smallest;
        else return smallest;
    }

    if ((!biggest.isUnspecified()) &&
            (specialCloserCondition(biggest, destination, *ret))) {
        if (optimize) ret = &biggest;
        else return biggest;
    }

    return *ret;
}

const TransportAddress& PastryLeafSet::failedNode(const TransportAddress& failed)
{
    std::vector<NodeHandle>::iterator i;
    const TransportAddress* ask;
    bool left = true;

    // search failed node in leafset:
    for (i = leaves.begin(); i != leaves.end(); i++) {
        if (i == bigger) left = false;
        if ((! i->isUnspecified()) && (i->getIp() == failed.getIp())) break;
    }

    // failed node not in leafset:
    if (i == leaves.end()) return TransportAddress::UNSPECIFIED_NODE;

    overlay->callUpdate(*i, false);

    // remove failed node:
    leaves.erase(i);
    newLeafs = true;

    wasFull = isFull;
    isFull = false;

    // insert UNSPECIFIED_NODE at front or back and return correct node
    // to ask for repair:
    if (left) {
        leaves.insert(leaves.begin(), NodeHandle::UNSPECIFIED_NODE);
        bigger = leaves.begin() + (numberOfLeaves >> 1);
        smaller = bigger - 1;
        ask = static_cast<const TransportAddress*>(&(getSmallestNode()));
    } else {
        leaves.push_back(NodeHandle::UNSPECIFIED_NODE);
        bigger = leaves.begin() + (numberOfLeaves >> 1);
        smaller = bigger - 1;
        ask = static_cast<const TransportAddress*>(&(getBiggestNode()));
    }

    assert(ask->isUnspecified() || *ask != overlay->getThisNode());

    balanceLeafSet();
    LEAF_TEST();

    assert(ask->isUnspecified() || *ask != overlay->getThisNode());

    if (! ask->isUnspecified())
        awaitingRepair[*ask] = PLSRepairData(simTime(), left);

    return *ask;
}

const TransportAddress& PastryLeafSet::repair(const PastryStateMessage* msg,
                                              const PastryStateMsgProximity* prox)
{
    std::map<TransportAddress, PLSRepairData>::iterator it;
    const TransportAddress* ask;
    bool left;

    simtime_t now = simTime();

    // first eliminate outdated entries in awaitingRepair:
    for (it = awaitingRepair.begin(); it != awaitingRepair.end();) {
        if (it->second.ts < (now - repairTimeout)) {
            awaitingRepair.erase(it++);
        }
        else it++;
    }

    // don't expect any more repair messages:
    if (awaitingRepair.empty()) return TransportAddress::UNSPECIFIED_NODE;

    // look for source node in our list:
    if ( (it = awaitingRepair.find(msg->getSender())) == awaitingRepair.end() )
        return TransportAddress::UNSPECIFIED_NODE;

    // which side of the LeafSet is affected:
    left = it->second.left;

    // remove source node from list:
    awaitingRepair.erase(it);

    // merge info from repair message:
    if (mergeState(msg, prox) || isFull || !wasFull) {
        EV << "[PastryLeafSet::repair()]\n"
           << "    LeafSet repair was successful."
           << endl;
        return TransportAddress::UNSPECIFIED_NODE;
    } else {
        // repair did not succeed, try again:
        ask = &( left ? getSmallestNode() : getBiggestNode() );
        if (ask->isUnspecified() || *ask == msg->getSender()) {
            EV << "[PastryLeafSet::repair()]\n"
               << "    LeafSet giving up repair attempt."
               << endl;
            return TransportAddress::UNSPECIFIED_NODE;
        } else {
            awaitingRepair[*ask] = PLSRepairData(simTime(), left);
        }
        return *ask;
    }
}

PastryNewLeafsMessage* PastryLeafSet::getNewLeafsMessage(void)
{
    std::vector<NodeHandle>::const_iterator it;
    PastryNewLeafsMessage* msg;
    uint32_t i = 0;

    if (! newLeafs) return NULL;
    newLeafs = false;

    msg = new PastryNewLeafsMessage("PastryNewLeafs");

    msg->setLeafsArraySize(numberOfLeaves);
    for (it = leaves.begin(); it != leaves.end(); it++)
        msg->setLeafs(i++, *it);

    msg->setBitLength(PASTRYNEWLEAFS_L(msg));
    return msg;
}
