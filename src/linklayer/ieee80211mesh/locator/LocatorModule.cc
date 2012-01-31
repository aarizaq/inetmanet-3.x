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
#include "UDP.h"
#include "IPv4InterfaceData.h"

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
    socket = NULL;
    useGlobal = false;
    mySequence = 0;
}

LocatorModule::~LocatorModule()
{
   locatorMapIp.clear();
   globalLocatorMapIp.clear();
   locatorMapMac.clear();
   globalLocatorMapMac.clear();
   if (socket)
       delete socket;
   sequenceMap.clear();
}

void LocatorModule::handleMessage(cMessage *msg)
{
    if (useGlobal)
    {
        delete msg;
        return;
    }

    LocatorPkt *pkt = dynamic_cast<LocatorPkt*> (msg);
    if (pkt)
    {
        if (socket)
        {
            if (pkt->getOrigin() == myIpAddress)
            {
                delete pkt;
                return;
            }
            std::map<IPv4Address,unsigned int>::iterator it = sequenceMap.find(pkt->getOrigin());
            if (it!=sequenceMap.end())
            {
                if (it->second >= pkt->getSequence())
                {
                    delete pkt;
                    return;
                }
            }
            sequenceMap[pkt->getOrigin()] = pkt->getSequence();
        }
        IPv4Address staIpaddr = pkt->getStaIPAddress();
        IPv4Address apIpaddr = pkt->getApIPAddress();
        MACAddress staAddr = pkt->getStaMACAddress();
        MACAddress apAddr = pkt->getApMACAddress();

        if (staIpaddr.isUnspecified() && staAddr.isUnspecified())
        {
            delete pkt;
            return;
        }

        for (int i =0 ; i < itable->getNumInterfaces();i++)
        {
            if (itable->getInterface(i)->getMacAddress() == staAddr || itable->getInterface(i)->getMacAddress() == apAddr)
            {
                // ignore, is local information and this information has been set by the  receiveChangeNotification
                delete pkt;
                return;
            }
        }

        if (staIpaddr.isUnspecified())
            staIpaddr = arp->getInverseAddressResolution(staAddr);
        if (staAddr.isUnspecified())
            staAddr = arp->getDirectAddressResolution(staIpaddr);
        if (apIpaddr.isUnspecified())
            apIpaddr = arp->getInverseAddressResolution(apAddr);
        if (apAddr.isUnspecified())
            apAddr = arp->getDirectAddressResolution(apIpaddr);
        if ( pkt->getOpcode() == LocatorAssoc)
            setTables(apAddr,staAddr,apIpaddr,staIpaddr,ASSOCIATION,NULL);
        else if (pkt->getOpcode() == LocatorDisAssoc)
            setTables(apAddr,staAddr,apIpaddr,staIpaddr,DISASSOCIATION,NULL);
    }
    if (socket)
        socket->sendTo(pkt,"255.255.255.255",port, interfaceId);
    else
        delete msg;
}

void LocatorModule::initialize(int stage)
{
    if (stage!=3)
        return;

    arp = ArpAccess().get();
    rt = RoutingTableAccess().get();
    itable = InterfaceTableAccess().get();

    if (dynamic_cast<UDP*>(gate("outGate")->getPathEndGate()->getOwnerModule()))
    {
        // bind the client to the udp port
        socket = new UDPSocket();
        socket->setOutputGate(gate("outGate"));
        port = par("locatorPort").longValue();
        socket->bind(port);
        socket->setBroadcast(true);
        InterfaceEntry *ie = itable->getInterfaceByName(this->par("iface"));
        useGlobal = par("useGlobal").boolValue();
        interfaceId = ie->getInterfaceId();
        myMacAddress =  ie->getMacAddress();
        myIpAddress = ie->ipv4Data()->getIPAddress();
    }
}


void  LocatorModule::sendMessage(const MACAddress &apMac,const MACAddress &staMac,const IPv4Address &apIp,const IPv4Address &staIp,const Action &action)
{
    if (useGlobal)
        return;
    LocatorPkt *pkt = new LocatorPkt();
    pkt->setApMACAddress(apMac);
    pkt->setStaMACAddress(staMac);
    pkt->setApIPAddress(apIp);
    pkt->setStaIPAddress(staIp);
    if (action == ASSOCIATION)
        pkt->setOpcode(LocatorAssoc);
    else if (action == DISASSOCIATION)
        pkt->setOpcode(LocatorDisAssoc);

    if (socket)
    {
        pkt->setByteLength(pkt->getByteLength()+8);
        pkt->setOrigin(myIpAddress);
        pkt->setSequence(mySequence);
        mySequence++;

        socket->sendTo(pkt,"255.255.255.255",port, interfaceId);
    }
    else
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
            if (category == NF_L2_AP_ASSOCIATED)
            {
                setTables(myMacAddress,infoSta->getStaAddress(),myIpAddress,add,ASSOCIATION,ie);
                sendMessage(myMacAddress,infoSta->getStaAddress(),myIpAddress,add,ASSOCIATION);
            }
            else if (category == NF_L2_DISSOCIATED)
            {
                setTables(myMacAddress,infoSta->getStaAddress(),myIpAddress,add,DISASSOCIATION,ie);
                sendMessage(myMacAddress,infoSta->getStaAddress(),myIpAddress,add,DISASSOCIATION);
            }
        }
    }
}

void LocatorModule::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{

}

const MACAddress  LocatorModule::getLocatorMacToMac(const MACAddress & add)
{
    if (useGlobal)
    {
        MapMacIterator itMac = globalLocatorMapMac.find(add);
        if (itMac != globalLocatorMapMac.end())
            return itMac->second.apMacAddr;
    }
    else
    {
        MapMacIterator itMac = locatorMapMac.find(add);
        if (itMac != locatorMapMac.end())
            return itMac->second.apMacAddr;
    }
    return MACAddress::UNSPECIFIED_ADDRESS;

}

const IPv4Address LocatorModule::getLocatorMacToIp(const MACAddress & add)
{
    if (useGlobal)
    {
        MapMacIterator itMac = globalLocatorMapMac.find(add);
        if (itMac != globalLocatorMapMac.end())
            return itMac->second.apIpAddr;
    }
    else
    {
        MapMacIterator itMac = locatorMapMac.find(add);
        if (itMac != locatorMapMac.end())
            return itMac->second.apIpAddr;
    }
    return IPv4Address::UNSPECIFIED_ADDRESS;
}

const IPv4Address LocatorModule::getLocatorIpToIp(const IPv4Address &add)
{
    if (useGlobal)
    {
        MapIpIteartor itIp = globalLocatorMapIp.find(add);
        if (itIp != globalLocatorMapIp.end())
            return itIp->second.apIpAddr;
    }
    else
    {
        MapIpIteartor itIp = locatorMapIp.find(add);
        if (itIp != locatorMapIp.end())
            return itIp->second.apIpAddr;
    }
    return IPv4Address::UNSPECIFIED_ADDRESS;
}

const MACAddress  LocatorModule::getLocatorIpToMac(const IPv4Address &add)
{
    if (useGlobal)
    {
        MapIpIteartor itIp = globalLocatorMapIp.find(add);
        if (itIp != globalLocatorMapIp.end())
            return itIp->second.apMacAddr;
    }
    else
    {
        MapIpIteartor itIp = locatorMapIp.find(add);
        if (itIp != locatorMapIp.end())
            return itIp->second.apMacAddr;
    }
    return MACAddress::UNSPECIFIED_ADDRESS;
}


void LocatorModule::modifyInformationMac(const MACAddress &APaddr, const MACAddress &STAaddr, const Action &action)
{
    const IPv4Address staIpadd = arp->getInverseAddressResolution(STAaddr);
    const IPv4Address apIpAddr = arp->getInverseAddressResolution(APaddr);
    InterfaceEntry * ie = NULL;
    for (int i =0 ; i < itable->getNumInterfaces();i++)
    {
        if (itable->getInterface(i)->getMacAddress() == APaddr)
        {
            ie = itable->getInterface(i);
            break;
        }
    }
    setTables(APaddr,STAaddr,apIpAddr,staIpadd,action,ie);
}

void LocatorModule::modifyInformationIp(const IPv4Address &apIpAddr, const IPv4Address &staIpAddr, const Action &action)
{
    const MACAddress STAaddr = arp->getDirectAddressResolution(staIpAddr);
    const MACAddress APaddr = arp->getDirectAddressResolution(apIpAddr);
    InterfaceEntry * ie = NULL;
    for (int i =0 ; i < itable->getNumInterfaces();i++)
    {
        if (itable->getInterface(i)->getMacAddress() == APaddr)
        {
            ie = itable->getInterface(i);
            break;
        }
    }
    setTables(APaddr,STAaddr,apIpAddr,staIpAddr,action,ie);
}

void LocatorModule::setTables(const MACAddress & APaddr, const MACAddress &STAaddr, const IPv4Address & apIpAddr, const IPv4Address & staIpAddr, const Action &action, InterfaceEntry *ie)
{
    if(action == ASSOCIATION)
    {
        const IPv4Address addAP = arp->getInverseAddressResolution(APaddr);
        LocEntry locEntry;
        locEntry.macAddr = STAaddr;
        locEntry.apMacAddr = APaddr;
        locEntry.ipAddr = staIpAddr;
        locEntry.apIpAddr = apIpAddr;
        if (!STAaddr.isUnspecified())
        {
            locatorMapMac[STAaddr] = locEntry;
            globalLocatorMapMac[STAaddr] = locEntry;
        }
        if (!staIpAddr.isUnspecified())
        {
            locatorMapIp[staIpAddr] = locEntry;
            globalLocatorMapIp[staIpAddr] = locEntry;
        }
        if (!staIpAddr.isUnspecified())
        {
            if  (ie)
            {
                IPv4Route *entry = new IPv4Route();
                entry->setDestination(apIpAddr);
                entry->setGateway(apIpAddr);
                entry->setNetmask(IPv4Address::ALLONES_ADDRESS);
                entry->setInterface(ie);
                rt->addRoute(entry);
            }
            else
            {
                for (int i = 0; i < rt->getNumRoutes(); i++)
                {
                    IPv4Route *entry = rt->getRoute(i);
                    if (entry->getDestination() == staIpAddr)
                    {
                        rt->deleteRoute(entry);
                    }
                }
            }
        }
    }
    else if (action == DISASSOCIATION)
    {
        if (!staIpAddr.isUnspecified())
        {
            for (int i = 0; i < rt->getNumRoutes(); i++)
            {
                IPv4Route *entry = rt->getRoute(i);
                if (entry->getDestination() == staIpAddr)
                {
                    rt->deleteRoute(entry);
                }
            }
        }
        MapIpIteartor itIp;
        MapMacIterator itMac;
        if (!STAaddr.isUnspecified())
        {
            itMac = locatorMapMac.find(STAaddr);
            if (itMac != locatorMapMac.end())
                locatorMapMac.erase(itMac);
            if (itMac != globalLocatorMapMac.end())
                globalLocatorMapMac.erase(itMac);
        }
        if (!staIpAddr.isUnspecified())
        {
            itIp = locatorMapIp.find(staIpAddr);
            if (itIp != locatorMapIp.end())
                locatorMapIp.erase(itIp);
            itMac = globalLocatorMapMac.find(STAaddr);
            itIp = globalLocatorMapIp.find(staIpAddr);
            if (itIp != globalLocatorMapIp.end())
                globalLocatorMapIp.erase(itIp);
        }
    }
}


