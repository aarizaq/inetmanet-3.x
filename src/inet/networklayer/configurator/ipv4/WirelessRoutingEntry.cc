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

#include <WirelessRoutingEntry.h>
#include "inet/common/WirelessNumHops.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "IIPv4RoutingTable.h"
#include "IInterfaceTable.h"

namespace inet{

WirelessRoutingEntry::WirelessRoutingEntry()
{
    // TODO Auto-generated constructor stub

}

WirelessRoutingEntry::~WirelessRoutingEntry()
{
    // TODO Auto-generated destructor stub
}

void WirelessRoutingEntry::fillTables(double distance)
{

    cModule* host = findContainingNode(dynamic_cast<cModule *>(this->getOwner()));
    IIPv4RoutingTable *rt = L3AddressResolver().findIPv4RoutingTableOf(host);
    IInterfaceTable *ift = L3AddressResolver().findInterfaceTableOf(host);
    InterfaceEntry *iface = nullptr;
    for (int i = 0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterfaceByNodeInputGateId(i);
        const char *name = ie->getName();
        if (strcmp(name, "wlan") == 0)
        {
            iface = ie;
            break;
        }
    }

    WirelessNumHops wn;
    wn.fillRoutingTables(distance);
    wn.setRoot(rt->getRouterId());
    wn.run();
    for(unsigned int i = 0;i<wn.getNumRoutes(); i++)
    {
        std::deque<IPv4Address> route;
        wn.getRoute(i, route);
        if (!route.empty())
        {
            IPv4Route *entry = new IPv4Route();
            entry->setDestination(route[route.size()-1]);
            entry->setGateway(route[0]);
            entry->setNetmask(IPv4Address::ALLONES_ADDRESS);
            entry->setMetric(route.size());
            entry->setInterface(iface);
            rt->addRoute(entry);
        }
    }
}

} //namespace
