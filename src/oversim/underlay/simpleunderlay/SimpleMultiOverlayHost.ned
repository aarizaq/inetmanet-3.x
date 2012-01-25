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
module SimpleMultiOverlayHost
{
    parameters:
        string overlayType; // overlay protocol compound module to use        
        string tier1Type; // tier 1 application to use
        string tier2Type;// tier 2 module to use
        string tier3Type; // tier 3 module to use
        int numTiers; // number of tiers
        int supportTcp; // set to 1 to include a TCP module in each host
        double numOverlayModulesPerNode; // number of parallel overlays
        string routingFile = default("");
        bool IPForward = default(false);

        @display("bgb=433,386");
    gates:
        input overlayNeighborArrowIn[]; // incoming gate for visualizing overlay neighborship with connection arrows
        output overlayNeighborArrowOut[]; // incoming gate for visualizing overlay neighborship with connection arrows
        output out; // Dummy gate for storing channel informations
        input in; // Dummy gate for connecting out

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=64,239");
        }
        tier3[numOverlayModulesPerNode]: <tier3Type> like ITier {
            parameters:
                @display("p=64,64;i=block/segm");
        }
        tier2[numOverlayModulesPerNode]: <tier2Type> like ITier {
            parameters:
                @display("p=179,104;i=block/segm");
        }
        tier1[numOverlayModulesPerNode]: <tier1Type> like ITier {
            parameters:
                @display("p=290,162;i=block/segm");
        }
        overlay[numOverlayModulesPerNode]: <overlayType> like IOverlay {
            parameters:
                @display("p=370,231;i=block/network2");
        }
        udp: SimpleUDP {
            parameters:
                @display("p=370,318");
        }
        tcp[supportTcp]: SimpleTCP {
            parameters:
                @display("p=309,318");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=64,318");
        }
        neighborCache: NeighborCache {
            parameters:
                @display("p=168,318");
        }
        cryptoModule: CryptoModule {
            parameters:
                @display("p=64,162");
        }
        bootstrapList: BootstrapList {
            parameters:
                @display("p=168,239");

        }
    connections allowunconnected:
        bootstrapList.udpOut --> udp.appIn++;
        bootstrapList.udpIn <-- udp.appOut++;

        for i=0..numOverlayModulesPerNode-1 {
            tier1[i].to_lowerTier --> overlay[i].appIn if numTiers>0;
            tier1[i].from_lowerTier <-- overlay[i].appOut if numTiers>0;
            tier1[i].udpOut --> udp.appIn++ if numTiers>0;
            udp.appOut++ --> tier1[i].udpIn if numTiers>0;
            tier1[i].tcpOut --> tcp[0].appIn++ if (numTiers>0 && supportTcp>0);
            tcp[0].appOut++ --> tier1[i].tcpIn if (numTiers>0 && supportTcp>0);


            tier2[i].to_lowerTier --> tier1[i].from_upperTier if numTiers > 1;
            tier2[i].from_lowerTier <-- tier1[i].to_upperTier if numTiers > 1;
            tier2[i].udpOut --> udp.appIn++ if numTiers>1;
            udp.appOut++ --> tier2[i].udpIn if numTiers>1;
            tier2[i].tcpOut --> tcp[0].appIn++ if (numTiers>1 && supportTcp>0);
            tcp[0].appOut++ --> tier2[i].tcpIn if (numTiers>1 && supportTcp>0);

            tier3[i].to_lowerTier --> tier2[i].from_upperTier if numTiers > 2;
            tier3[i].from_lowerTier <-- tier2[i].to_upperTier if numTiers > 2;
            tier3[i].udpOut --> udp.appIn++ if numTiers>2;
            udp.appOut++ --> tier3[i].udpIn if numTiers>2;
            tier3[i].tcpOut --> tcp[0].appIn++ if (numTiers>2 && supportTcp>0);
            tcp[0].appOut++ --> tier3[i].tcpIn if (numTiers>2 && supportTcp>0);

            overlay[i].udpOut --> udp.appIn++;
            overlay[i].udpIn <-- udp.appOut++;

            overlay[i].tcpOut --> tcp[0].appIn++ if supportTcp>0;
            overlay[i].tcpIn <-- tcp[0].appOut++ if supportTcp>0;

        }

}

