//
// Copyright (C) 2010 Institut fuer Telematik, Karlsruher Institut fuer Technologie (KIT)
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

///
// @file ReaSEUnderlay.ned
// @author Bernhard Mueller
// 

package inet.oversim.underlay.reaseunderlay;


import inet.oversim.common.ChurnGenerator;
import inet.oversim.common.GlobalObserver;
import inet.oversim.common.inet_fiberline;


//
// The InetUnderlay ned-file @see ReaSEUnderlayConfigurator
//
module ReaSEUnderlayNetworkBase
{
    parameters:
        string overlayType;    // the overlay used in the simulation (for overlayBackboneRouters)
        string tier1Type;    // the application on top of the overlay used in the simulation (for overlayBackboneRouters)
        string tier2Type;    // the module type on top of tier 1 (for overlayBackboneRouters)
        string tier3Type;    // the module type on top of tier 2 (for overlayBackboneRouters)
        @display("bgl=49;bgb=631,441");
    submodules:
        underlayConfigurator: ReaSEUnderlayConfigurator {
            @display("p=70,137");
        }
        globalObserver: GlobalObserver {
            parameters:
                @display("i=block/control;p=70,50");
        }
        churnGenerator[0]: ChurnGenerator {
            @display("p=70,224,column");
        }
        TerminalConnector: ConnectReaSE {
            @display("p=70,309");
        }
        RUNetworkConfigurator: RUNetworkConfigurator {
            @display("p=70,386");
        }
    connections:

}

