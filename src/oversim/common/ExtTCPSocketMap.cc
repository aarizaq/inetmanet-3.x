//
// Copyright (C) 2010 Institut fuer Telematik, Karlsruher Institut fuer Technologie (KIT)
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
 * @file ExtTCPSocketMap.cc
 * @author Bernhard Mueller
 */

#include <ExtTCPSocketMap.h>

TCPSocket *ExtTCPSocketMap::findSocketFor(IPvXAddress remoteAddress, int remotePort)
{
    SocketMap::iterator i = socketMap.begin();
    while (i != socketMap.end()) {
        if (i->second->getRemoteAddress().equals(remoteAddress)) {
            if (i->second->getRemotePort() == remotePort) {
                return i->second;
            }
        }
        i++;
    }
    return NULL;
}

TCPSocket *ExtTCPSocketMap::findSocketFor(int connId)
{
    SocketMap::iterator i = socketMap.find(connId);
    ASSERT(i==socketMap.end() || i->first==i->second->getConnectionId());
    return (i==socketMap.end()) ? NULL : i->second;
}
