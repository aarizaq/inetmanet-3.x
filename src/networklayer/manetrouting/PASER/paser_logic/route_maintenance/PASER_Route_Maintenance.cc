/*
 *\class       PASER_Route_Maintenance
 *@brief       Class provides functions to manage PASER timers and link layer feedback
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
#include "PASER_Route_Maintenance.h"
#include "PASER_TB_RERR.h"

PASER_Route_Maintenance::PASER_Route_Maintenance(PASER_Global *pGlobal,
        PASER_Configurations *pConfig, PASER_Socket *pModul) {
    paser_modul = pModul;
    paser_global = pGlobal;
    paser_configuration = pConfig;
}

void PASER_Route_Maintenance::handleSelfMsg(cMessage *msg) {
    EV << "handleSelfMsg\n";
    PASER_Timer_Message *nextTimout =
            paser_global->getTimer_queue()->timer_get_next_timer();
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    EV << "now: " << now.tv_sec << "." << now.tv_usec << "\n";
    if (now.tv_sec < nextTimout->timeout.tv_sec) {
        EV << "1\n";
        return;
    }
    if (now.tv_sec == nextTimout->timeout.tv_sec
            && now.tv_usec < nextTimout->timeout.tv_usec) {
        EV << "2\n";
        return;
    }
    EV << "timeout: " << nextTimout->timeout.tv_sec << "\n";

    if (nextTimout != NULL) {
        switch (nextTimout->handler) {
        case KDC_REQUEST:
            EV << "KDC_REQUEST\n";
            timeout_KDC_request(nextTimout);
            break;
        case ROUTE_DISCOVERY_UB:
            EV << "ROUTE_DISCOVERY_UB\n";
            timeout_ROUTE_DISCOVERY_UB(nextTimout);
            break;
        case ROUTINGTABLE_DELETE_ENTRY:
            EV << "ROUTINGTABLE_DELETE_ENTRY\n";
            timeout_ROUTINGTABLE_DELETE_ENTRY(nextTimout);
            break;
        case ROUTINGTABLE_VALID_ENTRY:
            EV << "ROUTINGTABLE_VALID_ENTRY\n";
            timeout_ROUTINGTABLE_NO_VALID_ENTRY(nextTimout);
            break;
        case NEIGHBORTABLE_DELETE_ENTRY:
            EV << "NEIGHBORTABLE_DELETE_ENTRY\n";
            timeout_NEIGHBORTABLE_DELETE_ENTRY(nextTimout);
            break;
        case NEIGHBORTABLE_VALID_ENTRY:
            EV << "NEIGHBORTABLE_VALID_ENTRY\n";
            timeout_NEIGHBORTABLE_NO_VALID_ENTRY(nextTimout);
            break;
        case TU_RREP_ACK_TIMEOUT:
            EV << "TU_RREP_ACK_TIMEOUT\n";
            timeout_TU_RREP_ACK_TIMEOUT(nextTimout);
            break;
        case HELLO_SEND_TIMEOUT:
            EV << "HELLO_SEND_TIMEOUT\n";
            timeout_HELLO_SEND_TIMEOUT(nextTimout);
            break;
        case PASER_ROOT:
            EV << "PASER_ROOT_TIMEOUT\n";
            timeout_ROOT_TIMEOUT(nextTimout);
            break;
        }
    }
}

void PASER_Route_Maintenance::timeout_KDC_request(PASER_Timer_Message *t) {
    EV << "timeout_KDC_request\n";
    if (paser_global->getIsRegistered() == false) {
        lv_block cert;
        if (!paser_global->getCrypto_sign()->getCert(&cert)) {
            EV << "cert ERROR\n";
            return;
        }
        paser_global->getPaket_processing()->sendKDCRequest(
                paser_configuration->getNetDevice()[0].ipaddr,
                paser_configuration->getNetDevice()[0].ipaddr, cert,
                paser_global->getLastGwSearchNonce());
        free(cert.buf);
        t->timeout = timeval_add(t->timeout, PASER_KDC_REQUEST_TIME);
        paser_global->getTimer_queue()->timer_sort();
    }
    return;
}

void PASER_Route_Maintenance::timeout_ROUTE_DISCOVERY_UB(
        PASER_Timer_Message *t) {
    EV << "timeout_ROUTE_DISCOVERY_UB\n";
    PASER_UB_RREQ * message = (PASER_UB_RREQ *) t->data;
    if (!message) {
        EV << "ERROR!\n";
        return;
    }

    message_rreq_entry *pend_rreq = paser_global->getRreq_list()->pending_add(
            t->destAddr);
    if (pend_rreq->tries < PASER_UB_RREQ_TRIES) {
        EV << "pend_rreq->tries = " << pend_rreq->tries << "\n";
        pend_rreq->tries++;
        if (paser_modul->isMyLocalAddress(message->srcAddress_var))
            message->seq = paser_global->getSeqNr();
        message->seqForw = paser_global->getSeqNr();
        //Send broadcast on all interfaces
        for (u_int32_t i = 0; i < paser_configuration->getNetDeviceNumber();
                i++) {
            PASER_UB_RREQ *messageToSend = new PASER_UB_RREQ(*message);
            struct timeval now;
            paser_modul->MYgettimeofday(&now, NULL);
            messageToSend->timestamp = now.tv_sec;
            messageToSend->srcAddress_var.S_addr =
                    paser_global->getNetDevice()[i].ipaddr.S_addr;
            messageToSend->AddressRangeList.clear();
            address_list tempAddList;
            tempAddList.ipaddr = paser_global->getNetDevice()[i].ipaddr;
            tempAddList.range = paser_global->getAddL();
            messageToSend->AddressRangeList.push_back(tempAddList);
            geo_pos myGeo = paser_global->getGeoPosition();
            messageToSend->geoForwarding.lat = myGeo.lat;
            messageToSend->geoForwarding.lon = myGeo.lon;
//            messageToSend->routeFromQueryingToForwarding.clear();
//            messageToSend->routeFromQueryingToForwarding.push_back( paser_global->getNetDevice()[i].ipaddr );
            paser_global->getCrypto_sign()->signUBRREQ(messageToSend);
            // Update Timer
            struct in_addr bcast_addr;
            bcast_addr.S_addr.set(IPv4Address::ALLONES_ADDRESS);
            paser_modul->send_message(
                    messageToSend,
                    bcast_addr,
                    paser_modul->MYgetWlanInterfaceIndexByAddress(
                            messageToSend->srcAddress_var.S_addr));
        }
        // SegNr++
        paser_global->setSeqNr(paser_global->getSeqNr() + 1);
        paser_modul->MYgettimeofday(&(t->timeout), NULL);
        t->timeout = timeval_add(t->timeout, PASER_UB_RREQ_WAIT_TIME);
        paser_global->getTimer_queue()->timer_sort();
        paser_global->resetHelloTimer();
        return;
    }
    //Remove timer
    EV << "pend_rreq->tries > PASER_UB_RREQ_TRIES\n";
    paser_global->getTimer_queue()->timer_remove(t);
    paser_global->getRreq_list()->pending_remove(pend_rreq);
    paser_global->getMessage_queue()->deleteMessages(t->destAddr);
    PASER_Routing_Entry *routeToGW =
            paser_global->getRouting_table()->findBestGW();
    if (routeToGW == NULL) {
        paser_global->getRoute_findung()->tryToRegister();
    }
    delete pend_rreq;
    delete t;
}

void PASER_Route_Maintenance::timeout_ROUTINGTABLE_DELETE_ENTRY(
        PASER_Timer_Message *t) {
    EV << "timeout_ROUTINGTABLE_DELETE_ENTRY\n";
    PASER_Routing_Entry *rEntry = paser_global->getRouting_table()->findDest(
            t->destAddr);
    if (rEntry != NULL) {
        paser_global->getRouting_table()->delete_entry(rEntry);
        delete rEntry;
    }
    paser_global->getTimer_queue()->timer_remove(t);
    delete t;
}

void PASER_Route_Maintenance::timeout_ROUTINGTABLE_NO_VALID_ENTRY(
        PASER_Timer_Message *t) {
    EV << "timeout_ROUTINGTABLE_NO_VALID_ENTRY\n";
    struct in_addr destAddr = t->destAddr;
    PASER_Routing_Entry *rEntry = paser_global->getRouting_table()->findDest(
            t->destAddr);
    if (rEntry != NULL) {
        rEntry->isValid = 0;
        rEntry->validTimer = NULL;
    }
    paser_global->getTimer_queue()->timer_remove(t);
    delete t;
    paser_global->getRouting_table()->deleteFromKernelRoutingTableNodesWithNextHopAddr(
            destAddr);
    PASER_Routing_Entry *routeToGW =
            paser_global->getRouting_table()->findBestGW();
    if (routeToGW == NULL && !paser_configuration->getIsGW()) {
        EV << "tryTORegister\n";
        paser_global->setIsRegistered(false);
        paser_global->getRoute_findung()->tryToRegister();
    }
}

void PASER_Route_Maintenance::timeout_NEIGHBORTABLE_DELETE_ENTRY(
        PASER_Timer_Message *t) {
    EV << "timeout_NEIGHBORTABLE_DELETE_ENTRY\n";
    PASER_Neighbor_Entry *nEntry = paser_global->getNeighbor_table()->findNeigh(
            t->destAddr);
    if (nEntry != NULL) {
        paser_global->getNeighbor_table()->delete_entry(nEntry);
        delete nEntry;
    }
    paser_global->getTimer_queue()->timer_remove(t);
    delete t;
}

void PASER_Route_Maintenance::timeout_NEIGHBORTABLE_NO_VALID_ENTRY(
        PASER_Timer_Message *t) {
    EV << "timeout_NEIGHBORTABLE_NO_VALID_ENTRY\n";
    struct in_addr destAddr = t->destAddr;
    PASER_Neighbor_Entry *nEntry = paser_global->getNeighbor_table()->findNeigh(
            t->destAddr);
    if (nEntry != NULL) {
        nEntry->isValid = 0;
//        nEntry->setValidTimer(NULL);
        nEntry->validTimer = NULL;
    }
    paser_global->getTimer_queue()->timer_remove(t);
    delete t;
    paser_global->getRouting_table()->deleteFromKernelRoutingTableNodesWithNextHopAddr(
            destAddr);
    PASER_Routing_Entry *routeToGW =
            paser_global->getRouting_table()->findBestGW();
    if (routeToGW == NULL && !paser_configuration->getIsGW()) {
        EV << "tryTORegister\n";
        paser_global->setIsRegistered(false);
        paser_global->getRoute_findung()->tryToRegister();
    }
}

void PASER_Route_Maintenance::timeout_TU_RREP_ACK_TIMEOUT(
        PASER_Timer_Message *t) {
    EV << "timeout_TU_RREP_ACK_TIMEOUT\n";
    PASER_UU_RREP * message = (PASER_UU_RREP *) t->data;
    if (!message) {
        message_rreq_entry *pend_rrep =
                paser_global->getRrep_list()->pending_find(t->destAddr);
        if (pend_rrep) {
            paser_global->getRrep_list()->pending_remove(pend_rrep);
            delete pend_rrep;
        }
        paser_global->getTimer_queue()->timer_remove(t);
        delete t;
        return;
    }
//    if (!message)
//    {
//        paser_global->getTimer_queue()->timer_remove(t);
//        delete t;
//        return;
//    }
    message_rreq_entry *pend_rrep = paser_global->getRrep_list()->pending_find(
            t->destAddr);
    if (pend_rrep == NULL) {
        EV << "ERROR!\n";
        paser_global->getTimer_queue()->timer_remove(t);
        delete t;
        return;
    }
    if (pend_rrep->tries < PASER_UU_RREP_TRIES) {
        EV << "pend_rrep->tries = " << pend_rrep->tries << "\n";
        pend_rrep->tries++;
        if (paser_modul->isMyLocalAddress(message->destAddress_var)) {
            message->seq = paser_global->getSeqNr();
            geo_pos myGeo = paser_global->getGeoPosition();
            message->geoForwarding.lat = myGeo.lat;
            message->geoForwarding.lon = myGeo.lon;
            paser_global->getCrypto_sign()->signUURREP(message);
        }
        PASER_UU_RREP *messageToSend = new PASER_UU_RREP(*message);
        //Update Timer
        PASER_Routing_Entry *rEntry =
                paser_global->getRouting_table()->findDest(
                        messageToSend->srcAddress_var);
        if (!rEntry) {
            delete messageToSend;
            paser_modul->MYgettimeofday(&(t->timeout), NULL);
            t->timeout = timeval_add(t->timeout, PASER_UU_RREP_WAIT_TIME);
            paser_global->getTimer_queue()->timer_sort();
            return;
        }
        PASER_Neighbor_Entry *nEntry =
                paser_global->getNeighbor_table()->findNeigh(
                        rEntry->nxthop_addr);
        if (!nEntry) {
            delete messageToSend;
            paser_modul->MYgettimeofday(&(t->timeout), NULL);
            t->timeout = timeval_add(t->timeout, PASER_UU_RREP_WAIT_TIME);
            paser_global->getTimer_queue()->timer_sort();
            return;
        }
        paser_modul->send_message(messageToSend, messageToSend->srcAddress_var,
                nEntry->ifIndex);
        //SeqNr++
        paser_global->setSeqNr(paser_global->getSeqNr() + 1);
        paser_modul->MYgettimeofday(&(t->timeout),
                NULL)/*MYgettimeofday(&(t->timeout), NULL)*/;
        t->timeout = timeval_add(t->timeout, PASER_UU_RREP_WAIT_TIME);
        paser_global->getTimer_queue()->timer_sort();
        return;
    }
    //remove timer
    EV << "pend_rrep->tries > PASER_UU_RREP_TRIES\n";
    paser_global->getTimer_queue()->timer_remove(t);
    paser_global->getRrep_list()->pending_remove(pend_rrep);
    delete pend_rrep;
    delete t;
}

void PASER_Route_Maintenance::timeout_HELLO_SEND_TIMEOUT(
        PASER_Timer_Message *t) {
    EV << "timeout_HELLO_SEND_TIMEOUT\n";
    if (!paser_configuration->isRootReady) {
        paser_modul->MYgettimeofday(&(t->timeout),
                NULL)/*MYgettimeofday(&(t->timeout), NULL)*/;
        t->timeout = timeval_add(t->timeout, PASER_TB_Hello_Interval);
        paser_global->getTimer_queue()->timer_sort();
        return;
    }

    for (u_int32_t i = 0; i < paser_configuration->getNetDeviceNumber(); i++) {
        EV << "i = " << i << "\n";
        network_device *tempDevice = paser_global->getNetDevice();
        EV << "tempDevice[i].ipaddr = "
                << tempDevice[i].ipaddr.S_addr.getIPv4().str() << "\n";
        PASER_TB_Hello *messageToSend = new PASER_TB_Hello(tempDevice[i].ipaddr,
                paser_global->getSeqNr());
        messageToSend->seq = paser_global->getSeqNr();
        std::list<address_list> tempList =
                paser_global->getRouting_table()->getNeighborAddressList(i);
        for (std::list<address_list>::iterator it = tempList.begin();
                it != tempList.end(); it++) {
            address_list tempEntry;
            tempEntry.ipaddr = ((address_list) *it).ipaddr;
            std::list<address_range> tempRange;
            for (std::list<address_range>::iterator it2 = tempRange.begin();
                    it2 != tempRange.end(); it2++) {
                address_range tempR;
                tempR.ipaddr = ((address_range) *it2).ipaddr;
                tempR.mask = ((address_range) *it2).mask;
                tempRange.push_back(tempR);
            }
            EV << "add ip to AddressRangeList: "
                    << tempEntry.ipaddr.S_addr.getIPv4().str() << "\n";
            messageToSend->AddressRangeList.push_back(tempEntry);
        }

        if (messageToSend->AddressRangeList.size() <= 1) {
            ev
                    << "es gibt keine Nachbarn. HELLO Nachricht wird nicht gesendet.\n";
            delete messageToSend;
            continue;
        }

        geo_pos myGeo = paser_global->getGeoPosition();
        messageToSend->geoQuerying.lat = myGeo.lat;
        messageToSend->geoQuerying.lon = myGeo.lon;

        int next_iv = 0;
        u_int8_t *secret = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * PASER_SECRET_LEN));
        messageToSend->auth = paser_global->getRoot()->root_get_next_secret(
                &next_iv, secret);
        messageToSend->secret = secret;
        EV << "next iv: " << next_iv << "\n";

        paser_global->getCrypto_hash()->computeHmacHELLO(messageToSend,
                paser_configuration->getGTK());

        struct in_addr bcast_addr;
        bcast_addr.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        paser_modul->send_message(
                messageToSend,
                bcast_addr,
                paser_modul->MYgetWlanInterfaceIndexByAddress(
                        messageToSend->srcAddress_var.S_addr));
        //SeqNr++!!!
        paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    }
    paser_modul->MYgettimeofday(&(t->timeout),
            NULL)/*MYgettimeofday(&(t->timeout), NULL)*/;
    t->timeout = timeval_add(t->timeout, PASER_TB_Hello_Interval);
    paser_global->getTimer_queue()->timer_sort();
}

void PASER_Route_Maintenance::timeout_ROOT_TIMEOUT(PASER_Timer_Message *t) {
    //send ROOT
    paser_global->getPaket_processing()->send_root();
    //remove timer
    paser_global->getTimer_queue()->timer_remove(t);
    delete t;
}

void PASER_Route_Maintenance::messageFailed(struct in_addr src,
        struct in_addr dest, bool sendRERR) {
    EV << "Link break for src: " << src.S_addr.getIPv4().str()
            << ", dest: " << dest.S_addr.getIPv4().str() << "\n";
    PASER_Routing_Entry *rEntry = paser_global->getRouting_table()->findDest(
            dest);
    if (rEntry == NULL) {
        return;
    }
    PASER_Neighbor_Entry *nEntry = paser_global->getNeighbor_table()->findNeigh(
            rEntry->nxthop_addr);
    if (nEntry == NULL) {
        return;
    }
    struct in_addr nextHop = nEntry->neighbor_addr;
    if (paser_global->getRouting_table()->findDest(nextHop) == NULL) {
        return;
    }
    EV << "Link broken to neighbor " << nextHop.S_addr.getIPv4().str()
            << "\n";

    std::list<PASER_Routing_Entry*> EntryList =
            paser_global->getRouting_table()->getListWithNextHop(nextHop);
    paser_global->getRouting_table()->deleteFromKernelRoutingTableNodesWithNextHopAddr(
            nextHop);

    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);

    //if(sendRERR && paser_global->getBlacklist()->setRerrTime(dest, now)){
    if (sendRERR) {
        std::list<unreachableBlock> allAddrList;
//        allAddrList.push_front(nextHop);
        for (std::list<PASER_Routing_Entry*>::iterator it = EntryList.begin();
                it != EntryList.end(); it++) {
            PASER_Routing_Entry *tempEntry = (PASER_Routing_Entry *) *it;
            unreachableBlock temp;
            temp.addr.S_addr = tempEntry->dest_addr.S_addr;
            temp.seq = tempEntry->seqnum;
            allAddrList.push_back(temp);
        }
        paser_global->getPaket_processing()->send_rerr(allAddrList);
    }
}
#endif
