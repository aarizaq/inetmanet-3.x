//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

//
// TODO generated message class
//


cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/L3Address.h"
}}

namespace inet::ieee80211;

class noncobject inet::IPv4Address;

class noncobject inet::MACAddress;

class noncobject inet::L3Address;
    
enum LocatorOpcode
{
    LocatorAssoc = 1;
    LocatorDisAssoc = 2;
    LocatorAskAddress = 3;
    LocatorAckAddress = 4;
    RequestAddress = 5;
    ReplyAddress = 6;
}

packet LocatorPkt 
{
    byteLength = 22;//6+6+4+4+2
    int opcode enum(LocatorOpcode);
    L3Address origin;
    unsigned int sequence;
    MACAddress apMACAddress;
    MACAddress staMACAddress;
    IPv4Address apIPAddress;
    IPv4Address staIPAddress;
}
