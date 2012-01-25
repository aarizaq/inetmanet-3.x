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

// @file GlobalCoordinator.ned
// @author Helge Backhaus
//

package inet.oversim.applications.simplegameclient;

import inet.oversim.common.IGlobalFunctions;

simple GlobalCoordinator
{
    parameters:
        double seed;      // seed for scenery generation
        string Hotspots;  // For hotspotRoaming, definition of the hotspots
        double HotspotStayTime @unit(s); // For hotspotRoaming, how long to stay in a hotspot
        @display("i=block/app");
}

simple ConnectivityProbeApp
{
    parameters:
        double connectivityProbeIntervall @unit(s);    // interval after which to check connectivity
        @display("i=block/network2");
}


module GlobalCoordinatorModules like IGlobalFunctions
{
    parameters:
        double connectivityProbeIntervall @unit(s);    // interval after which to check connectivity
    submodules:
        coordinator: GlobalCoordinator;
        probe: ConnectivityProbeApp {
            parameters:
                connectivityProbeIntervall = connectivityProbeIntervall;
        }
}
