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

#ifndef SAORSBASE_TOKENBUCKET_H
#define SAORSBASE_TOKENBUCKET_H

#include <omnetpp.h>


#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/networklayer/ipv4/ICMPMessage.h"
#include "inet/routing/extras/dymo_fau/DYMO_Packet_m.h"
#include "SAORSBase_RoutingTable.h"
#include "inet/routing/extras/dymo_fau/DYMO_OutstandingRREQList.h"
#include "inet/routing/extras/dymo_fau/DYMO_RM_m.h"
#include "SAORS_RREQ_m.h"
#include "SAORS_RREP_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_AddressBlock.h"
#include "inet/routing/extras/dymo_fau/DYMO_RERR_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_UERR_m.h"
#include "SAORSBase_DataQueue.h"
#include "inet/routing/extras/dymo_fau/DYMO_Timeout_m.h"

namespace inet {

namespace inetmanet {


/********************************************************************************************
 * class SAORSBase_TokenBucket: Simple rate limiting mechanism
 ********************************************************************************************/
class SAORSBase_TokenBucket {
	public:
        /** Class Constructor */
		SAORSBase_TokenBucket(double tokensPerTick, double maxTokens, simtime_t currentTime);

		/** @brief consumes the initial tokens assigned from the constuctor */
		bool consumeTokens(double tokens, simtime_t currentTime);

	protected:
		double tokensPerTick;
		double maxTokens;

		double availableTokens;
		simtime_t lastUpdate;

};

} // namespace inetmanet

} // namespace inet

#endif /* SAORSBASE_TOKENBUCKET_H */

