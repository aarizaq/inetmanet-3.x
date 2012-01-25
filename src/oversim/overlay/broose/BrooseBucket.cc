//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file BrooseBucket.cc
 * @author Jochen Schenk
 */

#include <omnetpp.h>
#include "BrooseBucket.h"
#include "BrooseHandle.h"

using namespace std;

const BrooseHandle* BrooseHandle::_unspecifiedNode = NULL;

Define_Module(BrooseBucket);

const int MAXBITS = 1024;

void BrooseBucket::initialize(int stage)
{
    if(stage != MIN_STAGE_OVERLAY)
        return;

    WATCH_MAP(bucket);
}

void BrooseBucket::handleMessage(cMessage* msg)
{
    error("BrooseBucket::handleMessage() shouldn't be called!");
}

void BrooseBucket::initializeBucket(int shiftingBits, uint32_t prefix,
                                    int size, Broose* overlay, bool isBBucket)
{
    maxSize = size;
    this->overlay = overlay;
    this->isBBucket = isBBucket;

    if (shiftingBits < 0) {
        key = overlay->getThisNode().getKey() << -shiftingBits;
    } else {
        key = overlay->getThisNode().getKey() >> shiftingBits;
    }

    if (prefix != 0) {
        OverlayKey tmp(prefix); // constraint
        tmp = tmp << (overlay->getThisNode().getKey().getLength() - shiftingBits);
        key = key + tmp;
    }
    bucket.clear();
}

bool BrooseBucket::add(const NodeHandle& node, bool isAlive, simtime_t rtt)
{
    OverlayKey tmp = key ^ node.getKey();

    bucketIter = bucket.find(tmp);

    if (bucketIter == bucket.end()) {
        // new node
        if (bucket.size() < maxSize) {
             bucketIter = bucket.insert(make_pair(tmp,node)).first;
        } else {
            std::map<OverlayKey, BrooseHandle>::iterator back = --bucket.end();

            // is the new node closer than the one farthest away,
            // remove the one and add the other
            if (back->first > tmp) {
                if (isBBucket) {
                    // call update() for removed sibling
                    overlay->callUpdate(back->second, false);
                }

                bucket.erase(back);
                bucketIter = bucket.insert(make_pair(tmp,node)).first;
            } else {
                // doesn't fit into bucket
                return false;
            }
        }

        if (isBBucket) {
            // call update() for new sibling
            overlay->callUpdate(node, true);
        }
    }

    if (isAlive) {
        bucketIter->second.failedResponses = 0;
        bucketIter->second.lastSeen = simTime();
    }

    if (rtt != MAXTIME) {
        bucketIter->second.rtt = rtt;
    }

    return true;
}

void BrooseBucket::remove(const NodeHandle& node)
{
    unsigned int i = 0;
    for (bucketIter = bucket.begin(); bucketIter != bucket.end(); i++) {
        if (bucketIter->second.getIp() == node.getIp()) {
            if (isBBucket && (i < (maxSize/7))) {
                // call update() for removed sibling
                overlay->callUpdate(node, false);
                if (bucket.size() > (maxSize/7)) {
                    // new replacement sibling
                    overlay->callUpdate(get(maxSize/7), true);
                }
            }
            bucket.erase(bucketIter++);
        } else {
            ++bucketIter;
        }
    }
}

const BrooseHandle& BrooseBucket::get(uint32_t pos)
{
    if (pos > bucket.size()) {
        error("Index out of bounds(BrooseBucket).");
    }

    uint32_t i = 0;
    std::map<OverlayKey, BrooseHandle>::iterator it;

    for (it = bucket.begin(); it != bucket.end(); it++, i++) {
        if (pos == i) {
            return it->second;
        }
    }

    return BrooseHandle::unspecifiedNode();
}

const OverlayKey& BrooseBucket::getDist(uint32_t pos)
{
    if (pos > bucket.size()) {
        error("Index out of bounds(BrooseBucket).");
    }

    uint32_t i = 0;
    std::map<OverlayKey, BrooseHandle>::iterator it;

    for (it = bucket.begin(); it != bucket.end(); it++, i++) {
        if (pos == i) {
            return it->first;
        }
    }

    return OverlayKey::UNSPECIFIED_KEY;
}


uint32_t BrooseBucket::getSize()
{
    return bucket.size();
}

uint32_t BrooseBucket::getMaxSize()
{
    return maxSize;
}

bool BrooseBucket::isEmpty()
{
    return bucket.empty();
}

void BrooseBucket::clear()
{
    bucket.clear();
}

void BrooseBucket::fillVector(NodeVector* result)
{
    for (bucketIter = bucket.begin(); bucketIter!=bucket.end(); bucketIter++) {
        result->add(bucketIter->second);
    }
}


int BrooseBucket::longestPrefix()
{
    if (bucket.size() < 2)
        return 0;

    return bucket.begin()->second.getKey().sharedPrefixLength(
                                             (--bucket.end())->second.getKey());
}

void BrooseBucket::output(int maxEntries)
{
    BrooseHandle node;
    OverlayKey dist;

    EV << "[BrooseBucket::output() @ " << overlay->getThisNode().getIp()
       << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
       << "    BucketSize/MaxSize: " << bucket.size() << "/" << maxSize
       << endl;

    int max;
    max = bucket.size();

    if (maxEntries != 0 && maxEntries < max) {
        max = maxEntries;
    }

    int i;

    for (bucketIter = bucket.begin(), i = 0; i < max; bucketIter++, i++) {
        dist = bucketIter->first;
        node = bucketIter->second;
        EV << "    " << dist << " " << node.getKey() << " " << node.getIp() << " RTT: "
           << node.rtt << " LS: " << node.lastSeen
           << endl;
    }
}

bool BrooseBucket::keyInRange(const OverlayKey& key)
{
    OverlayKey dist;

    if (bucket.size() == 0)
        return false;

    // check if the function was called to perform on a B bucket
    if (isBBucket) {
        if (bucket.size() <=  (maxSize / 7))
            return true;
        else
            dist = getDist((maxSize / 7) - 1);
    } else
        dist = getDist(bucket.size()-1);

    if ((key ^ overlay->getThisNode().getKey()) <= dist)
        return true;
    else
        return false;

}

int BrooseBucket::getPos(const NodeHandle& node)
{
    int i = -1;
    std::map<OverlayKey, BrooseHandle>::iterator it;
    for (it = bucket.begin(); it != bucket.end(); it++, i++) {
        if (node.getIp() == it->second.getIp())
            return i;
    }
    return i;
}

int BrooseBucket::getFailedResponses (const NodeHandle& node)
{
    for (bucketIter = bucket.begin(); bucketIter!=bucket.end(); bucketIter++) {
        if (node.getIp() == bucketIter->second.getIp())
            return bucketIter->second.failedResponses;
    }
    return -1;
}

void BrooseBucket::increaseFailedResponses (const NodeHandle& node)
{
    for (bucketIter = bucket.begin(); bucketIter!=bucket.end(); bucketIter++) {
        if (node.getIp() == bucketIter->second.getIp())
            bucketIter->second.failedResponses++;
    }
}

void BrooseBucket::resetFailedResponses (const NodeHandle& node)
{
    for (bucketIter = bucket.begin(); bucketIter!=bucket.end(); bucketIter++) {
        if (node.getIp() == bucketIter->second.getIp())
            bucketIter->second.failedResponses = 0;
    }
}


void BrooseBucket::setRTT(const NodeHandle& node, simtime_t rpcRTT)
{
    for (bucketIter = bucket.begin(); bucketIter!=bucket.end(); bucketIter++) {
        if (node.getIp() == bucketIter->second.getIp()) {
            bucketIter->second.rtt = rpcRTT;
        }
    }
}

simtime_t BrooseBucket::getRTT(const NodeHandle& node)
{
    for (bucketIter = bucket.begin(); bucketIter!=bucket.end(); bucketIter++) {
        if (node.getIp() == bucketIter->second.getIp())
            return bucketIter->second.rtt;
    }
    return -2;
}

void BrooseBucket::setLastSeen(const NodeHandle& node, simtime_t time)
{
    for (bucketIter = bucket.begin(); bucketIter!=bucket.end(); bucketIter++) {
        if (node.getIp() == bucketIter->second.getIp())
            bucketIter->second.lastSeen = time;
    }
}

simtime_t BrooseBucket::getLastSeen(const NodeHandle& node)
{
    for (bucketIter = bucket.begin(); bucketIter!=bucket.end(); bucketIter++) {
        if (node.getIp() == bucketIter->second.getIp())
            return bucketIter->second.lastSeen;
    }
    return -2;
}
