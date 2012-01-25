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

package inet.oversim.applications.dht;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


//
// Simple module of the DHT
//
// @author Gregoire Menuel, Ingmar Baumgart
//
simple DHT extends BaseApp
{
    parameters:
        @class(DHT);
        int numReplica;  // number of replica for stored data records
        int numGetRequests; // number of queried replica for get requests
        double ratioIdentical; // ratio of identical replica needed for a valid result
        bool secureMaintenance; // use a secure maintenance algorithm based on majority decisions
        bool invalidDataAttack; // if node is malicious, it tries a invalidData attack
        bool maintenanceAttack; // if node is malicious, it tries a maintenance attack
}

//
// Data storage module of the DHT
//
// @author Gregoire Menuel, Ingmar Baumgart
//
simple DHTDataStorage
{
    parameters:
        @display("i=block/table");
}

//
// Compound module for a simple DHT using the KBR interface
//
// @author Gregoire Menuel, Ingmar Baumgart
//
module DHTModules like ITier
{
    parameters:
        @display("i=block/segm");
    gates:
        input udpIn;    // gate from the UDP layer
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        output udpOut;    // gate to the UDP layer
        output to_lowerTier;    // gate to the lower tier
        output to_upperTier;    // gate to the upper tier

    submodules:
        dht: DHT;
        dhtDataStorage: DHTDataStorage;
    connections allowunconnected:
        from_lowerTier --> dht.from_lowerTier;
        to_lowerTier <-- dht.to_lowerTier;
        from_upperTier --> dht.from_upperTier;
        to_upperTier <-- dht.to_upperTier;

}

