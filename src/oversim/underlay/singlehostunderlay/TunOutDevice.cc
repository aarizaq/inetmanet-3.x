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
 * @file TunOutDevice.cc
 * @author Stephan Krause
 */

#include "TunOutDevice.h"
#include "IPv4Datagram_m.h"
#include "UDPPacket.h"

Define_Module(TunOutDevice);

#if not defined _WIN32 && not defined __APPLE__

#include <netinet/ip.h>
#include <netinet/udp.h>

char* TunOutDevice::encapsulate(cPacket *msg,
                                unsigned int* length,
                                sockaddr** addr,
                                socklen_t* addrlen)
{
    *addr = 0;
    *addrlen = 0;

    struct udppseudohdr {
        uint32_t saddr;
        uint32_t daddr;
        uint8_t zero;
        uint8_t protocol;
        uint16_t lenght;
    }*pseudohdr;

    unsigned int payloadlen;
    static unsigned int iplen = 20; // we don't generate IP options
    static unsigned int udplen = 8;
    cPacket* payloadMsg = NULL;
    char* buf = NULL, *payload = NULL;
    uint32_t saddr, daddr;
    volatile iphdr* ip_buf;
    volatile udphdr* udp_buf;

    IPv4Datagram* IP = check_and_cast<IPv4Datagram*>(msg);

    // FIXME: Cast ICMP-Messages
    UDPPacket* UDP = dynamic_cast<UDPPacket*>(IP->decapsulate());
    if (!UDP) {
        EV << "[TunOutDevice::encapsulate()]\n"
           << "    Can't parse non-UDP packets (e.g. ICMP) (yet)"
           << endl;
        goto parse_error;
    }

    // TODO(?) Handle fragmented packets
    if( IP->getMoreFragments() ) {
        EV << "[TunOutDevice::encapsulate()]\n"
            << "    Can't parse fragmented packets"
            << endl;
        goto parse_error;
    }
    payloadMsg = UDP->decapsulate();

    // parse payload
    payload = parser->encapsulatePayload(payloadMsg, &payloadlen);
    if (!payload ) {
        EV << "[TunOutDevice::encapsulate()]\n"
           << "    Can't parse packet payload, dropping packet"
           << endl;
        goto parse_error;
    }

    *length = payloadlen + iplen + udplen;
    if( *length > mtu ) {
        EV << "[TunOutDevice::encapsulate()]\n"
           << "    Error: Packet too big! Size = " << *length << " MTU = " << mtu
           << endl;
        goto parse_error;
    }

    buf = new char[*length];

    // We use the buffer to build an ip packet.
    // To minimise unnecessary copying, we start with the payload
    // and write it to the end of the buffer
    memcpy( (buf + iplen + udplen), payload, payloadlen);

    // write udp header in front of the payload
    udp_buf = (udphdr*) (buf + iplen);
    udp_buf->source = htons(UDP->getSourcePort());
    udp_buf->dest = htons(UDP->getDestinationPort());
    udp_buf->len = htons(udplen + payloadlen);
    udp_buf->check = 0;

    // Write udp pseudoheader in from of udp header
    // this will be overwritten by ip header
    pseudohdr = (udppseudohdr*) (buf + iplen - sizeof(struct udppseudohdr));
    saddr = htonl(IP->getSrcAddress().getInt());
    daddr = htonl(IP->getDestAddress().getInt());
    pseudohdr->saddr = saddr;
    pseudohdr->daddr = daddr;
    pseudohdr->zero = 0;
    pseudohdr->protocol = IPPROTO_UDP;
    pseudohdr->lenght = udp_buf->len;

    // compute UDP checksum
    udp_buf->check = cksum((uint16_t*) pseudohdr, sizeof(struct udppseudohdr) + udplen + payloadlen);

    // write ip header to begin of buffer
    ip_buf = (iphdr*) buf;
    ip_buf->version = 4; // IPv4
    ip_buf->ihl = iplen / 4;
    ip_buf->tos = IP->getDiffServCodePoint();
    ip_buf->tot_len = htons(*length);
    ip_buf->id = htons(IP->getIdentification());
    ip_buf->frag_off = htons(IP_DF); // DF, no fragments
    ip_buf->ttl = IP->getTimeToLive();
    ip_buf->protocol = IPPROTO_UDP;
    ip_buf->saddr = saddr;
    ip_buf->daddr = daddr;
    ip_buf->check = 0;
    ip_buf->check = cksum((uint16_t*) ip_buf, iplen);

    delete IP;
    delete UDP;
    delete payloadMsg;
    delete payload;

    return buf;

parse_error:
    delete IP;
    delete UDP;
    delete payloadMsg;
    delete payload;
    return NULL;

}

cPacket* TunOutDevice::decapsulate(char* buf,
                                    uint32_t length,
                                    sockaddr* addr,
                                    socklen_t addrlen)
{
    // Message starts with IP header
    iphdr* ip_buf = (iphdr*) buf;
    udphdr* udp_buf;
    IPv4Datagram* IP = new IPv4Datagram;
    UDPPacket* UDP = new UDPPacket;
    cPacket* payload = 0;
    unsigned int payloadLen, datagramlen;
    unsigned int packetlen = ntohs(ip_buf->tot_len);

    // Parsing of IP header, sanity checks
    if (  packetlen != length ) {
        EV << "[TunOutDevice::decapsulate()]\n"
           << "    Dropping bogus packet, header says: length = " << packetlen << "\n"
           << "    but actual length = " << length
           << endl;
        goto parse_error;
    }
    if (  packetlen > mtu ) {
        EV << "[TunOutDevice::decapsulate()]\n"
           << "    Dropping bogus packet, length = " << packetlen << "\n"
           << "    but mtu = " << mtu
           << endl;
        goto parse_error;
    }
    if ( ip_buf->version != 4 ) {
        EV << "[TunOutDevice::decapsulate()]\n"
           << "    Dropping Packet: Packet is not IPv4"
           << endl;
        goto parse_error;
    }
    if ( ntohs(ip_buf->frag_off) & 0xBFFF ) { // mask DF bit
        EV << "[TunOutDevice::decapsulate()]\n"
           << "    Dropping Packet: Can't handle fragmented packets"
           << endl;
        goto parse_error;
    }
    if ( ip_buf->protocol != IPPROTO_UDP ) { // FIXME: allow ICMP packets
        EV << "[TunOutDevice::decapsulate()]\n"
           << "    Dropping Packet: Packet is not UDP"
           << endl;
        goto parse_error;
    }
    IP->setSrcAddress( IPv4Address( ntohl(ip_buf->saddr) ));
    IP->setDestAddress( IPv4Address( ntohl(ip_buf->daddr) ));
    IP->setTransportProtocol( ip_buf->protocol );
    IP->setTimeToLive( ip_buf->ttl );
    IP->setIdentification( ntohs(ip_buf->id) );
    IP->setMoreFragments( false );
    IP->setDontFragment( true );
    IP->setFragmentOffset( 0 );
    IP->setDiffServCodePoint( ip_buf->tos );
    IP->setBitLength( ip_buf->ihl*32 );
    // FIXME: check IP and UDP checksum...

    // Parse UDP header, sanity checks
    udp_buf = (udphdr*)( ((uint32_t *)ip_buf) + ip_buf->ihl );
    datagramlen = ntohs(udp_buf->len);
    if ( (datagramlen != packetlen - ip_buf->ihl*4) ) {
        EV << "[TunOutDevice::decapsulate()]\n"
           << "    Dropping Packet: Bogus UDP datagram length: len = " << datagramlen << "\n"
           << "    packetlen = " << packetlen << " ihl*4 " << ip_buf->ihl*4
           << endl;
        goto parse_error;
    }
    UDP->setSourcePort( ntohs( udp_buf->source ));
    UDP->setDestinationPort( ntohs( udp_buf->dest ));
    UDP->setByteLength( sizeof( struct udphdr ) );

    // parse payload
    payloadLen = datagramlen - sizeof( struct udphdr );
    payload = parser->decapsulatePayload( ((char*) udp_buf) + sizeof( struct udphdr ), payloadLen );
    if (!payload) {
        EV << "[TunOutDevice::decapsulate()]\n"
           << "    Parsing of Payload failed, dropping packet"
           << endl;
        goto parse_error;
    }
    // encapsulate messages
    UDP->encapsulate( payload );
    IP->encapsulate( UDP );

    delete[] buf;
    delete addr;
    return IP;

    // In case the parsing of the packet failed, free allocated memory
parse_error:
    delete[] buf;
    delete addr;
    delete IP;
    delete UDP;
    return NULL;
}

#else

cPacket* TunOutDevice::decapsulate(char* buf,
                                   uint32_t length,
                                   sockaddr* addr,
                                   socklen_t addrlen)
{
    throw cRuntimeError("TunOutDevice::decapsulate(): Not supported on Windows/Mac OS yet");
}

char* TunOutDevice::encapsulate(cPacket *msg,
                                unsigned int* length,
                                sockaddr** addr,
                                socklen_t* addrlen)
{
    throw cRuntimeError("TunOutDevice::encapsulate(): Not supported on Windows/Mac OS yet");
}

#endif
