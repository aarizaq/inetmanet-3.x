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
 * @file GenericPacketParser.h
 * @author Bernhard Heep
 */

#ifndef __GENERICPACKETPARSER_H_
#define __GENERICPACKETPARSER_H_

#include <omnetpp.h>

#include <PacketParser.h>
#include <cnetcommbuffer.h>

/**
 * A message parser using the cMemCommBuffer to serialize cmessages
 *
 * @author Bernhard Heep
 */
class GenericPacketParser : public PacketParser
{
public:
    /**
     * serializes messages in a buffer
     *
     * @param msg the message to serialize
     * @param length the length of the message
     * @return the encapsulated messages as a char[] of size length
     */
    char* encapsulatePayload(cPacket *msg, unsigned int* length);

    /**
     * deserializes messages from a char[] of size length
     *
     * @param buf the buffer to extract the message from
     * @param length the length of the buffer
     * @return the decapsulated cPacket
     */
    cPacket* decapsulatePayload(char* buf, unsigned int length);

private:
    cNetCommBuffer commBuffer; /**< the buffer used to encapsulate and decapsulate messages */
};

#endif
