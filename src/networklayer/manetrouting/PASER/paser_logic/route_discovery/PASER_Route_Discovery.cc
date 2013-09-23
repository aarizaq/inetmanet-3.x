/*
 *\class       PASER_Route_Discovery
 *@brief       Class provides functions to manage a node registration at a gateway or to handle a route discovery
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
#include "PASER_Route_Discovery.h"
#include "PASER_Definitions.h"

PASER_Route_Discovery::PASER_Route_Discovery(PASER_Global *pGlobal,
        PASER_Configurations *pConfig, PASER_Socket *pModul, bool setGWsearch) {
    paser_global = pGlobal;
    paser_configuration = pConfig;
    paser_modul = pModul;

    isGWsearch = setGWsearch;
}

void PASER_Route_Discovery::tryToRegister() {
    if (paser_global->getIsRegistered()) {
        return;
    }
    if (paser_global->getWasRegistered() && !isGWsearch) {
        return;
    }
    struct in_addr bcast_addr;
    bcast_addr.s_addr.set(IPv4Address::ALLONES_ADDRESS);
    route_discovery(bcast_addr, 1);
}

void PASER_Route_Discovery::route_discovery(struct in_addr dest_addr,
        int isDestGW) {
    // If a route discovery has been already started for dest_addr,
    // then simply return
    struct in_addr bcast_addr;
    bcast_addr.S_addr.set(IPv4Address::ALLONES_ADDRESS);
    if (paser_global->getRreq_list()->pending_find(dest_addr)) {
        EV << "a route discovery are already started\n";
        return;
    }

    PASER_UB_RREQ *message = NULL;
    if (isDestGW) {
        paser_global->generateGwSearchNonce();
    }
    for (u_int32_t i = 0; i < paser_configuration->getNetDeviceNumber(); i++) {
        in_addr WlanAddrStruct;
        WlanAddrStruct.S_addr = paser_global->getNetDevice()[i].ipaddr.S_addr;
        if (message != NULL) {
            delete message;
        }
        message = paser_global->getPaket_processing()->send_ub_rreq(
                WlanAddrStruct, dest_addr, isDestGW);
    }
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);

    if (message == NULL)
        return;
    // Record information for destination
    message_rreq_entry *pend_rreq = paser_global->getRreq_list()->pending_add(
            dest_addr);

    PASER_Timer_Message *tPack = new PASER_Timer_Message();
    tPack->data = (void *) message;
    tPack->destAddr.S_addr = dest_addr.S_addr;
    tPack->handler = ROUTE_DISCOVERY_UB;
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    tPack->timeout = timeval_add(now, PASER_UB_RREQ_WAIT_TIME);
    pend_rreq->tries = 0;

    EV << "now: " << now.tv_sec << "\ntimeout: " << tPack->timeout.tv_sec
            << "\n";
    paser_global->getTimer_queue()->timer_add(tPack);
    pend_rreq->tPack = tPack;
}

void PASER_Route_Discovery::processMessage(IPv4Datagram* datagram) {
    if (!paser_global->getWasRegistered()) {
        delete datagram;
        return;
    }

    struct in_addr dest_addr, src_addr;
    bool isLocal = false;

    src_addr.s_addr.set(datagram->getSrcAddress());
    dest_addr.s_addr.set(datagram->getDestAddress());

    isLocal = true;
    if (!datagram->getSrcAddress().isUnspecified()) {
        isLocal = paser_modul->isMyLocalAddress(src_addr);
    }
    if (!isLocal && paser_configuration->isAddInMySubnetz(src_addr)) {
        isLocal = true;
    }

    // Look up destination in the routing table
    PASER_Routing_Entry *rEntry = paser_global->getRouting_table()->findDest(
            dest_addr);
    bool isRoute = false;
    // A valid route exists
    if (rEntry == NULL) {
    } else if (rEntry->isValid) {
        PASER_Neighbor_Entry *nEntry =
                paser_global->getNeighbor_table()->findNeigh(
                        rEntry->nxthop_addr);
        if (nEntry == NULL || !nEntry->neighFlag || !nEntry->isValid) {
            EV << "no Route to Dest.\n";
            isRoute = false;
        } else {
            EV << "found Route to Dest.\n";
            isRoute = true;
        }
    }
    // No route is found in the table found -> route discovery (only if there is no one already underway)
    if (!isRoute) {
        EV << "start Route discovery for "
                << dest_addr.S_addr.getIPv4().str() << "\n";
        if (isLocal
                || (rEntry && paser_configuration->isSetLocalRepair()
                        && paser_configuration->getMaxLocalRepairHopCount()
                                >= rEntry->hopcnt)) {
            if (datagram->getControlInfo())
                delete datagram->removeControlInfo();
            paser_global->getMessage_queue()->message_add(datagram, dest_addr);
            //TODO: edit isGWflag!!!
            route_discovery(dest_addr, 0);
        } else {
            EV << "Route discovery ERROR!\n";
            std::list<unreachableBlock> allAddrList;
            unreachableBlock temp;
            temp.addr.S_addr = dest_addr.S_addr;
            temp.seq = 0;
            allAddrList.push_back(temp);
            paser_global->getPaket_processing()->send_rerr(allAddrList);

            struct timeval now;
            paser_modul->MYgettimeofday(&now, NULL);
            paser_global->getBlacklist()->setRerrTime(dest_addr, now);

            delete datagram;
        }
    }
}
#endif
