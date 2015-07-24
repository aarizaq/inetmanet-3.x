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

package inet.routing.extras.saors.r_dymo;


import inet.routing.extras.saors.base.SAORSBase;


//
// <b> The Random DYMO </b>
//
// The Random DYMO is a simple extention of the original DYMO MANET
// routing protocol that enables opportunistic routing as well. It is
// based on the SAORSBase system and extends the necessary hook 
// functions. The methods used follow a randomly chosen forwarding
// technique. This method obviously does not achieve the desitrable
// outcomes, but could be very usefull for comparison pusposes. 
//
// @see SAORSBase, SDDYMO, DTDYMO , EPDYMO
//
// @author: Nikolaos Vastardis
//
simple RDYMO extends SAORSBase
{
    parameters:
        @class(RDYMO);
        double RAND_THRS = default (0.5);
        
    gates:
}

