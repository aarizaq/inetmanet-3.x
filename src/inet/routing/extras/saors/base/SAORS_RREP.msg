//
//  Copyright (C) 2012 Nikolaos Vastardis
//  Copyright (C) 2012 University of Essex
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

import inet.routing.extras.dymo_fau.DYMO_RM;
import inet.routing.extras.dymo_fau.DYMO_PacketBBMessage;

cplusplus {{
#include "inet/routing/extras/dymo_fau/DYMO_AddressBlock.h"
#include "inet/routing/extras/dymo_fau/InspectorHelpers.h"
#include <vector>
#include <sys/types.h>
}}

namespace inet::inetmanet;

//
// <b> The SAORS Routing-Reply Message </b>
//
// This is an extension of the DYMO_RM message for replying to the
// request for a path towards a destination. It includes two more fields,
// namely the searched destination and the replying node's probability
// of encountering this destination. This extra address field is
// included since the replying node has to enter its address as the
// original address. This was the initial nodes that transmitted the
// SAORS_RREQ will be able to differantiate whether the replies
// received define a path or an opportunity of meeting the requested
// destination, according to whether the seatched field in the 
// SOARS_RREPis empty or not.
//
// @see SAORSBase
//
// @author: Nikolaos Vastardis
//
message SAORS_RREP extends DYMO_RM
{
    DYMO_AddressBlock searchedNode;
    double deliveryProb;
}
