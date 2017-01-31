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

package inet.examples.manetrouting.saors;


import inet.routing.extras.saors.dtrouting.DTRouting;
import inet.node.inet.WirelessHost;


//
// Models a mobile host with a wireless (802.11g) card in ad-hoc mode.
// This model contains the new IEEE 802.11 implementation, Ieee80211gNicAdhoc,
// and IP, TCP and UDP protocols. The mobility model can be dynamically 
// specified with the mobilityType parameter.Supports opportunistic routing
// protocols by setting the
// **dtrouting.saorsmanager.routingProtocol
// parameter.
//
// @see MFMobileHost, WirelessHost, DTRouting, SaorsManager
//
// @author: Nikolaos Vastardis
//
module MobileDTRoutingHost extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");  // use adhoc management
        forwarding = default(true);

    submodules:
        dtrouting: DTRouting {
            parameters:
                @display("p=204,287");
        }
    connections:
        networkLayer.transportOut++ --> dtrouting.ipIn;
        networkLayer.transportIn++ <-- dtrouting.ipOut;
}

