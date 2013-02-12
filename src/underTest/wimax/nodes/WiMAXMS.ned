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
import inet.underTest.wimax.linklayer.Ieee80216.Ieee80216NicMS;
import inet.underTest.wimax.applications.TrafficGenerator_Erlangen.WiMAXQoSTrafficGenerator;
import inet.mobility.IMobility;


                                //
                                // Models a host with one wireless (802.11b) card in infrastructure mode.
                                // This module is basically a StandardHost with an Ieee80211NicSTA added.
                                // It should be used in conjunction with WirelessAP, or any other AP model
                                // which contains Ieee80211NicAP.
                                //

module WiMAXMS
{
    parameters:
        string mobilityType;
        @node();
    gates:
        input in[];
        output out[];
        input radioIn;
        output radioOut;

    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70;i=block/control");
        }
        wimax: Ieee80216NicMS {
            parameters:
                @display("p=288,278;q=queue;i=block/ifcard");
            gates:
                macUpperGateIn[5];
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=146,69;i=block/cogwheel_s");
        }
        trafGen_ftp: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=288,146");
        }
        trafGen_voice_no_supr: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=152,170");
        }
        trafGen_voice_supr: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=421,162");
        }
        trafGen_guaranteed_minbw_web_access: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=480,278");
        }
        trafGen_video_stream: WiMAXQoSTrafficGenerator {
            parameters:
                @display("i=device/wifilaptop;p=92,278");
        }
    connections allowunconnected:
        radioIn --> wimax.radioIn;

        trafGen_ftp.lowerLayerOut --> wimax.macUpperGateIn[0];
        trafGen_voice_no_supr.lowerLayerOut --> wimax.macUpperGateIn[1];
        trafGen_voice_supr.lowerLayerOut --> wimax.macUpperGateIn[2];
        trafGen_video_stream.lowerLayerOut --> wimax.macUpperGateIn[3];
        trafGen_guaranteed_minbw_web_access.lowerLayerOut --> wimax.macUpperGateIn[4];

}

