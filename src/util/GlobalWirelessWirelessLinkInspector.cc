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

#include "GlobalWirelessWirelessLinkInspector.h"
GlobalWirelessWirelessLinkInspector::CostMap* GlobalWirelessWirelessLinkInspector::costMap = NULL;

GlobalWirelessWirelessLinkInspector::GlobalWirelessWirelessLinkInspector()
{
    // TODO Auto-generated constructor stub
    costMap = NULL;
}

GlobalWirelessWirelessLinkInspector::~GlobalWirelessWirelessLinkInspector()
{
    // TODO Auto-generated destructor stub
    if (costMap != NULL)
    {
        while (!costMap->empty())
        {
            delete costMap->begin()->second;
            costMap->erase(costMap->begin());
        }
        delete costMap;
    }
}

void GlobalWirelessWirelessLinkInspector::initialize()
{

    if (costMap == NULL)
    {
        costMap = new CostMap;
    }
    else
    {
        opp_error("more that an instance of GlobalWirelessWirelessLinkInspector exist");
    }
}

void GlobalWirelessWirelessLinkInspector::handleMessage(cMessage *msg)
{
    opp_error ("GlobalWirelessWirelessLinkInspector has received a packet");
}

void GlobalWirelessWirelessLinkInspector::setLinkCost(const Uint128& org,const Uint128& dest,const Link &link)
{
    if (!costMap)
        return;
    CostMap::iterator it = costMap->find(org);
    if (it != costMap->end())
    {
        NodeLinkCost *nLinkCost = it->second;
        NodeLinkCost::iterator it2 = nLinkCost->find(dest);
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

bool GlobalWirelessWirelessLinkInspector::getLinkCost(const Uint128& org,const Uint128& dest,Link &link)
{

    if (!costMap)
        return false;
    CostMap::iterator it = costMap->find(org);
    if (it != costMap->end())
    {
        NodeLinkCost *nLinkCost = it->second;
        NodeLinkCost::iterator it2 = nLinkCost->find(dest);
        if (it2 != nLinkCost->end())
        {
            link = it2->second;
            return true;
        }
    }
    return false;
}

bool GlobalWirelessWirelessLinkInspector::getLinkCost(const std::vector<Uint128>& path, Link &link)
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
    return false;
}
