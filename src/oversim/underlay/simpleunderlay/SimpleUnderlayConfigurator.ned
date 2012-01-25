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

package inet.oversim.underlay.simpleunderlay;

import inet.oversim.common.UnderlayConfigurator;


//
// Configurator of the SimpleUnderlay
//
// Sets up a SimpleNetwork, adds overlay nodes to the network in init phase
// periodically with the interval set by "initPhaseCreationInterval",
// adds/removes/migrates overlay nodes after init phase according to given
// churn model.
//
simple SimpleUnderlayConfigurator extends UnderlayConfigurator
{
    parameters:
        @class(SimpleUnderlayConfigurator);
        double fieldSize; // maximum x/y-coordinate for nodes
        string nodeCoordinateSource; // name of xml-file with coordinates of nodes
        int sendQueueLength @unit(B); // send-queue length in bytes (0 = infinite)
        bool fixedNodePositions; // put nodes on fixed coordiantes in playground
        bool useIPv6Addresses;
}
