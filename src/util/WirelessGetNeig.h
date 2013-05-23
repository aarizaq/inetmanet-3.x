//
// Copyright (C) 2012 Univerdidad de Malaga.
// Author: Alfonso Ariza
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

#ifndef WIRELESSGETNEIG_H_
#define WIRELESSGETNEIG_H_

#include <cownedobject.h>
#include <vector>
#include <map>
#include "Coord.h"
#include "IPv4Address.h"

class IInterfaceTable;
class IMobility;

class WirelessGetNeig : public cOwnedObject
{
        struct nodeInfo
        {
            IMobility* mob;
            IInterfaceTable* itable;
        };
        typedef std::map<uint32_t,nodeInfo> ListNodes;
        ListNodes listNodes;
    public:
        WirelessGetNeig();
        virtual ~WirelessGetNeig();
        virtual void getNeighbours(const IPv4Address &node, std::vector<IPv4Address>&, const double &distance);
};



#endif /* WIRELESSNUMHOPS_H_ */
