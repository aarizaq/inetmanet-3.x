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

package inet.examples.WIMAX.WiMAXQoS;

import inet.examples.WIMAX.WiMAXQoS.Bild;
import inet.underTest.wimax.nodes.WiMAXMS;
import inet.underTest.wimax.nodes.WiMAXAP;
import inet.world.radio.ChannelControl;

module QoSNetwork
{
    parameters:
        //numHosts: numeric const;

    submodules:
        bild: Bild {
            parameters:
                @display("p=1061,740;i=centro_ca2122x1480_");
        }
        //ms: WiMAXMS[numHosts];
        //	display: "i=device/cellphone_s;r=,,#707070";
        ms01: WiMAXMS {
            parameters:
                @display("p=252,148;i=device/cellphone_s;r=,,#707070");
        }
        //ms02: WiMAXMS;
        //    display: "p=132,296;i=device/cellphone_s;r=,,#707070";
        //ms03: WiMAXMS;
        //    display: "p=380,292;i=device/cellphone_s;r=,,#707070";
        //ms04: WiMAXMS;
        //    display: "p=356,86;i=device/cellphone_s;r=,,#707070";
        //ms05: WiMAXMS;
        //    display: "p=346,222;i=device/cellphone_s;r=,,#707070";
        //ms06: WiMAXMS;
        //    display: "p=376,242;i=device/cellphone_s;r=,,#707070";
        //ms07: WiMAXMS;
        //    display: "p=344,332;i=device/cellphone_s;r=,,#707070";
        //ms08: WiMAXMS;
        //    display: "p=240,150;i=device/cellphone_s;r=,,#707070";
        //ms09: WiMAXMS;
        //    display: "p=110,318;i=device/cellphone_s;r=,,#707070";
        //ms10: WiMAXMS;
        //    display: "p=364,162;i=device/cellphone_s;r=,,#707070";
        bs1: WiMAXAP {
            parameters:
                @display("p=254,278;i=device/antennatower_l;r=,,#707070");
        }
        //bs2: WiMAXAP;
        //    display: "p=846,382;i=device/antennatower_l;r=,,#707070";
        //bs3: WiMAXAP;
        //    display: "p=1558,482;i=device/antennatower_l;r=,,#707070";
        channelControl: ChannelControl {
            parameters:
                @display("p=60,50;i=misc/sun");
        }
    connections allowunconnected:
}

network qos_network extends QoSNetwork
{
    parameters:
}

