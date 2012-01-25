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
 * @file RealworldApp.cc
 * @author Stephan Krause
 */


#include "RealworldApp.h"


Define_Module(RealworldApp);

char* RealworldApp::encapsulate(cPacket *msg,
                                unsigned int* length,
                                sockaddr** addr,
                                socklen_t* addrlen)
{
    unsigned int payloadLen;
    *addr = 0;
    *addrlen = 0;

    // parse payload
    char* payload = parser->encapsulatePayload(msg, &payloadLen);
    if (!payload )
        return NULL;

    if(payloadLen > 0xffff) {
	opp_error("RealworldApp: Encapsulating packet failed: packet too long");
    }
    *length = payloadLen;

    return payload;
}

cPacket* RealworldApp::decapsulate(char* buf,
                                   uint32_t length,
                                   sockaddr* addr,
                                   socklen_t addrlen)
{
    cPacket* payload = 0;
    // "Decode" packet: 16bit payload length|payload
    payload = parser->decapsulatePayload( buf, length );
    if (!payload) {
        EV << "[RealworldApp::decapsulate()]\n"
           << "    Parsing of Payload failed, dropping packet"
           << endl;
    }

    delete buf;
    return payload;
    delete addr; // FIXME: unreachable
}

