package inet.underTest.wimax.nodes;
//
// Copyright (C) 2006 Andras Varga
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


import inet.base.NotificationBoard;
//    "EthernetInterface",
import inet.underTest.wimax.linklayer.Ieee80216.Ieee80216NicBS;
//    "InterfaceTable",
//    "RoutingTable",
//    "NetworkLayer",
import inet.underTest.wimax.applications.TrafficGenerator_Erlangen.WiMAXQoSTrafficGenerator;
import inet.mobility.models.StationaryMobility;
//
// Models a 802.11 Access Point. It can be used in conjunction with
// WirelessHost, or any other host model containing WirelessNicSTA.
//
// @see WirelessAP, WirelessAPSimplified, WirelessAPWithEth, WirelessAPWithEthSimplified
// @see WirelessHost, WirelessHostSimplified
// @see MobileHost, MFMobileHost
//
module WiMAXAP
{
    parameters:
        bool IPForward;
        string routingFile;
        @node();
    gates:
        input radioIn;

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70;i=block/control");
        }
        wimax: Ieee80216NicBS { // see also Ieee80211NicAPSimplified
            parameters:
                @display("p=272,294;q=queue;i=block/ifcard");
            gates:
                macUpperGateIn[5];
        }
        mobility: StationaryMobility {
            parameters:
                @display("p=144,70;i=block/cogwheel_s");
        }
        trafGen_ftp: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=272,154");
        }
        trafGen_voice_no_supr: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=144,178");
        }
        trafGen_voice_supr: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=413,170");
        }
        trafGen_guaranteed_minbw_web_access: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=472,294");
        }
        trafGen_video_stream: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=84,294");
        }
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> wimax.radioIn;

        trafGen_ftp.lowerLayerOut --> wimax.macUpperGateIn[0];
        trafGen_voice_no_supr.lowerLayerOut --> wimax.macUpperGateIn[1];
        trafGen_voice_supr.lowerLayerOut --> wimax.macUpperGateIn[2];
        trafGen_video_stream.lowerLayerOut --> wimax.macUpperGateIn[3];
        trafGen_guaranteed_minbw_web_access.lowerLayerOut --> wimax.macUpperGateIn[4];

}

