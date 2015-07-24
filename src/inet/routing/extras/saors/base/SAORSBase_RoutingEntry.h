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

#ifndef SAORSBASE_ROUTINGENTRY_H
#define SAORSBASE_ROUTINGENTRY_H


#include <omnetpp.h>
#include <string.h>
#include <sstream>
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/routing/extras/dymo_fau/DYMO_Timer.h"

namespace inet {

namespace inetmanet {


class SAORSBase;


namespace {
	const double INIT_DEV_PROB = 0.5;
}


/********************************************************************************************
 * class SAORSBase_RoutingEntry: Defines the structure of the routing entries of SAORS
 ********************************************************************************************/
class SAORSBase_RoutingEntry
{
	public:
		SAORSBase_RoutingEntry(SAORSBase* SAORSBase);
		virtual ~SAORSBase_RoutingEntry();

		/**
		 * @name DYMO Mandatory Fields
		 */
		/*@{*/
		IPv4Address routeAddress;                 ///< The IP destination address of the getNode(s) associated with the routing table entry
		unsigned int routeSeqNum;               ///< The DYMO SeqNum associated with this routing information
		IPv4Address routeNextHopAddress;          ///< The IP address of the next DYMO router on the path toward the Route.Address
		InterfaceEntry* routeNextHopInterface;  ///< The interface used to send packets toward the Route.Address
		bool routeBroken;                       ///< A flag indicating whether this Route is broken.  This flag is set if the next hop becomes unreachable or in response to processing a RERR (see Section 5.5.4)
		/*@}*/

		/**
		 *  @name Delay Tolerant DYMO routing entry fields
		 */
		/*@}*/
		double deliveryProb;	///< The estimation of probability of delivery to the destination
		double similarity;      ///< The social similarity between the two nodes
		uint beaconsRcvd;		///< The number of beacons received
		uint meetings;			///< The number of meetings between the two nodes
		simtime_t last_aging;	///< Measures the last time a node has been spotted for adjusting the delivery probability
		simtime_t last_seen	;	///< Measures the last time a node has been spotted for adjusting the number of meetings
		/*@}*/

		/**
		 * @name DYMO Optional Fields
		 */
		/*@{*/
		short int routeDist;    ///< A metric indicating the distance traversed before reaching the Route.Address node
		int routePrefix;        ///< Indicates that the associated address is a network address, rather than a host address.  The value is the length of the netmask/prefix.  If an address block does not have an associated PREFIX_LENGTH TLV [I-D.ietf-manet-packetbb], the prefix may be considered to have a prefix length equal to the address length (in bits)
		/*@}*/

		/**
		 * @name DYMO Timers
		 * Each set to the simulation time at which it is meant to be considered expired or -1 if it's not running
		 */
		/*@{*/
		DYMO_Timer routeAgeMin; ///< Minimum Delete Timeout. After updating a route table entry, it should be maintained for at least ROUTE_AGE_MIN
		DYMO_Timer routeAgeMax; ///< After the ROUTE_AGE_MAX timeout a route must be deleted
		DYMO_Timer routeNew;    ///< After the ROUTE_NEW timeout if the route has not been used, a timer for deleting the route (ROUTE_DELETE) is set to ROUTE_DELETE_TIMEOUT
		DYMO_Timer routeUsed;   ///< When a route is used to forward data packets, this timer is set to expire after ROUTE_USED_TIMEOUT.  This operation is also discussed in Section 5.5.2
		DYMO_Timer routeDelete; ///< After the ROUTE_DELETE timeout, the routing table entry should be deleted
		/*@}*/

		/**
		 *  @name DTDYO Timer
		 */
		/*@{*/
		DYMO_Timer dtrtDelete;  ///< After the DTRT_DELETE timeout, the delay tolerant routing table entry should be deleted
		/*@{*/

		IPv4Route* routingEntry;  ///< Forwarding Route (entry in standard OMNeT++ routingTable)

		bool operator==(const SAORSBase_RoutingEntry& entry) {
				return (routeAddress.getInt()==entry.routeAddress.getInt());
		}

		/** @brief overloading the () operator for allowing search for specific addresses in the routing table*/
		bool operator==(const SAORSBase_RoutingEntry* entry)  {
				return (routeAddress.getInt()==entry->routeAddress.getInt());
		}

		/**
         *  @name DT Routing Type (DT-DYMO/SD-DYMO/...)
         */
        /*@{*/
		std::string DT_PROTO;
		/*@{*/


	protected:
		SAORSBase* dymo; /**< DYMO module */

	public:
	    friend std::ostream& operator<<(std::ostream& os, const SAORSBase_RoutingEntry& e);
	    bool hasActiveTimer() { return routeAgeMin.isActive() || routeAgeMax.isActive() || routeNew.isActive() || routeUsed.isActive() || routeDelete.isActive() || dtrtDelete.isActive(); }
};


/** @brief overloading operator for printing the info an Information Item class */
inline std::ostream& operator<<(std::ostream& os, SAORSBase_RoutingEntry& o)
{
	os << "[ ";
	os << "destination address: " << o.routeAddress;
	os << ", ";
	os << "sequence number: " << o.routeSeqNum;
	os << ", ";
	os << "next hop address: " << o.routeNextHopAddress;
	os << ", ";
	os << "next hop interface: " << ((o.routeNextHopInterface) ? (o.routeNextHopInterface->getName()) : "unknown");
	os << ", ";
	//Only if the PROPHET routing us used, print out these values
	if(o.DT_PROTO == "DT-DYMO") {
        os << "delivery_prob: " << o.deliveryProb;
        os << ", ";
        os << "Last aging: " << o.last_aging;
        os << ", ";
	}

	bool run=o.routeAgeMax.isRunning();
	//If ROUTE_AGE_MAX is running, it means this is a normal routing table entry, so print only the relevant info
	if(run) {
		os << "broken: " << o.routeBroken;
		os << ", ";
		os << "distance metric: " << o.routeDist;
		os << ", ";
		os << "prefix: " << o.routePrefix;
		os << ", ";
		os << "ROUTE_AGE_MIN: " << o.routeAgeMin;
		os << ", ";
		os << "ROUTE_AGE_MAX: " << o.routeAgeMax;
		os << ", ";
		os << "ROUTE_NEW: " << o.routeNew;
		os << ", ";
		os << "ROUTE_USED: " << o.routeUsed;
		os << ", ";
		os << "ROUTE_DELETE: " << o.routeDelete;
		os << ", ";
	}
	//else this is a DT routing entry, so again print only the relevant information
	else {
		os << "BeaconsReceived: " << o.beaconsRcvd;
		os << ", ";
		os << "Meetings: " << o.meetings;
		os << ", ";
		os << "MeetEnd: " << o.last_seen;
		os << ", ";
		os << "DTRT_DELETE: " << o.dtrtDelete;
		os << " ]";
	}
	return os;
}

} // namespace inetmanet

} // namespace inet

#endif /* SAORSBASE_ROUTINGENTRY_H */
