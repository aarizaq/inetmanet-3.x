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
 * @file NodeHandle.cc
 * @author Markus Mauch
 * @author Sebastian Mies
 */

#include "NodeHandle.h"

// predefined node handle
const NodeHandle NodeHandle::UNSPECIFIED_NODE;

std::ostream& operator<<(std::ostream& os, const NodeHandle& n)
{
    if (n.ip.isUnspecified()) {
        os << "<addr unspec> ";
    } else {
        os << n.ip << ":" << n.port << " ";
    }

    if (n.key.isUnspecified()) {
        os << "<key unspec>";
    } else {
        os << n.key;
    }

    return os;
};


//default-constructor
NodeHandle::NodeHandle()
{
    port = -1;
    key = OverlayKey(); // unspecified key, note: OverlayKey::UNSPECIFIED_KEY might not be initialized here
}

//copy constructor
NodeHandle::NodeHandle( const NodeHandle& handle )
{
    key = handle.key;
    port = handle.port;
    ip = handle.ip;
}

NodeHandle::NodeHandle( const TransportAddress& ta )
{
    this->setIp(ta.getIp());
    this->setPort(ta.getPort());
    this->setKey(OverlayKey::UNSPECIFIED_KEY);
}

//complete constructor
NodeHandle::NodeHandle( const OverlayKey& key,
                              const IPvXAddress& ip, int port )
{
    this->setIp(ip);
    this->setPort(port);
    this->setKey(key);
}

NodeHandle::NodeHandle( const OverlayKey& key, const TransportAddress& ta )
{
    this->setIp(ta.getIp());
    this->setPort(ta.getPort());
    this->setKey(key);
}

//public
bool NodeHandle::isUnspecified() const
{
    return (ip.isUnspecified() || key.isUnspecified());
}

//public
NodeHandle& NodeHandle::operator=(const NodeHandle& rhs)
{
    this->key = rhs.key;
    this->ip = rhs.ip;
    this->port = rhs.port;

    return *this;
}

//public
bool NodeHandle::operator==(const NodeHandle& rhs) const
{
    assertUnspecified(rhs);
    return (this->port == rhs.port && this->ip == rhs.ip &&
            this->key == rhs.key);
}

//public
bool NodeHandle::operator!=(const NodeHandle& rhs) const
{
    assertUnspecified( rhs );
    return !operator==(rhs);
}

//public
bool NodeHandle::operator<(const NodeHandle &rhs) const
{
    assertUnspecified(rhs);
    int cmp = key.compareTo(rhs.key);
    if (cmp < 0) {
        return true;
    } else if (cmp > 0) {
        return false;
    } else if (ip < rhs.ip) {
        return true;
    } else if (rhs.ip < ip) {
        return false;
    } else if (port < rhs.port) {
        return true;
    }

    return false;
}

//public
bool NodeHandle::operator>(const NodeHandle &rhs) const
{
    assertUnspecified(rhs);
    int cmp = key.compareTo(rhs.key);
    if (cmp > 0) {
        return true;
    } else if (cmp < 0) {
        return false;
    } else if (rhs.ip < ip) {
        return true;
    } else if (ip < rhs.ip) {
        return false;
    } else if (port > rhs.port) {
        return true;
    }

    return false;
}

//public
bool NodeHandle::operator<=(const NodeHandle &rhs) const
{
    return !operator>(rhs);
}

//public
bool NodeHandle::operator>=(const NodeHandle &rhs) const
{
    return !operator<(rhs);
}

//public
void NodeHandle::setKey( const OverlayKey& key )
{
    this->key = key;
}

//public
const OverlayKey& NodeHandle::getKey() const
{
    return key;
}


//private
inline void NodeHandle::assertUnspecified( const NodeHandle& handle ) const
{
    if ( this->isUnspecified() || handle.isUnspecified() )
        opp_error("NodeHandle: Trying to compare unspecified NodeHandle!");
}


void NodeHandle::netPack(cCommBuffer *b)
{
    //cMessage::netPack(b);
    doPacking(b,this->ip);
    doPacking(b,this->key);
    doPacking(b,this->port);
}

void NodeHandle::netUnpack(cCommBuffer *b)
{
    //cMessage::netUnpack(b);
    doUnpacking(b,this->ip);
    doUnpacking(b,this->key);
    doUnpacking(b,this->port);
}

TransportAddress* NodeHandle::dup() const
{
    return new NodeHandle(*this);
}
