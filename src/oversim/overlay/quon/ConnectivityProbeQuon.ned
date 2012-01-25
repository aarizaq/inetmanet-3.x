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

// @file ConnectivityProbeQuon.ned
// @author Helge Backhaus
// @author Stephan Krause
//

package inet.oversim.overlay.quon;

import inet.oversim.applications.simplegameclient.GlobalCoordinator;
import inet.oversim.common.IGlobalFunctions;

simple ConnectivityProbeQuon
{
    parameters:
        double connectivityProbeIntervall @unit(s);    // interval after which to check connectivity
        double visualizeNetworkIntervall @unit(s);
        double startPlotTime @unit(s);                 // when to start plotting graphs, 0 = accross the whole simulation
        double plotPeriod @unit(s);                    // how long graphs are plotted from startPlotTime on, 0 = until simulation ends
        bool plotConnections;
        bool plotBindings;
        bool plotMissing;
        @display("i=block/network2");
}

module GlobalQuonModules like IGlobalFunctions
{
    parameters:
        double connectivityProbeIntervall @unit(s);    // interval after which to check connectivity
        double visualizeNetworkIntervall @unit(s);
        double startPlotTime @unit(s);                 // when to start plotting graphs, 0 = accross the whole simulation
        double plotPeriod @unit(s);                    // how long graphs are plotted from startPlotTime on, 0 = until simulation ends
        bool plotConnections;
        bool plotBindings;
        bool plotMissing;
        double seed;

    submodules:
        coordinator: GlobalCoordinator {
            parameters:
                seed = seed;
                @display("i=block/app");
        }
        probe: ConnectivityProbeQuon {
            parameters:
                connectivityProbeIntervall = connectivityProbeIntervall;
                visualizeNetworkIntervall = visualizeNetworkIntervall;
                startPlotTime = startPlotTime;
                plotPeriod = plotPeriod;
                plotConnections = plotConnections;
                plotBindings = plotBindings;
                plotMissing = plotMissing;
                @display("i=block/network2");
        }
}
