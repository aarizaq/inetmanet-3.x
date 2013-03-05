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

import inet.base.MMapBoard;
import inet.base.NotificationBoard;
import inet.linklayer.ieee80211mesh.Ieee80211NicMesh;
import inet.linklayer.ieee80211mesh.Ieee80211NicMeshMulti;
import inet.networklayer.common.InterfaceTable;
import inet.base.Sink;

import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.IMobility;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.linklayer.IExternalNic;
import inet.base.NotificationBoard;


module BasicNetLayer
{
    gates:
        input ifIn[1];
        output ifOut[1];
    submodules:
        sink: Sink {
            @display("p=100,193;i=block/sink");
            gates:
                in[1];
        }
    connections allowunconnected:
        ifIn[0] --> sink.in[0];
}

//
// Models a mobile host with a wireless (802.11b) card. This module is
// a WirelessHost less the \PPP and Ethernet interfaces, plus mobility
// support (BasicMobility). It is intended mainly for mobile ad-hoc
// networks.
//
module BasicMobileManetMesh
{
    parameters:
        @node();
        @display("i=device/pocketpc_s");
        string mobilityType = default("StationaryMobility");
        int meshChannels = default(1);
    gates:
        input radioIn[meshChannels] @directIn;

    submodules:
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "";
                routingFile = "";
                @display("p=38,146");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=159,146");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=159,62");
        }
        sink: BasicNetLayer {
            @display("p=159,294;i=block/sink");
        }
        // the name of radio interface must have "wlan", valid examples wlan, wlan802, localwlan ....
        wlan: Ieee80211NicMeshMulti {
            parameters:
                meshChannels = meshChannels;
                @display("p=159,386;q=queue");
        }
        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" && meshChannels > 0 {
            parameters:
                @display("p=30,62");
        }


        mmapBoard: MMapBoard {
            parameters:
                @display("p=38,294;i=block/blackboard");
        }
    connections allowunconnected:

        // connections to network outside
        for i=0..sizeof(meshChannels)-1 {
            radioIn[i] --> wlan.radioIn[i];

        }
        wlan.upperLayerOut --> sink.ifIn++;
        wlan.upperLayerIn <-- sink.ifOut++;
}


