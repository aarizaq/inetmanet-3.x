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
#include "NotificationBoard.h"
#include "LocatorNotificationInfo_m.h"
#include "Ieee802Ctrl_m.h"
#include "ARPPacket_m.h"
#include "Ieee80211Frame_m.h"
#include "GlobalWirelessLinkInspector.h"

simsignal_t LocatorModule::locatorChangeSignal = SIMSIGNAL_NULL;
LocatorModule::LocatorMapIp LocatorModule::globalLocatorMapIp;
LocatorModule::LocatorMapMac LocatorModule::globalLocatorMapMac;
LocatorModule::ApIpSet LocatorModule::globalApIpSet;
LocatorModule::ApSet LocatorModule::globalApSet;

std::ostream& operator<<(std::ostream& os, const LocatorModule::LocEntry& e)
{
    os << " Mac Address " << e.macAddr << "\n";
    os << "IP Address " << e.ipAddr << "\n";
    os << "AP Mac Address"  << e.apMacAddr << "\n";
    os << "AP Ip Address "  << e.apIpAddr << "\n";
    return os;
}

Define_Module(LocatorModule);

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
    LocatorPkt *pkt = dynamic_cast<LocatorPkt*> (msg);

    if (!pkt)
    {
        delete msg;
        return;
    }

    if (pkt->getOpcode() == ReplyAddress)
    {
        processReply(pkt);
        return;
    }
    else if (pkt->getOpcode() == RequestAddress)
    {
        processRequest(pkt);
        return;
    }

    if (useGlobal)
    {
        delete msg;
        return;
    }

    if (pkt)
    {
        if ((pkt->getOrigin().getType() == ManetAddress::IPv4_ADDRESS && pkt->getOrigin().getIPv4() == myIpAddress)
                || (pkt->getOrigin().getType() == ManetAddress::MAC_ADDRESS && pkt->getOrigin().getMAC() == myMacAddress))
        {
            delete pkt;
            return;
        }
        std::map<ManetAddress,unsigned int>::iterator it = sequenceMap.find(pkt->getOrigin());
        if (it!=sequenceMap.end())
        {
            if (it->second >= pkt->getSequence())
            {
                delete pkt;
                return;
            }
        }
        sequenceMap[pkt->getOrigin()] = pkt->getSequence();

        IPv4Address staIpaddr = pkt->getStaIPAddress();
        IPv4Address apIpaddr = pkt->getApIPAddress();
        MACAddress staAddr = pkt->getStaMACAddress();
        MACAddress apAddr = pkt->getApMACAddress();

        apIpSet.find(apIpaddr);
        apSet.insert(apAddr);

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

        if (staIpaddr.isUnspecified() && staAddr.isUnspecified())
            opp_error("error in tables sta mac and sta ip address unknown ");
        if (apAddr.isUnspecified() && apIpaddr.isUnspecified())
            opp_error("error in tables ap mac and ap ip address unknown ");

        if (arp)
        {
            if (staIpaddr.isUnspecified())
                staIpaddr = arp->getInverseAddressResolution(staAddr);
            if (staAddr.isUnspecified())
                staAddr = arp->getDirectAddressResolution(staIpaddr);
            if (apIpaddr.isUnspecified())
                apIpaddr = arp->getInverseAddressResolution(apAddr);
            if (apAddr.isUnspecified())
                apAddr = arp->getDirectAddressResolution(apIpaddr);

            if (staIpaddr.isUnspecified())
                sendRequest(staAddr);

            if (apIpaddr.isUnspecified())
                sendRequest(apAddr);
        }

        if ( pkt->getOpcode() == LocatorAssoc)
            setTables(apAddr,staAddr,apIpaddr,staIpaddr,ASSOCIATION,NULL);
        else if (pkt->getOpcode() == LocatorDisAssoc)
            setTables(apAddr,staAddr,apIpaddr,staIpaddr,DISASSOCIATION,NULL);
    }
    if (socket)
    {
        if (pkt && pkt->getControlInfo())
            delete pkt->removeControlInfo();
        socket->sendTo(pkt,IPv4Address::ALLONES_ADDRESS,port, interfaceId);
    }
    else
    {
        delete msg;
    }
}

void LocatorModule::processReply(cPacket* msg)
{
    LocatorPkt *pkt = dynamic_cast<LocatorPkt*> (msg);
    MACAddress destAddr = pkt->getStaMACAddress();
    IPv4Address iv4Addr = pkt->getStaIPAddress();
    MapMacIterator itmac = locatorMapMac.find(destAddr);
    MapIpIteartor itip = locatorMapIp.find(iv4Addr);
    if (itmac == locatorMapMac.end())
    {
        // is ap?
        ApSetIterator it = globalApSet.find(destAddr);
        if (it == globalApSet.end())
            opp_error("error in tables \n");

        for (MapMacIterator itMac = globalLocatorMapMac.begin(); itMac != globalLocatorMapMac.end(); itMac++)
        {
            if (itMac->second.apMacAddr == destAddr)
                itMac->second.apIpAddr = iv4Addr;
        }

        for (MapMacIterator itMac = locatorMapMac.begin(); itMac != locatorMapMac.end(); itMac++)
        {
            if (itMac->second.apMacAddr == destAddr)
                itMac->second.apIpAddr = iv4Addr;
        }

    }
    else
    {
        if (itmac->second.ipAddr.isUnspecified())
        {
            itmac->second.ipAddr = iv4Addr;
        }
        if (itip != locatorMapIp.end())
        {
            if (itip->second.ipAddr.isUnspecified())
                itip->second.ipAddr = iv4Addr;
        }
        else
            locatorMapIp[iv4Addr] = itmac->second;
    }

    itmac = globalLocatorMapMac.find(destAddr);
    itip = globalLocatorMapIp.find(iv4Addr);
    if (itmac == globalLocatorMapMac.end())
    {
        // is ap?
        ApSetIterator it = globalApSet.find(destAddr);
        if (it == globalApSet.end())
            opp_error("error in tables \n");

        for (MapMacIterator itMac = globalLocatorMapMac.begin(); itMac != globalLocatorMapMac.end(); itMac++)
            if (itMac->second.apMacAddr == destAddr)
                itMac->second.apIpAddr = iv4Addr;

        for (MapMacIterator itMac = locatorMapMac.begin(); itMac != locatorMapMac.end(); itMac++)
            if (itMac->second.apMacAddr == destAddr)
                itMac->second.apIpAddr = iv4Addr;

    }
    else
    {
        if (itmac->second.ipAddr.isUnspecified())
        {
            itmac->second.ipAddr = iv4Addr;
        }
        if (itip != globalLocatorMapIp.end())
        {
            if (itip->second.ipAddr.isUnspecified())
                itip->second.ipAddr = iv4Addr;
        }
        else
            globalLocatorMapIp[iv4Addr] = itmac->second;
    }
}

void LocatorModule::processRequest(cPacket* msg)
{
    LocatorPkt *pkt = dynamic_cast<LocatorPkt*> (msg);

    if ((pkt->getOrigin().getType() == ManetAddress::IPv4_ADDRESS && pkt->getOrigin().getIPv4() == myIpAddress)
                  || (pkt->getOrigin().getType() == ManetAddress::MAC_ADDRESS && pkt->getOrigin().getMAC() == myMacAddress))
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
             if (itable->getInterface(i)->ipv4Data())
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


void LocatorModule::initialize(int stage)
{
    if (stage!=3)
        return;

    arp = ArpAccess().getIfExists();
    rt = RoutingTableAccess().getIfExists();
    itable = InterfaceTableAccess().get();
    InterfaceEntry *ie = NULL;
    if (dynamic_cast<UDP*>(gate("outGate")->getPathEndGate()->getOwnerModule()))
    {
        // bind the client to the udp port
        socket = new UDPSocket();
        socket->setOutputGate(gate("outGate"));
        port = par("locatorPort").longValue();
        socket->bind(port);
        socket->setBroadcast(true);
        isInMacLayer = false;
        ie = itable->getInterfaceByName(this->par("iface"));
        inMacLayer = false;
    }
    else
    {
        char *interfaceName = new char[strlen(getParentModule()->getFullName()) + 1];
        char *d = interfaceName;
        for (const char *s = getParentModule()->getFullName(); *s; s++)
            if (isalnum(*s))
                *d++ = *s;
        *d = '\0';
        ie = itable->getInterfaceByName(interfaceName);
        delete [] interfaceName;
        inMacLayer = true;
    }

    useGlobal = par("useGlobal").boolValue();
    if (ie)
    {
        interfaceId = ie->getInterfaceId();
        myMacAddress = ie->getMacAddress();
        if (ie->ipv4Data())
            myIpAddress = ie->ipv4Data()->getIPAddress();
    }
    else
    {
        opp_error("iface not found");
    }

    WATCH_MAP(globalLocatorMapIp);
    WATCH_MAP(globalLocatorMapMac);
    WATCH_MAP(locatorMapIp);
    WATCH_MAP(locatorMapMac);

    nb = NotificationBoardAccess().get();
    nb->subscribe(this,NF_L2_AP_DISASSOCIATED);
    nb->subscribe(this,NF_L2_AP_ASSOCIATED);
    // nb->subscribe(this,NF_LINK_FULL_PROMISCUOUS); // if not global ARP
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
        if (inMacLayer)
            pkt->setOrigin(ManetAddress(myMacAddress));
        else
            pkt->setOrigin(ManetAddress(myIpAddress));
        pkt->setSequence(mySequence);
        mySequence++;

        socket->sendTo(pkt,IPv4Address::ALLONES_ADDRESS,port, interfaceId);
    }
    else
    {
        Ieee802Ctrl *ctrl = new Ieee802Ctrl();
        ctrl->setDest(MACAddress::BROADCAST_ADDRESS);
        pkt->setControlInfo(ctrl);
        send(pkt,"outGate");
    }
}

void LocatorModule::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
    if(category == NF_L2_AP_DISASSOCIATED || category == NF_L2_AP_ASSOCIATED)
    {
        Ieee80211MgmtAP::NotificationInfoSta * infoSta = dynamic_cast<Ieee80211MgmtAP::NotificationInfoSta *>(const_cast<cObject*> (details));
        if (infoSta)
        {
            IPv4Address staIpAdd;
            if (arp)
            {
                const IPv4Address add = arp->getInverseAddressResolution(infoSta->getStaAddress());
                staIpAdd = add;
                if (add.isUnspecified())
                    sendRequest(infoSta->getStaAddress());
            }
            InterfaceEntry * ie = NULL;
            for (int i =0 ; i < itable->getNumInterfaces();i++)
            {
                if (itable->getInterface(i)->getMacAddress() == infoSta->getApAddress())
                {
                    ie = itable->getInterface(i);
                    break;
                }
            }

            if (!myIpAddress.isUnspecified())
            {
                globalApIpSet.insert(myIpAddress);
                apIpSet.insert(myIpAddress);
            }

            if (!myMacAddress.isUnspecified())
            {
                globalApSet.insert(myMacAddress);
                apSet.insert(myMacAddress);
            }


            if (category == NF_L2_AP_ASSOCIATED)
            {
                setTables(myMacAddress,infoSta->getStaAddress(),myIpAddress,staIpAdd,ASSOCIATION,ie);
                sendMessage(myMacAddress,infoSta->getStaAddress(),myIpAddress,staIpAdd,ASSOCIATION);
            }
            else if (category == NF_L2_AP_DISASSOCIATED)
            {
                setTables(myMacAddress,infoSta->getStaAddress(),myIpAddress,staIpAdd,DISASSOCIATION,ie);
                sendMessage(myMacAddress,infoSta->getStaAddress(),myIpAddress,staIpAdd,DISASSOCIATION);
            }
        }
    }
    else if (category == NF_LINK_FULL_PROMISCUOUS)
    {
        Ieee80211DataOrMgmtFrame * frame = dynamic_cast<Ieee80211DataOrMgmtFrame*> (const_cast<cObject*>(details));
        if (!frame)
            return;
        if (dynamic_cast<ARPPacket *>(frame->getEncapsulatedPacket()))
        {
            // dispatch ARP packets to ARP
            processARPPacket(frame->getEncapsulatedPacket());
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

void LocatorModule::setTables(const MACAddress & APaddr, const MACAddress &staAddr, const IPv4Address & apIpAddr, const IPv4Address & staIpAddr, const Action &action, InterfaceEntry *ie)
{
    if(action == ASSOCIATION)
    {
        LocEntry locEntry;
        locEntry.macAddr = staAddr;
        locEntry.apMacAddr = APaddr;
        locEntry.ipAddr = staIpAddr;
        locEntry.apIpAddr = apIpAddr;
        if (!staAddr.isUnspecified())
        {
            locatorMapMac[staAddr] = locEntry;
            globalLocatorMapMac[staAddr] = locEntry;
        }
        if (!staIpAddr.isUnspecified())
        {
            locatorMapIp[staIpAddr] = locEntry;
            globalLocatorMapIp[staIpAddr] = locEntry;
        }
        GlobalWirelessLinkInspector::setLocatorInfo(ManetAddress(staAddr), ManetAddress(APaddr));
        if (!staIpAddr.isUnspecified())
        {
            if (rt)
            {
                if (ie)
                {
                    IPv4Route *entry = new IPv4Route();
                    entry->setDestination(staIpAddr);
                    entry->setGateway(staIpAddr);
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
        LocatorNotificationInfo infoLoc;
        infoLoc.setMacAddr(staAddr);
        infoLoc.setIpAddr(staIpAddr);
        nb->fireChangeNotification(NF_LOCATOR_ASSOC,&infoLoc);
    }
    else if (action == DISASSOCIATION)
    {
        // first check validity exist the possibility that the assoc message can have arrived bebore

        MapIpIteartor itIp;
        MapMacIterator itMac;
        if (!staAddr.isUnspecified())
        {
            itMac = globalLocatorMapMac.find(staAddr);
            if (itMac != globalLocatorMapMac.end())
            {
                if (!APaddr.isUnspecified() && !itMac->second.apMacAddr.isUnspecified() && itMac->second.apMacAddr != APaddr)
                    return;
                globalLocatorMapMac.erase(itMac);
            }

            itMac = locatorMapMac.find(staAddr);
            if (itMac != locatorMapMac.end())
            {
                if (!APaddr.isUnspecified() && !itMac->second.apMacAddr.isUnspecified() && itMac->second.apMacAddr != APaddr)
                    return;
                locatorMapMac.erase(itMac);
            }

        }
        GlobalWirelessLinkInspector::setLocatorInfo(ManetAddress(staAddr), ManetAddress::ZERO);
        if (!staIpAddr.isUnspecified())
        {
            itIp = globalLocatorMapIp.find(staIpAddr);
            if (itIp != globalLocatorMapIp.end())
            {
                if (!apIpAddr.isUnspecified() && !itIp->second.apIpAddr.isUnspecified() && itIp->second.apIpAddr != apIpAddr)
                     return;
                globalLocatorMapIp.erase(itIp);
            }

            itIp = locatorMapIp.find(staIpAddr);
            if (itIp != locatorMapIp.end())
            {
                if (!apIpAddr.isUnspecified() && !itIp->second.apIpAddr.isUnspecified() && itIp->second.apIpAddr != apIpAddr)
                     return;
                locatorMapIp.erase(itIp);
            }
        }

        if (!staIpAddr.isUnspecified() && rt)
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

        LocatorNotificationInfo infoLoc;
        infoLoc.setMacAddr(staAddr);
        infoLoc.setIpAddr(staIpAddr);
        nb->fireChangeNotification(NF_LOCATOR_DISASSOC,&infoLoc);
    }
}

void LocatorModule::getApList(const MACAddress &add,std::vector<MACAddress>& list)
{
    list.clear();
    if (useGlobal)
    {
        for (MapMacIterator itMac = globalLocatorMapMac.begin(); itMac != globalLocatorMapMac.end(); itMac++)
            if (itMac->second.apMacAddr == add)
                list.push_back(itMac->first);
    }
    else
    {
        for (MapMacIterator itMac = locatorMapMac.begin(); itMac != locatorMapMac.end(); itMac++)
            if (itMac->second.apMacAddr == add)
                list.push_back(itMac->first);
    }
}

void LocatorModule::getApListIp(const IPv4Address & add,std::vector<IPv4Address>& list)
{
    list.clear();
    if (useGlobal)
    {
        for (MapIpIteartor itIp = globalLocatorMapIp.begin(); itIp != globalLocatorMapIp.end(); itIp++)
            if (itIp->second.apIpAddr == add)
                list.push_back(itIp->first);
    }
    else
    {
        for (MapIpIteartor itIp = locatorMapIp.begin(); itIp != locatorMapIp.end(); itIp++)
            if (itIp->second.apIpAddr == add)
                list.push_back(itIp->first);
    }
}


bool LocatorModule::isAp(const MACAddress & add)
{
    ApSetIterator it;
    if (useGlobal)
    {
        it = globalApSet.find(add);
        if (it != globalApSet.end())
            return true;
    }
    else
    {
        it = apSet.find(add);
        if (it != apSet.end())
            return true;
    }
    return false;
}

bool LocatorModule::isApIp(const IPv4Address &add)
{
    ApIpSetIterator it;
    if (useGlobal)
    {
        it = globalApIpSet.find(add);
        if (it != globalApIpSet.end())
            return true;
    }
    else
    {
        it = apIpSet.find(add);
        if (it != apIpSet.end())
            return true;
    }
    return false;
}

void LocatorModule::sendRequest(const MACAddress &destination)
{
    if (isInMacLayer)
        return;
    LocatorPkt *pkt = new LocatorPkt();
    pkt->setOpcode(RequestAddress);
    if (inMacLayer)
        pkt->setOrigin(ManetAddress(myMacAddress));
    else
        pkt->setOrigin(ManetAddress(myIpAddress));
    pkt->setStaMACAddress(destination);
    socket->sendTo(pkt,IPv4Address::ALLONES_ADDRESS,port, interfaceId);
}


void LocatorModule::processARPPacket(cPacket *pkt)
{
    ARPPacket *arp = dynamic_cast<ARPPacket *>(pkt);
    // extract input port
    MACAddress srcMACAddress = arp->getSrcMACAddress();
    IPv4Address srcIPAddress = arp->getSrcIPAddress();

    if (srcMACAddress.isUnspecified())
        return;
    if (srcIPAddress.isUnspecified())
        return;
    MapIpIteartor itIp;
    MapMacIterator itMac;
    LocEntry locEntry;
    if (useGlobal)
    {
        itMac = globalLocatorMapMac.find(srcMACAddress);
        itIp = globalLocatorMapIp.find(srcIPAddress);
        if ((itMac != globalLocatorMapMac.end() && itIp != globalLocatorMapIp.end())
                || (itMac == globalLocatorMapMac.end() && itIp == globalLocatorMapIp.end()))
            return;
        if (itMac != globalLocatorMapMac.end())
        {
            if (itMac->second.ipAddr.isUnspecified())
            {
                itMac->second.ipAddr = srcIPAddress;
                globalLocatorMapIp[srcIPAddress] = itMac->second;
                locEntry = itMac->second;
            }
        }
        else if (itIp != globalLocatorMapIp.end())
        {
            if (itIp->second.macAddr.isUnspecified())
            {
                itIp->second.macAddr = srcMACAddress;
                globalLocatorMapMac[srcMACAddress] = itIp->second;
                locEntry = itIp->second;
            }
        }
        else
            return;
    }
    else
    {
        itMac = locatorMapMac.find(srcMACAddress);
        itIp = locatorMapIp.find(srcIPAddress);
        if ((itMac != locatorMapMac.end() && itIp != locatorMapIp.end())
                || (itMac == locatorMapMac.end() && itIp == locatorMapIp.end()))
            return;
        if (itMac != locatorMapMac.end())
        {
            if (itMac->second.ipAddr.isUnspecified())
            {
                itMac->second.ipAddr = srcIPAddress;
                locatorMapIp[srcIPAddress] = itMac->second;
                locEntry = itMac->second;
            }
        }
        else if (itIp != locatorMapIp.end())
        {
            if (itIp->second.macAddr.isUnspecified())
            {
                itIp->second.macAddr = srcMACAddress;
                locatorMapMac[srcMACAddress] = itIp->second;
                locEntry = itIp->second;
            }
        }
        else
            return;
    }
    if (rt)
    {
        for (int i = 0; i < rt->getNumRoutes(); i++)
        {
            IPv4Route *entry = rt->getRoute(i);
            if (entry->getDestination() == srcIPAddress)
                return;
        }
        InterfaceEntry * ie = NULL;
        for (int i = 0; i < itable->getNumInterfaces(); i++)
        {
            if (itable->getInterface(i)->getMacAddress() == locEntry.apMacAddr)
            {
                ie = itable->getInterface(i);
                break;
            }
        }
        if (ie)
        {
            IPv4Route *entry = new IPv4Route();
            entry->setDestination(locEntry.ipAddr);
            entry->setGateway(locEntry.ipAddr);
            entry->setNetmask(IPv4Address::ALLONES_ADDRESS);
            entry->setInterface(ie);
            rt->addRoute(entry);
        }
    }
}

bool LocatorModule::isThisAp()
{
    return isAp(myMacAddress);
}

bool LocatorModule::isThisApIp()
{
    return isApIp(myIpAddress);
}

