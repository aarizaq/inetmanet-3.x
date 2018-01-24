//Simple configuration made to work mesh topology where source and target can directly talk

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
import inet.visualizer.contract.IIntegratedVisualizer;



network Hwmp_multiradio_1
{
    parameters:
        int numSta = default(0);
        int numMeshHost = default(0);
        double nodeSeparation @unit(m) = default(240m);
        @display("bgb=1800,1800;bgg=100,1,grey95");
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

                
        fixhost[3]: HostMeshMulti  {
            parameters:
                mobilityType = "LinearNodeDistributionMobility";
                mobility.initialY = 100m;// (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
                mobility.separation =500m;//nodeSeparation;
				mobility.initialX = 100m;
                wlanMesh.hasLocator = true;
                @display("i=device/accesspoint");//r=,,#707070");
                //@display("p=,,m,3,250,250;i=device/accesspoint;r=,,#707070");
        }
        
    connections allowunconnected:
}


