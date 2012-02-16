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
 * @file UdpOutDevice.cc
 * @author Stephan Krause
 */

#define WANT_WINSOCK2
#include <platdep/sockets.h>

#include "IPv4Datagram_m.h"
#include "UDPPacket.h"
#include "IPvXAddressResolver.h"

#include "UdpOutDevice.h"


Define_Module(UdpOutDevice);

char* UdpOutDevice::encapsulate(cPacket *msg,
                                unsigned int* length,
                                sockaddr** addr,
                                socklen_t* addrlen)
{
    cPacket* payloadMsg = NULL;
    char* payload = NULL;
    sockaddr_in* addrbuf = NULL;

    unsigned int payloadLen;

    IPv4Datagram* IP = check_and_cast<IPv4Datagram*>(msg);
    // FIXME: Cast ICMP-Messages
    UDPPacket* UDP = dynamic_cast<UDPPacket*>(IP->decapsulate());

    if (!UDP) {
        EV << "[UdpOutDevice::encapsulate()]\n"
           << "    Can't parse non-UDP packets (e.g. ICMP) (yet)"
           << endl;

        goto parse_error;
    }

    // TODO(?) Handle fragmented packets
    if( IP->getMoreFragments() ) {
        EV << "[UdpOutDevice::encapsulate()]\n"
            << "    Can't parse fragmented packets"
            << endl;
        goto parse_error;
    }

    payloadMsg = UDP->decapsulate();

    // parse payload
    payload = parser->encapsulatePayload(payloadMsg, &payloadLen);
    if (!payload) {
        EV << "[UdpOutDevice::encapsulate()]\n"
           << "    Can't parse packet payload, dropping packet"
           << endl;
        goto parse_error;
    }

    if (payloadLen > mtu) {
        EV << "[UdpOutDevice::encapsulate()]\n"
           << "    Encapsulating packet failed: packet too long"
           << endl;
        goto parse_error;
    }

    *length = payloadLen;

    // create sockaddr
    addrbuf = new sockaddr_in;
    addrbuf->sin_family = AF_INET;
    addrbuf->sin_port = htons(UDP->getDestinationPort());
    addrbuf->sin_addr.s_addr = htonl(IP->getDestAddress().getInt());
    *addrlen = sizeof(sockaddr_in);
    *addr = (sockaddr*) addrbuf;

    delete IP;
    delete UDP;
    delete payloadMsg;
    return payload;

parse_error:
    delete IP;
    delete UDP;
    delete payloadMsg;
    delete payload;
    return NULL;

}

cPacket* UdpOutDevice::decapsulate(char* buf,
                                    uint32_t length,
                                    sockaddr* addr,
                                    socklen_t addrlen)
{
    if (!addr) {
        opp_error("UdpOutDevice::decapsulate called without providing "
                  "sockaddr (addr = NULL)");
    }

    if (addrlen != sizeof(sockaddr_in) ) {
        opp_error("UdpOutDevice::decapsulate called with wrong sockaddr length. "
                  "Only IPv4 is supported at the moment!");
    }
    sockaddr_in* addrbuf = (sockaddr_in*) addr;

    IPv4Datagram* IP = new IPv4Datagram();
    UDPPacket* UDP = new UDPPacket;
    cPacket* payload = 0;

    // Parse Payload
    payload = parser->decapsulatePayload(buf, length);

    if (!payload) {
        EV << "[UdpOutDevice::decapsulate()]\n"
           << "    Parsing of payload failed, dropping packet"
           << endl;
        goto parse_error;
    }

    // Create IP + UDP header
    IP->setSrcAddress(IPv4Address(ntohl(addrbuf->sin_addr.s_addr)));
    IP->setDestAddress(IPvXAddressResolver().addressOf(getParentModule()).get4());
    IP->setTransportProtocol(IPPROTO_UDP);
    IP->setTimeToLive(42); // Does not matter, packet ends here
    IP->setIdentification(42); // Faked: There is no way to get the real ID
    IP->setMoreFragments(false);
    IP->setDontFragment(false);
    IP->setFragmentOffset(0);
    IP->setTypeOfService(0); // Faked...
    IP->setBitLength(160);

    UDP->setSourcePort(ntohs(addrbuf->sin_port));
    UDP->setDestinationPort(getParentModule()->getSubmodule("overlay", 0)->
                            gate("appIn")->getNextGate()->getOwnerModule()->
                            par("localPort").longValue());

    UDP->setByteLength(8);

    // Done...
    UDP->encapsulate(payload);
    IP->encapsulate(UDP);
    delete[] buf;
    delete addr;
    return IP;

parse_error:
    delete IP;
    delete UDP;
    delete[] buf;
    delete addr;
    delete payload;
    return NULL;
}

