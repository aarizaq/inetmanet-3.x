/**
 *\class       PASER_Neighbor_Table
 *@brief       Class is an implementation of the neighbor table. It provides a map of node's neighbors.
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
class PASER_Neighbor_Table;

#ifndef PASER_NEIGHBOR_TABLE_H_
#define PASER_NEIGHBOR_TABLE_H_

#include "openssl/x509.h"
#include <map>
#include <list>

#include "PASER_Definitions.h"
#include "PASER_Socket.h"
#include "PASER_Neighbor_Entry.h"
#include "PASER_Timer_Queue.h"


#include "compatibility.h"

class PASER_Neighbor_Table {
public:
    /**
     * @brief Map of node's neighbors.
     * Key   - IP address of the node.
     * Value - Pointer to the corresponding neighbor entry.
     */
    std::map<ManetAddress, PASER_Neighbor_Entry *> neighbor_table_map;

public:
    PASER_Neighbor_Table(PASER_Timer_Queue *tQueue, PASER_Socket *pModul);
    ~PASER_Neighbor_Table();
    /**
     * @brief Find a routing entry for a given destination address
     */
    PASER_Neighbor_Entry *findNeigh(struct in_addr neigh_addr);

    /**
     * @brief Insert an new entry into the map
     */
    PASER_Neighbor_Entry *insert(struct in_addr neigh_addr,
            PASER_Timer_Message * deleteTimer, PASER_Timer_Message * validTimer,
            int neighFlag, u_int8_t *root, u_int32_t IV, geo_pos position,
            u_int8_t *Cert, u_int32_t ifIndex);

    /**
     * @brief Update an entry in the map
     *
     *@param Pointer  Pointer to the entry which will be updated.
     */
    PASER_Neighbor_Entry *update(PASER_Neighbor_Entry *entry,
            struct in_addr neigh_addr, PASER_Timer_Message * deleteTimer,
            PASER_Timer_Message * validTimer, int neighFlag, u_int8_t *root,
            u_int32_t IV, geo_pos position, u_int8_t *Cert, u_int32_t ifIndex);

    /**
     *@brief Delete a entry from the map
     *
     *@param Pointer  Pointer to the entry which will be deleted
     */
    void delete_entry(PASER_Neighbor_Entry *entry);

    /**
     * @brief Insert or update an entry in the neighbor table.
     * If the entry does not exist then a new entry will be added to the table.
     * Hereby, Delete- and ValidTimer of the entry will be reseted.
     *
     *@param IP Address of the neighbor node the entry of which will be updated.
     */
    void updateNeighborTableAndSetTableTimeout(struct in_addr neigh, int nFlag,
            u_int8_t *root, int iv, geo_pos position, X509 *cert,
            struct timeval now, u_int32_t ifIndex);

    /**
     *@brief Reset the Delete- and ValidTimer of an entry.
     * If the entry does not exist, do nothing.
     */
    void updateNeighborTableTimeout(struct in_addr neigh, struct timeval now);

    /**
     * @brief Update IV of an entry and mark the entry as valid
     * If the entry does not exist, do nothing.
     */
    void updateNeighborTableIVandSetValid(struct in_addr neigh, u_int32_t IV);

    /**
     * @brief Update IV of an entry.
     * If the entry does not exist, do nothing.
     */
    void updateNeighborTableIV(struct in_addr neigh, u_int32_t IV);

    //    int checkAllCert();!!!

private:
    PASER_Timer_Queue *timer_queue;
    PASER_Socket *paser_modul;
};

#endif /* PASER_NEIGHBOR_TABLE_H_ */
#endif
