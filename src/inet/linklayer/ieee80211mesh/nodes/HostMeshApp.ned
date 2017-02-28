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
import inet.node.inet.WirelessHost;

import inet.common.packet.PcapRecorder;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.contract.IMobility;
import inet.linklayer.ieee80211mesh.ethernet.EthernetInterfaceMesh;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.contract.IExternalNic;
import inet.networklayer.contract.INetworkLayer;
import inet.linklayer.ieee80211mesh.Ieee80211NicMesh;
import inet.linklayer.ieee80211mesh.Ieee80211NicMeshMulti;
import inet.networklayer.configurator.ipv4.HostAutoConfigurator2;
import inet.linklayer.ieee80211.Ieee80211Nic;
import inet.linklayer.loopback.LoopbackInterface;

import inet.transportlayer.contract.IUDP;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.ISCTP;
import inet.applications.contract.IPingApp;
import inet.applications.contract.ISCTPApp;
import inet.applications.contract.ITCPApp;
import inet.applications.contract.IUDPApp;
import inet.node.inet.INetworkNode;

//
// BaseHost contains the common lower layers (linklayer and networklayer)

// of Router, StandardHost, WirelessHost etc. 

// 

// NOTE: - IPForward is turned on by default.

//       - use this model only as a base type. In simulations use Router, StandardHost,

@display("bgb=611,448;i=device/laptop");
// NOTE: - IPForward is turned on by default.
//       - use this model only as a base type. In simulations use Router, StandardHost,
        @display("bgb=611,448;i=device/laptop");
//
module HostMeshApp
{
    parameters:
        @display("bgb=611,448");
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
        int numTcpApps = default(0);  // no of TCP apps. Specify the app types in INI file with tcpApp[0..1].typename="TCPEchoApp" syntax
        int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax
        int numSctpApps = default(0); // no of SCTP apps. Specify the app types in INI file with sctpApp[0..1].typename="SCTPServer" syntax
        int numPingApps = default(0);  // no of PING apps. Specify the app types in INI file with pingApp[0..1].typename="PingApp" syntax        
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailable("UDP","UDP_None"));


        bool hasTcp = default(numTcpApps>0);
        bool hasUdp = default(numUdpApps>0);
        bool hasSctp = default(numSctpApps>0);
        
        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.
        string mobilityType = default("StationaryMobility");
        string routingFile = default("");
        bool forwarding = default(true);
        bool multicastForwarding = default(false);
        wlan[*].mgmtType = default("Ieee80211MgmtAP");
        networkLayer.configurator.networkConfiguratorModule = default("");

        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");
        string energyStorageType = default("");
        routingTable.forwarding = forwarding;
        routingTable.multicastForwarding = multicastForwarding;   // for IPv4, IPv6, Generic
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");
        *.energySourceModule = default(energyStorageType != "" ? absPath(".energyStorage") : "");
        *.mobilityModule = default(mobilityType != "" ? absPath(".mobility") : "");            
    gates:
        input radioIn[numRadios+1] @directIn;
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

       
        autoconfigurator: HostAutoConfigurator2 {
            @display("p=53,71");
        }
        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=53,121");
        }

        // network layer
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=329,287;q=queue");
        }

		// routing table
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=53,225;is=s");
        }


        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,386");
        }

        lo0: LoopbackInterface {
            @display("p=78,406");
        }
        
        wlanMesh: Ieee80211Nic {
            parameters:
                mgmtType = "Ieee80211MgmtAdhocWithEtx";
                @display("p=159,386;q=queue");
        }

        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=317,392;q=queue");
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

                @display("p=547,386,row,90;q=txQueue;i=block/ifcard");
        }

    connections allowunconnected:
        
        networkLayer.ifOut++ --> lo0.upperLayerIn;
        lo0.upperLayerOut --> networkLayer.ifIn++;
        
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.transportIn++ if hasTcp;
        tcp.ipIn <-- networkLayer.transportOut++ if hasTcp;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.transportIn++ if hasUdp;
        udp.ipIn <-- networkLayer.transportOut++ if hasUdp;

        for i=0..numSctpApps-1 {
            sctpApp[i].sctpOut --> sctp.from_appl++;
            sctp.to_appl++ --> sctpApp[i].sctpIn;
        }
        sctp.to_ip --> networkLayer.transportIn++ if hasSctp;
        networkLayer.transportOut++ --> sctp.from_ip if hasSctp;

        for i=0..numPingApps-1 {
            networkLayer.pingOut++ --> pingApp[i].pingIn;
            networkLayer.pingIn++ <-- pingApp[i].pingOut;
        }

        // connections to network outside

        radioIn[0] --> wlanMesh.radioIn;

        wlanMesh.upperLayerOut --> networkLayer.ifIn++;
        wlanMesh.upperLayerIn <-- networkLayer.ifOut++;

        for i=0..numRadios-1 {
            radioIn[i+meshChannels] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> networkLayer.ifIn++;
            wlan[i].upperLayerIn <-- networkLayer.ifOut++;
        }


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

