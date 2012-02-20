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
#include "IRoutingTable.h"
#include "IInterfaceTable.h"
#include "INotifiable.h"
#include "UDPSocket.h"
#include "INotifiable.h"
#include "ARP.h"


class LocatorModuleClient : public cSimpleModule, protected INotifiable
{
    protected:
        IInterfaceTable *itable;
        IRoutingTable *rt;
        int port;
        int interfaceId;
        UDPSocket * socket;
        InterfaceEntry * iface;
        ARP *arp;
    public:
        LocatorModuleClient();
        virtual ~LocatorModuleClient();
        virtual void handleMessage(cMessage *msg);
        virtual void initialize(int stage);
        virtual int numInitStages() const {return 4;}
        virtual void receiveChangeNotification(int category, const cObject *details);
};

#endif /* LOCATORMODULE_H_ */
