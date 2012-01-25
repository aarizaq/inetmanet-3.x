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

// @file Quon.ned
// @author Helge Backhaus
// @author Stephan Krause
//

package inet.oversim.overlay.quon;

import inet.oversim.common.BaseOverlay;
import inet.oversim.common.IOverlay;


simple Quon extends BaseOverlay
{
    parameters:
        @class(Quon);
        double areaDimension; // movement range from [0.0, 0.0] to [areaDimension, areaDimension]
        double joinTimeout @unit(s); // retry join timeout in seconds
        double deleteTimeout @unit(s); // delete dead/offline neighbors from list of known neighbors after # seconds
        double aliveTimeout @unit(s); // ping neighbor status in seconds
        double contactBackupIntervall @unit(s); // contact backup neighbors every # seconds
        int numBackups; // the number of binding beackups per quadrant
        double connectionLimit; // maximum # of neighbor connections (for use with dynamic AOI)
        double minAOIWidth; // minimum area of interest size (for use with dynamic AOI)
        double AOIWidth; // this nodes area of interest (maximum area of interest when dynamic AOI is used)
        double AOIBuffer; // increase AOI by fixed amount
        double AOIAdaptionSensitivity; // how much the AOI can be adapted in one step (for use with dynamic AOI) (max=1)
        double AOIGossipSensitivity; // how much the AOI adaption can be influenced by neighbors (for use with dynamic AOI) (max=1)
        bool AOIAdaptLinear; // use linear AOI adaption? (for use with dynamic AOI)
        bool useSquareMetric; // use max(|a.x - b.x|, |a.y - b.y|) as distance metric instead of euclidean distance
}

module OverlayQuon like IOverlay
{
    gates:
        input udpIn; // gate from the UDP layer
        output udpOut; // gate to the UDP layer
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn; // gate from the application
        output appOut; // gate to the application

    submodules:
        quon: Quon {
            parameters:
                @display("p=60,60");

        }
    connections allowunconnected:
        udpIn --> quon.udpIn;
        udpOut <-- quon.udpOut;
        appIn --> quon.appIn;
        appOut <-- quon.appOut;
}
