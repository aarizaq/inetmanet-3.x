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

#ifndef _SAMPHO_H_
#define _SAMPHO_H_


#include <omnetpp.h>
#include "inet/routing/extras/saors/base/SAORSBase.h"
#include "SAMPhO_RoutingTable.h"

namespace inet {

namespace inetmanet {


/********************************************************************************************
 * class SAMPhO: Implements a multi-stage socially aware network layer to route
 *               incoming messages.
********************************************************************************************/
class SAMPhO : public SAORSBase
{
  public:
	/** @brief the initialization function of the class */
    virtual void initialize(int aStage);

  protected:
    friend class SAMPhO_RoutingTable;
    //===============================================================================
	// OPERATIONS
	//===============================================================================
    /** @brief generates and sends a beacon to the surrounding nodes */
    virtual void sendBeacon();

    /** @brief performs the update of the routing entries, according to the beacon received */
    virtual void handleBeacon(SAORS_BEACON* my_beacon);

    /** @brief returns the probability of encountering the node indicated by the DT Routing Entry */
	virtual double findEncounterProb(const SAORSBase_RoutingEntry* routeToNode);

	/** @brief returns whether the RREP has a higher or lower  encounter probability */
	virtual bool compareEncounterProb(const SAORSBase_RoutingEntry* dtEntry, const SAORS_RREP* rrep);

	/** @brief determines whether the node will reply as an intermediate router, to inform of it's encounter probability*/
	virtual bool sendEncounterProb(SAORS_RREQ* rreq, const SAORSBase_RoutingEntry* dtEntry);

    //===============================================================================
	// MEMBER VARIABLES
	//===============================================================================
	SAMPhO_RoutingTable* SAMPhO_ref;	///< pointer to the owned routing table

	double SOCIAL_SENSITIVITY;			///< Indicates the percentage of the social neighbors to be considered as friends

	double ANTISOCIAL_SENSITIVITY;		///< Indicates the percentage of the neighbors to be considered as strangers

	double BETW_THRESHOLD;              ///< Indicates the threshold over which a node is considered to be central

	double SIM_THRESHOLD;               ///< Indicates the threshold under which two nodes are considered not to be co-members of a community

	simsignal_t betweenness;			///< Records the betweenness values of the node during the simulation

	enum MetricType {
	    BEACON,
	    BETW,
	    SIMILARITY,
	    BB_COMBINE,
	    COMBINE,
	    BB_STAGED,
	    EXTENDED
	};

	MetricType METRIC;                  ///< The type of the metric to be used for the message forwarding
};

} // namespace inetmanet

} // namespace inet

#endif /* _SAMPHO_H_ */
