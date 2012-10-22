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

import inet.underTest._4gsim.networklayer.gtp.path.GTPPathTable;
import inet.underTest._4gsim.networklayer.gtp.tunnel.TunnelEndpointTable;
import inet.underTest._4gsim.networklayer.gtp.GTPUser;
import inet.transport.udp.UDP;
import inet.underTest._4gsim.util.subscriber.SubscriberTable;
import inet.base.NotificationBoard;
import inet.underTest._4gsim.applications.nas.NAS;
import inet.mobility.IMobility;
import inet.underTest._4gsim.linklayer.lte.LTERadio;
import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ext.ExtInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.applications.pingapp.PingApp;
import inet.util.TCPDump;
import inet.nodes.inet.NetworkLayer;
import inet.transport.sctp.SCTP;
import inet.underTest._4gsim.applications.s1ap.S1AP;

module ENB
{
    parameters:
        @display("i=device/antennatower_l;bgb=559,396");
        int numSctpApps = default(0);
        string sctpAppType = default("n/a");
        string configFile;
        bool IPForward = default(false);
        int namid = default(-1);
        string routingFile = default("");
        int numExtInterfaces = default(0);
        string dumpFile = default("");
        string mobilityType = default("StationaryMobility");
        int numRadios = default(1);  
    gates:
        input radioIn @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        notificationBoard: NotificationBoard {
            @display("p=497,33");
        }
       // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" && numRadios > 0 {
            parameters:
                @display("p=53,121");
        }

        lteRadio: LTERadio {
            channelNumber = 10;
            @display("p=103,140");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=497,101");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=193,360,row,90;q=txQueue");
        }
        ext[numExtInterfaces]: ExtInterface {
            parameters:
                @display("p=323,360,row,90;q=txQueue;i=block/ifcard");
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=68,360,row,90;q=txQueue");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=497,181");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=239,197;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                ifOut[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
        }
        pingApp: PingApp {
            parameters:
                @display("p=396,200");
        }
        sctp: SCTP {
            @display("p=396,111;i=block/wheelbarrow");
        }
        s1ap: S1AP {
            parameters:
                configFile = configFile;
                @display("p=338,41");
        }
        tcpdump: TCPDump {
            parameters:
                @display("p=239,269;i=abstract/cache_s");
                dumpFile = dumpFile;
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                hlIn[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                ifOut[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
                hlOut[sizeof(pppg)+sizeof(ethg)+numExtInterfaces];
        }
        nas: NAS {
            @display("p=169,41");
            configFile = configFile;
        }
        subscriberTable: SubscriberTable {
            @display("p=487,329");
        }
        udp: UDP {
            @display("p=179,140");
        }
        gtpu: GTPUser {
            configFile = configFile;
            @display("p=239,80");
        }
        tunnelEndpointTable: TunnelEndpointTable {
            @display("p=86,220");
        }
        pathTable: GTPPathTable {
            @display("p=74,283");
        }
    connections allowunconnected:
        radioIn --> lteRadio.radioIn;

        lteRadio.uppergateIn <-- nas.radioOut;
        lteRadio.uppergateOut --> nas.radioIn;
        sctp.to_appl++ --> s1ap.sctpIn;
        s1ap.sctpOut --> sctp.from_appl++;

        sctp.to_ip --> networkLayer.sctpIn;
        networkLayer.sctpOut --> sctp.from_ip;

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
        nas.s1apOut --> s1ap.nasIn;
        s1ap.nasOut --> nas.s1apIn;
        udp.ipOut --> networkLayer.udpIn;
        networkLayer.udpOut --> udp.ipIn;
        gtpu.udpOut --> udp.appIn++;
        udp.appOut++ --> gtpu.udpIn;
}
