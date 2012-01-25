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


package inet.oversim.underlay.singlehostunderlay;

import inet.oversim.common.UnderlayConfigurator;


//
// Configurator of the SingleHostUnderlay
//
// Configures a single Host (with a connection to a real network)
//
// @author Stephan Krause, Ingmar Baumgart
//
simple SingleHostUnderlayConfigurator extends UnderlayConfigurator
{
    parameters:
        @class(SingleHostUnderlayConfigurator);
        string nodeIP; // IP of the node
        string nodeInterface; // interface name 
        string stunServer; // name of a STUN server (optional)
        string bootstrapIP; // IP of an overlay node for bootstrapping
        int bootstrapPort; // Port of the bootstrap node
}
