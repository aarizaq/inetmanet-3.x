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



class GlobalWirelessWirelessLinkInspector : public cSimpleModule
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
        typedef std::map<Uint128,Link> NodeLinkCost;
        typedef std::map<Uint128,NodeLinkCost*> CostMap;
        static CostMap * costMap;
        GlobalWirelessWirelessLinkInspector();
        virtual ~GlobalWirelessWirelessLinkInspector();
     public:
        static bool isActive() {return (costMap!=NULL);}
        static void setLinkCost(const Uint128& org,const Uint128& dest,const Link &);
        static bool getLinkCost(const Uint128& org,const Uint128& dest,Link &);
        static bool getLinkCost(const std::vector<Uint128>&, Link &);
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
};


#endif
