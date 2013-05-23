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

#include "WirelessGetNeig.h"
#include "MobilityAccess.h"
#include "IPvXAddressResolver.h"
#include "IInterfaceTable.h"
#include "IPv4InterfaceData.h"

WirelessGetNeig::WirelessGetNeig()
{
    cTopology topo("topo");
    topo.extractByProperty("node");
    cModule *mod = dynamic_cast<cModule*> (getOwner());
    for (mod = dynamic_cast<cModule*> (getOwner())->getParentModule(); mod != 0; mod = mod->getParentModule())
    {
            cProperties *properties = mod->getProperties();
            if (properties && properties->getAsBool("node"))
                break;
    }
    listNodes.clear();
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
        IPv4Address addr = IPvXAddressResolver().getAddressFrom(info.itable).get4();
        listNodes[addr.getInt()]= info;
    }
}

WirelessGetNeig::~WirelessGetNeig()
{
    // TODO Auto-generated destructor stub
    listNodes.clear();
}


void WirelessGetNeig::getNeighbours(const IPv4Address &node, std::vector<IPv4Address>&list, const double &distance)
{
    list.clear();

    ListNodes::iterator it = listNodes.find(node.getInt());
    if (it == listNodes.end())
        opp_error("node not found");

    Coord pos = it->second.mob->getCurrentPosition();
    for (it = listNodes.begin(); it != listNodes.end(); ++it)
    {
        if (it->first == node.getInt())
            continue;
        if (pos.distance(it->second.mob->getCurrentPosition()) < distance)
        {
            list.push_back(IPv4Address(it->first));
        }
    }
}
