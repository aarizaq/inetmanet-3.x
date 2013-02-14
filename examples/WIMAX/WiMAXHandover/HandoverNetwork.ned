//
// Copyright (C)  2007 Roland Siedlaczek
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.examples.WIMAX.WiMAXHandover;

import inet.examples.WIMAX.WiMAXHandover.Bild;
import inet.underTest.wimax.nodes.WiMAXMS;
import inet.underTest.wimax.nodes.WiMAXAP;
import inet.world.radio.ChannelControl;
import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;

network HandoverNetwork
{
    submodules:
        bild: Bild {
            parameters:
                @display("p=1468,760;i=auto_b");
        }
        ms1: WiMAXMS {
            parameters:
                @display("p=0,750;i=device/cellphone_s;r=,,#707070");
        }
        //ms2: WiMAXMS;
        //    display: "p=2728,164;i=device/cellphone_s;r=,,#707070";
        //ms3: WiMAXMS;
        //    display: "p=2500,136;i=device/cellphone_s;r=,,#707070";
        //ms4: WiMAXMS;
        //    display: "p=2360,140;i=device/cellphone_s;r=,,#707070";
        bs1: WiMAXAP {
            parameters:
                @display("p=1000,750;i=device/antennatower_l;r=,,#707070");
        }
        bs2: WiMAXAP {
            parameters:
                @display("p=2000,750;i=device/antennatower_l;r=,,#707070");
        }
        //bs3: WiMAXAP;
        //    display: "p=1534,154;i=device/antennatower_l;r=,,#707070";
        //bs4: WiMAXAP;
        //    display: "p=1168,412;i=device/antennatower_l;r=,,#707070";
        //bs5: WiMAXAP;
        //    display: "p=674,302;i=device/antennatower_l;r=,,#707070";
        //bs6: WiMAXAP;
        //    display: "p=782,670;i=device/antennatower_l;r=,,#707070";
        //bs7: WiMAXAP;
        //    display: "p=526,994;i=device/antennatower_l;r=,,#707070";
        //bs8: WiMAXAP;
        //    display: "p=1144,884;i=device/antennatower_l;r=,,#707070";
        //bs9: WiMAXAP;
        //    display: "p=1544,972;i=device/antennatower_l;r=,,#707070";
        channelControl: ChannelControl {
            parameters:
                @display("p=60,50;i=misc/sun");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
 //               moduleTypes = "WiMAXMS WiMAXAP";
 //               nonIPModuleTypes = "";
                networkAddress = "145.236.0.0";
                netmask = "255.255.0.0";
                @display("p=140,50;i=block/cogwheel_s");
        }
    connections allowunconnected:
}
