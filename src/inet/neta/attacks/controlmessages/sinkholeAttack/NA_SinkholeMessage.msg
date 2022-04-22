//
// Copyright (C) 2013 by NESG (Network Engineering and Security Group), http://nesg.ugr.es
// Dept. of Signal Theory, Telematics and Communications
// University of Granada
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

namespace inet::neta;

//
// Message interchanged between sinkhole attack controller and the corresponding hacked
// modules. In this case NA_AODVUU
//
// @see NA_SinkholeAttack, NA_AODVUU
//
// @author Gabriel Maciá Fernández, gmacia@ugr.es
// @date 01/22/2013
message NA_SinkholeMessage {
    bool sinkOnlyWhenRouteInTable; 	// TRUE indicates that only RREQ for routes that are in routing table are answered as the best route across the sink node.
    								// A false value indicates that all the sinkhole node will answer to all the RREQ with fake routes. 
    double sinkholeAttackProbability;  	//Probability for announcing a fake route when a RREQ is received. Value in [0,1]
    omnetpp::cPar * seqnoAdded; 					// Sequence number.
    int numHops;						// Number of hops.
}
