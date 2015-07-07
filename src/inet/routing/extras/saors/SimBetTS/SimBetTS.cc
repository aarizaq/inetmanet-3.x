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

#include "SimBetTS.h"

namespace inet {

namespace inetmanet {

Define_Module(SimBetTS);


#define DYMO_PORT 653


using namespace SAORSns;


/*****************************************************************************************
 * The initialization function of the class.
 *****************************************************************************************/
void SimBetTS::initialize(int stage)
{
	if (stage == 4)
	{
		//Initialize DYMO class
	    SAORSBase::initialize(stage);

		//Register betweenness signal
		betweenness = registerSignal("Betweenness");

		// Set PS-DYMO time parameters
		SOCIAL_SENSITIVITY = par("SOCIAL_SENSITIVITY");
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
		else if (mtr_par=="COPY")
		     METRIC = COPY;
		else
            METRIC = COMBINE;

		//Generate the DS-DYMO routing Table as the SAORSBase Routing Table
		if(dymo_routingTable!=NULL) {
            delete dymo_routingTable;
            dymo_routingTable=NULL;
        }
		dymo_routingTable = new SimBetTS_RoutingTable(this, IPv4Address(myAddr));

		//Reference to routing table as PS-DYMO
		SimBetTS_ref = dynamic_cast<SimBetTS_RoutingTable *> (dymo_routingTable);
	}
}


/*****************************************************************************************
 * Generates and sends a beacon to the surrounding nodes.
 *****************************************************************************************/
void SimBetTS::sendBeacon() {

	//Sort the delay tolerant routing table
	SimBetTS_ref->findNeighbors();

	SAORS_BEACON* bcn =  new SAORS_BEACON("DT-BEACON");

	// do not transmit DYMO messages when we lost our sequence number
	if (ownSeqNumLossTimeout->isRunning()) {
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
		SAORSBase_RoutingEntry* entry = SimBetTS_ref->getDTRoute(i);

		//Add neighbours only in the beacon entry field
		if(SimBetTS_ref->isFriend(entry)) {
		    SAORSBase_BeaconBlock bcn_entry(entry->routeAddress.getInt(), entry->routePrefix, entry->deliveryProb, entry->beaconsRcvd, entry->similarity);
		    bcn->getBeaconEntries().push_back(bcn_entry);
		}

		//Add the non stranger routing table entries in the routing entries field -  no need to used almost random meetings
		//if(!SimBetTS_ref->isStranger(entry)) {
		//    SAORSBase_BeaconBlock bcn_entry(entry->routeAddress.getInt(), entry->deliveryProb);
        //    bcn->getDTTableEntries().push_back(bcn_entry);
		//}
	}

	//Set betweenness as the delivery probability
	double betw=SimBetTS_ref->getBetweenness();

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
void SimBetTS::handleBeacon(SAORS_BEACON* my_beacon) {
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
	//SimBetTS_ref->adjust_Probabilities(my_beacon->getOrigNode().getAddress(), my_beacon->getBeaconEntries());
	SimBetTS_ref->updateBeaconCounter(my_beacon->getOrigNode().getAddress(), my_beacon->getBeaconEntries());
	SimBetTS_ref->updateSimilarity(my_beacon->getOrigNode().getAddress(), my_beacon->getBeaconEntries());

	//--------------------------------------------------------------------//
	// Check if final destination of any packets in DT queue, sent beacon //
	//--------------------------------------------------------------------//
	dst_entry = SimBetTS_ref->getByAddress(IPv4Address(my_beacon->getOrigNode().getAddress()));

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
		SimBetTS_ref->maintainAssociatedRoutingTable();
		int numOfPkts=dtqueuedDataPackets->dequeuePacketsTo(dst_entry->routeAddress, dst_entry->routePrefix);
		flag=true;

		//Count this as a DT Message
		DTMesgRcvd -= numOfPkts;
		previousPkts += numOfPkts;
	}

	//--------------------------------------------------------------------//
	// Check if opportunistic routing is necessary, according to SimBetTS //
	//--------------------------------------------------------------------//
	//Check all the DT Queue for the destinations
    for(int i=dtqueuedDataPackets->getSize()-1;i>=0;i--) {
        //Get destination that the packets are destined
        IPv4Address addr = dtqueuedDataPackets->getDestination(i);

        //Check for an entry in the DYMO routing table
        entry = dymo_routingTable->getByAddress(addr);

        //If destination exists in the routing table...
        dtentry=dymo_routingTable->getDTByAddress(addr);

        //Try to find the entry in the beacon social information
        SAORSBase_BeaconBlock *iterentry = NULL;
        for(std::vector<SAORSBase_BeaconBlock>::iterator iter=my_beacon->getBeaconEntries().begin(); iter < my_beacon->getBeaconEntries().end(); iter++) {

            //If found stop searching
            if(iter->getAddress() == addr) {
                iterentry= &(*iter);
                break;
            }
        }

        //Get the parameters from the routing entry
        double entrySimilarity=0;
        double entryMetric=0;

        if(dtentry) {
            entrySimilarity=(double)dtentry->similarity;
            entryMetric=(double)dtentry->beaconsRcvd;
        }
        //Get the parameters from the beacon entry
        double beaconSimilarity=0;
        double beaconMetric=0;
        if(iterentry!=NULL) {
            beaconSimilarity=(double)iterentry->getSimilarity();
            beaconMetric=(double)iterentry->getBeacons();
        }

        //If entry only exists in DT routing table and packets for that destination exist in the DT queue
        if( (!entry || entry->routeBroken) ) {
            //Packets sent
            int numOfPkts=0;
            //Check according to which metric the carrier will be determined
            if(METRIC == BETW) {
                // Check highest betweenness centrality
                if( SimBetTS_ref->getBetweenness() < my_beacon->getBetw() ) {
                    // Dequeue packets to IP
                    numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(addr, (iterentry)?iterentry->getPrefix():32, dst_entry->routeAddress, previousPkts);
                    flag=true;
                }
            }
            else if(METRIC == SIMILARITY) {
                // Check highest similarity
                if( entrySimilarity < beaconSimilarity ) {
                    // Dequeue packets to IP
                    numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(addr, (iterentry)?iterentry->getPrefix():32, dst_entry->routeAddress, previousPkts);
                    flag=true;
                }
            }
            else if(METRIC == BB_COMBINE) {
                int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
                if( ( SimBetTS_ref->getBetweenness()*0.5 + ((double)entryMetric/(double)maxBeacons)*0.5 ) <
                    ( my_beacon->getBetw()*0.5           + ((double)beaconMetric/(double)maxBeacons)*0.5 ) ) {
                    // Dequeue packets to IP
                    numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(addr, (iterentry)?iterentry->getPrefix():32, dst_entry->routeAddress, previousPkts);
                    flag=true;
                }
            }
            else if(METRIC == COMBINE) {
                int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
                if( ( SimBetTS_ref->getBetweenness() /*+ entrySimilarity*/  + ((double)entryMetric/(double)maxBeacons) ) <
                    ( my_beacon->getBetw()           /*+ beaconSimilarity*/ + ((double)beaconMetric/(double)maxBeacons) ) ) {
                    // Dequeue packets to IP
                    numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(addr, (iterentry)?iterentry->getPrefix():32, dst_entry->routeAddress, previousPkts);
                    flag=true;
                }
            }
            else if(METRIC == COPY) {
                int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
                if( ( SimBetTS_ref->getBetweenness() + entrySimilarity  + ((double)entryMetric/(double)maxBeacons) ) <
                    ( my_beacon->getBetw()           + beaconSimilarity + ((double)beaconMetric/(double)maxBeacons) ) ) {
                    // Dequeue packets to IP
                    numOfPkts = dtqueuedDataPackets->spreadPacketsTo(addr, (iterentry)?iterentry->getPrefix():32, dst_entry->routeAddress, previousPkts);
                    flag=true;

                    //Add since they will be removed as non-copies
                    DTMesgRcvd += numOfPkts;
                }
            }
            //For all other configurations
            else {
                //Check the number of Beacons to determined the probability of delivery
                if( entryMetric < beaconMetric ) {
                    //std::cout <<  simTime() << ": " <<  entryMetric << " vs " << beaconMetric << endl;
                    // Dequeue packets to IP
                    numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(addr, (iterentry)?iterentry->getPrefix():32, dst_entry->routeAddress, previousPkts);
                    flag=true;
                }
            }

            //Count the send DT Messages
            DTMesgRcvd -= numOfPkts;
            previousPkts += numOfPkts;
        }
    }
	//--------------------------------------------------------------------//

	/**
	 *  This technique only checks the beacon entries, therefore not so similar to SimBetTS
	 *  However it provides better results in conjunction with the Social Sensitivity.
	 *  It can be used for further testing.
	 *
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
			double entryMetric=0;
			if(dtentry) {
			    entrySimilarity=dtentry->similarity;
			    entryMetric=(double)dtentry->beaconsRcvd;
			}

			//If entry only exists in DT routing table and packets for that destination exist in the DT queue
			if( (!entry || entry->routeBroken) ) {

				//If there are packets for the dt-entry in the DT Packet Queue
				if( dtqueuedDataPackets->pktsexist(iterentry.getAddress(), iterentry.getPrefix()) ) {

				    //Packets sent
				    int numOfPkts=0;

					//Check according to which metric the carrier will be determined
				    if(METRIC == BETW) {
						// Check highest betweenness centrality
						if( SimBetTS_ref->getBetweenness() < my_beacon->getBetw() ) {
							// Dequeue packets to IP
							numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(), iterentry.getPrefix(), dst_entry->routeAddress, previousPkts);
							flag=true;
						}
					}
				    else if(METRIC == SIMILARITY) {
                        // Check highest similarity
                        if( entrySimilarity < iterentry.getSimilarity() ) {
                            // Dequeue packets to IP
                            numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(), iterentry.getPrefix(), dst_entry->routeAddress, previousPkts);
                            flag=true;
                        }
                    }
					else if(METRIC == BB_COMBINE) {
						int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
						if( ( SimBetTS_ref->getBetweenness()*0.5 + entrySimilarity*0.5 ) <
							( my_beacon->getBetw()*0.5 + iterentry.getSimilarity()*0.5  ) ) {
							// Dequeue packets to IP
							numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(), iterentry.getPrefix(), dst_entry->routeAddress, previousPkts);
							flag=true;
						}
					}
					else if(METRIC == COMBINE) {
                        int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
                        if( ( SimBetTS_ref->getBetweenness()*0.3 + entrySimilarity*0.3       + ((double)entryMetric/(double)maxBeacons)*0.3   ) <
                            ( my_beacon->getBetw()*0.3           + iterentry.getSimilarity()*0.3 + ((double)iterentry.getBeacons()/(double)maxBeacons)*0.3 ) ) {
                            // Dequeue packets to IP
                            numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(), iterentry.getPrefix(), dst_entry->routeAddress, previousPkts);
                            flag=true;
                        }
                    }
					//For all other configurations
					else {
						//Check the number of Beacons to determined the probability of delivery
						if( entryMetric < iterentry.getBeacons() ) {
						    std::cout <<  simTime() << ": " << entryMetric << " vs " << iterentry.getBeacons() << endl;
							// Dequeue packets to IP
							numOfPkts = dtqueuedDataPackets->dequeuePacketsTo(iterentry.getAddress(), iterentry.getPrefix(), dst_entry->routeAddress, previousPkts);
							flag=true;
						}
					}

					//Count the send DT Messages
                    DTMesgRcvd -= numOfPkts;
                    previousPkts += numOfPkts;
				}

			}
		}
	}*/


	// No proper destination found
	if(flag==false)
		ev << "Beacon was received but NO route or NO better carrier!" << endl;

	delete my_beacon;
}


/*****************************************************************************************
 * Returns the probability of encountering the node indicated by the SAORS routing entry.
 *****************************************************************************************/
double SimBetTS::findEncounterProb(const SAORSBase_RoutingEntry* routeToNode) {
	//Check according to which metric the carrier will be determined
	if(METRIC == BETW) {
		//Return the the betweenness of the carrier
		return SimBetTS_ref->getBetweenness();
	}
	else if(METRIC == SIMILARITY) {
        //Return the similarity with the searched node
        return routeToNode->similarity;
    }
	else if(METRIC == BB_COMBINE) {
		//Return the number of beacons received from that node
		int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
		return ( SimBetTS_ref->getBetweenness()*0.5 + ((double)routeToNode->beaconsRcvd/(double)maxBeacons)*0.5 );
	}
	else if(METRIC == COMBINE || METRIC == COPY) {
        //Return the number of beacons received from that node
        int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
        return ( SimBetTS_ref->getBetweenness() /*+ routeToNode->similarity*/ + ((double)routeToNode->beaconsRcvd/(double)maxBeacons) );
    }
	else {
		//Return the number of beacons received from that node true
		int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
		return (double)((double)routeToNode->beaconsRcvd/(double)maxBeacons);
	}
}


/*****************************************************************************************
 * Returns whether the RREP has a higher or lower  encounter probability.
 *****************************************************************************************/
bool SimBetTS::compareEncounterProb(const SAORSBase_RoutingEntry* dtEntry, const SAORS_RREP* rrep) {
	//Check according to which metric the carrier will be determined
	if(METRIC == BETW) {
		//Compare according to the betweenness of the carrier
		return(SimBetTS_ref->getBetweenness() < rrep->getDeliveryProb());
	}
	if(METRIC == SIMILARITY) {
        //Compare according to the similarity of the carrier
        return(dtEntry->similarity < rrep->getDeliveryProb());
    }
	else if(METRIC == BB_COMBINE) {
	    //Return a combination of the involved metrics
		int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
		if( ( SimBetTS_ref->getBetweenness()*0.5 + ((double)dtEntry->beaconsRcvd/(double)maxBeacons)*0.5 ) < rrep->getDeliveryProb() )
			return true;
		else
			return false;
	}
	else if(METRIC == COMBINE || METRIC == COPY) {
        //Return a combination of the involved metrics
        int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
        if( ( SimBetTS_ref->getBetweenness() /*+ dtEntry->similarity*/ + ((double)dtEntry->beaconsRcvd/(double)maxBeacons) ) < rrep->getDeliveryProb() )
            return true;
        else
            return false;
    }
	else {
		//Compare according to the number of beacons received from that node
		int maxBeacons=SimBetTS_ref->getMaxBeaconsNumber();
		return((double)((double)dtEntry->beaconsRcvd/(double)maxBeacons)<rrep->getDeliveryProb());
	}
}


/*****************************************************************************************
 * Determines whether the node will reply as an intermediate router, to inform of it's
 * encounter probability.
 *****************************************************************************************/
bool SimBetTS::sendEncounterProb(SAORS_RREQ* rreq, const SAORSBase_RoutingEntry* dtEntry) {
	return ( SimBetTS_ref->isFriend(dtEntry) && (rreq->getMinDeliveryProb() < findEncounterProb(dtEntry)) );
}

} // namespace inetmanet

} // namespace inet
