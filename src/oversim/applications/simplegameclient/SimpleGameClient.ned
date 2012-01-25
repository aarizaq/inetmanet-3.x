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

// @file SimpleGameClient.ned
// @author Helge Backhaus
// @author Stephan Krause
//

package inet.oversim.applications.simplegameclient;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


simple SimpleGameClient extends BaseApp
{
    parameters:
        @class(SimpleGameClient);
        double movementRate;      // movement updates per second
        double areaDimension;     // movement range from [0.0, 0.0] to [areaDimension, areaDimension]
        bool useScenery;           // generate scenery for simulated area
                                    // (very slow for large areas, should be false except when used with realworld/simplecraft)
        double movementSpeed @unit(mps);     // movement speed in m/s
        string movementGenerator;  // responsible for different movement models
                                   // (randomRoaming, groupRoaming, greatGathering, realWorldRoaming)
        int groupSize;         // clients per group
        int mtu;               // maximum transmission unit
}

module SimpleGameClientModules like ITier
{
    gates:
        input udpIn;               // gate from the UDP layer
        input from_lowerTier;         // gate from the lower tier
        input from_upperTier;         // gate from the upper tier
        input trace_in;               // gate for trace file commands
        input tcpIn;                  // gate from the TCP layer
        output tcpOut;                // gate to the TCP layer
        output udpOut;                // gate to the UDP layer
        output to_lowerTier;          // gate to the lower tier
        output to_upperTier;          // gate to the upper tier

    submodules:
        simpleGameClient: SimpleGameClient;
    connections allowunconnected:
        from_lowerTier --> simpleGameClient.from_lowerTier;
        to_lowerTier <-- simpleGameClient.to_lowerTier;
        trace_in --> simpleGameClient.trace_in;
}
