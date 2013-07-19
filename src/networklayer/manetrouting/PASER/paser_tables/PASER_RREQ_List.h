/**
 *\class       PASER_RREQ_List
 *@brief       Class represents an entry in the RREQ list
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
#ifndef PASER_RREQ_LIST_H_
#define PASER_RREQ_LIST_H_

#include "PASER_Definitions.h"

#include <map>
#include "compatibility.h"
#include "PASER_Timer_Message.h"

class message_rreq_entry {
public:
    int tries;
    struct in_addr dest_addr;
    PASER_Timer_Message *tPack;

    ~message_rreq_entry() {

    }
};

/**
 * This class is an implementation of the RREQ list.
 * Here, we maintain a map of the RREQs the RREPs of which
 * have not been received yet. Besides, a map of the RREPs the RREP-ACKs
 * of which have not been received yet is also maintained in this class.
 */
class PASER_RREQ_List {
public:
    /**
     * Map of RREQ/RREP
     * Key   - Destination IP Address
     * Value - Pointer to entry
     */
    std::map<ManetAddress, message_rreq_entry *> rreq_list;

public:
    ~PASER_RREQ_List();

    /**
     *@brief Add a new entry to the list
     */
    message_rreq_entry *pending_add(struct in_addr dest_addr);

    /**
     *@brief Remove an entry from the list
     */
    int pending_remove(message_rreq_entry *entry);

    /**
     *@brief Find an entry in the list for a given destination address
     */
    message_rreq_entry *pending_find(struct in_addr dest_addr);

    /**
     *@brief Find an entry in the list for a  given destination address and a given network mask
     */
    message_rreq_entry* pending_find_addr_with_mask(struct in_addr dest_addr,
            struct in_addr dest_mask);
};

#endif /* PASER_RREQ_LIST_H_ */
#endif
