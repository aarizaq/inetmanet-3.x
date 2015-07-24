/*
 *  Copyright (C) 2012 Nikolaos Vastardis
 *  Copyright (C) 2012 University of Essex
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "SAORSBase_RoutingEntry.h"
#include "SAORSBase.h"

namespace inet {

namespace inetmanet {


/********************************************************************************************
 * Class Constructor
 ********************************************************************************************/
SAORSBase_RoutingEntry::SAORSBase_RoutingEntry(SAORSBase* dymo) : routeAgeMin(dymo, "routeAgeMin"), routeAgeMax(dymo, "routeAgeMax"), routeNew(dymo, "routeNew"), routeUsed(dymo, "routeUsed"), routeDelete(dymo, "routeDelete"), dtrtDelete(dymo, "dtrtDelete"), routingEntry(0), dymo(dymo) {

	deliveryProb = INIT_DEV_PROB;
	similarity = 0;
	beaconsRcvd = 1;
	meetings = 1;
	last_aging = simTime();
	last_seen = simTime();
	DT_PROTO = dymo->getParentModule()->par("routingProtocol").stdstringValue();
}


/********************************************************************************************
 * Class Destructor
 ********************************************************************************************/
SAORSBase_RoutingEntry::~SAORSBase_RoutingEntry(){

}

} // namespace inetmanet

} // namespace inet
