//
// Copyright (C) 2012 Alfonso Ariza Universidad de Málaga
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
#include "INETDefs.h"
#include "uint128.h"



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
        typedef std::map<Uint128,Uint128> RouteMap;
        class ProtocolRoutingData
        {
            public:
                cModule * mod;
                RouteMap* routesVector;
                bool isProactive;
        };
        typedef std::vector<ProtocolRoutingData> ProtocolsRoutes;
        typedef std::map<Uint128, ProtocolsRoutes>GlobalRouteMap;
        static GlobalRouteMap *globalRouteMap;

        typedef std::map<Uint128,Link> NodeLinkCost;
        typedef std::map<Uint128,NodeLinkCost*> CostMap;
        static CostMap * costMap;

        typedef std::map<Uint128,Uint128> LocatorMap;
        typedef LocatorMap::iterator  LocatorIteartor;


        static LocatorMap *globalLocatorMap;

        typedef std::map<Uint128,uint64_t> QueueSize;
        static QueueSize *queueSize;

     public:
        GlobalWirelessLinkInspector();
        virtual ~GlobalWirelessLinkInspector();

        static bool isActive() {return (costMap!=NULL);}
        static bool isActiveLocator() {return (globalLocatorMap!=NULL && !globalLocatorMap->empty());}
        static void setLinkCost(const Uint128& org,const Uint128& dest,const Link &);
        static bool getLinkCost(const Uint128& org,const Uint128& dest,Link &);
        static bool getLinkCost(const std::vector<Uint128>&, Link &);
        static bool initRoutingProtocol (cModule *,bool);
        static bool getRoute(const Uint128 &src, const Uint128 &dest, std::vector<Uint128> &route);
        static bool getRouteWithLocator(const Uint128 &src, const Uint128 &dest, std::vector<Uint128> &route);
        static bool setRoute(const cModule *,const Uint128 &, const Uint128 &dest, const Uint128 &gw, const bool &erase);
        static void initRoutingTables (const cModule *,const Uint128 &,bool);
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

        static void setLocatorInfo(Uint128 , Uint128 );
        static bool getLocatorInfo(Uint128 , Uint128 &);
        static bool getNumNodes(Uint128,int &);
        static bool areNeighbour(const Uint128 &node1, const Uint128 &node2,bool &areN);
        static bool setQueueSize(const Uint128 &node, const uint64_t &);
        static bool getQueueSize(const Uint128 &node, uint64_t &);

};


#endif
