//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
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

import inet.networklayer.common.L3Address;

cplusplus {{
#include "inet/routing/dymo/DYMOdefs.h"
}}

namespace inet::dymo;


enum DYMOMetricType {
    HOP_COUNT = 3;    // Hop Count has Metric Type assignment 3
};

class DYMOSequenceNumber
{
    @existingClass;
    @opaque;
    @byValue;
    @fromString(string2ulong($));
    @toString(ulong2string($));
    @defaultValue(0);
    @toValue((omnetpp::intval_t)($));
    @fromValue(omnetpp::checked_int_cast<uint32_t>($.intValue()));
}

//
// classes
//


class AddressBlock extends cObject
{
    L3Address address;
    int prefixLength;
    bool hasValidityTime;
    simtime_t validityTime;
    bool hasMetric;
    double metric;
    bool hasMetricType;
    DYMOMetricType metricType = (DYMOMetricType)-1;
    bool hasSequenceNumber;
    DYMOSequenceNumber sequenceNumber = 0;
}

//
// DYMO timer messages
//

message RREQTimer {
    L3Address target;
}

message RREQWaitRREPTimer extends RREQTimer {
    int retryCount;
}

message RREQBackoffTimer extends RREQTimer {
    int retryCount;
}

message RREQHolddownTimer extends RREQTimer {
}

//
// DYMO packets
//

packet DYMOPacket {
    int hopLimit;
}

packet RteMsg extends DYMOPacket {
    int hopCount;
    AddressBlock originatorNode;
    AddressBlock targetNode;
    AddressBlock addedNode[];
}

//
// DYMO RREQ packet
//
packet RREQ extends RteMsg {
}

//
// DYMO RREP packet
//
packet RREP extends RteMsg {
}

//
// DYMO RREP ACK packet
//
packet RREP_ACK extends DYMOPacket {
}

//
// DYMO RERR packet
//
packet RERR extends DYMOPacket {
    L3Address PktSource; 
    AddressBlock unreachableNode[];
}
