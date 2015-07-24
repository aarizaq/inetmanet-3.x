//
//  Copyright (C) 2012 Nikolaos Vastardis
//  Copyright (C) 2012 University of Essex
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
}}

class noncobject IPv4Address;

enum IPProtocolId;

namespace inet::inetmanet;

//
// <b> The Delay Tolerant Message </b>
//
// This message encapsulated IP Packets, in order to provide delay
// tolerant functionality. It include a number of entra fields.
// These inlcude the addresses of the transmitted and receiving
// nodes, making it possible for this message to be exchanges
// between nodes that are neither the source nor the final desitnation
// of the encapulated mesage. It also defines the number of copies
// of this message that can be made, in order to allow flooding-like
// protocols. Finally it defines the IP protocol type, to enable
// developement. 
//
// @see SAORSBase
//
// @author: Nikolaos Vastardis
//
packet DT_MSG extends cPacket {
    IPv4Address dstAddress;
    IPv4Address srcAddress;
    uint8_t copiesLeft;
    bool isCopy;
    uint8_t transportProtocol enum(IPProtocolId) = IP_PROT_NONE;
}
