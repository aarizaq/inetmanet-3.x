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

package inet.oversim.underlay.inetunderlay.ipv6;

import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv6.RoutingTable6;
import inet.nodes.ipv6.NetworkLayer6;

import inet.oversim.underlay.inetunderlay.AccessNet;


//
// Access router connects the access nets to the network backbone
//
module AccessRouter6
{
    parameters:
        @node();
    gates:
        inout pppg[];    // gates from overlay
        inout ethg[];    // unused here
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,60;i=block/control");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=150,60;i=block/table");
        }
        routingTable6: RoutingTable6 {
            parameters:
                isRouter = true;
                @display("p=240,60;i=block/table");
        }
        accessNet: AccessNet {
            parameters:
                useIPv6Addresses = true;
                @display("p=330,60");
        }
        networkLayer: NetworkLayer6 {
            parameters:
                @display("p=199,141;i=block/fork;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=90,257,row,110;q=queue;i=block/ifcard");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=286,268,row,110;q=queue;i=block/ifcard");
        }
    connections allowunconnected:
        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }
}

