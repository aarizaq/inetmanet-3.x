///*
//*\class       PASERUDpTrafficSender
//*@brief       Class provides a constant bit rate UDP sender. It simulates to a large extent an Iperf UDP client.
//*
//*\authors     Dennis Kaulbars | Sebastian Subik \@tu-dortmund.de | Eugen.Paul | Mohamad.Sbeiti \@paser.info
//*
//*\copyright   (C) 2012 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
//*                  at Technische Universitaet Dortmund, Germany
//*                  http://www.kn.e-technik.tu-dortmund.de/
//*
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//********************************************************************************
//* This work is part of the secure wireless mesh networks framework, which is currently under development by CNI
//******************************************************************************** 
 // This is constant bit rate APP - Similar to Iperf UDP
package inet.applications.udpapp;

import inet.applications.IUDPApp;


simple PASERUdpTrafficSender like IUDPApp
{
    parameters:
        @display("i=block/departure");
        double bitRate = default(0.02);
        int packetLength = default(100);
        double offset = default(100);
        double stopTime = default(500);
        string destAddr;
        int port;

        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int typeOfService = default(-1); // if not -1, set the ToS (IPv4) or Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // if not empty, set the multicast output interface option on the socket (interface name expected) 
        bool receiveBroadcast = default(false); // if true, makes the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(false); // if true, makes the socket receive packets from all multicast groups set on local interfaces

    gates:
        input udpIn;
        output udpOut;

        input manetIn;
}
