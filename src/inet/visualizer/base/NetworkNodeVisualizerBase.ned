//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.visualizer.base;

//
// This is a base module for node visualizer simple modules. The actual
// visualization of network nodes is done in derived modules.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeOsgVisualizer, ~INetworkNodeVisualizer, ~VisualizerBase
//
simple NetworkNodeVisualizerBase extends VisualizerBase
{
    parameters:
        string nodeFilter @mutable = default("*"); // determines which network nodes are displayed, all nodes by default

        double annotationSpacing = default(4);
        double placementPenalty = default(10);

        @class(NetworkNodeVisualizerBase);
}
