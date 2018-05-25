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

#include <vector>
#include <map>
#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/geometry/common/EulerAngles.h"

namespace inet{

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
        typedef std::map<MACAddress,nodeInfo> ListNodesMac;

        ListNodes listNodes;
        ListNodesMac listNodesMac;
    public:
        WirelessGetNeig();
        virtual ~WirelessGetNeig();
        virtual void getNeighbours(const IPv4Address &node, std::vector<IPv4Address>&, const double &distance);
        virtual void getNeighbours(const MACAddress &node, std::vector<MACAddress>&, const double &distance);
        virtual void getNeighbours(const IPv4Address &node, std::vector<IPv4Address>&, const double &distance, std::vector<Coord> &);
        virtual void getNeighbours(const MACAddress &node, std::vector<MACAddress>&, const double &distance, std::vector<Coord> &);
        virtual EulerAngles getDirection(const MACAddress &, const MACAddress&, double &distance);
};


}

#endif /* WIRELESSNUMHOPS_H_ */
