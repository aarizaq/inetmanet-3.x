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


#include "DTDYMO.h"

namespace inet {

namespace inetmanet {

Define_Module(DTDYMO);


#define DYMO_PORT 653


using namespace SAORSns;


/*****************************************************************************************
 * The initialization function of the class.
 *****************************************************************************************/
void DTDYMO::initialize(int stage)
{
    if (stage == 4)
	{
		//Initialize DYMO class
	    SAORSBase::initialize(stage);

		//Generate the PS-DYMO routing Table as DT-DYMO
	    if(dymo_routingTable!=NULL) {
            delete dymo_routingTable;
            dymo_routingTable=NULL;
        }
        dymo_routingTable = new DTDYMO_RoutingTable(this, IPv4Address(myAddr));

        //Reference to routing table as PS-DYMO
        DTDYMO_ref = dynamic_cast<DTDYMO_RoutingTable *> (dymo_routingTable);
	}
}

/*****************************************************************************************
 * Generates and sends a beacon to the surrounding nodes.
 *****************************************************************************************/
void DTDYMO::sendBeacon() {

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
	bcn->getTargetNode().setAddress(IPv4Address::LL_MANET_ROUTERS.getInt());
	bcn->getTargetNode().setSeqNum(0);
	bcn->getTargetNode().setDist(1);

	//Set source node parameters
	bcn->getOrigNode().setAddress(myAddr);
	if (RESPONSIBLE_ADDRESSES_PREFIX != -1) bcn->getOrigNode().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
	bcn->getOrigNode().setSeqNum(ownSeqNum);
	bcn->getOrigNode().setDist(0);

	// Set beacon vector of entries
	for(uint i=0; i<dymo_routingTable->getDTNumRoutes(); i++) {
		SAORSBase_RoutingEntry* entry = dymo_routingTable->getDTRoute(i);

		// Update probability if entry exists
		DTDYMO_ref->age_Probability(entry);

		SAORSBase_BeaconBlock bcn_entry(entry->routeAddress.getInt(), entry->routePrefix, entry->deliveryProb);
		bcn->getBeaconEntries().push_back(bcn_entry);
	}

	sendDown(bcn, IPv4Address::LL_MANET_ROUTERS.getInt());
}


/*****************************************************************************************
 * Performs the update of the routing entries, according to the beacon received.
 *****************************************************************************************/
void DTDYMO::handleBeacon(SAORS_BEACON* my_beacon) {

	bool flag=false;

	EV << "received message is a Beacon message" << endl;

	// Increase the network density recording
	beaconsReceived++;

	/** Routing message pre-processing and updating routes from routing blocks **/
	if(updateRoutes(dynamic_cast<DYMO_RM*>(my_beacon)) == NULL)
		EV << "Updating routes from Beacon failed" << endl;

	// Update contact probabilities
	if(!my_beacon->getBeaconEntries().empty())
	    DTDYMO_ref->adjust_Probabilities(my_beacon->getOrigNode().getAddress(), my_beacon->getBeaconEntries());

	// Check if final destination of any packets in DT queue, sent beacon
	SAORSBase_RoutingEntry* entry = dymo_routingTable->getForAddress(IPv4Address(my_beacon->getOrigNode().getAddress()));
	if(entry && (!entry->routeBroken) && dtqueuedDataPackets->pktsexist(entry->routeAddress, entry->routePrefix)) {
		/** An entry was found in the routing table -> get control data from the table **/
		EV << "Beacon received and we DO have a route" << endl;

		//Dequeue packets and send them to IP
		dymo_routingTable->maintainAssociatedRoutingTable();
		int numOfPkts=dtqueuedDataPackets->dequeuePacketsTo(entry->routeNextHopAddress, entry->routePrefix);
		flag=true;

		//Count this as a DT Message
		DTMesgRcvd -= numOfPkts;
	}

	// Check if another node is a better carrier for storing packages or DT data queue
    if(!my_beacon->getBeaconEntries().empty()) {

        // Check all entries of beacon
        for(std::vector<SAORSBase_BeaconBlock>::iterator iter=my_beacon->getBeaconEntries().begin(); iter < my_beacon->getBeaconEntries().end(); iter++) {

            SAORSBase_BeaconBlock iterentry=*iter;

            // If entry only exists in DT routing table
            if( !dymo_routingTable->getByAddress(iterentry.getAddress()) &&
                dymo_routingTable->getDTByAddress(iterentry.getAddress()) )
            {

                SAORSBase_RoutingEntry* dtentry=dymo_routingTable->getDTByAddress(iterentry.getAddress());

                // If packets for that destination exist in DT queue
                if(dtqueuedDataPackets->pktsexist(dtentry->routeAddress, dtentry->routePrefix)) {

                    // Check highest probability of meeting the destination
                    if(iterentry.getProbability() > (dtentry->deliveryProb * HANDOVER_THESHOLD) ) {

                        // Dequeue packets to IP
                        int numOfPkts=dtqueuedDataPackets->dequeuePacketsTo(dtentry->routeAddress,
                                                                            dtentry->routePrefix,
                                                                            IPv4Address(my_beacon->getOrigNode().getAddress()) );
                        flag=true;

                        //Count this as a DT Message
                        DTMesgRcvd -= numOfPkts;
                    }
                }
            }
            //If this entry does not exists locally then just send it
            else if( !dymo_routingTable->getByAddress(iterentry.getAddress()) &&
                    !dymo_routingTable->getDTByAddress(iterentry.getAddress()) )
            {
                // Dequeue packets to IP
                int numOfPkts=dtqueuedDataPackets->dequeuePacketsTo( iterentry.getAddress(),
                                                                     iterentry.getPrefix(),
                                                                     IPv4Address(my_beacon->getOrigNode().getAddress()) );
                flag=true;

                //Count this as a DT Message
                DTMesgRcvd -= numOfPkts;
            }
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
double DTDYMO::findEncounterProb(const SAORSBase_RoutingEntry* routeToNode) {
	return routeToNode->deliveryProb;
}


/*****************************************************************************************
 * Returns whether the RREP has a higher or lower  encounter probability.
 *****************************************************************************************/
bool DTDYMO::compareEncounterProb(const SAORSBase_RoutingEntry* dtEntry, const SAORS_RREP* rrep) {
	return(dtEntry->deliveryProb < rrep->getDeliveryProb());
}


/*****************************************************************************************
 * Determines whether the node will reply as an intermediate router, to inform of it's
 * encounter probability.
 *****************************************************************************************/
bool DTDYMO::sendEncounterProb(SAORS_RREQ* rreq, const SAORSBase_RoutingEntry* dtEntry) {
	return ( rreq->getMinDeliveryProb()*HANDOVER_THESHOLD < dtEntry->deliveryProb );
}

} // namespace inetmanet

} // namespace inet


