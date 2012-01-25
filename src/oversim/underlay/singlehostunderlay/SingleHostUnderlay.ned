//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

//
// @author Stephan Krause
//

package inet.oversim.underlay.singlehostunderlay;

import inet.oversim.common.ChurnGenerator;
import inet.oversim.common.GlobalObserver;
import inet.oversim.common.GlobalStatistics;


//
// The SingleHostUnderlay ned-file @see SingleHostUnderlayConfigurator
//
network SingleHostUnderlayNetwork
{
    submodules:
        underlayConfigurator: SingleHostUnderlayConfigurator {
            @display("p=70,130");
        }
        churnGenerator[0]: ChurnGenerator {
            @display("p=70,210,column");
        }        
        overlayTerminal[1]: SingleHost {
            @display("p=200,130");
        }
        zeroconfConnector: ZeroconfConnector {
            @display("p=200,50");
        }
        globalObserver: GlobalObserver {
            @display("p=70,50");
        }
}

