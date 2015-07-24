//
// Copyright (C) 2012 Alfonso Ariza Universidad de M�laga
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

#include "inet/common/GlobalWirelessLinkInspector.h"



namespace inet{

GlobalWirelessLinkInspector::CostMap* GlobalWirelessLinkInspector::costMap = nullptr;
GlobalWirelessLinkInspector::GlobalRouteMap *GlobalWirelessLinkInspector::globalRouteMap = nullptr;
GlobalWirelessLinkInspector::LocatorMap *GlobalWirelessLinkInspector::globalLocatorMap = nullptr;
GlobalWirelessLinkInspector::QueueSize *GlobalWirelessLinkInspector::queueSize;

Define_Module(GlobalWirelessLinkInspector);

GlobalWirelessLinkInspector::GlobalWirelessLinkInspector()
{
    // TODO Auto-generated constructor stub
    costMap = nullptr;
    globalRouteMap = nullptr;
    globalLocatorMap = nullptr;
    queueSize = nullptr;
}

GlobalWirelessLinkInspector::~GlobalWirelessLinkInspector()
{
    // TODO Auto-generated destructor stub
    if (costMap != nullptr)
    {
        while (!costMap->empty())
        {
            delete costMap->begin()->second;
            costMap->erase(costMap->begin());
        }
        delete costMap;
        costMap = nullptr;
    }
    if (globalRouteMap != nullptr)
    {
        while (!globalRouteMap->empty())
        {
            ProtocolsRoutes vect = globalRouteMap->begin()->second;
            for (unsigned int i = 0;i < vect.size(); i++)
            {
                delete vect[i].routesVector;
            }
            globalRouteMap->erase(globalRouteMap->begin());
        }
        delete globalRouteMap;
        globalRouteMap = nullptr;
    }
    if (globalLocatorMap != nullptr)
    {
        delete globalLocatorMap;
        globalLocatorMap = nullptr;
    }
}

void GlobalWirelessLinkInspector::initialize()
{

    if (costMap == nullptr)
    {
        costMap = new CostMap;
    }
    else
    {
        throw cRuntimeError("more that an instance of GlobalWirelessWirelessLinkInspector exist");
    }

    if (globalRouteMap == nullptr)
    {
        globalRouteMap = new GlobalRouteMap;
    }
    else
    {
        throw cRuntimeError("more that an instance of GlobalWirelessWirelessLinkInspector exist");
    }
    if (globalLocatorMap == nullptr)
    {
        globalLocatorMap = new LocatorMap;
    }
    else
    {
        throw cRuntimeError("more that an instance of GlobalWirelessWirelessLinkInspector exist");
    }
    if (queueSize == nullptr)
    {
        queueSize = new QueueSize;
    }
    else
    {
        throw cRuntimeError("more that an instance of GlobalWirelessWirelessLinkInspector exist");
    }
}

void GlobalWirelessLinkInspector::handleMessage(cMessage *msg)
{
    throw cRuntimeError ("GlobalWirelessWirelessLinkInspector has received a packet");
}

void GlobalWirelessLinkInspector::setLinkCost(const L3Address& org,const L3Address& dest,const Link &link)
{
    if (!costMap)
        return;
    auto it = costMap->find(org);
    if (it != costMap->end())
    {
        NodeLinkCost *nLinkCost = it->second;
        auto it2 = nLinkCost->find(dest);
        if (it2 != nLinkCost->end())
        {
            it2->second = link;
            return;
        }
        else
        {
            nLinkCost->insert(std::make_pair(dest,link));
        }
    }
    else
    {
        NodeLinkCost *nLinkCost = new NodeLinkCost;
        nLinkCost->insert(std::make_pair(dest,link));
        costMap->insert(std::make_pair(org,nLinkCost));
    }
}

bool GlobalWirelessLinkInspector::getLinkCost(const L3Address& org,const L3Address& dest,Link &link)
{

    if (!costMap)
        return false;
    auto it = costMap->find(org);
    if (it != costMap->end())
    {
        NodeLinkCost *nLinkCost = it->second;
        auto it2 = nLinkCost->find(dest);
        if (it2 != nLinkCost->end())
        {
            link = it2->second;
            return true;
        }
    }
    return false;
}

bool GlobalWirelessLinkInspector::getCostPath(const std::vector<L3Address>& path, Link &link)
{

    if (!costMap)
        return false;
    link.costEtt = 0;
    link.costEtx = 0;
    for (unsigned int i = 0; i < path.size()-1; i++)
    {
        Link linkAux;
        if (getLinkCost(path[i],path[i+1],linkAux))
        {
            link.costEtt += linkAux.costEtt;
            link.costEtx += linkAux.costEtx;
        }
        else
            return false;
    }
    return true;
}

bool GlobalWirelessLinkInspector::getWorst(const std::vector<L3Address>& path, Link &link)
{

    if (!costMap)
        return false;
    link.costEtt = 0;
    link.costEtx = 0;
    for (unsigned int i = 0; i < path.size()-1; i++)
    {
        Link linkAux;
        if (getLinkCost(path[i],path[i+1],linkAux))
        {
            if (link.costEtx < linkAux.costEtx)
            {
                link.costEtt = linkAux.costEtt;
                link.costEtx = linkAux.costEtx;
            }
        }
        else
            return false;
    }
    return true;
}


bool GlobalWirelessLinkInspector::setRoute(const cModule* mod,const L3Address &orgA, const L3Address &dest, const L3Address &gw, const bool &erase)
{
    if (globalRouteMap == nullptr)
        return false;
    auto it = globalRouteMap->find(orgA);
    if (it == globalRouteMap->end())
        return false;
    RouteMap* routesVector = nullptr;
    for (unsigned int i = 0; i < it->second.size(); i++)
    {
        if (it->second[i].mod == mod)
        {
            routesVector = it->second[i].routesVector;
            break;
        }
    }
    if (!routesVector)
        return false;
    auto it2 = routesVector->find(dest);
    if (it2 != routesVector->end())
    {
        if (erase)
            routesVector->erase(it2);
        else
            it2->second = gw;
    }
    else
    {
        if (!erase)
            routesVector->insert(std::make_pair(dest,gw));
    }
    return true;
}

void GlobalWirelessLinkInspector::initRoutingTables (const cModule* mod,const L3Address &orgA, bool isProact)
{
    if (globalRouteMap == nullptr)
        return;
    auto it = globalRouteMap->find(orgA);
    if (it == globalRouteMap->end())
    {
        ProtocolRoutingData data;
        ProtocolsRoutes vect;
        data.isProactive = isProact;
        data.routesVector = new RouteMap;
        data.mod = const_cast<cModule*> (mod);
        vect.push_back(data);
        globalRouteMap->insert(std::pair<L3Address,ProtocolsRoutes>(orgA,vect));
    }
    else
    {
        ProtocolRoutingData data;
        data.isProactive = isProact;
        data.mod = const_cast<cModule*> (mod);
        data.routesVector = new RouteMap;
        it->second.push_back(data);
    }
}

bool GlobalWirelessLinkInspector::getRoute(const L3Address &src, const L3Address &dest, std::vector<L3Address> &route)
{
    if (globalRouteMap == nullptr)
        return false;
    L3Address next = src;
    route.clear();
    route.push_back(src);
    if (src == dest)
        return true;
    while (1)
    {
        auto it = globalRouteMap->find(next);
        if (it == globalRouteMap->end())
            return false;
        if (it->second.empty())
            return false;

        if (it->second.size() == 1)
        {
            RouteMap * rt = it->second[0].routesVector;
            auto it2 = rt->find(dest);
            if (it2 == rt->end())
                return false;
            if (it2->second == dest)
            {
                route.push_back(dest);
                return true;
            }
            else
            {
                route.push_back(it2->second);
                next = it2->second;
            }
        }
        else
        {
            if (it->second.size() > 2)
                throw cRuntimeError("Number of routing protocols bigger that 2");
            // if several protocols, search before in the proactive
            RouteMap * rt;
            if (it->second[0].isProactive)
                rt = it->second[0].routesVector;
            else
                rt = it->second[1].routesVector;
            auto it2 = rt->find(dest);
            if (it2 == rt->end())
            {
                // search in the reactive

                if (it->second[0].isProactive)
                    rt = it->second[1].routesVector;
                else
                    rt = it->second[0].routesVector;
                it2 = rt->find(dest);
                if (it2 == rt->end())
                    return false;
            }
            if (it2->second == dest)
            {
                route.push_back(dest);
                return true;
            }
            else
            {
                route.push_back(it2->second);
                next = it2->second;
            }
        }
    }
    return false;
}


bool GlobalWirelessLinkInspector::getRouteWithLocator(const L3Address &src, const L3Address &dest, std::vector<L3Address> &route)
{
    if (globalRouteMap == nullptr)
        return false;
    // search in the locator tables
    //
    L3Address origin;
    L3Address destination;
    if (!getLocatorInfo(src, origin))
    {
        origin = src;
    }
    if (!getLocatorInfo(dest, destination))
    {
        destination = dest;
    }
    return getRoute(origin,destination,route);
}



void GlobalWirelessLinkInspector::setLocatorInfo(L3Address node, L3Address ap)
{
    if (globalLocatorMap == nullptr)
        return;
    if (!ap.isUnspecified())
        (*globalLocatorMap)[node] = ap;
    else
    {
        LocatorIteartor it =  globalLocatorMap->find(node);
        if (it != globalLocatorMap->end())
            globalLocatorMap->erase(it);
    }
}

bool GlobalWirelessLinkInspector::getLocatorInfo(L3Address node, L3Address &ap)
{
    if (globalLocatorMap == nullptr)
        return false;
    LocatorIteartor it =  globalLocatorMap->find(node);
    if (it == globalLocatorMap->end())
        return false;
    ap = it->second;
    return true;
}


bool GlobalWirelessLinkInspector::getNumNodes(L3Address node, int &cont)
{
    cont = 0;
    if (globalLocatorMap == nullptr)
        return false;
    LocatorIteartor it =  globalLocatorMap->find(node);
    if (it == globalLocatorMap->end())
        return false;
    L3Address ap = it->second;
    for (it = globalLocatorMap->begin();it != globalLocatorMap->end();it++)
    {
        if (it->second == ap && it->first != node)
            cont++;
    }
    return true;
}

bool GlobalWirelessLinkInspector::areNeighbour(const L3Address &node1, const L3Address &node2,bool &areNei)
{
    areNei = false;
    if (globalLocatorMap == nullptr)
        return false;
    LocatorIteartor it1 =  globalLocatorMap->find(node1);
    if (it1 == globalLocatorMap->end())
        return false;
    LocatorIteartor it2 =  globalLocatorMap->find(node2);
    if (it2 == globalLocatorMap->end())
        return false;
    L3Address ap1 = it1->second;
    L3Address ap2 = it2->second;
    if (ap1 == ap2)
        areNei = true;
    return true;
}


bool GlobalWirelessLinkInspector::setQueueSize(const L3Address &node, const uint64_t &val)
{
    if (queueSize == nullptr)
        return false;
    (*queueSize)[node] = val;
    return true;
}

bool GlobalWirelessLinkInspector::getQueueSize(const L3Address &node, uint64_t & val)
{
    if (queueSize == nullptr)
        return false;
    auto it = queueSize->find(node);
    if (it == queueSize->end())
        return false;
    val = it->second;
    return true;
}

}

