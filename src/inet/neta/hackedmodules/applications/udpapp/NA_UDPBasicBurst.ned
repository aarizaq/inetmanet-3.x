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

package inet.neta.hackedmodules.applications.udpapp;
import inet.applications.udpapp.UDPBasicBurst;


//
// UDPBasicBurst hacked module. Allow new statistics on incoming packets.
//
// Allowed new statistics on incoming packets:
//  - numHops: hop count of not dropped, not out-of-order packets.   
//
// @author Leovigildo Sánchez Casado, sancale@ugr.es
// @date 03/16/2013//

//
//
simple NA_UDPBasicBurst extends UDPBasicBurst
{
    parameters:
        
        //# Normal parameters
        @display("i=block/app,white;i2=status/green");
	    @class(NA_UDPBasicBurst);	    
	    
	    //# Hop count statistics
	    @signal[hopCount](type=long); // Signal for hop count
	    @statistic[hopCount](title="Hop count"; record=last); // Record the hop count

}

