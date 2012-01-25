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
 * @file TunOutDevice.h
 * @author Stephan Krause
 */

#ifndef _TUNOUTDEVICE_H__
#define _TUNOUTDEVICE_H__

#include "tunoutscheduler.h"
#include "RealworldDevice.h"

/**
 * TunOutDevice is a pseudo interface that allows communcation with the real world
 * through the TunOutScheduler
 *
 * WARNING: This does ONLY work with the combination IPv4|UDP|OverlayMessage
 */
class TunOutDevice : public RealworldDevice
{
protected:

    /**
     * Converts an IP datagram to a data block for sending it to the (realworld) network
     *
     * \param msg A pointer to the message to be converted
     * \param length A pointer to an int that will hold the length of the converted data
     * \param addr Ignored (set to 0)
     * \param addrlen Ignored (set to 0)
     * \return A pointer to the converted data
     */
    virtual char* encapsulate(cPacket *msg,
		              unsigned int* length,
			      sockaddr** addr,
			      socklen_t* addrlen);

    /**
     * Parses data received from the (realworld) network and converts it into a cMessage
     *
     * \param buf A pointer to the data to be parsed
     * \param length The lenght of the buffer in bytes
     * \param addr Ignored (deleted)
     * \param addrlen Ignored
     * \return The parsed message
     */
    virtual cPacket *decapsulate(char* buf,
		                  uint32_t length,
				  sockaddr* addr,
				  socklen_t addrlen);
};

#endif


