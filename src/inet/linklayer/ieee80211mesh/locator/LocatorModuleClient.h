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

#ifndef LOCATORMODULECLIENT_H_
#define LOCATORMODULECLIENT_H_

#include <csimplemodule.h>
#include <map>
#include <set>
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/arp/IARP.h"
#include "inet/networklayer/common/IInterfaceTable.h"


namespace inet {
namespace ieee80211 {
class LocatorModuleClient : public cSimpleModule, protected cListener
{
    protected:
        IInterfaceTable *itable;
        IIPv4RoutingTable *rt;
        int port;
        int interfaceId;
        UDPSocket * socket;
        InterfaceEntry * iface;
        IARP *arp;
    public:
        LocatorModuleClient();
        virtual ~LocatorModuleClient();
        virtual void handleMessage(cMessage *msg);
        virtual void initialize(int stage);
        virtual int numInitStages() const {return 4;}
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
};

}

}

#endif /* LOCATORMODULE_H_ */
