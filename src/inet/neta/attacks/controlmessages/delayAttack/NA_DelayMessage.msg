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
// Message interchanged between delay attack controller and the corresponding hacked
// modules. In this case NA_IPv4
//
// @see NA_DelayAttack, NA_IPv4
//
// @author Gabriel Maciá Fernández, gmacia@ugr.es
// @date 01/22/2013
message NA_DelayMessage {
    double delayAttackProbability; // Attack probability
    omnetpp::cPar * delayAttackValue;  // Implemented as a pointer to cPar to allow the use of random distributions for the delay value

}
