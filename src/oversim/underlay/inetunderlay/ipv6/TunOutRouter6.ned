//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv6.RoutingTable6;
import inet.nodes.ipv6.NetworkLayer6;
import inet.oversim.common.IPacketParser;
import inet.oversim.underlay.singlehostunderlay.TunOutDevice;


//
// \TunOut router.
//
module TunOutRouter6
{
    parameters:
        @node();
        string parser; // name of the PacketParser to use
        @display("i=misc/globe");
    gates:
        inout pppg[]; // incoming gates to router
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,60");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=150,60");
        }
        routingTable6: RoutingTable6 {
            parameters:
                isRouter = true;
                @display("p=240,60");
        }
        packetParser: <parser> like IPacketParser {
            parameters:
                @display("p=59,126");
        }
        networkLayer: NetworkLayer6 {
            parameters:
                @display("p=200,141;q=queue");
            gates:
                ifIn[sizeof(pppg)+1];
                ifOut[sizeof(pppg)+1];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=90,257,row,110;q=l2queue");
        }
        tunDev: TunOutDevice {
            parameters:
                @display("p=145,257,row,110;q=l2queue");
        }
    connections allowunconnected:
        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

        tunDev.netwOut --> networkLayer.ifIn[sizeof(pppg)];
        tunDev.netwIn <-- networkLayer.ifOut[sizeof(pppg)];
}

