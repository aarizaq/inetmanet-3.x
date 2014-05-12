//**
//*Authors Mohamad.Nehme | Mohamad.Sbeiti \@tu-dortmund.de
//*
//*copyright (C) 2013 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
//* at Technische Universitaet Dortmund, Germany
//* http://www.kn.e-technik.tu-dortmund.de/
//*
//*
//* This program is free software; you can redistribute it
//* and/or modify it under the terms of the GNU General Public
//* License as published by the Free Software Foundation; either
//* version 2 of the License, or (at your option) any later
//* version.
//* For further information see file COPYING
//* in the top level directory
//********************************************************************************
//* This work is part of the secure wireless mesh networks framework, which is currently under development by CNI */

package inet.securityModule;
simple SecurityWPA2
{
    parameters:
         // mn
        string ssid = default("SSID");
        double beaconInterval @unit("s") = default(100ms);
        int numAuthSteps = default(4); // use 2 for Open System auth, 4 for WEP, 4 for SAE   

        double authenticationTimeout_a @unit(s) = default(200ms); // timeout for the authentication procedure
        double authenticationTimeout_b @unit(s) = default(200ms); // timeout for the authentication procedure
        double groupAuthenticationTimeout @unit(s) = default(200ms); // timeout for the group hande shake pocedure        
        double PTKTimeout @unit("s") = default(600s); // handle new PTK, 4 way handshake 
        double GTKTimeout @unit("s") = default(600s); // handle new GTK 

        bool activeHandshake = default(true);  // selects between 'send only beacons' and 'normal operating'
        string PSK = default("12345");
        bool hasSecurity = default(false);
        bool securityActive = default(false);
        string iface = default("");

        @statistic[sentRequest](title="sent requests"; record=count,vector); // ENUM! ~Ieee80211PrimRequestCode
        @statistic[acceptConfirm](title="accepted confirms"; record=count,vector); // ENUM! ~Ieee80211PrimConfirmCode
        @statistic[dropConfirm](title="dropped confirms"; record=count,vector); // ENUM! ~Ieee80211PrimConfirmCode

            //dataRate: numeric; XXX TBD
        @signal[enqueuePk](type=cMessage);
        @signal[dequeuePk](type=cMessage);
        @signal[dropPkByQueue](type=cMessage);
        @signal[queueingTime](type=simtime_t; unit=s);
        @signal[dataQueueLen](type=long);  // number of stored packets
        @statistic[dataQueueLen](title="data queue length"; record=max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[dropPkByQueue](title="packets dropped by queue"; source=dropPkByQueue; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        
        @signal[AuthTimeoutsNr](type=long);  // number of stored packets
        @statistic[AuthTimeoutsNr](title="Number of Authentication Tiemouts"; source=AuthTimeoutsNr; record=count,vector; interpolationmode=none);
        
        @signal[BeaconTimeoutsNr](type=long);  // number of stored packets
        @statistic[BeaconTimeoutsNr](title="Number of Beacon Tiemouts"; source=BeaconTimeoutsNr; record=count,vector; interpolationmode=none);
        
        @signal[deletedFramesNr](type=long);  // number of stored packets
        @statistic[deletedFramesNr](title="Number of deleted Frames"; source=deletedFramesNr; record=count,vector; interpolationmode=none);


        @class(SecurityWPA2);

    gates:
        input mgmtIn;
        output mgmtOut;
}
