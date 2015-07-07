//
//  Copyright (C) 2012 Nikolaos Vastardis
//  Copyright (C) 2012 University of Essex
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package inet.routing.extras.saors.sampho;


import inet.routing.extras.saors.base.SAORSBase;

//
// <b> Socially-Aware Multi Phase Opportunistic Routing </b>
//
// The Socially-Aware Multi Phase Opportunistic Routing (SAMPhO) 
// is a more sophisticated  version of DT-DYMO. It is also based on 
// the SAORSBase classes but does not use PROPHET to determine the
// best carriers. Instead it uses multiple metrics such as 
// beacons received, ego-betweenness and community similarity
// in order to determine the best one for the various stages of
// the routing proccess. Therefore, it can provide much better 
// results than DTDYMO.
//
// @see SAORSBase, DTDYMO, EPDYMO, RDYMO
//
// @author: Nikolaos Vastardis
//
simple SAMPhO extends SAORSBase
{
    parameters:
        @class(SAMPhO);
        @signal[Betweenness](type="double");
        @statistic[Betweenness](title="Betweenness"; source="Betweenness"; record=vector; interpolationmode=none);
        string METRIC = default("BEACON");
        double BETW_THRESHOLD = default(0.3);
        double SIM_THRESHOLD = default(0.3);   
        double SOCIAL_SENSITIVITY = default(30);
        double ANTISOCIAL_SENSITIVITY = default(30);     
    gates:
}
