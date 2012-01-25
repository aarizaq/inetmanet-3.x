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

import inet.oversim.common.GlobalObserver;
import inet.oversim.common.GlobalStatistics;

import inet.oversim.common.ChurnGenerator;
//
// The SimpleUnderlay ned-file @see SimpleUnderlayConfigurator
//
// @author Stephan Krause, Bernhard Heep
//
network SimpleUnderlayNetwork
{
    parameters:
        //@display("bgb=4000,4000"); // uncomment for large canvas

    submodules:
        underlayConfigurator: SimpleUnderlayConfigurator {
            @display("p=70,130");
        }
        churnGenerator[0]: ChurnGenerator {
            @display("p=70,210,column");
        }
        globalObserver: GlobalObserver {
            @display("p=70,50");
        }
        overlayTerminal[0]: SimpleOverlayHost { @dynamic; }
}
