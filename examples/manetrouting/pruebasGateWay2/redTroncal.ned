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
import inet.linklayer.ieee80211mesh.nodes.ApMeshRelay;
import inet.nodes.ethernet.EtherSwitch;
import inet.linklayer.ieee80211mesh.nodes.ApMeshRelay;
import ned.DatarateChannel;

channel cable extends DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 1000Mbps;
}

network redTroncal
{
    parameters:
        int numSta = default(0);
        int numFixHosts;
        int numBasic;
        double nodeSeparation @unit(m) = default(80m);
    submodules:
        staHost[numSta]: HostSta {
            @display("p=247,89");
        }
        fixhostA[8]: BasicMobileManetMesh {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 315m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.initialX = nodeSeparation/2.0;
                mobility.separation = nodeSeparation*2;
                @display("i=block/wrxtx;r=,,#707070");
        }
        fixhostB[7]: ApMeshRelay {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 315m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.initialX = (nodeSeparation/2.0)+nodeSeparation;
                mobility.separation = nodeSeparation*2;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint;r=,,#707070");

        }
        fixhost2[8]: ApMeshRelay {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 235m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.initialX = nodeSeparation/2.0;
                mobility.separation = nodeSeparation*2;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint;r=,,#707070");
        }
        fixhost3[8]: ApMeshRelay {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 395m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;
                mobility.initialX = nodeSeparation/2.0;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint;r=,,#707070");
        }

        fixhost4A[8]: BasicMobileManetMesh {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 155m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;
                mobility.initialX = nodeSeparation/2.0;
                @display("i=block/wrxtx;r=,,#707070");
        }
        fixhost4B[7]: ApMeshRelay {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 155m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;
                mobility.initialX = (nodeSeparation/2.0) + nodeSeparation;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint;r=,,#707070");
        }
        fixhost5A[8]: BasicMobileManetMesh {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 475m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;
                mobility.initialX = nodeSeparation/2.0;
                @display("i=block/wrxtx;r=,,#707070");
        }
        fixhost5B[7]: ApMeshRelay {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 475m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;
                mobility.initialX = (nodeSeparation/2.0) + nodeSeparation;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint;r=,,#707070");
        }
        fixhost6[8]: ApMeshRelay {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 75m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;
                mobility.initialX = nodeSeparation/2.0;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint");
        }
        fixhost7[8]: ApMeshRelay {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 555m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;
                mobility.initialX = nodeSeparation/2.0;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint;r=,,#707070");
        }
        channelControl: ChannelControl {
            parameters:
                @display("p=60,23;i=misc/sun");
        }
        SW: EtherSwitch {
            @display("p=559,344");

        }
        gateWay: gateWayMesh {
            parameters:
                wlanMesh.hasLocator = true;
                @display("i=device/router;r=,,#707070;p=159,127");
        }
        gateWay1: gateWayMesh {
            parameters:
                wlanMesh.hasLocator = true;
                @display("i=device/router;r=,,#707070;p=94,507");
        }
        gateWay2: gateWayMesh {
            parameters:
                wlanMesh.hasLocator = true;
                @display("i=device/router;r=,,#707070;p=650,345");
        }

        gateWay3: gateWayMesh {
            parameters:
                wlanMesh.hasLocator = true;
                @display("i=device/router;r=,,#707070;p=1126,127");
        }
        gateWay4: gateWayMesh {
            parameters:
                wlanMesh.hasLocator = true;
                @display("i=device/router;r=,,#707070;p=1062,507");
        }

    connections allowunconnected:
        gateWay.ethg++ <--> cable <--> SW.ethg++;
        gateWay1.ethg++ <--> cable <--> SW.ethg++;
        gateWay2.ethg++ <--> cable <--> SW.ethg++;
        gateWay3.ethg++ <--> cable <--> SW.ethg++;
        gateWay4.ethg++ <--> cable <--> SW.ethg++;
}


