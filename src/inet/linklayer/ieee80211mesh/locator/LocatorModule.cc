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

#include "inet/linklayer/ieee80211mesh/locator/LocatorModule.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAP.h"
#include "inet/linklayer/ieee80211mesh/locator/locatorPkt_m.h"
#include "inet/transportlayer/udp/UDP.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/routing/extras/base/LocatorNotificationInfo_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/common/GlobalWirelessLinkInspector.h"
#include "inet/networklayer/ipv4/IPv4Route.h"

namespace inet {

namespace ieee80211 {


simsignal_t LocatorModule::locatorChangeSignal = SIMSIGNAL_NULL;
LocatorModule::LocatorMapIp LocatorModule::globalLocatorMapIp;
LocatorModule::LocatorMapMac LocatorModule::globalLocatorMapMac;
LocatorModule::ApIpSet LocatorModule::globalApIpSet;
LocatorModule::ApSet LocatorModule::globalApSet;

LocatorModule::ReverseList LocatorModule::reverseList;
LocatorModule::DirectList LocatorModule::directList;


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
    arp = nullptr;
    rt = nullptr;
    itable = nullptr;
    isInMacLayer = true;
    socket = nullptr;
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
   reverseList.clear();
   directList.clear();
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
        if ((pkt->getOrigin().getType() == L3Address::IPv4 && pkt->getOrigin().toIPv4() == myIpAddress)
                || (pkt->getOrigin().getType() == L3Address::MAC && pkt->getOrigin().toMAC() == myMacAddress))
        {
            delete pkt;
            return;
        }
        auto it = sequenceMap.find(pkt->getOrigin());
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


        if (staIpaddr.isUnspecified())
            staIpaddr = getReverseAddress(staAddr);
        if (staAddr.isUnspecified())
            staAddr = geDirectAddress(staIpaddr);
        if (apIpaddr.isUnspecified())
            apIpaddr = getReverseAddress(apAddr);
        if (apAddr.isUnspecified())
            apAddr = geDirectAddress(apIpaddr);

        if (staIpaddr.isUnspecified())
            sendRequest(staAddr);

        if (apIpaddr.isUnspecified())
            sendRequest(apAddr);

        if ( pkt->getOpcode() == LocatorAssoc)
            setTables(apAddr,staAddr,apIpaddr,staIpaddr,ASSOCIATION,nullptr);
        else if (pkt->getOpcode() == LocatorDisAssoc)
            setTables(apAddr,staAddr,apIpaddr,staIpaddr,DISASSOCIATION,nullptr);
    }
    if (socket)
    {
        if (pkt && pkt->getControlInfo())
            delete pkt->removeControlInfo();
        UDPSocket::SendOptions options;
        options.outInterfaceId = interfaceId;
        socket->sendTo(pkt,IPv4Address::ALLONES_ADDRESS,port, &options);
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

    if ((pkt->getOrigin().getType() == L3Address::IPv4 && pkt->getOrigin().toIPv4() == myIpAddress)
                  || (pkt->getOrigin().getType() == L3Address::MAC && pkt->getOrigin().toMAC() == myMacAddress))
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
     UDPSocket::SendOptions options;
     options.outInterfaceId = interfaceId;
     socket->sendTo(pkt, udpCtrl->getSrcAddr(), port, &options);
}


void LocatorModule::initialize(int stage)
{
    if (stage!=INITSTAGE_NETWORK_LAYER_3)
        return;

    arp =  getModuleFromPar<IARP>(par("arpModule"), this);

    rt = findModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
    itable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);




    InterfaceEntry *ie = nullptr;
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

    for (int i = 0; i < itable->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = itable->getInterface(i);
        IPv4InterfaceData * ipData = ie->ipv4Data();
        reverseList.insert(std::make_pair(ie->getMacAddress(),L3Address(ipData->getIPAddress())));
        directList.insert(std::make_pair(L3Address(ipData->getIPAddress()),ie->getMacAddress()));
    }

    WATCH_MAP(globalLocatorMapIp);
    WATCH_MAP(globalLocatorMapMac);
    WATCH_MAP(locatorMapIp);
    WATCH_MAP(locatorMapMac);
    cModule *host = getContainingNode(this);
    host->subscribe(NF_L2_AP_DISASSOCIATED,this);
    host->subscribe(NF_L2_AP_ASSOCIATED,this);
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
            pkt->setOrigin(L3Address(myMacAddress));
        else
            pkt->setOrigin(L3Address(myIpAddress));
        pkt->setSequence(mySequence);
        mySequence++;
        UDPSocket::SendOptions options;
        options.outInterfaceId = interfaceId;
        socket->sendTo(pkt,IPv4Address::ALLONES_ADDRESS,port, &options);
    }
    else
    {
        Ieee802Ctrl *ctrl = new Ieee802Ctrl();
        ctrl->setDest(MACAddress::BROADCAST_ADDRESS);
        pkt->setControlInfo(ctrl);
        send(pkt,"outGate");
    }
}

void LocatorModule::receiveSignal(cComponent *source, simsignal_t category, cObject *details)
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
                const IPv4Address add = getReverseAddress(infoSta->getStaAddress());
                staIpAdd = add;
                if (add.isUnspecified())
                    sendRequest(infoSta->getStaAddress());
            }
            InterfaceEntry * ie = nullptr;
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
    const IPv4Address staIpadd = getReverseAddress(STAaddr);
    const IPv4Address apIpAddr = getReverseAddress(APaddr);
    InterfaceEntry * ie = nullptr;
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
    const MACAddress STAaddr = geDirectAddress(staIpAddr);
    const MACAddress APaddr = geDirectAddress(apIpAddr);
    InterfaceEntry * ie = nullptr;
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
        GlobalWirelessLinkInspector::setLocatorInfo(L3Address(staAddr), L3Address(APaddr));
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
        inetmanet::LocatorNotificationInfo infoLoc;
        infoLoc.setMacAddr(staAddr);
        infoLoc.setIpAddr(staIpAddr);
        emit(NF_LOCATOR_ASSOC,&infoLoc);
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
        GlobalWirelessLinkInspector::setLocatorInfo(L3Address(staAddr), L3Address());
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

        inetmanet::LocatorNotificationInfo infoLoc;
        infoLoc.setMacAddr(staAddr);
        infoLoc.setIpAddr(staIpAddr);
        emit(NF_LOCATOR_DISASSOC,&infoLoc);
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
        pkt->setOrigin(L3Address(myMacAddress));
    else
        pkt->setOrigin(L3Address(myIpAddress));
    pkt->setStaMACAddress(destination);
    UDPSocket::SendOptions options;
    options.outInterfaceId = interfaceId;
    socket->sendTo(pkt,IPv4Address::ALLONES_ADDRESS,port, &options);
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
        InterfaceEntry * ie = nullptr;
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

}

}
