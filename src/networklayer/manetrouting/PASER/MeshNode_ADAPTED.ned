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

package inet.networklayer.manetrouting.PASER;

import inet.applications.IPingApp;
import inet.applications.ISCTPApp;
import inet.applications.ITCPApp;
import inet.applications.IUDPApp;
import inet.base.NotificationBoard;
import inet.battery.IBattery;
import inet.linklayer.IExternalNic;
import inet.linklayer.IWiredNic;
import inet.linklayer.IWirelessNic;
import inet.mobility.contract.IMobility;
import inet.networklayer.IManetRouting;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.status.NodeStatus;
import inet.transport.ISCTP;
import inet.transport.ITCP;
import inet.transport.IUDP;
import inet.util.PcapRecorder;
import inet.nodes.inet.NetworkLayer;


//
// Models a mobile host with a wireless (802.11g) card. This module is
// a WirelessHost less the \PPP and Ethernet interfaces, plus fixed mobility
// support (NullMobility). It is intended mainly for mobile ad-hoc
// networks where the host is stationary.
//
module meshnode
{
    parameters:
        @display("bgb=,448");
        @node;
        @labels(node,ethernet-node,wireless-node);
        bool hasStatus = default(false);
        int numExtInterfaces = default(0);
        int numRadios = default(1);               // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.
        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");
        string routingFile = default("");
        bool IPForward = default(true);
        bool forwardMulticast = default(false);
        string batteryType = default("");
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        int numSctpApps = default(0);
        int numPingApps = default(0);
        string tcpAppType = default("");
        string udpAppType = default("");
        bool isGW = default(false);
        bool isBlackHole = default(false);
        //double startTime = exponential(1);
        @display("i=device/pocketpc_s");
        string routingProtocol @enum("PASER","AODVUU","DYMOUM","DYMO","DSRUU","OLSR","OLSR_ETX","DSDV_2","Batman") = default("");  // used mobile routing protocol. see: inet.networklayer.manetrouting
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailable("UDP","UDP_None"));
        string sctpType = default(firstAvailable("SCTP","SCTP_None"));
        bool hasTcp = default(numTcpApps>0);
        bool hasUdp = default(numUdpApps>0);
        bool hasSctp = default(numSctpApps>0);
        networkLayer.proxyARP = default(false);
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");
    gates:
        //input radioIn @directIn;
        input radioIn[numRadios] @directIn;
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        status: NodeStatus if hasStatus {
            @display("p=50,50");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=53,155;is=s");
        }

        battery: <batteryType> like IBattery if batteryType != "" {
            parameters:
                @display("p=53,77;i=block/plug;is=s");
        }


        manetrouting: <routingProtocol> like IManetRouting if routingProtocol != "" {
            @display("p=153,247;i=block/network2");
        }

        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" && numRadios > 0 {
            parameters:
                @display("p=149,307");
        }

        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=163,67;i=block/app");
        }

        tcp: <tcpType> like ITCP if numTcpApps>0 {
            parameters:
                @display("p=163,154;i=block/wheelbarrow");
        }

        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("i=block/app;p=272,67");
        }

        udp: <udpType> like IUDP if numUdpApps>0 {
            parameters:
                @display("p=272,154;i=block/transport");
        }

        sctpApp[numSctpApps]: <> like ISCTPApp {
            parameters:
                @display("p=381,67,row");
        }

        sctp: <sctpType> like ISCTP if numSctpApps>0 {
            @display("p=381,154");
        }

        pingApp[numPingApps]: <default("PingApp")> like IPingApp {
            parameters:
                @display("i=block/app;p=381,247");
        }

        networkLayer: NetworkLayer {
            parameters:
                @display("p=248,247;i=block/fork;q=queue");
        }


        routingTable: RoutingTable {
            parameters:
                @display("p=53,225;is=s");
                IPForward = IPForward;
                forwardMulticast = forwardMulticast;
                routingFile = routingFile;
        }

        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,287;is=s");
        }

        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=53,338,r,10");
        }

        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=216,406,row,60;q=queue");
        }

        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=368,406,row,60;q=txQueue");
        }

        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=149,406,row,10;q=txQueue;i=block/ifcard");
        }


    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.transportIn++ if numTcpApps>0;
        tcp.ipIn <-- networkLayer.transportOut++ if numTcpApps>0;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.transportIn++ if numUdpApps>0;
        udp.ipIn <-- networkLayer.transportOut++ if numUdpApps>0;

        for i=0..numSctpApps-1 {
            sctpApp[i].sctpOut --> sctp.from_appl++;
            sctp.to_appl++ --> sctpApp[i].sctpIn;
        }
        sctp.to_ip --> networkLayer.transportIn++ if numSctpApps>0;
        networkLayer.transportOut++ --> sctp.from_ip if numSctpApps>0;

        //        networkLayer.pingOut --> pingApp.pingIn;
        //        networkLayer.pingIn <-- pingApp.pingOut;
        for i=0..numPingApps-1 {
            networkLayer.pingOut++ --> pingApp[i].pingIn;
            networkLayer.pingIn++ <-- pingApp[i].pingOut;
        }

        //        networkLayer.MANETOut --> manetrouting.from_ip;
        //        networkLayer.MANETIn <-- manetrouting.to_ip;
        networkLayer.transportOut++ --> manetrouting.from_ip if routingProtocol != "";
        networkLayer.transportIn++ <-- manetrouting.to_ip if routingProtocol != "";

        //        for i=0..sizeof(ethg)-1 {
        //            ethg[i] <--> eth[i].phys;
        //            eth[i].netwOut --> networkLayer.ifIn[sizeof(pppg)+i];
        //            eth[i].netwIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        //        }
        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn++;
            eth[i].upperLayerIn <-- networkLayer.ifOut++;
        }
        // connections to network outside
        //        radioIn --> wlan.radioIn;
        //        wlan.uppergateOut --> networkLayer.ifIn[0];
        //        wlan.uppergateIn <-- networkLayer.ifOut[0];
        //        for i=0..numRadios-1 {
        //            radioIn[i] --> wlan[i].radioIn;
        //            wlan[i].upperLayerOut --> networkLayer.ifIn[sizeof(pppg)+sizeof(ethg)+i];
        //            wlan[i].upperLayerIn <-- networkLayer.ifOut[sizeof(pppg)+sizeof(ethg)+i];
        //        }
        for i=0..numRadios-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> networkLayer.ifIn++;
            wlan[i].upperLayerIn <-- networkLayer.ifOut++;
        }
        //        for i=0..numGRadios-1 {
        //            radioIn[i+numRadios] --> wlanG[i].radioIn;
        //            wlanG[i].upperLayerOut --> networkLayer.ifIn[sizeof(pppg)+sizeof(ethg)+i+numRadios];
        //            wlanG[i].upperLayerIn <-- networkLayer.ifOut[sizeof(pppg)+sizeof(ethg)+i+numRadios];
        //        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> networkLayer.ifIn++;
            ext[i].upperLayerIn <-- networkLayer.ifOut++;
        }

}

