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


package inet.oversim.underlay.simpleunderlay;

import inet.base.NotificationBoard;
import inet.networklayer.common.InterfaceTable;
import inet.oversim.common.BootstrapList;
import inet.oversim.common.CryptoModule;
import inet.oversim.common.IOverlay;
import inet.oversim.common.ITier;
import inet.oversim.common.NeighborCache;


//
// Host in the simple network that participates in the overlay
//
// @author Stephan Krause, Bernhard Heep, Ingmar Baumgart
//
module SimpleOverlayHost
{
    parameters:
        string overlayType; // overlay protocol compound module to use
        string tier1Type; // tier 1 application to use
        string tier2Type; // tier 2 module to use
        string tier3Type; // tier 3 module to use
        int numTiers; // number of tiers

        @display("bgb=433,386;i=device/wifilaptop_l;i2=block/circle_s");
    gates:
        input overlayNeighborArrowIn[]; // incoming gate for visualizing overlay neighborship with connection arrows
        output overlayNeighborArrowOut[]; // incoming gate for visualizing overlay neighborship with connection arrows

    submodules:
        tier3: <tier3Type> like ITier {
            parameters:
                @display("p=64,57;i=block/segm");
        }
        tier2: <tier2Type> like ITier {
            parameters:
                @display("p=179,90;i=block/segm");
        }
        tier1: <tier1Type> like ITier {
            parameters:
                @display("p=286,135;i=block/segm");
        }
        overlay: <overlayType> like IOverlay {
            parameters:
                @display("p=382,193;i=block/network2");
        }
        udp: SimpleUDP {
            parameters:
                @display("p=382,305");
        }
        tcp: SimpleTCP {
            parameters:
                @display("p=286,305");
        }
        cryptoModule: CryptoModule {
            parameters:
                @display("p=64,154");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=64,232");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=64,305");
        }
        neighborCache: NeighborCache {
            parameters:
                @display("p=179,305");
        }
        bootstrapList: BootstrapList {
            parameters:
                @display("p=179,232");
        }
    connections allowunconnected:
        tier1.to_lowerTier --> overlay.appIn if numTiers > 0;
        tier1.from_lowerTier <-- overlay.appOut if numTiers > 0;
        tier1.udpOut --> udp.appIn++ if numTiers > 0;
        udp.appOut++ --> tier1.udpIn if numTiers > 0;
        tier1.tcpOut --> tcp.appIn++ if numTiers > 0;
        tcp.appOut++ --> tier1.tcpIn if numTiers > 0;

        tier2.to_lowerTier --> tier1.from_upperTier if numTiers > 1;
        tier2.from_lowerTier <-- tier1.to_upperTier if numTiers > 1;
        tier2.udpOut --> udp.appIn++ if numTiers > 1;
        udp.appOut++ --> tier2.udpIn if numTiers > 1;
        tier2.tcpOut --> tcp.appIn++ if numTiers > 1;
        tcp.appOut++ --> tier2.tcpIn if numTiers > 1;

        tier3.to_lowerTier --> tier2.from_upperTier if numTiers > 2;
        tier3.from_lowerTier <-- tier2.to_upperTier if numTiers > 2;
        tier3.udpOut --> udp.appIn++ if numTiers > 2;
        udp.appOut++ --> tier3.udpIn if numTiers > 2;
        tier3.tcpOut --> tcp.appIn++ if numTiers > 2;
        tcp.appOut++ --> tier3.tcpIn if numTiers > 2;

        overlay.udpOut --> udp.appIn++;
        overlay.udpIn <-- udp.appOut++;

        overlay.tcpOut --> tcp.appIn++;
        overlay.tcpIn <-- tcp.appOut++;

        bootstrapList.udpOut --> udp.appIn++;
        bootstrapList.udpIn <-- udp.appOut++;
}

