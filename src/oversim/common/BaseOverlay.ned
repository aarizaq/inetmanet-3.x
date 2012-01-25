//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

package inet.oversim.common;

//
// Base class for overlay implementations
//
// @author Ingmar Baumgart, Bernhard Heep, Stephan Krause, Sebastian Mies
//
simple BaseOverlay extends BaseRpc
{
    parameters:
        @class(BaseOverlay);
        @display("i=block/circle");
        int localPort; // UDP port for overlay messages
        bool debugOutput; // enable debug output
        int keyLength; // overlay key length in bits
        string nodeId; // optional nodeId as string in hexadecimal notation 
        bool useCommonAPIforward; // enable CommonAPI forward() calls
        bool drawOverlayTopology; // draw arrow to successor node?
        int hopCountMax; // maximum number of overlay hops
        int recNumRedundantNodes; // numRedundantNodes for recursive routing
        bool joinOnApplicationRequest; // only join the overlay on application request
        bool collectPerHopDelay; // delay statistics for single hops
        bool routeMsgAcks; // use RPCs for route messages
        int lookupRedundantNodes; // number of next hops in each step
        int lookupParallelPaths; // number of parallel paths
        int lookupParallelRpcs; // number of nodes to ask in parallel      
        bool lookupVerifySiblings; //true, if siblings need to be authenticated with a ping
        bool lookupMajoritySiblings; // true, if sibling candidates are selected by a majority decision if using parallel paths  
        bool lookupMerge; // true, if parallel Rpc results should be merged
        bool lookupFailedNodeRpcs; // communicate failed nodes
        bool lookupStrictParallelRpcs; // limited the number of concurrent rpcs to parameter parallelRpcs
        bool lookupUseAllParallelResponses; // merge all parallel responses from earlier steps
        bool lookupNewRpcOnEveryTimeout; // send a new RPC immediately after an RPC timeouts
        bool lookupNewRpcOnEveryResponse; // send a new RPC after every response, even if there was no progress
        bool lookupFinishOnFirstUnchanged; // finish lookup, if the last pending RPC returned without progress    
        bool lookupVisitOnlyOnce; // if true, the same node is never asked twice during a single lookup
        bool lookupAcceptLateSiblings; // if true, a FindNodeResponse with sibling flag set is always accepted, even if it is from a previous lookup step
        string routingType; // default routing mode (iterative, semi-recursive,...)
        bool rejoinOnFailure; // rejoin after loosing connection to the overlay?
        bool sendRpcResponseToLastHop; // needed by KBR protocols for NAT support
        bool recordRoute; // record visited hops on route 

        bool dropFindNodeAttack; // if node is malicious, it tries a findNode attack
        bool isSiblingAttack; // if node is malicious, it tries a isSibling attack
        bool invalidNodesAttack; // if node is malicious, it tries a invalidNode attack
        bool dropRouteMessageAttack; // if node is malicious, it drops all received BaseRouteMessages
        bool measureAuthBlock; // if true, measure the overhead of signatures in rpc messages
        bool restoreContext; // if true, a node rejoins with its old nodeId and malicious state

    gates:
        input udpIn; // gate from the UDP layer
        output udpOut; // gate to the UDP layer
        input tcpIn; // gate from the TCP layer
        output tcpOut; // gate to the TCP layer
        input appIn; // gate from the application
        output appOut; // gate to the application
        input direct_in; // gate for sendDirect
}

