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

#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <math.h>
#include "SAORSBase_RoutingTable.h"
#include "SAORSBase_RoutingEntry.h"
#include "SAORSBase.h"
#include "inet/routing/extras/base/ManetRoutingBase.h"

namespace inet {

namespace inetmanet {


/*****************************************************************************************
 * The Constructor function of the SAORS routing table class.
 *****************************************************************************************/
SAORSBase_RoutingTable::SAORSBase_RoutingTable(cObject* host, const IPv4Address& myAddr)
{
	// get our host module
	if (!host) throw std::runtime_error("No parent module found");

	dymoProcess = host;

	// get our routing table
	// routingTable = IPv4AddressResolver().routingTableOf(host);
	// if (!routingTable) throw std::runtime_error("No routing table found");

	// get our interface table
	// IInterfaceTable *ift = IPv4AddressResolver().interfaceTableOf(host);
	// look at all interface table entries
}


/*****************************************************************************************
 * The Destructor of the DT-DYMOrouting table class.
 *****************************************************************************************/
SAORSBase_RoutingTable::~SAORSBase_RoutingTable() {
	SAORSBase_RoutingEntry* entry;
	while ((entry = getRoute(0))) deleteRoute_destrOnly(entry);
	while ((entry = getDTRoute(0))) deleteDTRoute(entry);
}


/*****************************************************************************************
 * Return the name of the routing table class.
 *****************************************************************************************/
const char* SAORSBase_RoutingTable::getFullName() const {
	return "DT-DYMO_RoutingTable";
}


/*****************************************************************************************
 * Prints out the information conerning the routing table of SAORS.
 *****************************************************************************************/
std::string SAORSBase_RoutingTable::info() const {
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
	ss << "}";

	return ss.str();
}


/*****************************************************************************************
 * Prints out the detailed information concerning the routing table of SAORS.
 *****************************************************************************************/
std::string SAORSBase_RoutingTable::detailedInfo() const {
	return info();
}


/*****************************************************************************************
 * Function returns the size of the table.
 *****************************************************************************************/
uint SAORSBase_RoutingTable::getNumRoutes() const {
  return (int)routeVector.size();
}


/*****************************************************************************************
 * Function returns the size of the delay tolerant routing table.
 *****************************************************************************************/
uint SAORSBase_RoutingTable::getDTNumRoutes() const {
  return (int)dtrouteVector.size();
}


/*****************************************************************************************
 * Function gets an routing entry at the given position.
 *****************************************************************************************/
SAORSBase_RoutingEntry* SAORSBase_RoutingTable::getRoute(int k){
  if(k < (int)routeVector.size())
    return routeVector[k];
  else
    return NULL;
}


/*****************************************************************************************
 * Function gets a delay tolerant routing entry at the given position.
 *****************************************************************************************/
SAORSBase_RoutingEntry* SAORSBase_RoutingTable::getDTRoute(int k){
  if(k < (int)dtrouteVector.size())
    return dtrouteVector[k];
  else
    return NULL;
}


/*****************************************************************************************
 * Function adds a new entry to the table.
 *****************************************************************************************/
void SAORSBase_RoutingTable::addRoute(SAORSBase_RoutingEntry *entry){
	routeVector.push_back(entry);
}


/*****************************************************************************************
 * Function adds a new entry to the delay tolerant routing table.
 *****************************************************************************************/
void SAORSBase_RoutingTable::addDTRoute(SAORSBase_RoutingEntry *entry){

	SAORSBase_RoutingEntry* dt_entry = getDTByAddress(entry->routeAddress);
	// If not already in the table add, else delete entry
	if( dt_entry == 0) {
		dtrouteVector.push_back(entry);
	}
	// Else update DT-DYMO fields of the existing entry
	else
		delete entry;
}


/*****************************************************************************************
 * Function deletes an entry from the routing table.
 *****************************************************************************************/
void SAORSBase_RoutingTable::deleteRoute(SAORSBase_RoutingEntry *entry){

	// update DYMO routingTable
	std::vector<SAORSBase_RoutingEntry *>::iterator iter;
	for(iter = routeVector.begin(); iter < routeVector.end(); iter++){
		if(entry == *iter){
			routeVector.erase(iter);
			(dynamic_cast <SAORSBase*> (dymoProcess))->omnet_chg_rte (L3Address(entry->routeAddress),
			        L3Address(entry->routeAddress),
			        L3Address(entry->routeAddress),
			        0,true);

			// Cancel all previous timers and move entry to delay tolerant routing table
			entry->routeAgeMin.cancel();
			entry->routeAgeMax.cancel();
			entry->routeNew.cancel();
			entry->routeUsed.cancel();
			entry->routeDelete.cancel();
			entry->dtrtDelete.cancel();
			addDTRoute(entry);
			return;
		}
	}

	throw std::runtime_error("unknown routing entry requested to be deleted");
}


/*****************************************************************************************
 * Function deletes an entry from the table in destructor.
 *****************************************************************************************/
void SAORSBase_RoutingTable::deleteRoute_destrOnly(SAORSBase_RoutingEntry *entry){

	// update DYMO routingTable
	std::vector<SAORSBase_RoutingEntry *>::iterator iter;
	for(iter = routeVector.begin(); iter < routeVector.end(); iter++){
		if(entry == *iter){
			routeVector.erase(iter);

			// Cancel all previous timers and move entry to delay tolerant routing table
			entry->routeAgeMin.cancel();
			entry->routeAgeMax.cancel();
			entry->routeNew.cancel();
			entry->routeUsed.cancel();
			entry->routeDelete.cancel();
			addDTRoute(entry);
			return;
		}
	}

	throw std::runtime_error("unknown routing entry requested to be deleted");
}


/*****************************************************************************************
 * Function deletes an entry from the delay tolerant routing table.
 *****************************************************************************************/
void SAORSBase_RoutingTable::deleteDTRoute(SAORSBase_RoutingEntry *entry){

	// update DYMO routingTable
	std::vector<SAORSBase_RoutingEntry *>::iterator iter;
	for(iter = dtrouteVector.begin(); iter < dtrouteVector.end(); iter++){
		if(entry == *iter){
			dtrouteVector.erase(iter);
			delete entry;
			return;
		}
	}

	throw std::runtime_error("unknown routing entry requested to be deleted from delay tolerant routing table");
}


/*****************************************************************************************
 * It calls the maintaining function for every entry of the routing table.
 *****************************************************************************************/
void SAORSBase_RoutingTable::maintainAssociatedRoutingTable() {
	std::vector<SAORSBase_RoutingEntry *>::iterator iter;
	for(iter = routeVector.begin(); iter < routeVector.end(); iter++){
		maintainAssociatedRoutingEntryFor(*iter);
	}
}


/*****************************************************************************************
 * Function searches an entry (exact match) and gives back a pointer to it, or 0 if none
 * is found.
 *****************************************************************************************/
SAORSBase_RoutingEntry* SAORSBase_RoutingTable::getByAddress(IPv4Address addr){

	std::vector<SAORSBase_RoutingEntry *>::iterator iter;

	for(iter = routeVector.begin(); iter < routeVector.end(); iter++){
		SAORSBase_RoutingEntry *entry = *iter;

		if(entry->routeAddress == addr){
			return entry;
		}
	}

	return 0;
}


/*****************************************************************************************
 * Function searches an delay tolerant entry (exact match) and gives back a pointer to
 * it, or 0.
 *****************************************************************************************/
SAORSBase_RoutingEntry* SAORSBase_RoutingTable::getDTByAddress(IPv4Address addr){

	std::vector<SAORSBase_RoutingEntry *>::iterator iter;

	for(iter = dtrouteVector.begin(); iter < dtrouteVector.end(); iter++){
		SAORSBase_RoutingEntry *entry = *iter;

		if(entry->routeAddress == addr){
			return entry;
		}
	}

	return NULL;
}


/*****************************************************************************************
 * Function searches an entry (longest-prefix match) and gives back a pointer to it, or
 * 0 if none.
 *****************************************************************************************/
SAORSBase_RoutingEntry* SAORSBase_RoutingTable::getForAddress(IPv4Address addr) {
	std::vector<SAORSBase_RoutingEntry *>::iterator iter;

	int longestPrefix = 0;
	SAORSBase_RoutingEntry* longestPrefixEntry = 0;
	for(iter = routeVector.begin(); iter < routeVector.end(); iter++) {
		SAORSBase_RoutingEntry *entry = *iter;

		// skip if we already have a more specific match
		if (!(entry->routePrefix > longestPrefix)) continue;

		// skip if address is not in routeAddress/routePrefix block
		if (!addr.prefixMatches(entry->routeAddress, entry->routePrefix)) continue;

		// we have a match
		longestPrefix = entry->routePrefix;
		longestPrefixEntry = entry;
	}

	return longestPrefixEntry;
}


/*****************************************************************************************
 * Function returns the routing table.
 *****************************************************************************************/
SAORSBase_RoutingTable::RouteVector SAORSBase_RoutingTable::getRoutingTable(){
	return routeVector;
}


/*****************************************************************************************
 * Function returns the delay tolerant routing table.
 *****************************************************************************************/
SAORSBase_RoutingTable::RouteVector SAORSBase_RoutingTable::getDTRoutingTable(){
	return dtrouteVector;
}


/*****************************************************************************************
 * Add or delete network layer routing table entry for given DYMO routing table entry,
 * based on whether it's valid.
 *****************************************************************************************/
void SAORSBase_RoutingTable::maintainAssociatedRoutingEntryFor(SAORSBase_RoutingEntry* entry){
	if (!entry->routeBroken) {
		// entry is valid
	    (dynamic_cast <SAORSBase*> (dymoProcess))->setIpEntry (L3Address(entry->routeAddress),
	            L3Address(entry->routeNextHopAddress),
	            L3Address(IPv4Address::ALLONES_ADDRESS),
	            entry->routeDist);

	}
	else
	{
		(dynamic_cast <SAORSBase*> (dymoProcess))->deleteIpEntry(L3Address(entry->routeAddress));
	}
}


std::ostream& operator<<(std::ostream& os, const SAORSBase_RoutingTable& o)
{
	os << o.info();
	return os;
}

} // namespace inetmanet

} // namespace inet
