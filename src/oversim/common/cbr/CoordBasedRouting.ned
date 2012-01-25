//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
// @author Fabian Hartmann
//

package inet.oversim.common.cbr;
//package oversim.common;
import inet.oversim.common.IGlobalFunctions;

//
// Global Module for providing allround CBR information
//
simple CoordBasedRouting
{
    parameters:
        @class(CoordBasedRouting);
        //@display("i=block/users");
        //bool enabled;
        string areaCoordinateSource;        // XML filename
        int CBRstartAtDigit;                // start at this digit using CBR
        int CBRstopAtDigit;                 // stop at this digit using CBR
        //int coordCalcRuns;                  // How many runs will be started with random coordinates when calculating a node's coordinates?
        //double waitForLandmarks @unit(s);   // If not enough landmarks are available yet: Wait how long until next try?
        //bool onlyCoordFault;                // only coord measuring RTTs should be manipulated
        //int coordCheckPercentage;           // Sets the  Range (in Percent) for which malicious computed Coordinates are allowed
}

module CoordBasedRoutingModules like IGlobalFunctions
{
    submodules:
        coordBasedRouting: CoordBasedRouting {
            parameters:
                @display("i=block/users");
        }
}