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

// @file NTree.ned
// @author Stephan Krause
//

package inet.oversim.overlay.ntree;

import inet.oversim.common.BaseOverlay;
import inet.oversim.common.IOverlay;


simple NTree extends BaseOverlay
{
    parameters:
        @class(NTree);
        double areaDimension; // size of the play filed
        double joinDelay @unit(s); // delay between join retries (sec)
        double pingInterval @unit(s); // Ping nodes every pingInterval
        int maxChildren; // the maximum number of childs in the laodbalancing tree
        double AOIWidth; // the size of the area of interest (i.e. subscriptionSize)
}

module NTreeModules like IOverlay
{
    gates:
        input udpIn;    // gate from the UDP layer
        output udpOut;    // gate to the UDP layer
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn;    // gate from the application
        output appOut;    // gate to the application

    submodules:
        nTree: NTree {
            parameters:
                @display("p=60,60");

        }
    connections allowunconnected:
        udpIn --> nTree.udpIn;
        udpOut <-- nTree.udpOut;
        appIn --> nTree.appIn;
        appOut <-- nTree.appOut;
}
