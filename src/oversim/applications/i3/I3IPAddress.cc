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
 * @file I3IPAddress.cc
 * @author Antonio Zea
 */


#include "I3IPAddress.h"

I3IPAddress::I3IPAddress()
{
    port = 0;
}

I3IPAddress::I3IPAddress(IPvXAddress add, int p)
{
    ip = add;
    port = p;
}

bool I3IPAddress::operator<(const I3IPAddress &a) const
{
    return ip < a.ip || (ip == a.ip && port < a.port);
}

bool I3IPAddress::operator==(const I3IPAddress &a) const
{
    return ip == a.ip && port == a.port;
}

bool I3IPAddress::operator>(const I3IPAddress &a) const
{
    return a < *this;
}

int I3IPAddress::length() const {
    //return sizeof(address) + sizeof(port);
    return (ip.isIPv6() ? 128 : 32) + 16; // 16 = port length
}

std::ostream& operator<<(std::ostream& os, const I3IPAddress& ip) {
    os << ip.ip << ':' << ip.port;
    return os;
}
