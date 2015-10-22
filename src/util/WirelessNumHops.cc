//
// Copyright (C) 2012 Univerdidad de Malaga.
// Author: Alfonso Ariza
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "WirelessNumHops.h"
#include "MobilityAccess.h"
#include "IPvXAddressResolver.h"
#include "IInterfaceTable.h"
#include "IPv4InterfaceData.h"
#include "GlobalWirelessLinkInspector.h"
#include <algorithm>    // std::max
#include "IRoutingTable.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"

WirelessNumHops::WirelessNumHops()
{
    kshortest = NULL;
    staticScenario = false;
    reStart();
}

void WirelessNumHops::reStart()
{
    // TODO Auto-generated constructor stub
    // fill in routing tables with static routes
    cTopology topo("topo");
    topo.extractByProperty("node");
    cModule *mod = dynamic_cast<cModule*> (getOwner());
    for (mod = dynamic_cast<cModule*> (getOwner())->getParentModule(); mod != 0; mod = mod->getParentModule())
    {
            cProperties *properties = mod->getProperties();
            if (properties && properties->getAsBool("node"))
                break;
    }
    vectorList.clear();
    routeCacheMac.clear();
    linkCache.clear();
    routeCacheIp.clear();
    if (kshortest)
    {
        delete kshortest;
        kshortest = new DijkstraKshortest(limitKshort);
    }

    for (int i = 0; i < topo.getNumNodes(); i++)
    {
        cTopology::Node *destNode = topo.getNode(i);
        IMobility *mod;
        mod = MobilityAccess().get(destNode->getModule());
        if (mod == NULL)
            opp_error("node or mobility module not found");
        nodeInfo info;
        info.mob = mod;
        info.itable = IPvXAddressResolver().findInterfaceTableOf(destNode->getModule());
        vectorList.push_back(info);
        for (int j = 0 ; j < vectorList[i].itable->getNumInterfaces(); j++)
        {
            InterfaceEntry *e = vectorList[i].itable->getInterface(j);
            if (e->getMacAddress().isUnspecified())
                continue;
            if (e->isLoopback())
                continue;
            if (!e->getMacAddress().isUnspecified())
            {
                related[e->getMacAddress()] = i;
                vectorList[i].macAddress.push_back(e->getMacAddress());
            }
            IPv4Address adr = e->ipv4Data()->getIPAddress();
            if (!adr.isUnspecified())
            {
                relatedIp [adr] = i;
                vectorList[i].ipAddress.push_back(adr);
            }
        }

    }
}

WirelessNumHops::~WirelessNumHops()
{
    // TODO Auto-generated destructor stub
    cleanLinkArray();
    vectorList.clear();
    routeCacheMac.clear();
    linkCache.clear();
    routeCacheIp.clear();
    if (kshortest)
        delete kshortest;
}

void WirelessNumHops::fillRoutingTables(const double &tDistance)
{

    if (!linkCache.empty() && staticScenario)
        return;

    // fill in routing tables with static routes
    LinkCache templinkCache;
    // first find root node connections
    Coord cRoot = vectorList[rootNode].mob->getCurrentPosition();
    for (int i=0; i< (int)vectorList.size(); i++)
    {
        if (i == rootNode)
            continue;
        Coord ci = vectorList[i].mob->getCurrentPosition();
        if (cRoot.distance(ci) <= tDistance)
        {
            templinkCache.insert(LinkPair(rootNode,i));
        }
    }
    if (templinkCache.empty())
    {
        // root node doesn't have connections
        linkCache.clear();
        routeCacheMac.clear();
        routeMap.clear();
        routeCacheIp.clear();
        cleanLinkArray();
        return;
    }
    for (int i=0; i< (int)vectorList.size(); i++)
    {
        if (i == rootNode)
            continue;
        for (int j = i; j < (int)vectorList.size(); j++)
        {
            if (i == j)
                continue;
            if (j == rootNode)
                continue;
            Coord ci = vectorList[i].mob->getCurrentPosition();
            Coord cj = vectorList[j].mob->getCurrentPosition();
            if (ci.distance(cj) <= tDistance)
            {
                templinkCache.insert(LinkPair(i,j));
            }
        }
    }

    if (linkCache == templinkCache)
    {
        return;
    }

    linkCache = templinkCache;
    routeCacheMac.clear();
    routeMap.clear();
    routeCacheIp.clear();
    // clean edges
    cleanLinkArray();
    for (LinkCache::iterator it = linkCache.begin(); it != linkCache.end(); ++it)
    {
        addEdge ((*it).node1, (*it).node2,1);
        addEdge ((*it).node2, (*it).node1,1);
    }
}

void WirelessNumHops::fillRoutingTablesWitCost(const double &tDistance)
{
    if (!linkCache.empty() && staticScenario)
        return;

    // fill in routing tables with static routes
    LinkCache templinkCache;
    // first find root node connections

    ManetAddress rootAddr(vectorList[rootNode].macAddress.front());
    Coord cRoot = vectorList[rootNode].mob->getCurrentPosition();
    for (int i=0; i< (int)vectorList.size(); i++)
    {
        if (i == rootNode)
            continue;
        Coord ci = vectorList[i].mob->getCurrentPosition();

        if (cRoot.distance(ci) <= tDistance)
        {

            if (GlobalWirelessLinkInspector::isActive())
            {
                GlobalWirelessLinkInspector::Link linkCost;
                ManetAddress nodeAddr(vectorList[i].macAddress.front());
                if (GlobalWirelessLinkInspector::getLinkCost(rootAddr,nodeAddr,linkCost))
                    if (linkCost.costEtx<1e30)
                        templinkCache.insert(LinkPair(rootNode,i,linkCost.costEtx));
            }
            else
                templinkCache.insert(LinkPair(rootNode,i));
        }
    }
    if (templinkCache.empty())
    {
        // root node doesn't have connections
        linkCache.clear();
        routeCacheMac.clear();
        routeMap.clear();
        routeCacheIp.clear();
        cleanLinkArray();
        return;
    }
    for (int i=0; i< (int)vectorList.size(); i++)
    {
        if (i == rootNode)
            continue;
        for (int j = i; j < (int)vectorList.size(); j++)
        {
            if (i == j)
                continue;
            if (j == rootNode)
                continue;
            Coord ci = vectorList[i].mob->getCurrentPosition();
            Coord cj = vectorList[j].mob->getCurrentPosition();
            if (ci.distance(cj) <= tDistance)
            {
                if (GlobalWirelessLinkInspector::isActive())
                {
                    GlobalWirelessLinkInspector::Link linkCost;
                    ManetAddress iAdd(vectorList[i].macAddress.front());
                    ManetAddress jAdd(vectorList[j].macAddress.front());
                    if (GlobalWirelessLinkInspector::getLinkCost(iAdd,jAdd,linkCost))
                        if (linkCost.costEtx<1e30)
                            templinkCache.insert(LinkPair(i,j,linkCost.costEtx));
                }
                else
                   templinkCache.insert(LinkPair(i,j));
            }
        }
    }

    if (linkCache == templinkCache)
    {
        return;
    }

    linkCache = templinkCache;
    routeCacheMac.clear();
    routeMap.clear();
    routeCacheIp.clear();
    // clean edges
    cleanLinkArray();
    for (LinkCache::iterator it = linkCache.begin(); it != linkCache.end(); ++it)
    {
        if ((*it).cost == -1)
        {
            addEdge ((*it).node1, (*it).node2,1);
            addEdge ((*it).node2, (*it).node1,1);
        }
        else
        {
            addEdge ((*it).node1, (*it).node2,1,(*it).cost,(*it).cost);
            addEdge ((*it).node2, (*it).node1,1,(*it).cost,(*it).cost);
        }
    }
}


WirelessNumHops::DijkstraShortest::State::State()
{
    idPrev = -1;
    label=tent;
}

WirelessNumHops::DijkstraShortest::State::State(const unsigned int  &costData)
{
    idPrev=-1;
    label=tent;
    cost = costData;
    costAdd = 0;
    costMax = 0;
}

WirelessNumHops::DijkstraShortest::State::State(const unsigned int  &costData,const double &cost1, const double &cost2)
{
    idPrev=-1;
    label=tent;
    cost = costData;
    costAdd = cost1;
    costMax = cost2;
}

void WirelessNumHops::DijkstraShortest::State::setCostVector(const unsigned int &costData)
{
    cost=costData;
    costAdd = 0;
    costMax = 0;
}

void WirelessNumHops::DijkstraShortest::State::setCostVector(const unsigned int  &costData,const double &cost1, const double &cost2)
{
    cost = costData;
    costAdd = cost1;
    costMax = cost2;
}

void WirelessNumHops::cleanLinkArray()
{
    for (LinkArray::iterator it=linkArray.begin();it!=linkArray.end();it++)
        while (!it->second.empty())
        {
            delete it->second.back();
            it->second.pop_back();
        }
    linkArray.clear();
}

void WirelessNumHops::addEdge (const int & originNode, const int & last_node,unsigned int cost)
{
    LinkArray::iterator it;
    it = linkArray.find(originNode);
    if (it!=linkArray.end())
    {
         for (unsigned int i=0;i<it->second.size();i++)
         {
             if (last_node == it->second[i]->last_node_)
             {
                  it->second[i]->cost =cost;
                  return;
             }
         }
    }
    WirelessNumHops::DijkstraShortest::Edge *link = new WirelessNumHops::DijkstraShortest::Edge;
    // The last hop is the interface in which we have this neighbor...
    link->last_node_ = last_node;
    // Also record the link delay and quality..
    link->cost = cost;
    linkArray[originNode].push_back(link);

    if (kshortest)
    {
        kshortest->addEdge (originNode, last_node,cost,1,1e30,1);
    }
}

void WirelessNumHops::addEdge (const int & originNode, const int & last_node,unsigned int cost, double costAdd, double costMax)
{
    LinkArray::iterator it;
    it = linkArray.find(originNode);
    if (it!=linkArray.end())
    {
         for (unsigned int i=0;i<it->second.size();i++)
         {
             if (last_node == it->second[i]->last_node_)
             {
                  it->second[i]->cost = cost;
                  it->second[i]->costAdd = costAdd;
                  it->second[i]->costMax = costMax;
                  return;
             }
         }
    }
    WirelessNumHops::DijkstraShortest::Edge *link = new WirelessNumHops::DijkstraShortest::Edge;
    // The last hop is the interface in which we have this neighbor...
    link->last_node_ = last_node;
    // Also record the link delay and quality..
    link->cost = cost;
    link->costAdd = costAdd;
    link->costMax = costMax;
    linkArray[originNode].push_back(link);
}

int WirelessNumHops::getIdNode(const MACAddress &add)
{
    std::map<MACAddress,int>::iterator it = related.find(add);
    if (it != related.end())
        return it->second;
    opp_error("Node not found with MAC Address %s",add.str().c_str());
    return -1;
}

int WirelessNumHops::getIdNode(const IPv4Address &add)
{
    std::map<IPv4Address,int>::iterator it = relatedIp.find(add);
    if (it != relatedIp.end())
        return it->second;
    opp_error("Node not found with IP Address %s", add.str().c_str());
    return -1;
}

void WirelessNumHops::setRoot(const int & dest_node)
{
    rootNode = dest_node;
    if (kshortest)
    {
        kshortest->setRoot (rootNode);
    }
}


void WirelessNumHops::run()
{
    std::multiset<WirelessNumHops::DijkstraShortest::SetElem> heap;
    routeMap.clear();
    if (linkArray.empty())
        return;

    LinkArray::iterator it;
    it = linkArray.find(rootNode);
    if (it==linkArray.end())
        opp_error("Node root not found %i",rootNode);
    WirelessNumHops::DijkstraShortest::State state(0);
    state.label = tent;
    routeMap[rootNode] = state;


    WirelessNumHops::DijkstraShortest::SetElem elem;
    elem.iD = rootNode;
    elem.cost= 0;
    elem.costAdd = 0;
    elem.costMax = 0;
    heap.insert(elem);

    while (!heap.empty())
    {

        std::multiset<WirelessNumHops::DijkstraShortest::SetElem>::iterator itHeap = heap.begin();
        // search if exist several with the same cost and extract randomly one
        std::vector<std::multiset<WirelessNumHops::DijkstraShortest::SetElem>::iterator> equal;
        while(1)
        {
            equal.push_back(itHeap);
            std::multiset<WirelessNumHops::DijkstraShortest::SetElem>::iterator itHeap3 = itHeap;
            ++itHeap3;
            if (itHeap3 == heap.end())
                break;

            if (itHeap3->cost > itHeap->cost)
                break;
            itHeap = itHeap3;
        }
        int numeq = equal.size()-1;
        int val = numeq > 0?intuniform(0,numeq):0;
        itHeap = equal[val];
        equal.clear();

        //
        WirelessNumHops::DijkstraShortest::SetElem elem = *itHeap;
        heap.erase(itHeap);

        RouteMap::iterator it;

        it = routeMap.find(elem.iD);
        if (it==routeMap.end())
            opp_error("node not found in routeMap %i",elem.iD);
        if ((it->second).label == perm)
            continue;

        (it->second).label = perm;

        LinkArray::iterator linkIt=linkArray.find(elem.iD);

        for (unsigned int i=0;i<linkIt->second.size();i++)
        {
            WirelessNumHops::DijkstraShortest::Edge* current_edge= (linkIt->second)[i];
            RouteMap::iterator itNext = routeMap.find(current_edge->last_node_);
            if (itNext != routeMap.end() && itNext->second.label == perm)
                continue;
            unsigned int cost = current_edge->cost + (it->second).cost;
            double costAdd = current_edge->costAdd + (it->second).costAdd;
            double costMax = std::max(current_edge->costMax ,(it->second).costMax);

            if (current_edge->last_node_ == 113)
                EV << "destino \n";
            if (itNext == routeMap.end())
            {
                WirelessNumHops::DijkstraShortest::State state;
                state.idPrev = elem.iD;
                state.cost = cost;
                state.costAdd = costAdd;
                state.costMax = costMax;
                state.label = tent;

                routeMap[current_edge->last_node_] = state;
                WirelessNumHops::DijkstraShortest::SetElem newElem;
                newElem.iD = current_edge->last_node_;
                newElem.cost = cost;
                newElem.costAdd = costAdd;
                newElem.costMax = costMax;
                heap.insert(newElem);
            }
            else
            {
                if (cost < itNext->second.cost)
                {
                    itNext->second.cost = cost;
                    itNext->second.costAdd = costAdd;
                    itNext->second.costMax = costMax;
                    itNext->second.idPrev = elem.iD;
                    // actualize heap
                    WirelessNumHops::DijkstraShortest::SetElem newElem;
                    newElem.iD=current_edge->last_node_;
                    newElem.cost = cost;
                    newElem.costAdd = costAdd;
                    newElem.costMax = costMax;
                    heap.insert(newElem);
                }
            }
        }
    }
    if (kshortest)
    {
        kshortest->run();
    }
}


void WirelessNumHops::runUntil (const int &target)
{
    std::multiset<WirelessNumHops::DijkstraShortest::SetElem> heap;
    routeMap.clear();

    if (linkArray.empty())
        return;

    LinkArray::iterator it;
    it = linkArray.find(rootNode);
    if (it==linkArray.end())
        opp_error("Root node not found %i",rootNode);
    WirelessNumHops::DijkstraShortest::State state(0);
    state.label = tent;
    routeMap[rootNode] = state;


    WirelessNumHops::DijkstraShortest::SetElem elem;
    elem.iD = rootNode;
    elem.cost= 0;
    elem.costAdd = 0;
    elem.costMax = 0;
    heap.insert(elem);

    while (!heap.empty())
    {
        std::multiset<WirelessNumHops::DijkstraShortest::SetElem>::iterator itHeap = heap.begin();
        // search if exist several with the same cost and extract randomly one
        std::vector<std::multiset<WirelessNumHops::DijkstraShortest::SetElem>::iterator> equal;
        while(1)
        {
            equal.push_back(itHeap);
            std::multiset<WirelessNumHops::DijkstraShortest::SetElem>::iterator itHeap3 = itHeap;
            ++itHeap3;
            if (itHeap3 == heap.end())
                break;

            if (itHeap3->cost > itHeap->cost)
                break;
            itHeap = itHeap3;
        }
        int numeq = equal.size()-1;
        int val = numeq > 0?intuniform(0,numeq):0;
        itHeap = equal[val];
        equal.clear();


        //
        WirelessNumHops::DijkstraShortest::SetElem elem = *itHeap;
        heap.erase(itHeap);

        heap.erase(heap.begin());

        RouteMap::iterator it;

        it = routeMap.find(elem.iD);
        if (it==routeMap.end())
            opp_error("node not found in routeMap %i",elem.iD);
        if ((it->second).label == perm)
            continue;
        (it->second).label = perm;
        if (target == elem.iD)
            return;

        LinkArray::iterator linkIt=linkArray.find(elem.iD);

        for (unsigned int i=0;i<linkIt->second.size();i++)
        {
            WirelessNumHops::DijkstraShortest::Edge* current_edge= (linkIt->second)[i];
            RouteMap::iterator itNext = routeMap.find(current_edge->last_node_);
            unsigned int cost = current_edge->cost + (it->second).cost;
            double costAdd = current_edge->costAdd + (it->second).costAdd;
            double costMax = std::max(current_edge->costMax ,(it->second).costMax);
            if (itNext == routeMap.end())
            {
                WirelessNumHops::DijkstraShortest::State state;
                state.idPrev=elem.iD;
                state.cost = cost;
                state.costAdd = costAdd;
                state.costMax = costMax;
                state.label = tent;
                routeMap[current_edge->last_node_] = state;
                WirelessNumHops::DijkstraShortest::SetElem newElem;
                newElem.iD=current_edge->last_node_;
                newElem.cost = cost;
                newElem.costAdd = costAdd;
                newElem.costMax = costMax;
                heap.insert(newElem);
            }
            else
            {
                if (itNext->second.label == perm)
                    continue;
                if ( cost < itNext->second.cost)
                {
                    itNext->second.cost = cost;
                    itNext->second.costAdd = costAdd;
                    itNext->second.costMax = costMax;
                    itNext->second.idPrev = elem.iD;
                    // actualize heap
                    WirelessNumHops::DijkstraShortest::SetElem newElem;
                    newElem.iD=current_edge->last_node_;
                    newElem.cost = cost;
                    newElem.costAdd = costAdd;
                    newElem.costMax = costMax;
                    heap.insert(newElem);
                }
            }
        }
    }
    if (kshortest)
    {
        kshortest->runUntil(target);
    }
}


bool WirelessNumHops::getRoute(const int &nodeId,std::deque<int> &pathNode)
{
    RouteMap::iterator it = routeMap.find(nodeId);
    if (it==routeMap.end())
        return false;

    std::deque<int> path;
    int currentNode = nodeId;
    pathNode.clear();
    while (currentNode!=rootNode)
    {
        pathNode.push_front(currentNode);
        currentNode = it->second.idPrev;
        it = routeMap.find(currentNode);
        if (it==routeMap.end())
            opp_error("error in data routeMap");
    }
    return true;
}


bool WirelessNumHops::getRouteCost(const int &nodeId,std::deque<int> &pathNode,double &costAdd, double &costMax)
{
    RouteMap::iterator it = routeMap.find(nodeId);
    if (it==routeMap.end())
        return false;

    int currentNode = nodeId;
    pathNode.clear();
    costAdd = it->second.costAdd;
    costMax = it->second.costMax;
    while (currentNode!=rootNode)
    {
        pathNode.push_front(currentNode);
        currentNode = it->second.idPrev;
        it = routeMap.find(currentNode);
        if (it==routeMap.end())
            opp_error("error in data routeMap");
    }
    return true;
}


bool WirelessNumHops::findRoutePath(const int &nodeId,std::deque<int> &pathNode)
{
    if (getRoute(nodeId,pathNode))
        return true;
    else
    {
        run();
        if (getRoute(nodeId,pathNode))
             return true;
    }
    return false;
}

bool WirelessNumHops::findRoutePathCost(const int &nodeId,std::deque<int> &pathNode,double &costAdd, double &costMax)
{
    if (getRouteCost(nodeId,pathNode,costAdd,costMax))
        return true;
    else
    {
        run();
        if (getRouteCost(nodeId,pathNode,costAdd,costMax))
             return true;
    }
    return false;
}



bool WirelessNumHops::findRouteWithCost(const double &coverageArea, const MACAddress &dest,std::deque<MACAddress> &pathNode, bool withCost,double &costAdd, double &costMax)
{
    if (withCost)
        fillRoutingTablesWitCost(coverageArea);
    else
        fillRoutingTables(coverageArea);

    RouteCacheMac::iterator it = routeCacheMac.find(dest);
    if (it!=routeCacheMac.end())
    {
        pathNode = it->second;
        return true;
    }

    std::deque<int> route;
    int nodeId = getIdNode(dest);
    if (findRoutePathCost(nodeId, route,costAdd,costMax))
    {
        std::deque<MACAddress> path;
        for (unsigned int i = 0; i < route.size(); i++)
        {
            for (std::map<MACAddress,int>::iterator it2 = related.begin(); it2 != related.end(); ++it2)
            {
                if (it2->second ==  route[i])
                    path.push_back(it2->first);
            }
        }
        if (path.size() != route.size())
        {
            opp_error("node id not found");
        }
        pathNode = path;
        // include path in the cache
        routeCacheMac[dest] = path;
        return true;
    }
    return false;
}


bool WirelessNumHops::findRouteWithCost(const double &coverageArea, const IPv4Address &dest,std::deque<IPv4Address> &pathNode, bool withCost, double &costAdd, double &costMax)
{
    if (withCost)
        fillRoutingTablesWitCost(coverageArea);
    else
        fillRoutingTables(coverageArea);

    RouteCacheIp::iterator it = routeCacheIp.find(dest);
    if (it!=routeCacheIp.end())
    {
        pathNode = it->second;
        return true;
    }

    std::deque<int> route;
    int nodeId = getIdNode(dest);
    if (findRoutePathCost(nodeId, route,costAdd,costMax))
    {
        std::deque<IPv4Address> path;
        for (unsigned int i = 0; i < route.size(); i++)
        {
            for (std::map<IPv4Address,int>::iterator it2 = relatedIp.begin(); it2 != relatedIp.end(); ++it2)
            {
                if (it2->second ==  route[i])
                    path.push_back(it2->first);
            }
        }
        if (path.size() != route.size())
        {
            opp_error("node id not found");
        }
        // include path in the cache
        pathNode = path;
        routeCacheIp[dest] = path;
        return true;
    }
    return false;
}

Coord WirelessNumHops::getPos(const int &node)
{
    return vectorList[node].mob->getCurrentPosition();
}

void WirelessNumHops::getNeighbours(const IPv4Address &node, std::vector<IPv4Address>&list, const double &distance)
{
    list.clear();
    int nodeId =  getIdNode(node);
    Coord pos = vectorList[nodeId].mob->getCurrentPosition();
    for (int i = 0; i < (int)vectorList.size(); i++)
    {
        if (i == nodeId)
            continue;
        if (pos.distance(vectorList[i].mob->getCurrentPosition()) < distance)
        {
            for (std::map<IPv4Address,int>::iterator it2 = relatedIp.begin(); it2 != relatedIp.end(); ++it2)
            {
                if (it2->second ==  i)
                    list.push_back(it2->first);
            }
        }
    }
}

void WirelessNumHops::getNeighbours(const MACAddress &node, std::vector<MACAddress>& list, const double &distance)
{
    list.clear();

    list.clear();
    int nodeId =  getIdNode(node);
    Coord pos = vectorList[nodeId].mob->getCurrentPosition();
    for (int i = 0; i < (int)vectorList.size(); i++)
    {
        if (i == nodeId)
            continue;
        if (pos.distance(vectorList[i].mob->getCurrentPosition()) < distance)
        {
            for (std::map<MACAddress,int>::iterator it2 = related.begin(); it2 != related.end(); ++it2)
            {
                if (it2->second ==  i)
                    list.push_back(it2->first);
            }
        }
    }
}


std::deque<int> WirelessNumHops::getRoute(int index)
{
    std::deque<int> route;
    if (index>=(int)routeMap.size())
        return route;
    RouteMap::iterator it = routeMap.begin();
    for (int i = 0; i<index; i++)
        ++it;

    int currentNode = it->first;

    while (currentNode!=rootNode)
    {
        route.push_front(currentNode);
        currentNode = it->second.idPrev;
        it = routeMap.find(currentNode);
        if (it==routeMap.end())
            opp_error("error in data routeMap");
    }
    return route;
}

void WirelessNumHops::getRoute(int i, std::deque<IPv4Address> &pathNode)
{
    std::deque<int> route = getRoute(i);
    pathNode.clear();

    for (unsigned int i = 0; i < route.size(); i++)
    {
        pathNode.push_back(vectorList[route[i]].ipAddress[0]);
    }
    if (pathNode.size() != route.size())
    {
        opp_error("node id not found");
    }
}

void WirelessNumHops::getRoute(int i,std::deque<MACAddress> &pathNode)
{
    std::deque<int> route = getRoute(i);
    pathNode.clear();
    for (unsigned int i = 0; i < route.size(); i++)
    {
        pathNode.push_back(vectorList[route[i]].macAddress[0]);
    }
    if (pathNode.size() != route.size())
    {
        opp_error("node id not found");
    }
}



void WirelessNumHops::setIpRoutingTable()
{
    for (unsigned int i = 0; i< getNumRoutes();i++)
    {
        std::deque<IPv4Address> pathNode;
        getRoute(i, pathNode);
        if (pathNode.empty())
            continue;
        setIpRoutingTable(pathNode.back(), pathNode[0] , pathNode.size());
    }
}


void WirelessNumHops::setIpRoutingTable(const IPv4Address &desAddress, const IPv4Address &gateway,  int hops)
{


    IRoutingTable *inet_rt = RoutingTableAccess().getIfExists();
    IInterfaceTable* itable = InterfaceTableAccess().getIfExists();

    InterfaceEntry *iface = NULL;
    bool found = false;
    for (int j = 0; j < itable->getNumInterfaces(); j++)
    {
        InterfaceEntry *e = itable->getInterface(j);
        if (e->getMacAddress().isUnspecified())
            continue;
        if (e->isLoopback())
            continue;
        if (strstr(e->getName(), "wlan") != NULL)
        {
            iface = e;
            break;
        }
    }

    if (!iface)
        return;

    IPv4Route *oldentry = NULL;
    for (int i = inet_rt->getNumRoutes(); i > 0; --i)
    {
        IPv4Route *e = inet_rt->getRoute(i - 1);
        if (desAddress == e->getDestination())
        {
            found = true;
            oldentry = e;
            break;
        }
    }

    IPv4Address netmask = IPv4Address::ALLONES_ADDRESS;
    IPv4Route::SourceType sourceType =  IPv4Route::MANET;

    if (found)
    {
        if (oldentry->getDestination() == desAddress
                && oldentry->getNetmask() == netmask
                && oldentry->getGateway() == gateway
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == iface
                && oldentry->getSourceType() == sourceType)
            return;
        inet_rt->deleteRoute(oldentry);
    }

    IPv4Route *entry = new IPv4Route();

    /// Destination
    entry->setDestination(desAddress);
    /// Route mask
    entry->setNetmask(netmask);
    /// Next hop
    entry->setGateway(gateway);
    /// Metric ("cost" to reach the destination)
    entry->setMetric(hops);
    /// Interface name and pointer

    entry->setInterface(iface);

    /// Source of route, MANUAL by reading a file,
    /// routing protocol name otherwise
    entry->setSourceType(sourceType);
    inet_rt->addRoute(entry);
}


void WirelessNumHops::setIpRoutingTable(const IPv4Address &root, const IPv4Address &desAddress, const IPv4Address &gateway,  int hops)
{

    int id = getIdNode(root);


    IRoutingTable *inet_rt = RoutingTableAccess().getIfExists((cModule*)vectorList[id].mob);
    InterfaceEntry *iface = NULL;
    bool found = false;
    for (int j = 0; j < vectorList[id].itable->getNumInterfaces(); j++)
    {
        InterfaceEntry *e = vectorList[id].itable->getInterface(j);
        if (e->getMacAddress().isUnspecified())
            continue;
        if (e->isLoopback())
            continue;
        if (strstr(e->getName(), "wlan") != NULL)
        {
            iface = e;
            break;
        }
    }

    if (!iface)
        return;

    IPv4Route *oldentry = NULL;
    for (int i = inet_rt->getNumRoutes(); i > 0; --i)
    {
        IPv4Route *e = inet_rt->getRoute(i - 1);
        if (desAddress == e->getDestination())
        {
            found = true;
            oldentry = e;
            break;
        }
    }

    IPv4Address netmask = IPv4Address::ALLONES_ADDRESS;
    IPv4Route::SourceType sourceType =  IPv4Route::MANET;

    if (found)
    {
        if (oldentry->getDestination() == desAddress
                && oldentry->getNetmask() == netmask
                && oldentry->getGateway() == gateway
                && oldentry->getMetric() == hops
                && oldentry->getInterface() == iface
                && oldentry->getSourceType() == sourceType)
            return;
        inet_rt->deleteRoute(oldentry);
    }

    IPv4Route *entry = new IPv4Route();

    /// Destination
    entry->setDestination(desAddress);
    /// Route mask
    entry->setNetmask(netmask);
    /// Next hop
    entry->setGateway(gateway);
    /// Metric ("cost" to reach the destination)
    entry->setMetric(hops);
    /// Interface name and pointer

    entry->setInterface(iface);

    /// Source of route, MANUAL by reading a file,
    /// routing protocol name otherwise
    entry->setSourceType(sourceType);
    inet_rt->addRoute(entry);
}


bool WirelessNumHops::getKshortest(const MACAddress &dest,KroutesMac &routes)
{
    if (!kshortest)
        return false;

    int nodeId = getIdNode(dest);

    DijkstraKshortest::Kroutes kroute;

    kshortest->getRouteMapK(nodeId,kroute);
    for (unsigned int i = 0; i < kroute.size();i++)
    {
        RouteMac routeAux;
        for (unsigned int j = 0; j < kroute[i].size();j++)
        {
            Uint128 index = kroute[i][j];
            routeAux.push_back(vectorList[index.getLo()].macAddress[0]);
        }
        routes.push_back(routeAux);
    }
#if 1
    // check if the route is in the list
    DijkstraKshortest::Route route;
    std::deque<int>routeMin;
    getRoute(nodeId,routeMin);
    bool find=false;
    for (unsigned int i = 0; i < kroute.size();i++)
    {
        if (routeMin.size()!=kroute[i].size())
            continue;
        bool equal=true;
        for (unsigned int j =0; j <routeMin.size();j++)
            if (routeMin[j]!=kroute[i][j].getLo())
            {
                equal = false;
                break;
            }
        if (equal)
        {
            find = true;
            break;
        }
    }
    if (!find)
    {
        printf("Ruta 1 : ");
        for (unsigned int j =0; j <routeMin.size();j++)
            printf("%i - ",routeMin[j]);
        printf("\n Rutas k-short \n");
        for (unsigned int i = 0; i < kroute.size();i++)
        {
            printf("Ruta %i : ",i);
            for (unsigned int j =0; j <kroute[i].size();j++)
                printf("%i - ",kroute[i][j].getLo());
            printf("\n");
        }
        throw cRuntimeError("Discrepancias en la ruta k-shortes");
    }
#endif
    return true;
}


bool WirelessNumHops::getKshortest(const IPv4Address &dest,KroutesIp &routes)
{
    if (!kshortest)
        return false;

    int nodeId = getIdNode(dest);

    DijkstraKshortest::Kroutes kroute;

    kshortest->getRouteMapK(nodeId,kroute);
    for (unsigned int i = 0; i < kroute.size();i++)
    {
        RouteIp routeAux;
        for (unsigned int j = 0; j < kroute[i].size();j++)
        {
            Uint128 index = kroute[i][j];
            routeAux.push_back(vectorList[index.getLo()].ipAddress[0]);
        }
        routes.push_back(routeAux);
    }
    return true;
}
