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

package inet.examples.manetrouting.pruebasP2PAleatorio;


import inet.nodes.inet.AdhocHost;
import inet.linklayer.ieee80211mesh.nodes.HostMeshApp;
import inet.linklayer.ieee80211mesh.nodes.HostMesh;
import inet.linklayer.ieee80211mesh.nodes.HostMeshMulti;
import inet.linklayer.ieee80211mesh.nodes.HostSta;
import inet.linklayer.ieee80211mesh.nodes.BasicMobileManetMesh;
import inet.world.radio.ChannelControl;
import inet.linklayer.ieee80211mesh.nodes.gateWayMesh;
import inet.linklayer.ieee80211mesh.nodes.ApMeshNode;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.inet.WirelessHost;
import inet.linklayer.ieee80211mesh.nodes.ApMeshRelay;
import inet.util.GlobalWirelessLinkInspector;
import inet.world.obstacles.ObstacleControl;
import inet.world.annotations.AnnotationManager;

network redSimple
{
    parameters:
        int numNodes = default(64);
        double nodeSeparation @unit(m) = default(80m);
    submodules:
        fixhost[numNodes]: HostMeshApp {
            parameters:
                @display("i=device/accesspoint");
        }

        globalWirelessLinkInspector: GlobalWirelessLinkInspector {
            parameters:
                @display("p=386,27;i=misc/city");
        }
        channelControl: ChannelControl {
            parameters:
                @display("p=60,23;i=misc/sun");
        }
        obstacles: ObstacleControl {
            @display("p=139,24");
        }
        annotations: AnnotationManager {
            @display("p=235,24");
        }
}


