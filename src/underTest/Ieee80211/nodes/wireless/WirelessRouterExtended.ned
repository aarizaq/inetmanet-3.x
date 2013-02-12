//
// Copyright (C) 2008 INRIA Sophia-Antipolis
// Juan-Carlos Maureira
// Project MASCOTTE
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
package inet.underTest.Ieee80211.nodes.wireless;

import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.applications.IPingApp;
import inet.applications.ITCPApp;
import inet.applications.IUDPApp;
import inet.linklayer.ieee80211.Ieee80211Nic;
import inet.transport.udp.UDP;
import inet.transport.tcp.TCP;
import inet.mobility.models.StationaryMobility;
import inet.linklayer.ieee80211.Ieee80211Nic;
import inet.underTest.Ieee80211.linklayer.ieee80211.mgmt.Ieee80211NicExtended;
//
// \Wireless IP router.
//
module WirelessRouterExtended
{
    parameters:
        @node();
        string routingFile = default("");
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        int numPingApps = default(0);  // no of PING apps. Specify the app types in INI file with pingApp[0..1].typename="PingApp" syntax        
        string tcpAppType = default("n/a");
        string udpAppType = default("n/a");
        string mgmtType = default("IEEE80211MgmtAPExtended");
        bool IPForward = default(true);
        int namid = default(-1);
        @display("i=device/accesspoint;bgl=2");
    gates:
        inout pppg[];
        inout ethg[];
        input radioIn @directIn;
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,60");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=178,60");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "auto";
                routingFile = routingFile;
                @display("p=285,60");
        }
        networkLayer: NetworkLayer {
            parameters:
                @display("p=210,156;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=217,309,row,110;q=l2queue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=344,309,row,110;q=l2queue");
        }
        udp: UDP {
            @display("p=330,127");
        }
        udpApp[numUdpApps]: <udpAppType> like IUDPApp {
            parameters:
                @display("p=453,127");
        }
        tcp: TCP {
            @display("p=330,194");
        }
        tcpApp[numTcpApps]: <tcpAppType> like ITCPApp {
            parameters:
                @display("p=453,194");
        }
        pingApp[numPingApps]: <default("PingApp")> like IPingApp {
            parameters:
                @display("p=369,254");
        }
        nullMobility: StationaryMobility {
            @display("p=393,60");
        }
        wlan: Ieee80211NicExtended {
            parameters:
                @display("p=111,309;i=block/wrxtx");
                mgmtType = mgmtType;
        }
    connections allowunconnected:
        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn[i];
            ppp[i].upperLayerIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].upperLayerIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.udpIn;
        networkLayer.udpOut --> udp.ipIn;
        tcp.ipOut --> networkLayer.tcpIn;
        networkLayer.tcpOut --> tcp.ipIn;
        for i=0..numPingApps-1 {
            networkLayer.pingOut++ --> pingApp[i].pingIn;
            networkLayer.pingIn++ <-- pingApp[i].pingOut;
        }
        networkLayer.ifOut++ --> wlan.upperLayerIn;
        wlan.upperLayerOut --> networkLayer.ifIn++;
        radioIn --> wlan.radioIn;
}

