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

#ifndef _SIMBETTS_ROUTINGTABLE_H
#define _SIMBETTS_ROUTINGTABLE_H


#include <omnetpp.h>
#include <vector>
#include "inet/common/INETDefs.h"
#include "inet/routing/extras/saors/base/SAORSBase_RoutingTable.h"
#include "inet/routing/extras/saors/base/SAORSBase_RoutingEntry.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/routing/extras/saors/base/SAORS_BEACON_m.h"
#include "inet/routing/extras/saors/sampho/EgoAdjacency.h"


namespace inet {

namespace inetmanet {

/*****************************************************************************************
 * class SimBetTS_RoutingTable: Describes the extended functionality of the SimBetTS routing
 *                            table.
 *****************************************************************************************/
class SimBetTS_RoutingTable : public SAORSBase_RoutingTable
{
	protected:
		EgoAdjacency egoInfo;

	public:
		double betw;
		/** @brief class constructor */
		SimBetTS_RoutingTable(cObject* host, const IPv4Address& myAddr);

		/** @brief class destructor */
		virtual ~SimBetTS_RoutingTable();

		//-------------------------------------------------------------//
		//          -- Route table manipulation operations --          //
		//-------------------------------------------------------------//
		/** @brief ages all the DT-entries' probabilities in the DT-DYMO field of the entries */
		virtual void ageAllBeacons();

		/** @brief updates the beacon counter of the specified entry, according to the maximum time window */
        virtual void updateBeaconCounter(uint32_t Address, VectorOfSAORSBeaconBlocks entryVector);

        /** @brief updates the similarity metric for the specified entry, according to the friends of the met node */
        virtual void updateSimilarity(uint32_t Address, VectorOfSAORSBeaconBlocks entryVector);

		/** @brief Determines the social neighbors of the host */
		virtual void findNeighbors();

		/** @brief returns the number of the social friends of the host */
		virtual uint getNumOfNeighbors();

		/** @brief locates the entry rank for a given address, returns negative if not found */
		virtual uint findEntryRank(uint32_t Address);

		/** @brief prints out the info of the class to include the Ego-Adjacency Matrix */
		virtual std::string str() const;

		/** @brief returns the betweenness centrality value of the node */
		virtual double getBetweenness();

		/** @brief returns the number of maximum beacons per counting interval */
		virtual uint getMaxBeaconsNumber();

		/** @brief returns whether a certain routing entry describes a friend of the current node or not */
		virtual bool isFriend(const SAORSBase_RoutingEntry *entry);

		/** @brier returns the neighbor similarity between the node and the carrier sending the beacon */
		virtual double getSimilarityScore(VectorOfSAORSBeaconBlocks beaconEntries);
		//-------------------------------------------------------------//
};

} // namespace inetmanet

} // namespace inet

#endif /* SIMBETTS_ROUTINGTABLE_H */
