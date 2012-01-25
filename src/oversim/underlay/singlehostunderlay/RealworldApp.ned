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

//
// Applicaton that communicates with a realworld application via a socket
//
simple RealworldApp
{
    parameters:
        int mtu;    // maximum transmission unit of the underlaying network
        string parser;    // name of the PacketParser to use

    gates:
        input udpIn;
        input from_lowerTier;
        input from_upperTier;    // incoming gates from udp, the lower tier and from the upper tier
        output udpOut;
        output to_lowerTier;
        output to_upperTier;    // outgoing gates to udp, the lower tier and to the upper tier
}


