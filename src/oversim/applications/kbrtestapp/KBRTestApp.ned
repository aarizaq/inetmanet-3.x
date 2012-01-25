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

package inet.oversim.applications.kbrtestapp;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


//
// Simple module of the KBR test application
//
// @author Ingmar Baumgart, Bernhard Heep
//
simple KBRTestApp extends BaseApp
{
    parameters:
        @class(KBRTestApp);
        double testMsgInterval @unit(s);    // interval for sending test messages
        int testMsgSize @unit(B);  // payload size of the test message in bytes
        int msgHandleBufSize;    // how many MsgHandles to store in circular buffer
        bool lookupNodeIds;    // lookup only existing nodeIDs
        bool activeNetwInitPhase;    // is app active in network init phase?
        bool kbrOneWayTest;  // enable sending of one way test messages
        bool kbrRpcTest;     // enable sending of RPC test calls
        bool kbrLookupTest;  // enable periodic test lookups
        double failureLatency @unit(s); // this latency is recorded for failed lookups and RPCs 
        bool onlyLookupInoffensiveNodes; // if true only search for inoffensive nodes (use together with lookupNodeIds)
}

//
// Compound module for a simple test application using the KBR interface
//
// @author Ingmar Baumgart, Bernhard Heep
//
module KBRTestAppModules like ITier
{
    parameters:
        @display("i=block/segm");
    gates:
        input udpIn;     // gate from the UDP layer
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        input trace_in;    // gate for trace file commands
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        output udpOut;    // gate to the UDP layer
        output to_lowerTier;    // gate to the lower tier
        output to_upperTier;    // gate to the upper tier

    submodules:
        kbrTestApp: KBRTestApp;
    connections allowunconnected:
        from_lowerTier --> kbrTestApp.from_lowerTier;
        to_lowerTier <-- kbrTestApp.to_lowerTier;
        udpIn --> kbrTestApp.udpIn;
        udpOut <-- kbrTestApp.udpOut;
        trace_in --> kbrTestApp.trace_in;
}

