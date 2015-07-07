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

package inet.routing.extras.saors.dtrouting;


import inet.routing.extras.saors.SaorsManager.SaorsManager;

//
// DT router 
//
// This router module should be attached to the network layer of the node that 
// participates in MANET routing. It contains a SaorsManeger which instantiates the
// requested routing protocol. You should configure the routing protcol to be the same
// across the whole network. Place the following in your INI file:
//
// **dtrouting.saorsmanager.routingProtocol="DT-DYMO"
// 
// along with parameters for the choosen routing algorithm.
// At the moment the following DT routing algorithms are supported:
// -> SAMPhO (Socially-Aware Multi-Phase Opportunistic routing)
// -> DT-DYMO (Delay-Toleranent DYMO)
// -> SimBetTS (The similar to DT-DYMO bit using the SimBetTS utilities)
// -> EP-DYMO (Epidemic DYMO)
// -> R-DYMO  (Random DYMO)
// 
// NOTE:
//  - A node participating in DT routing must have a single network interface with "wlan" 
//    in its name somewhere e.g: wlan0 or localwlan
//
// @see SaorsManager, SAORSBase, SAMPhO, DTDYMO, EPDYMO, RDYMO
//
// @author: Nikolaos Vastardis
//
module DTRouting
{
    parameters:
        @display("bgb=161,99;i=block/network2");
    gates:
        input from_ip;
        output to_ip;
    submodules:
        // the name of manet routing manager module must be "manetmanager", the routing protocols is ManetRouting.ned
        manetmanager: SaorsManager {
            parameters:
                // Manager parameters
                // eliminate in the routing table all the initial reference to wlan interfaces
                manetActive = true;
                // manetPurgeRoutingTables = true;
                @display("p=77,52;b=40,24;i=block/arrival");
        }
    connections allowunconnected:
        manetmanager.from_ip <-- from_ip;
        manetmanager.to_ip --> to_ip;
}






