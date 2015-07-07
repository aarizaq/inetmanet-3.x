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

package inet.routing.extras.saors.ep_dymo;


import inet.routing.extras.saors.base.SAORSBase;


//
// <b> The Epidemic DYMO </b>
//
// The Epidemic DYMO is a simple extention of the original DYMO MANET
// routing protocol that enables opportunistic routing as well. It is
// based on the SAORSBase system and extends the necessary hook 
// functions. The methods used follow a epidemic forwarding technique.
// This method achieves the highest possible delivery in the current
// network configuration, with the tradeoff of very high traffic and
// high network congestion. This can cause multiple packet collisions,
// even in sparse MANETs.
//
// @see SAORSBase, SDDYMO, DTDYMO, RDYMO
//
// @author: Nikolaos Vastardis
//
simple EPDYMO extends SAORSBase
{
    parameters:
        @class(EPDYMO);
        
    gates:
}

