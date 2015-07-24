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

#ifndef _DTDYMO_H
#define _DTDYMO_H


#include <omnetpp.h>
#include "inet/routing/extras/saors/base/SAORSBase.h"
#include "DTDYMO_RoutingTable.h"

namespace inet {

namespace inetmanet {

/********************************************************************************************
 * class DTDYMO: Implements a socially-aware network layer to route incoming messages
 ********************************************************************************************/
class DTDYMO : public SAORSBase {
	public:
        /** @brief the initialization function of the class */
		void initialize(int);

	protected:
		friend class DTDYMO_RoutingTable;
		//===============================================================================
		// OPERATIONS
		//===============================================================================
		/** @brief generates and sends a beacon to the surrounding nodes */
		virtual void sendBeacon();

		/** @brief performs the update of the routing entries, according to the beacon received */
		virtual void handleBeacon(SAORS_BEACON* my_beacon);

		/** @brief returns the probability of encountering the node indicating by the DT Routing Entry */
		virtual double findEncounterProb(const SAORSBase_RoutingEntry* routeToNode);

		/** @brief returns whether the RREP has a higher or lower  encounter probability */
		virtual bool compareEncounterProb(const SAORSBase_RoutingEntry* dtEntry, const SAORS_RREP* rrep);

		/** @brief determines whether the node will reply as an intermediate router, to inform of it's encounter probability*/
		virtual bool sendEncounterProb(SAORS_RREQ* rreq, const SAORSBase_RoutingEntry* dtEntry);

		//===============================================================================
        // MEMBER VARIABLES
        //===============================================================================
        DTDYMO_RoutingTable* DTDYMO_ref;    ///< pointer to the owned routing table
};

} // namespace inetmanet

} // namespace inet



#endif /* _DTDYMO_H */

