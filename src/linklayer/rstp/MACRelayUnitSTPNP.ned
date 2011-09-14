package inet.linklayer.rstp;
//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
// Copyright (C) 2009 Juan-Carlos Maureira. INRIA 2009
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

//
// A MACRelayRSTPUnit implementation which models one or more CPUs
// with shared memory, working from a single shared queue with 
// the Rapid Spanning Tree Algorithm to avoid loops
//
// It also models fixed delay for precessing each frame.
// Finite memory is taken into account by dropping frames if
// total number of bits enqueued exceed a given limit.
//
// A simple scheme for sending PAUSE frames is built in (although
// users will probably change it). When the buffer level goes
// above a high watermark, PAUSE frames are sent on all ports.
// The watermark and the pause time is configurable; use zero
// values to disable the PAUSE feature.

import inet.linklayer.IMACRelayUnit;

simple MACRelayUnitSTPNP like IMACRelayUnit
{
    parameters:
        int numWirelessPorts = default(0); // see ~IMACRelayUnit
        string addressTableFile = default("");  // see MACRelayUnit
        int addressTableSize = default(100); // see MACRelayUnit
        double agingTime @unit("s") = default(300s); // see MACRelayUnit
        int numCPUs = default(1);  // number of CPUs
        double processingTime @unit("s") = default(0s);  // processing time of one frame
        int bufferSize @unit("B") = default(1MiB);  // memory
        int highWatermark @unit("B") = default(512KiB);  // buffer usage threshold to send PAUSE frame
        int pauseUnits = default(300);  // time to put in PAUSE frames (in units of 512 bit times)
        
        // RSTP parameters
        string bridgeAddress = default("auto"); // bridge mac address (automatic)
        int priority = default(32768);     // bridge priority (0x8000 default value)
        double helloTime = default(2);     // hello time delay (default 2 sec)
        double maxAge    = default(20);    // max age of a BPDU  (20 sec or hops default for RSTP). BPDU TimeToLive
        double bpduTimeout  = default(6);  // BPDU timeout to considers the root port lost (default 6 sec for RSTP) 
        double forwardDelay = default(14); // time taken to get a port into a forwarding mode (14 sec default)
        double holdTime  = default(1);     // hold time to avoid rate excess limit of BPDUs in a port (1s default)
        double migrateDelay = default(3);  // migration delay for negotiate the port migration 
        double portEdgeDelay = default(3); // delay to set a port in edge status in absence of BPDUs 
                                           // (according 802.1w it must be initally setted to migrateDelay)
		int packetFwdLimit  = default(500);  // maximum number of forwarded packet in a port before to trigger a bpdu transmission
		                                     // if the port forward this number a packets without send a bpdu (2 seconds), it forces
		                                     // a bpdu transmission in order to tie a possible loop                                             
                                           
        double powerOn @unit("s") = default(0s);  // Bridge Power on (become active in the network).
        
        bool showInfo = default(true); // show path cost over each bridge 
        
        @display("i=block/network2");
        
    gates:
        input lowerLayerIn[];    // see MACRelayUnit
        output lowerLayerOut[];  // see MACRelayUnit
}

