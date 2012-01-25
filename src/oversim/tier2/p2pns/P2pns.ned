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

package inet.oversim.tier2.p2pns;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


//
// The main module of the P2PNS implementation
//
// @author Ingmar Baumgart
//
simple P2pns extends BaseApp
{
    parameters:
        @class(P2pns);
        bool twoStageResolution; // enable two stage name resolution (KBR/DHT)
        double keepaliveInterval @unit(s); // interval between two keeaplive pings for active connections
        double idCacheLifetime @unit(s); // idle connections in the idCache get deleted after this time
        string registerName; // register the P2PNS id under this name 
}

//
// The name cache module of the P2PNS implementation
//
// @author Ingmar Baumgart
//
simple P2pnsCache
{
    parameters:
        @display("i=block/table");
}

//
// Implementation of "P2PNS: A distributed name service for P2PSIP"
//
// @author Ingmar Baumgart
//
module P2pnsModules like ITier
{
    parameters:
        @display("i=block/segm");
    gates:
        input udpIn;    // gate from the UDP layer
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        input tcpIn;             // gate from the TCP layer
        output tcpOut;           // gate to the TCP layer
        output udpOut;    // gate to the UDP layer
        output to_lowerTier;    // gate to the lower tier
        output to_upperTier;    // gate to the upper tier

    submodules:
        p2pns: P2pns {
            @display("p=137,42");
        }
        p2pnsCache: P2pnsCache {
            @display("p=53,42");
        }
    connections allowunconnected:
        from_lowerTier --> p2pns.from_lowerTier;
        to_lowerTier <-- p2pns.to_lowerTier;
        from_upperTier --> p2pns.from_upperTier;
        to_upperTier <-- p2pns.to_upperTier;

}
