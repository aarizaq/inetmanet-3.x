//
// Copyright (C) 2005 Andras Varga
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

package inet.examples.wireless.hosttohosttest;

import inet.applications.ethernet.EtherAppCli;
import inet.common.queue.Sink;
import inet.linklayer.ieee80211.Ieee80211Nic;
import inet.mobility.single.CircleMobility;
import inet.mobility.static.StationaryMobility;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.common.misc.ThruputMeter;

network Network
{
    @display("bgb=363,274");
    //string mediumType = default("IdealRadioMedium");
    //string hostType = default("WirelessHost");
   
        submodules:
        node: WirelessHost {
            @display("p=234,69");
            wlan[*].mgmtType = "Ieee80211MgmtSTASimplified";
        }
        node2: WirelessHost {
            @display("p=149,102");
            wlan[*].mgmtType = "Ieee80211MgmtSTASimplified";
        }

        ap: AccessPoint {
            @display("p=210,152;r=180");
        }
        ipv4networkconfigurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=47,179");
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                     
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=148,247");
        }
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,50");
        }
}

