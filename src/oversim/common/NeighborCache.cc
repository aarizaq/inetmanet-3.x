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
 * @file NeighborCache.cc
 * @author Antonio Zea
 * @author Bernhard Heep
 */

#include <cassert>

#include <TransportAddress.h>
#include <NodeHandle.h>
#include <PeerInfo.h>
#include <GlobalStatisticsAccess.h>
#include <CoordMessages_m.h>
#include <GlobalNodeListAccess.h>
#include <hashWatch.h>

#include "NeighborCache.h"


const std::vector<double> NeighborCache::coordsDummy;

std::ostream& operator<<(std::ostream& os,
                         const NeighborCache::NeighborCacheEntry& entry)
{
    if (entry.rttState == NeighborCache::RTTSTATE_VALID) {
        os << entry.rtt;
    } else {
        if (entry.rttState == NeighborCache::RTTSTATE_TIMEOUT) os << "TIMEOUT";
        else if (entry.rttState == NeighborCache::RTTSTATE_UNKNOWN) os << "UNKNOWN";
        else if (entry.rttState == NeighborCache::RTTSTATE_WAITING) os << "WAITING";
    }
    os << " (inserted: " << entry.insertTime;

    os << ", #contexts: "
       << entry.waitingContexts.size();

    if (!entry.nodeRef.isUnspecified()) os <<  ", <KEY>";

    //TODO entry.coordsInfo

    os << ")";

    return os;
}


Define_Module(NeighborCache);

void NeighborCache::initializeApp(int stage)
{
    if (stage == MAX_STAGE_COMPONENTS) {
        neighborCache.clear();
        WATCH_UNORDERED_MAP(neighborCache);

        enableNeighborCache = par("enableNeighborCache");
        rttExpirationTime = par("rttExpirationTime");
        maxSize = par("maxSize");
        doDiscovery = par("doDiscovery");
        ncsSendBackOwnCoords = par("ncsSendBackOwnCoords");

        // set default query types
        std::string temp = par("defaultQueryType").stdstringValue();
        if (temp == "exact")
            defaultQueryType = NEIGHBORCACHE_EXACT;
        else if (temp == "exact_timeout")
            defaultQueryType = NEIGHBORCACHE_EXACT_TIMEOUT;
        else if (temp == "available")
            defaultQueryType = NEIGHBORCACHE_AVAILABLE;
        else if (temp == "estimated")
            defaultQueryType = NEIGHBORCACHE_ESTIMATED;
        else throw cRuntimeError((std::string("Wrong query type: ")
            + temp).c_str());

        temp = par("defaultQueryTypeI").stdstringValue();
        if (temp == "available")
            defaultQueryTypeI = NEIGHBORCACHE_AVAILABLE;
        else if (temp == "estimated")
            defaultQueryTypeI = NEIGHBORCACHE_ESTIMATED;
        else throw cRuntimeError((std::string("Wrong query type (I): ")
            + temp).c_str());

        temp = par("defaultQueryTypeQ").stdstringValue();
        if (temp == "exact")
            defaultQueryTypeQ = NEIGHBORCACHE_EXACT;
        else if (temp == "exact_timeout")
            defaultQueryTypeQ = NEIGHBORCACHE_EXACT_TIMEOUT;
        else if (temp == "query")
            defaultQueryTypeQ = NEIGHBORCACHE_QUERY;
        else throw cRuntimeError((std::string("Wrong query type (Q): ")
        + temp).c_str());

        temp = par("ncsType").stdstringValue();
        if (temp == "none") ncs = NULL;
        else if (temp == "vivaldi") ncs = new Vivaldi();
        else if (temp == "svivaldi") ncs = new SVivaldi();
        else if (temp == "gnp") ncs = new Nps(); //TODO
        else if (temp == "nps") ncs = new Nps();
        else if (temp == "simple") ncs = new SimpleNcs();
        else throw cRuntimeError((std::string("Wrong NCS type: ")
            + temp).c_str());

        globalStatistics = GlobalStatisticsAccess().get();

        misses = 0;
        hits = 0;

        rttHistory = par("rttHistory");
        timeoutAccuracyLimit = par("timeoutAccuracyLimit");

        numMsg = 0;
        absoluteError = 0.0;
        relativeError = 0.0;
        numRttErrorToHigh = 0;
        numRttErrorToLow = 0;
        lastAbsoluteErrorPerNode.clear();
        WATCH(absoluteError);
        WATCH(relativeError);
        WATCH(numMsg);
    } else if (stage == MIN_STAGE_TIER_1) {
        if (ncs) ncs->init(this);
    }
}


void NeighborCache::finishApp()
{
    if ((misses + hits) != 0) {
        globalStatistics
            ->addStdDev("NeighborCache: Ping hit rate",
                        ((double)hits / (double)(misses + hits)));
    }

    if (ncs && numMsg > 0) {
        globalStatistics->addStdDev("NeighborCache: NCS absolute RTT error",
                                    absoluteError / (double)numMsg);
        globalStatistics->addStdDev("NeighborCache: NCS relative RTT error",
                                    relativeError / (double)numMsg);
        globalStatistics->addStdDev("NeighborCache: number of messages/s",
                                    numMsg / SIMTIME_DBL(simTime() - creationTime));
        globalStatistics->addStdDev("NeighborCache: NCS percentage of RTT errors to high",
                                    (double)numRttErrorToHigh / (double)numMsg);
        globalStatistics->addStdDev("NeighborCache: NCS percentage of RTT errors to low",
                                    (double)numRttErrorToLow / (double)numMsg);
    }
}


NeighborCache::~NeighborCache()
{
    delete ncs;
}

bool NeighborCache::insertNodeContext(const TransportAddress& handle,
                                     cPolymorphic* context,
                                     ProxListener* rpcListener,
                                     int rpcId)
{
    if (!enableNeighborCache) return false;
    if (neighborCache.count(handle) == 0) {
        NeighborCacheEntry& entry = neighborCache[handle];

        entry.insertTime = simTime();
        entry.rttState = RTTSTATE_WAITING;

        neighborCacheExpireMap.insert(std::make_pair(entry.insertTime,
                                                     handle));

        cleanupCache();

        assert(neighborCache.size() == neighborCacheExpireMap.size());
        return false;
    } else {
        NeighborCacheEntry& entry = neighborCache[handle];

        // waiting?
        if (entry.rttState == RTTSTATE_WAITING) {
            WaitingContext temp(rpcListener, context, rpcId);
            entry.waitingContexts.push_back(temp);

            return true;
        } else {
            if (entry.waitingContexts.size() > 0) {
                throw cRuntimeError("not waiting for response,"
                                    " but additional contexts found!");
            }

            updateEntry(handle, entry.insertTime);

            entry.rttState = RTTSTATE_WAITING;
            entry.insertTime = simTime();

            return false;
        }
    }
}


NeighborCache::WaitingContexts NeighborCache::getNodeContexts(const TransportAddress& handle)
{
    if (neighborCache.count(handle) == 0)
        throw cRuntimeError("NeighborCache error!");
    WaitingContexts temp = neighborCache[handle].waitingContexts;
    neighborCache[handle].waitingContexts.clear();

    return temp;
}


void NeighborCache::setNodeTimeout(const TransportAddress& handle)
{
    //if (!enableNeighborCache) return;

    if (neighborCache.count(handle) == 0) {
        NeighborCacheEntry& entry = neighborCache[handle];

        entry.insertTime = simTime();
        entry.rttState = RTTSTATE_TIMEOUT;

        neighborCacheExpireMap.insert(std::make_pair(entry.insertTime,
                                                     handle));
        cleanupCache();
    } else {
        NeighborCacheEntry& entry = neighborCache[handle];

        updateEntry(handle, entry.insertTime);

        entry.insertTime = simTime();
        entry.rttState = RTTSTATE_TIMEOUT;

        WaitingContexts waitingContexts = getNodeContexts(handle);

        for (uint32_t i = 0; i < waitingContexts.size(); ++i) {
            assert(waitingContexts[i].proxListener || !waitingContexts[i].proxContext);
            if (waitingContexts[i].proxListener) {
                waitingContexts[i].proxListener->proxCallback(handle,
                                                              waitingContexts[i].id,
                                                              waitingContexts[i].proxContext,
                                                              Prox::PROX_TIMEOUT);
            }
        }
    }
    assert(neighborCache.size() == neighborCacheExpireMap.size());
}


void NeighborCache::updateNode(const NodeHandle& add, simtime_t rtt,
                               const NodeHandle& srcRoute,
                               AbstractNcsNodeInfo* ncsInfo)
{
    Enter_Method_Silent();

    EV << "[NeighborCache::updateNode() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    inserting rtt(" << rtt << ") of node " << add.getIp()
           << endl;

    if (rtt <= 0) {
        delete ncsInfo;
        return; //TODO broose
    }

    bool deleteInfo = false;

    //if (enableNeighborCache) {
    if (neighborCache.count(add) == 0) {
        NeighborCacheEntry& entry = neighborCache[add];

        entry.insertTime = simTime();
        entry.rtt = rtt;
        entry.rttState = RTTSTATE_VALID;
        entry.nodeRef = add;
        entry.coordsInfo = ncsInfo;
        entry.lastRtts.push_back(rtt);

        neighborCacheExpireMap.insert(std::make_pair(entry.insertTime, add));

        cleanupCache();
    } else {
        updateEntry(add, neighborCache[add].insertTime);

        NeighborCacheEntry& entry = neighborCache[add];

        entry.insertTime = simTime();
        if (entry.rttState != RTTSTATE_VALID || entry.rtt > rtt)
            entry.rtt = rtt;
        entry.rttState = RTTSTATE_VALID;
        entry.nodeRef = add;

        entry.lastRtts.push_back(rtt);
        if (entry.lastRtts.size()  > rttHistory) {
            entry.lastRtts.pop_front();
        }

        if (ncsInfo) {
            if (entry.coordsInfo) {
                entry.coordsInfo->update(*ncsInfo);
                deleteInfo = true;
            } else {
                entry.coordsInfo = ncsInfo;
            }
        }

        WaitingContexts waitingContexts = getNodeContexts(add);

        for (uint32_t i = 0; i < waitingContexts.size(); ++i) {
            if (waitingContexts[i].proxListener) {
                waitingContexts[i].proxListener
                ->proxCallback(add,
                               waitingContexts[i].id,
                               waitingContexts[i].proxContext,
                               Prox(rtt, 1));
            }
        }
    }
    assert(neighborCache.size() == neighborCacheExpireMap.size());

    calcRttError(add, rtt);

    if (ncs) ncs->processCoordinates(rtt, *ncsInfo);

    // delete ncsInfo if old info is used
    if (deleteInfo) delete ncsInfo;
}


void NeighborCache::updateNcsInfo(const TransportAddress& node,
                                  AbstractNcsNodeInfo* ncsInfo)
{
    Enter_Method_Silent();

    EV << "[NeighborCache::updateNcsInfo() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]\n"
       << "    inserting new NcsInfo of node " << node.getIp()
       << endl;

    if (neighborCache.count(node) == 0) {
        NeighborCacheEntry& entry = neighborCache[node];

        entry.insertTime = simTime();
        entry.coordsInfo = ncsInfo;

        neighborCacheExpireMap.insert(std::make_pair(entry.insertTime, node));

        cleanupCache();
    } else {
        updateEntry(node, neighborCache[node].insertTime);

        NeighborCacheEntry& entry = neighborCache[node];

        if (ncsInfo) {
            if (entry.coordsInfo) {
                entry.coordsInfo->update(*ncsInfo);
                delete ncsInfo;
            } else {
                entry.coordsInfo = ncsInfo;
            }
        }
    }
}


NeighborCache::Rtt NeighborCache::getNodeRtt(const TransportAddress &add)
{
    // cache disabled or entry not there
    if (!enableNeighborCache ||
        add.isUnspecified() ||
        (neighborCache.count(add) == 0)) {
        misses++;
        return std::make_pair(0.0, RTTSTATE_UNKNOWN);
    }

    NeighborCacheEntry &entry = neighborCache[add];

    if (entry.rttState == RTTSTATE_WAITING ||
        entry.rttState == RTTSTATE_UNKNOWN)
        return std::make_pair(entry.rtt, entry.rttState);
    // entry expired
    if ((simTime() - entry.insertTime) >= rttExpirationTime) {
        entry.rttState = RTTSTATE_UNKNOWN;
        return std::make_pair(entry.rtt, RTTSTATE_UNKNOWN);
    }
    hits++;
    assert(!(entry.rtt == 0.0 && entry.rttState == RTTSTATE_VALID));
    return std::make_pair(entry.rtt, entry.rttState);
}


const NodeHandle& NeighborCache::getNodeHandle(const TransportAddress &add)
{
    if (neighborCache.count(add) == 0) {
        throw cRuntimeError("NeighborCache.cc: getNodeHandle was asked for "
                            "a non-existent node reference.");
    }
    return neighborCache[add].nodeRef;
}


bool NeighborCache::cleanupCache()
{
    bool result = false;
    uint32_t size = neighborCache.size();

    if (size > maxSize) {
        neighborCacheExpireMapIterator it;
        for (uint32_t i = 0; i < (size - (maxSize / 2)); ++i) {
            it = neighborCacheExpireMap.begin();
            if ((neighborCache[it->second].rttState == RTTSTATE_WAITING) ||
                (neighborCache[it->second].insertTime == simTime())) {
                break;
            }
            neighborCache.erase(it->second);
            neighborCacheExpireMap.erase(it);
            result = true;
        }
    }
    assert(neighborCache.size() == neighborCacheExpireMap.size());
    return result;
}


void NeighborCache::updateEntry(const TransportAddress& address,
                                simtime_t insertTime)
{
    neighborCacheExpireMapIterator it =
        neighborCacheExpireMap.lower_bound(insertTime);
    while (it->second != address) ++it;
    neighborCacheExpireMap.erase(it);
    neighborCacheExpireMap.insert(std::make_pair(simTime(),
                                                 address));
    assert(neighborCache.size() == neighborCacheExpireMap.size());
}

//TODO
TransportAddress NeighborCache::getNearestNode(uint8_t maxLayer)
{
    TransportAddress nearestNode = TransportAddress::UNSPECIFIED_NODE;
    simtime_t nearestNodeRtt = MAXTIME;
    NeighborCacheIterator it;

    for(it = neighborCache.begin(); it != neighborCache.end(); it++ ) {
        if (it->second.rtt < nearestNodeRtt &&
            it->second.rtt > 0 /*&&
            it->second.coordsInfo.npsLayer < maxLayer+1 &&
            it->second.coordsInfo.npsLayer > 0*/) {
            nearestNode.setIp(it->first.getIp());
            nearestNodeRtt = it->second.rtt;
            nearestNode.setPort(it->second.nodeRef.getPort());
        }
    }

    return nearestNode;
}


bool NeighborCache::isEntry(const TransportAddress &node)
{
    if (neighborCache.count(node) > 0) return true;
    return false;
}


// Vivaldi stuff
double NeighborCache::getAvgAbsPredictionError()
{
    /*
    //TODO retain and consider the last measured RTTs not the last error(s)
    double absoluteDiff = 0.0;
    uint32_t numNeighbors = 0;

    for (std::map<TransportAddress, std::vector<double> >::iterator it =
        lastAbsoluteErrorPerNode.begin(); it != lastAbsoluteErrorPerNode.end();
        it++) {
        double tempAbsoluteDiff = 0.0;
        for (uint32_t i = 0; i < it->second.size(); i++) {
            tempAbsoluteDiff += it->second.at(i);
        }
        absoluteDiff += (tempAbsoluteDiff / it->second.size());
        numNeighbors++;
    }

    absoluteDiff /= numNeighbors;
    return (absoluteDiff > 1.0) ? 1.0 : absoluteDiff;
     */

    // old version
    //if (neighborCache.size() < 2 || sampleSize == 0) return 1.0;

    double absoluteDiff = 0;
    uint32_t numNeighbors = 0;
    uint32_t sampleSize = 32; //test

    for (std::map<simtime_t, TransportAddress>::reverse_iterator it =
         neighborCacheExpireMap.rbegin();
         it != neighborCacheExpireMap.rend() &&
         numNeighbors < sampleSize; ++it) {
        NeighborCacheEntry& cacheEntry = neighborCache[it->second];

        double dist = ncs->getOwnNcsInfo().getDistance(*cacheEntry.coordsInfo);

        if (dist != 0 && cacheEntry.rttState == RTTSTATE_VALID) {
            double predictionError = fabs(dist - SIMTIME_DBL(cacheEntry.rtt));

            //test: error weighted
            //if (it->second.coordErr < 1) {
            //    predictionError /= it->second.coordErr;
            //}
            //test: age weighted
            //if ((simTime() - it->second.insertTime) > 1) {
            //    predictionError /= (simTime() - it->second.insertTime);
            //}

            numNeighbors++;
            absoluteDiff += predictionError;
        }
    }
    assert(numNeighbors != 0);
    absoluteDiff /= numNeighbors;

    return (absoluteDiff > 1.0) ? 1.0 : absoluteDiff;
}


void NeighborCache::handleReadyMessage(CompReadyMessage* readyMsg)
{
    if (readyMsg->getReady() && readyMsg->getComp() == BOOTSTRAPLIST_COMP) {
        if (doDiscovery) {
            //TODO
            // 1. ask bootstrap node for other nodes and his coordinates
            // 2. probe other nodes and optionally ask them for more nodes
            //    (try to get close as well as distant nodes)
            // sendReadyMessage();
        } else {
            sendReadyMessage();
        }
    }
    delete readyMsg;
}


void NeighborCache::handleTimerEvent(cMessage* msg)
{
    if (ncs) {
        ncs->handleTimerEvent(msg);
    }
}


bool NeighborCache::handleRpcCall(BaseCallMessage* msg)
{
    if (ncs) {
        return ncs->handleRpcCall(msg);
    }

    return false;
}


// Prox stuff
Prox NeighborCache::getProx(const TransportAddress &node,
                              NeighborCacheQueryType type,
                              int rpcId,
                              ProxListener *listener,
                              cPolymorphic *contextPointer)
{
    Enter_Method("getProx()");

    if (!enableNeighborCache) {
        queryProx(node, rpcId, listener, contextPointer);
        return Prox::PROX_UNKNOWN;
    }

    if (node == overlay->getThisNode()) {
        delete contextPointer;
        return Prox::PROX_SELF;
    }

    bool sendQuery = false;
    Prox result = Prox::PROX_UNKNOWN;
    Rtt rtt = getNodeRtt(node);

    //countGetProxTotal++;
    if (type == NEIGHBORCACHE_DEFAULT) type = defaultQueryType;
    else if (type == NEIGHBORCACHE_DEFAULT_IMMEDIATELY) type = defaultQueryTypeI;
    else if (type == NEIGHBORCACHE_DEFAULT_QUERY) type = defaultQueryTypeQ;

    switch(type) {
        case NEIGHBORCACHE_EXACT:
            if (rtt.second == RTTSTATE_TIMEOUT) {
                // if timeout, return unknown, and send a query!
                sendQuery = true;
            } else if (rtt.second == RTTSTATE_WAITING) {
                // if a query was sent, return UNKNOWN
                sendQuery = true; //just inserting a context, no real ping is sent
            } else if (rtt.second == RTTSTATE_UNKNOWN) {
                // if no entry known, send a query and return UNKNOWN
                sendQuery = true;
            } else {
                // else, return whatever we have
                result = rtt.first;
            }
            break;
        case NEIGHBORCACHE_EXACT_TIMEOUT:
            if (rtt.second == RTTSTATE_TIMEOUT) {
                // if timeout, return that
                result = Prox::PROX_TIMEOUT;
            } else if (rtt.second == RTTSTATE_WAITING) {
                // if a query was sent, return UNKNOWN
                sendQuery = true; //just inserting a context, no real ping is sent
            } else if (rtt.second == RTTSTATE_UNKNOWN) {
                // if no entry known, send a query and return UNKNOWN
                sendQuery = true;
            } else {
                // else, return whatever we have
                result = rtt.first;
            }
            break;
        case NEIGHBORCACHE_ESTIMATED:
            if (rtt.second == RTTSTATE_TIMEOUT) {
                // if timeout, return that
                result = Prox::PROX_TIMEOUT;
            } else if (rtt.second == RTTSTATE_WAITING) {
                // if a query was sent, return an estimate
                result = estimateProx(node);
            } else if (rtt.second == RTTSTATE_UNKNOWN) {
                // if no entry known, return an estimate
                result = estimateProx(node);
            } else {
                // else return whatever we have
                result = rtt.first;
            }
            break;
        case NEIGHBORCACHE_AVAILABLE:
            if (rtt.second == RTTSTATE_TIMEOUT) {
                // if timeout, return that.
                result = Prox::PROX_TIMEOUT;
            } else if ((rtt.second == RTTSTATE_WAITING) ||
                       (rtt.second == RTTSTATE_UNKNOWN)) {
                // if a query was sent or entry unknown, return UNKNOWN
            } else {
                // else return what we have
                result = rtt.first;
            }
            break;
        case NEIGHBORCACHE_QUERY:
            // simply send a query and return UNKNOWN
            sendQuery = true;
            break;
        default:
            throw cRuntimeError("Unknown query type!");
            break;

    }
    if (sendQuery) {
        if (!insertNodeContext(node, contextPointer, listener, rpcId)) {
            queryProx(node, rpcId, listener, contextPointer);
        }
    } else delete contextPointer;

    return result;
}

Prox NeighborCache::estimateProx(const TransportAddress &node)
{
    Enter_Method("estimateProx()");

    Rtt rtt = getNodeRtt(node);

    if (rtt.second != RTTSTATE_UNKNOWN) return rtt.first;

    if (ncs && neighborCache.count(node)) {
        return getCoordinateBasedProx(node);
    }

    return Prox::PROX_UNKNOWN;
}

void NeighborCache::queryProx(const TransportAddress &node,
                              int rpcId,
                              ProxListener *listener,
                              cPolymorphic *contextPointer)
{
    Enter_Method("queryProx()");

    WaitingContext temp(listener, contextPointer, rpcId);

    if (neighborCache.count(node) == 0) {
        NeighborCacheEntry& entry = neighborCache[node];

        entry.waitingContexts.push_back(temp);
        neighborCacheExpireMap.insert(std::make_pair(entry.insertTime, node));
        cleanupCache();
    } else {
        NeighborCacheEntry& entry = neighborCache[node];
        entry.waitingContexts.push_back(temp);
    }
    assert(neighborCache.size() == neighborCacheExpireMap.size());

    // TODO: this ping traffic is accounted application data traffic!
    pingNode(node, -1, 0, NULL, "PING");
}

Prox NeighborCache::getCoordinateBasedProx(const TransportAddress& node)
{
    if (ncs && isEntry(node)) {
        const AbstractNcsNodeInfo* temp = getNodeCoordsInfo(node);
        if (temp) return ncs->getCoordinateBasedProx(*temp);
    }
    return Prox::PROX_UNKNOWN;
}


const AbstractNcsNodeInfo* NeighborCache::getNodeCoordsInfo(const TransportAddress &node)
{
    if (neighborCache.count(node) == 0) {
        throw cRuntimeError("NeighborCache.cc: getNodeCoords was asked for "
                            "a non-existent node reference.");
    }
    return neighborCache[node].coordsInfo;
}


void NeighborCache::calcRttError(const NodeHandle& handle, simtime_t rtt)
{
    if (!ncs) return;

    // Check if data collection can start
    if (!globalStatistics->isMeasuring()) return;

    Prox prox = getCoordinateBasedProx(handle);
    if (prox == Prox::PROX_UNKNOWN) return;

    //calculate absolute rtt error of the last message
    double tempRttError = prox.proximity - SIMTIME_DBL(rtt);

    if (tempRttError < 0){
        tempRttError *= -1;
        ++numRttErrorToLow;
    } else ++numRttErrorToHigh;

    numMsg++;
    absoluteError += tempRttError;
    relativeError += tempRttError / SIMTIME_DBL(rtt);

    globalStatistics->recordOutVector("NCS: absolute Rtt Error",
                                      tempRttError);
    globalStatistics->recordOutVector("NCS: relative Rtt Error",
                                      (tempRttError / SIMTIME_DBL(rtt)));
}


std::pair<simtime_t, simtime_t> NeighborCache::getMeanVarRtt(const TransportAddress &node,
                                                             bool returnVar)
{
    if (neighborCache.count(node) == 0) {
        throw cRuntimeError("NeighborCache.cc: getMeanVarRtt was asked for"
                            "a non-existent node reference.");
    }

    uint16_t size = neighborCache[node].lastRtts.size();
    if (size == 0) return std::make_pair(-1.0,-1.0);

    simtime_t rttSum = 0;
    for (int i = 0; i < size; i++){
        rttSum += neighborCache[node].lastRtts[i];
    }
    simtime_t meanRtt = rttSum / size;
    if (!returnVar) {
        return std::make_pair(meanRtt, -1.0);
    }
    if (size == 1) {
        return std::make_pair(meanRtt, 0.0);
    }

    double sum = 0.0;
    for (int i = 0; i < size; i++){
        simtime_t tempRtt = neighborCache[node].lastRtts.at(i) - meanRtt;
        sum += (SIMTIME_DBL(tempRtt) * SIMTIME_DBL(tempRtt));
    }

    return std::make_pair(meanRtt, (sum / size));
}


simtime_t NeighborCache::getNodeTimeout(const NodeHandle &node)
{
    simtime_t timeout = getRttBasedTimeout(node);
    if (timeout == -1 && ncs) return getNcsBasedTimeout(node);
    return timeout;
}


//Calculate timeout with RTT
simtime_t NeighborCache::getRttBasedTimeout(const NodeHandle &node)
{
    simtime_t timeout = -1;

    // check if an entry is available in NeighborCache
    if (isEntry(node)) {
        std::pair<simtime_t, simtime_t> temp = getMeanVarRtt(node, true);
        simtime_t meanRtt = temp.first;
        simtime_t varRtt = temp.second;

        // TODO return value even if node has timed out
        if (meanRtt == -1) return -1;
        if (varRtt > 0) {
            // like TCP
            timeout = meanRtt + 4 * varRtt;
        } else {
            // only one RTT is available
            timeout = meanRtt * 1.2;
        }
        // adjustment
        timeout *= RTT_TIMEOUT_ADJUSTMENT;
        //if (timeout > SIMTIME_DBL(defaultTimeout)) return -1;
    }
    return timeout;
}

//Calculate timeout with NCS
simtime_t NeighborCache::getNcsBasedTimeout(const NodeHandle &node)
{
    double timeout = -1;
    Prox prox = Prox::PROX_UNKNOWN;

    // check if an entry is available in NeighborCache
    if (isEntry(node)) {
        prox = getProx(node, NEIGHBORCACHE_ESTIMATED);

        if (prox != Prox::PROX_UNKNOWN  && prox != Prox::PROX_TIMEOUT &&
            prox.proximity > 0 && prox.accuracy > timeoutAccuracyLimit) {
            timeout = prox.proximity + (6 * (1 - prox.accuracy));
            timeout += NCS_TIMEOUT_CONSTANT;
        } else return -1;

        if (/*timeout > SIMTIME_DBL(defaultTimeout) ||*/ timeout < 0)
            return -1;
    }
    return timeout;
}

