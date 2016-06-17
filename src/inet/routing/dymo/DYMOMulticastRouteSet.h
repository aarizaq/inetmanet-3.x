//
// Copyright (C) 2016
// Author: Alfonso Ariza
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_DYMOMULTICASTROUTESET_H
#define __INET_DYMOMULTICASTROUTESET_H

#include <omnetpp.h>
#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/dymo/DYMOdefs.h"

namespace inet {

namespace dymo {

class INET_API DYMOMulticastRouteSet : public cObject
{
  private:
    struct OriginatorAddressPair {
        L3Address prefixAddr;
        int orPrefixLen;
        bool operator<(const OriginatorAddressPair& other) const {
            if (orPrefixLen != other.orPrefixLen)
                return (orPrefixLen < other.orPrefixLen);
            else
                return prefixAddr < other.prefixAddr;
        };
        bool operator>(const OriginatorAddressPair& other) const { return other < *this; };
        bool operator==(const OriginatorAddressPair& other) const
        {
            return ((orPrefixLen == other.orPrefixLen) && (prefixAddr == other.prefixAddr));
        };
        bool operator!=(const OriginatorAddressPair& other) const {
            return ((orPrefixLen != other.orPrefixLen) || (prefixAddr != other.prefixAddr));
        }
    };

    struct MulticastRouteInfo{
        DYMOSequenceNumber orSequenceNumber;
        L3Address target;
        DYMOSequenceNumber trSequenceNumber;
        bool hasMetricType;
        DYMOMetricType mType;
        double metric;
        simtime_t timeStamp;
        simtime_t removeTime;
    };

    bool active = true;

    simtime_t lifetime = 300;
    // Is it necessary? it shouldn't be more than 1 element in this vector
    typedef std::vector <MulticastRouteInfo> MulticastRouteVect;
    typedef std::map<OriginatorAddressPair,MulticastRouteVect> MulticastRouteSet;

    MulticastRouteSet multicastRouteSet;

    L3Address selfAddress;


  public:
    DYMOMulticastRouteSet();
    virtual ~DYMOMulticastRouteSet() {multicastRouteSet.clear();}

    virtual void setLifeTime(const simtime_t &v) {lifetime = v;}
    virtual simtime_t getLifeTime() {return lifetime;}

    virtual void setSelftAddress(const L3Address & add){selfAddress = add;}
    virtual L3Address getSlftAddress() {return selfAddress;}

    virtual void setActive(const bool &v) {active = v;}
    virtual bool getActive() {return active;}

    virtual void clear() {multicastRouteSet.clear();}
    virtual bool check(RteMsg *rteMsg);
};

} // namespace dymo

} // namespace inet

#endif // ifndef __INET_DYMOROUTEDATA_H

