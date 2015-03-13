//*********************************************************************************
// File:           HipHost6.ned
//
// Authors:        Laszlo Tamas Zeke, Levente Mihalyi, Laszlo Bokor
//
// Copyright: (C) 2008-2009 BME-HT (Department of Telecommunications,
// Budapest University of Technology and Economics), Budapest, Hungary
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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//**********************************************************************************
// Part of: HIPSim++ Host Identity Protocol Simulation Framework developed by BME-HT
//**********************************************************************************
// Adapted to Inetmanet by Alfonso Ariza
// 
// Copyright (C) 2011 Alfonso Ariza

package inet.underTest.hip.nodes;

import inet.underTest.hip.base.HIP;
import inet.node.ipv6.StandardHost6;
import inet.applications.contract.IPingApp;
import inet.applications.contract.ISCTPApp;
import inet.applications.contract.ITCPApp;
import inet.applications.contract.IUDPApp;
import inet.transportlayer.contract.ISCTP;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;



//
// \IPv6 host with TCP, UDP layers and applications.
//
module HipHost6B extends StandardHost6
{
     parameters:
        @display("i=device/pc2");
    submodules:
        hip: HIP {
            parameters:
                @display("i=block/star;p=297,200");
        }
    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }
       
        tcp.ipOut --> hip.tcpIn  if hasTcp;
        hip.tcp6Out --> networkLayer.transportIn++ if hasTcp;
        tcp.ipIn <-- hip.tcpOut if hasTcp;
        hip.tcp6In <-- networkLayer.transportOut++ if hasTcp;


		for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> hip.udpIn if hasUdp;
        hip.udp6Out --> networkLayer.transportIn++ if hasUdp;
        udp.ipIn <-- hip.udpOut if hasUdp;
        hip.udp6In <-- networkLayer.transportOut++ if hasUdp;
        
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
        //hip.fromNetworkAddrChange <-- networkLayer.ipchangeOut;
}

