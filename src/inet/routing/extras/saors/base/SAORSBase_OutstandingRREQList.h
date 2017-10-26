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

#ifndef SAORSBASE_OUTSTANDINGRREQLIST_H_
#define SAORSBASE_OUTSTANDINGRREQLIST_H_

#include "SAORSBase_CarrierSelection.h"
#include <omnetpp.h>
#include <vector>
#include "inet/routing/extras/dymo_fau/DYMO_Timer.h"
#include "inet/common/INETDefs.h"

namespace inet {

namespace inetmanet {


/********************************************************************************************
 * class SAORSBase_OutstandingRREQ: A single item of the SAORSBase_OutstandingRREQList class
 ********************************************************************************************/
class SAORSBase_OutstandingRREQ {
	public:
		SAORSBase_CarrierSelection CS_List;
		unsigned int tries;
		DYMO_Timer* wait_time;
		DYMO_Timer* RREPgather_time;
		unsigned int destAddr;
		simtime_t creationTime;

	public:
		friend std::ostream& operator<<(std::ostream& os, const SAORSBase_OutstandingRREQ& o);
};


/********************************************************************************************
 * class SAORSBase_OutstandingRREQList: Controls the generation of SAORS_RREQs for a specific
 *                                      destination. It incorporates the uses the
 *                                      SAORSBase_CarrierSelection class to achieve its
 *                                      functionality.
 ********************************************************************************************/
class SAORSBase_OutstandingRREQList : public cObject
{
	public:
        /** Class Constructor */
		SAORSBase_OutstandingRREQList();

		/** Class Destructor */
		~SAORSBase_OutstandingRREQList();

		/** @brief inherited from cObject */
		virtual const char* getFullName() const override;

		/** @brief inherited from cObject */
		virtual std::string info() const override;

		/** @brief inherited from cObject */
		virtual std::string str() const override;

		/** @brief returns SAORSBase_OutstandingRREQ with matching destAddr or 0 if none is found */
		SAORSBase_OutstandingRREQ* getByDestAddr(unsigned int destAddr, int prefix);

		/** @brief returns a SAORSBase_OutstandingRREQ whose wait_time is expired or 0 if none is found */
		SAORSBase_OutstandingRREQ* getExpired();

		/** @brief returns a SAORSBase_OutstandingRREQ whose RREPgather_time is expired or 0 if none is found */
		SAORSBase_OutstandingRREQ* getRREPGTExpired();

		/** @brief returns true if there is an active time in the outstanding RREQ list */
		bool hasActive() const;

		/**  @brief add the returned RREP to the Carrier selection queue to compare probabilities */
		void addRREPtoList(SAORS_RREP *rrep);

		/** @brief adds an entry for a specific request to the outstandingRREQs list */
		void add(SAORSBase_OutstandingRREQ* outstandingRREQ);

		/** @brief deletes an entry for a specific request from the outstandingRREQs list */
		void del(SAORSBase_OutstandingRREQ* outstandingRREQ);

		/** @brief empties the outstandingRREQs list from all requests */
		void delAll();

	protected:
		cModule* host;                                              ///< pointer to the owning class
		std::vector<SAORSBase_OutstandingRREQ*> outstandingRREQs;   ///< the list containing all the requests made

	public:
		friend std::ostream& operator<<(std::ostream& os, const SAORSBase_OutstandingRREQList& o);

};

} // namespace inetmanet

} // namespace inet

#endif /* SAORSBASE_OUTSTANDINGRREQLIST_H_ */
