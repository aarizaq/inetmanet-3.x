//
// Copyright (C) 2008 Alfonso Ariza
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


package inet.linklayer.ieee80211mesh.nodes;
import inet.nodes.inet.WirelessHost;

import inet.util.PcapRecorder;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.IMobility;
import inet.linklayer.ieee80211mesh.ethernet.EthernetInterfaceMesh;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.linklayer.IExternalNic;
import inet.base.NotificationBoard;
import inet.nodes.inet.NetworkLayer;
import inet.linklayer.ieee80211mesh.Ieee80211NicMesh;
import inet.linklayer.ieee80211mesh.Ieee80211NicMeshMulti;
import inet.networklayer.autorouting.ipv4.HostAutoConfigurator2;

import inet.transport.IUDP;
import inet.transport.ITCP;
import inet.transport.ISCTP;
import inet.applications.IPingApp;
import inet.applications.IUDPApp;
import inet.applications.ITCPApp;
import inet.applications.ISCTPApp;

//
// BaseHost contains the common lower layers (linklayer and networklayer)
// of Router, StandardHost, WirelessHost etc. 
// 
// NOTE: - IPForward is turned on by default.
//       - use this model only as a base type. In simulations use Router, StandardHost,
//         WirelessHost and AndhocHost
//
module gateWayMesh
{
    parameters:
        @display("bgb=611,448");
        @node;
        @labels(node,ethernet-node,wireless-node);
        int numTcpApps = default(0);  // no of TCP apps. Specify the app types in INI file with tcpApp[0..1].typename="TCPEchoApp" syntax
        int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax
        int numSctpApps = default(0); // no of SCTP apps. Specify the app types in INI file with sctpApp[0..1].typename="SCTPServer" syntax
        int numPingApps = default(0);  // no of PING apps. Specify the app types in INI file with pingApp[0..1].typename="PingApp" syntax        
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailable("UDP","UDP_None"));
        string sctpType = default(firstAvailable("SCTP","SCTP_None"));

        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.
        string mobilityType = default("StationaryMobility");
        string routingFile = default("");
        bool IPForward = default(true);
        int meshChannels = default(1);
        wlan[*].mgmtType = default("Ieee80211MgmtAP");
    gates:
        input radioIn[numRadios+meshChannels] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:

        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=186,54,row");
        }
        tcp: <tcpType> like ITCP if numTcpApps>0 {
            parameters:
                @display("p=186,141");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("p=336,54,row");
        }
        udp: <udpType> like IUDP if numUdpApps>0 {
            parameters:
                @display("p=336,141");
        }
        sctpApp[numSctpApps]: <> like ISCTPApp {
            parameters:
                @display("p=501,54,row");
        }
        sctp: <sctpType> like ISCTP if numSctpApps>0 {
            @display("p=501,141");
        }
        pingApp[numPingApps]: <default("PingApp")> like IPingApp {
            parameters:
                @display("p=501,230");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=53,194");
        }

        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if numRadios>0 {
            parameters:
                @display("p=53,121");
        }

        // network layer
        networkLayer: NetworkLayer {
            parameters:
                @display("p=356,265;q=queue");
        }

        routingTable: RoutingTable {
            parameters:
                @display("p=53,287");
                IPForward = IPForward;
                routingFile = routingFile;
        }

        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,386");
        }
        autoconfigurator: HostAutoConfigurator2 {
            @display("p=53,71");
        }

        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=183,233");
        }

        wlanMesh: Ieee80211NicMeshMulti {
            parameters:
                meshChannels = meshChannels;
                @display("p=159,386;q=queue");
        }

        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=317,392;q=queue");
        }
        ethMesh: EthernetInterfaceMesh {
            parameters:
                @display("p=225,386,row,90;q=txQueue");
        }
        eth[sizeof(ethg)-1]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=384,392,row,90;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=456,392,row,90;q=txQueue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=547,386,row,90;q=txQueue;i=block/ifcard");
        }

    connections allowunconnected:

        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.tcpIn if numTcpApps>0;
        tcp.ipIn <-- networkLayer.tcpOut if numTcpApps>0;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.udpIn if numUdpApps>0;
        udp.ipIn <-- networkLayer.udpOut if numUdpApps>0;

        for i=0..numSctpApps-1 {
            sctpApp[i].sctpOut --> sctp.from_appl++;
            sctp.to_appl++ --> sctpApp[i].sctpIn;
        }
        sctp.to_ip --> networkLayer.sctpIn if numSctpApps>0;
        networkLayer.sctpOut --> sctp.from_ip if numSctpApps>0;

        for i=0..numPingApps-1 {
            networkLayer.pingOut++ --> pingApp[i].pingIn;
            networkLayer.pingIn++ <-- pingApp[i].pingOut;
        }

        // connections to network outside

        for i=0..meshChannels-1 {            
            radioIn[i] --> wlanMesh.radioIn[i];

        }
        wlanMesh.upperLayerOut --> networkLayer.ifIn++;
        wlanMesh.upperLayerIn <-- networkLayer.ifOut++;
        ethMesh.wifiMeshIn <-- wlanMesh.toEthernet;
        ethMesh.wifiMeshOut --> wlanMesh.fromEthernet;
        for i=0..numRadios-1 {
            radioIn[i+meshChannels] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> networkLayer.ifIn++;
            wlan[i].upperLayerIn <-- networkLayer.ifOut++;
        }


        ethg[0] <--> ethMesh.phys;
        ethMesh.upperLayerOut --> networkLayer.ifIn++;
        ethMesh.upperLayerIn <-- networkLayer.ifOut++;

        for i=0..sizeof(ethg)-2 {
            ethg[i+1] <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn++;
            eth[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn++;
            ppp[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> networkLayer.ifIn++;
            ext[i].upperLayerIn <-- networkLayer.ifOut++;
        }
}

