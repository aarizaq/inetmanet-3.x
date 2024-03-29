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
// This is a base module for link break visualizer simple modules. It keeps
// track of link breaks in a network. Link breaks are not displayed by
// default, see module parameters for how to enable the visualization. The
// actual visualization of link breaks is done in derived modules.
//
// This module subscribes for NF_LINK_BREAK on the module determined by the
// subscriptionModule parameter. If the visualizer receives a link break signal,
// then it stores the broken link and starts the link break visualization.
//
// @see ~LinkBreakCanvasVisualizer, ~LinkBreakOsgVisualizer, ~ILinkBreakVisualizer, ~VisualizerBase
//
simple LinkBreakVisualizerBase extends VisualizerBase
{
    parameters:
        string subscriptionModule = default("<root>"); // module where the visualizer subscribes for link break signals, root module by default

        bool displayLinkBreaks = default(false); // display icons for link breaks, disabled by default

        string nodeFilter @mutable = default("*"); // which network nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // which interfaces are considered, matches all interfaces by default
        string packetFilter @mutable = default("*"); // which packets are considered, matches all packets by default

        string icon = default("status/stop");
        double iconTintAmount = default(0);
        string iconTintColor = default("");

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies how inactive link breaks fade out
        double fadeOutTime @unit(s) = default(1s); // how quickly link breaks fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // animation speed while there are link breaks, value must be in the range (0, +inf)

        @class(LinkBreakVisualizerBase);
}
