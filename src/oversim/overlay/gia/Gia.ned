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

// @file Gia.ned
// @author Robert Palmer, Bernhard Heep

package inet.oversim.overlay.gia;

import inet.oversim.common.BaseOverlay;
import inet.oversim.common.IOverlay;


// The main module of the GIA implementation
// @author Robert Palmer
simple Gia extends BaseOverlay
{
    parameters:
        @class(Gia);
        int maxNeighbors; // maximum number of neighbors
        int minNeighbors; // minimum number of neighbors
        double maxTopAdaptionInterval @unit(s); // maximum topology adaption interval
        int topAdaptionAggressiveness; // topology adaption aggressiveness
        double maxLevelOfSatisfaction; // maximum level of satisfaction
        int maxHopCount; // maximum TTL for sent messages
        double messageTimeout @unit(s); // message timeout
        double neighborTimeout @unit(s); // neighbor timeout
        double sendTokenTimeout @unit(s); // token timeout
        double tokenWaitTime @unit(s); // delay when sending new token        
        double keyListDelay @unit(s); // delay when sending new key list to our neighbors
        double updateDelay @unit(s); // delay between two update messages
        bool outputNodeDetails; // output node details
        bool optimizeReversePath; // use optimized reverse path?
}

//
// Module which handles all current neighbors of the node. Contains capacity,
// connection degree, sent and received token count and the keylist of all
// neighbors
// @author Robert Palmer
simple GiaNeighbors
{
    parameters:
        @display("i=block/table");
}

//
// Module for visualizing the keylist
// @author Robert Palmer
simple GiaKeyListModule
{
    parameters:
        @display("i=block/table");
}

//
// Module for sending new tokens to direct neighbors
// @author Robert Palmer
simple GiaTokenFactory
{
    parameters:
        @display("i=block/table");
}

//
// Implementation of the Gia overlay as described in
// "Making Gnutella-like P2P Systems Scalable"
// by Yatin Chawathe, Sylvia Ratnasamy, Lee Breslau, Nick Lanham
// and Scott Shenker, published in "In Proc. ACM SIGCOMM (Aug. 2003)".
// @author Robert Palmer
//
module GiaModules like IOverlay
{
    gates:
        input udpIn;  // gate from UDP
        output udpOut;   // gate to UDP
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn;  // gate from application
        output appOut;   // gate to application

    submodules:
        gia: Gia {
            parameters:
                @display("p=60,60;i=block/circle");
        }
        keyListModule: GiaKeyListModule {
            parameters:
                @display("p=150,60;i=block/table");
        }
        neighbors: GiaNeighbors {
            parameters:
                @display("p=240,60;i=block/table");
        }
        tokenFactory: GiaTokenFactory {
            parameters:
                @display("p=330,60;i=block/table");
        }
    connections allowunconnected:
        udpIn --> gia.udpIn; // connect udpIn with gia.udpIn
        udpOut <-- gia.udpOut;     // connect to_upd with gia.udpOut
        appIn --> gia.appIn;   // connect appIn with gia.appIn
        appOut <-- gia.appOut;       // connect appOut with gia.appOut

}
