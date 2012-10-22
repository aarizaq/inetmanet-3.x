//
// HostAutoConfigurator - automatically assigns IP addresses and sets up routing table
// Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
// Copyright (C) 2010 Alfonso Ariza 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
 
package inet.networklayer.autorouting.ipv4;
 
//
// HostAutoConfigurator automatically assigns IP addresses and sets up routing table.
//
simple HostAutoConfigurator2
{
    parameters:
        bool debug = default(false);                 // emit debug messages?
        string interfaces = default("wlan0 eth0 ppp0");         // list of interfaces to autoassign addresses to, separated by a single space characters
        string defaultInterface = default("eth0"); // only if a node has been defined like   isDefaultRoute=true for all nodes
        string defaultAddressInterface = default("eth0"); // only if a node has been defined like   isDefaultRoute=true, is the interface that will be used for search the ip address, only for the default
        bool isDefaultRoute = default(false); // we use this route like default route for all address
        string addressBaseList = default("192.168.0.0 192.161.1.0 192.161.2.0");         // list of base address to autoassign 
        string addressMask = default("255.255.0.0 255.255.255.0 255.255.255.0");         // list of interfaces to autoassign addresses to, separated by a single space characters	
		@display("i=block/cogwheel_s");
}

