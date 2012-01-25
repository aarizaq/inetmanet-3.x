//
// Copyright (C) 2010 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

/**
 * @file KademliaNodeHandle.cc
 * @author Bernhard Heep
 */


#include <KademliaNodeHandle.h>


std::ostream& operator<<(std::ostream& os, const MarkedNodeHandle& n)
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

    if (n.isAlive) {
        os << " ALIVE";
    } else {
        os << " NOT ALIVE";
    }

    return os;
};
