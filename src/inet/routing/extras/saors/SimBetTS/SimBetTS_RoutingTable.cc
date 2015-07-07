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
#include "SimBetTS.h"
#include "SimBetTS_RoutingTable.h"
#include "inet/routing/extras/base/ManetRoutingBase.h"

namespace inet {

namespace inetmanet {

namespace {
 const double GAMMA = 0.9;
 const double BETA = 0.25;
 const double BEACON_COUNTER_WINDOW = 15000;
 const uint MIN_RT_SIZE = 6;
 const double BEACON_THRESHOLD = 1.1;
}


bool compare_function(SAORSBase_RoutingEntry *entry1, SAORSBase_RoutingEntry *entry2) { return (entry1->beaconsRcvd < entry2->beaconsRcvd); }


/*****************************************************************************************
 * The Constructor function of the PS-DYMO routing table class.
 *****************************************************************************************/
SimBetTS_RoutingTable::SimBetTS_RoutingTable(cObject* host, const IPv4Address& myAddr) : SAORSBase_RoutingTable(host, myAddr)
{
	// get our host module
	if (!host) throw std::runtime_error("No parent module found");

	dymoProcess = host;

	//Initialize the host betweenness
	betw=-1;
}


/*****************************************************************************************
 * The Destructor of the PS-DYMOrouting table class.
 *****************************************************************************************/
SimBetTS_RoutingTable::~SimBetTS_RoutingTable() {
	SAORSBase_RoutingEntry* entry;
	while ((entry = getRoute(0))) deleteRoute_destrOnly(entry);
	while ((entry = getDTRoute(0))) deleteDTRoute(entry);
}


/*****************************************************************************************
 * Prints out the information concerning the routing table of DS-DYMO.
 *****************************************************************************************/
std::string SimBetTS_RoutingTable::info() const {
	std::ostringstream ss;

	ss << getNumRoutes() << " entries";

	int broken = 0;
	for (std::vector<SAORSBase_RoutingEntry *>::const_iterator iter = routeVector.begin(); iter < routeVector.end(); iter++) {
	    SAORSBase_RoutingEntry* e = *iter;
		if (e->routeBroken) broken++;
	}
	ss << " (" << broken << " broken)";

	ss << " {" << std::endl;
	for (std::vector<SAORSBase_RoutingEntry *>::const_iterator iter = routeVector.begin(); iter < routeVector.end(); iter++) {
	    SAORSBase_RoutingEntry* e = *iter;
		ss << "  " << *e << std::endl;
	}
	ss << "}" << endl;

	//Print Delay tolerant entries as well
	ss << getDTNumRoutes() << " delay tolerant entries";

    ss << " {" << std::endl;
    for (std::vector<SAORSBase_RoutingEntry *>::const_iterator iter = dtrouteVector.begin(); iter < dtrouteVector.end(); iter++) {
        SAORSBase_RoutingEntry* e = *iter;
        ss << "  " << *e << std::endl;
    }
	ss << "}" << endl;

	//Print the Ego-Adjacency information
	ss <<  egoInfo.getEgoAdjacencySize() << " entries in Ego-Adjacency Matrix";
	ss << " {" << std::endl;
	ss << egoInfo;
	ss << "}";

	return ss.str();
}


/*****************************************************************************************
 * Ages all the DT-entries' probabilities in the DS-DYMO field of the entries.
 *****************************************************************************************/
void SimBetTS_RoutingTable::ageAllBeacons() {

    SAORSBase_RoutingEntry *entry=NULL;

	double timeout = dynamic_cast <SimBetTS*> (dymoProcess)->BEACON_TIMEOUT;

	for(RouteVectorIter iter=dtrouteVector.begin();iter!=dtrouteVector.end();iter++) {
		entry=*iter;

		//Compute the meeting intervals
		double meeting_interval = simTime().dbl() - entry->last_seen.dbl();

		//Compute the average time that a next meeting should take place
		double avgInterval = (BEACON_COUNTER_WINDOW - entry->beaconsRcvd*timeout) / (entry->meetings);

		if ( meeting_interval > 2*avgInterval) {
			entry->meetings /= 2;
			entry->beaconsRcvd /= 2;
		}

		//Beacon counters cannot be lower than 1
		if(entry->beaconsRcvd<1)
			entry->beaconsRcvd=1;
		if(entry->meetings<1)
			entry->meetings=1;
	}
}


/*****************************************************************************************
 * Determines the social neighbors of the host.
 *****************************************************************************************/
void SimBetTS_RoutingTable::findNeighbors() {

	//Age all entries to refresh the beacon counters
	ageAllBeacons();

	//Calculate the past betweenness metric
	betw=egoInfo.calculateBetweenness();

	//Sort the DT-DYMO routing table in an ascending order
	sort(dtrouteVector.begin(),dtrouteVector.end(),compare_function);

	//Reverse the table to get the descending order
	reverse(dtrouteVector.begin(),dtrouteVector.end());

	//Find out how many contacts will be considered as friends
	uint numOfFriends=getNumOfNeighbors();

	//----------------------------------------------------------------------------------------------------
	//---- Fix the Ego Adjacency metric to contain only the beacon information for the friendly nodes ----
	//----------------------------------------------------------------------------------------------------
	std::vector<SAORSBase_RoutingEntry *>::iterator iter;

	//For all the social neighbors found, initialize the Ego Adjacency beacon information
	for(iter=dtrouteVector.begin();iter<dtrouteVector.begin()+numOfFriends;iter++) {
		//If the address is not found, add it
		if(egoInfo.findBeaconInfo((*iter)->routeAddress.getInt()) == NULL) {
			VectorOfSAORSBeaconBlocks emptyVector;
			egoInfo.proccessBeaconInfo((*iter)->routeAddress.getInt(),emptyVector);
		}
	}

	//Delete the beacon information of nodes which are not social neighbors any more
	for(int i=0;i<egoInfo.getEgoAdjacencySize();i++) {
		//Get the stored beacon information from the selected entry
		bcnInfoPair* info=egoInfo.getBeaconInfo(i);

		//Find the position of the entry in the delay tolerant routing table
		int ranking=findEntryRank(info->first);

		//If the entry is not found or it doesn't a defines a social neighbor
		if(ranking<0 || ranking>=(int)numOfFriends) {
			egoInfo.removeBeaconInfo(info->first);
		}
	}
	//----------------------------------------------------------------------------------------------------

	//---------- Construct the ego adjacency matrix file ----------
	//Get the base IP Address
	/*uint base_IP = IPAddress(simulation.getModuleByPath("flatNetworkConfigurator")->par("networkAddress").stringValue()).getInt();

	//If the delay-tolerant routing table has enough entries
	if(dtrouteVector.size()>MIN_RT_SIZE && dtrouteVector.size()>numOfFriends) {
		//Initialize the adjacency line and the ID of this node
		int adjLine[int(simulation.getSystemModule()->par("numHosts"))];
		for(uint i=0;i<sizeof(adjLine)/sizeof(int);i++)
			adjLine[i]=0;
		int myID = dynamic_cast <SimBetTS*> (dymoProcess)->myAddr - base_IP;

		//Initialize the output file, be careful of the name though...!
		std::stringstream fileName;
		fileName << "adjLine";
		char prefix[]=".idat";
		if(myID>9)
		    fileName << myID;
		else
		    fileName << "0" << myID;
		fileName << prefix;


		//Open File
		std::ofstream myfile;
		myfile.open(fileName.str().c_str());

		//Read the chosen delay-tolerant routing entries
		for(int i=0;i<egoInfo.getEgoAdjacencySize();i++) {
			//Find the node ID
			int nodeID=egoInfo.getBeaconEntry(i) - base_IP;

			//Sanity Check
			if( nodeID<0 || nodeID>int(simulation.getSystemModule()->par("numHosts")) )
				throw std::runtime_error("Node ID in social network generation not recognized!!!");

			adjLine[nodeID]=1;
		}

		//Write output to the temp file
		for(uint i=0;i<sizeof(adjLine)/sizeof(int);i++)
			myfile << adjLine[i] << " ";

		//Close file
		myfile.close();
	}
	//Else wait until the host makes more contacts
	else {
		ev << "The routing table is too small to provide clear social information!" << endl;
	}*/
	//------------------------------------------------------
}


/*****************************************************************************************
 * Returns the number of the social friends of the host.
 *****************************************************************************************/
uint SimBetTS_RoutingTable::getNumOfNeighbors() {
//	uint numOfFriends=floor((dynamic_cast <SimBetTS*> (dymoProcess)->SOCIAL_SENSITIVITY)*dtrouteVector.size()/100);

//This code would calculate the social neighbors according to the metric of the most close friend
	uint numOfFriends=0;

	//If the DT Routing Vector is not empty
	if(!dtrouteVector.empty()) {
		//Get the first entry
		std::vector<SAORSBase_RoutingEntry *>::iterator iter = dtrouteVector.begin();

		//Get the threshold for considering a node friendly
		double threshold = (*iter)->beaconsRcvd - ((*iter)->beaconsRcvd*(dynamic_cast <SimBetTS*> (dymoProcess)->SOCIAL_SENSITIVITY)/100);

		//Count the nodes that are considered as friends
		for(iter=dtrouteVector.begin();iter<dtrouteVector.end();++iter) {
			if((*iter)->beaconsRcvd > threshold)
				numOfFriends++;
		}
	}

	return(numOfFriends);
}



/*****************************************************************************************
 * Returns whether a certain routing entry describes a friend of the current node or not.
 *****************************************************************************************/
bool SimBetTS_RoutingTable::isFriend(const SAORSBase_RoutingEntry *entry) {
    if(entry) {
        //Check is the received entry is member of the neighbors groups
        if( findEntryRank(entry->routeAddress.getInt()) < getNumOfNeighbors() )
            return true;
    }

	//Else return false
    return false;
}



/*****************************************************************************************
 * Locates the entry rank for a given address, returns negative if not found.
 *****************************************************************************************/
uint SimBetTS_RoutingTable::findEntryRank(uint32_t Address) {
	uint rank=0;
	std::vector<SAORSBase_RoutingEntry *>::iterator iter;

	//For all the social neighbors found check the Ego Adjacency beacon information
	for(iter=dtrouteVector.begin();iter<dtrouteVector.end();iter++) {
		if((*iter)->routeAddress.getInt()==Address)
			return(rank);
		rank++;
	}

	return(-1);
}


/*****************************************************************************************
 * Returns the betweenness centrality value of the node.
*****************************************************************************************/
double SimBetTS_RoutingTable::getBetweenness() {
	//Check  is betweenness has a valid value, else return 0
	if(betw<=0)
		return 0;
	else
		return betw/(( (double)pow( (double)getNumOfNeighbors(), 2)-(double)getNumOfNeighbors() )/2);
}


/*****************************************************************************************
 * Updates the beacon counter of the specified entry, according to the maximum time window.
 *****************************************************************************************/
void SimBetTS_RoutingTable::updateBeaconCounter(uint32_t Address, VectorOfSAORSBeaconBlocks entryVector) {

    //Process the beacon information of the ego network if it came from a social neighbor
    std::vector<SAORSBase_RoutingEntry *>::iterator iter;
    for(iter=dtrouteVector.begin();iter<dtrouteVector.begin()+getNumOfNeighbors();iter++) {
        if(Address==(*iter)->routeAddress.getInt()) {
            egoInfo.proccessBeaconInfo(Address,entryVector);
            continue;
        }
    }

    // Looking for requested Address first in Delay-tolerant routing table and then in DYMO
    SAORSBase_RoutingEntry* entry = getDTByAddress(IPv4Address(Address));
    if(!entry)
        entry = getByAddress(IPv4Address(Address));

    //If an entry exists
    if(entry) {

        //Compute Interval
        double meeting_interval = simTime().dbl() - entry->last_seen.dbl();

        //Compute the average time that a next meeting should take place
        double avgInterval = (BEACON_COUNTER_WINDOW - entry->beaconsRcvd*dynamic_cast <SimBetTS*> (dymoProcess)->BEACON_TIMEOUT) / (entry->meetings);

        //Record this meeting
        entry->last_seen = simTime();

        //Increase the number of beacons received
        entry->beaconsRcvd++;

        //Check is the recorded beacons are more than the maximum
        if(entry->beaconsRcvd > getMaxBeaconsNumber() )
            entry->beaconsRcvd = getMaxBeaconsNumber() - 1;

        //If the measured interval is equal or smaller than the beacon interval, then this is the continuation
        //of a past meeting, so do nothing
        if( meeting_interval > 2*dynamic_cast <SimBetTS*> (dymoProcess)->BEACON_TIMEOUT*BEACON_THRESHOLD) {
            //If the measured interval is less than the expected interval, then increase the meetings counter.
            if ( meeting_interval < 2*avgInterval ) {
                //If the counters haven't exceeded the maximum, increase the metrics
                if( entry->meetings < entry->beaconsRcvd )
                    entry->meetings++;
            }
        }

        //Beacon counters cannot be lower than 1
        if(entry->beaconsRcvd<1)
            entry->beaconsRcvd=1;
        if(entry->meetings<1)
            entry->meetings=1;
    }
    else {
        ev << "Unknown routing entry requested to updated beacon info of delay tolerant routing table..." << endl;
    }
}


/*****************************************************************************************
 * Updates the similarity metric for the specified entry, according to the friends of the
 * met node.
 *****************************************************************************************/
void SimBetTS_RoutingTable::updateSimilarity(uint32_t Address, VectorOfSAORSBeaconBlocks entryVector) {

    // Looking for requested Address first in Delay-tolerant routing table and then in DYMO
    SAORSBase_RoutingEntry* entry = getDTByAddress(IPv4Address(Address));
    if(!entry)
        entry = getByAddress(IPv4Address(Address));

    //If an entry exists
    if(entry) {
        entry->similarity=getSimilarityScore(entryVector);
    }
    else {
        ev << "Unknown routing entry requested to updated beacon info of delay tolerant routing table..." << endl;
    }
}


/*****************************************************************************************
 * Returns the number of maximum beacons per counting interval.
 *****************************************************************************************/
uint SimBetTS_RoutingTable::getMaxBeaconsNumber() {
	return(floor(BEACON_COUNTER_WINDOW/dynamic_cast <SimBetTS*> (dymoProcess)->BEACON_TIMEOUT));
}


/*****************************************************************************************
 * Returns the neighbor similarity between the node and the carrier sending the beacon.
 *****************************************************************************************/
double SimBetTS_RoutingTable::getSimilarityScore(VectorOfSAORSBeaconBlocks beaconEntries) {
    //The  similarity metric
    double similarity=0;

    //For all the entries in the beacon info field
    for(std::vector<SAORSBase_BeaconBlock>::iterator iter=beaconEntries.begin(); iter < beaconEntries.end(); iter++) {
        //Check if the entry is also a friend of the current node
        if(egoInfo.isCommuninityMenber(iter->getAddress().getInt())) {
            similarity++;
        }
    }

    //Get the percentage
    if(beaconEntries.size()!=0)
        similarity=(similarity/beaconEntries.size());
    else
        similarity=0;

    //Return the score
    return similarity;
}



std::ostream& operator<<(std::ostream& os, const SimBetTS_RoutingTable& o)
{
	os << o.info();
	return os;
}

} // namespace inetmanet

} // namespace inet

