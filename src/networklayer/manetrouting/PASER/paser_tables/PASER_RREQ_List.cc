/*
 *\class       PASER_RREQ_List
 *@brief       Class represents an entry in the RREQ list
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
#include "PASER_RREQ_List.h"

PASER_RREQ_List::~PASER_RREQ_List() {
    for (std::map<ManetAddress, message_rreq_entry *>::iterator it =
            rreq_list.begin(); it != rreq_list.end(); it++) {
        message_rreq_entry *temp = it->second;
        delete temp;
    }
    rreq_list.clear();
}

message_rreq_entry* PASER_RREQ_List::pending_add(struct in_addr dest_addr) {
    message_rreq_entry *entry = pending_find(dest_addr);
    if (entry)
        return entry;
    entry = new message_rreq_entry();

    if (entry == NULL) {
        opp_error("failed malloc()");
        exit(EXIT_FAILURE);
    }

    entry->dest_addr.S_addr = dest_addr.S_addr;
    entry->tries = 0;
    rreq_list.insert(std::make_pair(dest_addr.s_addr, entry));
    return entry;
}

int PASER_RREQ_List::pending_remove(message_rreq_entry *entry) {
//    if (!entry)
//        return 0;
//
//    for (std::list<message_rreq_entry * >::iterator it = rreq_list.begin(); it != rreq_list.end(); it++){
//        message_rreq_entry* temp = (message_rreq_entry*)*it;
//        if(temp->dest_addr.S_addr == entry->dest_addr.S_addr && temp->forw_addr.S_addr == entry->forw_addr.S_addr && temp->src_addr.S_addr == entry->src_addr.S_addr){
//            rreq_list.erase(it);
//            return 1;
//        }
//    }
//    return 0;

    if (!entry)
        return 0;

    std::map<ManetAddress, message_rreq_entry *>::iterator it = rreq_list.find(
            entry->dest_addr.s_addr);
    if (it != rreq_list.end()) {
        if ((*it).second == entry) {
            rreq_list.erase(it);
        } else
            opp_error("Error in PASERPendingRreq table");

    }
    return 1;
}

message_rreq_entry* PASER_RREQ_List::pending_find(struct in_addr dest_addr) {
//    for (std::list<message_rreq_entry * >::iterator it = rreq_list.begin(); it != rreq_list.end(); it++){
//        message_rreq_entry* temp = (message_rreq_entry*)*it;
//        if(temp->dest_addr.S_addr == dest_addr.S_addr && temp->forw_addr.S_addr == forw_addr.S_addr && temp->src_addr.S_addr == src_addr.S_addr){
//            return temp;
//        }
//    }
//    return NULL;
    std::map<ManetAddress, message_rreq_entry *>::iterator it = rreq_list.find(
            dest_addr.s_addr);
    if (it != rreq_list.end()) {
        message_rreq_entry *entry = it->second;
        if (entry->dest_addr.s_addr == dest_addr.s_addr)
            return entry;
        else
            opp_error("Error in rreq_list table");
    }
    return NULL;
}

message_rreq_entry* PASER_RREQ_List::pending_find_addr_with_mask(
        struct in_addr dest_addr, struct in_addr dest_mask) {
    for (std::map<ManetAddress, message_rreq_entry *>::iterator it =
            rreq_list.begin(); it != rreq_list.end(); it++) {
        ManetAddress tempAddr = it->first;
        message_rreq_entry *entry = it->second;
        if (IPv4Address::maskedAddrAreEqual(tempAddr.getIPv4(),dest_addr.S_addr.getIPv4(), dest_mask.S_addr.getIPv4())) {
            return entry;
        }
    }
    return NULL;
//    std::map<ManetAddress, message_rreq_entry * >::iterator it = rreq_list.find(dest_addr.s_addr);
//    if (it != rreq_list.end())
//    {
//        message_rreq_entry *entry = it->second;
//        if (entry->dest_addr.s_addr == dest_addr.s_addr)
//            return entry;
//        else
//            opp_error("Error in rreq_list table");
//    }
//    return NULL;
}
#endif
