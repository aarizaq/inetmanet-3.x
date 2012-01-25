//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

package inet.oversim.overlay.bamboo;

import inet.oversim.common.IOverlay;
import inet.oversim.overlay.pastry.BasePastry;
import inet.oversim.overlay.pastry.PastryLeafSet;
import inet.oversim.overlay.pastry.PastryNeighborhoodSet;
import inet.oversim.overlay.pastry.PastryRoutingTable;


//
// The main module of the Bamboo implementation
//
// @author Gerhard Petruschat
//
simple Bamboo extends BasePastry
{
    parameters:
        @class(Bamboo);

        double localTuningInterval @unit(s);    // interval for periodic routing table maintenance
        double leafsetMaintenanceInterval @unit(s); // leafset push/pull interval
        double globalTuningInterval @unit(s);  // global tuning interval
}

//
// Implementation of the Bamboo KBR overlay as described in
// ...
//
// @author Gerhard Petruschat
//
module BambooModules like IOverlay
{
    gates:
        input udpIn;    // gate from the UDP layer
        output udpOut;    // gate to the UDP layer
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn;    // gate from the application
        output appOut;    // gate to the application

    submodules:
        bamboo: Bamboo {
            parameters:
                @display("p=60,52;i=block/circle");
        }
        pastryRoutingTable: PastryRoutingTable {
            parameters:
                @display("p=140,68;i=block/table");
        }
        pastryLeafSet: PastryLeafSet {
            parameters:
                @display("p=220,52;i=block/table");
        }
        pastryNeighborhoodSet: PastryNeighborhoodSet {
            parameters:
                @display("p=300,68;i=block/table");

        }
    connections allowunconnected:
        udpIn --> bamboo.udpIn;
        udpOut <-- bamboo.udpOut;
        appIn --> bamboo.appIn;
        appOut <-- bamboo.appOut;
}

