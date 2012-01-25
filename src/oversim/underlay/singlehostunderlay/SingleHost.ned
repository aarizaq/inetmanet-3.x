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
// @author Stephan Krause, Ingmar Baumgart
//

package inet.oversim.underlay.singlehostunderlay;

import inet.base.NotificationBoard;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.udp.UDP;
import inet.oversim.common.BootstrapList;
import inet.oversim.common.CryptoModule;
import inet.oversim.common.IOverlay;
import inet.oversim.common.IPacketParser;
import inet.oversim.common.ITier;
import inet.oversim.common.NeighborCache;


//
// simulates a single host to connect it to a real network
//
module SingleHost
{
    parameters:
        string outDeviceType; // outDevice to use
        string overlayType; // overlay protocol compound module to use
        string tier1Type; // tier 1 module to use
        string tier2Type; // tier 2 module to use
        string tier3Type; // tier 3 module to use
        int numTiers; // number of tiers
        string parser; // name of the PacketParser to use
        string appParser; // name of the PacketParser to use
        string routingFile = default("");
        bool IPForward = default(false);
        @display("bgb=330,461;i=device/wifilaptop_l;i2=block/circle_s");

    gates:
        input overlayNeighborArrowIn[]; // incoming gate for direct connections from overlay neighbors
        output overlayNeighborArrowOut[]; // outgoing gate for direct connections to overlay neighbors

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=64,415");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=64,202");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward; // true if ip packets should be forwarded
                routerId = ""; // id of the router
                routingFile = routingFile; // use predefined routing file if given
                @display("p=64,271");
        }
        packetParser: <parser> like IPacketParser {
            parameters:
                @display("p=171,271;i=block/wheelbarrow");
        }
        applicationParser: <appParser> like IPacketParser {
            parameters:
                @display("p=171,342;i=block/wheelbarrow");
        }
        tier3: <tier3Type> like ITier {
            parameters:
                @display("p=54,32;i=block/segm");
        }
        tier2: <tier2Type> like ITier {
            parameters:
                @display("p=137,71;i=block/segm");
        }
        tier1: <tier1Type> like ITier {
            parameters:
                @display("p=223,110;i=block/segm");
        }
        overlay: <overlayType> like IOverlay {
            parameters:
                @display("p=287,163;i=block/network2");
        }
        udp: UDP {
            parameters:
                @display("p=287,264");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false; // sets proxy \ARP mode (replying to \ARP requests for the addresses for which a routing table entry exists)
                @display("p=287,342;i=block/fork;q=queue");
            gates:
                ifIn[1];
                ifOut[1];
        }
        outDev: <outDeviceType> like RealworldDevice {
            parameters:
                @display("p=287,415,row,90;q=txQueue;i=block/ifcard");
        }
        neighborCache: NeighborCache {
            parameters:
                @display("p=64,342");
        }
        bootstrapList: BootstrapList {
            parameters:
                @display("p=171,202");
        }
        cryptoModule: CryptoModule {
            parameters:
                @display("p=171,415");
        }
    connections allowunconnected:
        tier1.to_lowerTier --> overlay.appIn if numTiers>0;
        tier1.from_lowerTier <-- overlay.appOut if numTiers>0;
        tier1.udpOut --> udp.appIn++ if numTiers>0;
        udp.appOut++ --> tier1.udpIn if numTiers>0;

        tier2.to_lowerTier --> tier1.from_upperTier if numTiers > 1;
        tier2.from_lowerTier <-- tier1.to_upperTier if numTiers > 1;
        tier2.udpOut --> udp.appIn++ if numTiers>1;
        udp.appOut++ --> tier2.udpIn if numTiers>1;

        tier3.to_lowerTier --> tier2.from_upperTier if numTiers > 2;
        tier3.from_lowerTier <-- tier2.to_upperTier if numTiers > 2;
        tier3.udpOut --> udp.appIn++ if numTiers>2;
        udp.appOut++ --> tier3.udpIn if numTiers>2;

        overlay.udpOut --> udp.appIn++;
        overlay.udpIn <-- udp.appOut++;

        bootstrapList.udpOut --> udp.appIn++;
        bootstrapList.udpIn <-- udp.appOut++;

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        // connections to network outside
        outDev.netwOut --> networkLayer.ifIn[0];
        outDev.netwIn <-- networkLayer.ifOut[0];

}

