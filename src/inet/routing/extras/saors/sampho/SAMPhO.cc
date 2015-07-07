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

#include "SAMPhO.h"

namespace inet {

namespace inetmanet {

Define_Module(SAMPhO);


#define DYMO_PORT 653


using namespace SAORSns;


/*****************************************************************************************
 * The initialization function of the class.
 *****************************************************************************************/
void SAMPhO::initialize(int stage)
{
	if (stage == 4)
	{
		//Initialize DYMO class
	    SAORSBase::initialize(stage);

		//Register betweenness signal
		betweenness = registerSignal("Betweenness");

		SOCIAL_SENSITIVITY = par("SOCIAL_SENSITIVITY");
		ANTISOCIAL_SENSITIVITY = par("ANTISOCIAL_SENSITIVITY");
		BETW_THRESHOLD = par("BETW_THRESHOLD");
		SIM_THRESHOLD = par("SIM_THRESHOLD");

		//Read the forwarding method to be used
		std::string mtr_par=par("METRIC").stdstringValue();
		if(mtr_par=="BEACON")
		    METRIC = BEACON;
		else if (mtr_par=="BETW")
            METRIC = BETW;
		else if (mtr_par=="SIMILARITY")
            METRIC = SIMILARITY;
		else if (mtr_par=="BB_COMBINE")
            METRIC = BB_COMBINE;
		else if (mtr_par=="COMBINE")
            METRIC = COMBINE;
		else if (mtr_par=="BB_STAGED")
            METRIC = BB_STAGED;
		else
            METRIC = EXTENDED;

		//Generate the DS-DYMO routing Table as the SAORSBase Routing Table
		if(dymo_routingTable!=NULL) {
		    delete dymo_routingTable;
		    dymo_routingTable=NULL;
		}
		dymo_routingTable = new SAMPhO_RoutingTable(this, IPv4Address(myAddr));

		//Reference to routing table as PS-DYMO
		SAMPhO_ref = dynamic_cast<SAMPhO_RoutingTable *> (dymo_routingTable);
	}
}


/*****************************************************************************************
 * Generates and sends a beacon to the surrounding nodes.
 *****************************************************************************************/
void SAMPhO::sendBeacon() {

	//Sort the delay tolerant routing table
	SAMPhO_ref->findNeighbors();

	SAORS_BEACON* bcn =  new SAORS_BEACON("DT-BEACON");

	// do not transmit DYMO messages when we lost our sequence number
	if (ownSeqNumLossTimeout->stopWhenExpired()) {
		ev << "node has lost sequence number -> not transmitting Beacon" << endl;
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

	// Age the delivery probability for all entries{
    for(uint i=0; i<dymo_routingTable->getDTNumRoutes(); i++) {
		SAORSBase_RoutingEntry* entry = SAMPhO_ref->getDTRoute(i);

		// Update probability - no need to update PROPHET
		//SAMPhO_ref->age_Probability(entry);

		//Add neighbours only in the beacon entry field
		if(SAMPhO_ref->isFriend(entry)) {
		    SAORSBase_BeaconBlock bcn_entry(entry->routeAddress.getInt(), entry->routePrefix, entry->deliveryProb, entry->beaconsRcvd, entry->similarity);
		    bcn->getBeaconEntries().push_back(bcn_entry);
		}

		//Add the non stranger routing table entries in the routing entries field -  no need to used almost random meetings
		//if(!SAMPhO_ref->isStranger(entry)) {
		//    SAORSBase_BeaconBlock bcn_entry(entry->routeAddress.getInt(), entry->deliveryProb);
        //    bcn->getDTTableEntries().push_back(bcn_entry);
		//}
	}

	//Set betweenness as the delivery probability
	double betw=SAMPhO_ref->getBetweenness();

	//Record betweenness value
	emit(betweenness, betw);

	//Update the value of the node's ego betweenness
	bcn->setBetw(betw);

	//Send Beacon
	sendDown(bcn, IPv4Address::LL_MANET_ROUTERS.getInt());
}


/*****************************************************************************************
 * Performs the update of the routing entries, according to the beacon received.
 *****************************************************************************************/
void SAMPhO::handleBeacon(SAORS_BEACON* my_beacon) {
    //Total number of packets sent
    int previousPkts=0;

	//Routing entries used below
	SAORSBase_RoutingEntry *entry, *dtentry, *dst_entry;

	//Flag, in case packets were transmitted
	bool flag=false;

	ev << "received message is a Beacon message" << endl;

	// Increase the network density recording
	beaconsReceived++;

	// Routing message pre-processing and updating routes from routing blocks
	if(updateRoutes(dynamic_cast<DYMO_RM*>(my_beacon)) == NULL)
		ev << "Updating routes from Beacon failed" << endl;

	// Update contact probabilities - no need to update PROPHET
	//SAMPhO_ref->adjust_Probabilities(my_beacon->getOrigNode().getAddress(), my_beacon->getBeaconEntries());
	SAMPhO_ref->updateBeaconCounter(my_beacon->getOrigNode().getAddress(), my_beacon->getBeaconEntries());
	SAMPhO_ref->updateSimilarity(my_beacon->getOrigNode().getAddress(), my_beacon->getBeaconEntries());

	//--------------------------------------------------------------------//
	// Check if final destination of any packets in DT queue, sent beacon //
	//--------------------------------------------------------------------//
	dst_entry = SAMPhO_ref->getByAddress(IPv4Address(my_beacon->getOrigNode().getAddress()));
	//Sanity check -- That there is a routing table entry
	if(!dst_entry) {
	    ev << "ERROR: The node that just sent a beacon cannot be found in the routing table!!!" << endl;
	    return;
	}

	//If there is a route to the beacon node
	if( (!dst_entry->routeBroken) && dtqueuedDataPackets->pktsexist(dst_entry->routeAddress, dst_entry->routePrefix)) {
		// An entry was found in the routing table -> get control data from the table
		ev << "Beacon received and we DO have a route" << endl;

		//Dequeue packets and send them to IP
		SAMPhO_ref->maintainAssociatedRoutingTable();
		int numOfPkts=dtqueuedDataPackets->dequeuePacketsTo(dst_entry->routeAddress, dst_entry->routePrefix);
		flag=true;

		//Count this as a DT Message
		DTMesgRcvd -= numOfPkts;
		previousPkts += numOfPkts;
	}

	//--------------------------------------------------------------------//
	//  Check if to fake IP routing table addresses for SD-DYMO purposes  //
	//--------------------------------------------------------------------//
	if(!my_beacon->getBeaconEntries().empty()) {

		// Check all entries of beacon
		for(std::vector<SAORSBase_BeaconBlock>::iterator iter=my_beacon->getBeaconEntries().begin(); iter < my_beacon->getBeaconEntries().end(); iter++) {

			SAORSBase_BeaconBlock iterentry=*iter;

			//Check for an entry in the DYMO routing table
			entry = dymo_routingTable->getByAddress(iterentry.getAddress());

			//Check for an entry in the Delay tolerant routing table
			dtentry=dymo_routingTable->getDTByAddress(iterentry.getAddress());

			//Get the parameters
            double entrySimilarity=0;
            double entryBeacons=0;
            if(dtentry) {
                entrySimilarity=dtentry->similarity;
                entryBeacons=(double)dtentry->beaconsRcvd;
            }


			//If entry only exists in DT routing table and packets for that destination exist in the DT queue
			if( !entry || entry->routeBroken ) {

				//If there are packets for the dt-entry in the DT Packet Queue
				if( dtqueuedDataPackets->pktsexist(iterentry.getAddress(), iterentry.getPrefix()) ) {

				    //Packets sent
				    int numOfPkts=0;

					//Check according to which metric the carrier will be determined
				    if(METRIC == BETW) {
						// Check highest betweenness centrality
						if( SAMPhO_ref->getBetweenness() < my_beacon->getBetw() ) {
							// Dequeue packets to IP
							numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(),
							                                                  iterentry.getPrefix(),
							                                                  dst_entry->routeAddress,
							                                                  previousPkts);
							flag=true;
						}
					}
				    else if(METRIC == SIMILARITY) {
                        // Check highest similarity
                        if( entrySimilarity < iterentry.getSimilarity() ) {
                            // Dequeue packets to IP
                            numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(),
                                                                              iterentry.getPrefix(),
                                                                              dst_entry->routeAddress,
                                                                              previousPkts);
                            flag=true;
                        }
                    }
					else if(METRIC == BB_COMBINE) {
						int maxBeacons=SAMPhO_ref->getMaxBeaconsNumber();
						if( ( SAMPhO_ref->getBetweenness() + ((double)entryBeacons/(double)maxBeacons) ) <
							( my_beacon->getBetw()         + ((double)iterentry.getBeacons()/(double)maxBeacons) ) ) {
							// Dequeue packets to IP
							numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(),
							                                                  iterentry.getPrefix(),
							                                                  dst_entry->routeAddress,
							                                                  previousPkts);
							flag=true;
						}
					}
					else if(METRIC == COMBINE) {
                        int maxBeacons=SAMPhO_ref->getMaxBeaconsNumber();
                        if( ( SAMPhO_ref->getBetweenness() + entrySimilarity           + ((double)entryBeacons/(double)maxBeacons)   ) <
                            ( my_beacon->getBetw()         + iterentry.getSimilarity() + ((double)iterentry.getBeacons()/(double)maxBeacons) ) ) {
                            // Dequeue packets to IP
                            numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(),
                                                                              iterentry.getPrefix(),
                                                                              dst_entry->routeAddress,
                                                                              previousPkts);
                            flag=true;
                        }
                    }
					//For all other configurations
					else {
						//Check the number of Beacons to determined the probability of delivery
						if( entryBeacons < iterentry.getBeacons() ) {
							// Dequeue packets to IP
							numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(),
							                                                  iterentry.getPrefix(),
							                                                  dst_entry->routeAddress,
							                                                  previousPkts);
							flag=true;
						}
					}

					//Count the send DT Messages
                    DTMesgRcvd -= numOfPkts;
                    previousPkts += numOfPkts;
				}
			}
		}
	}

	//--------------------------------------------------------------------//
	//               EXTENDED STAGE ROUTING FOR SAMPhO                    //
	//--------------------------------------------------------------------//
	//If the met node is a high centrality carrier
	if( (METRIC == EXTENDED || METRIC ==  BB_STAGED) ) {

        //--------------------------------------------------------------------//
        // Check for the remaining DT Packets, if there is a valid route or   //
        // if the betweenness of the met node is higher than the current one's//
        //--------------------------------------------------------------------//

	    //Check all the DT Queue for the destinations
        for(int i=dtqueuedDataPackets->getSize()-1;i>=0;i--) {

            //Get destination that the packets are destined
            IPv4Address addr = dtqueuedDataPackets->getDestination(i);

            //If destination exists in the routing table...
            dtentry=dymo_routingTable->getDTByAddress(addr);

            // ----------------------------------------------------- //
            // Notice that the case where a link exists between the  //
            // current node and the destination is not taken into    //
            // consideration. This is done, because that would cause //
            // extra overhead in the network, while it's proven      //
            // that it does not lead to better delivery results.     //
            // ----------------------------------------------------- //
            int numOfPkts=0;

            //For the non copy scheme
            if( METRIC == BB_STAGED ) {
                //If the betweenness of the met node is higher than the current node's and over the threshold
                if( my_beacon->getBetw()>BETW_THRESHOLD && my_beacon->getBetw() > SAMPhO_ref->getBetweenness() ) {

                    //If the message destination is a complete stranger, send the stored messages to the met node
                    if(!(dtentry))  {
                        numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(addr, (dtentry)?dtentry->routePrefix:32, dst_entry->routeAddress, previousPkts);
                        flag=true;

                        //Count this as a DT Message
                        DTMesgRcvd -= numOfPkts;
                    }
                }
            }
            //For the Extended SAMPhO scheme
            else {
                //If both nodes are quite central, copy the message
                if(  my_beacon->getBetw()>BETW_THRESHOLD && SAMPhO_ref->getBetweenness()>BETW_THRESHOLD ) {

                    //If the message destination and the met node are strangers, copy the stored messages to the met node.
                    //This allows spreading of packets when there is no obvious path to be followed.
                    if(SAMPhO_ref->isStranger(dtentry)) {
                        numOfPkts = dtqueuedDataPackets->spreadPacketsTo(addr, (dtentry)?dtentry->routePrefix:32, dst_entry->routeAddress, previousPkts);
                        flag=true;
                    }
                }
                //If not, check who if at least the encounter node is central
                else if( my_beacon->getBetw()>BETW_THRESHOLD && my_beacon->getBetw() > SAMPhO_ref->getBetweenness()) {

                    //If the message destination is a complete stranger, send the stored messages to the met node
                    if(SAMPhO_ref->isStranger(dtentry))  {
                        numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(addr, (dtentry)?dtentry->routePrefix:32, dst_entry->routeAddress, previousPkts);
                        flag=true;

                        //Count this as a DT Message
                        DTMesgRcvd -= numOfPkts;
                    }
                }
            }

            //Count the total sent DT Messages
            previousPkts += numOfPkts;
        }
	}
	//--------------------------------------------------------------------//
    //               END OF EXTENDED ROUTING FOR SAMPhO                   //
    //--------------------------------------------------------------------//

	// No proper destination found
	if(flag==false)
		ev << "Beacon was received but NO route or NO better carrier!" << endl;

	delete my_beacon;
}


/*****************************************************************************************
 * Returns the probability of encountering the node indicated by the SAORS routing entry.
 *****************************************************************************************/
double SAMPhO::findEncounterProb(const SAORSBase_RoutingEntry* routeToNode) {
	//Check according to which metric the carrier will be determined
	if(METRIC == BETW) {
		//Return the the betweenness of the carrier
		return SAMPhO_ref->getBetweenness();
	}
	else if(METRIC == SIMILARITY) {
        //Return the similarity with the searched node
        return routeToNode->similarity;
    }
	else if(METRIC == BB_COMBINE) {
		//Return the number of beacons received from that node
		int maxBeacons=SAMPhO_ref->getMaxBeaconsNumber();
		return ( SAMPhO_ref->getBetweenness() + ((double)routeToNode->beaconsRcvd/(double)maxBeacons) );
	}
	else if(METRIC == COMBINE) {
        //Return the number of beacons received from that node
        int maxBeacons=SAMPhO_ref->getMaxBeaconsNumber();
        return ( SAMPhO_ref->getBetweenness() + routeToNode->similarity + ((double)routeToNode->beaconsRcvd/(double)maxBeacons) );
    }
	else {
		//Return the number of beacons received from that node true
		int maxBeacons=SAMPhO_ref->getMaxBeaconsNumber();
		return (double)((double)routeToNode->beaconsRcvd/(double)maxBeacons);
	}
}


/*****************************************************************************************
 * Returns whether the RREP has a higher or lower  encounter probability.
 *****************************************************************************************/
bool SAMPhO::compareEncounterProb(const SAORSBase_RoutingEntry* dtEntry, const SAORS_RREP* rrep) {
	//Check according to which metric the carrier will be determined
	if(METRIC == BETW) {
		//Compare according to the betweenness of the carrier
		return(SAMPhO_ref->getBetweenness() < rrep->getDeliveryProb());
	}
	if(METRIC == SIMILARITY) {
        //Compare according to the similarity of the carrier
        return(dtEntry->similarity < rrep->getDeliveryProb());
    }
	else if(METRIC == BB_COMBINE) {
	    //Return a combination of the involved metrics
		int maxBeacons=SAMPhO_ref->getMaxBeaconsNumber();
		if( ( SAMPhO_ref->getBetweenness() + ((double)dtEntry->beaconsRcvd/(double)maxBeacons) ) < rrep->getDeliveryProb() )
			return true;
		else
			return false;
	}
	else if(METRIC == COMBINE) {
        //Return a combination of the involved metrics
        int maxBeacons=SAMPhO_ref->getMaxBeaconsNumber();
        if( ( SAMPhO_ref->getBetweenness() + dtEntry->similarity + ((double)dtEntry->beaconsRcvd/(double)maxBeacons) ) < rrep->getDeliveryProb() )
            return true;
        else
            return false;
    }
	else {
		//Compare according to the number of beacons received from that node
		int maxBeacons=SAMPhO_ref->getMaxBeaconsNumber();
		return((double)((double)dtEntry->beaconsRcvd/(double)maxBeacons)<rrep->getDeliveryProb());
	}
}


/*****************************************************************************************
 * Determines whether the node will reply as an intermediate router, to inform of it's
 * encounter probability.
 *****************************************************************************************/
bool SAMPhO::sendEncounterProb(SAORS_RREQ* rreq, const SAORSBase_RoutingEntry* dtEntry) {
	return ( SAMPhO_ref->isFriend(dtEntry) && (rreq->getMinDeliveryProb() < findEncounterProb(dtEntry)) );
}

} // namespace inetmanet

} // namespace inet

