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
 * @file GlobalNodeList.cc
 * @author Markus Mauch, Robert Palmer
 */

#include <iostream>

#include <omnetpp.h>

#include <NotificationBoard.h>
#include <BinaryValue.h>
#include <OverlayKey.h>
#include <PeerInfo.h>
#include <BaseOverlay.h>
#include <GlobalStatisticsAccess.h>
#include <hashWatch.h>
#include <BootstrapList.h>

#include "GlobalNodeList.h"

Define_Module(GlobalNodeList);

std::ostream& operator<<(std::ostream& os, const BootstrapEntry entry)
{
    NodeHandle* nodeHandle = dynamic_cast<NodeHandle*>(entry.node);

    os << "Address: " << entry.node->getIp()
       << " Port: " << entry.node->getPort();

    if (nodeHandle) {
        os << " NodeId: " << nodeHandle->getKey();
    }

    os << " ModuleID: "
       << entry.info->getModuleID() << " Bootstrapped: "
       << (entry.info->isBootstrapped() ? "true" : "false")
       << " NPS Layer: " << ((int) entry.info->getNpsLayer())
       << " TypeID: " << (entry.info->getTypeID());

    return os;
}

void GlobalNodeList::initialize()
{
    maxNumberOfKeys = par("maxNumberOfKeys");
    keyProbability = par("keyProbability");
    isKeyListInitialized = false;
    WATCH_UNORDERED_MAP(peerStorage.getPeerHashMap());
    WATCH_VECTOR(keyList);
    WATCH(landmarkPeerSize);

    landmarkPeerSize = 0;

    for (int i = 0; i < MAX_NODETYPES; i++) {
        landmarkPeerSizePerType[i] = 0;
    }

    preKilledNodes = 0;

    if (par("maliciousNodeChange")) {
        if ((double) par("maliciousNodeProbability") > 0)
            error("maliciousNodeProbability and maliciousNodeChange are not supported concurrently");

        cMessage* msg = new cMessage("maliciousNodeChange");
        scheduleAt(simTime() + (int) par("maliciousNodeChangeStartTime"), msg);
        maliciousNodesVector.setName("MaliciousNodeRate");
        maliciousNodesVector.record(0);
        maliciousNodeRatio = 0;
    }

    for (int i=0; i<MAX_NODETYPES; i++) {
        for (int j=0; j<MAX_NODETYPES; j++) {
            connectionMatrix[i][j] = true;
        }
    }

    globalStatistics = GlobalStatisticsAccess().get();

    cMessage* timer = new cMessage("oracleTimer");

    scheduleAt(simTime(), timer);
}

void GlobalNodeList::handleMessage(cMessage* msg)
{
    if (msg->isName("maliciousNodeChange")) {
        double newRatio = maliciousNodeRatio + (double) par("maliciousNodeChangeRate"); // ratio to obtain
        if (maliciousNodeRatio < (double) par("maliciousNodeChangeStartValue"))
            newRatio = (double) par("maliciousNodeChangeStartValue");

        if (newRatio < (double) par("maliciousNodeChangeStopValue")) // schedule next event
            scheduleAt(simTime() + (int) par("maliciousNodeChangeInterval"), msg);

        int nodesNeeded = (int) (((double) par("maliciousNodeChangeRate")) * peerStorage.size());

        EV << "[GlobalNodeList::handleMessage()]\n"
           << "    Changing " << nodesNeeded << " nodes to be malicious"
           << endl;

        for (int i = 0; i < nodesNeeded; i++) {
            // search a node that is not yet malicious
            NodeHandle node;
            do {
                node = getRandomNode(0, false);
            } while (isMalicious(node));

            setMalicious(node, true);
        }

        maliciousNodesVector.record(newRatio);
        maliciousNodeRatio = newRatio;

        return;
    }

    else if (msg->isName("oracleTimer")) {
        RECORD_STATS(globalStatistics->recordOutVector(
                     "GlobalNodeList: Number of nodes", peerStorage.size()));
        scheduleAt(simTime() + 50, msg);
    } else {
        opp_error("GlobalNodeList::handleMessage: Unknown message type!");
    }
}

const NodeHandle& GlobalNodeList::getBootstrapNode(const NodeHandle &node)
{
    uint32_t nodeType;
    PeerHashMap::iterator it;

    // always prefer boot node from the same TypeID
    // if there is no such node, go through all
    // connected partitions until a bootstrap node is found
    if (!node.isUnspecified()) {
        it = peerStorage.find(node.getIp());

        // this should never happen
        if (it == peerStorage.end()) {
           return getRandomNode(0, true);
        }

        nodeType = it->second.info->getTypeID();
        const NodeHandle &tempNode1 = getRandomNode(nodeType, true);

        if (tempNode1.isUnspecified()) {
            for (uint32_t i = 0; i < MAX_NODETYPES; i++) {
                if (i == nodeType)
                    continue;

                if (connectionMatrix[nodeType][i]) {
                    const NodeHandle &tempNode2 = getRandomNode(i, true);
                    if (!tempNode2.isUnspecified())
                        return tempNode2;
                }
            }
            return NodeHandle::UNSPECIFIED_NODE;
        } else {
            return tempNode1;
        }
    } else {
        return getRandomNode(0, true);
    }
}

const NodeHandle& GlobalNodeList::getRandomNode(int32_t nodeType,
                                                bool bootstrappedNeeded,
                                                bool inoffensiveNeeded)
{
    PeerHashMap::iterator it = peerStorage.getRandomNode(nodeType,
                                                         bootstrappedNeeded,
                                                         inoffensiveNeeded);
    if (it == peerStorage.end()) {
        return NodeHandle::UNSPECIFIED_NODE;
    }

    if (dynamic_cast<NodeHandle*>(it->second.node)) {
        return *dynamic_cast<NodeHandle*>(it->second.node);
    } else {
        return NodeHandle::UNSPECIFIED_NODE;
    }
}

void GlobalNodeList::sendNotificationToAllPeers(int category)
{
    PeerHashMap::iterator it;
    for (it = peerStorage.begin(); it != peerStorage.end(); it++) {
        NotificationBoard* nb = check_and_cast<NotificationBoard*>(
                simulation.getModule(it->second.info->getModuleID())
                ->getSubmodule("notificationBoard"));

        nb->fireChangeNotification(category);
    }
}

void GlobalNodeList::addPeer(const IPvXAddress& ip, PeerInfo* info)
{
    BootstrapEntry temp;
    temp.node = new TransportAddress(ip);
    temp.info = info;
    temp.info->setPreKilled(false);

    peerStorage.insert(std::make_pair(temp.node->getIp(), temp));

    if (uniform(0, 1) < (double) par("maliciousNodeProbability") ||
            (par("maliciousNodeChange") && uniform(0, 1) < maliciousNodeRatio)) {
        setMalicious(*temp.node, true);
    }

    if (peerStorage.size() == 1) {
        // we need at least one inoffensive bootstrap node
        setMalicious(*temp.node, false);
    }
}

void GlobalNodeList::registerPeer(const TransportAddress& peer)
{
    PeerHashMap::iterator it = peerStorage.find(peer.getIp());

    if (it == peerStorage.end()) {
        throw cRuntimeError("GlobalNodeList::registerPeer(): "
                "Peer is not in peer set");
    } else {
        delete it->second.node;
        it->second.node = new TransportAddress(peer);
        peerStorage.setBootstrapped(it, true);
    }
}

void GlobalNodeList::registerPeer(const NodeHandle& peer)
{
    PeerHashMap::iterator it = peerStorage.find(peer.getIp());

    if (it == peerStorage.end()) {
        throw cRuntimeError("GlobalNodeList::registerPeer(): "
                "Peer is not in peer set");
    } else {
        delete it->second.node;
        it->second.node = new NodeHandle(peer);
        peerStorage.setBootstrapped(it, true);
    }
}

void GlobalNodeList::refreshEntry(const TransportAddress& peer)
{
    PeerHashMap::iterator it = peerStorage.find(peer.getIp());

    if (it == peerStorage.end()) {
        throw cRuntimeError("GlobalNodeList::refreshEntry(): "
                "Peer is not in peer set");
    } else {
        delete it->second.node;
        it->second.node = new TransportAddress(peer);
    }
}

void GlobalNodeList::removePeer(const TransportAddress& peer)
{
    PeerHashMap::iterator it = peerStorage.find(peer.getIp());

    if (it != peerStorage.end()) {
        peerStorage.setBootstrapped(it, false);
    }
}

void GlobalNodeList::killPeer(const IPvXAddress& ip)
{
    PeerHashMap::iterator it = peerStorage.find(ip);
    if (it != peerStorage.end()) {
        if (it->second.info->isPreKilled()) {
            it->second.info->setPreKilled(false);
            preKilledNodes--;
        }

        // if valid NPS landmark: decrease landmarkPeerSize
        PeerInfo* peerInfo = it->second.info;
        if (peerInfo->getNpsLayer() > -1) {
            landmarkPeerSize--;
            landmarkPeerSizePerType[it->second.info->getTypeID()]--;
        }

        peerStorage.erase(it);
    }
}

PeerInfo* GlobalNodeList::getPeerInfo(const TransportAddress& peer)
{
    return getPeerInfo(peer.getIp());
}

PeerInfo* GlobalNodeList::getPeerInfo(const IPvXAddress& ip)
{
    PeerHashMap::iterator it = peerStorage.find(ip);

    if (it == peerStorage.end())
        return NULL;
    else
        return it->second.info;
}

PeerInfo* GlobalNodeList::getRandomPeerInfo(int32_t nodeType,
                                            bool bootstrappedNeeded)
{
    PeerHashMap::iterator it = peerStorage.getRandomNode(nodeType,
                                                         bootstrappedNeeded,
                                                         false);
    if (it == peerStorage.end()) {
        return NULL;
    } else {
        return it->second.info;
    }
}

void GlobalNodeList::setPreKilled(const TransportAddress& address)
{
    PeerInfo* peer = getPeerInfo(address);

    if ((peer != NULL) && !(peer->isPreKilled())) {
        preKilledNodes++;
        peer->setPreKilled(true);
    }
}

// TODO: this method should be removed in the future
TransportAddress* GlobalNodeList::getRandomAliveNode(int32_t nodeType)
{
    if (peerStorage.size() <= preKilledNodes) {
        // all nodes are already marked for deletion;
        return NULL;
    } else {
        PeerHashMap::iterator it = peerStorage.getRandomNode(nodeType, false,
                                                             false);
        while (it != peerStorage.end()) {
            if (!it->second.info->isPreKilled()) {
                return it->second.node;
            } else {
                it = peerStorage.getRandomNode(nodeType, false, false);
            }
        }
        return NULL;
    }
}

void GlobalNodeList::setMalicious(const TransportAddress& address, bool malicious)
{
    peerStorage.setMalicious(peerStorage.find(address.getIp()), malicious);
}

bool GlobalNodeList::isMalicious(const TransportAddress& address)
{
    PeerInfo* peer = getPeerInfo(address);

    if (peer != NULL) {
        return peer->isMalicious();
    }

    return false;
}

cObject** GlobalNodeList::getContext(const TransportAddress& address)
{
    PeerInfo* peer = getPeerInfo(address);

    if (peer != NULL) {
        return peer->getContext();
    }

    return NULL;
}

void GlobalNodeList::setOverlayReadyIcon(const TransportAddress& address,
                                         bool ready)
{
    if (ev.isGUI()) {
        const char* color;

        if (ready) {
            // change color if node is malicious
            color = isMalicious(address) ? "green" : "";
        } else {
            color = isMalicious(address) ? "yellow" : "red";
        }

        PeerInfo* info = getPeerInfo(address);

        if (info != NULL) {
            simulation.getModule(info->getModuleID())->
                    getDisplayString().setTagArg("i2", 1, color);
        }
    }
}

bool GlobalNodeList::areNodeTypesConnected(int32_t a, int32_t b)
{
    if ((a > MAX_NODETYPES) || (b > MAX_NODETYPES)) {
        throw cRuntimeError("GlobalNodeList::areNodeTypesConnected(): nodeType "
              "bigger then MAX_NODETYPES");
    }

    return connectionMatrix[a][b];
}

void GlobalNodeList::connectNodeTypes(int32_t a, int32_t b)
{
    if ((a > MAX_NODETYPES) || (b > MAX_NODETYPES)) {
        throw cRuntimeError("GlobalNodeList::connectNodeTypes(): nodeType "
              "bigger then MAX_NODETYPES");
    }

    connectionMatrix[a][b]=true;

    EV << "[GlobalNodeList::connectNodeTypes()]\n"
       << "    Connecting " << a << "->" << b
       << endl;

}

void GlobalNodeList::disconnectNodeTypes(int32_t a, int32_t b)
{
    if ((a > MAX_NODETYPES) || (b > MAX_NODETYPES)) {
        throw cRuntimeError("GlobalNodeList::disconnectNodeTypes(): nodeType "
              "bigger then MAX_NODETYPES");
    }

    connectionMatrix[a][b]=false;

    EV << "[GlobalNodeList::disconnectNodeTypes()]\n"
       << "    Disconnecting " << a << "->" << b
       << endl;

}

void GlobalNodeList::mergeBootstrapNodes(int toPartition, int fromPartition,
                                         int numNodes)
{
    BootstrapList* bootstrapList =
        check_and_cast<BootstrapList*>(simulation.getModule(
            getRandomPeerInfo(toPartition, false)->getModuleID())->
            getSubmodule("bootstrapList"));

    bootstrapList->insertBootstrapCandidate(getRandomNode(fromPartition, true),
                                       DNSSD);
}


void GlobalNodeList::createKeyList(uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
        keyList.push_back(OverlayKey::random());
}

GlobalNodeList::KeyList* GlobalNodeList::getKeyList(uint32_t maximumKeys)
{
    if( !isKeyListInitialized ) createKeyList(maxNumberOfKeys);
    if (maximumKeys > keyList.size()) {
        maximumKeys = keyList.size();
    }
    // copy keylist to temporary keylist
    KeyList tmpKeyList;
    tmpKeyList.clear();

    for (uint32_t i=0; i < keyList.size(); i++) {
        tmpKeyList.push_back(keyList[i]);
    }

    KeyList* returnList = new KeyList;

    for (uint32_t i=0; i < ((float)maximumKeys * keyProbability); i++) {
        uint32_t index = intuniform(0, tmpKeyList.size()-1);

        returnList->push_back(tmpKeyList[index]);
        tmpKeyList.erase(tmpKeyList.begin()+index);
    }

    return returnList;
}

const OverlayKey& GlobalNodeList::getRandomKeyListItem()
{
    if( !isKeyListInitialized ) createKeyList(maxNumberOfKeys);
    return keyList[intuniform(0,keyList.size()-1)];
}
