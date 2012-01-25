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
 * @file BrooseHandle.cc
 * @author Jochen Schenk
 */

#include <omnetpp.h>


#include "BrooseHandle.h"

std::ostream& operator<<(std::ostream& os, const BrooseHandle& n)
{
    if (n.isUnspecified()) {
        os << "<unspec>";
    } else {
        os << n.getIp() << ":" << n.getPort() << " " << n.getKey() << " last-seen: " << n.lastSeen
           << " failedResponses: " << n.failedResponses << " rtt: " << n.rtt;
    }

    return os;
};

BrooseHandle::BrooseHandle()
{
    //
    // Default-constructor.
    //
    port = -1;
    key = OverlayKey::UNSPECIFIED_KEY;
    failedResponses = 0;
    rtt = -1;
    lastSeen = -1;
}

BrooseHandle::BrooseHandle(OverlayKey initKey, IPvXAddress initIP, int initPort)
{
    //
    // Constructor. Initializes the node handle with the passed arguments.
    //
    ip = initIP;
    port = initPort;
    key = initKey;
    failedResponses = 0;
    rtt = -1;
    lastSeen = -1;
}

BrooseHandle::BrooseHandle(const NodeHandle& node)
{
    //
    // Constructor. Initializes the node handle with the passed arguments.
    //
    ip = node.getIp();
    port = node.getPort();
    key = node.getKey();
    failedResponses = 0;
    rtt = -1;
    lastSeen = -1;
}


BrooseHandle::BrooseHandle(const TransportAddress& node, const OverlayKey& destKey)
{
    //
    // Constructor. Initializes the node handle with the passed arguments.
    //
    ip = node.getIp();
    port = node.getPort();
    key = destKey;
    rtt = -1;
    lastSeen = -1;
}

BrooseHandle& BrooseHandle::operator=(const BrooseHandle& rhs)
{

    this->key = rhs.getKey();
    this->ip = rhs.getIp();
    this->port = rhs.getPort();
    this->failedResponses = rhs.failedResponses;
    this->rtt = rhs.rtt;
    this->lastSeen = rhs.lastSeen;
    return *this;
}

bool BrooseHandle::operator==(const BrooseHandle& rhs) const
{
    if (this->isUnspecified() || rhs.isUnspecified())
        opp_error("BrooseHandle: Trying to compare unspecified nodeHandle!");

    if (this->key != rhs.getKey() )
        return false;
    if (this->ip != rhs.getIp() )
        return false;
    if (this->port != rhs.getPort() )
        return false;
    return true;
}

bool BrooseHandle::operator!=(const BrooseHandle& rhs) const
{
    if (this->isUnspecified() || rhs.isUnspecified())
        opp_error("BrooseHandle: Trying to compare unspecified nodeHandle!");

    if (this->key == rhs.getKey() &&
            this->ip == rhs.getIp() && this->port == rhs.getPort())
        return false;
    return true;
}

