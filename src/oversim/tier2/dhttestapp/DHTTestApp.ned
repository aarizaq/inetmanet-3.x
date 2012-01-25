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

package inet.oversim.tier2.dhttestapp;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


//
// Simple module of the DHT test application
//
// @author Gregoire Menuel, Ingmar Baumgart
//
simple DHTTestApp extends BaseApp
{
    parameters:
        @class(DHTTestApp);
        double testInterval @unit(s);    // time interval for DHT operations (put/get/modify)
        int testTtl;      // TTL for stored test records
        bool activeNetwInitPhase;    // send messages when network is in init phase?
        bool p2pnsTraffic; // model p2pns application traffic
}


//
// Compound module for a simple DHT test application
//
// @author Gregoire Menuel, Ingmar Baumgart
//
module DHTTestAppModules like ITier
{
    gates:
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        input udpIn;    // gate from the UDP layer
        input trace_in;    // gate for trace file commands
        input tcpIn;             // gate from the TCP layer
        output tcpOut;           // gate to the TCP layer
        output to_lowerTier;    // gate to the lower tier
        output to_upperTier;    // gate to the upper tier
        output udpOut;    // gate to the UDP layer

    submodules:
        dhtTestApp: DHTTestApp;
    connections allowunconnected:
        from_lowerTier --> dhtTestApp.from_lowerTier;
        to_lowerTier <-- dhtTestApp.to_lowerTier;
        trace_in --> dhtTestApp.trace_in;
}

