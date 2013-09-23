/*
 *\class       PASER_Neighbor_Table
 *@brief       Class is an implementation of the neighbor table. It provides a map of node's neighbors.
 *
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
#include "PASER_Neighbor_Table.h"

PASER_Neighbor_Table::PASER_Neighbor_Table(PASER_Timer_Queue *tQueue,
        PASER_Socket *pModul) {
    timer_queue = tQueue;
    paser_modul = pModul;
}

PASER_Neighbor_Table::~PASER_Neighbor_Table() {
    for (std::map<ManetAddress, PASER_Neighbor_Entry*>::iterator it =
            neighbor_table_map.begin(); it != neighbor_table_map.end(); it++) {
        PASER_Neighbor_Entry *temp = it->second;
        delete temp;
    }
    neighbor_table_map.clear();
}

/* Find an routing entry given the destination address */
PASER_Neighbor_Entry *PASER_Neighbor_Table::findNeigh(
        struct in_addr neigh_addr) {
    std::map<ManetAddress, PASER_Neighbor_Entry*>::iterator it =
            neighbor_table_map.find(neigh_addr.s_addr);
    if (it != neighbor_table_map.end()) {
        if (it->second)
            return it->second;
    }
    return NULL;
}

PASER_Neighbor_Entry *PASER_Neighbor_Table::insert(struct in_addr neigh_addr,
        PASER_Timer_Message * deleteTimer, PASER_Timer_Message * validTimer,
        int neighFlag, u_int8_t *root, u_int32_t IV, geo_pos position,
        u_int8_t *Cert, u_int32_t ifIndex) {
    PASER_Neighbor_Entry *entry = new PASER_Neighbor_Entry();
    entry->IV = IV;
    entry->neighFlag = neighFlag;
    entry->neighbor_addr.S_addr = neigh_addr.S_addr;
    entry->deleteTimer = deleteTimer;
    entry->validTimer = validTimer;
    entry->position.lat = position.lat;
    entry->position.lon = position.lon;
    entry->Cert = Cert;
    entry->root = root;
    entry->isValid = 1;
    entry->ifIndex = ifIndex;

    neighbor_table_map.insert(std::make_pair(neigh_addr.s_addr, entry));
    return entry;
}

PASER_Neighbor_Entry *PASER_Neighbor_Table::update(PASER_Neighbor_Entry *entry,
        struct in_addr neigh_addr, PASER_Timer_Message * deleteTimer,
        PASER_Timer_Message * validTimer, int neighFlag, u_int8_t *root,
        u_int32_t IV, geo_pos position, u_int8_t *Cert, u_int32_t ifIndex) {
    if (entry) {
        std::map<ManetAddress, PASER_Neighbor_Entry*>::iterator it =
                neighbor_table_map.find(entry->neighbor_addr.s_addr);
        if (it != neighbor_table_map.end()) {
            if ((*it).second == entry) {
                neighbor_table_map.erase(it);
            } else
                opp_error("Error in PASER routing table");
        }
        delete entry;
    }

    entry = new PASER_Neighbor_Entry();
    entry->IV = IV;
    entry->neighFlag = neighFlag;
    entry->neighbor_addr.S_addr = neigh_addr.S_addr;
    entry->deleteTimer = deleteTimer;
    entry->validTimer = validTimer;
    entry->position.lat = position.lat;
    entry->position.lon = position.lon;
    entry->Cert = Cert;
    entry->root = root;
    entry->isValid = 1;
    entry->ifIndex = ifIndex;

    neighbor_table_map.insert(std::make_pair(neigh_addr.s_addr, entry));
    return entry;
}

void PASER_Neighbor_Table::delete_entry(PASER_Neighbor_Entry *entry) {
    if (!entry)
        return;

    if (entry) {
        std::map<ManetAddress, PASER_Neighbor_Entry*>::iterator it =
                neighbor_table_map.find(entry->neighbor_addr.s_addr);
        if (it != neighbor_table_map.end()) {
            if ((*it).second == entry) {
                neighbor_table_map.erase(it);
            } else
                opp_error("Error in PASER routing table");
        }
    }
}

void PASER_Neighbor_Table::updateNeighborTableAndSetTableTimeout(
        struct in_addr neigh, int nFlag, u_int8_t *root, int iv,
        geo_pos position, X509 *cert, struct timeval now, u_int32_t ifIndex) {
    PASER_Timer_Message *deletePack = NULL;
    PASER_Timer_Message *validPack = NULL;
    PASER_Neighbor_Entry *nEntry = findNeigh(neigh);
    int32_t setTrusted = 0;
    if (nEntry) {
        setTrusted = nEntry->neighFlag;
        deletePack = nEntry->deleteTimer;
        validPack = nEntry->validTimer;
        if (validPack == NULL) {
            validPack = new PASER_Timer_Message();
            validPack->data = NULL;
            validPack->destAddr.S_addr = neigh.S_addr;
            validPack->handler = NEIGHBORTABLE_VALID_ENTRY;
            nEntry->validTimer = validPack;
            validPack->timeout = timeval_add(now, PASER_NEIGHBOR_VALID_TIME);
            setTrusted = 0;
        } else if (nFlag) {
            validPack->timeout = timeval_add(now, PASER_NEIGHBOR_VALID_TIME);
        }
        deletePack->timeout = timeval_add(now, PASER_NEIGHBOR_DELETE_TIME);
    } else {
        setTrusted = nFlag;
        deletePack = new PASER_Timer_Message();
        deletePack->data = NULL;
        deletePack->destAddr.S_addr = neigh.S_addr;
        deletePack->handler = NEIGHBORTABLE_DELETE_ENTRY;
        validPack = new PASER_Timer_Message();
        validPack->data = NULL;
        validPack->destAddr.S_addr = neigh.S_addr;
        validPack->handler = NEIGHBORTABLE_VALID_ENTRY;
        deletePack->timeout = timeval_add(now, PASER_NEIGHBOR_DELETE_TIME);
        validPack->timeout = timeval_add(now, PASER_NEIGHBOR_VALID_TIME);
    }

    if (validPack != 0) {
        EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
        timer_queue->timer_add(validPack);
    }
    EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
            << deletePack->timeout.tv_sec << "\n";
    timer_queue->timer_add(deletePack);

    u_int8_t *rootN = (u_int8_t *) malloc(
            (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    memcpy(rootN, root, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    if (nEntry == NULL) {
        nEntry = insert(neigh, deletePack, validPack, nFlag, rootN, iv,
                position, (u_int8_t*) cert, ifIndex);
    } else {
        int tempFlag = 0;
        if (nEntry->neighFlag || nFlag) {
            tempFlag = 1;
        }
        nEntry = update(nEntry, neigh, deletePack, validPack, tempFlag, rootN,
                iv, position, (u_int8_t*) cert, ifIndex);
    }
    nEntry->neighFlag = setTrusted || nFlag;
}

void PASER_Neighbor_Table::updateNeighborTableTimeout(struct in_addr neigh,
        struct timeval now) {
    PASER_Timer_Message *deletePack = NULL;
    PASER_Timer_Message *validPack = NULL;
    PASER_Neighbor_Entry *nEntry = findNeigh(neigh);
    if (nEntry) {
        deletePack = nEntry->deleteTimer;
        validPack = nEntry->validTimer;
        if (validPack == NULL) {
            validPack = new PASER_Timer_Message();
            validPack->data = NULL;
            validPack->destAddr.S_addr = neigh.S_addr;
            validPack->handler = NEIGHBORTABLE_VALID_ENTRY;
            nEntry->validTimer = validPack;
        }
    } else {
        return;
    }
    nEntry->isValid = 1;
    deletePack->timeout = timeval_add(now, PASER_NEIGHBOR_DELETE_TIME);
    validPack->timeout = timeval_add(now, PASER_NEIGHBOR_VALID_TIME);

    EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
            << deletePack->timeout.tv_sec << "\n";
    EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
    timer_queue->timer_add(deletePack);
    timer_queue->timer_add(validPack);
}

void PASER_Neighbor_Table::updateNeighborTableIVandSetValid(
        struct in_addr neigh, u_int32_t IV) {
    PASER_Neighbor_Entry *nEntry = findNeigh(neigh);
    if (nEntry) {
        nEntry->IV = IV;
        nEntry->isValid = 1;
    } else {
        return;
    }
}

void PASER_Neighbor_Table::updateNeighborTableIV(struct in_addr neigh,
        u_int32_t IV) {
    PASER_Neighbor_Entry *nEntry = findNeigh(neigh);
    if (nEntry) {
        nEntry->IV = IV;
    } else {
        return;
    }
}

#endif
