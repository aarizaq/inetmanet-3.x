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

package inet.oversim.applications.realworldtestapp;

//
// Simple module of the real wolrd test application
//
// @author Bernhard Heep
//
simple RealWorldTestApp
{
    parameters:
        bool debugOutput;    // enable debug output
        @display("i=block/app");
    
    gates:
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        input direct_in;    // gate for sendDirect
        input trace_in;    // gate for trace file commands
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        output to_lowerTier;    // gate to the lower tier
        output to_upperTier;    // gate to the upper tier
}


//
// Compound module for a simple real world test application 
//
// @author Bernhard Heep
//
module RealWorldTestAppModules
{
    gates:
        input udpIn;     // gate from the UDP layer
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        input trace_in;    // gate for trace file commands
        output udpOut;    // gate to the UDP layer
        output to_lowerTier;    // gate to the lower tier
        output to_upperTier;    // gate to the upper tier

    submodules:
        realWorldTestApp: RealWorldTestApp;

    connections allowunconnected:
        from_lowerTier --> realWorldTestApp.from_lowerTier;
        to_lowerTier <-- realWorldTestApp.to_lowerTier;
        from_upperTier --> realWorldTestApp.from_upperTier;
        to_upperTier <-- realWorldTestApp.to_upperTier;
        trace_in --> realWorldTestApp.trace_in;
}

