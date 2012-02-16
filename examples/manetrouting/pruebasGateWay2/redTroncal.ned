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
import inet.linklayer.ieee80211mesh.nodes.BasicMobileManetMesh;
import inet.world.radio.ChannelControl;
import inet.linklayer.ieee80211mesh.nodes.gateWayMesh;
import inet.linklayer.ieee80211mesh.nodes.ApMeshNode;
import inet.nodes.ethernet.EtherSwitch;

network redTroncal
{
    parameters:
        int numHostsLower;
        int numFixHosts;
        int numBasic;
        double nodeSeparation @unit(m)= default(80m);
    submodules:
        fixhost[15]: ApMeshNode {
            parameters:
                mobility.mgmtType = "LinealNodeDistributionMobility";
                mobility.initialY = 315;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);                
                mobility.separation = nodeSeparation;
                @display("i=device/accesspoint;r=,,#707070");
        }
        fixhost2[8]: ApMeshNode {
            parameters:
                mobility.mgmtType = "LinealNodeDistributionMobility";
                mobility.initialY = 235;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;
                @display("i=device/accesspoint;r=,,#707070");
        }
        fixhost3[8]: ApMeshNode {
            parameters:
                mobility.mgmtType = "LinealNodeDistributionMobility";
                mobility.initialY = 395;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;
                @display("i=device/accesspoint;r=,,#707070");
        }

        fixhost4[15]: ApMeshNode {
            parameters:
                mobility.mgmtType = "LinealNodeDistributionMobility";
                mobility.initialY = 155;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;                
                @display("i=device/accesspoint;r=,,#707070");
        }
        fixhost5[15]: ApMeshNode {
            parameters:
                mobility.mgmtType = "LinealNodeDistributionMobility";
                mobility.initialY = 475;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;                
                @display("i=device/accesspoint;r=,,#707070");
        }

        fixhost6[8]: ApMeshNode {
            parameters:
                mobility.mgmtType = "LinealNodeDistributionMobility";
                mobility.initialY = 75;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;                
                @display("i=device/accesspoint;r=,,#707070");
        }
        fixhost7[8]: ApMeshNode {
            parameters:
                mobility.mgmtType = "LinealNodeDistributionMobility";
                mobility.initialY = 555;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = nodeSeparation*2;                
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
                @display("i=device/router;r=,,#707070;p=159,127");
        }
        gateWay1: gateWayMesh {
            parameters:
                @display("i=device/router;r=,,#707070;p=94,507");
        }
        gateWay2: gateWayMesh {
            parameters:
                @display("i=device/router;r=,,#707070;p=650,345");
        }

        gateWay3: gateWayMesh {
            parameters:
                @display("i=device/router;r=,,#707070;p=1126,127");
        }
        gateWay4: gateWayMesh {
            parameters:
                @display("i=device/router;r=,,#707070;p=1062,507");
        }

    connections allowunconnected:
        gateWay.ethg++ <--> SW.ethg++;
        gateWay1.ethg++ <--> SW.ethg++;
        gateWay2.ethg++ <--> SW.ethg++;
        gateWay3.ethg++ <--> SW.ethg++;
        gateWay4.ethg++ <--> SW.ethg++;
}


