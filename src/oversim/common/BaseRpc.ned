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
// Base module for simple RPC communication
//
// @author Sebastien Mies, Ingmar Baumgart, Bernhard Heep, Gregoire Menuel
//
simple BaseRpc
{
    parameters: 
        double rpcUdpTimeout @unit(s); // default timeout value for direct RPCs
        double rpcKeyTimeout @unit(s); // default timeout value for routed RPCs
        bool optimizeTimeouts;        // calculate timeouts from measured RTTs
                                       // and network coordinates
        bool rpcExponentialBackoff;    // if true, doubles the timeout for
                                       // every retransmission
}
