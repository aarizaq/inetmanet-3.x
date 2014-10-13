//
// Copyright (C) 2012 Alfonso Ariza Universidad de Malaga
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

#ifndef __GlobalWirelessWirelessLinkInspector_H_
#define __GlobalWirelessWirelessLinkInspector_H_
#include <vector>
#include <map>
#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet{

class GlobalWirelessLinkInspector : public cSimpleModule
{
    public:
        class Link
        {
            public:
               double costEtt;
               double costEtx;
               double snr;
        };
     protected:
        typedef std::map<L3Address,L3Address> RouteMap;
        class ProtocolRoutingData
        {
            public:
                cModule * mod;
                RouteMap* routesVector;
                bool isProactive;
        };
        typedef std::vector<ProtocolRoutingData> ProtocolsRoutes;
        typedef std::map<L3Address, ProtocolsRoutes>GlobalRouteMap;
        static GlobalRouteMap *globalRouteMap;

        typedef std::map<L3Address,Link> NodeLinkCost;
        typedef std::map<L3Address,NodeLinkCost*> CostMap;
        static CostMap * costMap;

        typedef std::map<L3Address,L3Address> LocatorMap;
        typedef LocatorMap::iterator  LocatorIteartor;


        static LocatorMap *globalLocatorMap;

        typedef std::map<L3Address,uint64_t> QueueSize;
        static QueueSize *queueSize;

     public:
        GlobalWirelessLinkInspector();
        virtual ~GlobalWirelessLinkInspector();

        static bool isActive() {return (costMap!=NULL);}
        static bool isActiveLocator() {return (globalLocatorMap!=NULL && !globalLocatorMap->empty());}
        static void setLinkCost(const L3Address& org,const L3Address& dest,const Link &);
        static bool getLinkCost(const L3Address& org,const L3Address& dest,Link &);
        static bool getCostPath(const std::vector<L3Address>&, Link &);
        static bool getWorst(const std::vector<L3Address>&, Link &);
        static bool initRoutingProtocol (cModule *,bool);
        static bool getRoute(const L3Address &src, const L3Address &dest, std::vector<L3Address> &route);
        static bool getRouteWithLocator(const L3Address &src, const L3Address &dest, std::vector<L3Address> &route);
        static bool setRoute(const cModule *,const L3Address &, const L3Address &dest, const L3Address &gw, const bool &erase);
        static void initRoutingTables (const cModule *,const L3Address &,bool);
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

        static void setLocatorInfo(L3Address , L3Address );
        static bool getLocatorInfo(L3Address , L3Address &);
        static bool getNumNodes(L3Address,int &);
        static bool areNeighbour(const L3Address &node1, const L3Address &node2,bool &areN);
        static bool setQueueSize(const L3Address &node, const uint64_t &);
        static bool getQueueSize(const L3Address &node, uint64_t &);

};

}

#endif
