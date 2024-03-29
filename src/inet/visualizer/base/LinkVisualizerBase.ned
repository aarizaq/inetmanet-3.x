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
// This is a base module for link visualizer simple modules. It keeps track of
// active links in a network. A link between two network node is considered
// active if a packet is sent recently between the corresponding layers. Active
// links are not displayed by default, see the module parameters for how to
// enable the visualization. The actual visualization of active links is done
// in derived modules.
//
// This module subscribes for signals at the provided subscription module. When
// determining active links, it only considers packets matched by the provided
// filters. The filters use the format of cMatchExpression with the packet name
// as the default attribute.
//
// The format string can contain the following directives:
//  - %n packet name
//  - %c packet class
//
// @see ~PhysicalLinkVisualizerBase, ~DataLinkVisualizerBase, ~VisualizerBase
//
simple LinkVisualizerBase extends VisualizerBase
{
    parameters:
        string subscriptionModule = default("<root>"); // module where the visualizer subscribes for signals, root module by default

        bool displayLinks = default(false); // display arrows for active links, disabled by default

        string nodeFilter @mutable = default("*"); // which nodes are considered, matches all nodes by default
        string interfaceFilter @mutable = default("*"); // which interfaces are considered, matches all interfaces by default
        string packetFilter @mutable = default("*"); // which packets are considered, matches all packets by default

        string lineColor = default("black"); // link arrow color, black by default
        string lineStyle = default("solid"); // link arrow style (solid, dashed, dotted)
        double lineWidth = default(3); // link arrow width

        double lineShift = default(16); // line shift to avoid overlapping arrows
        string lineShiftMode = default("normal"); // determines how overlapping arrows are shifted, possible values are: none, normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // determines how arrows are clipped at the submodules

        string labelFormat = default("%n"); // determines what link data is displayed
        string labelFont = default("<default>, 8px"); // label font, automatic by default
        string labelColor = default("black"); // label color, black by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies how inactive links fade out
        double fadeOutTime @unit(s) = default(1s); // how quickly inactive links fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // animation speed while there are active links, value must be in the range (0, +inf)

        @class(LinkVisualizerBase);
}
