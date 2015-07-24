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

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <math.h>
#include <fstream>
#include "DTDYMO.h"
#include "DTDYMO_RoutingTable.h"
#include "inet/routing/extras/base/ManetRoutingBase.h"

namespace inet {

namespace inetmanet {

namespace {
 const double GAMMA = 0.9;
 const double BETA = 0.25;
}

/*****************************************************************************************
 * The Constructor function of the DT-DYMO routing table class.
 *****************************************************************************************/
DTDYMO_RoutingTable::DTDYMO_RoutingTable(cObject* host, const IPv4Address& myAddr) : SAORSBase_RoutingTable(host, myAddr)
{
    // get our host module
    if (!host) throw std::runtime_error("No parent module found");

    dymoProcess = host;
}


/*****************************************************************************************
 * The Destructor of the PS-DYMOrouting table class.
 *****************************************************************************************/
DTDYMO_RoutingTable::~DTDYMO_RoutingTable() {
    SAORSBase_RoutingEntry* entry;
    while ((entry = getRoute(0))) deleteRoute_destrOnly(entry);
    while ((entry = getDTRoute(0))) deleteDTRoute(entry);
}


/*****************************************************************************************
 * Ages the delivery probability of the a given DT-DYMO entry.
 *****************************************************************************************/
bool DTDYMO_RoutingTable::age_Probability(SAORSBase_RoutingEntry *entry) {

    //If entry exists
	if(entry) {
		//Update Interval
		double interval = simTime().dbl() - entry->last_aging.dbl();

		//Check the if the last aging time was less than 1.1*time-slot ago (safety interval),
		//and afterwards age the PROPHET metric
		if((interval/(dynamic_cast <DTDYMO*> (dymoProcess))->BEACON_TIMEOUT)>1.1) {
			entry->last_aging = simTime();
			entry->deliveryProb = entry->deliveryProb * ( pow(GAMMA, interval/(dynamic_cast <DTDYMO*> (dymoProcess))->BEACON_TIMEOUT) );

			// Check for probability mistakes
			if(entry->deliveryProb>0.99999)
				entry->deliveryProb=1;
			if(entry->deliveryProb<0)
				entry->deliveryProb=0;

			return true;
		}

		//Else return false
		return false;
	}
	else {
		EV << "Unknown routing entry requested to be updated from delay tolerant routing table..." << endl;
		return false;
	}
}


/*****************************************************************************************
 * Updates the DT-DYMO field of the entries, when the given node is contacted.
 *****************************************************************************************/
void DTDYMO_RoutingTable::adjust_Probabilities(uint32_t Address, VectorOfSAORSBeaconBlocks entryVector) {

	// Looking for requested Address first in DT-DYMO table and then in DYMO
	SAORSBase_RoutingEntry* entry = getDTByAddress(IPv4Address(Address));
	if(!entry)
		entry = getByAddress(IPv4Address(Address));

	if(entry) {
		// First age the Beacon sender's specific probability
		bool age=age_Probability(entry);

		EV << "Probability being updated from : " << entry->deliveryProb << endl;

		//Check if delivery probability was just initialized
		if(age!=false && entry->deliveryProb!=INIT_DEV_PROB) {
			entry->deliveryProb = entry->deliveryProb + ( (1 - entry->deliveryProb) * INIT_DEV_PROB );

			// Check for probability mistakes
			if(entry->deliveryProb>0.99999)
				entry->deliveryProb=1;
			if(entry->deliveryProb<0)
				entry->deliveryProb=0;
		}

		EV << "Probability updated to: " << entry->deliveryProb << endl;

		//Then age the beacon entries probabilities
		if(!entryVector.empty()) {
			for(std::vector<SAORSBase_BeaconBlock>::iterator iter=entryVector.begin(); iter < entryVector.end(); iter++) {

			    SAORSBase_BeaconBlock bcn_entry=*iter;

				// Look first in DT-DYMO table and the in DYMO routing table
				SAORSBase_RoutingEntry* dtentry = getDTByAddress(IPv4Address(bcn_entry.getAddress()));
				if(!dtentry)
					dtentry = getByAddress(IPv4Address(bcn_entry.getAddress()));

				// If entry is found, update it's probability
				if(dtentry) {
				    //First age probability for vector entry
                    age_Probability(dtentry);

					EV << "Probability being updated from : " << dtentry->deliveryProb << endl;

					dtentry->deliveryProb = dtentry->deliveryProb + ( (1 - dtentry->deliveryProb) * entry->deliveryProb * bcn_entry.getProbability() * BETA );

					// Check for probability mistakes
					if(dtentry->deliveryProb>0.99999)
						dtentry->deliveryProb=1;
					if(dtentry->deliveryProb<0)
						dtentry->deliveryProb=0;

					EV << "Probability updated to: " << dtentry->deliveryProb << endl;
				}
			}
		}
	}
	else {
		EV << "Unknown routing entry requested to be updated from delay tolerant routing table..." << endl;
	}
}


} // namespace inetmanet

} // namespace inet



