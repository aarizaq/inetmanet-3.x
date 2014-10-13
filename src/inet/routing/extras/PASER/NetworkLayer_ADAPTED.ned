////
//// Copyright (C) 2004 Andras Varga
//// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
////
//// This program is free software; you can redistribute it and/or
//// modify it under the terms of the GNU General Public License
//// as published by the Free Software Foundation; either version 2
//// of the License, or (at your option) any later version.
////
//// This program is distributed in the hope that it will be useful,
//// but WITHOUT ANY WARRANTY; without even the implied warranty of
//// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//// GNU General Public License for more details.
////
//// You should have received a copy of the GNU General Public License
//// along with this program; if not, write to the Free Software
//// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////
//
//
//
package inet.routing.extras.PASER;
//
//import inet.networklayer.arp.ARP;
//import inet.networklayer.ipv4.ErrorHandling;
//import inet.networklayer.ipv4.ICMP;
//import inet.networklayer.ipv4.IGMP;
//import inet.networklayer.ipv4.IPv4;
//
//import paser.ned.paser.*;
//
////
//// Network layer of an \IP node.
////
//// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
////
//module NetworkLayerGlobalArp_paser
//{
//    parameters:
//        bool proxyARP = default(true);
//        bool globalARP = default(true);
//        @display("i=block/fork");
//    gates:
//        input ifIn[] @labels(IPDatagram);
//        input tcpIn @labels(TCPSegment,IPControlInfo/down);
//        input udpIn @labels(UDPPacket,IPControlInfo/down);
//        input sctpIn @labels(IPControlInfo/down,SCTPPacket);   //I.R.
//        input rsvpIn @labels(IPControlInfo/down);
//        input ospfIn @labels(IPControlInfo/down);
//        input pingIn[];
//        input  manetIn;        
//        output ifOut[];
//        output tcpOut @labels(TCPSegment,IPControlInfo/up);
//        output udpOut @labels(UDPPacket,IPControlInfo/up);
//        output sctpOut @labels(IPControlInfo/up,SCTPPacket); //I.R.
//        output rsvpOut @labels(IPControlInfo/up);
//        output ospfOut @labels(IPControlInfo/up);
//        output pingOut[];
//        output manetOut;
//
//    submodules:
//        ip: IPv4_paser {
//            parameters:
//                timeToLive = 32;
//                multicastTimeToLive = 32;
//                fragmentTimeout = 60s;
////                protocolMapping = "6:0,17:1,1:2,2:3,46:4,89:5,132:6,254:7,135:7";
//				protocolMapping = "6:0,17:1,1:2,2:3,46:4,89:5,132:6,48:7,138:7";
//                @display("p=85,95;q=queue");
//            gates:
//                transportIn[8];
//                transportOut[8];
//                queueIn[sizeof(ifIn)];
//        }
//        arp: ARP {
//            parameters:
//                proxyARP = proxyARP;
//                globalARP = globalARP;
//                //edit by eugen
////                proxyARP = true;
////                globalARP = false;
//                @display("p=163,206;q=pendingQueue");
//            gates:
//                nicOut[sizeof(ifOut)];
//        }
//        icmp: ICMP {
//            parameters:
//                @display("p=160,63");
//        }
//        igmp: IGMP {
//            parameters:
//                @display("p=160,122");
//        }
//        errorHandling: ErrorHandling {
//            parameters:
//                @display("p=239,63");
//        }
//    connections allowunconnected:
//        // transport Layer
////        ip.transportOut[0] --> tcpOut;
////        ip.transportIn[0] <-- tcpIn;
////
////        ip.transportOut[1] --> udpOut;
////        ip.transportIn[1] <-- udpIn;
////
////        ip.transportOut[2] --> icmp.localIn;
////        ip.transportIn[2] <-- icmp.sendOut;
////
////        ip.transportOut[3] --> igmp.localIn;
////        ip.transportIn[3] <-- igmp.sendOut;
////
////        ip.transportOut[4] --> rsvpOut;
////        ip.transportIn[4] <-- rsvpIn;
////
////        ip.transportOut[5] --> ospfOut;
////        ip.transportIn[5] <-- ospfIn;
////
////        ip.transportOut[6] --> sctpOut;     //I.R.
////        ip.transportIn[6] <-- sctpIn;
////        
////        ip.transportOut[7] --> manetOut;
////        ip.transportIn[7] <-- manetIn;
//		ip.transportOut[0] --> { @display("m=n"); } --> tcpOut;
//        ip.transportIn[0] <-- { @display("m=n"); } <-- tcpIn;
//
//        ip.transportOut[1] --> { @display("m=n"); } --> udpOut;
//        ip.transportIn[1] <-- { @display("m=n"); } <-- udpIn;
//
//        ip.transportOut[2] --> icmp.localIn;
//        ip.transportIn[2] <-- icmp.sendOut;
//
//        ip.transportOut[3] --> igmp.localIn;
//        ip.transportIn[3] <-- igmp.sendOut;
//
//        ip.transportOut[4] --> { @display("m=n"); } --> rsvpOut;
//        ip.transportIn[4] <-- { @display("m=n"); } <-- rsvpIn;
//
//        ip.transportOut[5] --> { @display("m=n"); } --> ospfOut;
//        ip.transportIn[5] <-- { @display("m=n"); } <-- ospfIn;
//
//        ip.transportOut[6] --> { @display("m=n"); } --> sctpOut;
//        ip.transportIn[6] <-- { @display("m=n"); } <-- sctpIn;
//
//        ip.transportOut[7] --> { @display("m=n"); } --> manetOut;
//        ip.transportIn[7] <-- { @display("m=n"); } <-- manetIn;
//
////        icmp.pingOut --> pingOut;
////        icmp.pingIn <-- pingIn;
//		for i=0..sizeof(pingOut)-1 {
//            icmp.pingOut++ --> { @display("m=n"); } --> pingOut[i];
//        }
//        for i=0..sizeof(pingIn)-1 {
//            icmp.pingIn++ <-- { @display("m=n"); } <-- pingIn[i];
//        }
//
//        icmp.errorOut --> errorHandling.in;
//
//        ip.queueOut --> arp.ipIn;
//
//        // L2 interfaces to IP and from ARP
//        for i=0..sizeof(ifOut)-1 {
//            ifIn[i] --> { @display("m=s"); } --> ip.queueIn[i];
// //           ifOut[i] <-- { @display("m=s"); } <-- arp.nicOut[i];
//        }
//}
//

import inet.networklayer.arp.IARP;
import inet.networklayer.configurator.ipv4.IPv4NodeConfigurator;
import inet.networklayer.ipv4.ErrorHandling;
import inet.networklayer.ipv4.ICMP;
import inet.networklayer.ipv4.IIGMP;


//
// Network layer of an IPv4 node.
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
//
module NetworkLayer_Paser
{
    parameters:
        @display("i=block/fork");      
        
        bool proxyARP = default(true);
        string igmpType = default("IGMPv2");
        string arpType = default("ARP");
        string interfaceTableModule;
        string routingTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(routingTableModule));
        *.arpModule = default(absPath(".arp"));
        *.icmpModule = default(absPath(".icmp"));
        arp.respondToProxyARP = proxyARP;
        @display("i=block/fork");
        
    gates:
        input ifIn[] @labels(IPv4Datagram);
        input tcpIn @labels(TCPSegment,IPv4ControlInfo/down);
        input udpIn @labels(UDPPacket,IPv4ControlInfo/down);
        input sctpIn @labels(IPv4ControlInfo/down,SCTPPacket);
        input rsvpIn @labels(IPv4ControlInfo/down);
        input ospfIn @labels(IPv4ControlInfo/down);
        input pingIn[];
        input manetIn;
        input igmpIn;
        output ifOut[] @labels(IPv4Datagram);
        output tcpOut @labels(TCPSegment,IPv4ControlInfo/up);
        output udpOut @labels(UDPPacket,IPv4ControlInfo/up);
        output sctpOut @labels(IPv4ControlInfo/up,SCTPPacket);
        output rsvpOut @labels(IPv4ControlInfo/up);
        output ospfOut @labels(IPv4ControlInfo/up);
        output pingOut[];
        output manetOut;
        output igmpOut;

    submodules:
        configurator: IPv4NodeConfigurator {
            @display("p=39,158");
        }
        ip: IPv4_paser {
            parameters:
                multicastTimeToLive = 32;
                protocolMapping = "6:0,17:1,1:2,2:3,46:4,89:5,132:6,48:7,138:7"; // see: http://www.iana.org/assignments/protocol-numbers
                @display("p=85,95;q=queue");
            gates:
                transportIn[8];
                transportOut[8];
                queueIn[sizeof(ifIn)];
        }
        arp: <arpType> like IARP {
            parameters:
                @display("p=165,120;q=pendingQueue;is=s");
        }
        icmp: ICMP {
            parameters:
                @display("p=160,63");
        }
        igmp: <igmpType> like IIGMP {
            parameters:
                @display("p=39,33");
        }
        errorHandling: ErrorHandling {
            parameters:
                @display("p=239,63");
        }

    connections allowunconnected:
        // transport Layer
        ip.transportOut[0] --> { @display("m=n"); } --> tcpOut;
        ip.transportIn[0] <-- { @display("m=n"); } <-- tcpIn;

        ip.transportOut[1] --> { @display("m=n"); } --> udpOut;
        ip.transportIn[1] <-- { @display("m=n"); } <-- udpIn;

        ip.transportOut[2] --> icmp.localIn;
        ip.transportIn[2] <-- icmp.sendOut;

        ip.transportOut[3] --> igmp.ipIn;
        ip.transportIn[3] <-- igmp.ipOut;

        ip.transportOut[4] --> { @display("m=n"); } --> rsvpOut;
        ip.transportIn[4] <-- { @display("m=n"); } <-- rsvpIn;

        ip.transportOut[5] --> { @display("m=n"); } --> ospfOut;
        ip.transportIn[5] <-- { @display("m=n"); } <-- ospfIn;

        ip.transportOut[6] --> { @display("m=n"); } --> sctpOut;
        ip.transportIn[6] <-- { @display("m=n"); } <-- sctpIn;

        ip.transportOut[7] --> { @display("m=n"); } --> manetOut;
        ip.transportIn[7] <-- { @display("m=n"); } <-- manetIn;

        for i=0..sizeof(pingOut)-1 {
            icmp.pingOut++ --> { @display("m=n"); } --> pingOut[i];
        }
        for i=0..sizeof(pingIn)-1 {
            icmp.pingIn++ <-- { @display("m=n"); } <-- pingIn[i];
        }

        icmp.errorOut --> errorHandling.in;

        ip.arpOut --> arp.netwIn;
        ip.arpIn <-- arp.netwOut;

        igmp.routerOut --> igmpOut;
        igmp.routerIn <-- igmpIn;

        for i=0..sizeof(ifIn)-1 {
            ifIn[i] --> { @display("m=s"); } --> ip.queueIn[i];
        }
}

