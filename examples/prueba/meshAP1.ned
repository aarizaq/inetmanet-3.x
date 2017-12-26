//APMeshNode in place of APMeshRelay

//802.11n
//With minimum power for UE and distance between AP to be 200m


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

package inet.examples.prueba;


import inet.linklayer.ieee80211mesh.nodes.HostMesh;
import inet.linklayer.ieee80211mesh.nodes.HostMeshMulti;
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
import inet.common.figures.DelegateSignalConfigurator;
import inet.visualizer.contract.IIntegratedVisualizer;

channel cable extends DatarateChannel
{
    parameters:
        delay = 0.1us;
        datarate = 1000Mbps;
}

network meshAP1
{
    parameters:
        int numMeshHost = default(0);
        double nodeSeparation @unit(m) = default(120m);
        @display("bgb=4000,4000;bgg=100,1,grey95");
        
        //@figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);
        @figure[rcvdPkText0](type=indicatorText; pos=1500,250; anchor=w; font=,50; textFormat="pk rcd by Host: %g"; initialValue=0);
        @statistic[rcvdPk0](source=host0_rcvdPk; record=figure(count); targetFigure=rcvdPkText0);
        @signal[host0_rcvdPk];
        @delegatesignal[rcvdPk0](source=meshHost.udpApp[0].rcvdPk; target=host0_rcvdPk);
        
        
        
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=50,50");
        }
        
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=100,50");
        }
        
        configurator: IPv4NetworkConfigurator {
            @display("p=150,50");
        }
        
        figureHelper: DelegateSignalConfigurator {
            @display("p=75,50");
        }
        
       
        SW: EtherSwitch {
            @display("p=1,1");

        }
       
        MPP1: gateWayMesh {
            parameters:
                wlanMesh.hasLocator = true;
                @display("i=device/router;r=,,#707070;p=10,10");
        }
        
        MPP2: gateWayMesh {
            parameters:
                wlanMesh.hasLocator = true;
                @display("i=device/router;r=,,#707070;p=1000,10");
        }
        
        MPP3: gateWayMesh {
            parameters:
                wlanMesh.hasLocator = true;
                @display("i=device/router;r=,,#707070;p=10,1000");
        }
        
        fixhost1[4]: ApMeshNode {//ApMeshRelay{ //ApMeshNode {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 200m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = 200m;
                mobility.initialX = 200m;
                wlanMesh.hasLocator = true;
                //numRadios=2;
                @display("i=device/accesspoint;r=,,#707070");
        }
        
        fixhost2[4]: ApMeshNode {//ApMeshRelay{ //ApMeshNode {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 400m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = 200m;
                mobility.initialX = 200m;
                wlanMesh.hasLocator = true;
                //numRadios=2;
                @display("i=device/accesspoint;r=,,#707070");
        }
        
        fixhost3[4]: ApMeshNode {//ApMeshRelay{ //ApMeshNode {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 600m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = 200m;
                mobility.initialX = 200m;
                wlanMesh.hasLocator = true;
                //numRadios=2;
                @display("i=device/accesspoint;r=,,#707070");
        }
        
        fixhost4[4]: ApMeshNode {//ApMeshRelay{ //ApMeshNode {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 800m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation = 200m;
                mobility.initialX = 200m;
                wlanMesh.hasLocator = true;
                //numRadios=2;
                @display("i=device/accesspoint;r=,,#707070");
        }
        
        
        meshHost: HostSta {//HostMesh {
              parameters:
                wlanMesh.hasLocator = true;
                @display("r=,,#707070;p=900,900");
        }       
    connections allowunconnected:
       
        MPP1.ethg++ <--> cable <--> SW.ethg++;
        MPP1.ethg++ <--> cable <--> MPP2.ethg++;
        MPP1.ethg++ <--> cable <--> MPP3.ethg++;
        //MPP1.ethg++ <--> cable <--> SW.ethg++;
        
}


