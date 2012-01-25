//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file BootstrapList.cc
 * @author Bin Zheng, Ingmar Baumgart
 */

#include <BaseApp.h>
#include <BootstrapList.h>
#include <GlobalNodeList.h>
#include <BaseOverlay.h>
#include <UnderlayConfiguratorAccess.h>
#include <ZeroconfConnector.h>
#include <CommonMessages_m.h>
#include <hashWatch.h>

#include <fstream>
#include <iostream>

using namespace std;

Define_Module(BootstrapList);

std::ostream& operator<<(std::ostream& os, const BootstrapNodeHandle* n)
{
    if (n == NULL) {
        os << "<NULL>";
        return os;
    }

    if (n->isUnspecified()) {
        os << "<unspec>";
    } else {
        os << n->getIp() << ":" << n->getPort() << " " << n->getKey()
           << " last ping:" <<  n->getLastPing()
           << " prio:" << n->getNodePrio();
    }

    return os;
};


BootstrapList::BootstrapList()
{
    zeroconfConnector = NULL;
    timerMsg = NULL;
}


BootstrapList::~BootstrapList()
{
    cancelAndDelete(timerMsg);
}


void BootstrapList::initializeApp(int stage)
{
    if (stage != MIN_STAGE_COMPONENTS) {
        return;
    }

    mergeOverlayPartitions = par("mergeOverlayPartitions");
    maintainList = par("maintainList");

    if (getParentModule()->getParentModule()->getSubmodule("zeroconfConnector", 0)) {
        zeroconfConnector = check_and_cast<ZeroconfConnector *>(getParentModule()->
                getParentModule()->getSubmodule("zeroconfConnector", 0));
        if (!zeroconfConnector->isEnabled()) {
            zeroconfConnector = NULL;
        }
    }

    // load local cache list only when using SingleHostUnderlayConfigurator
    if (zeroconfConnector) {
        ifstream nodeListFile("nodelist.dat", ios::in);
        if (!nodeListFile) {
            EV << "[BootstrapList::initializeApp() @ " << overlay->getThisNode().getIp()
               << " (" << overlay->getThisNode().getKey() << ")]\n"
               << "    nodelist.dat not found"
               << endl;
        } else {
            char address[16];
            int port;
            string key;
            BootstrapNodeHandle *node;

            while (nodeListFile >> address >> port >> key) {
                node = new BootstrapNodeHandle(OverlayKey(key, 16),
                                               IPvXAddress(address), port,
                                               CACHE);
                if (node)
                    insertBootstrapCandidate(*node);
            }
            nodeListFile.close();

            //TODO
            if (bootstrapList.size()) sendReadyMessage();
        }
    } else {
        sendReadyMessage();
    }

    WATCH_UNORDERED_MAP(bootstrapList);

    if (maintainList) {
        timerMsg = new cMessage("BootstrapListTimer");
        scheduleAt(simTime() + timerInterval, timerMsg);
    }
}


void BootstrapList::handleTimerEvent(cMessage *msg)
{
    if (msg == timerMsg) {
        handleBootstrapListTimerExpired();
        scheduleAt(simTime() + timerInterval, msg);
    } else {
        throw cRuntimeError("BootstrapList::handleTimerEvent(): "
                             "Received unknown self-message!");
    }
}

void BootstrapList::handleBootstrapListTimerExpired()
{
    simtime_t current = simTime();
    BootstrapNodeHandle *node;
    BootstrapNodeSet::iterator iter;

    for (iter = bootstrapList.begin(); iter != bootstrapList.end(); iter++) {
        node = iter->second;
        if (timerInterval <= SIMTIME_DBL(current - node->getLastPing())) {
            pingNode(*node);
        }
    }
}


void BootstrapList::handleRpcResponse(BaseResponseMessage* msg,
                                      cPolymorphic* context,
                                      int rpcId, simtime_t rtt)
{
    RPC_SWITCH_START(msg)
    RPC_ON_RESPONSE(Lookup) {
        handleLookupResponse(_LookupResponse);
        EV << "[BootstrapList::handleRpcResponse() @ "
           << overlay->getThisNode().getIp()
           << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
           << "    Lookup RPC Response received: id=" << rpcId << "\n"
           << "    msg=" << *_LookupResponse << " rtt=" << rtt
           << endl;
    }
    RPC_SWITCH_END( )
}

void BootstrapList::handleLookupResponse(LookupResponse* msg)
{
    EV << "[BootstrapList::handleLookupResponse() @ "
       << overlay->getThisNode().getIp()
       << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
       << "    Lookup response for key " << msg->getKey()<< " : ";

    if (msg->getIsValid()) {
        for (uint32_t i=0; i<msg->getSiblingsArraySize(); i++) {
            if (msg->getSiblings(i).getKey() == msg->getKey()) {
                EV << "Key " << msg->getSiblings(i)
                   << "belongs to a local node." << endl;
                return;
            }
        }

        EV << "New partition discovered, going to join." << endl;
        BootstrapNodeSet::iterator iter;
        for (iter = bootstrapList.begin(); iter != bootstrapList.end(); iter++) {
            if (iter->second->getKey() == msg->getKey())
                overlay->joinForeignPartition(*iter->second);
        }

    } else {
        EV << "failed" << endl;
    }

    return;
}


void BootstrapList::pingResponse(PingResponse* pingResponse,
                                 cPolymorphic* context,
                                 int rpcId, simtime_t rtt)
{
    NodeHandle &srcNode = pingResponse->getSrcNode();
    BootstrapNodeSet::iterator iter;

    EV << "[BootstrapList::pingResponse() @ " << overlay->getThisNode().getIp()
           << " (" << overlay->getThisNode().getKey() << ")]\n"
           << "    Ping response from node " << srcNode
           << " with rtt " << rtt << endl;

    // update the lastPing field of the source node
    if (!srcNode.isUnspecified())
        if ((iter = bootstrapList.find(srcNode)) != bootstrapList.end())
            iter->second->setLastPing(simTime());

    return;
}


void BootstrapList::pingTimeout(PingCall* pingCall,
                                const TransportAddress& dest,
                                cPolymorphic* context,
                                int rpcId)
{
    EV << "[BootstrapList::pingTimeout() @ " << overlay->getThisNode().getIp()
       << " (" << overlay->getThisNode().getKey() << ")]\n"
       << "    Ping timeout for node " << dest << endl;
    // destination does not answer, remove it from the list
    removeBootstrapCandidate(dest);
}


const TransportAddress BootstrapList::getBootstrapNode()
{
    if (!maintainList) {
        // TODO: add a parameter to return malicious bootstrap nodes also
        return overlay->globalNodeList->getRandomNode(-1, true, true);
        // return overlay->globalNodeList->getBootstrapNode();
    } else {
        const NodeHandle *bootstrapNode = &NodeHandle::UNSPECIFIED_NODE;
        int i, j = 0;

        // pick a random node from the list
        if (!bootstrapList.empty()) {
            i = intrand(bootstrapList.size());
            BootstrapNodeSet::iterator iter = bootstrapList.begin();

            while (j < i) {
                iter++;
                j++;
            }

            bootstrapNode = (NodeHandle *)iter->second;
        } else {
            // if the list empty, get a bootstrap node from GlobalNodeList
            if (!zeroconfConnector)
                bootstrapNode = &overlay->globalNodeList->
                        getBootstrapNode(overlay->getThisNode());
        }
        return *bootstrapNode;
    }
}


void BootstrapList::locateBootstrapNode(const NodeHandle& node)
{
    Enter_Method_Silent();

    if ((overlay->getState() == BaseOverlay::READY) && (!node.isUnspecified())
            && mergeOverlayPartitions) {
       LookupCall* call = new LookupCall();
       call->setBitLength(0);
       call->setKey(node.getKey());
       call->setNumSiblings(overlay->getMaxNumSiblings());
       sendInternalRpcCall(OVERLAY_COMP, call);
    }

    return;
}


bool BootstrapList::insertBootstrapCandidate(const NodeHandle& node,
                                             BootstrapNodePrioType prio)
{
    Enter_Method_Silent();

    BootstrapNodeSet::iterator iter;
    BootstrapNodeHandle *bootstrapNode;

    // if node already in the list, update its lastPing and nodePrio fields
    if ((iter = bootstrapList.find(node)) != bootstrapList.end()) {
        iter->second->setLastPing(simTime());
        iter->second->setNodePrio(prio);

        return true;
    }

    if ((bootstrapNode = new BootstrapNodeHandle(node, prio))) {
        bootstrapNode->setLastPing(simTime());
        bootstrapList.insert(NodePair(*bootstrapNode, bootstrapNode));
    }

    locateBootstrapNode(node);

    return false;
}


bool BootstrapList::insertBootstrapCandidate(BootstrapNodeHandle& node)
{
    Enter_Method_Silent();

    BootstrapNodeSet::iterator iter;

    node.setLastPing(simTime());

    // if node already in the list, update its lastPing and nodePrio fields
    if ((iter = bootstrapList.find(node)) != bootstrapList.end()) {
        iter->second->setLastPing(node.getLastPing());
        iter->second->setNodePrio(node.getNodePrio());

        return true;
    }

    bootstrapList.insert(NodePair(node, &node));
    locateBootstrapNode(node);

    return false;
}


void BootstrapList::removeBootstrapCandidate(const TransportAddress& addr)
{
    BootstrapNodeSet::iterator iter;
    BootstrapNodeHandle* bootstrapNode;

    if (!addr.isUnspecified()) {
        if ((iter = bootstrapList.find(addr)) != bootstrapList.end()) {
            bootstrapNode = iter->second;
            bootstrapList.erase(iter);
            delete bootstrapNode;
        }
    }

    return;
}


void BootstrapList::removeBootstrapNode(const NodeHandle& node)
{
    overlay->globalNodeList->removePeer(node);
    // at this point, we consider this node not being able to provide the
    // boot service anymore, therefore we have to revoke
    // the service via zeroconfConnector
    if (zeroconfConnector) {
        zeroconfConnector->revokeService();
    }
}


void BootstrapList::registerBootstrapNode(const NodeHandle& node)
{
    globalNodeList->registerPeer(node);

    // at this point, we consider this node as booted and therefore have to
    // announce the boot service for it via zeroconfConnector
    if (zeroconfConnector) {
        zeroconfConnector->announceService(node);
    }
}


void BootstrapList::finishApp()
{
    // dump the list of bootstrap nodes into the local cache only when
    // using SingleHostUnderlayConfigurator
    if (zeroconfConnector) {
        ofstream nodeListFile("nodelist.dat", ios::out|ios::trunc);
        if (!nodeListFile) {
            std::cerr << "nodelist.dat could not be opened" << endl;
        } else {
            for (BootstrapNodeSet::iterator iter = bootstrapList.begin();
                 iter != bootstrapList.end(); iter++) {
                nodeListFile << iter->second->getIp().str() << ' '
                             << iter->second->getPort() << ' '
                             << iter->second->getKey().toString(16) << '\n';
            }
            nodeListFile.close();
        }
    }

    // delete all bootstrap nodes and clear the list
    for (BootstrapNodeSet::iterator iter = bootstrapList.begin();
         iter != bootstrapList.end(); iter++) {
        delete iter->second;
    }

    bootstrapList.clear();
}
