//
// Copyright (C) 2013, NESG (Network Engineering and Security Group), http://nesg.ugr.es,
// - Gabriel Maciá Fernández (gmacia@ugr.es)
// - Leovigildo Sánchez Casado (sancale@ugr.es)
// - Rafael A. Rodríguez Gómez (rodgom@ugr.es)
// - Roberto Magán Carrión (rmagan@ugr.es)
// - Pedro García Teodoro (pgteodor@ugr.es)
// - José Camacho Páez (josecamacho@ugr.es)
// - Jesús E. Díaz Verdejo (jedv@ugr.es)
//
// This file is part of NETA.
//
//    NETA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NETA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NETA.  If not, see <http://www.gnu.org/licenses/>.
//

package inet.neta.hackedmodules.networklayer.ipv4;
import inet.networklayer.ipv4.IPv4;

//
// IPv4 hacked module. Allow attacks on IPv4 INET layer.
//
// Allowed attacks (by its corresponding tag):
// - Dropping
// - Delay 
//
// @see NA_Attack, NA_AODVUU
//
// @author Gabriel Maciá Fernández, gmacia@ugr.es
// @date 01/22/2013
simple NA_IPv4 extends IPv4
{
    parameters: 
        //#Labels for the potential attacks that affect this module
        @dropping;	// Dropping attack in IPv4 Layer
        @delay;		// Delay attack in IPv4 Layer
        
        //# Normal parameters
        @display("i=block/routing,red;i2=status/excl3");
	    @class(neta::NA_IPv4);
	    
	    //# Dropping attack statistics
	    @signal[droppings](type=long); // Signal for number of packet discarded
	    @statistic[droppings](title="Number of drops"; record=count, vector; interpolationmode=none); // Record the number of packet discarded
	    
	    //# Packet received attack statistics
	    @signal[rcvdPkt](type=long); // Signal for number of packet received
	    @statistic[rcvdPkt](title="Number of packet received"; record=count, vector; interpolationmode=none); // Record the number of packet received
	    
	    //# Delay attack statistics
	    @signal[delayed](type=long); // Signal for number of packet delayed
	    @statistic[delayed](title="Number of packet delayed"; record=count, vector; interpolationmode=none); // Record the number of packet delayed
}
