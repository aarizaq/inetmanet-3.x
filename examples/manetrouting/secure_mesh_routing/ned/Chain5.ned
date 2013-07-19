//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.examples.manetrouting.secure_mesh_routing.ned;

import inet.networklayer.manetrouting.PASER.paser_FixManetRoutingHostShare;
import inet.nodes.inet.StandardHost;
import inet.world.radio.ChannelControl;
import ned.DatarateChannel;


network Chain5
{
    @display("bgb=600,600");
    types:
        channel ethline extends DatarateChannel
        {
            delay = 50ns;
            datarate = 100Mbps;
        }
    submodules:
        channelControl: ChannelControl {
            @display("p=60,50");
        }
        host_send: paser_FixManetRoutingHostShare {
            parameters:
                @display("p=150,150;r=230,orange;i=device/smallrouter");
                isGW = false;
                routingFile = "routing_tables/host_send_ohne_subnetz.rt";
        }
        gw0: paser_FixManetRoutingHostShare {
            parameters:
                // @display("p=750,150;r=230,blue;i=device/server2");
                @display("p=850,150;r=230,blue;i=device/server2");
                isGW = true;
                routingFile = "routing_tables/gw.rt";
        }
        Key_Distribution_Center: StandardHost {
            @display("p=900,150");
            routingFile = "routing_tables/Key_Distribution_Center.rt";
            udpType = "UDP";
            numUdpApps = 1;
        }
        host_no_send1: paser_FixManetRoutingHostShare {
            parameters:
                @display("p=325,150;r=230,green;i=device/smallrouter");
                isGW = false;
                routingFile = "routing_tables/host_no_send/host_no_send_ohne_subnetz1.rt";
        }
        host_no_send2: paser_FixManetRoutingHostShare {
            parameters:
                @display("p=500,150;r=230,green;i=device/smallrouter");
                isGW = false;
                routingFile = "routing_tables/host_no_send/host_no_send_ohne_subnetz2.rt";
        }
        host_no_send3: paser_FixManetRoutingHostShare {
            parameters:
                @display("p=675,150;r=230,green;i=device/smallrouter");
                isGW = false;
                routingFile = "routing_tables/host_no_send/host_no_send_ohne_subnetz3.rt";
        }
//        host_no_send4: paser_FixManetRoutingHostShare {
//            parameters:
//                @display("p=850,150;r=230,green;i=device/smallrouter");
//                isGW = false;
//                routingFile = "routing_tables/host_no_send/host_no_send_ohne_subnetz4.rt";
//        }
//                host_no_send5: paser_FixManetRoutingHostShare {
//            parameters:
//                @display("p=1025,150;r=230,green;i=device/smallrouter");
//                isGW = false;
//                routingFile = "routing_tables/host_no_send/host_no_send_ohne_subnetz5.rt";
//        }
//                host_no_send6: paser_FixManetRoutingHostShare {
//            parameters:
//                @display("p=1200,150;r=230,green;i=device/smallrouter");
//                isGW = false;
//                routingFile = "routing_tables/host_no_send/host_no_send_ohne_subnetz6.rt";
//        }
//        //host_no_send2: paser_FixManetRoutingHostShare {
//        //    parameters:
        //        @display("p=657,119;r=230,red;i=device/smallrouter");
        //        isGW = false;
        //        routingFile = "routing_tables/host_no_send/host_no_send_ohne_subnetz2.rt";
        //}
    connections allowunconnected:
        gw0.ethg++ <--> ethline <--> Key_Distribution_Center.ethg++;
}
