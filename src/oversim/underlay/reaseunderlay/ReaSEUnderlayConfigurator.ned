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

//
// @file ReaSEUnderlayConfigurator.ned
// @author Markus Mauch, Stephan Krause, Bernhard Heep, Bernhard Mueller
//

package inet.oversim.underlay.reaseunderlay;

import inet.oversim.common.UnderlayConfigurator;


//
// Configurator of the ReaSEUnderlay
//
// Sets up an IPv4 network, adds overlay nodes to
// the network in init phase periodically with the interval set by
// "initPhaseCreationInterval", adds/removes/migrates overlay nodes after init
// phase according to the given churn model.
//
simple ReaSEUnderlayConfigurator extends UnderlayConfigurator {
    parameters:
        @class(ReaSEUnderlayConfigurator);
}
