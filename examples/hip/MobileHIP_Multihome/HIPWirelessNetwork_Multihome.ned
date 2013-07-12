//*********************************************************************************
// File:           HIPWirelessNetwork_Multihome.ned
//
// Authors:        Laszlo Tamas Zeke, Laszlo Bokor
//
// Copyright: (C) 2008-2009 BME-HT (Department of Telecommunications,
// Budapest University of Technology and Economics), Budapest, Hungary
//
// Copyright (C) 2007 Faqir Zarrar Yousaf
// Communication Networks Institute
// University of Dortmund, Germany
// faqir.yousaf@uni-dortmund.de
//
// Copyright (C) 2004 Andras Varga
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
//**********************************************************************************
// Part of: HIPSim++ Host Identity Protocol Simulation Framework developed by BME-HT
//**********************************************************************************


package inet.examples.hip.MobileHIP_Multihome;

import inet.nodes.ethernet.EtherSwitch;
import inet.networklayer.autorouting.ipv6.FlatNetworkConfigurator6;
import inet.nodes.wireless.AccessPoint;
import inet.nodes.ipv6.Router6;
import inet.underTest.hip.nodes.HipHost6B;
import inet.underTest.hip.nodes.WirelessHipHost6B;
import inet.underTest.hip.nodes.RvsHost6;
import inet.nodes.ipv6.StandardHost6;
import inet.world.radio.ChannelControl;




channel fiberline extends ned.DatarateChannel
{
    parameters:
        delay = 1us;
        datarate = 512*1000000bps;
}

channel ethernetline extends ned.DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 10Mbps;
}

network HIPWirelessNetwork_Multihome
{
    submodules:
        configurator: FlatNetworkConfigurator6 {
            parameters:
                @display("p=763,53;i=block/cogwheel");
        }
        channelControl: ChannelControl {
            parameters:
                numChannels = 5;
                @display("p=753,123;i=misc/sun");
        }
        R_3: Router6 {
            parameters:
                @display("p=217,227;i=abstract/router");
        }
        R_1: Router6 {
            parameters:
                @display("p=605,227;i=abstract/router");
        }
        R_2: Router6 {
            parameters:
                @display("p=406,355;i=abstract/router");
        }
        R_4: Router6 {
            parameters:
                @display("p=406,227;i=abstract/router");
        }
        mobilehiphost: WirelessHipHost6B {
            parameters:
                @display("p=249,112;i=device/wifilaptop");
        }
        hipsrv: HipHost6B {
            parameters:
                @display("i=device/pc2;p=566,447");
        }
        dnssrv: StandardHost6 {
            parameters:
                @display("i=device/server_l;p=406,494");
        }
        rvs: RvsHost6 {
            parameters:
                @display("i=device/server_l;p=249,455");
        }
        AP_Home: AccessPoint {
            parameters:
                @display("p=217,172;i=device/accesspoint_s");
        }
        AP_1: AccessPoint {
            parameters:
                @display("p=605,172;i=device/accesspoint_s");
        }
        AP_2: AccessPoint {
            parameters:
                @display("p=406,172;i=device/accesspoint_s");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=406,430;i=device/switch_s");
        }
    connections allowunconnected:

                         //Home_Agent.ethOut++ --> ethernetline --> R_1.ethIn++;
                         //Home_Agent.ethIn++ <-- ethernetline <-- R_1.ethOut++;

        R_1.ethg++ <--> ethernetline <--> R_2.ethg++;

                                                       //R_2.ethOut++ --> ethernetline --> R_3.ethIn++;
                                                       //R_2.ethIn++ <-- ethernetline <-- R_3.ethOut++;

        R_3.ethg++ <--> ethernetline <--> R_2.ethg++;

        hipsrv.ethg++ <--> ethernetline <--> switch.ethg++;

        dnssrv.ethg++ <--> ethernetline <--> switch.ethg++;

        rvs.ethg++ <--> ethernetline <--> switch.ethg++;

        switch.ethg++ <--> ethernetline <--> R_2.ethg++;

                                                    //R_1.ethOut++ --> ethernetline --> CN.ethIn++;
                                                    //R_1.ethIn++ <-- ethernetline <-- CN.ethOut++;

        AP_Home.ethg++ <--> ethernetline <--> R_3.ethg++;

        AP_1.ethg++ <--> ethernetline <--> R_1.ethg++;

        AP_2.ethg++ <--> ethernetline <--> R_4.ethg++;

        R_4.ethg++ <--> ethernetline <--> R_2.ethg++;
                                                        //AP_2.ethIn++ <-- ethernetline <-- R_2.ethOut++;

                                                        //AP_3.ethOut++ --> ethernetline --> R_3.ethIn++;
                                                        //AP_3.ethIn++ <-- ethernetline <-- R_3.ethOut++;

}


