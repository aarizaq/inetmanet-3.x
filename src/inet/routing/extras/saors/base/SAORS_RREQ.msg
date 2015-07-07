//
//  Copyright (C) 2005 Mohamed Louizi
//  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

cplusplus {{
#include "inet/routing/extras/dymo_fau/DYMO_RM_m.h"
}}

namespace inet::inetmanet;

class DYMO_RM;


//
// <b> The SAORS Routing-Request Message </b>
//
// This is an extension of the DYMO_RM message for requesting a
// path towards another destination. It includes one more field,
// namely the probability of the requesting node to contact the
// searched destination. Other SAOSR nodes will only reply if
// they has a path towards the destination or if their probability
// of meeting that node in the future is higher than the one stated
// in the SAORS_RREQ.
//
// @see SAORSBase
//
// @author: Nikolaos Vastardis
//
message SAORS_RREQ extends DYMO_RM
{
    double minDeliveryProb;
}
