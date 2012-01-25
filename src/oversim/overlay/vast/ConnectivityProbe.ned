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

// @file ConnectivityProbe.ned
// @author Helge Backhaus
//

package inet.oversim.overlay.vast;

import inet.oversim.common.IGlobalFunctions;
import inet.oversim.applications.simplegameclient.GlobalCoordinator;

simple ConnectivityProbe
{
    parameters:
        double connectivityProbeIntervall @unit(s); // interval after which to check connectivity
        double visualizeNetworkIntervall @unit(s);
        bool plotConnections;
        bool plotMissing;
        @display("i=block/network2");
}

module GlobalVastModules like IGlobalFunctions
{
    parameters:
        double connectivityProbeIntervall @unit(s); // interval after which to check connectivity
        double visualizeNetworkIntervall @unit(s);
        bool plotConnections;
        bool plotMissing;
        double seed;                                   // seed for scenery generation

    submodules:
        coordinator: GlobalCoordinator {
            parameters:
                seed = seed;
                @display("i=block/app");
        }
        probe: ConnectivityProbe {
            parameters:
                connectivityProbeIntervall = connectivityProbeIntervall;
                visualizeNetworkIntervall = visualizeNetworkIntervall;
                plotConnections = plotConnections;
                plotMissing = plotMissing;
                @display("i=block/network2");
        }
}
