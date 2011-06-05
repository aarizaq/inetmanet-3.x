//
// Copyright (C) 2010 Alfonso Ariza, Malaga University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "dijktraKShortest.h"
#include <omnetpp.h>

DijkstraKshortest::CostVector DijkstraKshortest::minimumCost;
DijkstraKshortest::CostVector DijkstraKshortest::maximumCost;

DijkstraKshortest::State::State()
{
    idPrev=InvalidId;
    idPrevIdx=-1;
    label=tent;
}

DijkstraKshortest::State::State(const CostVector &costData)
{
    idPrev=InvalidId;
    idPrevIdx=-1;
    label=tent;
    cost = costData;
}

DijkstraKshortest::State::~State() {
    cost.clear();
}

void DijkstraKshortest::State::setCostVector(const CostVector &costData)
{
    cost=costData;
}

void DijkstraKshortest::addCost (CostVector &val, const CostVector & a, const CostVector & b)
{
    val.clear();
    for (unsigned int i=0;i<a.size();i++)
    {
        Cost aux;
        aux.metric=a[i].metric;
        switch(aux.metric)
        {
            case aditiveMin:
            case aditiveMax:
                aux.value =a[i].value+b[i].value;
                break;
            case concaveMin:
                aux.value = std::min (a[i].value,b[i].value);
                break;
            case concaveMax:
                aux.value = std::max (a[i].value,b[i].value);
                break;
        }
        val.push_back(aux);
    }
}


void DijkstraKshortest::initMinAndMax()
{
   CostVector defaulCost;
   Cost costData;
   costData.metric=aditiveMin;
   costData.value=0;
   minimumCost.push_back(costData);
   costData.metric=aditiveMin;
   costData.value=0;
   minimumCost.push_back(costData);
   costData.metric=concaveMax;
   costData.value=100e100;
   minimumCost.push_back(costData);
   costData.metric=aditiveMin;
   costData.value=0;
   minimumCost.push_back(costData);

   costData.metric=aditiveMin;
   costData.value=10e100;
   maximumCost.push_back(costData);
   costData.metric=aditiveMin;
   costData.value=1e100;
   maximumCost.push_back(costData);
   costData.metric=concaveMax;
   costData.value=0;
   maximumCost.push_back(costData);
   costData.metric=aditiveMin;
   costData.value=10e100;
   maximumCost.push_back(costData);
}

DijkstraKshortest::DijkstraKshortest()
{
    initMinAndMax();
    K_LIMITE = 5;
    resetLimits();
}

void DijkstraKshortest::cleanLinkArray()
{
    for (LinkArray::iterator it=linkArray.begin();it!=linkArray.end();it++)
        while (!it->second.empty())
        {
            delete it->second.back();
            it->second.pop_back();
        }
    linkArray.clear();
}


DijkstraKshortest::~DijkstraKshortest()
{
    cleanLinkArray();
}

void DijkstraKshortest::setLimits(const std::vector<double> & vectorData)
{

    limitsData =maximumCost;
    for (unsigned int i=0;i<vectorData.size();i++)
    {
        if (i>=limitsData.size()) continue;
        limitsData[i].value=vectorData[i];
    }
}



void DijkstraKshortest::addEdge (const NodeId & originNode, const NodeId & last_node,double cost,double delay,double bw,double quality)
{
    LinkArray::iterator it;
    it = linkArray.find(originNode);
    if (it!=linkArray.end())
    {
         for (unsigned int i=0;i<it->second.size();i++)
         {
             if (last_node == it->second[i]->last_node_)
             {
                  it->second[i]->Cost()=cost;
                  it->second[i]->Delay()=delay;
                  it->second[i]->Bandwith()=bw;
                  it->second[i]->Quality()=quality;
                  return;
             }
         }
    }
    Edge *link = new Edge;
    // The last hop is the interface in which we have this neighbor...
    link->last_node() = last_node;
    // Also record the link delay and quality..
    link->Cost()=cost;
    link->Delay()=delay;
    link->Bandwith()=bw;
    link->Quality()=quality;
    linkArray[originNode].push_back(link);
}

void DijkstraKshortest::setRoot(const NodeId & dest_node)
{
    rootNode = dest_node;

}

void DijkstraKshortest::run ()
{
    std::multiset<SetElem> heap;
    routeMap.clear();

    LinkArray::iterator it;
    it = linkArray.find(rootNode);
    if (it==linkArray.end())
        opp_error("Node not found");
    for (int i=0;i<K_LIMITE;i++)
    {
        State state(minimumCost);
        state.label=perm;
        routeMap[rootNode].push_back(state);
    }
    SetElem elem;
    elem.iD=rootNode;
    elem.idx=0;
    elem.cost=minimumCost;
    heap.insert(elem);
    while (!heap.empty())
    {
        SetElem elem=*heap.begin();
        heap.erase(heap.begin());
        RouteMap::iterator it;
        it = routeMap.find(elem.iD);
        if (it==routeMap.end())
            opp_error("node not found in routeMap");
        if ((int)it->second.size()<=elem.idx)
        {
            for (int i = elem.idx -((int)it->second.size()-1);i>=0;i--)
            {
                State state(maximumCost);
                state.label=tent;
                it->second.push_back(state);
            }
        }
        (it->second)[elem.idx].label=perm;
        LinkArray::iterator linkIt=linkArray.find(elem.iD);
        for (unsigned int i=0;i<linkIt->second.size();i++)
        {
            Edge* current_edge= (linkIt->second)[i];
            CostVector cost;
            CostVector maxCost = maximumCost;
            int nextIdx;
            RouteMap::iterator itNext = routeMap.find(current_edge->last_node());
            addCost(cost,current_edge->cost,(it->second)[elem.idx].cost);
            if (!limitsData.empty())
            {
                if (limitsData<cost)
                    continue;
            }
            if (itNext==routeMap.end() || (itNext!=routeMap.end() && (int)itNext->second.size()<K_LIMITE))
            {
                State state;
                state.idPrev=elem.iD;
                state.idPrevIdx=elem.idx;
                state.cost=cost;
                state.label=tent;
                routeMap[current_edge->last_node()].push_back(state);
                nextIdx = routeMap[current_edge->last_node()].size()-1;
                SetElem newElem;
                newElem.iD=current_edge->last_node();
                newElem.idx=nextIdx;
                newElem.cost=cost;
                heap.insert(newElem);
            }
            else
            {
                for (unsigned i=0;i<itNext->second.size();i++)
                {
                    if ((maxCost<itNext->second[i].cost)&&(itNext->second[i].label==tent))
                    {
                        maxCost = itNext->second[i].cost;
                        nextIdx=i;
                    }
                }
            }
            if (cost<maxCost)
            {
                itNext->second[nextIdx].cost=cost;
                itNext->second[nextIdx].idPrev=elem.iD;
                itNext->second[nextIdx].idPrevIdx=elem.idx;
                SetElem newElem;
                newElem.iD=current_edge->last_node();
                newElem.idx=nextIdx;
                newElem.cost=cost;
                heap.insert(newElem);
            }
        }
    }
}

void DijkstraKshortest::runUntil (const NodeId &target)
{
    std::multiset<SetElem> heap;
    routeMap.clear();

    LinkArray::iterator it;
    it = linkArray.find(rootNode);
    if (it==linkArray.end())
        opp_error("Node not found");
    for (int i=0;i<K_LIMITE;i++)
    {
        State state(minimumCost);
        state.label=perm;
        routeMap[rootNode].push_back(state);
    }
    SetElem elem;
    elem.iD=rootNode;
    elem.idx=0;
    elem.cost=minimumCost;
    heap.insert(elem);
    while (!heap.empty())
    {
        SetElem elem=*heap.begin();
        heap.erase(heap.begin());
        RouteMap::iterator it;
        it = routeMap.find(elem.iD);
        if (it==routeMap.end())
            opp_error("node not found in routeMap");
        if ((int)it->second.size()<=elem.idx)
        {
            for (int i = elem.idx -((int)it->second.size()-1);i>=0;i--)
            {
                State state(maximumCost);
                state.label=tent;
                it->second.push_back(state);
            }
        }
        (it->second)[elem.idx].label=perm;
        if ((int)it->second.size()==K_LIMITE && target==elem.iD)
            return;
        LinkArray::iterator linkIt=linkArray.find(elem.iD);
        for (unsigned int i=0;i<linkIt->second.size();i++)
        {
            Edge* current_edge= (linkIt->second)[i];
            CostVector cost;
            CostVector maxCost = maximumCost;
            int nextIdx;
            RouteMap::iterator itNext = routeMap.find(current_edge->last_node());
            addCost(cost,current_edge->cost,(it->second)[elem.idx].cost);
            if (!limitsData.empty())
            {
                if (limitsData<cost)
                    continue;
            }
            if (itNext==routeMap.end() || (itNext!=routeMap.end() && (int)itNext->second.size()<K_LIMITE))
            {
                State state;
                state.idPrev=elem.iD;
                state.idPrevIdx=elem.idx;
                state.cost=cost;
                state.label=tent;
                routeMap[current_edge->last_node()].push_back(state);
                nextIdx = routeMap[current_edge->last_node()].size()-1;
                SetElem newElem;
                newElem.iD=current_edge->last_node();
                newElem.idx=nextIdx;
                newElem.cost=cost;
                heap.insert(newElem);
            }
            else
            {
                for (unsigned i=0;i<itNext->second.size();i++)
                {
                    if ((maxCost<itNext->second[i].cost)&&(itNext->second[i].label==tent))
                    {
                        maxCost = itNext->second[i].cost;
                        nextIdx=i;
                    }
                }
            }
            if (cost<maxCost)
            {
                itNext->second[nextIdx].cost=cost;
                itNext->second[nextIdx].idPrev=elem.iD;
                itNext->second[nextIdx].idPrevIdx=elem.idx;
                SetElem newElem;
                newElem.iD=current_edge->last_node();
                newElem.idx=nextIdx;
                newElem.cost=cost;
                heap.insert(newElem);
            }
        }
    }
}

int DijkstraKshortest::getNumRoutes(const NodeId &nodeId)
{
    RouteMap::iterator it;
    it = routeMap.find(nodeId);
    if (it==routeMap.end())
        return -1;
    return (int)it->second.size();
}

bool DijkstraKshortest::getRoute(const NodeId &nodeId,std::vector<NodeId> &pathNode,int k)
{
    RouteMap::iterator it = routeMap.find(nodeId);
    if (it==routeMap.end())
        return false;
    if (k>=(int)it->second.size())
        return false;
    std::vector<NodeId> path;
    NodeId currentNode = nodeId;
    int idx=it->second[k].idPrevIdx;
    while (currentNode!=rootNode)
    {
        path.push_back(currentNode);
        currentNode = it->second[idx].idPrev;
        idx=it->second[idx].idPrevIdx;
        it = routeMap.find(currentNode);
        if (it==routeMap.end())
            opp_error("error in data");
        if (idx>=(int)it->second.size())
            opp_error("error in data");
    }
    pathNode.clear();
    while (!path.empty())
    {
        pathNode.push_back(path.back());
        path.pop_back();
    }
    return true;
}

void DijkstraKshortest::setFromTopo(const cTopology *topo)
{
    for (int i=0; i<topo->getNumNodes(); i++)
    {
    	cTopology::Node *node = const_cast<cTopology*>(topo)->getNode(i);
    	NodeId id=node->getModuleId();
    	for (int j=0; j<node->getNumOutLinks(); j++)
    	{

    		NodeId idNex = node->getLinkOut(j)->getRemoteNode()->getModuleId();
    		double cost=node->getLinkOut(j)->getWeight();
    		addEdge (id,idNex,cost,0,1000,0);
    	}
    }
}

