//
// Copyright (C) 2010 Institut fuer Telematik, Karlsruher Institut fuer Technologie (KIT)
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
 * @file RUNetworkConfigurator.cc
 * @author Bernhard Müller et.al.
 *    Modification of TGMNetworkConfigurator.h provided by ReaSE.
 */

#include "RUNetworkConfigurator.h"
#include <sstream>


Define_Module( RUNetworkConfigurator);

using namespace std;
using namespace RUNetConf;

RUNetworkConfigurator::RUNetworkConfigurator()
{
}

RUNetworkConfigurator::~RUNetworkConfigurator()
{
}

void RUNetworkConfigurator::initialize(int stage)
{
    if (stage != 2)
        return;


    //TODO: read number of AS
    cTopology tempTopology("tempTopo");
    tempTopology.extractByProperty("AS");

    noAS = tempTopology.getNumNodes();
    cout << noAS <<endl;
    nextPow = 1;
    while ((1 << nextPow) < noAS + 1) {
        nextPow++;
    }
    IP_NET_SHIFT = 32 - nextPow;
    NET_MASK = 0xffffffff << IP_NET_SHIFT;

    // extract topology nodes and assign IP addresses
    extractTopology();

    // assign an IPAddress to all nodes in the network
    //FIXME: does asNodeVec.size() work as intended? may need to use noAS instead
    for (unsigned int i = 0; i < asNodeVec.size(); i++) {
        if ((unsigned int) rlTopology[i]->getNumNodes() > (0xffffffff - NET_MASK))
            opp_error("to many nodes in current topology");
        cerr << asNodeVec[i].module->getFullPath() << endl;
        //
        //  insert each router-level node into a node map
        //
        for (int j = 0; j < rlTopology[i]->getNumNodes(); j++) {
            nodeInfoRL curRLNode(rlTopology[i]->getNode(j));
            asNodeVec[i].nodeMap.insert(NODE_MAP_PAIR(curRLNode.moduleId, curRLNode));
        }

        // assign IP address and add default route
        assignAddressAndSetDefaultRoutes(asNodeVec[i]);
    }

    // add all further routes in router-level topology
    // (unequal to default route)
    for (unsigned int i = 0; i < asNodeVec.size(); i++)
        setIntraASRoutes(*rlTopology[i], asNodeVec[i]);

    // Having configured all router topologies, add Inter-AS routing paths
    if (noAS > 0)
        createInterASPaths();

    // free Memory
    for (int i = 0; i < noAS; i++) {
        rlTopology[i]->clear();
        delete rlTopology[i];
        asNodeVec[i].nodeMap.clear();
    }
    asTopology.clear();
    asNodeVec.clear();
    tempTopology.clear();
}

void RUNetworkConfigurator::createInterASPaths()
{
    IPv4Address netmask(NET_MASK);
    int asIdHistory = 0;
    unsigned int tmpAddr = 0;
    for (int i = 0; i < asTopology.getNumNodes(); i++) {
        // calculate prefix of current core node
        nodeInfoRL destCore(asTopology.getNode(i));
        tmpAddr = destCore.addr.getInt() >> IP_NET_SHIFT;
        tmpAddr = tmpAddr << IP_NET_SHIFT;
        destCore.addr = IPv4Address(tmpAddr);
        asIdHistory = -1;
        for (int j = 0; j < asTopology.getNumNodes(); j++) {
            if (i == j)
                continue;
            nodeInfoRL srcCore(asTopology.getNode(j));
            tmpAddr = srcCore.addr.getInt() >> IP_NET_SHIFT;
            tmpAddr = tmpAddr << IP_NET_SHIFT;
            srcCore.addr = IPv4Address(tmpAddr);

            // do not calculate paths between nodes of the same AS
            if (destCore.asId == srcCore.asId)
                continue;

            // cross only transit AS in order to reach destination core node
            // therefore, temporarily disable all stub links
            if (asIdHistory != srcCore.asId) {
                disableStubLinks(destCore, srcCore);
                asTopology.calculateUnweightedSingleShortestPathsTo(destCore.node);
            }
            // add routing entry from srcCore to destCore into routing table of srcCore
            InterfaceEntry *ie = srcCore.ift->getInterfaceByNodeOutputGateId(srcCore.node->getPath(0)->getLocalGate()->getId());
            IPv4Route *e = new IPv4Route();
            e->setDestination(destCore.addr);
            e->setNetmask(netmask);
            e->setInterface(ie);
            e->setType(IPv4Route::DIRECT);
            e->setSource(IPv4Route::MANUAL);
            srcCore.rt->addRoute(e);

            // re-enable all stub links
            if (asIdHistory != srcCore.asId) {
                enableStubLinks();
            }
            asIdHistory = srcCore.asId;
        }
    }
}

void RUNetworkConfigurator::disableStubLinks(nodeInfoRL &dst, nodeInfoRL &src)
{
    for (unsigned int i = 0; i < asNodeVec.size(); i++) {
        if ((asNodeVec[i].id == dst.asId) || (asNodeVec[i].id == src.asId))
            continue;
        if (asNodeVec[i].asType == TRANSIT_AS)
            continue;

        for (unsigned int j = 0; j < asNodeVec[i].coreNode.size(); j++) {
            for (int k = 0; k < asNodeVec[i].coreNode[j].node->getNumInLinks(); k++)
                asNodeVec[i].coreNode[j].node->getLinkIn(k)->disable();
        }
    }
}

void RUNetworkConfigurator::enableStubLinks()
{
    for (unsigned int i = 0; i < asNodeVec.size(); i++) {
        if (asNodeVec[i].asType == TRANSIT_AS)
            continue;
        for (unsigned int j = 0; j < asNodeVec[i].coreNode.size(); j++) {
            for (int k = 0; k < asNodeVec[i].coreNode[j].node->getNumInLinks(); k++)
                asNodeVec[i].coreNode[j].node->getLinkIn(k)->enable();
        }
    }
}

void RUNetworkConfigurator::extractTopology()
{
    cTopology currentAS;

    // get the AS-level topology
    if (noAS > 0) {
        currentAS.extractByProperty("AS"); //TODO: check if this is acceptable
        if (currentAS.getNumNodes() != noAS)
            opp_error("Error: AS-Topology contains %u elements - expected %u\n", currentAS.getNumNodes(), noAS);
    }
    else if (noAS == 0) {
        // Network is router-level only
        currentAS.extractByProperty("Internet"); //TODO: check if this is acceptable
        if (currentAS.getNumNodes() != 1)
            opp_error("Error: tried to extract router-level only topology, but found more than 1 Inet module\n");
    }

    // get each router-level topology
    unsigned int netIP = 1 << IP_NET_SHIFT;
    for (int i = 0; i < currentAS.getNumNodes(); i++) {
        cTopology *tmpTopo = new cTopology();
        // extract router-level nodes from NED file
        tmpTopo->extractFromNetwork(getRouterLevelNodes, (void *) currentAS.getNode(i)->getModule()->getName());
        rlTopology.push_back(tmpTopo);
        // assign unique /16 IP address prefix to each AS
        asNodeVec.push_back(nodeInfoAS(currentAS.getNode(i), IPv4Address(netIP), IPv4Address(NET_MASK)));
        netIP += 1 << IP_NET_SHIFT;
    }

    asTopology.extractFromNetwork(getCoreNodes); //TODO: the extra function may be superfuous. extraction could be probably be done via asTopology.extractByProperty("CoreRouter"); -Claus

    //free memory
    currentAS.clear();
}

bool RUNetConf::getRouterLevelNodes(cModule *curMod, void *name)
{
    char *curName = (char*) name;
    if (curName == NULL)
        opp_error("Error while casting void* name to char*\n");

    string sCurName = curName;
    sCurName += ".";
    string curModPath = curMod->getFullPath();
    if (curModPath.find(sCurName) == string::npos)
        return 0;
    //TODO: took some code from ctopology.cc to implement this, check if functionality is correct -Claus
    const char* property = "RL";
    cProperty *prop = curMod->getProperties()->get(property);
    if (!prop)
        return 0;
    const char *value = prop->getValue(cProperty::DEFAULTKEY, 0);
    return opp_strcmp(value, "false")!=0;
}

bool RUNetConf::getCoreNodes(cModule *curMod, void *nullPointer)
{
    //TODO: took some code from ctopology.cc to implement this, check if functionality is correct -Claus
    const char* property = "CoreRouter";
    cProperty *prop = curMod->getProperties()->get(property);
    if (!prop)
        return 0;
    const char *value = prop->getValue(cProperty::DEFAULTKEY, 0);
    return opp_strcmp(value, "false")!=0;
}

void RUNetworkConfigurator::assignAddressAndSetDefaultRoutes(nodeInfoAS &asInfo)
{
    unsigned int currentIP = asInfo.addr.getInt() + 1;
    int countEdgeRouter = 2; // avoids IP address 127.0.0.1
    int countRouter = 1;
    int edgeShift = 0;

    NODE_MAP::iterator mapIt = asInfo.nodeMap.begin();
    while (mapIt != asInfo.nodeMap.end()) {
        if (mapIt->second.routerType == EDGE) {
            countEdgeRouter++;
        }
        mapIt++;
    }

    nextPow = 0;
    while ((1 << nextPow) < countEdgeRouter) {
        nextPow++;
    }

    edgeShift = IP_NET_SHIFT - nextPow;
    asInfo.subnetmask = IPv4Address(0xffffffff << edgeShift);
    countEdgeRouter = 1;

    mapIt = asInfo.nodeMap.begin();
    while (mapIt != asInfo.nodeMap.end()) {
        if (mapIt->second.routerType == ENDSYS) {
            mapIt++;
            continue;
        }

        if (mapIt->second.routerType == EDGE) {
            currentIP = asInfo.addr.getInt() + 1 + (countEdgeRouter++ << edgeShift);
            edgeRouter temp;
            temp.edgeIP = currentIP;
            temp.usedIPs = 1;
            temp.module = mapIt->second.module;
            asInfo.edgeRouter.push_back(temp);
        }
        else {
            currentIP = asInfo.addr.getInt() + countRouter++;
        }

        for (int j = 0; j < mapIt->second.ift->getNumInterfaces(); j++) {
            //
            // all interfaces except loopback get the same IP address
            //
            InterfaceEntry *ie = mapIt->second.ift->getInterface(j);
            if (!ie->isLoopback()) {
                ie->ipv4Data()->setIPAddress(IPv4Address(currentIP));
                ie->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS);
            }
        }
        if (mapIt->second.rt->getRouterId().isUnspecified())
            mapIt->second.rt->setRouterId(IPv4Address(currentIP));
        mapIt->second.addr.set(currentIP);

        // remember core nodes of each AS in additional list for assignment
        // of Inter-AS routing paths
        if (mapIt->second.routerType == CORE)
            asInfo.coreNode.push_back(mapIt->second);
        else {
            //
            // add default route in case of gw, edge, or host
            //
            IPv4Route *e = new IPv4Route();
            e->setDestination(IPv4Address());
            e->setNetmask(IPv4Address());
            e->setInterface(mapIt->second.defaultRouteIE);
            e->setType(IPv4Route::REMOTE);
            e->setSource(IPv4Route::MANUAL);
            //e->setMetric(1);
            mapIt->second.rt->addRoute(e);
        }

        currentIP++;
        mapIt++;
    }
}

void RUNetworkConfigurator::setIntraASRoutes(cTopology &topology, nodeInfoAS &asInfo)
{
    // calculate static routes from each of the AS's router-level nodes to all
    // other nodes of the AS

    for (int i = 0; i < topology.getNumNodes(); i++) {
        nodeInfoRL destNode = asInfo.nodeMap[topology.getNode(i)->getModule()->getId()];
        //
        // calculate shortest path form everywhere toward destNode
        //
        topology.calculateUnweightedSingleShortestPathsTo(destNode.node);
        for (int j = 0; j < topology.getNumNodes(); j++) {
            if (j == i)
                continue;
            nodeInfoRL srcNode = asInfo.nodeMap[topology.getNode(j)->getModule()->getId()];
            // no route exists at all
            if (srcNode.node->getNumPaths() == 0)
                continue;
            // end systems only know a default route to the edge router
            else if (srcNode.routerType == ENDSYS)
                continue;
            else if (destNode.routerType == ENDSYS) {
                if (srcNode.routerType != EDGE)
                        continue;
                InterfaceEntry *ie = srcNode.ift->getInterfaceByNodeOutputGateId(srcNode.node->getPath(0)->getLocalGate()->getId());
                if (ie == srcNode.defaultRouteIE)
                    continue;

                for (uint32 i = 0; i < asInfo.edgeRouter.size(); i++) {

                    if (srcNode.module == asInfo.edgeRouter[i].module) {

                        destNode.addr = IPv4Address(asInfo.edgeRouter[i].edgeIP + asInfo.edgeRouter[i].usedIPs);
                        asInfo.edgeRouter[i].usedIPs++;
                    }
                }

                for (int j = 0; j < destNode.ift->getNumInterfaces(); j++) {
                    //
                    // all interfaces except loopback get the same IP address
                    //
                    InterfaceEntry *ie = destNode.ift->getInterface(j);
                    if (!ie->isLoopback()) {
                        ie->ipv4Data()->setIPAddress(destNode.addr);
                        ie->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS);
                    }
                }
                IPv4Route *e = new IPv4Route();
                e->setDestination(IPv4Address());
                e->setNetmask(IPv4Address());
                e->setInterface(destNode.defaultRouteIE);
                e->setType(IPv4Route::REMOTE);
                e->setSource(IPv4Route::MANUAL);
                destNode.rt->addRoute(e);

                ie = srcNode.ift->getInterfaceByNodeOutputGateId(srcNode.node->getPath(0)->getLocalGate()->getId());
                e = new IPv4Route();
                e->setDestination(destNode.addr);
                e->setNetmask(IPv4Address(255, 255, 255, 255));
                e->setInterface(ie);
                e->setType(IPv4Route::DIRECT);
                e->setSource(IPv4Route::MANUAL);
                srcNode.rt->addRoute(e);
            }
            else {
                //
                // if destination is reachable through default route, no routing entry is necessary
                //
                InterfaceEntry *ie = srcNode.ift->getInterfaceByNodeOutputGateId(srcNode.node->getPath(0)->getLocalGate()->getId());
                if (ie == srcNode.defaultRouteIE)
                    continue;
                else {
                    // add specific routing entry into routing table
                    IPv4Route *e = new IPv4Route();
                    e->setDestination(destNode.addr);
                    if (destNode.routerType == EDGE)
                        e->setNetmask(asInfo.subnetmask);
                    else
                        e->setNetmask(IPv4Address(255, 255, 255, 255));
                    e->setInterface(ie);
                    e->setType(IPv4Route::DIRECT);
                    e->setSource(IPv4Route::MANUAL);
                    srcNode.rt->addRoute(e);
                }
            }
        }
    }
}


