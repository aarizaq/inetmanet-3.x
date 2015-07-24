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

#ifndef _DTDYMO_ROUTINGTABLE_H
#define _DTDYMO_ROUTINGTABLE_H

#include <omnetpp.h>
#include <vector>
#include "inet/common/INETDefs.h"
#include "inet/routing/extras/saors/base/SAORSBase_RoutingTable.h"
#include "inet/routing/extras/saors/base/SAORSBase_RoutingEntry.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/routing/extras/saors/base/SAORS_BEACON_m.h"

namespace inet {

namespace inetmanet {

/*****************************************************************************************
 * class DTDYMO_RoutingTable: Describes the extended functionality of the DT-DYMO routing
 *                            table.
 *****************************************************************************************/
class DTDYMO_RoutingTable : public SAORSBase_RoutingTable
{
	public:
        /** @brief class constructor */
        DTDYMO_RoutingTable(cObject* host, const IPv4Address& myAddr);

        /** @brief class destructor */
        virtual ~DTDYMO_RoutingTable();

		/** @brief ages the delivery probability of the a given DT-DYMO entry */
		virtual bool age_Probability(SAORSBase_RoutingEntry *entry);

		/** @brief updates the DT-DYMO field of the entries, when the given node is contacted */
		virtual void adjust_Probabilities(uint32_t Address, VectorOfSAORSBeaconBlocks entryVector);
};

} // namespace inetmanet

} // namespace inet



#endif
