//*********************************************************************************
// File:           WirelessMultihomeHipHost6.ned
//
// Authors:        Laszlo Tamas Zeke, Laszlo Bokor
//
// Copyright: (C) 2008-2009 BME-HT (Department of Telecommunications,
// Budapest University of Technology and Economics), Budapest, Hungary
//
// Copyright (C) 2004 Andras Varga
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
//**********************************************************************************
// Part of: HIPSim++ Host Identity Protocol Simulation Framework developed by BME-HT
//**********************************************************************************


package inet.nodes.hip;

import inet.applications.pingapp.PingApp;
import inet.applications.tcpapp.TCPApp;
import inet.applications.udpapp.UDPApp;
import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv6.RoutingTable6;
import inet.nodes.ipv6.NetworkLayer6;
import inet.transport.tcp.TCP;
import inet.transport.udp.UDP;
import inet.mobility.BasicMobility;
//"EthernetInterface",
import inet.linklayer.ieee80211.Ieee80211NicSTA;
import inet.hip.HIP;



//
// HIP host with TCP, UDP layers and applications and a wireless interface.
//
module WirelessMultihomeHipHost6
{
    parameters:
        double numTcpApps;
        double numUdpApps;
        string tcpAppType;
        string udpAppType;
        string mobilityType;
        int numInterfaces;
    gates:
        input radioIn[numInterfaces];
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70;i=block/control");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,150;i=block/table");
        }
        routingTable6: RoutingTable6 {
            parameters:
                isRouter = false;
                @display("p=60,230;i=block/table");
        }
        tcpApp[numTcpApps]: <tcpAppType> like TCPApp {
            parameters:
                @display("p=163,67;i=block/app");
        }
        tcp: TCP {
            parameters:
                @display("p=163,154;i=block/wheelbarrow");
        }
        udpApp[numUdpApps]: <udpAppType> like UDPApp {
            parameters:
                @display("i=block/app;p=272,67");
        }
        udp: UDP {
            parameters:
                @display("p=272,154;i=block/transport");
        }
        hip: HIP {
            parameters:
                @display("i=block/star;p=190,208");
        }
        pingApp: PingApp {
            parameters:
                @display("i=block/app;p=343,200");
        }
        networkLayer: NetworkLayer6 {
            parameters:
                @display("p=248,247;i=block/fork;q=queue");
            gates:
                ifIn[numInterfaces];
                ifOut[numInterfaces];
        }
        wlan[numInterfaces]: Ieee80211NicSTA {
            parameters:
                @display("p=158,358,row,70;q=l2queue;i=block/ifcard");
        }
        mobility: <mobilityType> like BasicMobility {
            parameters:
                @display("p=66,357;i=block/cogwheel_s");
        }
    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipv6Out --> hip.tcpIn;
        hip.tcp6Out --> networkLayer.tcpIn;
        tcp.ipv6In <-- hip.tcpOut;
        hip.tcp6In <-- networkLayer.TCPOut;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipv6Out --> hip.udpIn;
        hip.udp6Out --> networkLayer.udpIn;
        udp.ipv6In <-- hip.udpOut;
        hip.udp6In <-- networkLayer.udpOut;
        //hip.fromNetworkAddrChange <-- networkLayer.ipchangeOut;

        networkLayer.pingOut --> pingApp.pingv6In;
        networkLayer.pingIn <-- pingApp.pingv6Out;

        for i=0..numInterfaces-1 {
            wlan[i].radioIn <-- radioIn++;
            wlan[i].uppergateOut --> networkLayer.ifIn++;
            wlan[i].uppergateIn <-- networkLayer.ifOut++;
        }

}

