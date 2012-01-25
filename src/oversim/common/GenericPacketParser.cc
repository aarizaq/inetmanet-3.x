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
 * @file GenericPacketParser.cc
 * @author Bernhard Heep
 */
#include <omnetpp.h>

#include "GenericPacketParser.h"

Define_Module(GenericPacketParser);

char* GenericPacketParser::encapsulatePayload(cPacket *msg, unsigned int* length)
{
    commBuffer.reset();
    commBuffer.packObject(msg);

    *length = commBuffer.getMessageSize();
    char* byte_buf = new char[*length];
    memcpy(byte_buf, commBuffer.getBuffer(), *length);

    return byte_buf;
}

cPacket* GenericPacketParser::decapsulatePayload(char* buf, unsigned int length)
{
    cPacket *msg = NULL;

    commBuffer.reset();
    commBuffer.allocateAtLeast(length);
    memcpy(commBuffer.getBuffer(), buf, length);
    commBuffer.setMessageSize(length);

    try {
        msg = check_and_cast<cPacket*>(commBuffer.unpackObject());
        if (!commBuffer.isBufferEmpty()) {
            ev << "[GenericPacketParser::decapsulatePayload()]\n"
               << "    Parsing of payload failed: buffer size mismatch"
               << endl;
            delete msg;
            return NULL;
        }
//    } catch (cRuntimeError err) {
//    FIXME:
//    the above does, for some reason, not work. So we catch everything,
//    which may prevent the simulation from terminating correctly while
//    parsing a message.
    } catch (...) {
        ev << "[GenericPacketParser::decapsulatePayload()]\n"
           << "    Parsing of payload failed"
           << endl;
        delete msg;
        return NULL;
    }

    return msg;
}
