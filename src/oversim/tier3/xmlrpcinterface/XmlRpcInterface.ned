//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

package inet.oversim.tier3.xmlrpcinterface;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


//
// The main module of the XML-RPC interface
//
// @author Ingmar Baumgart
//
simple XmlRpcInterface extends BaseApp
{
    parameters:
        @class(XmlRpcInterface);
        bool activeNetwInitPhase;    // is app active in network init phase?
        bool limitAccess;   // limit access to some methods (e.g. join()) to localhost applications
        int mtu;   // maximum transmission unit
        string parser;    // name of the parser used
}


//
// Compound module for the XML-RPC interface
//
// @author Ingmar Baumgart
//
module XmlRpcInterfaceModules like ITier
{
    gates:
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        input udpIn;   // gate from the UDP layer
        input tcpIn;             // gate from the TCP layer
        output tcpOut;           // gate to the TCP layer
        output to_lowerTier;    // gate to the lower tier
        output to_upperTier;    // gate to the upper tier
        output udpOut;    // gate to the UDP layer

    submodules:
        xmlRpcInterface: XmlRpcInterface;
    connections allowunconnected:
        from_lowerTier --> xmlRpcInterface.from_lowerTier;
        to_lowerTier <-- xmlRpcInterface.to_lowerTier;
}

