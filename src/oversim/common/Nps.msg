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

static const int NPSLAYER_L = 8;

#define COORDSREQCALL_L(msg) BASECALL_L(msg)
#define COORDSREQRESPONSE_L(msg) (BASERESPONSE_L(msg) + NPSLAYER_L)
}}

class noncobject TransportAddress;
class BaseCallMessage;
class BaseResponseMessage;
//
// RPC Call asking for Coordinates
//
packet CoordsReqCall extends BaseCallMessage
{
}

//
// RPC Response with Coordinates and Layer
//
packet CoordsReqResponse extends BaseResponseMessage
{
    //    double coordinates[];
    char layer;
}

