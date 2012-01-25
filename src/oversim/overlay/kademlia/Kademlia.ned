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

package inet.oversim.overlay.kademlia;

import inet.oversim.common.BaseOverlay;
import inet.oversim.common.IOverlay;


//
// The main module of the Kademlia implementation
//
// @author Sebastian Mies, Ingmar Baumgart, Bernhard Heep
//
simple Kademlia extends BaseOverlay
{
    parameters:
        @class(Kademlia);
        double minSiblingTableRefreshInterval @unit(s); // siblingTable refresh delay
        double minBucketRefreshInterval @unit(s); // bucket refresh delay
        double siblingPingInterval @unit(s); // interval in which all siblings get pinged (0 to disable)
        int maxStaleCount; // number of timeouts before node removal
        int k; // number of paths (size of k-bucket)
        int b; // network diameter O(log_{2^(b)})
        int s; // number of siblings
        bool exhaustiveRefresh; // if true, use exhaustive-iterative lookups to refresh buckets
        bool pingNewSiblings; // ping new unknown siblings?
        bool secureMaintenance; // if true, ping not authenticated nodes before adding them to a bucket
        bool newMaintenance;

        bool enableReplacementCache; // enables the replacement cache to store nodes if a bucket is full
        bool replacementCachePing; // ping the least recently used node in a full bucket, when a node is added to the replacement cache
        int replacementCandidates; // maximum number of candidates in the replacement cache for each bucket
        int siblingRefreshNodes; // number of redundant nodes for exhaustive sibling table refresh lookups (0 = numRedundantNodes)
        int bucketRefreshNodes; // number of redundant nodes for exhaustive bucket refresh lookups (0 = numRedundantNodes)

        // R/Kademlia
        bool activePing; // ping new neighbors?
        bool proximityRouting; // enable proximity routing (recursive only)
        bool proximityNeighborSelection; // enable proximity neighbor selection
        bool altRecMode; // use source-routing mode instead of direct mode
}

//
// Implementation of the Kademlia DHT overlay as described in
// "Kademlia: A peer-to-peer information system based on the XOR metric"
// by P. Maymounkov and D. Mazieres, published in "In Proceedings of IPTPS02"
//
// @author Sebastian Mies, Ingmar Baumgart, Bernhard Heep
//
module KademliaModules like IOverlay
{
    gates:
        input udpIn;    // gate from the UDP layer
        output udpOut;  // gate to the UDP layer
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn;    // gate from the application
        output appOut;  // gate to the application

    submodules:
        kademlia: Kademlia {
            parameters:
                @display("p=60,60;i=block/circle");

        }
    connections allowunconnected:
        udpIn --> kademlia.udpIn;
        udpOut <-- kademlia.udpOut;
        appIn --> kademlia.appIn;
        appOut <-- kademlia.appOut;
}
