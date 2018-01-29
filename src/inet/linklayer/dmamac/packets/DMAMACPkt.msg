//**************************************************************************
// * file:        DMAMACPkt.msg
// *
// * author:      A. Ajith Kumar S.
// * copyright:   (c) A. Ajith Kumar S. 
// * homepage:    www.hib.no/ansatte/aaks
// * email:       aji3003 @ gmail.com
// **************************************************************************
// * part of:     A dual mode adaptive MAC (DMAMAC) protocol for WSAN.
// * Refined on:  25-Apr-2015
// **************************************************************************
// *This file is part of DMAMAC (DMAMAC Protocol Implementation on MiXiM-OMNeT).
// *
// *DMAMAC is free software: you can redistribute it and/or modify
// *it under the terms of the GNU General Public License as published by
// *the Free Software Foundation, either version 3 of the License, or
// *(at your option) any later version.
// *
// *DMAMAC is distributed in the hope that it will be useful,
// *but WITHOUT ANY WARRANTY; without even the implied warranty of
// *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *GNU General Public License for more details.
// *
// *You should have received a copy of the GNU General Public License
// *along with DMAMAC.  If not, see <http://www.gnu.org/licenses/>./
// **************************************************************************

cplusplus {{
  #include "inet/linklayer/base/MACFrameBase_m.h"
}}

namespace inet;

class MACFrameBase;

class noncobject MACAddress;


//
// Defines all fields of an DMAMAC frame (Extends MacPkt)
//
class Alarms {
   MACAddress address;
   uint8_t alarms = 0;
}

packet DMAMACPkt extends MACFrameBase
{
    short nextHopUp;   // Multi-hop facilty, next hop towards sink
    short nextHopDown; // Multi-hop facilty, next hop towards sink 
	short numTX;       // Number of transmission attempts
    short type;
    short mySlot;      // the node's current slot number
    short ttl;         // Time to live
    MACAddress sourceAddress;
    MACAddress destinationAddress;
    int networkId = -1;
    unsigned long sequence;
    unsigned char seq;
    simtime_t timeRef;
    int initialSeed = -1;
    uint8_t alarms = 0;
    Alarms alarmsArray[];
}

packet DMAMACSinkPkt extends DMAMACPkt
{
    bool changeMacMode;   // Multi-hop facilty, next hop towards sink
	int channel = -1;	
    double timeSlot;
    int numSlots;
    int numSlot;
    int macMode;
    
    int channelNumSlots;
    int tdmaNumSlots;
    int currentTimeSlot;
    int currentChannelSlot;    
    simtime_t nextSlot;
}

