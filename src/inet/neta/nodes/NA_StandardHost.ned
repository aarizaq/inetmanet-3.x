//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

//
// Copyright (C) 2013 and modified by NESG (Network Engineering and Security Group), http://nesg.ugr.es,
// - Gabriel Maciá Fernández (gmacia@ugr.es)
// - Leovigildo Sánchez Casado (sancale@ugr.es)
// - Rafael A. Rodríguez Gómez (rodgom@ugr.es)
// - Roberto Magán Carrión (rmagan@ugr.es)
// - Pedro García Teodoro (pgteodor@ugr.es)
// - José Camacho Páez (josecamacho@ugr.es)
// - Jesús E. Díaz Verdejo (jedv@ugr.es)
//
package inet.neta.nodes;

import inet.applications.contract.IPingApp;
import inet.applications.contract.ISCTPApp;
import inet.applications.contract.ITCPApp;
import inet.applications.contract.IUDPApp;
import inet.transportlayer.contract.ISCTP;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;
import inet.applications.contract.ITunApp;
import inet.common.misc.ThruputMeter;

//
// Generic standard host. Modificated from StandarHost module. 
// Migrated from INET for inheritance requirements 
// of NA_AttackerAdhocHost and NA_AttackerWirelessHost
//
// @see StandarHost, NodeBase
//
// @author Gabriel Maciá Fernández, gmacia@ugr.es
// @date 01/22/2013
module NA_StandardHost extends NA_NodeBase
{
    parameters:
        @display("i=device/pc2");
        int numTcpApps = default(0);  // no of TCP apps. Specify the app types in INI file with tcpApp[0..1].typename="TCPEchoApp" syntax
        int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax
        int numSctpApps = default(0); // no of SCTP apps. Specify the app types in INI file with sctpApp[0..1].typename="SCTPServer" syntax
        int numPingApps = default(0);  // no of PING apps. Specify the app types in INI file with pingApp[0..1].typename="PingApp" syntax
        bool hasTcp = default(numTcpApps > 0);
        bool hasUdp = default(numUdpApps > 0);
        bool hasSctp = default(numSctpApps > 0);
        bool hasTun = default(numTunInterfaces > 0);
        bool hasThrugmeter = default(false);
        string tcpType = default(firstAvailableOrEmpty("TCP", "TCP_lwIP", "TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailableOrEmpty("UDP"));
        string sctpType = default(firstAvailableOrEmpty("SCTP"));
        forwarding = default(false);  // disable routing by default
        networkLayer.proxyARP = default(false);
    submodules:
        
        tunApp[numTunInterfaces]: <> like ITunApp {
            parameters:
                @display("p=280,40");
        }
        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=147,54,row,60");
        }
        truTcpApp[numTcpApps*2]: ThruputMeter if hasTcp && hasThrugmeter {
            parameters:
                @display("p=189,100");
        }
        tcp: <tcpType> like ITCP if hasTcp {
            parameters:
                @display("p=147,141");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("p=329,54,row,60");
        }
        truUdpApp[numUdpApps*2]: ThruputMeter if hasUdp && hasThrugmeter {
            parameters:
                @display("p=380,100");
        }        
        udp: <udpType> like IUDP if hasUdp {
            parameters:
                @display("p=329,141");
        }
        sctpApp[numSctpApps]: <> like ISCTPApp {
            parameters:
                @display("p=527,54,row,60");
        }
        sctp: <sctpType> like ISCTP if hasSctp {
            @display("p=527,141");
        }
        pingApp[numPingApps]: <default("PingApp")> like IPingApp {
            parameters:
                @display("p=635,141,row,60");
        }
    connections allowunconnected:
       for i=0..numTcpApps-1 , if !hasThrugmeter{
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }
        
        for i=0..numTcpApps-1, if  hasThrugmeter{
            tcpApp[i].tcpOut --> truTcpApp[2*i].in;
            truTcpApp[2*i].out --> tcp.appIn++;
            tcpApp[i].tcpIn <-- truTcpApp[(2*i)+1].out;
            truTcpApp[(2*i)+1].in <-- tcp.appOut++;
        }
        
        tcp.ipOut --> networkLayer.transportIn++ if hasTcp;
        tcp.ipIn <-- networkLayer.transportOut++ if hasTcp;

        for i=0..numUdpApps-1, if !hasThrugmeter {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        for i=0..numUdpApps-1, if  hasThrugmeter{
            udpApp[i].udpOut --> truUdpApp[2*i].in;
            truUdpApp[2*i].out --> udp.appIn++;
            udpApp[i].udpIn <-- truUdpApp[(2*i)+1].out;
            truUdpApp[(2*i)+1].in <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.transportIn++ if hasUdp;
        udp.ipIn <-- networkLayer.transportOut++ if hasUdp;

        for i=0..numSctpApps-1 {
            sctpApp[i].sctpOut --> sctp.from_appl++;
            sctp.to_appl++ --> sctpApp[i].sctpIn;
        }
        sctp.to_ip --> networkLayer.transportIn++ if hasSctp;
        networkLayer.transportOut++ --> sctp.from_ip if hasSctp;

        for i=0..numPingApps-1 {
            networkLayer.pingOut++ --> pingApp[i].pingIn;
            networkLayer.pingIn++ <-- pingApp[i].pingOut;
        }

        for i=0..numTunInterfaces-1 {
            tunApp[i].tunOut --> tun[i].appIn if hasTun;
            tunApp[i].tunIn <-- tun[i].appOut if hasTun;
        }
}
