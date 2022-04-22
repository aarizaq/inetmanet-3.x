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
#include "SAORSBase_BeaconBlock.h"
#include "inet/routing/extras/dymo_fau/DYMO_AddressBlock.h"
#include "inet/routing/extras/dymo_fau/InspectorHelpers.h"
#include <vector>
#include <sys/types.h>
#include <iostream>
}}

namespace inet::inetmanet;

cplusplus {{
typedef std::vector<SAORSBase_BeaconBlock> VectorOfSAORSBeaconBlocks;

inline std::ostream& operator<<(std::ostream& out, VectorOfSAORSBeaconBlocks& vctr) {
	SAORSBase_BeaconBlock entry;
	
	out << "{";
	
    for(std::vector<SAORSBase_BeaconBlock>::iterator iter=vctr.begin(); iter < vctr.end(); iter++) {
    	entry=*iter;
    	out << "Address: " << entry.getAddress() << " Prob: " << entry.getBeacons() << ", " << endl;
    }

    out << "}" << endl;

    return out;
}
}}

class  VectorOfSAORSBeaconBlocks { @existingClass; @opaque;}


//
// <b> The SAORS Beacon Message </b>
//
// SAORS is an opportunistic routing protocol development system
// and therefore based on a beacon mechanism. Consequently, beacons
// are the most important component of the system. Beacons are 
// transmitted periodically and contain information such as the
// social neighbours of a node and other global metrics such
// as betweenness. The metrics that are contained in the routing
// entries are transmitted using the BeaconEntries, achieving
// differentiation between each entry. On the other hand, metrics
// characterizing the transmitting node itself, are send included
// dirrectly into the beacon. 
//
// @see SAORSBase
//
// @author: Nikolaos Vastardis
//
message SAORS_BEACON extends DYMO_RM
{
    double betw;
	VectorOfSAORSBeaconBlocks BeaconEntries;
}
