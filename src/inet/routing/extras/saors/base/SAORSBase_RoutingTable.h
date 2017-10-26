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

#ifndef SAORSBASE_ROUTINGTABLE_H
#define SAORSBASE_ROUTINGTABLE_H

#include <omnetpp.h>
#include <vector>
#include "inet/common/INETDefs.h"
#include "SAORS_BEACON_m.h"
#include "SAORSBase_RoutingEntry.h"

namespace inet {

namespace inetmanet {


/********************************************************************************************
 * class SAORSBase_RoutingTable: Describes the functionality of the routing table
 ********************************************************************************************/
class SAORSBase_RoutingTable : public cObject
{
	public:
		/** Class Constructor */
		SAORSBase_RoutingTable(cObject* host, const IPv4Address& myAddr, const char* DYMO_INTERFACES, const IPv4Address& LL_MANET_ROUTERS){SAORSBase_RoutingTable(host,myAddr);}

		/** Class Constructor */
		SAORSBase_RoutingTable(cObject* host, const IPv4Address& myAddr);

		/** Class Destructor */
		virtual ~SAORSBase_RoutingTable();

		/** @brief inherited from cObject */
		virtual const char* getFullName() const override;

		/** @brief inherited from cObject */
		virtual std::string info() const override;

		/** @brief inherited from cObject */
		virtual std::string str() const override;

		//-----------------------------------------------------------------------
		//Route table manipulation operations
		//-----------------------------------------------------------------------
		/** @brief returns the size of the table **/
		virtual uint getNumRoutes() const;

		/** @brief returns the size of the delay tolerant  table **/
		virtual uint getDTNumRoutes() const;

		/** @brief gets an routing entry at the given position **/
		virtual SAORSBase_RoutingEntry* getRoute(int k);

		/** @brief gets a delay tolerant routing entry at the given position **/
		virtual SAORSBase_RoutingEntry* getDTRoute(int k);

		/** @brief adds a new entry to the table **/
		virtual void addRoute(SAORSBase_RoutingEntry *entry);

		/** @brief adds a new entry to the delay tolerant routing table **/
		virtual void addDTRoute(SAORSBase_RoutingEntry *entry);

		/** @brief deletes an entry from the table **/
		virtual void deleteRoute (SAORSBase_RoutingEntry *entry);

		/** @brief deletes an entry from the table in destructor**/
		virtual void deleteRoute_destrOnly (SAORSBase_RoutingEntry *entry);

		/** @brief deletes an entry from the delay tolerant routing table **/
		virtual void deleteDTRoute (SAORSBase_RoutingEntry *entry);

		/** @brief removes invalid routes from the network layer routing table **/
		virtual void maintainAssociatedRoutingTable ();

		/** @brief searches an entry (exact match) and gives back a pointer to it, or 0 if none is found **/
		virtual SAORSBase_RoutingEntry* getByAddress(IPv4Address addr);

		/** @brief searches an delay tolerant entry (exact match) and gives back a pointer to it, or 0 if none is found **/
		virtual SAORSBase_RoutingEntry* getDTByAddress(IPv4Address addr);

		/** @brief searches an entry (longest-prefix match) and gives back a pointer to it, or 0 if none is found **/
		virtual SAORSBase_RoutingEntry* getForAddress(IPv4Address addr);

		/** @brief returns the routing table **/
		virtual std::vector<SAORSBase_RoutingEntry *> getRoutingTable();

		/** @brief returns the delay tolerant routing table **/
		virtual std::vector<SAORSBase_RoutingEntry *> getDTRoutingTable();

	protected:
		typedef std::vector<SAORSBase_RoutingEntry *> RouteVector;
		typedef std::vector<SAORSBase_RoutingEntry *>::iterator RouteVectorIter;
		RouteVector routeVector;
		RouteVector dtrouteVector;
		cObject * dymoProcess;

		/**
		 * @brief add or delete network layer routing table entry for given DYMO routing table entry, based on whether it's valid
		 */
		virtual void maintainAssociatedRoutingEntryFor(SAORSBase_RoutingEntry* entry);

	public:
		friend std::ostream& operator<<(std::ostream& os, const SAORSBase_RoutingTable& o);
};

} // namespace inetmanet

} // namespace inet

#endif
