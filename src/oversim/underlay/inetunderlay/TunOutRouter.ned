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


package inet.oversim.underlay.inetunderlay;

import inet.base.NotificationBoard;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.oversim.common.IPacketParser;
import inet.oversim.underlay.singlehostunderlay.TunOutDevice;


//
// \TunOut router.
//
module TunOutRouter
{
    parameters:
        @node();
        string parser; // name of the PacketParser to use
        string routingFile = default("");
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
        routingTable: RoutingTable {
            parameters:
                IPForward = true; // true if ip packets should be forwarded
                routerId = "auto"; // id of the router
                routingFile = routingFile; // use predefined routing file if given
                @display("p=240,60");
        }
        packetParser: <parser> like IPacketParser {
            parameters:
                @display("p=59,126");
        }
        networkLayer: NetworkLayer {
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

