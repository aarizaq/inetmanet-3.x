//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

//
// TODO generated message class
//

cplusplus {{
#include <TransportAddress.h>
#include <CommonMessages_m.h>

static const int LAYER_L = 8;

#define COORDSOFNODECALL_L(msg) BASECALL_L(msg)
#define COORDSOFNODERESPONSE_L(msg) (BASERESPONSE_L(msg) + LAYER_L)
}}

class noncobject TransportAddress;
class BaseCallMessage;
class BaseResponseMessage;
//
// Ping RPC Call asking for Coordinates
//
packet CoordsOfNodeCall extends BaseCallMessage
{
}

//
// Ping RPC Call asking for RTT to Node
//
packet RttToNodeCall extends BaseCallMessage
{
    TransportAddress nodeToPing;
}

//
// Ping Response with Coordinates and Layer
//
// @author Christian Fickinger
//
packet CoordsOfNodeResponse extends BaseResponseMessage
{
    //    double coordinates[];
    char layer;
}

//
// Ping Response with RTT to Node
//
// @author Christian Fickinger
//
packet RttToNodeResponse extends BaseResponseMessage
{
    TransportAddress pingedNode;
    double ownCoordinates[];
    simtime_t rttToNode;
}
