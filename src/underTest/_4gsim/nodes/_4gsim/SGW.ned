//
// Copyright (C) 2012 Calin Cerchez
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

package inet.underTest._4gsim.nodes._4gsim;

import inet.underTest._4gsim.util.gateway.GatewayTable;
import inet.underTest._4gsim.networklayer.gtp.GTPUser;
import inet.underTest._4gsim.util.subscriber.SubscriberTable;
import inet.applications.pingapp.PingApp;
import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ext.ExtInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.transport.sctp.SCTP;
import inet.transport.ITCP;
import inet.transport.udp.UDP;
import inet.util.TCPDump;
import inet.nodes.inet.NetworkLayer;
import inet.underTest._4gsim.networklayer.gtp.GTPControl;
import inet.underTest._4gsim.networklayer.gtp.tunnel.TunnelEndpointTable;
import inet.underTest._4gsim.networklayer.gtp.path.GTPPathTable;

module SGW
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/router");
        int numUdpApps = 2;
        string udpAppType = default("n/a");
        bool IPForward = default(false);
        int namid = default(-1);
        string routingFile = default("");
        int numExtInterfaces = default(0);
        string configFile;
        string dumpFile = default("");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=57,54");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=57,134");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=57,214");
        }
        udp: UDP {
            parameters:
                @display("p=336,141");
        }
        pingApp: PingApp {
            parameters:
                @display("p=501,230");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=336,230;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                ifOut[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=186,390,row,90;q=txQueue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=336,390,row,90;q=txQueue");
        }
        ext[numExtInterfaces]: ExtInterface {
            parameters:
                @display("p=501,390,row,90;q=txQueue;i=block/ifcard");
        }
        tcpdump: TCPDump {
            parameters:
                @display("p=336,302;i=abstract/cache_s");
                dumpFile = dumpFile;
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                hlIn[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                ifOut[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                hlOut[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
        }

        gtpc: GTPControl {
            @display("p=414,67");
            configFile = configFile;
        }
        subscriberTable: SubscriberTable {
            @display("p=59,368");
        }
        gtpu: GTPUser {
            @display("p=277,70");
            configFile = configFile;
        }
        tunnelEndpointTable: TunnelEndpointTable {
            @display("p=186,180");
        }
        pathTable: GTPPathTable {
            @display("p=186,246");
        }
    connections allowunconnected:
        gtpc.udpOut --> udp.appIn++;
        gtpc.udpIn <-- udp.appOut++;

        gtpu.udpOut --> udp.appIn++;
        gtpu.udpIn <-- udp.appOut++;

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        networkLayer.pingOut++ --> pingApp.pingIn;
        networkLayer.pingIn++ <-- pingApp.pingOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> tcpdump.ifIn[i];
            tcpdump.hlOut[i] --> networkLayer.ifIn[i];
            ppp[i].upperLayerIn <-- tcpdump.ifOut[i];
            tcpdump.hlIn[i] <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> tcpdump.ifIn[sizeof(pppg)+i];
            tcpdump.hlOut[sizeof(pppg)+i] --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].upperLayerIn <-- tcpdump.ifOut[sizeof(pppg)+i];
            tcpdump.hlIn[sizeof(pppg)+i] <-- networkLayer.ifOut[sizeof(pppg)+i];
        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> tcpdump.ifIn[sizeof(pppg)+sizeof(ethg)+i];
            tcpdump.hlOut[sizeof(pppg)+sizeof(ethg)+i] --> networkLayer.ifIn[sizeof(pppg)+sizeof(ethg)+i];
            ext[i].upperLayerIn <-- tcpdump.ifOut[sizeof(pppg)+sizeof(ethg)+i];
            tcpdump.hlIn[sizeof(pppg)+sizeof(ethg)+i] <-- networkLayer.ifOut[sizeof(pppg)+sizeof(ethg)+i];
        }
}
