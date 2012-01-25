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

/** @author Antonio Zea */

/**
 * @file I3SubIdentifier.cc
 * @author Antonio Zea
 */

#include "I3SubIdentifier.h"


I3SubIdentifier::I3SubIdentifier() :
        type(Invalid)
{
}

void I3SubIdentifier::setIPAddress(const I3IPAddress &address)
{
    type = IPAddress;
    ipAddress = address;
    identifier.clear();
}

void I3SubIdentifier::setIdentifier(const I3Identifier &id)
{
    type = Identifier;

    I3IPAddress zero; // initialized to zero
    ipAddress = zero; // set to clear

    identifier = id;
}

I3SubIdentifier::IdentifierType I3SubIdentifier::getType() const
{
    return type;
}

I3IPAddress &I3SubIdentifier::getIPAddress()
{
    if (type != IPAddress) {
        // error!
    }
    return ipAddress;
}

I3Identifier &I3SubIdentifier::getIdentifier()
{
    if (type != Identifier) {
        // error!
    };
    return identifier;
}

int I3SubIdentifier::compareTo(const I3SubIdentifier &id) const
{
    if (type != id.type) {
        return type - id.type;
    } else if (type == Identifier) {
        return identifier.compareTo(id.identifier);
    } else if (type == IPAddress) {
        if (ipAddress.getIp() < id.ipAddress.getIp()) return -1;
        else if (ipAddress.getIp() == id.ipAddress.getIp()) return ipAddress.getPort() - id.ipAddress.getPort();
        else return 1;
    }
    return 0;
}

int I3SubIdentifier::length() const {
//	return sizeof(type) + (type == IPAddress ? ipAddress.length() : identifier.length());
    return 1 + (type == IPAddress ? ipAddress.length() : identifier.length());
}

std::ostream& operator<<(std::ostream& os, const I3SubIdentifier &s) {
    if (s.type == I3SubIdentifier::Identifier) {
        os << s.identifier;
    } else {
        os << s.ipAddress;
    }
    return os;
}
