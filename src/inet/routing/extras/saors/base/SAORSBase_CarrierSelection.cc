/*
 *  Copyright (C) 2012 Nikolaos Vastardis
 *  Copyright (C) 2012 University of Essex
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "SAORSBase_CarrierSelection.h"

namespace inet {


namespace inetmanet {

/********************************************************************************************
 * Class Constructor
 ********************************************************************************************/
SAORSBase_CarrierSelection::SAORSBase_CarrierSelection() {
}


/********************************************************************************************
 * Class Destructor
 ********************************************************************************************/
SAORSBase_CarrierSelection::~SAORSBase_CarrierSelection() {
	clearCSList();
}


/********************************************************************************************
 * Empties the RREP list from all objects.
 ********************************************************************************************/
void SAORSBase_CarrierSelection::clearCSList() {
	std::list<SAORS_RREP *>::iterator iter = RREP_list.begin();
	while( !RREP_list.empty() ) {
		// Deleting contents
		SAORS_RREP *rrep = *iter;
		RREP_list.erase(iter);
		delete rrep;

		iter = RREP_list.begin();
	}

	EV << "Carrier selection list cleared" << endl;
}


/********************************************************************************************
 * Adds new RREP packets to list.
 ********************************************************************************************/
void SAORSBase_CarrierSelection::addRREPtoList(SAORS_RREP *rrep) {

	if( RREP_list.size() < MAX_RREP_LIST_SIZE ) {
		RREP_list.push_back(rrep);
	}
	// Find the RREP with the smallest probability to exchange it with the new RREP
	else {
		double minprob=0;
		std::list<SAORS_RREP *>::iterator rpl;
		SAORS_RREP *node;

		for (std::list<SAORS_RREP *>::iterator iter = RREP_list.begin(); iter != RREP_list.end(); iter++) {

			node = *iter;

			// A smaller probability is found
			if( minprob <= node->getDeliveryProb() ) {
				minprob = node->getDeliveryProb();
				rpl = iter;
			}
		}

		node = *rpl;

		// If present RREP has smaller probability that the new one exchange
		if( node->getDeliveryProb() < rrep->getDeliveryProb() ) {

			// Erasing node
			delete node;
			RREP_list.erase(rpl);

			// Adding new RREP to list
			RREP_list.push_back(rrep);
		}
		else
			delete rrep;
	}

	EV << "RREP added to carrier selection list" << endl;
}


/********************************************************************************************
 * Returns the size of the RREP list.
 ********************************************************************************************/
int SAORSBase_CarrierSelection::CSListSize() {
	return RREP_list.size();
}


/********************************************************************************************
 * Returns the received RREP with the highest probability.
 ********************************************************************************************/
SAORS_RREP* SAORSBase_CarrierSelection::findBestCarrier() {

	double prob=0;
	SAORS_RREP *rrep = NULL;

	// Check the whole list for the RREP with the highest probability of encountering the destination
	for( std::list<SAORS_RREP *>::iterator iter = RREP_list.begin(); iter != RREP_list.end(); iter++) {
		// Checking the probabilities
		if( prob < (*iter)->getDeliveryProb() ) {
			rrep = *iter;
			prob = (*iter)->getDeliveryProb();
		}
	}
	return rrep;
}

} // namespace inetmanet

} // namespace inet
