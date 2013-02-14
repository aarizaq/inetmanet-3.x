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

import inet.underTest.wimax.nodes.WiMAXAP;
import inet.underTest.wimax.nodes.WiMAXMS;
import inet.world.radio.ChannelControl;


network HandoverNetworkClusterB
{
    parameters:
        @display("bgb=1000,1000");
    submodules:
        channelControl: ChannelControl {
            parameters:
                @display("p=60,50;i=misc/sun");
        }
        ms2: WiMAXMS {
            parameters:
                @display("p=250,300;i=device/cellphone_s;r=,,#707070");
        }
        bs1: WiMAXAP {
            parameters:
                @display("p=300,300;i=device/antennatower_l;r=,,#707070");
        }
        bs2: WiMAXAP {
            parameters:
                @display("p=100,300;i=device/antennatower_l;r=,,#707070");
        }
        bs3: WiMAXAP {
            parameters:
                @display("p=200,473;i=device/antennatower_l;r=,,#707070");
        }
        bs4: WiMAXAP {
            parameters:
                @display("p=400,473;i=device/antennatower_l;r=,,#707070");
        }
        bs7: WiMAXAP {
            parameters:
                @display("p=500,300;i=device/antennatower_l;r=,,#707070");
        }
        bs6: WiMAXAP {
            parameters:
                @display("p=400,127;i=device/antennatower_l;r=,,#707070");
        }
        bs5: WiMAXAP {
            parameters:
                @display("p=200,127;i=device/antennatower_l;r=,,#707070");

        }
    connections allowunconnected:
}

//network HandoverNetworkClusterB extends HandoverNetworkClusterB
//{
//    parameters:
//}

