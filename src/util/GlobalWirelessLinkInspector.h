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
#include "INETDefs.h"
#include "ManetAddress.h"



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
        typedef std::map<ManetAddress,ManetAddress> RouteMap;
        class ProtocolRoutingData
        {
            public:
                cModule * mod;
                RouteMap* routesVector;
                bool isProactive;
        };
        typedef std::vector<ProtocolRoutingData> ProtocolsRoutes;
        typedef std::map<ManetAddress, ProtocolsRoutes>GlobalRouteMap;
        static GlobalRouteMap *globalRouteMap;

        typedef std::map<ManetAddress,Link> NodeLinkCost;
        typedef std::map<ManetAddress,NodeLinkCost*> CostMap;
        static CostMap * costMap;

        typedef std::map<ManetAddress,ManetAddress> LocatorMap;
        typedef LocatorMap::iterator  LocatorIteartor;


        static LocatorMap *globalLocatorMap;

        typedef std::map<ManetAddress,uint64_t> QueueSize;
        static QueueSize *queueSize;

     public:
        GlobalWirelessLinkInspector();
        virtual ~GlobalWirelessLinkInspector();

        static bool isActive() {return (costMap!=NULL);}
        static bool isActiveLocator() {return (globalLocatorMap!=NULL && !globalLocatorMap->empty());}
        static void setLinkCost(const ManetAddress& org,const ManetAddress& dest,const Link &);
        static bool getLinkCost(const ManetAddress& org,const ManetAddress& dest,Link &);
        static bool getCostPath(const std::vector<ManetAddress>&, Link &);
        static bool getWorst(const std::vector<ManetAddress>&, Link &);
        static bool initRoutingProtocol (cModule *,bool);
        static bool getRoute(const ManetAddress &src, const ManetAddress &dest, std::vector<ManetAddress> &route);
        static bool getRouteWithLocator(const ManetAddress &src, const ManetAddress &dest, std::vector<ManetAddress> &route);
        static bool setRoute(const cModule *,const ManetAddress &, const ManetAddress &dest, const ManetAddress &gw, const bool &erase);
        static void initRoutingTables (const cModule *,const ManetAddress &,bool);
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

        static void setLocatorInfo(ManetAddress , ManetAddress );
        static bool getLocatorInfo(ManetAddress , ManetAddress &);
        static bool getNumNodes(ManetAddress,int &);
        static bool areNeighbour(const ManetAddress &node1, const ManetAddress &node2,bool &areN);
        static bool setQueueSize(const ManetAddress &node, const uint64_t &);
        static bool getQueueSize(const ManetAddress &node, uint64_t &);

};


#endif
