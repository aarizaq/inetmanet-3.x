//
// Copyright (C) 2010 Institut fuer Telematik, Karlsruher Institut fuer Technologie (KIT)
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

///
// @file ReaSEOverlayHost.ned
// @author Markus Mauch, Bernhard Heep, Bernhard Mueller
//

package inet.oversim.underlay.reaseunderlay;

import inet.base.NotificationBoard;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.udp.UDP;
import inet.transport.tcp.TCP;
import inet.oversim.common.BootstrapList;
import inet.oversim.common.CryptoModule;
import inet.oversim.common.IOverlay;
import inet.oversim.common.ITier;
import inet.oversim.common.NeighborCache;


//
// Host that participates in the overlay
//
module ReaSEOverlayHost
{
    parameters:
        string overlayType; // overlay protocol compound module to use
        string tier1Type; // tier 1 application to use
        string tier2Type; // tier 2 module to use
        string tier3Type; // tier 3 module to use
        int numTiers; // number of tiers
        string routingFile = default("");
        bool IPForward = default(false);
        @display("bgb=537,472;i=device/wifilaptop_l;i2=block/circle_s");
    gates:
        inout pppg[]; // gates from router
        inout ethg[]; // placeholder for zero-size vector
        input overlayNeighborArrowIn[]; // incoming gate for visualizing overlay neighborship with connection arrows
        output overlayNeighborArrowOut[]; // incoming gate for visualizing overlay neighborship with connection arrows

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=68,208");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=68,293");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward; // true if ip packets should be forwarded
                routerId = ""; // id of the router
                routingFile = routingFile; // use predefined routing file if given
                @display("p=68,371");
        }
        tier3: <tier3Type> like ITier {
            parameters:
                @display("p=107,48");
        }
        tier2: <tier2Type> like ITier {
            parameters:
                @display("p=207,70");
        }
        tier1: <tier1Type> like ITier {
            parameters:
                @display("p=294,99");
        }
        overlay: <overlayType> like IOverlay {
            parameters:
                @display("p=374,154");
        }
        udp: UDP {
            parameters:
                @display("p=374,247");
        }
        tcp: TCP {
            parameters:
                @display("p=285,247");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false; // sets proxy \ARP mode (replying to \ARP requests for the addresses for which a routing table entry exists)
                @display("p=374,332;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=374,414,row,90;q=txQueue");
        }
        neighborCache: NeighborCache {
            parameters:
                @display("p=168,208");
        }
        bootstrapList: BootstrapList {
            parameters:
                @display("p=168,293");
        }
        cryptoModule: CryptoModule {
            parameters:
                @display("p=168,371");
        }
    connections allowunconnected:
        tier1.to_lowerTier --> overlay.appIn if numTiers>0;
        tier1.from_lowerTier <-- overlay.appOut if numTiers>0;
        tier1.udpOut --> udp.appIn++ if numTiers>0;
        udp.appOut++ --> tier1.udpIn if numTiers>0;
        tier1.tcpOut --> tcp.appIn++ if numTiers>0;
        tcp.appOut++ --> tier1.tcpIn if numTiers>0;

        tier2.to_lowerTier --> tier1.from_upperTier if numTiers > 1;
        tier2.from_lowerTier <-- tier1.to_upperTier if numTiers > 1;
        tier2.udpOut --> udp.appIn++ if numTiers>1;
        udp.appOut++ --> tier2.udpIn if numTiers>1;
        tier2.tcpOut --> tcp.appIn++ if numTiers>1;
        tcp.appOut++ --> tier2.tcpIn if numTiers>1;

        tier3.to_lowerTier --> tier2.from_upperTier if numTiers > 2;
        tier3.from_lowerTier <-- tier2.to_upperTier if numTiers > 2;
        tier3.udpOut --> udp.appIn++ if numTiers>2;
        udp.appOut++ --> tier3.udpIn if numTiers>2;
        tier3.tcpOut --> tcp.appIn++ if numTiers>2;
        tcp.appOut++ --> tier3.tcpIn if numTiers>2;

        overlay.udpOut --> udp.appIn++;
        overlay.udpIn <-- udp.appOut++;

        overlay.tcpOut --> tcp.appIn++;
        overlay.tcpIn <-- tcp.appOut++;

        bootstrapList.udpOut --> udp.appIn++;
        bootstrapList.udpIn <-- udp.appOut++;

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

}

