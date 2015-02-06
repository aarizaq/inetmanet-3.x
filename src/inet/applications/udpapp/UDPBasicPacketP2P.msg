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
cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
}}

namespace inet;

class noncobject L3Address;

enum PacketType
{
    REQUEST = 1;
    SEGMEN = 2;
    GENERAL = 3;
}

packet UDPBasicPacketP2P 
{
    L3Address nodeId;
    L3Address destination;
    int type @enum(PacketType);
    int64_t segmentRequest = -1;
    uint64_t totalSize;
    uint64_t segmentId;
    uint16_t subSegmentId;
    uint16_t subSegmentRequest[];
    char mapSegments[];
}

packet UDPBasicPacketP2PNotification 
{
    L3Address nodeId;
    uint64_t seqnum;    
    int mapSegmentsSize;
    char mapSegments[];
}
