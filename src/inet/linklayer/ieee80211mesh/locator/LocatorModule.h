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

#ifndef LOCATORMODULE_H_
#define LOCATORMODULE_H_

#include <map>
#include <set>
#include "inet/linklayer/ieee80211mesh/locator/ILocator.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/contract/IARP.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

namespace ieee80211 {

class LocatorModule : public cSimpleModule, public ILocator, protected cListener
{
    protected:
        struct LocEntry
        {
            MACAddress macAddr;
            IPv4Address ipAddr;
            MACAddress apMacAddr;
            IPv4Address apIpAddr;
        };

        typedef std::map<IPv4Address,LocEntry> LocatorMapIp;
        typedef std::map<MACAddress,LocEntry>  LocatorMapMac;
        typedef LocatorMapIp::iterator  MapIpIteartor;
        typedef LocatorMapMac::iterator MapMacIterator;
        typedef std::set<IPv4Address> ApIpSet;
        typedef std::set<MACAddress> ApSet;

        typedef ApIpSet::iterator ApIpSetIterator;
        typedef ApSet::iterator ApSetIterator;

        IPv4Address myIpAddress;
        MACAddress  myMacAddress;
        UDPSocket * socket;
        int port;
        int interfaceId;

        bool inMacLayer;

        LocatorMapIp locatorMapIp;
        static LocatorMapIp globalLocatorMapIp;
        LocatorMapMac locatorMapMac;
        static LocatorMapMac globalLocatorMapMac;

        static ApIpSet globalApIpSet;
        static ApSet globalApSet;
        ApIpSet apIpSet;
        ApSet apSet;

        typedef std::map<MACAddress,L3Address> ReverseList;
        typedef std::map<L3Address,MACAddress> DirectList;

        static ReverseList reverseList;
        static DirectList directList;


        static simsignal_t locatorChangeSignal;
        IARP *arp;
        IInterfaceTable *itable;
        IIPv4RoutingTable *rt;
        bool isInMacLayer;

        enum Action
        {
            ASSOCIATION,
            DISASSOCIATION
        };
        virtual void modifyInformationMac(const MACAddress &,const MACAddress &, const Action &);
        virtual void modifyInformationIp(const IPv4Address &, const IPv4Address &, const Action &);
        virtual void setTables(const MACAddress & APaddr, const MACAddress &STAaddr, const IPv4Address & apIpAddr, const IPv4Address & staIpAddr, const Action &action, InterfaceEntry *ie);
        bool useGlobal;
        unsigned int mySequence;
        std::map<L3Address,unsigned int> sequenceMap;

        virtual void  sendMessage(const MACAddress &,const MACAddress &,const IPv4Address &,const IPv4Address &,const Action &);
        virtual void sendRequest(const MACAddress &);

        virtual void processReply(cPacket* pkt);
        virtual void processRequest(cPacket* pkt);
        virtual void processARPPacket(cPacket *arp);

        IPv4Address getReverseAddress(const MACAddress & addr)
        {
            L3Address aux;
            ReverseList::iterator it = reverseList.find(addr);
            if (it != reverseList.end())
                aux = it->second;
            return aux.toIPv4();
        }

        MACAddress geDirectAddress(const IPv4Address & addr)
        {
            MACAddress aux;
            DirectList::iterator it = directList.find(L3Address(addr));
            if (it != directList.end())
                aux = it->second;
            return aux;
        }

    public:
        friend std::ostream& operator<<(std::ostream& os, const LocatorModule::LocEntry& e);
        LocatorModule();
        virtual ~LocatorModule();
        virtual void initialize(int stage);
        virtual int numInitStages() const {return NUM_INIT_STAGES;}
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
        virtual void handleMessage(cMessage *);
        virtual const MACAddress  getLocatorMacToMac(const MACAddress &);
        virtual const IPv4Address getLocatorMacToIp(const MACAddress &);
        virtual const IPv4Address getLocatorIpToIp(const IPv4Address &);
        virtual const MACAddress  getLocatorIpToMac(const IPv4Address &);
        virtual void getApList(const MACAddress &,std::vector<MACAddress>&);
        virtual void getApListIp(const IPv4Address &,std::vector<IPv4Address>&);
        virtual bool isAp(const MACAddress & add);
        virtual bool isApIp(const IPv4Address &add);
        virtual bool isThisAp();
        virtual bool isThisApIp();




        virtual void setIpAddress(const IPv4Address &add) {myIpAddress = add;}
        virtual void setMacAddress(const MACAddress &add) {myMacAddress = add;}
        virtual IPv4Address getIpAddress() {return myIpAddress;}
        virtual MACAddress getMacAddress() {return myMacAddress;}

};

}

}

#endif /* LOCATORMODULE_H_ */
