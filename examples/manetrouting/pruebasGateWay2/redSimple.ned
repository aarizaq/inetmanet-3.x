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

package inet.examples.manetrouting.pruebasGateWay2;


import inet.nodes.inet.AdhocHost;
import inet.linklayer.ieee80211mesh.nodes.HostMesh;
import inet.linklayer.ieee80211mesh.nodes.HostSta;
import inet.linklayer.ieee80211mesh.nodes.BasicMobileManetMesh;
import inet.world.radio.ChannelControl;
import inet.linklayer.ieee80211mesh.nodes.gateWayMesh;
import inet.linklayer.ieee80211mesh.nodes.ApMeshNode;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.inet.WirelessHost;
import inet.linklayer.ieee80211mesh.nodes.ApMeshRelay;
network redSimple
{
    parameters:
        int numHostsLower;
        int numFixHosts;
        int numBasic;
        double nodeSeparation @unit(m) = default(80m);
    submodules:
        fixhost[1]: HostSta {

            @display("p=24,66");
        }
        fixhostA[1]: BasicMobileManetMesh {
            parameters:
                mobilityType = "LinealNodeDistributionMobility";
                mobility.initialY = 150m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.initialX = nodeSeparation/2.0+nodeSeparation;
                mobility.separation = nodeSeparation*2;
                @display("i=block/wrxtx;r=,,#707070;p=158,150");
        }
        fixhostB[1]: ApMeshRelay {
            parameters:
                mobilityType = "LinealNodeDistributionMobility";
                mobility.initialY = 150m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.initialX = (nodeSeparation/2.0);
                mobility.separation = nodeSeparation*2;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint;r=,,#707070;p=48,114");

        }
        channelControl: ChannelControl {
            parameters:
                @display("p=60,23;i=misc/sun");
        }
        gateWay: HostMesh {
            parameters:
                wlanMesh.hasLocator = true;
                @display("i=device/router;r=,,#707070;p=245,115");
        }
}


