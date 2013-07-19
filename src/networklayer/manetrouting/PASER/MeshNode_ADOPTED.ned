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
import inet.linklayer.IWiredNic;
import inet.linklayer.IWirelessNic;
import inet.mobility.IMobility;
import inet.networklayer.IManetRouting;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.transport.ISCTP;
import inet.transport.ITCP;
import inet.transport.IUDP;


//
// Models a mobile host with a wireless (802.11g) card. This module is
// a WirelessHost less the \PPP and Ethernet interfaces, plus fixed mobility
// support (NullMobility). It is intended mainly for mobile ad-hoc
// networks where the host is stationary.
//
module paser_FixManetRoutingHostShare
{
    parameters:
        @node();
        @node;
        @labels(node,ethernet-node,wireless-node);
        int numRadios = default(1);
        int numGRadios = default(1);
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        int numSctpApps = default(0);
        int numPingApps = default(0);
        string tcpAppType = default("");
        string udpAppType = default("");
        string routingFile = default("");
        bool isGW = default(false);
        bool isBlackHole = default(false);
        //double startTime = exponential(1);
        @display("i=device/pocketpc_s");
        string mobilityType = default("StationaryMobility");
        string routingProtocol @enum("PASER","AODVUU","DYMOUM","DYMO","DSRUU","OLSR","OLSR_ETX","DSDV_2","Batman") = default("");  // used mobile routing protocol. see: inet.networklayer.manetrouting
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailable("UDP","UDP_None"));
        string sctpType = default(firstAvailable("SCTP","SCTP_None"));

        bool IPForward = default(true);

        wlanG[*].mgmtType = default("Ieee80211MgmtAdhoc");
    gates:
        //input radioIn @directIn;
        input radioIn[numRadios+numGRadios] @directIn;
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70;i=block/control");
        }
        //        mmapBoard: MMapBoard {
        //            parameters:
        //                @display("p=60,307;i=block/blackboard");
        //        }
        //        mobility: NullMobility {
        //            parameters:
        //                //x = x_position;
        //                //y = y_position;
        //                @display("p=149,307;i=block/cogwheel");
        //        }
        //        mobility: <mobilityType> like BasicMobility {
        //            parameters:
        //                @display("p=149,307");
        //        }
        manetrouting: <routingProtocol> like IManetRouting if routingProtocol != "" {
            @display("p=153,247;i=block/network2");
        }

        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" && numGRadios > 0 {
            parameters:
                @display("p=149,307");
        }
        //        networkLayer: NetworkLayer_Paser {
        //            parameters:
        //                //                proxyARP = true;
        //                //                globalARP = false;
        //                @display("p=248,247;i=block/fork;q=queue");
        //            gates:
        //                //                ifIn[1];
        //                //                ifOut[1];
        //                ifIn[(sizeof(pppg)+sizeof(ethg))+numRadios+numGRadios];
        //                ifOut[(sizeof(pppg)+sizeof(ethg))+numRadios+numGRadios];
        //        }
        networkLayer: NetworkLayer_Paser {
            parameters:
                @display("p=248,247;i=block/fork;q=queue");
        }
        routingTable: RoutingTable {
            parameters:
                //IPForward = IPForward,
                IPForward = true;
                routerId = "";
                routingFile = routingFile;
                @display("p=60,230;i=block/table");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,150;i=block/table");
        }
        // the name of radio interface must have "wlan", valid examples wlan, wlan802, localwlan ....
        //        wlan: Ieee80211gNicAdhoc { // Nic80211 also works, it uses MF's 802.11 implementation
        //            parameters:
        //                @display("p=248,349;q=queue;i=block/ifcard");
        //        }
        //        wlanG[numGRadios]: Ieee80211aNicAdhoc {
        //            parameters:
        //                @display("p=202,357,row,90;q=queue");
        //        }
        wlanG[numGRadios]: <default("Ieee80211NewNic")> like IWirelessNic {
            parameters:
                @display("p=202,357,row,90;q=queue");
        }
        //        wlan[numRadios]: Ieee80211NicAP {
        //            parameters:
        //                @display("p=99,357,row,90;q=queue");
        //        }
        wlan[numRadios]: <default("Ieee80211NewNic")> like IWirelessNic {
            parameters:
                @display("p=99,357,row,90;q=queue");
        }
        //        wlan: Nic80211;
        //            display: "p=248,349;q=queue;i=block/ifcard";
        //        eth[sizeof(ethg)]: EthernetInterface {
        //            parameters:
        //                @display("p=298,357,row,90;q=txQueue");
        //        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=298,357,row,90;q=txQueue");
        }
        //        tcpApp[numTcpApps]: <tcpAppType> like ITCPApp {
        //            parameters:
        //                @display("p=163,67;i=block/app");
        //        }
        //        tcp: TCP {
        //            parameters:
        //                @display("p=163,154;i=block/wheelbarrow");
        //        }
        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=163,67;i=block/app");
        }
        tcp: <tcpType> like ITCP if numTcpApps>0 {
            parameters:
                @display("p=163,154;i=block/wheelbarrow");
        }
        //        udpApp[numUdpApps]: <udpAppType> like IUDPApp {
        //            parameters:
        //                @display("i=block/app;p=272,67");
        //        }
        //        udp: UDP {
        //            parameters:
        //                @display("p=272,154;i=block/transport");
        //        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("i=block/app;p=272,67");
        }
        udp: <udpType> like IUDP if numUdpApps>0 {
            parameters:
                @display("p=272,154;i=block/transport");
        }
        //        pingApp: PingApp {
        //            parameters:
        //                @display("i=block/app;p=343,200");
        //        }
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

//        manetrouting: ManetRouting {
//            parameters:
//                @display("p=153,247;i=block/network2");
//        }



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

        //        networkLayer.pingOut --> pingApp.pingIn;
        //        networkLayer.pingIn <-- pingApp.pingOut;
        for i=0..numPingApps-1 {
            networkLayer.pingOut++ --> pingApp[i].pingIn;
            networkLayer.pingIn++ <-- pingApp[i].pingOut;
        }

        //        networkLayer.MANETOut --> manetrouting.from_ip;
        //        networkLayer.MANETIn <-- manetrouting.to_ip;
        networkLayer.manetOut --> manetrouting.from_ip if routingProtocol != "";
        networkLayer.manetIn <-- manetrouting.to_ip if routingProtocol != "";

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
        for i=0..numGRadios-1 {
            radioIn[i+numRadios] --> wlanG[i].radioIn;
            wlanG[i].upperLayerOut --> networkLayer.ifIn++;
            wlanG[i].upperLayerIn <-- networkLayer.ifOut++;
        }
}

