//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

/**
 * @file TransportAddress.cc
 * @author Markus Mauch
 * @author Sebastian Mies
 */

#include <omnetpp.h>

#include "TransportAddress.h"

// predefined transport address
const TransportAddress TransportAddress::UNSPECIFIED_NODE;
const std::vector<TransportAddress> TransportAddress::UNSPECIFIED_NODES;

std::ostream& operator<<(std::ostream& os, const TransportAddress& n)
{
    if (n.isUnspecified()) {
        os << "<addr unspec>";
    } else {
        os << n.ip << ":" << n.port;
    }

    if (n.getSourceRouteSize() > 0) {
        os << "(SR:";
        for (size_t i = 0; i < n.getSourceRouteSize(); i++) {
            os << " " << n.getSourceRoute()[i].ip << ":"
               << n.getSourceRoute()[i].port;
        }
        os << ")";
    }

    return os;
};


//default-constructor
TransportAddress::TransportAddress()
{
    port = -1;
    natType = UNKNOWN_NAT;
}

//copy constructor
TransportAddress::TransportAddress( const TransportAddress& handle )
{
    port = handle.port;
    ip = handle.ip;
    natType = handle.natType;
    sourceRoute = handle.sourceRoute;
}


//complete constructor
TransportAddress::TransportAddress( const IPvXAddress& ip, int port,
                                    NatType natType)
{
    this->ip = ip;
    this->port = port;
    this->natType = natType;
}

//public
bool TransportAddress::isUnspecified() const
{
    return (ip.isUnspecified() || (port == -1));
}

//public
TransportAddress& TransportAddress::operator=(const TransportAddress& rhs)
{
    this->ip = rhs.ip;
    this->port = rhs.port;

    return *this;
}

//public
bool TransportAddress::operator==(const TransportAddress& rhs) const
{
    assertUnspecified(rhs);
    return (this->ip == rhs.ip && this->port == rhs.port);
}

//public
bool TransportAddress::operator!=(const TransportAddress& rhs) const
{
    assertUnspecified(rhs);
    return !operator==(rhs);
}

//public
bool TransportAddress::operator<(const TransportAddress &rhs) const
{
    assertUnspecified(rhs);
    if (ip < rhs.ip) {
        return true;
    } else if (rhs.ip < ip) {
        return false;
    } else if (port < rhs.port) {
        return true;
    }
    return false;
}

//public
bool TransportAddress::operator>(const TransportAddress &rhs) const
{
    assertUnspecified(rhs);
    if (rhs.ip < ip) {
        return true;
    } else if (ip < rhs.ip) {
        return false;
    } else if (port > rhs.port) {
        return true;
    }
    return false;
}

//public
bool TransportAddress::operator<=(const TransportAddress &rhs) const
{
    return !operator>(rhs);
}

//public
bool TransportAddress::operator>=(const TransportAddress &rhs) const
{
    return !operator<(rhs);
}

//public
void TransportAddress::setIp(const IPvXAddress& ip, int port,
                             NatType natType)
{
    this->ip = ip;
    if (port!=-1)
        this->port = port;
    if (natType != UNKNOWN_NAT)
        this->natType = natType;
}

//public
void TransportAddress::setPort( int port )
{
    this->port = port;
}

//public
const IPvXAddress& TransportAddress::getIp() const
{
    return ip;
}

//public
int TransportAddress::getPort() const
{
    return port;
}

//public
TransportAddress::NatType TransportAddress::getNatType() const
{
    return natType;
}

//public
size_t TransportAddress::getSourceRouteSize() const
{
    return sourceRoute.size();
}

//public
const TransportAddressVector& TransportAddress::getSourceRoute() const
{
    return sourceRoute;
}

//public
void TransportAddress::appendSourceRoute(const TransportAddress& add)
{
    sourceRoute.push_back(TransportAddress(add.ip, add.port, add.natType));
    const TransportAddressVector& sr = add.getSourceRoute();
    for (size_t i = 0; i < sr.size(); i++) {
        if (sr[i].getSourceRouteSize() > 0) {
            throw cRuntimeError("TransportAddress::appendSourceRoute(): "
                                "Trying to add source route to source route!");
        }
        sourceRoute.push_back(TransportAddress(sr[i].ip, sr[i].port,
                                               sr[i].natType));
    }
}


//public
size_t TransportAddress::hash() const
{
    size_t iphash;
    if (ip.isIPv6()) {
        uint32_t* addr = (uint32_t*) ip.get6().words();
        iphash = (size_t)(addr[0]^addr[1]^addr[2]^addr[3]);
    } else {
        iphash = (size_t)ip.get4().getInt();
    }

    return (size_t)(iphash^port);
}

TransportAddress* TransportAddress::dup() const
{
    return new TransportAddress(*this);
}

//private
inline void TransportAddress::assertUnspecified( const TransportAddress& handle ) const
{
    if ( this->isUnspecified() || handle.isUnspecified() )
        opp_error("TransportAddress: Trying to compare unspecified TransportAddress!");
}
