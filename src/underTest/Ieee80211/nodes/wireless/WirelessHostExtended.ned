//
// 2009 Juan-Carlos Maureira
//
// WirelessHost with variable Management module.  
//
// Based on WirelessHost from Andras Varga.
package inet.underTest.Ieee80211.nodes.wireless;

import inet.applications.IPingApp;
import inet.applications.ITCPApp;
import inet.applications.IUDPApp;
import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.mobility.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.tcp.TCP;
import inet.transport.udp.UDP;

import inet.linklayer.ieee80211.mgmt.Ieee80211AgentSTA;
import inet.underTest.Ieee80211.linklayer.ieee80211.mgmt.Ieee80211NicExtended;
import inet.underTest.Ieee80211.linklayer.ieee80211.mgmt.Ieee80211AgentInterface;

module WirelessHostExtended
{
    parameters:
        @node();
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        int numPingApps = default(0);  // no of PING apps. Specify the app types in INI file with pingApp[0..1].typename="PingApp" syntax        
        string tcpAppType = default("");
        string udpAppType = default("");
        bool IPForward = default(false);
        string routingFile = default("");
        string mobilityType = default("NoMobility");
        string mgmtType = default("Ieee80211MgmtSTAExtended");
        string agentType = default("Ieee80211AgentSTAExtended");
        @display("i=device/wifilaptop;bgb=516,438");
    gates:
        inout pppg[];
        inout ethg[];
        input radioIn @directIn;
    submodules:

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,150");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=60,230");
        }
        tcpApp[numTcpApps]: <tcpAppType> like ITCPApp {
            parameters:
                @display("p=252,70");
        }
        tcp: TCP {
            parameters:
                @display("p=252,157");
        }
        udpApp[numUdpApps]: <udpAppType> like IUDPApp {
            parameters:
                @display("p=361,70");
        }
        udp: UDP {
            parameters:
                @display("p=361,157");
        }
        pingApp[numPingApps]: <default("PingApp")> like IPingApp {
            parameters:
                @display("p=450,140");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=337,250;q=queue");
            gates:
                ifIn[(sizeof(pppg)+sizeof(ethg))+1];
                ifOut[(sizeof(pppg)+sizeof(ethg))+1];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=294,353,row,90;q=txQueue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=393,353,row,90;q=txQueue");
        }
        wlan: Ieee80211NicExtended {
            parameters:
                mgmtType = mgmtType;
                @display("p=167,353;q=queue;i=block/ifcard");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=58,301");
        }

        ieee80211Agent: <agentType> like Ieee80211AgentInterface {
            @display("p=167,250");
        }
    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        for i=0..numPingApps-1 {
            networkLayer.pingOut++ --> pingApp[i].pingIn;
            networkLayer.pingIn++ <-- pingApp[i].pingOut;
        }

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn[i];
            ppp[i].upperLayerIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].upperLayerIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }

        radioIn --> wlan.radioIn;
        wlan.upperLayerOut --> networkLayer.ifIn[sizeof(pppg)+sizeof(ethg)];
        wlan.upperLayerIn <-- networkLayer.ifOut[sizeof(pppg)+sizeof(ethg)];

        ieee80211Agent.mgmtOut --> wlan.agentIn;
        wlan.agentOut --> ieee80211Agent.mgmtIn;
}
