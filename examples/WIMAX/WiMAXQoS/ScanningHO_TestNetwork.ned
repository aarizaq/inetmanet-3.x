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

module ScanningHONetwork
{
    submodules:
        bild: Bild {
            parameters:
                @display("p=1061,740;i=centro_ca2122x1480_");
        }
        ms01: WiMAXMS {
            parameters:
                @display("p=56,318;i=device/cellphone_s;r=,,#707070");
        }
        ms02: WiMAXMS {
            parameters:
                @display("p=196,200;i=device/cellphone_s;r=,,#707070");
        }
        ms03: WiMAXMS {
            parameters:
                @display("p=420,268;i=device/cellphone_s;r=,,#707070");
        }
        //ms04: WiMAXMS;
        //    display: "p=316,222;i=device/cellphone_s;r=,,#707070";
        //ms05: WiMAXMS;
        //    display: "p=346,222;i=device/cellphone_s;r=,,#707070";
        //ms06: WiMAXMS;
        //    display: "p=376,242;i=device/cellphone_s;r=,,#707070";
        bs1: WiMAXAP {
            parameters:
                @display("p=550,382;i=device/antennatower_l;r=,,#707070");
        }
        bs2: WiMAXAP {
            parameters:
                @display("p=1046,382;i=device/antennatower_l;r=,,#707070");
        }
        //bs3: WiMAXAP;
        //    display: "p=1558,482;i=device/antennatower_l;r=,,#707070";
        channelControl: ChannelControl {
            parameters:
                @display("p=60,50;i=misc/sun");
        }
    connections allowunconnected:
}

network scanning_ho_network extends ScanningHONetwork
{
    parameters:
}

