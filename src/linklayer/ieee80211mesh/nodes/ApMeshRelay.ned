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

import inet.linklayer.IMACRelayUnit;
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

module simpleNetLayer
{
    parameters:
        string relayType;
    gates:
        input ifIn[];
        output ifOut[];
    submodules:
        relayUnit: <relayType> like IMACRelayUnit {
            parameters:
                @display("p=232,283");
            gates:
                lowerLayerIn[sizeof(ifIn)];
                lowerLayerOut[sizeof(ifOut)];
        }

    connections allowunconnected:
        for i=0..sizeof(ifIn)-1 {
            ifIn[i] --> relayUnit.lowerLayerIn[i];
        }
        for i=0..sizeof(ifOut)-1 {
            relayUnit.lowerLayerOut[i] --> ifOut[i];
        }
}

module ApMeshRelay
{
    parameters:
        @display("bgb=611,448");
        @node;
        @labels(node,ethernet-node,wireless-node);
        int numExtInterfaces = default(0);
        int numRadios = default(1);               // the number of radios in the router. by default no wireless     
        string mobilityType = default("StationaryMobility");
        string relayUnitType = default("MACRelayUnitNP");
        wlan[*].mgmtType = default("Ieee80211MgmtAP");
    gates:
        input radioIn[numRadios+1] @directIn;
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=53,259");
        }
        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,386");
        }

        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if numRadios>0 {
            parameters:
                @display("p=61,192");
        }

        relayUnit: simpleNetLayer {
            parameters:
                relayType = relayUnitType;
                @display("p=223,298");
            gates:
                ifIn[numRadios+1];
                ifOut[numRadios+1];
        }

        wlanMesh: Ieee80211NicMeshMulti {
            parameters:
                mgmt.forceRelayUnit = true;
                @display("p=159,386;q=queue");
        }

        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                mgmt.forceRelayUnit = true;
                @display("p=271,392;q=queue");
        }

    connections allowunconnected:
        // connections to network outside

        if (sizeof(radioIn)>0) {
            radioIn[0] --> wlanMesh.radioIn[0];
            wlanMesh.upperLayerOut --> relayUnit.ifIn++;
            wlanMesh.upperLayerIn <-- relayUnit.ifOut++;
        }

        for i=0..sizeof(radioIn)-2 {
            radioIn[i+1] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> relayUnit.ifIn++;
            wlan[i].upperLayerIn <-- relayUnit.ifOut++;
        }
}

