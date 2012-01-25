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

// @file Vast.ned
// @author Helge Backhaus
//

package inet.oversim.overlay.vast;

import inet.oversim.common.BaseOverlay;
import inet.oversim.common.IOverlay;


simple Vast extends BaseOverlay
{
    parameters:
        @class(Vast);
        double areaDimension; // movement range from [0.0, 0.0] to [areaDimension, areaDimension]
        bool debugVastOutput; // enable voronoi diagram debug information
        double AOIWidth; // this nodes area of interest
        double stockListSize; // size of the stockList
        double joinTimeout @unit(s); // retry join timeout in seconds
        double pingTimeout @unit(s); // ping neighbor status in seconds
        double discoveryIntervall @unit(s); // asks stock neighbors for their neighborslist
        double criticalCheckIntervall @unit(s); // checks intervall for NeighborLevel for preventing partitions
        double criticalThreshold; // threshold to activate backup operation
}

module OverlayVast like IOverlay
{
    gates:
        input udpIn; // gate from the UDP layer
        output udpOut; // gate to the UDP layer
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn; // gate from the application
        output appOut; // gate to the application

    submodules:
        vast: Vast {
            parameters:
                @display("p=60,60");

        }
    connections allowunconnected:
        udpIn --> vast.udpIn;
        udpOut <-- vast.udpOut;
        appIn --> vast.appIn;
        appOut <-- vast.appOut;
}
