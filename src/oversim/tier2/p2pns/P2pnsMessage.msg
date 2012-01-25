//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

//
// @author Ingmar Baumgart
//

cplusplus {{
#include <IPvXAddress.h>
#include <NodeHandle.h>
#include <TransportAddress.h>
#include <CommonMessages_m.h>
#include <BinaryValue.h>

#define P2PNSTUNNELMESSAGE_L(msg) (KEY_L + \
                                   msg->getPayload().size() * sizeof(char))
}}

class noncobject IPvXAddress;
class noncobject NodeHandle;
class noncobject OverlayKey;
class noncobject TransportAddress;
class noncobject BinaryValue;
class OverSimMessage;
class BaseOverlayMessage;
class BaseRpcMessage;
class BaseCallMessage;
class BaseResponseMessage;

//
// Message to TUNNEL application IPv6 packets to destination nodeId 
//
packet P2pnsTunnelMessage extends OverSimMessage
{
    OverlayKey srcId;   // the id of the originator
    BinaryValue payload; // the IPv6 message which is tunneled
}

message P2pnsKeepaliveTimer
{
    OverlayKey key;      // the nodeId of the cache entry to ping 
}
