/**
 *\class       PASER_Routing_Entry
 *@brief       Class represents an entry in the routing table
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
#ifndef PASER_ROUTING_ENTRY_H_
#define PASER_ROUTING_ENTRY_H_

#include <list>

//OMNET

#include "PASER_Timer_Message.h"
#include "compatibility.h"

class PASER_Routing_Entry {
public:
    struct in_addr dest_addr; ///< IP address of the destination node
    struct in_addr nxthop_addr; ///< IP address of the next hop towards that destination
    PASER_Timer_Message * deleteTimer; ///< Pointer to the deleteTimer (When the route should be deleted)
    PASER_Timer_Message * validTimer; ///< Pointer to the validTimer (when the route is marked as invalid)
    u_int32_t seqnum; ///< Sequence number of the destination node
    u_int32_t hopcnt; ///< Metric of the route
    u_int32_t is_gw; ///< Is the destination node a Gateway
    u_int32_t isValid; ///< Is the route to the destination node fresh/valid
    std::list<address_range> AddL; ///< IP Addresses of all destination node's subnetworks
    u_int8_t *Cert; ///< Certificate of the destination node

public:
    ~PASER_Routing_Entry();
    bool operator==(PASER_Routing_Entry ent);

    /**
     * @brief Set <b>validTimer</b>.
     * Old validTimer will be not freed.
     */
    void setValidTimer(PASER_Timer_Message *_validTimer);
};

#endif /* PASER_ROUTING_ENTRY_H_ */
#endif
