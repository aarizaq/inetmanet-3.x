/**
 *\class       PASER_Routing_Table
 *@brief       Class  is an implementation of the routing table. It provides a map of all existing routes. Each valid route is automatically added to the kernel routing table.
 *@ingroup     Tables
 *\authors     Eugen.Paul | Mohamad.Sbeiti \@paser.info
 *
 *\copyright   (C) 2012 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
 *                  at Technische Universitaet Dortmund, Germany
 *                  http://www.kn.e-technik.tu-dortmund.de/
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ********************************************************************************
 * This work is part of the secure wireless mesh networks framework, which is currently under development by CNI
 ********************************************************************************/
#include "Configuration.h"
#ifdef OPENSSL_IS_LINKED
class PASER_Routing_Table;

#ifndef PASER_ROUTING_TABLE_H_
#define PASER_ROUTING_TABLE_H_
#include "openssl/x509.h"
#include <map>
#include <list>


#include "ManetAddress.h"
#include "PASER_Routing_Entry.h"
#include "PASER_Neighbor_Table.h"
#include "PASER_Neighbor_Entry.h"
#include "PASER_Timer_Message.h"
#include "PASER_Timer_Queue.h"
#include "PASER_Socket.h"
#include "PASER_Global.h"

#include "compatibility.h"

class PASER_Routing_Table {
public:
    /**
     * Map of all node's routes.
     * Key   - IP address of the node.
     * Value - Pointer to the corresponding route entry.
     */
    std::map<ManetAddress, PASER_Routing_Entry*> route_table;

private:
    PASER_Timer_Queue *timer_queue;
    PASER_Neighbor_Table *neighbor_table;
    PASER_Socket *paser_modul;
    PASER_Global *paser_global;

public:
    PASER_Routing_Table(PASER_Timer_Queue *tQueue, PASER_Neighbor_Table *nTable,
            PASER_Socket *pModul, PASER_Global *pGlobal);
    ~PASER_Routing_Table();

    void init();
    void destroy();

    /**
     * @brief Find a routing entry for a given destination address (mesh node, client, subnetwork etc.)
     */
    PASER_Routing_Entry *findAdd(struct in_addr addr);

    /**
     * @brief Find a routing entry for a given mesh node
     */
    PASER_Routing_Entry *findDest(struct in_addr dest_addr);

    /**
     * @brief Get a list of all routes having  a given IP address as next hop
     */
    std::list<PASER_Routing_Entry*> getListWithNextHop(struct in_addr nextHop);

    /**
     * @brief Insert a new entry into the map
     */
    PASER_Routing_Entry *insert(struct in_addr dest_addr,
            struct in_addr nxthop_addr, PASER_Timer_Message * deltimer,
            PASER_Timer_Message * validtimer, u_int32_t seqnum, u_int32_t hopcnt,
            u_int32_t is_gw, std::list<address_range> AddL, u_int8_t *Cert);

    /**
     *@brief Update an entry in the map
     *
     *@param Pointer to the entry which will be updated.
     */
    PASER_Routing_Entry *update(PASER_Routing_Entry *entry,
            struct in_addr dest_addr, struct in_addr nxthop_addr,
            PASER_Timer_Message * deltimer, PASER_Timer_Message * validtimer,
            u_int32_t seqnum, u_int32_t hopcnt, u_int32_t is_gw,
            std::list<address_range> AddL, u_int8_t *Cert);

    /**
     *@brief Delete a entry from the map.
     *
     *@param Pointer to the entry which will be deleted.
     */
    void delete_entry(PASER_Routing_Entry *entry);

    /**
     *@brief Get the shortest route to the gateway.
     *
     *@return Route to the gateway or NULL if no such route exists.
     */
    PASER_Routing_Entry *getRouteToGw();

    /**
     *@brief Get the shortest route to gateway (similar to getRouteToGw()).
     *
     *@return Route to the gateway or NULL if no such route exists.
     */
    PASER_Routing_Entry *findBestGW();

    /**
     * @brief Add a route to the kernel routing table.
     *
     *@param dest_addr IP address of the destination node
     *@param forw_addr IP address of the next hop
     *@param netmask Network mask of the destination node
     *@param metric Metric of the route towards the destination node
     *@param del_entry If true, the route will be deleted from the kernel table.
     * Else a new route will be added to the kernel table.
     *@param ifIndex Interface over which the destination node is reachable.
     */
    void updateKernelRoutingTable(struct in_addr dest_addr,
            struct in_addr forw_addr, struct in_addr netmask, u_int32_t metric,
            bool del_entry, int ifIndex);

    /**
     * @brief Insert or update an entry in the routing table.
     * If the entry does not exist, a new entry will be added to the table.
     * The delete- and validTimer of the entry will be reseted.
     *
     *@param src_addr IP address of the node the entry of which will be updated.
     */
    void updateRoutingTableAndSetTableTimeout(std::list<address_range> addList,
            struct in_addr src_addr, int seq, X509 *cert,
            struct in_addr nextHop, u_int32_t metric, int ifIndex,
            struct timeval now, u_int32_t gFlag, bool trusted);

    /**
     * @brief Reset the delete- and validTimer of an entry.
     * If the entry does not exist, do nothing.
     */
    void updateRoutingTableTimeout(struct in_addr src_addr, struct timeval now,
            int ifIndex);

    /**
     * @brief Reset the sequence number, the delete- and validTimer of an entry.
     * If the entry does not exist, do nothing.
     */
    void updateRoutingTableTimeout(struct in_addr src_addr, u_int32_t seq,
            struct timeval now);

    /**
     * @brief Insert or update the route of a given list of nodes
     *
     *@param addList A list of nodes the route to which will be updated
     */
    void updateRoutingTable(struct timeval now, std::list<address_list> addList,
            struct in_addr nextHop, int ifIndex);

    /**
     * @brief Delete all entries from routing, neighbor and kernel routing tables and theirs timers
     * the next hop of which is the given IP address.
     *
     *@param nextHop IP address
     */
    void deleteFromKernelRoutingTableNodesWithNextHopAddr(
            struct in_addr nextHop);

    /**
     * @brief Update  valid- and deleteTimer of the route entry towards a given destination IP address
     */
    void updateRouteLifetimes(struct in_addr dest_addr);

    /**
     * @brief Update or add a neighbor entry for a list of IP addresses.
     * update or add a routing entry for a list of subnetworks.
     * All entries will be added with a metric equal to 2.
     */
    void updateNeighborFromHELLO(address_list liste, u_int32_t seq,
            int ifIndex);

    /**
     * @brief Update or add a routing entry for a list of IP addresses.
     * All entries will be added with a metric equal to 3.
     */
    void updateRouteFromHELLO(address_list liste, int ifIndex,
            struct in_addr nextHop);

    /**
     * @brief Get all neighbor nodes on a given interface.
     */
    std::list<address_list> getNeighborAddressList(int ifNr);
private:

};

#endif /* PASER_ROUTING_TABLE_H_ */
#endif
