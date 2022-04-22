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

#include "RDYMO.h"

namespace inet {

namespace inetmanet {

Define_Module(RDYMO);


#define DYMO_PORT 653

using namespace SAORSns;


/********************************************************************************************
 * The Initialization function of the class.
 ********************************************************************************************/
void RDYMO::initialize(int stage)
{
    if (stage == 4)
    {
        //Initialize DYMO class
        SAORSBase::initialize(stage);

        //Set RDYMO time parameters
        RAND_THRS = par("RAND_THRS").doubleValue();
    }
}


/*****************************************************************************************
 * Generates and sends a beacon to the surrounding nodes.
 *****************************************************************************************/
void RDYMO::sendBeacon() {

	SAORS_BEACON* bcn =  new SAORS_BEACON("DT-BEACON");

	// do not transmit DYMO messages when we lost our sequence number
	if (ownSeqNumLossTimeout->isRunning()) {
		EV << "node has lost sequence number -> not transmitting Beacon" << endl;
		delete bcn;
		return;
	}

	//Increment sequence number
	incSeqNum();

	//Set target node parameters
	bcn->setMsgHdrHopLimit(1);
	bcn->getTargetNodeForUpdate().setAddress(IPv4Address::LL_MANET_ROUTERS.getInt());
	bcn->getTargetNodeForUpdate().setSeqNum(0);
	bcn->getTargetNodeForUpdate().setDist(1);

	//Set source node parameters
	bcn->getOrigNodeForUpdate().setAddress(myAddr);
	if (RESPONSIBLE_ADDRESSES_PREFIX != -1) bcn->getOrigNodeForUpdate().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
	bcn->getOrigNodeForUpdate().setSeqNum(ownSeqNum);
	bcn->getOrigNodeForUpdate().setDist(0);

	sendDown(bcn, IPv4Address::LL_MANET_ROUTERS.getInt());
}


/*****************************************************************************************
 * Performs the update of the routing entries, according to the beacon received.
 *****************************************************************************************/
void RDYMO::handleBeacon(SAORS_BEACON* my_beacon) {

    SAORSBase_RoutingEntry *dtentry, *dst_entry;
	bool flag=false;
	//Total number of packets sent
    int totalPkts=0;

	EV << "received message is a Beacon message" << endl;

	// Increase the network density recording
	beaconsReceived++;

	/** Routing message pre-processing and updating routes from routing blocks **/
	if(updateRoutes(dynamic_cast<DYMO_RM*>(my_beacon)) == NULL)
		EV << "Updating routes from Beacon failed" << endl;

	//--------------------------------------------------------------------//
    // Check if final destination of any packets in DT queue, sent beacon //
    //--------------------------------------------------------------------//
    dst_entry = dymo_routingTable->getByAddress(IPv4Address(my_beacon->getOrigNodeForUpdate().getAddress()));
    //Sanity check -- That there is a routing table entry
    if(!dst_entry) {
        EV << "ERROR: The node that just sent a beacon cannot be found in the routing table!!!" << endl;
        return;
    }

    //If there is a route to the beacon node
    if((!dst_entry->routeBroken) && dtqueuedDataPackets->pktsexist(dst_entry->routeAddress, dst_entry->routePrefix)) {
        // An entry was found in the routing table -> get control data from the table
        EV << "Beacon received and we DO have a route" << endl;

        //Dequeue packets and send them to the met carrier
        dymo_routingTable->maintainAssociatedRoutingTable();
        int numOfPkts=dtqueuedDataPackets->dequeuePacketsTo(dst_entry->routeAddress, dst_entry->routePrefix);
        flag=true;

        //Count this as a DT Message
        DTMesgRcvd -= numOfPkts;
        totalPkts += numOfPkts;
    }

	//Check all the DT Queue for the destinations
	for(uint i=0;i<dtqueuedDataPackets->getSize();i++) {

        //Get destination that the packets are destined
        IPv4Address addr = dtqueuedDataPackets->getDestination(i);

        //Check if destination exists in the routing table...
        dtentry=dymo_routingTable->getDTByAddress(addr);

        // Check the random probability of meeting the destination
        if( dblrand() < RAND_THRS ) {
            // Dequeue packets to IP
            int numOfPkts=dtqueuedDataPackets->dequeuePacketsTo(addr, (dtentry)?dtentry->routePrefix:32, dst_entry->routeAddress, totalPkts );
            flag=true;

            //Count this as a DT Message
            DTMesgRcvd -= numOfPkts;
            totalPkts += numOfPkts;
        }
	}

	// No proper destination found
	if(flag==false)
		EV << "Beacon was received but NO route or NO better carrier!" << endl;

	delete my_beacon;
}


/*****************************************************************************************
 * Returns the probability of encountering the node indicating by the SAORS routing entry.
 *****************************************************************************************/
double RDYMO::findEncounterProb(const SAORSBase_RoutingEntry* routeToNode) {
	return 0;
}


/*****************************************************************************************
 * Returns whether the RREP has a higher or lower  encounter probability.
 *****************************************************************************************/
bool RDYMO::compareEncounterProb(const SAORSBase_RoutingEntry* dtEntry, const SAORS_RREP* rrep) {
	return false;
}


/*****************************************************************************************
 * Determines whether the node will reply as an intermediate router, to inform of it's
 * encounter probability.
 *****************************************************************************************/
bool RDYMO::sendEncounterProb(SAORS_RREQ* rreq, const SAORSBase_RoutingEntry* dtEntry) {
	return false;
}

} // namespace inetmanet

} // namespace inet
