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
 * @file PacketParser.h
 * @author Stephan Krause
 */

#ifndef __PACKETPARSER_H__
#define __PACKETPARSER_H__

#include <omnetpp.h>

/**
 * Class that performes parsing of the payload of packets that are send to or received by the tun device
 */
class PacketParser : public cSimpleModule
{

public:
    /**
     * Called on initialisation
     */
    virtual void initialize()
    {}
    ;

    /**
     * Is called if the modules receives a message. That should never happen
     */
    virtual void handleMessage(cMessage *msg)
    {
        opp_error("A PacketParser is not intendet to receive Messages!");
    };

    /**
     * Convert a cMessage to a data block for sending it to the tun device.
     * Pure virtual function, has to be implemented by inherited classes.
     *
     * \param msg A pointer to the message to be converted
     * \param length A pointer to an integer that will hold the length of the data
     * \return A pointer to the converted data
     */
    virtual char* encapsulatePayload(cPacket *msg, unsigned int* length) = 0;

    /**
     * Parses a block of data received from the tun device.
     * Pure virtual function, has to be implemented by inherited classes.
     *
     * \param buf The data to be parsed
     * \param length The length of the data
     * \return A cMessage containing the parsed data
     */
    virtual cPacket *decapsulatePayload(char* buf, unsigned int length) = 0;
};

#endif


