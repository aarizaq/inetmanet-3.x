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
 * @file ExtTCPSocketMap.h
 * @author Bernhard Mueller
 */

#ifndef EXTTCPSOCKETMAP_H_
#define EXTTCPSOCKETMAP_H_

#include <TCPSocketMap.h>
#include <IPvXAddress.h>

class ExtTCPSocketMap : public TCPSocketMap
{
public:
    /**
         * Member function to find the TCPSocket by the remote address and port
         *
         * @param remoteAddress IPvXAddress of the remote host
         * @param remotePort portnumber of the connection to connect with remote host
         *
         * @returns The TCPSocket object of the connection with the specified address and port of the remote host
     */
    virtual TCPSocket *findSocketFor(IPvXAddress remoteAddress, int remotePort);

    /**
         * Member function to find the TCPSocket by an incoming tcp message
         *
         * @param msg incoming TCP message
         *
         * @returns The TCPSocket object of the connection wich msg belongs to
     */
    virtual TCPSocket *findSocketFor(cMessage* msg){return TCPSocketMap::findSocketFor(msg);};

    /**
         * Member function to find the TCPSocket by a connection ID given
         *
         * @param connId integer ID of an connection
         *
         * @returns The TCPSocket object of the connection with connection ID connId
     */
    virtual TCPSocket *findSocketFor(int connId);
};

#endif /* EXTTCPSOCKETMAP_H_ */
