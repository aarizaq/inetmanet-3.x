//*********************************************************************************
// File:           HipMessages.msg
//
// Authors:        Laszlo Tamas Zeke, Levente Mihalyi, Laszlo Bokor
//
// Copyright: (C) 2008-2009 BME-HT (Department of Telecommunications,
// Budapest University of Technology and Economics), Budapest, Hungary
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
//**********************************************************************************
// Part of: HIPSim++ Host Identity Protocol Simulation Framework developed by BME-HT
//**********************************************************************************


cplusplus {{
#include "IPv6Address.h"
#include "IPv6Datagram.h"
#include "IPv6ExtensionHeaders_m.h"
}}



//
// HIP header section, 2008.02 ML
//
//



cplusplus {{
//the IPv6 no next header value
const int IPPROTO_NONE = 59;
//maximum length of the HIP Parameters field is (255*8)-32 = 2008 bytes
const int HIP_MAX_HEADER_BYTES = 2008;

typedef cMessage *cMessagePtr;
typedef IPv6EncapsulatingSecurityPayloadHeader *IPv6EncapsulatingSecurityPayloadHeaderPtr;

}}


class noncobject IPv6Address;



struct IPv6EncapsulatingSecurityPayloadHeaderPtr;


struct cMessagePtr;



//HIP message types
enum HipPacketType
{

    I1 = 1;
    R1 = 2;
    I2 = 3;
    R2 = 4;
    UPDATE = 16;
    NOTIFY = 17;
    CLOSE = 18;
    CLOSE_ACK = 19;
    ESP_DATA = 666;
    UPDATE_SEQ = 20;
    UPDATE_ACK = 21;
}

//Internet draft HIP Mobility and Multihoming
//March 2007
struct HipLocator
{
    int type = 193;
    int length;
    short trafficType;
    //default is the ESP-based usage
    short locatorType = 1;
    int locatorLength;
    short reserved = 0;
    short p = 0;
    int locatorLifetime;
    //the IPv4-in-IPv6 format IPv4 address is not implemented
    IPv6Address locatorIPv6addr;
    int locatorESP_SPI;

}

//
//Host Identity Protocol header
// Internet draft October 2007
//section 5.
//
packet HIPHeaderMessage
{
    //the no next header value - no other values definied
    int nextHeader = IPPROTO_NONE;
    //minimum value is 4 bytes
    int length = 4;
    int packetType enum(HipPacketType);
    //current version is 1
    short version = 1;
    //the controls field is not implemented
    IPv6Address srcHIT;
    IPv6Address destHIT;
    //Registration Extension
    short reg_info = 0;
    short reg_req = 0;
    short reg_res = 0;
    IPv6Address from_i;
    int rvs_mac;
    IPv6Address via_rvs;

    //PARAMATERS:
    HipLocator locator[];
    unsigned int r1_counter;
    int puzzle;
    int soution;
    int seq;
    int ack;
    int diffie_hellman;
    int hip_transform;
    int encrypted;
    int host_id;
    int cert;
    int notification;
    int echo_request_signed;
    int echo_response_signed;
    int hmac;
    int hmac_2;
    int hip_signature;
    int echo_request_unsigned;
    int echo_response_unsigned;
}

packet ESPMessage
{
    IPv6EncapsulatingSecurityPayloadHeaderPtr esp;
}
