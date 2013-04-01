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

class noncobject ManetAddress;

cplusplus {{
#include "IPv4Address.h"
#include "MACAddress.h"
#include "ManetAddress.h"
}}

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
    ManetAddress origin;
    unsigned int sequence;
    MACAddress apMACAddress;
    MACAddress staMACAddress;
    IPv4Address apIPAddress;
    IPv4Address staIPAddress;
}
