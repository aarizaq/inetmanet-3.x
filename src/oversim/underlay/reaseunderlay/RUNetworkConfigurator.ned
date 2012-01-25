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
// @file RUNetworkConfigurator.ned
// @author Bernhard Mueller et.al.
//	Modification of TGMNetworkConfigurator.h provided by ReaSE.
//
package inet.oversim.underlay.reaseunderlay;

//
// This simple module is responsible for configuration of the simulated
// topology, i.e. assignment of IP addresses to all nodes of the topology. 
// Furthermore, routing tables have to be filled and default routing 
// paths must be created.
// Routing is separated into Intra-AS and Inter-AS routing.
//
simple RUNetworkConfigurator {
    parameters:
        @display("i=block/network2");
}
