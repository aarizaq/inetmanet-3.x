//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
// @author Bernhard Heep
//

cplusplus {{
#include <KademliaNodeHandle.h>
#include <OverlayKey.h>
#include <CommonMessages_m.h>

static const int ALIVEFLAG_L = 8;
#define MARKEDNODEHANDLE_L (NODEHANDLE_L + ALIVEFLAG_L)

#define KADEMLIAROUTINGINFO_L(msg) (BASEOVERLAY_L(msg) + NODEHANDLE_L + \
                                    KEY_L + \
                                    msg->getNextHopsArraySize() * \
                                    MARKEDNODEHANDLE_L)
}}

class noncobject MarkedNodeHandle;
class noncobject NodeHandle;
class noncobject OverlayKey;

class BaseOverlayMessage;

//
// Message to inform node about potential next hops during recursive routing
//
packet KademliaRoutingInfoMessage extends BaseOverlayMessage
{
        NodeHandle srcNode;
        OverlayKey destKey = OverlayKey::UNSPECIFIED_KEY;
        MarkedNodeHandle nextHops[];
}
