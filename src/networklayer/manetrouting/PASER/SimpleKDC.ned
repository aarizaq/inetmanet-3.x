//
//\class       SimpleKDC
// *@ingroup KDC
// *@brief       Class handles all Key Distribution Center (KDC) actions
// *
// *\authors     Eugen.Paul | Mohamad.Sbeiti \@paser.info
// *
// *\copyright   (C) 2012 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
// *                  at Technische Universitaet Dortmund, Germany
// *                  http://www.kn.e-technik.tu-dortmund.de/
// *
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// ********************************************************************************
// * This work is part of the secure wireless mesh networks framework, which is currently under development by CNI
// ********************************************************************************/
package inet.networklayer.manetrouting.PASER;
import inet.applications.IUDPApp;

simple SimpleKDC like IUDPApp
{
    parameters:
        int port = default(653); // port number to listen on
        double kdc_verify_mess_delay = default(0.000041); //s
        double kdc_sign_mess_delay = default(0.000194); //s
        double kdc_rsa_enc_delay = default(0.000046); //s
        double kdc_rsa_dec_delay = default(0.000296); //s

        double resetMessageTime = default(-35.5); //s

        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int typeOfService = default(-1); // if not -1, set the ToS (IPv4) or Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // if not empty, set the multicast output interface option on the socket (interface name expected) 
        bool receiveBroadcast = default(false); // if true, makes the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(false); // if true, makes the socket receive packets from all multicast groups set on local interfaces

        @display("i=block/app");
    gates:
        input udpIn @labels(TCPCommand/up);
        output udpOut @labels(TCPCommand/down);
}
