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

package inet.oversim.applications.giasearchapp;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


//
// Test application for testing GIA search-protocol
//
// @author Robert Palmer, Bernhard Heep
//
simple GIASearchApp extends BaseApp
{
    parameters:
        @class(GIASearchApp);
        double messageDelay @unit(s);    // interval for sending route or search messages
        bool randomNodes;    // use random destination nodes or only nodes from GlobalNodeList
        int maximumKeys;    // maximum number of keys for this application
        int maxResponses;    // maximum number of responses per search message
        bool activeNetwInitPhase;    // send messages when network is in init phase?
}


//
// Compound module for a search application using the GIA search interface
//
// @author Bernhard Heep
//
module GIASearchAppModules like ITier
{
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
        giaSearchApp: GIASearchApp;
    connections allowunconnected:
        from_lowerTier --> giaSearchApp.from_lowerTier;
        to_lowerTier <-- giaSearchApp.to_lowerTier;
        trace_in --> giaSearchApp.trace_in;
}

