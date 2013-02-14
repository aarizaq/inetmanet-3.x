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


module HandoverNetworkFourBS
{
    parameters:
        @display("bgb=1085,582");
    submodules:
        ms2: WiMAXMS {
            parameters:
                @display("p=750,2000;i=device/cellphone_s;r=,,#707070");
        }
        bs1: WiMAXAP {
            parameters:
                @display("p=1000,2500;i=device/antennatower_l;r=,,#707070");
        }
        bs2: WiMAXAP {
            parameters:
                @display("p=2000,4000;i=device/antennatower_l;r=,,#707070");
        }
        bs3: WiMAXAP {
            parameters:
                @display("p=3000,2500;i=device/antennatower_l;r=,,#707070");
        }
        bs4: WiMAXAP {
            parameters:
                @display("p=2000,1000;i=device/antennatower_l;r=,,#707070");
        }
        channelControl: ChannelControl {
            parameters:
                @display("p=60,50;i=misc/sun");
        }
    connections allowunconnected:
}

//network HandoverNetworkFourBS extends HandoverNetworkFourBS
//{
//    parameters:
//}

