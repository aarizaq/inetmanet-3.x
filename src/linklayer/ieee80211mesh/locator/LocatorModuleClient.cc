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

#include "LocatorModuleClient.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "locatorPkt_m.h"
#include "UDP.h"
#include "IPv4InterfaceData.h"
#include "Ieee80211MgmtSTA.h"


LocatorModuleClient::LocatorModuleClient()
{

    rt = NULL;
    itable = NULL;
    socket = NULL;
    iface = NULL;
}

LocatorModuleClient::~LocatorModuleClient()
{
   if (socket)
       delete socket;
}

void LocatorModuleClient::handleMessage(cMessage *msg)
{
    LocatorPkt *pkt = dynamic_cast<LocatorPkt*>(msg);
    if (pkt)
    {
        if (rt->isLocalAddress(pkt->getOrigin().getIPv4()))
        {
            delete pkt;
            return;
        }
        if (pkt->getOpcode() != RequestAddress)
        {
            delete pkt;
            return;
        }
        MACAddress destAddr = pkt->getStaMACAddress();
        IPv4Address iv4Addr;
        UDPDataIndication *udpCtrl = check_and_cast<UDPDataIndication*>(pkt->removeControlInfo());

        for (int i = 0; i < itable->getNumInterfaces(); i++)
        {
            if (itable->getInterface(i)->getMacAddress() == destAddr)
            {
                // ignore, is local information and this information has been set by the  receiveChangeNotification
                iv4Addr = itable->getInterface(i)->ipv4Data()->getIPAddress();
                break;
            }
        }
        if (iv4Addr.isUnspecified())
        { // not for us
            delete pkt;
            return;
        }
        pkt->setOpcode(ReplyAddress);
        pkt->setStaIPAddress(iv4Addr);
        socket->sendTo(pkt, udpCtrl->getSrcAddr(), port, interfaceId);
    }
    else
        delete msg;
}

void LocatorModuleClient::initialize(int stage)
{
    if (stage!=3)
        return;
    rt = RoutingTableAccess().get();
    itable = InterfaceTableAccess().get();
    arp = ArpAccess().get();

    InterfaceEntry *ie = itable->getInterfaceByName(this->par("iface"));
    if (ie)
        iface = ie;
    if (dynamic_cast<UDP*>(gate("outGate")->getPathEndGate()->getOwnerModule()))
    {
        // bind the client to the udp port
        socket = new UDPSocket();
        socket->setOutputGate(gate("outGate"));
        port = par("locatorPort").longValue();
        socket->bind(port);
        socket->setBroadcast(true);

    }
}


void LocatorModuleClient::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
    if(category == NF_L2_ASSOCIATED)
    {
    }
}

