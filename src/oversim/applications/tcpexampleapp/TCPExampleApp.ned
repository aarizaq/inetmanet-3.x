//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
// @author Antonio Zea
//

package inet.oversim.applications.tcpexampleapp;

import inet.oversim.common.ITier;
import inet.oversim.common.BaseApp;


simple TCPExampleApp extends BaseApp
{
    parameters:
        @class(TCPExampleApp);
        int sendPeriod @unit(s) = default(30s);   // how much time to wait between sending packets
        int numToSend = default(0);             // how many packets will we send at the same time?
        int largestKey = default(10);            // largest key we can pick
}

module TCPExampleAppModules like ITier
{
    gates:
        input udpIn;             // gate from the UDP layer
        output udpOut;           // gate to the UDP layer
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        output to_lowerTier;     // gate to the lower tier
        output to_upperTier;     // gate to the upper tier
        input trace_in;          // gate for trace file commands
        input tcpIn;             // gate from the TCP layer
        output tcpOut;           // gate to the TCP layer

    submodules:
        application: TCPExampleApp;
    connections allowunconnected:
        from_lowerTier --> application.from_lowerTier;
        to_lowerTier <-- application.to_lowerTier;
        udpIn --> application.udpIn;
        udpOut <-- application.udpOut;
        tcpIn --> application.tcpIn;
        tcpOut <-- application.tcpOut;
        trace_in --> application.trace_in;
}
