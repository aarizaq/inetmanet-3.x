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
 * @file GiaNode.cc 
 * @author Robert Palmer 
 */
#include <OverlayKey.h>

#include "GiaNode.h"


// predefined gia node
const GiaNode GiaNode::UNSPECIFIED_NODE;

GiaNode::GiaNode()
{
    //...
}

GiaNode::GiaNode(const NodeHandle& handle) : NodeHandle(handle)
{
    //...
}

//TODO
GiaNode::GiaNode(const NodeHandle& handle, double cap, int degree) : NodeHandle(handle)
{
    capacity = cap;
}

GiaNode& GiaNode::operator=(const NodeHandle& handle)
{
    ip = handle.getIp();
    port = handle.getPort();
    key = handle.getKey();
    capacity = 0;

    return *this;
}

void GiaNode::setCapacity(double cap)
{
    capacity = cap;
}

double GiaNode::getCapacity() const
{
    return capacity;
}

std::ostream& operator<<(std::ostream& os, const GiaNode& node)
{
    if(node.ip.isUnspecified() == true && node.key.isUnspecified() && node.port == -1) {
        os << "<unspec>";
    } else {
        os << node.ip << ":" << node.port << " "
           << node.key.toString() << " with capacity: "
           << node.capacity //<< " , degree: " << node.connectionDegree
            //<< " , sentTokens: " << node.sentTokens << " , receivedTokens: " << node.receivedTokens;
            ;
    }
    return os;
}

// bool GiaNode::operator==(const GiaNode& rhs) const
// {
//     if(this->getNodeHandle() != rhs.getNodeHandle())
//         return false;
//     return true;
// }

// bool GiaNode::operator!=(const GiaNode& rhs) const
// {
//     if(this->getNodeHandle() == rhs.getNodeHandle())
//         return false;
//     return true;
// }
