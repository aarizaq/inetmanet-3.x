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

#ifndef SAORSBASE_CARRIER_SELECTION_H_
#define SAORSBASE_CARRIER_SELECTION_H_


#include "SAORS_RREP_m.h"

namespace inet {

namespace inetmanet {

namespace {
	const unsigned int MAX_RREP_LIST_SIZE = 5;
}


/********************************************************************************************
 * class SAORSBase_CarrierSelection: This class implements a list for saving the received
 *                                   SAORS_RREP for as long as non of them defines a valid
 *                                   path towards the requested destination. After the timer
 *                                   set has finished, the reply with the highest probability
 *                                   of contacting the destination is chosen and this node
 *                                   is selected as the best carrier.
 ********************************************************************************************/
class SAORSBase_CarrierSelection {
	protected:
		std::list<SAORS_RREP *> RREP_list;
	public:
		SAORSBase_CarrierSelection();
		virtual ~SAORSBase_CarrierSelection();

		/** @brief empties RREP list from all objects */
		void clearCSList();

		/** @brief adds new RREP packets to list */
		void addRREPtoList(SAORS_RREP *rrep);

		/** @brief returns the size of the RREP list */
		int CSListSize();

		/** @brief returns the received RREP with the highest probability */
		SAORS_RREP* findBestCarrier();
};

} // namespace inetmanet

} // namespace inet
#endif /* SAORSBase_CARRIER_SELECTION_H_ */
