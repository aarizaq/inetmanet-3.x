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

//
// BaseHost contains the common lower layers (linklayer and networklayer)
// of Router, StandardHost, WirelessHost etc. 
// 
// NOTE: - IPForward is turned on by default.
//       - use this model only as a base type. In simulations use Router, StandardHost,
//         WirelessHost and AndhocHost
//
module ApMeshNode
{
    parameters:
        @display("bgb=611,448");
        @node;
        @labels(node,ethernet-node,wireless-node);
        int numExtInterfaces = default(0);
        int numRadios = default(1);               // the number of radios in the router. by default no wireless
        int meshChannels = default(1);
        int numPcapRecorders = default(0); // no of PcapRecorders.
        string mobilityType = default("StationaryMobility");
        string routingFile = default("");
        bool IPForward = default(true);
        wlan[*].mgmtType = default("Ieee80211MgmtAP");
    gates:
        input radioIn[numRadios+meshChannels] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
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
            @display("p=204,138");
        }

        wlanMesh: Ieee80211NicMeshMulti {
            parameters:
                meshChannels = meshChannels;
                @display("p=159,386;q=queue");
        }

        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=271,392;q=queue");
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
        // connections to network outside


        for i=0..meshChannels-1 {
            radioIn[i] --> wlanMesh.radioIn[i];
        }
        wlanMesh.upperLayerOut --> networkLayer.ifIn++;
        wlanMesh.upperLayerIn <-- networkLayer.ifOut++;

        for i=0..numRadios-1 {
            radioIn[i+meshChannels] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> networkLayer.ifIn++;
            wlan[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
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

