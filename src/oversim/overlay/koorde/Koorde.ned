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

package inet.oversim.overlay.koorde;

import inet.oversim.common.BaseOverlay;
import inet.oversim.common.IOverlay;
import inet.oversim.overlay.chord.ChordSuccessorList;


@namespace(oversim);

//
// The main module of the Koorde implementation
//
// @author Jochen Schenk, Ingmar Baumgart
//
simple Koorde extends BaseOverlay
{
    parameters:
        @class(Koorde);
        double stabilizeDelay @unit(s); // number of seconds between two stabilize algo calls
        int stabilizeRetry; // retries before neighbor considered failed
        int joinRetry; // number of join retries
        double joinDelay @unit(s); // time interval between two join tries
        volatile int successorListSize; // maximal number of nodes in successor list
        int deBruijnListSize; // maximal number of nodes in de Bruijn list
        int shiftingBits; // number of bits concurrently shifted
        double deBruijnDelay @unit(s); // number of seconds between two de Bruijn algo calls
        double fixfingersDelay @unit(s); // relic from Chord
        double checkPredecessorDelay @unit(s); // checkPredecessor interval (sec)
        bool aggressiveJoinMode; // use modified (faster) JOIN protocol
        bool extendedFingerTable; // use the extended finger table?
        int numFingerCandidates; // relic from Chord
        bool proximityRouting; // use proximity routing
        bool useOtherLookup; // flag which is indicating that the optimization other lookup is enabled
        bool useSucList; // flag which is indicating that the optimization using the successorlist is enabled
        bool setupDeBruijnBeforeJoin; // if true, first setup the de bruijn node using the bootstrap node and than join the ring
        bool setupDeBruijnAtJoin; // if true, join the ring and setup the de bruijn node using the bootstrap node in parallel
        bool mergeOptimizationL1; // enable merge optimization OPL1
        bool mergeOptimizationL2; // activate merge optimization OPL2
        bool mergeOptimizationL3; // activate merge optimization OPL3
        bool mergeOptimizationL4; // activate merge optimization OPL4
        bool memorizeFailedSuccessor;

}

//
// Implementation of the Koorde DHT overlay as described in
// "Koorde: A simple degree-optimal distributed hash table"
// by M. Frans Kaashoek and David R. Karger,. published in
// Proceedings of the 2nd International Workshop on
// Peer-to-Peer Systems (IPTPS '03).
//
// @author Jochen Schenk, Ingmar Baumgart
//
module KoordeModules like IOverlay
{
    gates:
        input udpIn; // gate from the UDP layer
        output udpOut; // gate to the UDP layer
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn; // gate from the application
        output appOut; // gate to the application

    submodules:
        koorde: Koorde {
            parameters:
                @display("p=60,60;i=block/circle");
        }
        successorList: ChordSuccessorList {
            parameters:
                @display("p=136,60;i=block/table");
        }
    connections allowunconnected:
        udpIn --> koorde.udpIn;
        udpOut <-- koorde.udpOut;
        appIn --> koorde.appIn;
        appOut <-- koorde.appOut;

}

