//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

// @file UnderlayConfigurator.ned
// @author Ingmar Baumgart
//

package inet.oversim.common;

//
// UnderlayConfigurator base module
//
simple UnderlayConfigurator
{
    parameters:
        string terminalTypes; // list of terminal types
        string churnGeneratorTypes; // a list of churn generators to be used
        double transitionTime @unit(s); // time to wait before measuring after init phase is finished
        double measurementTime @unit(s); // duration of the simulation after init and transition phase
        double gracefulLeaveDelay @unit(s); // time between prekill and removal from overlay
        double gracefulLeaveProbability; // probability of gracefull leave        
        @display("i=block/cogwheel");
}
