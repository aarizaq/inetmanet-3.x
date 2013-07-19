///**
//*\class       PASER
//*@brief       Class comprises configuration parameters of the  Position Aware Secure and Efficient Route Discovery Protocol (PASER)
//*@ingroup     Socket
//*\authors     Eugen.Paul | Mohamad.Sbeiti \@paser.info
// *
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
//********************************************************************************/

package inet.networklayer.manetrouting;

import inet.networklayer.manetrouting.base.BaseRouting;
import inet.networklayer.IManetRouting;

//
//
//
simple PASER extends BaseRouting like IManetRouting
{
    parameters:
        @class(PASER_Socket);
        @reactive;
        bool isGWsearch = default(false);
        double startRegTime @unit("s") = default(uniform(4.4s, 6.5s));//default(exponential(1s)); //default(exponential(1s));
        double PASER_radius = default(250);
        int PASER_RouteTimeOut = default(4600);    // timeout for routing entries (ms)
        int PASER_RouteDeleteTimeOut = default(21000);
        int PASER_NeighborTimeOut = default(4500); // timeout for neighbor entries (ms)
        int PASER_NeighborDeleteTimeOut = default(20000);
        int PASER_RREQWaitTime = default(1000);    // (ms) max wait time for a route request response
        int PASER_RREQTries = default(3);          // Rreq reties before fail and sending unreachable
        int PASER_RREPWaitTime = default(1000);    // (ms) max wait time for a route request replay
        int PASER_RREPTries = default(1);          // rrep reties before fail
        int PASER_KDCWaitTime = default(500);     // (ms) max wait time for a kdc request replay

        bool PASER_local_repair = default(false); // allow the routes to be locally repaired if a node detects link failure
        int PASER_local_repair_hopCount = default(10);  // max HopCout for a route that can be repaired

        int secret_parameter = default(14);        // number of generated secrets 2^secret_parameter
        int secret_repeat_broadcast_timeout = default(1000); // (ms) max wait time for a new root broadcast replay
        int secret_repeat_broadcast = default(2); // number of new root broadcast repetitions

        bool isHELLO = default(true);       // enable HELLO messages
        int HELLO_Interval = default(2000);  // (ms) hello sending interval
        bool isLinkLayer = default(true);   // enable link layer feedback

        double auth_tree_gen_delay = default(4.0);
        double auth_tree_verify_delay = default(0.000183);
        double encryption_rsa_delay = default(0.003703);
        double deryption_rsa_delay = default(0.035943);
        double sing_rsa_delay = default(0.038675);
        double verify_rsa_delay = default(0.001737);
        double one_hash_delay = default(0.000081);

        double PASER_Data_Message_Send_Delay = default(0.0);

        volatile double broadCastDelay @unit("s") = default(uniform(0s,0.005s));  // the delay added to broadcast operations if EqualDelay is set (used to model processing time) 
        bool PublicRoutingTables = default(false);
    gates:
        input from_ip;
        output to_ip;
}


