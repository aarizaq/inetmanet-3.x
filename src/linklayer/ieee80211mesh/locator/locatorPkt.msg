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
class noncobject IPv4Address;

class noncobject MACAddress;

cplusplus {{
#include "IPv4Address.h"
#include "MACAddress.h"

// ARP header length for IPv4 (4-byte addresses) and 802 LANs (6-byte MAC addrs)
#define ARP_HEADER_BYTES   28
}}

packet LocatorPkt 
{
    byteLength = 20;//6+6+4+4 
    MACAddress apMACAddress;
    MACAddress staMACAddress;
    IPv4Address apIPAddress;
    IPv4Address staIPAddress;
}
