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

package inet.examples.manetrouting.meshTestsimple;


import inet.linklayer.ieee80211mesh.nodes.HostMesh;
import inet.linklayer.ieee80211mesh.nodes.HostSta;
import inet.linklayer.ieee80211mesh.nodes.BasicMobileManetMesh;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.linklayer.ieee80211mesh.nodes.gateWayMesh;
import inet.linklayer.ieee80211mesh.nodes.ApMeshNode;
import inet.node.ethernet.EtherSwitch;
import inet.linklayer.ieee80211mesh.nodes.ApMeshRelay;
import ned.DatarateChannel;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;



network meshTest
{
    parameters:
        int numSta = default(0);
        int numMeshHost = default(0);
        double nodeSeparation @unit(m) = default(240m);
    submodules:


        fixhost[6]: HostMesh {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 435m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation;
                mobility.initialX = nodeSeparation/2.0;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint;r=,,#707070");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=50,50");
        }
     
    connections allowunconnected:
}


