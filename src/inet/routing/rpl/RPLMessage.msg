//
// Copyright (C) 2005 Wei Yang, Ng
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

cplusplus {{
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/L3Address.h"
}}

namespace inet;

packet ICMPv6Message;

class noncobject L3Address;

class noncobject MACAddress;

// TLB options (type, length, bytes), length must be divisible by eight, max value is 8*255
// Length in bytes:
enum IPv6NDOptionLength {
    RPL_DIS_LENGTH = 8;  
    RPL_DIO_LENGTH =24;  
    RPL_DIO_LENGTH =4; // D bit to 0
    RPL_DIO2_LENGTH = 20; // D bit to 1
    RPL_DIO_ACK_LENGTH =4;   // D bit to 0
    RPL_DIO_ACK2_LENGTH = 20; // D bit to 1
    RPL_CONFIGURATION = 14;
    RPL_RIO_LENGTH = 24;
}


enum Grounded
{
    FLOATING  = 0;
    GROUNDED  = 1;
}


// TLB options (type, length, bytes), length must be divisible by eight, max value is 8*255
// Type codes:
enum IPv6NDOptionTypes {
    IPv6ND_SOURCE_LINK_LAYER_ADDR_OPTION = 1;          // RFC 2461
    IPv6ND_TARGET_LINK_LAYER_ADDR_OPTION = 2;          // RFC 2461
}


enum MopTypes
{
    NO_DOWNWARD  = 0;
    NON_STORING  = 1;
    STORING_NO_MULTICASt  = 2;
    STORING_MULTICASt  = 3;
}


//
// IPv6ND Prefix Information
// RFC 2461 / RFC 4861 Section 4.6.2
// RFC 3775 Section 7.2 (routerAddressFlag)
//
class RplSequrity
{
    char TFlag= 0;
    char Algorithm = 0;
    char Kim = 0;
    char Lvl = 0;
    char Flag = 0;
    int counter = 0;
    uint64_t keySource;
    uint8_t keyIndex;
}

packet RplControlMessage extends ICMPv6Message
{
    type = 155;
    int code = -1; 
}

packet RplDis extends RplControlMessage
{
    code = 0;
    char flags = 0;
    char reserved = 0;
    char options[3];
}

class RplControlOptions
{
    char type;
    char lentgh;
}


class RplDagMetric
{
    type = 0x2; // if type == 0 the node assume that this field doesn't exist
    int metricType;
    double value;
}

class RplRio
{
    type = 0x3; // if type == 0 the node assume that this field doesn't exist
    uint32_t lifetime;
    L3Address Prefix;
}

class DODAGConfig
{
    type = 0x4; // if type == 0 the node assume that this field doesn't exist
    uint8_t pcs = 0;
    uint8_t intDoubl = 20;
    uint8_t intMin = 3;
    uint8_t redun;
    uint16_t maxRankInc;
    uint16_t minHopRankInc;
    uint16_t ocp;
    uint8_t default_lifetime;
    uint16_t lifetime;
}


packet RplDio extends RplControlMessage
{
    code = 1;
    uint8_t RplInstanceId;
    uint8_t VersionNumber;
    uint8_t Rank;
    uint8_t Ground @enum(Grounded) = 0;
    uint8_t Mop @enum(MopTypes) = 0;
    uint8_t DODAGPreference = 0;
    uint8_t Dtns;
    uint8_t Flags = 0;
    char reserved = 0;
    L3Address DODAGID;
    char options[];
    RplDagMetric metric;
    RplRio rio;
    DODAGConfig config;
}


packet RplDao extends RplControlMessage
{
    code = 2;
    uint8_t RplInstanceId;
    uint8_t K = 0;
    uint8_t D = 0;
    uint8_t Flags = 0;
    uint8_t DODAGSequence = 0;
    char reserved = 0;
    L3Address DODAGID;
    // options
    L3Address Target;
    RplDagMetric metric;
}

packet RplDaoAck extends RplControlMessage
{
    code = 3;
    uint8_t RplInstanceId;
    uint8_t D = 0;
    uint8_t DODAGSequence = 0;
    uint8_t Status = 0;
    char reserved = 0;
    L3Address DODAGID;
}

