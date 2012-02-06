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
 * @file Broose.cc
 * @author Jochen Schenk, Ingmar Baumgart
 */

#include "Broose.h"
#include <RpcMacros.h>
#include <GlobalStatistics.h>
#include <BootstrapList.h>
#include <LookupListener.h>

using namespace std;

Define_Module(Broose);

class BrooseLookupListener : public LookupListener
{
private:
    Broose* overlay;
public:
    virtual ~BrooseLookupListener() {}
    BrooseLookupListener(Broose* overlay)
    {
        this->overlay = overlay;
    }

    virtual void lookupFinished(AbstractLookup *lookup)
    {
        delete this;
    }
};

Broose::Broose()
{
    join_timer = NULL;
    bucket_timer = NULL;
    rBucket = NULL;
    lBucket = NULL;
    bBucket =  NULL;
}
Broose::~Broose()
{
    // delete timers
    cancelAndDelete(join_timer);
    cancelAndDelete(bucket_timer);
}

void Broose::initializeOverlay(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces
    // are registered, address auto-assignment takes place etc.
    if (stage != MIN_STAGE_OVERLAY)
        return;

    // Broose provides KBR services
    kbr = true;

    // fetch some parameters
    bucketSize = par("bucketSize"); // = k
    rBucketSize = par("rBucketSize"); // = k'
    joinDelay = par("joinDelay");
    shiftingBits = par("brooseShiftingBits");
    userDist = par("userDist");
    refreshTime = par("refreshTime");
    numberRetries = par("numberRetries");
    stab1 = par("stab1");
    stab2 = par("stab2");

    //statistics
    bucketCount = 0;
    bucketBytesSent = 0;

    //init local parameters
    chooseLookup = 0;
    receivedJoinResponse = 0;
    receivedBBucketLookup = 0;
    numberBBucketLookup = 0;
    receivedLBucketLookup = 0;
    numberLBucketLookup = 0;
    powShiftingBits = 1 << shiftingBits;
    keyLength = OverlayKey::getLength();
    numFailedPackets = 0;
    bucketRetries = 0;

    // add some watches
    WATCH(receivedJoinResponse);
    WATCH(receivedBBucketLookup);
    WATCH(numberBBucketLookup);
    WATCH(receivedLBucketLookup);
    WATCH(numberLBucketLookup);
    WATCH(state);

    // get module pointers for all buckets
    rBucket = new BrooseBucket*[powShiftingBits];

    for (int i = 0; i < powShiftingBits; i++) {
        rBucket[i] = check_and_cast<BrooseBucket*>
                     (getParentModule()->getSubmodule("rBucket",i));
        bucketVector.push_back(rBucket[i]);
    }

    lBucket = check_and_cast<BrooseBucket*>
              (getParentModule()->getSubmodule("lBucket"));
    bucketVector.push_back(lBucket);

    bBucket = check_and_cast<BrooseBucket*>
              (getParentModule()->getSubmodule("bBucket"));
    bucketVector.push_back(bBucket);

    // create join and bucket timer
    join_timer = new cMessage("join_timer");
    bucket_timer = new cMessage("bucket_timer");
}

void Broose::joinOverlay()
{
    changeState(INIT);

    // if the bootstrap node is unspecified we are the only node in the network
    // so we can skip the "normal" join protocol
    if (bootstrapNode.isUnspecified()) {
        changeState(READY);
    }
}


void Broose::changeState(int toState)
{
    switch (toState) {
    case INIT: {
        state = INIT;

        // find a new bootstrap node and enroll to the bootstrap list
        bootstrapNode = bootstrapList->getBootstrapNode();

        cancelEvent(join_timer);
        scheduleAt(simTime(), join_timer);

        // initialize respectively clear the buckets
        for (int i = 0; i < powShiftingBits; i++) {
            rBucket[i]->initializeBucket(shiftingBits, i, rBucketSize, this);
        }

        lBucket->initializeBucket(-shiftingBits, 0, powShiftingBits*rBucketSize,
                                  this);
        bBucket->initializeBucket(0, 0, 7*bucketSize, this, true);

        // if we have restarted the join protocol reset parameters
        receivedBBucketLookup = 0;
        receivedLBucketLookup = 0;
        receivedJoinResponse = 0;

        getParentModule()->getParentModule()->bubble("Enter INIT state.");
        updateTooltip();
        break;
    }

    case RSET: {
        state = RSET;

        BrooseBucket* tmpBucket = new BrooseBucket();
        tmpBucket->initializeBucket(0, 0, powShiftingBits*rBucketSize, this);

        for (int i = 0; i < powShiftingBits; i++) {
            int size = rBucket[i]->getSize();

            for (int j = 0; j < size; j++) {
                tmpBucket->add(rBucket[i]->get(j));
            }
        }

        BucketCall** bCall = new BucketCall*[tmpBucket->getSize()];
        for (uint32_t i = 0; i < tmpBucket->getSize(); i++) {
            bCall[i] = new BucketCall("LBucketCall");
            bCall[i]->setBucketType(LEFT);
            bCall[i]->setProState(PRSET);
            bCall[i]->setBitLength(BUCKETCALL_L(bcall[i]));

            sendUdpRpcCall(tmpBucket->get(i), bCall[i], NULL,
                           10);
        }

        // half of the calls must return for a state change
        numberBBucketLookup = ceil((double)tmpBucket->getSize() / 2);

        delete tmpBucket;

        getParentModule()->getParentModule()->bubble("Enter RSET state.");
        break;
    }

    case BSET: {
        state = BSET;

        // half of the calls must return for a state change
        numberLBucketLookup = ceil((double)bBucket->getSize() / 2);

        // send messages to all entries of the B Bucket
        int size2 = bBucket->getSize();
        BucketCall** bCall2 = new BucketCall*[size2];
        for (int i = 0; i < size2; i++) {
            bCall2[i] = new BucketCall("LBucketCall");
            bCall2[i]->setBucketType(LEFT);
            bCall2[i]->setProState(PBSET);
            bCall2[i]->setBitLength(BUCKETCALL_L(bcall2[i]));

            sendUdpRpcCall(bBucket->get(i), bCall2[i], NULL,
                           10);
        }

        getParentModule()->getParentModule()->bubble("Enter BSET state.");
        break;
    }

    case READY: {
        state = READY;

        // fill the bucket also with this node
        for (size_t i = 0; i < bucketVector.size(); i++) {
            bucketVector[i]->add(thisNode);
        }

        // to disable the ping protocol a pingDelay or
        // refreshTime of zero was given
        if (refreshTime != 0) {
            cancelEvent(bucket_timer);
            scheduleAt(simTime() + (refreshTime / 2.0), bucket_timer);
        }

        getParentModule()->getParentModule()->bubble("Enter READY state.");

        updateTooltip();
        break;
    }

    }
    setOverlayReady(state == READY);
}

void Broose::handleTimerEvent(cMessage* msg)
{
    if (msg == join_timer)
        handleJoinTimerExpired(msg);
    else if (msg == bucket_timer)
        handleBucketTimerExpired(msg);
    else
        error("Broose::handleTimerEvent - no other timer currently in use!");
}

void Broose::handleJoinTimerExpired(cMessage* msg)
{
    if (state == READY)
        return;

    if (!bootstrapNode.isUnspecified()) {
        // create new lookup message
#if 0
        BucketCall* bCall = new BucketCall();
        bCall->setBucketType(BROTHER);
        bCall->setProState(FAILED);
        bCall->setBitLength(BUCKETCALL_L(call));
        sendRouteRpcCall(OVERLAY_COMP, bootstrapNode, thisNode.getKey(),
                         bCall);

        BucketCall* lCall = new BucketCall();
        lCall->setBucketType(BROTHER);
        lCall->setProState(FAILED);
        lCall->setBitLength(BUCKETCALL_L(call));
        sendRouteRpcCall(OVERLAY_COMP, bootstrapNode,
                         thisNode.getKey() << shiftingBits, lCall);
#endif
        // do lookups for key >> shiftingBits for each prefix
        OverlayKey newKey = thisNode.getKey() >> shiftingBits;
        BucketCall* bCallArray[powShiftingBits];
        for (int i = 0; i < powShiftingBits; i++) {
            OverlayKey add(i);
            add = add << (keyLength - shiftingBits);
            add += newKey;

            bCallArray[i] = new BucketCall("BBucketCall");
            bCallArray[i]->setBucketType(BROTHER);
            bCallArray[i]->setBucketIndex(i);
            bCallArray[i]->setProState(PINIT);
            bCallArray[i]->setBitLength(BUCKETCALL_L(bCallArray[i]));

            // restart join protocol if one call times out
            // otherwise the node might be isolated
            sendRouteRpcCall(OVERLAY_COMP, bootstrapNode, add,
                             bCallArray[i]);
        }
        //createLookup()->lookup(getThisNode().getKey() + 1, 0, 0, 0,
        //                       new BrooseLookupListener(this));
    } else {
        // if the bootstrap node is unspecified we are the only node in the network
        // so we can skip the "normal" join protocol
        changeState(READY);
    }
}

void Broose::handleBucketTimerExpired(cMessage* msg)
{
    BrooseBucket* tmpBucket = new BrooseBucket();
    tmpBucket->initializeBucket(0, 0,
                                (2*powShiftingBits*rBucketSize + 7*bucketSize),
                                this);

    for (size_t i = 0; i < bucketVector.size(); i++) {
        for(uint32_t j = 0; j < bucketVector[i]->getSize(); j++) {
            if ((simTime() - bucketVector[i]->getLastSeen(
                        bucketVector[i]->get(j))) > refreshTime
                    || bucketVector[i]->getRTT(bucketVector[i]->get(j)) == -1) {

                tmpBucket->add(BrooseHandle(bucketVector[i]->get(j)));
            }
        }
    }

    for (uint32_t i = 0; i < tmpBucket->getSize(); i++) {
        pingNode(tmpBucket->get(i));
    }

    delete tmpBucket;

    scheduleAt(simTime() + (refreshTime / 2.0), bucket_timer);
}


int Broose::getMaxNumSiblings()
{
    return bucketSize;
}

int Broose::getMaxNumRedundantNodes()
{
    return bucketSize;
}

int Broose::getRoutingDistance(const OverlayKey& key, const OverlayKey& node,
                               int dist)
{
    for (uint i = 0; i < (uint)abs(dist); i++) {
        if (node.sharedPrefixLength(key << i) >= (abs(dist) - i)) {
             return i; // right shifting
         }
        if (key.sharedPrefixLength(node << i) >= (abs(dist) - i)) {
            return -i; // left shifting
        }
    }

    if (((chooseLookup++) % 2) == 0) {
        return -dist;
    } else {
        return dist;
    }
}

#if 0
// TODO: work in progress: new findNode() code which tries to calculate
//       the distance approximation and new routing key in each routing step
NodeVector* Broose::findNode(const OverlayKey& key,
                             int numRedundantNodes,
                             int numSiblings,
                             BaseOverlayMessage* msg)
{
    BrooseFindNodeExtMessage *findNodeExt = NULL;
    bool err;
    bool isSibling = isSiblingFor(thisNode, key, numSiblings, &err);
    int resultSize;

    if (numSiblings < 0) {
        // exhaustive iterative doesn't care about siblings
        resultSize = numRedundantNodes;
    } else {
        resultSize = isSibling ? (numSiblings ? numSiblings : 1)
                                                      : numRedundantNodes;
    }
    assert(numSiblings || numRedundantNodes);
    NodeVector* result = new NodeVector(resultSize);

    if (isSibling) {
        //return the closest nodes
        // sort with XOR distance to key
        KeyDistanceComparator<KeyXorMetric>* comp =
            new KeyDistanceComparator<KeyXorMetric>(key);
        result->setComparator(comp);

        bBucket->fillVector(result);
        result->add(thisNode);

        delete comp;


        std::cout << "key: " << key.toString(2).substr(0, 8)
                  << " ThisNode: " << thisNode.getKey().toString(2).substr(0, 8);
        if (result->size() > 0) {
            std::cout << " next hop (final): " << (*result)[0].getKey().toString(2).substr(0, 8);
        } else {
            std::cout << " no next hop! (final)";
        }
        std::cout << std::endl << std::endl;


        return result;
    }

    // estimate distance
    int dist = max(rBucket[0]->longestPrefix(),
                   rBucket[1]->longestPrefix()) + 1 + userDist;

    if ((dist % shiftingBits) != 0)
        dist += (shiftingBits - (dist % shiftingBits));

    if (dist > keyLength) {
        if ((keyLength % shiftingBits) == 0) {
            dist = keyLength;
        } else {
            dist = (keyLength - keyLength % shiftingBits);
        }
    }

    if (msg != NULL) {
        if (!msg->hasObject("findNodeExt")) {
            findNodeExt = new BrooseFindNodeExtMessage("findNodeExt");

            findNodeExt->setMaxDistance(dist);

            //add contact for next Hop
            findNodeExt->setLastNode(thisNode);
            findNodeExt->setBitLength(BROOSEFINDNODEEXTMESSAGE_L);

            msg->addObject( findNodeExt );
        }

        findNodeExt = (BrooseFindNodeExtMessage*) msg->getObject("findNodeExt");
    }

    // update buckets with last hop
    routingAdd(findNodeExt->getLastNode(), true);

    // replace last hop contact information with
    // this hop contact information
    findNodeExt->setLastNode(thisNode);

    //findNodeExt->setMaxDistance(max(findNodeExt->getMaxDistance(), dist));

    int step = getRoutingDistance(key, thisNode.getKey(),
                                  findNodeExt->getMaxDistance());

    bool leftShifting;
    if (step < 0) {
        leftShifting = true;
        step *= -1;
    }

    if ((step % shiftingBits) != 0)
        step += (shiftingBits - (step % shiftingBits));

    if (step > keyLength) {
        if ((keyLength % shiftingBits) == 0) {
            step = keyLength;
        } else {
            step = (keyLength - keyLength % shiftingBits);
        }
    }

    if (leftShifting) {
        step *= -1;
    }

    // check for messages which couldn't be routed
    if (step == 0) {
        //return the closest nodes
        // sort with XOR distance to key
        KeyDistanceComparator<KeyXorMetric>* comp =
            new KeyDistanceComparator<KeyXorMetric>(key);
        result->setComparator(comp);

        bBucket->fillVector(result);
        result->add(thisNode);


        std::cout << "key: " << key.toString(2).substr(0, 8)
                  << " dist: " << step << " (max: " << findNodeExt->getMaxDistance() << ")"
                  << " rtkey: " << thisNode.getKey().toString(2).substr(0, 8)
                  << " ThisNode: " << thisNode.getKey().toString(2).substr(0, 8);
        if (result->size() > 0) {
            std::cout << " next hop: " << (*result)[0].getKey().toString(2).substr(0, 8);
        } else {
            std::cout << " no next hop!";
        }
        std::cout << std::endl << std::endl;


        delete comp;
        return result;
    } else if (step < 0) {
        if (state == BSET) {
            return result;
        }
        // Left Shifting Lookup
        OverlayKey routingKey = key >> (-step - 1);
        for (int i = 0; i < (-step - 1); i++) {
            routingKey.setBit(OverlayKey::getLength() - i - 1,
                              thisNode.getKey().getBit(
                              OverlayKey::getLength() - i - 2));
        }

        KeyDistanceComparator<KeyXorMetric>* comp =
            new KeyDistanceComparator<KeyXorMetric>(routingKey);

        result->setComparator(comp);
        lBucket->fillVector(result);
        result->add(thisNode);
        delete comp;

        std::cout << "key: " << key.toString(2).substr(0, 8)
                  << " dist: " << step << " (max: " << findNodeExt->getMaxDistance() << ")"
                  << " rtkey: " << routingKey.toString(2).substr(0, 8)
                  << " ThisNode: " << thisNode.getKey().toString(2).substr(0, 8);
        if (result->size() > 0) {
            std::cout << " next hop: " << (*result)[0].getKey().toString(2).substr(0, 8);
        } else {
            std::cout << " no next hop!";
        }
        std::cout << std::endl << std::endl;


    } else {
        // Right Shifting Lookup
        KeyDistanceComparator<KeyXorMetric>* comp = NULL;
            comp = new KeyDistanceComparator<KeyXorMetric>(key << (step - shiftingBits));

        result->setComparator(comp);
        rBucket[key.getBitRange(key.getLength() - step - 1,
                                shiftingBits)]->fillVector(result);
        result->add(thisNode);
        delete comp;

        std::cout << "key: " << key.toString(2).substr(0, 8)
                  << " dist: " << step << " (max: " << findNodeExt->getMaxDistance() << ")"
                  << " rtkey: " << (key >> step).toString(2).substr(0, 8)
                  << " ThisNode: " << thisNode.getKey().toString(2).substr(0, 8);
        if (result->size() > 0) {
            std::cout << " next hop: " << (*result)[0].getKey().toString(2).substr(0, 8);
        } else {
            std::cout << " no next hop!";
        }
        std::cout << std::endl << std::endl;

    }

    return result;
}
#endif

NodeVector* Broose::findNode(const OverlayKey& key,
                             int numRedundantNodes,
                             int numSiblings,
                             BaseOverlayMessage* msg)
{
    if ((state == INIT) || (state == RSET) || (state == FAILED))
        return new NodeVector();

    BrooseFindNodeExtMessage *findNodeExt = NULL;
    bool err;
    bool isSibling = isSiblingFor(thisNode, key, numSiblings, &err);
    int resultSize;

    if (numSiblings < 0) {
        // exhaustive iterative doesn't care about siblings
        resultSize = numRedundantNodes;
    } else {
        resultSize = isSibling ? (numSiblings ? numSiblings : 1)
                                                      : numRedundantNodes;
    }
    assert(numSiblings || numRedundantNodes);
    NodeVector* result = new NodeVector(resultSize);

    if (isSibling) {
        //return the closest nodes
        // sort with XOR distance to key
        KeyDistanceComparator<KeyXorMetric>* comp =
            new KeyDistanceComparator<KeyXorMetric>(key);
        result->setComparator(comp);

        bBucket->fillVector(result);
        result->add(thisNode);

        delete comp;

        /*
        std::cout << "key: " << key.toString(2).substr(0, 8)
                  << " ThisNode: " << thisNode.getKey().toString(2).substr(0, 8);
        if (result->size() > 0) {
            std::cout << " next hop (final): " << (*result)[0].getKey().toString(2).substr(0, 8);
        } else {
            std::cout << " no next hop! (final)";
        }
        std::cout << std::endl << std::endl;
        */

        return result;
    }

    if (msg != NULL) {
        if (!msg->hasObject("findNodeExt")) {
            findNodeExt = new BrooseFindNodeExtMessage("findNodeExt");

            OverlayKey routeKey = thisNode.getKey();
            // estimate distance
            int dist = max(rBucket[0]->longestPrefix(),
                           rBucket[1]->longestPrefix()) + 1 + userDist;

            if ((dist % shiftingBits) != 0)
                dist += (shiftingBits - (dist % shiftingBits));

            if (dist > keyLength) {
                if ((keyLength % shiftingBits) == 0) {
                    dist = keyLength;
                } else {
                    dist = (keyLength - keyLength % shiftingBits);
                }
            }

            if ((chooseLookup++) % 2 == 0) {
                // init left shifting lookup
                findNodeExt->setRightShifting(false);

                int prefix = 0;
                for (int i = 0; i < dist; i++) {
                    prefix += thisNode.getKey().getBit(thisNode.getKey().getLength() - i - 1) << (dist - i - 1);
                }

                OverlayKey pre(prefix);
                routeKey = key >> dist;
                routeKey += (pre << key.getLength() - dist);

                dist = -dist;
            } else {
                // init right shifting lookup
                findNodeExt->setRightShifting(true);
            }

            //add contact for next Hop
            findNodeExt->setLastNode(thisNode);
            findNodeExt->setRouteKey(routeKey);
            findNodeExt->setStep(dist);
            findNodeExt->setBitLength(BROOSEFINDNODEEXTMESSAGE_L);

            msg->addObject( findNodeExt );
        }

        findNodeExt = (BrooseFindNodeExtMessage*) msg->getObject("findNodeExt");
    }

    // update buckets with last hop
    addNode(findNodeExt->getLastNode());
    setLastSeen(findNodeExt->getLastNode());

    // replace last hop contact information with
    // this hop contact information
    findNodeExt->setLastNode(thisNode);

    // brother lookup
    if (findNodeExt->getStep() == 0) {
        // return the closest nodes sorted by XOR distance to key
        KeyDistanceComparator<KeyXorMetric>* comp =
            new KeyDistanceComparator<KeyXorMetric>(key);
        result->setComparator(comp);

        bBucket->fillVector(result);
        result->add(thisNode);

        delete comp;
        return result;
    }

    if (findNodeExt->getRightShifting() == false) {
        // Left Shifting Lookup

        // can't handle left shifting lookup in BSET-State
        if (state == BSET)
            return result;

        // calculate routing key
        findNodeExt->setRouteKey((findNodeExt->getRouteKey()) << shiftingBits);
        findNodeExt->setStep(findNodeExt->getStep() + shiftingBits);

        KeyDistanceComparator<KeyXorMetric>* comp = NULL;
        comp = new KeyDistanceComparator<KeyXorMetric>(
                findNodeExt->getRouteKey());

        result->setComparator(comp);
        lBucket->fillVector(result);
        result->add(thisNode);
        delete comp;
        /*
        std::cout << "key: " << key.toString(2).substr(0, 8)
                  << " dist: " << findNodeExt->getStep()
                  << " rtkey: " << findNodeExt->getRouteKey().toString(2).substr(0, 8)
                  << " ThisNode: " << thisNode.getKey().toString(2).substr(0, 8);
        if (result->size() > 0) {
            std::cout << " next hop: " << (*result)[0].getKey().toString(2).substr(0, 8);
        } else {
            std::cout << " no next hop!";
        }
        std::cout << std::endl << std::endl;
        */

    } else {
        // Right Shifting Lookup


        // calculate routing key
        int prefix = 0;
        int dist = findNodeExt->getStep();
        OverlayKey routeKey = findNodeExt->getRouteKey() >> shiftingBits;
        for (int i = 0; i < shiftingBits; i++)
            prefix += ((int)key.getBit(key.getLength() - dist + i) << i);
        OverlayKey pre(prefix);
        routeKey += (pre << (routeKey.getLength()-shiftingBits));

        findNodeExt->setRouteKey(routeKey);
        findNodeExt->setStep(dist - shiftingBits);

        KeyDistanceComparator<KeyXorMetric>* comp = NULL;
        comp = new KeyDistanceComparator<KeyXorMetric>(routeKey);

        result->setComparator(comp);
        rBucket[prefix]->fillVector(result);
        result->add(thisNode);
        delete comp;
        /*
        std::cout << "key: " << key.toString(2).substr(0, 8)
                  << " dist: " << findNodeExt->getStep()
                  << " rtkey: " << findNodeExt->getRouteKey().toString(2).substr(0, 8)
                  << " ThisNode: " << thisNode.getKey().toString(2).substr(0, 8);
        if (result->size() > 0) {
            std::cout << " next hop: " << (*result)[0].getKey().toString(2).substr(0, 8);
        } else {
            std::cout << " no next hop!";
        }
        std::cout << std::endl << std::endl;
        */
    }

    if ((*result)[0] == thisNode) {
        delete result;
        return (findNode(key, numRedundantNodes, numSiblings, msg));
    } else
        return result;
}

void Broose::finishOverlay()
{
    // store statistics
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;

    globalStatistics->addStdDev("Broose: Number of non-routable packets/s", numFailedPackets / time);
    globalStatistics->addStdDev("Broose: Sent BUCKET Messages/s", bucketCount / time);
    globalStatistics->addStdDev("Broose: Sent BUCKET Byte/s", bucketBytesSent / time);
    globalStatistics->addStdDev("Broose: Bucket retries at join", bucketRetries);

}

void Broose::recordOverlaySentStats(BaseOverlayMessage* msg)
{
    BaseOverlayMessage* innerMsg = msg;
    while (innerMsg->getType() != APPDATA &&
           innerMsg->getEncapsulatedPacket() != NULL) {
        innerMsg =
            static_cast<BaseOverlayMessage*>(innerMsg->getEncapsulatedPacket());
    }

    switch (innerMsg->getType()) {
    case RPC:
        if ((dynamic_cast<BucketCall*>(innerMsg) != NULL) ||
                (dynamic_cast<BucketResponse*>(innerMsg) != NULL)) {
            RECORD_STATS(bucketCount++; bucketBytesSent +=
                             msg->getByteLength());
        }
        break;
    }
}

void Broose::displayBucketState()
{
    EV << "[Broose::displayBucketState() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]" << endl;

    for (int i = 0; i < powShiftingBits; i++) {
        EV << "    Content of rBucket[" << i << "]: ";
        rBucket[i]->output();
    }

    EV << "    Content of lBucket: ";
    lBucket->output();
    EV << "    Content of bBucket: ";
    bBucket->output();
    EV << endl;
}


bool Broose::isSiblingFor(const NodeHandle& node,
                          const OverlayKey& key,
                          int numSiblings,
                          bool* err)
{
// TODO: node != thisNode doesn't work yet
    if (key.isUnspecified())
        error("Broose::isSiblingFor(): key is unspecified!");

    if (node != thisNode)
        error("Broose::isSiblingsFor(): "
              "node != thisNode is not implemented!");

    if (numSiblings > getMaxNumSiblings()) {
        opp_error("Broose::isSiblingFor(): numSiblings too big!");
    }
    // set default number of siblings to consider
    if (numSiblings == -1) numSiblings = getMaxNumSiblings();

    if (numSiblings == 0) {
        *err = false;
        return (node.getKey() == key);
    }

    if (state != READY) {
        *err = true;
        return false;
    }

    // TODO: handle numSibling parameter
    return bBucket->keyInRange(key);
}

void Broose::updateTooltip()
{
    if (ev.isGUI()) {
        std::stringstream ttString;

        // show our ip and key in tooltip
        ttString << thisNode.getIp() << " " << thisNode.getKey();

        getParentModule()->getParentModule()->getDisplayString().
                                  setTagArg("tt", 0, ttString.str().c_str());
        getParentModule()->getDisplayString().
                                  setTagArg("tt", 0, ttString.str().c_str());
        getDisplayString().setTagArg("tt", 0, ttString.str().c_str());

    }
}

bool Broose::handleRpcCall(BaseCallMessage* msg)
{
    if (state == BSET || state == READY) {
        // delegate messages
        RPC_SWITCH_START(msg)
        RPC_DELEGATE(Bucket, handleBucketRequestRpc);
        RPC_ON_CALL(Ping) {
            // add pinging node to all buckets and update lastSeen of node
            routingAdd(msg->getSrcNode(), true);
            return false;
            break;
        }
        RPC_ON_CALL(FindNode) {
            // add pinging node to all buckets and update lastSeen of node
            routingAdd(msg->getSrcNode(), true);
            return false;
            break;
        }
        RPC_SWITCH_END()
        return RPC_HANDLED;
    } else {
        RPC_SWITCH_START(msg)
        // don't answer PING and FIND_NODE calls, if the node can't route yet
        RPC_ON_CALL(Ping) {
            delete msg;
            return true;
            break;
        }
        RPC_ON_CALL(FindNode) {
            delete msg;
            return true;
            break;
        }
        RPC_SWITCH_END()
        return RPC_HANDLED;
    }
}

void Broose::handleRpcResponse(BaseResponseMessage* msg,
                               const RpcState& rpcState,
                               simtime_t rtt)
{
    // add sender to all buckets and update lastSeen of node
    routingAdd(msg->getSrcNode(), true, rtt);

    RPC_SWITCH_START(msg)
    RPC_ON_RESPONSE( Bucket ) {
        handleBucketResponseRpc(_BucketResponse, rpcState);
        EV << "[Broose::handleRpcResponse() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Bucket RPC Response received: id=" << rpcState.getId() << "\n"
           << "    msg=" << *_BucketResponse << " rtt=" << rtt
           << endl;
        break;
    }
    RPC_ON_RESPONSE(FindNode)
    {
        // add inactive nodes
        for (uint32_t i=0; i<_FindNodeResponse->getClosestNodesArraySize(); i++)
            routingAdd(_FindNodeResponse->getClosestNodes(i), false);
        break;
    }
    RPC_SWITCH_END( )
}

void Broose::handleRpcTimeout(const RpcState& rpcState)
{
    RPC_SWITCH_START(rpcState.getCallMsg())
    RPC_ON_CALL(FindNode) {
        handleFindNodeTimeout(_FindNodeCall, rpcState.getDest(), rpcState.getDestKey());
        EV << "[Broose::handleRpcTimeout() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Find Node RPC Call timed out: id=" << rpcState.getId() << "\n"
        << "    msg=" << *_FindNodeCall
        << endl;
        break;
    }
    RPC_ON_CALL(Bucket) {
        handleBucketTimeout(_BucketCall);
        EV << "[Broose::handleRpcTimeout() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Bucket RPC Call timed out: id=" << rpcState.getId() << "\n"
        << "    msg=" << *_BucketCall
        << endl;
        break;
    }
    RPC_SWITCH_END()
}

void Broose::handleBucketRequestRpc(BucketCall* msg)
{
    if (msg->getBucketType() == LEFT) {
        // TODO: dependent on the churn scenarios this may give better
        //       or worse results
        if (stab1 && (state == BSET)) {
            // can't handle LBucketRequest in BSET-State
            delete msg;
            return;
        }

        // return L-Bucket
        int size = lBucket->getSize();
        BucketResponse* bResponse = new BucketResponse("LBucketResponse");
        bResponse->setNodesArraySize(size);

        for (int i = 0; i < size; i++) {
            bResponse->setNodes(i, lBucket->get(i));
        }

        bResponse->setBitLength(BUCKETRESPONSE_L(bResponse));

        // only add, if the originator is already in the BSET state
        // in which the node already is able to do right shifting lookups
        // TODO: this leads to lower lookup success rates in some scenarios
        //       but helps to prevent deadlock situations with high churn rates
        if (stab2 || (msg->getProState() == PBSET)) {
            routingAdd(msg->getSrcNode(), true);
        }

        sendRpcResponse(msg, bResponse);
    } else if (msg->getBucketType() == BROTHER) {
        // return B-Bucket
        int size = bBucket->getSize();
        BucketResponse* bResponse = new BucketResponse("BBucketResponse");
        bResponse->setNodesArraySize(size);

        for (int i = 0; i < size; i++) {
            bResponse->setNodes(i, bBucket->get(i));
        }
        bResponse->setBitLength(BUCKETRESPONSE_L(bResponse));

        sendRpcResponse(msg, bResponse);
    } else
        error("Broose::handleBucketRequestRpc() - Wrong Bucket Type!");
}

void Broose::handleBucketResponseRpc(BucketResponse* msg,
                                     const RpcState& rpcState)
{
    BucketCall* call = check_and_cast<BucketCall*>(rpcState.getCallMsg());

    for (uint i = 0; i < msg->getNodesArraySize(); i++) {
        routingAdd(msg->getNodes(i), false);
    }

    if (call->getBucketType() == LEFT) {
        switch (state) {
        case RSET:
            if (call->getProState() == PRSET) {
                receivedBBucketLookup++;

                if (receivedBBucketLookup == numberBBucketLookup)
                    changeState(BSET);
            }
            break;
        case BSET:
            if (call->getProState() == PBSET) {
                receivedLBucketLookup++;

                if (receivedLBucketLookup == numberLBucketLookup)
                    changeState(READY);
            }
            break;
        default:
            break;
        }
    } else if (call->getBucketType() == BROTHER) {
        switch(state) {
        case INIT:
            if (call->getProState() == PINIT) {
                receivedJoinResponse++;
                if (receivedJoinResponse == powShiftingBits)
                    changeState(RSET);
            }
        default:
            break;
        }
    } else
        error("Broose::handleBucketRequestRpc() - unknown error.");
}


void Broose::handleBucketTimeout(BucketCall* msg)
{
    if (state == READY)
        return;
    else {
        bucketRetries++;
        changeState(INIT);
    }
}

void Broose::pingResponse(PingResponse* pingResponse, cPolymorphic* context,
                          int rpcId, simtime_t rtt) {
    // if node respond reset failedResponses and add lastSeen to node
    routingAdd(pingResponse->getSrcNode(), true, rtt);
}

void Broose::routingTimeout(const BrooseHandle& handle)
{
    for (size_t i = 0; i < bucketVector.size(); i++) {
        if (bucketVector[i]->getFailedResponses(handle) == numberRetries)
            bucketVector[i]->remove(handle);
        else
            bucketVector[i]->increaseFailedResponses(handle);
    }
    // TODO: if we loose the last node (despite ourself) from the
    //       B bucket, we should call join() to rejoin the network
}

void Broose::handleFindNodeTimeout(FindNodeCall* findNode,
                                   const TransportAddress& dest,
                                   const OverlayKey& destKey)
{
    routingTimeout(dynamic_cast<const NodeHandle&>(dest));
}

void Broose::pingTimeout(PingCall* pingCall,
                        const TransportAddress& dest,
                        cPolymorphic* context, int rpcId)
{
    routingTimeout(dynamic_cast<const NodeHandle&>(dest));
}

bool Broose::routingAdd(const NodeHandle& node, bool isAlive,
                        simtime_t rtt)
{
    bool added = false;

    for (size_t i = 0; i < bucketVector.size(); i++) {
        added |= bucketVector[i]->add(node, isAlive, rtt);
    }

    return added;
}

void Broose::setLastSeen(const NodeHandle& node)
{
    for (size_t i = 0; i < bucketVector.size(); i++) {
        bucketVector[i]->setLastSeen(node, simTime());
    }
}

void Broose::addNode(const NodeHandle& node)
{
    // add node to all buckets
    for (size_t i = 0; i < bucketVector.size(); i++) {
        bucketVector[i]->add(node);
    }
}

void Broose::resetFailedResponses(const NodeHandle& node)
{
    for (size_t i = 0; i < bucketVector.size(); i++) {
        bucketVector[i]->resetFailedResponses(node);
    }
}

void Broose::setRTT(const NodeHandle& node, simtime_t rtt)
{
    for (size_t i = 0; i < bucketVector.size(); i++) {
        bucketVector[i]->setRTT(node, rtt);
    }
}


