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

#include "SAORSBase_OutstandingRREQList.h"
#include <stdexcept>
#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet {

namespace inetmanet {


std::ostream& operator<<(std::ostream& os, const SAORSBase_OutstandingRREQ& o)
{
	os << "[ ";
	os << "destination: " << o.destAddr << ", ";
	os << "tries: " << o.tries << ", ";
	os << "wait time: " << *o.wait_time << ", ";
	os << "RREP gather time: " << *o.RREPgather_time << ", ";
	os << "creationTime: " << o.creationTime;
	os << " ]";

	return os;
}


/********************************************************************************************
 * Class Constructor
 ********************************************************************************************/
SAORSBase_OutstandingRREQList::SAORSBase_OutstandingRREQList() {
}


/********************************************************************************************
 * Class Destructor
 ********************************************************************************************/
SAORSBase_OutstandingRREQList::~SAORSBase_OutstandingRREQList() {
	delAll();
}


/********************************************************************************************
 * Returns the name of the class.
 ********************************************************************************************/
const char* SAORSBase_OutstandingRREQList::getFullName() const {
	return "SAORSBase_OutstandingRREQList";
}


/********************************************************************************************
 * Returns a character string with the current information of the class.
 ********************************************************************************************/
std::string SAORSBase_OutstandingRREQList::info() const {
	std::ostringstream ss;

	int total = outstandingRREQs.size();
	ss << total << " outstanding RREQs: ";

	ss << "{" << std::endl;
	for (std::vector<SAORSBase_OutstandingRREQ*>::const_iterator iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++) {
		SAORSBase_OutstandingRREQ* e = *iter;
		ss << "  " << *e << std::endl;
	}
	ss << "}";

	return ss.str();
}


/********************************************************************************************
 * Returns a character string with the detailed current information of the class.
 * Currently the same as the SAORSBase_OutstandingRREQList::info() function.
 ********************************************************************************************/
std::string SAORSBase_OutstandingRREQList::str() const {
	return info();
}


/********************************************************************************************
 * Returns SAORSBase_OutstandingRREQ with matching destAddr or 0 if none is found
 ********************************************************************************************/
SAORSBase_OutstandingRREQ* SAORSBase_OutstandingRREQList::getByDestAddr(unsigned int destAddr, int prefix) {
	for(std::vector<SAORSBase_OutstandingRREQ*>::iterator iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++){
		if (IPv4Address(destAddr).prefixMatches(IPv4Address((*iter)->destAddr), prefix))
		    return *iter;
	}
	return NULL;
}


/********************************************************************************************
 * Returns a SAORSBase_OutstandingRREQ whose wait_time is expired or 0 if none is found
 ********************************************************************************************/
SAORSBase_OutstandingRREQ* SAORSBase_OutstandingRREQList::getExpired() {
	for(std::vector<SAORSBase_OutstandingRREQ*>::iterator iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++){
		if (!(*iter)->wait_time->stopWhenExpired())
		    return *iter;
	}
	return 0;
}


/********************************************************************************************
 * Returns a SAORSBase_OutstandingRREQ whose RREPgather_time is expired or 0 if none found.
 ********************************************************************************************/
SAORSBase_OutstandingRREQ* SAORSBase_OutstandingRREQList::getRREPGTExpired() {
	for(std::vector<SAORSBase_OutstandingRREQ*>::iterator iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++){
		if (!(*iter)->RREPgather_time->stopWhenExpired())
		    return *iter;
	}
	return 0;
}


/********************************************************************************************
 * returns true if there is an active time in the outstanding RREQ list
 ********************************************************************************************/
bool SAORSBase_OutstandingRREQList::hasActive() const
{
    for (std::vector<SAORSBase_OutstandingRREQ*>::const_iterator iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++)
    {
        if ((*iter)->wait_time->isActive())
            return true;
    }

    for (std::vector<SAORSBase_OutstandingRREQ*>::const_iterator iter = outstandingRREQs.begin(); iter < outstandingRREQs.end(); iter++)
    {
        if ((*iter)->RREPgather_time->isActive())
            return true;
    }

    return false;
}

/********************************************************************************************
 * Adds the returned RREP to the Carrier selection queue to compare probabilities.
 ********************************************************************************************/
void SAORSBase_OutstandingRREQList::addRREPtoList(SAORS_RREP *rrep) {
	// Look for the OutstandingRREQ corresponding to the received RREP
	SAORSBase_OutstandingRREQ* match = getByDestAddr( rrep->getOrigNode().getAddress(), 32 );

	if(match) {
		// Add the received RREP to the Carrier Selection List
		match->CS_List.addRREPtoList(rrep);
	}
	else
		throw std::runtime_error("received unrequested RREP message");
}


/********************************************************************************************
 * Adds an entry for a specific request to the outstandingRREQs list.
 ********************************************************************************************/
void SAORSBase_OutstandingRREQList::add(SAORSBase_OutstandingRREQ* outstandingRREQ) {
	outstandingRREQs.push_back(outstandingRREQ);
}


/********************************************************************************************
 * Deletes an entry for a specific request from the outstandingRREQs list.
 ********************************************************************************************/
void SAORSBase_OutstandingRREQList::del(SAORSBase_OutstandingRREQ* outstandingRREQ) {
	std::vector<SAORSBase_OutstandingRREQ*>::iterator iter = outstandingRREQs.begin();
	while (iter != outstandingRREQs.end()) {
		if((*iter) == outstandingRREQ) {
			(*iter)->CS_List.clearCSList();
			(*iter)->wait_time->cancel();
			(*iter)->RREPgather_time->cancel();
			delete (*iter)->wait_time;
			delete (*iter)->RREPgather_time;
			delete (*iter);
			iter = outstandingRREQs.erase(iter);
		} else {
			iter++;
		}
	}
}


/********************************************************************************************
 * Empties the outstandingRREQs list from all requests.
 ********************************************************************************************/
void SAORSBase_OutstandingRREQList::delAll() {
	std::vector<SAORSBase_OutstandingRREQ*>::iterator iter = outstandingRREQs.begin();
	while (iter != outstandingRREQs.end()) {
		(*iter)->CS_List.clearCSList();
		(*iter)->wait_time->cancel();
		(*iter)->RREPgather_time->cancel();
		delete (*iter)->wait_time;
		delete (*iter)->RREPgather_time;
		delete (*iter);
		iter = outstandingRREQs.erase(iter);
	}
}


/********************************************************************************************
 * Overloads the operator<< for this class
 ********************************************************************************************/
std::ostream& operator<<(std::ostream& os, const SAORSBase_OutstandingRREQList& o)
{
	os << o.info();
	return os;
}

} // namespace inetmanet

} // namespace inet
