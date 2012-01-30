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

#include "LocatorModule.h"
#include "Ieee80211MgmtAP.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "locatorPkt_m.h"

simsignal_t LocatorModule::locatorChangeSignal = SIMSIGNAL_NULL;

LocatorModule::LocatorMapIp LocatorModule::globalLocatorMapIp;

LocatorModule::LocatorMapMac LocatorModule::globalLocatorMapMac;

LocatorModule::LocatorModule()
{
    // TODO Auto-generated constructor stub
    arp = NULL;
    rt = NULL;
    itable = NULL;
    isInMacLayer = true;
}

LocatorModule::~LocatorModule()
{
    // TODO Auto-generated destructor stub
}

void LocatorModule::handleMessage(cMessage *msg)
{
   delete msg;
}

void LocatorModule::initialize()
{
    arp = ArpAccess().get();
    rt = RoutingTableAccess().get();
    itable = InterfaceTableAccess().get();
}


void  LocatorModule::sendMessage(const MACAddress &staMac,const MACAddress &apMac,const IPv4Address &staIp,const IPv4Address &apIp)
{
    LocatorPkt *pkt = new LocatorPkt();
    pkt->setApMACAddress(apMac);
    pkt->setStaMACAddress(staMac);
    pkt->setApIPAddress(apIp);
    pkt->setStaIPAddress(staIp);
    send(pkt,"outGate");
}

void LocatorModule::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
    if(category == NF_L2_AP_DISSOCIATED || category == NF_L2_AP_ASSOCIATED)
    {
        Ieee80211MgmtAP::NotificationInfoSta * infoSta = dynamic_cast<Ieee80211MgmtAP::NotificationInfoSta *>(const_cast<cObject*> (details));
        if (infoSta)
        {
            const IPv4Address add = arp->getInverseAddressResolution(infoSta->getStaAddress());
            InterfaceEntry * ie = NULL;
            for (int i =0 ; i < itable->getNumInterfaces();i++)
            {
                if (itable->getInterface(i)->getMacAddress() == infoSta->getApAddress())
                {
                    ie = itable->getInterface(i);
                    break;
                }
            }

            if (!add.isUnspecified() && ie)
            {
                if(category == NF_L2_AP_ASSOCIATED)
                {
                    IPv4Route *entry = new IPv4Route();
                    entry->setDestination(add);
                    entry->setGateway(add);
                    entry->setNetmask(IPv4Address::ALLONES_ADDRESS);
                    entry->setInterface(ie);
                    rt->addRoute(entry);
                    LocEntry locEntry;
                    locEntry.macAddr = infoSta->getStaAddress();
                    locEntry.apMacAddr = myMacAddress;
                    locEntry.ipAddr = add;
                    locEntry.apIpAddr = myIpAddress;

                    locatorMapMac[infoSta->getStaAddress()] = locEntry;
                    locatorMapIp[add] = locEntry;
                    globalLocatorMapMac[infoSta->getStaAddress()] = locEntry;
                    globalLocatorMapIp[add] = locEntry;
                }
                else if (category == NF_L2_AP_ASSOCIATED)
                {
                    for (int i = 0; i < rt->getNumRoutes(); i++)
                    {
                        IPv4Route *entry = rt->getRoute(i);
                        if (entry->getDestination() == add)
                        {
                            rt->deleteRoute(entry);
                        }
                    }
                    MapIpIteartor itIp;
                    MapMacIterator itMac;
                    itMac = locatorMapMac.find(infoSta->getStaAddress());
                    if (itMac != locatorMapMac.end())
                        locatorMapMac.erase(itMac);

                    itIp = locatorMapIp.find(add);
                    if (itIp != locatorMapIp.end())
                        locatorMapIp.erase(itIp);

                    itMac = globalLocatorMapMac.find(infoSta->getStaAddress());
                    if (itMac != globalLocatorMapMac.end())
                        globalLocatorMapMac.erase(itMac);

                    itIp = globalLocatorMapIp.find(add);
                    if (itIp != globalLocatorMapIp.end())
                        globalLocatorMapIp.erase(itIp);
                }
            }
        }
    }
}

void LocatorModule::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{

}

const MACAddress  LocatorModule::getLocatorMacToMac(const MACAddress & add)
{
    MapMacIterator itMac = globalLocatorMapMac.find(add);
    if (itMac != globalLocatorMapMac.end())
        return itMac->second.apMacAddr;
    return MACAddress::UNSPECIFIED_ADDRESS;
}

const IPv4Address LocatorModule::getLocatorMacToIp(const MACAddress & add)
{
    MapMacIterator itMac = globalLocatorMapMac.find(add);
    if (itMac != globalLocatorMapMac.end())
    {
        return itMac->second.apIpAddr;
    }
    return IPv4Address::UNSPECIFIED_ADDRESS;
}

const IPv4Address LocatorModule::getLocatorIpToIp(const IPv4Address &add)
{
    MapIpIteartor itIp = globalLocatorMapIp.find(add);
    if (itIp != globalLocatorMapIp.end())
    {
        return itIp->second.apIpAddr;
    }
    return IPv4Address::UNSPECIFIED_ADDRESS;
}

const MACAddress  LocatorModule::getLocatorIpToMac(const IPv4Address &add)
{
    MapIpIteartor itIp = globalLocatorMapIp.find(add);
    if (itIp != globalLocatorMapIp.end())
    {
        return itIp->second.apMacAddr;
    }
    return MACAddress::UNSPECIFIED_ADDRESS;
}



