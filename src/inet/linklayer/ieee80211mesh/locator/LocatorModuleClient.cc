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

#include "inet/linklayer/ieee80211mesh/locator/LocatorModuleClient.h"
#include "inet/linklayer/ieee80211mesh/locator/locatorPkt_m.h"
#include "inet/transportlayer/udp/UDP.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSTA.h"



namespace inet {

namespace ieee80211 {


LocatorModuleClient::LocatorModuleClient()
{

    rt = nullptr;
    itable = nullptr;
    socket = nullptr;
    iface = nullptr;
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
        if (rt->isLocalAddress(pkt->getOrigin().toIPv4()))
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
        UDPSocket::SendOptions options;
        options.outInterfaceId = interfaceId;
        socket->sendTo(pkt, udpCtrl->getSrcAddr(), port, &options);
    }
    else
        delete msg;
}

void LocatorModuleClient::initialize(int stage)
{
    if (stage!=3)
        return;
    arp =  getModuleFromPar<IARP>(par("arpModule"), this);
    rt = findModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
    itable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

    InterfaceEntry *ie = itable->getInterfaceByName(this->par("iface"));
    if (ie)
        iface = ie;
    UDP* udpmodule = dynamic_cast<UDP*>(gate("outGate")->getPathEndGate()->getOwnerModule());
    if (udpmodule)
    {
        // bind the client to the udp port
        socket = new UDPSocket();
        socket->setOutputGate(gate("outGate"));
        port = par("locatorPort").intValue();
        socket->bind(port);
        socket->setBroadcast(true);
    }
}


void LocatorModuleClient::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();
    if(signalID == NF_L2_ASSOCIATED)
    {
    }
}

}

}
