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


package inet.oversim.overlay.myoverlay;

import inet.oversim.common.BaseOverlay;
import inet.oversim.common.IOverlay;

simple MyOverlay extends BaseOverlay
{
    parameters:
        @class(MyOverlay);
        bool enableDrops;       // do we enable random packet drops for testing?
        double dropChance;      // chance that we drop a packet
}

module MyOverlayModules like IOverlay
{
    gates:
        input udpIn;   // gate from the UDP layer
        output udpOut;    // gate to the UDP layer
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn;   // gate from the application
        output appOut;    // gate to the application

    submodules:
        overlay: MyOverlay;

    connections allowunconnected:
        udpIn --> overlay.udpIn;
        udpOut <-- overlay.udpOut;
        appIn --> overlay.appIn;
        appOut <-- overlay.appOut;
}
