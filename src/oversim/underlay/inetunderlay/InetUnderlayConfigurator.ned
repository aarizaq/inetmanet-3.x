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

package inet.oversim.underlay.inetunderlay;

import inet.oversim.common.UnderlayConfigurator;


//
// Configurator of the IPv4Underlay
//
// Sets up an IPv4Network (backbone and access routers), adds overlay nodes to
// the network in init phase periodically with the interval set by
// "initPhaseCreationInterval", adds/removes/migrates overlay nodes after init
// phase according to the given churn model.
//
// @author Markus Mauch
//
simple InetUnderlayConfigurator extends UnderlayConfigurator
{
    parameters:
        @class(InetUnderlayConfigurator);
        string startIPv4; // IP configuration starts with this IP
        string startIPv6; 
        string gatewayIP; // IP of the gateway (if an outRouter is used)
        bool useIPv6Addresses = default(false);
}

