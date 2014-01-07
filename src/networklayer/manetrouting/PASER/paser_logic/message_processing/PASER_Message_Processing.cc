/*
 *\class       PASER_Message_Processing
 *@brief       Class provides functions to manage all PASER messages
 *
 *\authors     Eugen.Paul | Mohamad.Sbeiti \@paser.info
 *
 *\copyright   (C) 2012 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
 *                  at Technische Universitaet Dortmund, Germany
 *                  http://www.kn.e-technik.tu-dortmund.de/
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
#include "PASER_Message_Processing.h"

#include "compatibility.h"

PASER_Message_Processing::PASER_Message_Processing(PASER_Global *pGlobal,
        PASER_Configurations *pConfig, PASER_Socket *pModul) {
    paser_global = pGlobal;
    paser_configuration = pConfig;
    paser_modul = pModul;

    timer_queue = paser_global->getTimer_queue();
    routing_table = paser_global->getRouting_table();
    neighbor_table = paser_global->getNeighbor_table();
    message_queue = paser_global->getMessage_queue();
    route_findung = paser_global->getRoute_findung();

    rreq_list = paser_global->getRreq_list();
    rrep_list = paser_global->getRrep_list();

    root = paser_global->getRoot();
    crypto_sign = paser_global->getCrypto_sign();
    crypto_hash = paser_global->getCrypto_hash();

//    AddL = paser_global->getAddL();

    isGW = paser_global->getIsGW();
//    isRegistered = paser_global->getIsRegistered();
//    wasRegistered = paser_global->getWasRegistered();

    netDevice = paser_global->getNetDevice();

    if (paser_configuration->getGTK().len > 0) {
        GTK.buf = paser_configuration->getGTK().buf;
        GTK.len = paser_configuration->getGTK().len;
    } else {
        GTK.len = 0;
        GTK.buf = NULL;
    }

}

PASER_Message_Processing::~PASER_Message_Processing() {
    // GTK.buf ist ein zeiger auf paser_config->gtk.buf, und der Speicher wird im paser_config freigegeben
//    if(GTK.len > 0){
//        free(GTK.buf);
//    }
}

//Funktion bearbeitet den ankommenden PASER Paket
void PASER_Message_Processing::handleLowerMsg(cPacket * msg, u_int32_t ifIndex) {
    //ein CRL-Replay ist angekommen
    if (dynamic_cast<crl_message *>(msg)) {
        checkKDCReply(msg);
        return;
    }
//    if(dynamic_cast<kdcReset *>(msg)){
//        checkKDCReset(msg);
//        return;
//    }
    //Falls kein AuthTree vorhanden ist werden alle Pakete weggeworfen, da generierung eine AuthTree viel Zeit kostet(bis zu 5 sek.)
    if (!paser_configuration->isRootReady) {
        delete msg;
        return;
    }
    PASER_MSG *paser_msg = check_and_cast<PASER_MSG *>(msg);
    switch (paser_msg->type) {
    case 0:
        EV << "Incoming PASER_UB_RREQ\n";
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("verify_rsa_delay");
        handleUBRREQ(msg, ifIndex);
        break;
    case 1:
        EV << "Incoming PASER_UU_RREP\n";
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("verify_rsa_delay");
        handleUURREP(msg, ifIndex);
        break;
    case 2:
        EV << "Incoming PASER_TU_RREQ\n";
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("auth_tree_verify_delay");
        handleTURREQ(msg, ifIndex);
        break;
    case 3:
        EV << "Incoming PASER_TU_RREP\n";
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("auth_tree_verify_delay");
        handleTURREP(msg, ifIndex);
        break;
    case 4:
        EV << "Incoming PASER_TU_RREP_ACK\n";
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("auth_tree_verify_delay");
        handleTURREPACK(msg, ifIndex);
        break;
    case 5:
        EV << "Incoming PASER_TB_RERR\n";
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("auth_tree_verify_delay");
        handleRERR(msg, ifIndex);
        break;
    case 6:
        EV << "Incoming PASER_TB_Hello\n";
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("auth_tree_verify_delay");
        handleHELLO(msg, ifIndex);
        break;
    case 7:
        EV << "Incoming PASER_UB_Root_Refresh\n";
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("verify_rsa_delay");
        handleB_ROOT(msg, ifIndex);
        break;
    case 8:
        EV << "Incoming PASER_B_RESET\n";
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("verify_rsa_delay");
        handleB_RESET(msg, ifIndex);
        break;
    default:
        EV << "false PASER Message type\n";
        delete msg;
        return;
    }
}

/**
 * Funktion prueft den Sequenznummer des Pakets
 * return 1: message is new
 *        0: message is old
 */
int PASER_Message_Processing::check_seq_nr(PASER_MSG *paser_msg,
        struct in_addr forwarding) {
    PASER_Routing_Entry *srcNode = NULL;
    struct in_addr destAddr;
    // beim UB_RREQ und TU_RREQ Paketen wird nicht nur die Sequenznummer des Absenders, sondern auch die
    // Sequenznummer des Knotens ueberprueft, das das Paket weitergeleitet hat.
    if (paser_msg->type == UB_RREQ || paser_msg->type == TU_RREQ) {
        u_int32_t seqForw = 0;
        if (paser_msg->type == UB_RREQ) {
            PASER_UB_RREQ *ubrreq_msg = check_and_cast<PASER_UB_RREQ *>(
                    paser_msg);
            seqForw = ubrreq_msg->seqForw;
        } else {
            PASER_TU_RREQ *turreq_msg = check_and_cast<PASER_TU_RREQ *>(
                    paser_msg);
            seqForw = turreq_msg->seqForw;
        }
        EV << "seq: " << paser_msg->seq << " seqForw: " << seqForw << "\n";
        srcNode = routing_table->findDest(paser_msg->srcAddress_var);
        destAddr.S_addr = paser_msg->destAddress_var.S_addr;

        PASER_Routing_Entry *forwardingNode = routing_table->findDest(
                forwarding);
        if (srcNode && isGW
                && (destAddr.S_addr.getIPv4().getInt() == 0xFFFFFFFF
                        || paser_modul->isMyLocalAddress(destAddr))
                && forwardingNode) {
//            if(paser_msg->seq >= srcNode->seqnum && seqForw > forwardingNode->seqnum){
            if ((paser_msg->seq == srcNode->seqnum
                    || paser_global->isSeqNew(srcNode->seqnum, paser_msg->seq))
                    && paser_global->isSeqNew(forwardingNode->seqnum, seqForw)) {
                return 1;
            } else {
                return 0;
            }
        }
        if (srcNode && paser_modul->isMyLocalAddress(destAddr)
                && forwardingNode) {
//            if(paser_msg->seq >= srcNode->seqnum && seqForw > forwardingNode->seqnum){
            if ((paser_msg->seq == srcNode->seqnum
                    || paser_global->isSeqNew(srcNode->seqnum, paser_msg->seq))
                    && paser_global->isSeqNew(forwardingNode->seqnum, seqForw)) {
                return 1;
            } else {
                return 0;
            }
        }
        if (srcNode && !paser_modul->isMyLocalAddress(destAddr)
                && forwardingNode) {
//            if(paser_msg->seq > srcNode->seqnum && seqForw > forwardingNode->seqnum){
            if (paser_global->isSeqNew(srcNode->seqnum, paser_msg->seq)
                    && paser_global->isSeqNew(forwardingNode->seqnum, seqForw)) {
                return 1;
            } else {
                return 0;
            }
        }
        if (!srcNode && forwardingNode) {
//            if(seqForw > forwardingNode->seqnum){
            if (paser_global->isSeqNew(forwardingNode->seqnum, seqForw)) {
                return 1;
            } else {
                return 0;
            }
        }
        if (srcNode && !forwardingNode) {
//            if(paser_msg->seq > srcNode->seqnum){
            if (paser_global->isSeqNew(srcNode->seqnum, paser_msg->seq)) {
                return 1;
            } else {
                return 0;
            }
        }
        return 1;
    } else if (paser_msg->type == UU_RREP || paser_msg->type == TU_RREP) {
        srcNode = routing_table->findDest(paser_msg->destAddress_var);
        EV << "seq: " << paser_msg->seq << "\n";
        destAddr.S_addr = paser_msg->srcAddress_var.S_addr;

        if (srcNode
                && (destAddr.S_addr.getIPv4().getInt() == 0xFFFFFFFF
                        || paser_modul->isMyLocalAddress(destAddr))) {
//            if(paser_msg->seq >= srcNode->seqnum){
            if (paser_msg->seq == srcNode->seqnum
                    || paser_global->isSeqNew(srcNode->seqnum, paser_msg->seq)) {
                return 1;
            } else {
                return 0;
            }
        }
        if (srcNode) {
            EV << "srcNode->seqnum: " << srcNode->seqnum << "\n";
//            if(paser_msg->seq > srcNode->seqnum){
            if (paser_global->isSeqNew(srcNode->seqnum, paser_msg->seq)) {
                return 1;
            } else {
                return 0;
            }
        }
        return 1;
    } else if (paser_msg->type == TU_RREP_ACK) {
        srcNode = routing_table->findDest(paser_msg->srcAddress_var);
        EV << "seq: " << paser_msg->seq << "\n";
        destAddr.S_addr = paser_msg->srcAddress_var.S_addr;

        if (srcNode && isGW
                && (destAddr.S_addr.getIPv4().getInt() == 0xFFFFFFFF
                        || paser_modul->isMyLocalAddress(destAddr))) {
//            if(paser_msg->seq > srcNode->seqnum){
            if (paser_global->isSeqNew(srcNode->seqnum, paser_msg->seq)) {
                return 1;
            } else {
                return 0;
            }
        }
        if (srcNode) {
            EV << "srcNode->seqnum: " << srcNode->seqnum << "\n";
//            if(paser_msg->seq > srcNode->seqnum){
            if (paser_global->isSeqNew(srcNode->seqnum, paser_msg->seq)) {
                return 1;
            } else {
                return 0;
            }
        }
        return 1;
    } else if (paser_msg->type == B_RERR || paser_msg->type == B_HELLO
            || paser_msg->type == B_ROOT) {
        srcNode = routing_table->findDest(paser_msg->srcAddress_var);
        EV << "seq: " << paser_msg->seq << "\n";
        destAddr.S_addr = paser_msg->srcAddress_var.S_addr;
        if (srcNode) {
            EV << "srcNode->seqnum: " << srcNode->seqnum << "\n";
//            if(paser_msg->seq > srcNode->seqnum){
            if (paser_global->isSeqNew(srcNode->seqnum, paser_msg->seq)) {
                return 1;
            } else {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

/**
 * Funktion prueft GeoPosition des Absenders
 * @return: 1 - Absender befindet sich in WLAN Radius
 *          0 - false
 */
int PASER_Message_Processing::check_geo(geo_pos position) {

//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
//    geo_pos myGeo;
//    myGeo.lat = myPos.x;
//    myGeo.lon = myPos.y;
    geo_pos myGeo = paser_global->getGeoPosition();

    double temp = (position.lat - myGeo.lat) * (position.lat - myGeo.lat)
            + (position.lon - myGeo.lon) * (position.lon - myGeo.lon);
    EV << "myPos x: " << myGeo.lat << " myPos y: " << myGeo.lon << "\n";
    EV << "Node x: " << position.lat << " Node: " << position.lon << "\n";
    EV << "dist: " << temp << " max Dist: " << PASER_radius * PASER_radius
            << "\n";
    if (temp > (PASER_radius * PASER_radius)) {
        return 0;
    }
    return 1;
}

/*
 * Hier wird geprueft, ob der Knoten in "adress_list" vorkommt. Falls ja, dann wurde das Message schon vom Knote verarbeitet.
 * return: 1 - Schleife
 *         0 - OK
 */
int PASER_Message_Processing::checkRouteList(std::list<address_list> rList) {
    EV << " Entering checkRouteList \n";
    for (std::list<address_list>::iterator it = rList.begin();
            it != rList.end(); it++) {
        EV << " Entering for in checkRouteList \n";
        if (paser_modul->isMyLocalAddress(((address_list) *it).ipaddr)) {
            return 1;
        }
        EV << " Entering for in checkRouteList getting end of for \n";
    }
    return 0;
}

/**
 * Process UBRREQ message
 */
void PASER_Message_Processing::handleUBRREQ(cPacket * msg, u_int32_t ifIndex) {
    PASER_UB_RREQ *ubrreq_msg = check_and_cast<PASER_UB_RREQ *>(msg);
    EV << " Entering handleUBRREQ \n";
    if (!paser_global->getWasRegistered()) {
        delete ubrreq_msg;
        return;
    }
    EV << " checking AddressRangeList \n";
    if (checkRouteList(ubrreq_msg->AddressRangeList)) {
        delete ubrreq_msg;
        return;
    }
    struct in_addr forwarding = ubrreq_msg->AddressRangeList.back().ipaddr;
    // Check message sequence number
    //Pruefe Sequenznummer des Pakets
    EV << " checking sequence number \n";
    if (!check_seq_nr((check_and_cast<PASER_MSG *>(msg)), forwarding)) {
        EV << "REPLAY SEQ\n";
        delete ubrreq_msg;
        return;
    }
    // Check geo position of sending node
    //Pruefe GeoPosition des Absenders
    EV << " checking sender range \n";
    if (!check_geo(ubrreq_msg->geoForwarding)) {
        EV << "WORMHOLE\n";
        delete ubrreq_msg;
        return;
    }

    EV << "iv = " << ubrreq_msg->initVector << "\n";

    // Check Timestamp
    //pruefe Timestamp
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    if (now.tv_sec - ubrreq_msg->timestamp > PASER_time_diff
            || now.tv_sec - ubrreq_msg->timestamp < -PASER_time_diff) {
        EV << "old paket\n";
        delete ubrreq_msg;
        return;
    }

    // Check message signature
    //Pruefe Signatur des Pakets
    if (!crypto_sign->checkSignUBRREQ(ubrreq_msg)) {
        EV << "SIGN ERROR\n";
        delete ubrreq_msg;
        return;
    } else {
        EV << "SIGN OK\n";
    }
    // Check keyNr
    //pruefe keyNr
    if (ubrreq_msg->keyNr != paser_configuration->getKeyNr()
            && ubrreq_msg->keyNr != 0) {
        send_reset();
        delete ubrreq_msg;
        return;
    }
    EV << "destAddr: "
            << ubrreq_msg->destAddress_var.S_addr.getIPv4().str() << "\n";
    // Update neighbor table
    //aktualisiere NeighborTable
    X509 *certNeigh = crypto_sign->extractCert(ubrreq_msg->certForw);
    neighbor_table->updateNeighborTableAndSetTableTimeout(forwarding, 0,
            ubrreq_msg->root, ubrreq_msg->initVector, ubrreq_msg->geoForwarding,
            certNeigh, now, ifIndex);
    // Update routing table with information of neighbor
    //aktualisiere RoutingTable mit der Information ueber den Nachbar
    std::list<address_range> addList(ubrreq_msg->AddressRangeList.back().range);
    X509 *certForw = crypto_sign->extractCert(ubrreq_msg->certForw);
    routing_table->updateRoutingTableAndSetTableTimeout(addList, forwarding,
            ubrreq_msg->seqForw, certForw, forwarding, 0, ifIndex, now,
            crypto_sign->isGwCert(certForw), false);
    // Update routing table with information of sending node
    //aktualisiere RoutingTable mit der Information des Absenders
    X509 *cert = NULL;
    if (ubrreq_msg->GFlag) {
        cert = crypto_sign->extractCert(ubrreq_msg->cert);
        if (cert != NULL && crypto_sign->checkOneCert(cert) == 0) {
            X509_free(cert);
            delete ubrreq_msg;
            return;
        }
    }
    routing_table->updateRoutingTableAndSetTableTimeout(
            ubrreq_msg->AddressRangeList.front().range,
            ubrreq_msg->srcAddress_var, ubrreq_msg->seq, cert, forwarding,
            ubrreq_msg->metricBetweenQueryingAndForw, ifIndex, now,
            crypto_sign->isGwCert(cert), false);

    PASER_Routing_Entry *rEntry = routing_table->findDest(
            ubrreq_msg->srcAddress_var);
    PASER_Neighbor_Entry *nEntry = NULL;
    if (rEntry) {
        nEntry = neighbor_table->findNeigh(rEntry->nxthop_addr);
    }

    //Aktualisiere die RoutingTabele mit der Informationen ueber alle Knoten, die die Nachricht weitergeleitet haben
    if (nEntry && nEntry->neighFlag && nEntry->isValid) {
        routing_table->updateRoutingTable(now, ubrreq_msg->AddressRangeList,
                forwarding, ifIndex);
    }

    //Verschicke alle Pakete, die zwischen gespeichert sind
    if (nEntry && nEntry->neighFlag && nEntry->isValid) {
        struct in_addr netmask;
        netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        routing_table->updateKernelRoutingTable(ubrreq_msg->srcAddress_var,
                rEntry->nxthop_addr, netmask, rEntry->hopcnt, false,
                nEntry->ifIndex);
        deleteRouteRequestTimeout(ubrreq_msg->srcAddress_var);
        message_queue->send_queued_messages(ubrreq_msg->srcAddress_var);

        deleteRouteRequestTimeoutForAddList(ubrreq_msg->AddressRangeList);
        message_queue->send_queued_messages_for_AddList(
                ubrreq_msg->AddressRangeList);
    }

    int ifId = paser_modul->getIfIdFromIfIndex(ifIndex);
    // sende RREP
    if ((isGW && ubrreq_msg->destAddress_var.S_addr.getIPv4().getInt() == 0xFFFFFFFF)
            || (paser_modul->isMyLocalAddress(ubrreq_msg->destAddress_var)
                    && ifId >= 0
                    && (netDevice[ifId].ipaddr.S_addr
                            == ubrreq_msg->destAddress_var.S_addr))
            || (paser_configuration->isAddInMySubnetz(
                    ubrreq_msg->destAddress_var))) {
        EV << "send RREP" << "\n";
        // uu-rrep
        if (isGW && ubrreq_msg->GFlag) {
            //sende anfrage an KDC
            sendKDCRequest(ubrreq_msg->srcAddress_var, forwarding,
                    ubrreq_msg->cert, ubrreq_msg->nonce);
            delete ubrreq_msg;
            return;
        }
        in_addr WlanAddrStruct;
        PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(forwarding);
        ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
        WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
        EV << "nxtHopIf: " << netDevice[ifId].ifindex << "\n";
        EV << "nxtHopIfName: " << netDevice[ifId].ifname << "\n";
        EV << "my Addr: " << WlanAddrStruct.S_addr.getIPv4().str() << "\n";
        cert = (X509*) rEntry->Cert;
        kdc_block kdcData;
        PASER_UU_RREP *message = send_uu_rrep(ubrreq_msg->srcAddress_var,
                forwarding, WlanAddrStruct/*myAddrStruct*/, ubrreq_msg->GFlag,
                cert, kdcData);
        message_rreq_entry *rrep = rrep_list->pending_find(forwarding);
        if (rrep) {
            timer_queue->timer_remove(rrep->tPack);
            delete rrep->tPack;
        } else {
            rrep = rrep_list->pending_add(forwarding);
        }
        rrep->tries = 0;

        PASER_Timer_Message *tPack = new PASER_Timer_Message();
        tPack->data = (void *) message;
        tPack->destAddr.S_addr = forwarding.S_addr;
        tPack->handler = TU_RREP_ACK_TIMEOUT;
        tPack->timeout = timeval_add(now, PASER_UU_RREP_WAIT_TIME);

        EV << "now: " << now.tv_sec << "\ntimeout: " << tPack->timeout.tv_sec
                << "\n";
        timer_queue->timer_add(tPack);
        rrep->tPack = tPack;

    }
    // leite die Nachricht weiter
    else if (!paser_modul->isMyLocalAddress(ubrreq_msg->destAddress_var)) {
        PASER_Routing_Entry *routeToDest = NULL;
        if (ubrreq_msg->destAddress_var.S_addr.getIPv4().getInt() == 0xFFFFFFFF) {
            EV << "destAddr = GW\n";
            routeToDest = routing_table->getRouteToGw();
            if (!routeToDest) {
                EV << "routeToDest = NULL\n";
            } else if (!routeToDest->isValid) {
                EV << "routeToDest is not Valid\n";
            }
        } else {
            routeToDest = routing_table->findDest(ubrreq_msg->destAddress_var);
            if (!routeToDest) {
                routeToDest = routing_table->findAdd(
                        ubrreq_msg->destAddress_var);
                if (routeToDest) {
                    ubrreq_msg->destAddress_var = routeToDest->dest_addr;
                }
            }
        }
        PASER_Neighbor_Entry *neighToDest = NULL;
        if (routeToDest != NULL && routeToDest->isValid) {
            EV << "routeToDest != NULL\n";
            neighToDest = neighbor_table->findNeigh(routeToDest->nxthop_addr);
        }

        //Falls nexhop bekannt und vertraulich ist sende TURREQ, sonst UBRREQ
        if (neighToDest != NULL && neighToDest->isValid
                && neighToDest->neighFlag) {
            EV << "forward_ub_rreq_to_tu_rreq\n";
            if (forwarding.S_addr != routeToDest->nxthop_addr.S_addr) {
                PASER_TU_RREQ *newMessage = forward_ub_rreq_to_tu_rreq(ubrreq_msg,
                        routeToDest->nxthop_addr, routeToDest->dest_addr);
                delete newMessage;
            }
        } else {
            // UB-RREQ
            EV << "forward_ub_rreq\n";
            PASER_UB_RREQ *newMessage = forward_ub_rreq(ubrreq_msg);
            delete newMessage;
        }
    }

    delete ubrreq_msg;
}

void PASER_Message_Processing::handleUURREP(cPacket * msg, u_int32_t ifIndex) {
    PASER_UU_RREP *uurrep_msg = check_and_cast<PASER_UU_RREP *>(msg);
    if ((!paser_global->getIsRegistered()
            && !paser_modul->isMyLocalAddress(uurrep_msg->srcAddress_var)
            && !paser_global->getWasRegistered())) {
        EV << "unRegistred && wrong Destination\n";
        delete uurrep_msg;
        return;
    }

    if (checkRouteList(uurrep_msg->AddressRangeList)) {
        delete uurrep_msg;
        return;
    }

    struct in_addr forwarding = uurrep_msg->AddressRangeList.back().ipaddr;
    if (!check_seq_nr((check_and_cast<PASER_MSG *>(msg)), forwarding)) {
        EV << "REPLAY SEQ\n";
        delete uurrep_msg;
        return;
    }
    if (!check_geo(uurrep_msg->geoForwarding)) {
        EV << "WORMHOLE\n";
        delete uurrep_msg;
        return;
    }

    EV << "iv = " << uurrep_msg->initVector << "\n";

    //Check Timestamp
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    if (now.tv_sec - uurrep_msg->timestamp > PASER_time_diff
            || now.tv_sec - uurrep_msg->timestamp < -PASER_time_diff) {
        EV << "old paket\n";
        delete uurrep_msg;
        return;
    }

    // Read KDC
    if (uurrep_msg->GFlag
            && paser_modul->isMyLocalAddress(uurrep_msg->srcAddress_var)) {
        if (paser_global->getLastGwSearchNonce()
                != uurrep_msg->kdc_data.nonce) {
            EV << "wrong nonce!\n";
            EV << "my nonce: " << paser_global->getLastGwSearchNonce()
                    << " incomming nonce: " << uurrep_msg->kdc_data.nonce
                    << "\n";
            delete uurrep_msg;
            return;
        }
        if (crypto_sign->checkSignKDC(uurrep_msg->kdc_data) != 1) {
            EV << "checkSignKDC Error\n";
            delete uurrep_msg;
            return;
        }
//        //Check keyNr
//        if(uurrep_msg->keyNr != paser_configuration->getKeyNr()){
//            send_reset();
//            delete uurrep_msg;
//            return;
//        }
        //da GTK.buf ein Zeiger auf paser_config->gtk.buf ist, wird es auch in paser_config freigegeben
        lv_block gtk;
        gtk.len = 0;
        gtk.buf = NULL;
        crypto_sign->rsa_dencrypt(uurrep_msg->kdc_data.GTK, &gtk);
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("deryption_rsa_delay");
        paser_configuration->setGTK(gtk);
        if (gtk.len > 0) {
            free(gtk.buf);
        }
        //speichere neuen Zertifikat und Signatur von RESET
        paser_configuration->setKDC_cert(uurrep_msg->kdc_data.cert_kdc);
        paser_configuration->setRESET_sign(uurrep_msg->kdc_data.sign_key);

        paser_configuration->setKeyNr(uurrep_msg->kdc_data.key_nr);

        GTK.buf = paser_configuration->getGTK().buf;
        GTK.len = paser_configuration->getGTK().len;
    }

    if (!crypto_sign->checkSignUURREP(uurrep_msg)) {
        EV << "UURREP SIGN ERROR\n";
        delete uurrep_msg;
       return;
    } else {
        EV << "UURREP SIGN OK\n";
    }
    //Check keyNr
    if (uurrep_msg->keyNr != paser_configuration->getKeyNr()) {
        send_reset();
        delete uurrep_msg;
        return;
    }
    //Update neighbor table
    X509 *certNeigh = crypto_sign->extractCert(uurrep_msg->certForw);
    neighbor_table->updateNeighborTableAndSetTableTimeout(forwarding, 1,
            uurrep_msg->root, uurrep_msg->initVector, uurrep_msg->geoForwarding,
            certNeigh, now, ifIndex);

    //Update routing table with forwarding node
    std::list<address_range> addList(uurrep_msg->AddressRangeList.back().range);
    X509 *certForw = crypto_sign->extractCert(uurrep_msg->certForw);
    routing_table->updateRoutingTableAndSetTableTimeout(addList, forwarding,
    /*uurrep_msg->seqForw,*/0, certForw, forwarding, 0, ifIndex, now,
            crypto_sign->isGwCert(certForw), true);

    //Update Routing Table
//    std::list<address_range> EmptyAddList( uurrep_msg->AddressRangeList.front().range );
    routing_table->updateRoutingTableAndSetTableTimeout(
            uurrep_msg->AddressRangeList.front().range,
            uurrep_msg->destAddress_var, uurrep_msg->seq, NULL, forwarding,
            uurrep_msg->metricBetweenDestAndForw, ifIndex, now,
            uurrep_msg->GFlag, true);

    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(forwarding);

    if (nEntry && nEntry->neighFlag && nEntry->isValid) {
        routing_table->updateRoutingTable(now, uurrep_msg->AddressRangeList,
                forwarding, ifIndex);
    }

    // Send TU-RREP-ACK
    in_addr WlanAddrStruct;
//    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh( forwarding );
    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
    PASER_TU_RREP_ACK * turrepack = send_tu_rrep_ack(
            WlanAddrStruct/*myAddrStruct*/, forwarding);
    delete turrepack;

    if (uurrep_msg->GFlag) {
//        if(!paser_global->getIsRegistered() && !paser_global->getWasRegistered()){
//            sendCrlRequest();
//            delete uurrep_msg;
//            return;
//        }
        paser_global->setIsRegistered(true);
        paser_global->setWasRegistered(true);
    }

    // Delete TIMEOUT
    struct in_addr bcast_addr;
    bcast_addr.s_addr.set(IPv4Address::ALLONES_ADDRESS);
    if (uurrep_msg->GFlag == 0x01) {
        deleteRouteRequestTimeout(bcast_addr);
    }
    deleteRouteRequestTimeout(uurrep_msg->destAddress_var);
    message_queue->send_queued_messages(uurrep_msg->destAddress_var);
    deleteRouteRequestTimeoutForAddList(uurrep_msg->AddressRangeList);
    message_queue->send_queued_messages_for_AddList(uurrep_msg->AddressRangeList);

    if (!paser_modul->isMyLocalAddress(uurrep_msg->srcAddress_var)) {
        PASER_Routing_Entry *rEntry = routing_table->findDest(
                uurrep_msg->srcAddress_var);
        if (rEntry != NULL && rEntry->isValid) {
            PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
                    rEntry->nxthop_addr);
            if (nEntry != NULL && nEntry->isValid) {
                if (nEntry->neighFlag) {
                    //forwarding TU-RREP
                    PASER_TU_RREP *message = forward_uu_rrep_to_tu_rrep(
                            uurrep_msg, rEntry->nxthop_addr);
                    delete message;
                } else {
                    //forwarding UU-RREP
                    EV << "forwarding UU-RREP\n";
                    PASER_UU_RREP *message = forward_uu_rrep(uurrep_msg,
                            rEntry->nxthop_addr);
                    if (message == NULL) {
                        delete uurrep_msg;
                        return;
                    }
                    PASER_Routing_Entry *rEntry = routing_table->findDest(
                            message->srcAddress_var);
                    message_rreq_entry *rrep = rrep_list->pending_find(
                            rEntry->nxthop_addr);
                    if (rrep) {
                        timer_queue->timer_remove(rrep->tPack);
                        delete rrep->tPack;
                    } else {
                        rrep = rrep_list->pending_add(rEntry->nxthop_addr);
                    }
                    rrep->tries = 0;

                    PASER_Timer_Message *tPack = new PASER_Timer_Message();
                    tPack->data = (void *) message;
                    tPack->destAddr.S_addr = rEntry->nxthop_addr.S_addr;
                    tPack->handler = TU_RREP_ACK_TIMEOUT;
                    tPack->timeout = timeval_add(now, PASER_UU_RREP_WAIT_TIME);

                    EV << "now: " << now.tv_sec << "\ntimeout: "
                            << tPack->timeout.tv_sec << "\n";
                    timer_queue->timer_add(tPack);
                    rrep->tPack = tPack;
                }
            } else {
                ev
                        << "handleUURREP Error: Could not find the destination node(Route not exist)!\n";
                std::list<unreachableBlock> allAddrList;
                unreachableBlock temp;
                temp.addr.S_addr = uurrep_msg->srcAddress_var.S_addr;
                temp.seq = 0;
                allAddrList.push_back(temp);
                paser_global->getPaket_processing()->send_rerr(allAddrList);
                delete uurrep_msg;
                return;
            }
        } else {
            ev
                    << "handleUURREP Error: Could not find the destination node(NextHop not exist)!\n";
            std::list<unreachableBlock> allAddrList;
            unreachableBlock temp;
            temp.addr.S_addr = uurrep_msg->srcAddress_var.S_addr;
            temp.seq = 0;
            allAddrList.push_back(temp);
            paser_global->getPaket_processing()->send_rerr(allAddrList);
            delete uurrep_msg;
            return;
        }
    }
    delete uurrep_msg;
}

void PASER_Message_Processing::handleTURREQ(cPacket * msg, u_int32_t ifIndex) {
    PASER_TU_RREQ *turreq_msg = check_and_cast<PASER_TU_RREQ *>(msg);
    if (!paser_global->getWasRegistered()) {
        delete turreq_msg;
        return;
    }
    //pruefe keyNr
    if (turreq_msg->keyNr != paser_configuration->getKeyNr()) {
        send_reset();
        delete turreq_msg;
        return;
    }
    if (checkRouteList(turreq_msg->AddressRangeList)) {
        delete turreq_msg;
        return;
    }
    struct in_addr forwarding = turreq_msg->AddressRangeList.back().ipaddr;
    if (!check_seq_nr((check_and_cast<PASER_MSG *>(msg)), forwarding)) {
        EV << "REPLAY SEQ\n";
        delete turreq_msg;
        return;
    }
    if (!check_geo(turreq_msg->geoForwarding)) {
        EV << "WORMHOLE\n";
        delete turreq_msg;
        return;
    }

    PASER_Neighbor_Entry *neigh = neighbor_table->findNeigh(forwarding);
    if (!neigh || !neigh->neighFlag) {
        EV << "neighbor_table->findNeigh ERROR!\n";
        delete turreq_msg;
        return;
    }
    if (!crypto_hash->checkHmacTURREQ(turreq_msg, GTK)) {
        EV << "checkHmacTURREQ ERROR!\n";
        delete turreq_msg;
        return;
    }
    u_int32_t newIV = 0;
    if (!root->root_check_root(neigh->root, turreq_msg->secret,
            turreq_msg->auth, neigh->IV, &newIV)) {
        EV << "root_check_root ERROR\n";
        delete turreq_msg;
        return;
    }
    neighbor_table->updateNeighborTableIVandSetValid(neigh->neighbor_addr,
            newIV);
    EV << "root_check_root OK\n";

    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);

    //Update neighbor table
    neighbor_table->updateNeighborTableTimeout(forwarding, now);

    //Update routing table with forwarding node
    routing_table->updateRoutingTableTimeout(forwarding, turreq_msg->seqForw,
            now);

    //Update routing table
//    std::list<address_range> EmptyAddList;
    X509 *cert = NULL;
    if (turreq_msg->GFlag) {
        cert = crypto_sign->extractCert(turreq_msg->cert);
        if (cert != NULL && crypto_sign->checkOneCert(cert) == 0) {
            X509_free(cert);
            delete turreq_msg;
            return;
        }
    }
    routing_table->updateRoutingTableAndSetTableTimeout(
            turreq_msg->AddressRangeList.front().range,
            turreq_msg->srcAddress_var, turreq_msg->seq, cert, forwarding,
            turreq_msg->metricBetweenQueryingAndForw, ifIndex, now,
            crypto_sign->isGwCert(cert), true);

    PASER_Routing_Entry *rEntry = routing_table->findDest(
            turreq_msg->srcAddress_var);
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
            rEntry->nxthop_addr);
    if (nEntry && nEntry->neighFlag && nEntry->isValid) {
        routing_table->updateRoutingTable(now, turreq_msg->AddressRangeList,
                forwarding, ifIndex);
    }
    // Release queued Messages
    if (nEntry && nEntry->neighFlag) {
        deleteRouteRequestTimeout(turreq_msg->srcAddress_var);
        message_queue->send_queued_messages(turreq_msg->srcAddress_var);
        deleteRouteRequestTimeoutForAddList(turreq_msg->AddressRangeList);
        message_queue->send_queued_messages_for_AddList(
                turreq_msg->AddressRangeList);
    }

    if (paser_modul->isMyLocalAddress(turreq_msg->destAddress_var)) {
        EV << "send TURREP" << "\n";
        // UU-RREP
        if (turreq_msg->GFlag) {
            cert = crypto_sign->extractCert(turreq_msg->cert);
            if (isGW) {
                // Send request to KDC
                //sende anfrage an KDC
                sendKDCRequest(turreq_msg->srcAddress_var, forwarding,
                        turreq_msg->cert, turreq_msg->nonce);
                X509_free(cert);
                delete turreq_msg;
                return;
            }
        }
        in_addr WlanAddrStruct;
        PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(forwarding);
        int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
        WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
        kdc_block tempKdcBlock;
        PASER_TU_RREP *message = send_tu_rrep(turreq_msg->srcAddress_var,
                forwarding, WlanAddrStruct/*myAddrStruct*/, turreq_msg->GFlag,
                cert, tempKdcBlock);
        if (turreq_msg->GFlag) {
            X509_free(cert);
        }
        delete message;
    }
    // Forwarding TURREQ
    else {
        EV << "forwarding TURREP" << "\n";
        PASER_Routing_Entry *rEntry = routing_table->findDest(
                turreq_msg->destAddress_var);
        if (rEntry == NULL || !rEntry->isValid) {
            ev
                    << "handleUURREP Error: Could not find the destination node(Route not exist)!\n";
            std::list<unreachableBlock> allAddrList;
            unreachableBlock temp;
            temp.addr.S_addr = turreq_msg->destAddress_var.S_addr;
            temp.seq = 0;
            allAddrList.push_back(temp);
            paser_global->getPaket_processing()->send_rerr(allAddrList);
            delete turreq_msg;
            return;
        }
        PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
                rEntry->nxthop_addr);
        if (nEntry == NULL || !nEntry->neighFlag || !nEntry->isValid) {
            ev
                    << "handleUURREP Error: Could not find the destination node(NextHop not exist)!\n";
            std::list<unreachableBlock> allAddrList;
            unreachableBlock temp;
            temp.addr.S_addr = turreq_msg->destAddress_var.S_addr;
            temp.seq = 0;
            allAddrList.push_back(temp);
            paser_global->getPaket_processing()->send_rerr(allAddrList);
            delete turreq_msg;
            return;
        } else {
            PASER_TU_RREQ *message = forward_tu_rreq(turreq_msg,
                    rEntry->nxthop_addr);
            delete message;
            delete turreq_msg;
            return;
        }
    }
    delete turreq_msg;
}

void PASER_Message_Processing::handleTURREP(cPacket * msg, u_int32_t ifIndex) {
    PASER_TU_RREP *turrep_msg = check_and_cast<PASER_TU_RREP *>(msg);
    if (!paser_global->getWasRegistered()) {
        delete turrep_msg;
        return;
    }
    //Pruefe keyNr
    if (turrep_msg->keyNr != paser_configuration->getKeyNr()) {
        send_reset();
        delete turrep_msg;
        return;
    }
    if (checkRouteList(turrep_msg->AddressRangeList)) {
        delete turrep_msg;
        return;
    }
    struct in_addr forwarding = turrep_msg->AddressRangeList.back().ipaddr;
    EV << "forwarding: " << forwarding.S_addr.getIPv4().str() << "\n";
    if (!check_seq_nr((check_and_cast<PASER_MSG *>(msg)), forwarding)) {
        EV << "REPLAY SEQ\n";
        delete turrep_msg;
        return;
    }
    if (!check_geo(turrep_msg->geoForwarding)) {
        EV << "WORMHOLE\n";
        delete turrep_msg;
        return;
    }
    PASER_Neighbor_Entry *neigh = neighbor_table->findNeigh(forwarding);
    if (!neigh || !neigh->neighFlag) {
        EV << "neighbor_table->findNeigh ERROR!\n";
        delete turrep_msg;
        return;
    }
    if (!crypto_hash->checkHmacTURREP(turrep_msg, GTK)) {
        EV << "checkHmacTURREPACK ERROR!\n";
        delete turrep_msg;
        return;
    }

    // Read KDC
    if (turrep_msg->GFlag
            && paser_modul->isMyLocalAddress(turrep_msg->srcAddress_var)) {
        if (paser_global->getLastGwSearchNonce()
                != turrep_msg->kdc_data.nonce) {
            delete turrep_msg;
            return;
        }
        if (crypto_sign->checkSignKDC(turrep_msg->kdc_data) != 1) {
            delete turrep_msg;
            return;
        }
        //Da GTK.buf ein Zeiger auf paser_config->gtk.buf ist, wird es auch in paser_config freigegeben
        lv_block gtk;
        gtk.len = 0;
        gtk.buf = NULL;
        crypto_sign->rsa_dencrypt(turrep_msg->kdc_data.GTK, &gtk);
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("deryption_rsa_delay");
        paser_configuration->setGTK(gtk);
        if (gtk.len > 0) {
            free(gtk.buf);
        }
        GTK.buf = paser_configuration->getGTK().buf;
        GTK.len = paser_configuration->getGTK().len;
    }

    u_int32_t newIV = 0;
    if (!root->root_check_root(neigh->root, turrep_msg->secret,
            turrep_msg->auth, neigh->IV, &newIV)) {
        EV << "root_check_root ERROR\n";
        delete turrep_msg;
        return;
    }
    neighbor_table->updateNeighborTableIVandSetValid(neigh->neighbor_addr,
            newIV);
    EV << "root_check_root OK\n";

    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);

    EV << "update Neighbor Table\n";
    //update Neighbor Table
    neighbor_table->updateNeighborTableTimeout(forwarding, now);
    EV << "update Routing Table with forwarding Node\n";
    //update Routing Table with forwarding Node
    EV << "update Routing Table with forwarding Node\n";
    routing_table->updateRoutingTableTimeout(forwarding,
//            /*turrep_msg->seqForw,*/0,
            now, ifIndex);
    //update Routing Table
//    std::list<address_range> EmptyAddList;
    EV << "update Routing Table\n";
    routing_table->updateRoutingTableAndSetTableTimeout(
            turrep_msg->AddressRangeList.front().range,
            turrep_msg->destAddress_var, turrep_msg->seq, NULL, forwarding,
            turrep_msg->metricBetweenDestAndForw, ifIndex, now,
            turrep_msg->GFlag, true);

//    routing_table->findBestGW();

    if (turrep_msg->GFlag) {
//        if(!paser_global->getIsRegistered() && !paser_global->getWasRegistered()){
//            sendCrlRequest();
//            delete turrep_msg;
//            return;
//        }
        paser_global->setIsRegistered(true);
        paser_global->setWasRegistered(true);
    }

    // Delete TIMEOUT
    struct in_addr bcast_addr;
    bcast_addr.s_addr.set(IPv4Address::ALLONES_ADDRESS);
    if (turrep_msg->GFlag == 0x01) {
        deleteRouteRequestTimeout(bcast_addr);
    }

    EV << "update AddressRangeList\n";
    PASER_Routing_Entry *rEntry = routing_table->findDest(
            turrep_msg->destAddress_var);
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
            rEntry->nxthop_addr);
    if (nEntry && nEntry->neighFlag && nEntry->isValid) {
        routing_table->updateRoutingTable(now, turrep_msg->AddressRangeList,
                forwarding, ifIndex);
    }
    EV << "send queued Messages\n";
    // Release queued Messages
    if (nEntry && nEntry->neighFlag) {
        deleteRouteRequestTimeout(turrep_msg->destAddress_var);
        message_queue->send_queued_messages(turrep_msg->destAddress_var);
        deleteRouteRequestTimeoutForAddList(turrep_msg->AddressRangeList);
        message_queue->send_queued_messages_for_AddList(
                turrep_msg->AddressRangeList);
    }

    // Am I a querying Node? !!!
    if (paser_modul->isMyLocalAddress(turrep_msg->srcAddress_var)) {
        // Delete TIMEOUT
        struct in_addr bcast_addr;
        bcast_addr.s_addr.set(IPv4Address::ALLONES_ADDRESS);
        message_rreq_entry *rreq_bc = rreq_list->pending_find(bcast_addr);
        if (rreq_bc && turrep_msg->GFlag == 0x01) {
            rreq_list->pending_remove(rreq_bc);
            PASER_Timer_Message *timeout = rreq_bc->tPack;
            timer_queue->timer_remove(timeout);
            delete timeout;
            delete rreq_bc;
        }
        message_rreq_entry *rreq = rreq_list->pending_find(
                turrep_msg->destAddress_var);
        if (rreq) {
            rreq_list->pending_remove(rreq);
            PASER_Timer_Message *timeout = rreq->tPack;
            timer_queue->timer_remove(timeout);
            delete timeout;
            delete rreq;
        }
        EV << "send queued messages\n";
        message_queue->send_queued_messages(turrep_msg->destAddress_var);
    } else {
        // Forwarding
        PASER_Routing_Entry *rout = routing_table->findDest(
                turrep_msg->srcAddress_var);
        PASER_Neighbor_Entry *neigh = NULL;
        if (rout != NULL) {
            neigh = neighbor_table->findNeigh(rout->nxthop_addr);
        }
        if (rout == NULL || neigh == NULL) {
            //ERROR
            EV << "ERROR!\n";
            delete turrep_msg;
            return;
        }
        // Forwarding TURREP to UURREP
        if (neigh->neighFlag == 0) {
            EV << "forwarding TURREP to UURREP\n";
            PASER_UU_RREP *message = forward_tu_rrep_to_uu_rrep(turrep_msg,
                    neigh->neighbor_addr);
            // set TU-RREP-ACK Timeout
            message_rreq_entry *rrep = rrep_list->pending_find(
                    neigh->neighbor_addr);
            if (rrep) {
                timer_queue->timer_remove(rrep->tPack);
                delete rrep->tPack;
            } else {
                rrep = rrep_list->pending_add(neigh->neighbor_addr);
            }
            rrep->tries = 0;

            PASER_Timer_Message *tPack = new PASER_Timer_Message();
            tPack->data = (void *) message;
            tPack->destAddr.S_addr = neigh->neighbor_addr.S_addr;
            tPack->handler = TU_RREP_ACK_TIMEOUT;
            tPack->timeout = timeval_add(now, PASER_UU_RREP_WAIT_TIME);

            EV << "now: " << now.tv_sec << "\ntimeout: "
                    << tPack->timeout.tv_sec << "\n";
            timer_queue->timer_add(tPack);
            rrep->tPack = tPack;
        }
        // Forwarding TURREP!!!
        else {
            EV << "forwarding TU-RREP\n";
            PASER_Routing_Entry *rEntry = routing_table->findDest(
                    turrep_msg->srcAddress_var);
            if (rEntry == NULL) {

            } else {
                PASER_TU_RREP *message = forward_tu_rrep(turrep_msg,
                        rEntry->nxthop_addr);
                delete message;
            }
        }
    }
    delete turrep_msg;
}

void PASER_Message_Processing::handleTURREPACK(cPacket * msg, u_int32_t ifIndex) {
    PASER_TU_RREP_ACK *turrepack_msg = check_and_cast<PASER_TU_RREP_ACK *>(msg);
    if (!paser_global->getWasRegistered()) {
        delete turrepack_msg;
        return;
    }
    //Check keyNr
    if (turrepack_msg->keyNr != paser_configuration->getKeyNr()) {
        send_reset();
        delete turrepack_msg;
        return;
    }
    struct in_addr neighbor = turrepack_msg->srcAddress_var;
    PASER_Neighbor_Entry *neigh = neighbor_table->findNeigh(neighbor);
    if (!neigh) {
        EV << "neighbor_table->findNeigh ERROR!\n";
        delete turrepack_msg;
        return;
    }
    if (!check_seq_nr((check_and_cast<PASER_MSG *>(msg)), neighbor)) {
        EV << "REPLAY SEQ\n";
        delete turrepack_msg;
        return;
    }
    if (!crypto_hash->checkHmacTURREPACK(turrepack_msg, GTK)) {
        EV << "checkHmacTURREPACK ERROR!\n";
        delete turrepack_msg;
        return;
    }
    u_int32_t newIV = 0;
    if (!root->root_check_root(neigh->root, turrepack_msg->secret,
            turrepack_msg->auth, neigh->IV, &newIV)) {
        EV << "root_check_root ERROR!\n";
        delete turrepack_msg;
        return;
    }
    neighbor_table->updateNeighborTableIVandSetValid(neigh->neighbor_addr,
            newIV);
    EV << "root_check_root OK\n";

    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    //Update routing table with forwarding Node
    PASER_Routing_Entry *rEntry = routing_table->findDest(neighbor);
    if (!rEntry) {
        EV << "rEntry not found\n";
        delete turrepack_msg;
        return;
    }
    rEntry->nxthop_addr.S_addr = neighbor.S_addr;
    rEntry->hopcnt = 1;

    routing_table->updateRoutingTableTimeout(neighbor, turrepack_msg->seq, now);
    //Update neighbor table
    neighbor_table->updateNeighborTableTimeout(neighbor, now);
    neigh->neighFlag = 1;
    //OMNET: Kernel Routing Table
    struct in_addr netmask;
    netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
    if (true) {
   //     routing_table->updateKernelRoutingTable(neighbor, neighbor, netmask, 1, true, ifIndex);
        routing_table->updateKernelRoutingTable(neighbor, neighbor, netmask, 1,
                false, ifIndex);
    }

    // Release queued Messages
    PASER_Routing_Entry *rEntryTemp = routing_table->findDest(
            turrepack_msg->srcAddress_var);
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
            rEntryTemp->nxthop_addr);
    if (nEntry && nEntry->neighFlag && nEntry->isValid) {
        deleteRouteRequestTimeout(turrepack_msg->srcAddress_var);
        message_queue->send_queued_messages(turrepack_msg->srcAddress_var);
        std::list<PASER_Routing_Entry*> entryList =
                routing_table->getListWithNextHop(
                        turrepack_msg->srcAddress_var);
        std::list<address_list> addList;
        for (std::list<PASER_Routing_Entry*>::iterator it = entryList.begin();
                it != entryList.end(); it++) {
            address_list tempAddList;
            PASER_Routing_Entry *tempRoutingEntry = (PASER_Routing_Entry*) *it;
            tempAddList.ipaddr = tempRoutingEntry->dest_addr;
            tempAddList.range = tempRoutingEntry->AddL;
        }

        deleteRouteRequestTimeoutForAddList(addList);
        message_queue->send_queued_messages_for_AddList(addList);
    }

    message_rreq_entry * rrep = rrep_list->pending_find(
            turrepack_msg->srcAddress_var);
    EV << "src: " << turrepack_msg->srcAddress_var.S_addr.getIPv4().str()
            << " dest: "
            << turrepack_msg->destAddress_var.S_addr.getIPv4().str()
            << "\n";
    if (rrep) {
        PASER_Timer_Message *tPack = rrep->tPack;
        rrep_list->pending_remove(rrep);
        int k = timer_queue->timer_remove(tPack);
        if (k == 0) {
            EV << "timer not found\n";
        }
        delete tPack;
        delete rrep;
    } else {
        EV << "timeout not found\n";
    }
    delete turrepack_msg;
}

void PASER_Message_Processing::handleRERR(cPacket * msg, u_int32_t ifIndex) {
    PASER_TB_RERR *rerr_msg = check_and_cast<PASER_TB_RERR *>(msg);
    if (!paser_global->getWasRegistered()) {
        delete rerr_msg;
        return;
    }
    //Check keyNr
    if (rerr_msg->keyNr != paser_configuration->getKeyNr()) {
        send_reset();
        delete rerr_msg;
        return;
    }
    struct in_addr neighbor = rerr_msg->srcAddress_var;
    PASER_Neighbor_Entry *neigh = neighbor_table->findNeigh(neighbor);
    if (!neigh) {
        EV << "neighbor_table->findNeigh ERROR!\n";
        delete rerr_msg;
        return;
    }
    if (!check_seq_nr((check_and_cast<PASER_MSG *>(msg)), neighbor)) {
        EV << "REPLAY SEQ\n";
        delete rerr_msg;
        return;
    }
    //Check HASH
    if (!crypto_hash->checkHmacRERR(rerr_msg, GTK)) {
        EV << "checkHmacTURREPACK ERROR!\n";
        delete rerr_msg;
        return;
    }
    //Check root
    u_int32_t newIV = 0;
    if (!root->root_check_root(neigh->root, rerr_msg->secret, rerr_msg->auth,
            neigh->IV, &newIV)) {
        EV << "root_check_root ERROR!\n";
        delete rerr_msg;
        return;
    }
    neighbor_table->updateNeighborTableIV(neigh->neighbor_addr, newIV);
    EV << "root_check_root OK\n";

    std::list<unreachableBlock> forwardingList;
    //check each SeqNr
    for (std::list<unreachableBlock>::iterator it =
            rerr_msg->UnreachableAdressesList.begin();
            it != rerr_msg->UnreachableAdressesList.end(); it++) {
        unreachableBlock temp = (unreachableBlock) *it;
        PASER_Routing_Entry *tempEntry = routing_table->findDest(temp.addr);
        if (tempEntry == NULL
                || neighbor.S_addr != tempEntry->nxthop_addr.S_addr
                || !tempEntry->isValid) {
            continue;
        }
//        if(temp.seq < tempEntry->seqnum && temp.seq!=0){
        if (paser_global->isSeqNew(tempEntry->seqnum, temp.seq)
                && temp.seq != 0) {
            continue;
        }
        //loesche Route
        EV << "delete addr: "
                << tempEntry->dest_addr.S_addr.getIPv4().str() << "\n";
        for (std::list<address_range>::iterator it2 = tempEntry->AddL.begin();
                it2 != tempEntry->AddL.end(); it2++) {
            address_range addList = (address_range) *it2;
            EV << "    subnetz: " << addList.ipaddr.S_addr.getIPv4().str()
                    << "\n";
            routing_table->updateKernelRoutingTable(addList.ipaddr,
                    tempEntry->nxthop_addr, addList.mask, tempEntry->hopcnt + 1,
                    true, 0);
        }
        in_addr tempMask;
        tempMask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        routing_table->updateKernelRoutingTable(tempEntry->dest_addr,
                tempEntry->nxthop_addr, tempMask, tempEntry->hopcnt, true, 0);
        PASER_Timer_Message *validTimer = tempEntry->validTimer;
        if (validTimer) {
            EV << "loesche Timer\n";
            if (timer_queue->timer_remove(validTimer)) {
                EV << "Timer geloescht\n";
            } else {
                EV << "Timer wurde nicht geloescht\n";
            }
            delete validTimer;
            tempEntry->validTimer = NULL;
        }
        tempEntry->isValid = 0;
        if (temp.seq != 0) {
            tempEntry->seqnum = temp.seq;
        }
        unreachableBlock newEntry;
        newEntry.addr.S_addr = temp.addr.S_addr;
        newEntry.seq = temp.seq;
        forwardingList.push_back(newEntry);
    }

    //Generate and send RERR
    if (forwardingList.size() > 0) {
        send_rerr(forwardingList);
    }

    delete rerr_msg;
}

void PASER_Message_Processing::handleHELLO(cPacket * msg, u_int32_t ifIndex) {
    PASER_TB_Hello *hello_msg = check_and_cast<PASER_TB_Hello *>(msg);
    if (!paser_global->getWasRegistered()) {
        delete hello_msg;
        return;
    }
    struct in_addr neighbor = hello_msg->srcAddress_var;
    PASER_Neighbor_Entry *neigh = neighbor_table->findNeigh(neighbor);
    if (!neigh) {
        EV << "neighbor_table->findNeigh ERROR!\n";
        delete hello_msg;
        return;
    }
    if (!check_seq_nr((check_and_cast<PASER_MSG *>(msg)), neighbor)) {
        EV << "REPLAY SEQ\n";
        delete hello_msg;
        return;
    }
    //Check HASH
    if (!crypto_hash->checkHmacHELLO(hello_msg, GTK)) {
        EV << "checkHmacTURREPACK ERROR!\n";
        delete hello_msg;
        return;
    }
    //Check root
    u_int32_t newIV = 0;
    if (!root->root_check_root(neigh->root, hello_msg->secret, hello_msg->auth,
            neigh->IV, &newIV)) {
        EV << "root_check_root ERROR!\n";
        delete hello_msg;
        return;
    }
    EV << "root_check_root OK\n";

    if (!neigh->neighFlag) {
        EV << neigh->neighbor_addr.S_addr.getIPv4().str()
                << " is untrusted neighbor!\n";
        route_findung->route_discovery(neigh->neighbor_addr,0);
        delete hello_msg;
        return;
    }

    bool found = false;
    for (std::list<address_list>::iterator it =
            hello_msg->AddressRangeList.begin();
            it != hello_msg->AddressRangeList.end(); it++) {
        address_list tempMe = (address_list) *it;
        if (paser_modul->isMyLocalAddress(tempMe.ipaddr)) {
            found = true;
            break;
        }
    }
    if (!found) {
        EV << "der Nachbar vertraut mir nicht :(\n";
        route_findung->route_discovery(neigh->neighbor_addr,0);
        delete hello_msg;
        return;
    }
    neighbor_table->updateNeighborTableIVandSetValid(neigh->neighbor_addr,
            newIV);

    //Update all routes and neighbors
    for (std::list<address_list>::iterator it =
            hello_msg->AddressRangeList.begin();
            it != hello_msg->AddressRangeList.end(); it++) {
        address_list tempList = (address_list) *it;
        EV << "update address: " << tempList.ipaddr.S_addr.getIPv4().str()
                << "\n";
        if (paser_modul->isMyLocalAddress(tempList.ipaddr)) {
            EV << "it's me\n";
            continue;
        }
        if (tempList.ipaddr.S_addr == neighbor.S_addr) {
            routing_table->updateNeighborFromHELLO(tempList, hello_msg->seq,
                    ifIndex);
        } else {
            routing_table->updateRouteFromHELLO(tempList, ifIndex, neighbor);
        }
    }
    delete hello_msg;
}

void PASER_Message_Processing::handleB_ROOT(cPacket * msg, u_int32_t ifIndex) {
    PASER_UB_Root_Refresh *b_root_msg = check_and_cast<PASER_UB_Root_Refresh *>(msg);
    if (!paser_global->getWasRegistered()) {
        delete b_root_msg;
        return;
    }
    struct in_addr querying = b_root_msg->srcAddress_var;
    EV << "ROOT from IP: " << querying.S_addr.getIPv4().str() << "\n";
    //Pruefe Sequenznummer des Pakets
    if (!check_seq_nr((check_and_cast<PASER_MSG *>(msg)), querying)) {
        EV << "REPLAY SEQ\n";
        delete b_root_msg;
        return;
    }
    // Check Geo positions of the sender
    //Pruefe GeoPosition des Absenders
    if (!check_geo(b_root_msg->geoQuerying)) {
        EV << "WORMHOLE\n";
        delete b_root_msg;
        return;
    }
    EV << "iv = " << b_root_msg->initVector << "\n";
//Check Timestamf
    //pruefe Timestamp
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    if (now.tv_sec - b_root_msg->timestamp > PASER_time_diff
            || now.tv_sec - b_root_msg->timestamp < -PASER_time_diff) {
        EV << "old paket\n";
        delete b_root_msg;
        return;
    }
// Verify the message signature
    //Pruefe Signatur des Pakets
    if (!crypto_sign->checkSignB_ROOT(b_root_msg)) {
        EV << "SIGN ERROR\n";
        delete b_root_msg;
        return;
    }
    EV << "SIGN OK\n";

    //Save new ROOT and IV
    //speichere neues ROOT und IV
    PASER_Routing_Entry *rEntry = routing_table->findDest(querying);
    if (rEntry == NULL || rEntry->hopcnt != 1) {
        EV << "rEntry not found\n";
        delete b_root_msg;
        return;
    }
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(querying);
    if (nEntry == NULL) {
        EV << "nEntry not found\n";
        delete b_root_msg;
        return;
    }
    rEntry->seqnum = b_root_msg->seq;

    free(nEntry->root);
    u_int8_t *rootN = (u_int8_t *) malloc(
            (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    memcpy(rootN, b_root_msg->root, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    nEntry->root = rootN;
    nEntry->IV = b_root_msg->initVector;
//printf("e_root:0x");
//for (int n = 0; n < SHA256_DIGEST_LENGTH; n++)
//    printf("%02x", rootN[n]);
//putchar('\n');
    delete b_root_msg;
}

void PASER_Message_Processing::handleB_RESET(cPacket * msg, u_int32_t ifIndex) {
    PASER_UB_Key_Refresh *b_reset_msg = check_and_cast<PASER_UB_Key_Refresh *>(msg);
    if (!paser_global->getWasRegistered()) {
        delete b_reset_msg;
        return;
    }
    // Check the number of the key used to secure this message
    //pruefe keyNr
    if (b_reset_msg->keyNr < paser_configuration->getKeyNr()) {
        send_reset();
        delete b_reset_msg;
        return;
    }
    struct in_addr querying = b_reset_msg->srcAddress_var;
    EV << "RESET from IP: " << querying.S_addr.getIPv4().str() << "\n";
    // Check if message key number is equal to the number of the key currently in use
    //pruefe Schluesselnummer
    u_int32_t myKeyNr = paser_configuration->getKeyNr();
    if (myKeyNr >= b_reset_msg->keyNr) {
        EV << "old paket\n";
        delete b_reset_msg;
        return;
    }
    // Check if the certificate belongs to KDC
    //Pruefe ob Zertifikat ein KDC Zertifikat ist
    X509* certFromKDC = crypto_sign->extractCert(b_reset_msg->cert);
    if (!crypto_sign->isKdcCert(certFromKDC)) {
        EV << "falsches Zertifikat\n";
        X509_free(certFromKDC);
        delete b_reset_msg;
        return;
    }
    X509_free(certFromKDC);
    // Check the signature of the key number
    //Pruefe Signatur des Schluesselsnummer
    if (!crypto_sign->checkSignRESET(b_reset_msg)) {
        EV << "SIGN ERROR\n";
        delete b_reset_msg;
        return;
    }
    EV << "SIGN OK\n";
    paser_configuration->setKeyNr(b_reset_msg->keyNr);
    paser_global->resetPASER();
    GTK.len = 0;
    GTK.buf = NULL;
    // Save the new certificate and signature of RESET!!!
    //Speichere neuen Zertifikat und Signatur von RESET
    paser_configuration->setKDC_cert(b_reset_msg->cert);
    paser_configuration->setRESET_sign(b_reset_msg->sign);
//printf("sign_from__RESET_:");
//for (int n = 0; n < b_reset_msg->sign.len; n++)
//    printf("%02x", b_reset_msg->sign.buf[n]);
//putchar('\n');

    //leite RESET nachricht weiter
    send_reset();
//return;

    //Registriere sich neu
    if (paser_configuration->getIsGW()) {
        lv_block cert;
        if (!crypto_sign->getCert(&cert)) {
            EV << "cert ERROR\n";
            delete b_reset_msg;
            opp_error("RESET FEHLER! KEIN ZERTIFIKAT!");
            return;
        }
        paser_global->generateGwSearchNonce();
        sendKDCRequest(paser_configuration->getNetDevice()[0].ipaddr,
                paser_configuration->getNetDevice()[0].ipaddr, cert,
                paser_global->getLastGwSearchNonce());
//return;
        free(cert.buf);

        PASER_Timer_Message *timeMessage = new PASER_Timer_Message();
        struct timeval now;
        paser_modul->MYgettimeofday(&now, NULL);
        timeMessage->handler = KDC_REQUEST;
//        timeMessage->timeout = timeval_add(now, paser_modul->par("KDCWaitTime").doubleValue()/(double)1000);
        timeMessage->timeout = timeval_add(now, PASER_KDC_REQUEST_TIME);
        timeMessage->destAddr = paser_configuration->getAddressOfKDC();
        timer_queue->timer_add(timeMessage);
    } else {
        route_findung->tryToRegister();
    }

    delete b_reset_msg;
}

PASER_UB_RREQ * PASER_Message_Processing::send_ub_rreq(struct in_addr src_addr,
        struct in_addr dest_addr, int isDestGW) {
    PASER_UB_RREQ *message = new PASER_UB_RREQ(src_addr, dest_addr,
            paser_global->getSeqNr());
    message->keyNr = paser_configuration->getKeyNr();
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    message->timestamp = now.tv_sec;
    message->seqForw = paser_global->getSeqNr();
    message->GFlag = isDestGW;
    if (isDestGW) {
        //Get nonce
        message->nonce = paser_global->getLastGwSearchNonce();
    }
    address_list myAddrList;
    myAddrList.ipaddr = src_addr;
    myAddrList.range = paser_global->getAddL();
    message->AddressRangeList.push_back(myAddrList);
//    message->AddressRangeList.assign( AddL.begin(), AddL.end() );
//    message->routeFromQueryingToForwarding.push_front( src_addr );
    message->metricBetweenQueryingAndForw = 0;

    if (isDestGW) {
        lv_block cert;
        if (!crypto_sign->getCert(&cert)) {
            EV << "cert ERROR\n";
            return NULL;
        }
        message->cert.buf = cert.buf;
        message->cert.len = cert.len;
    }
    lv_block certForw;
    if (!crypto_sign->getCert(&certForw)) {
        EV << "certForw ERROR\n";
        return NULL;
    }
    message->certForw.buf = certForw.buf;
    message->certForw.len = certForw.len;
    message->root = root->root_get_root();
    message->initVector = root->root_get_iv();

//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
    geo_pos myGeo = paser_global->getGeoPosition();

    message->geoQuerying.lat = myGeo.lat;
    message->geoQuerying.lon = myGeo.lon;
    message->geoForwarding.lat = myGeo.lat;
    message->geoForwarding.lon = myGeo.lon;

    crypto_sign->signUBRREQ(message);

    struct in_addr bcast_addr;
    bcast_addr.S_addr.set(IPv4Address::ALLONES_ADDRESS);
    PASER_UB_RREQ *messageToSend = new PASER_UB_RREQ(*message);
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("sing_rsa_delay");
    paser_modul->send_message(
            messageToSend,
            bcast_addr,
            paser_modul->MYgetWlanInterfaceIndexByAddress(
                    messageToSend->srcAddress_var.S_addr));
    paser_global->resetHelloTimer();
    return message;
}

PASER_UU_RREP * PASER_Message_Processing::send_uu_rrep(struct in_addr src_addr,
        struct in_addr forw_addr, struct in_addr dest_addr, int isDestGW,
        X509 *cert, kdc_block kdcData) {
    EV << "src: " << src_addr.S_addr.getIPv4().str() << " dest: "
            << dest_addr.S_addr.getIPv4().str() << "\n";
    PASER_Routing_Entry * routeEntry = routing_table->findDest(src_addr);
    if (routeEntry == NULL || !routeEntry->isValid) {
        EV << "Route to " << src_addr.S_addr.getIPv4().str()
                << " not Found. ERROR!\n";
        return NULL;
    }
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(forw_addr);

    PASER_UU_RREP *message = new PASER_UU_RREP(src_addr, dest_addr,
            paser_global->getSeqNr());
    message->keyNr = paser_configuration->getKeyNr();
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    message->timestamp = now.tv_sec;
    message->GFlag = isDestGW;

    in_addr WlanAddrStruct;
    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
    address_list myAddrList;
    myAddrList.ipaddr = WlanAddrStruct;
    myAddrList.range = paser_global->getAddL();
    message->AddressRangeList.push_back(myAddrList);
//    message->AddressRangeList.assign( AddL.begin(), AddL.end() );
//    message->routeFromQueryingToForwarding.push_front( WlanAddrStruct/*myAddrStruct*/ );
    message->metricBetweenQueryingAndForw = routeEntry->hopcnt;
    message->metricBetweenDestAndForw = 0;

    lv_block certForw;
    if (!crypto_sign->getCert(&certForw)) {
        EV << "certForw ERROR\n";
        return NULL;
    }
    message->certForw.buf = certForw.buf;
    message->certForw.len = certForw.len;
    message->root = root->root_get_root();
    message->initVector = root->root_get_iv();

//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
    geo_pos myGeo = paser_global->getGeoPosition();

    EV << "posX: " << myGeo.lat << ", posY: " << myGeo.lon << "\n";
    message->geoDestination.lat = myGeo.lat;
    message->geoDestination.lon = myGeo.lon;
    message->geoForwarding.lat = myGeo.lat;
    message->geoForwarding.lon = myGeo.lon;

    if (isDestGW) {
        message->kdc_data.GTK.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.GTK.len));
        memcpy(message->kdc_data.GTK.buf, kdcData.GTK.buf,
                (sizeof(u_int8_t) * kdcData.GTK.len));
        message->kdc_data.GTK.len = kdcData.GTK.len;

        message->kdc_data.nonce = kdcData.nonce;

        message->kdc_data.CRL.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.CRL.len));
        memcpy(message->kdc_data.CRL.buf, kdcData.CRL.buf,
                (sizeof(u_int8_t) * kdcData.CRL.len));
        message->kdc_data.CRL.len = kdcData.CRL.len;

        message->kdc_data.cert_kdc.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.cert_kdc.len));
        memcpy(message->kdc_data.cert_kdc.buf, kdcData.cert_kdc.buf,
                (sizeof(u_int8_t) * kdcData.cert_kdc.len));
        message->kdc_data.cert_kdc.len = kdcData.cert_kdc.len;

        message->kdc_data.sign.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.sign.len));
        memcpy(message->kdc_data.sign.buf, kdcData.sign.buf,
                (sizeof(u_int8_t) * kdcData.sign.len));
        message->kdc_data.sign.len = kdcData.sign.len;

        message->kdc_data.key_nr = kdcData.key_nr;

        message->kdc_data.sign_key.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.sign_key.len));
        memcpy(message->kdc_data.sign_key.buf, kdcData.sign_key.buf,
                (sizeof(u_int8_t) * kdcData.sign_key.len));
        message->kdc_data.sign_key.len = kdcData.sign_key.len;
    } else {
        message->kdc_data.GTK.buf = NULL;
        message->kdc_data.GTK.len = 0;
        message->kdc_data.nonce = 0;
        message->kdc_data.CRL.buf = NULL;
        message->kdc_data.CRL.len = 0;
        message->kdc_data.cert_kdc.buf = NULL;
        message->kdc_data.cert_kdc.len = 0;
        message->kdc_data.sign.buf = NULL;
        message->kdc_data.sign.len = 0;
        message->kdc_data.key_nr = 0;
        message->kdc_data.sign_key.buf = NULL;
        message->kdc_data.sign_key.len = 0;
    }

    crypto_sign->signUURREP(message);
    PASER_UU_RREP *messageToSend = new PASER_UU_RREP(*message);
    EV << "send to: " << forw_addr.S_addr.getIPv4().str() << "\n";
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("sing_rsa_delay");
    paser_modul->send_message(messageToSend, forw_addr, nEntry->ifIndex);
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    return message;
}

PASER_TU_RREP * PASER_Message_Processing::send_tu_rrep(struct in_addr src_addr,
        struct in_addr forw_addr, struct in_addr dest_addr, int isDestGW,
        X509 *cert, kdc_block kdcData) {
    PASER_Routing_Entry * routeEntry = routing_table->findDest(src_addr);
    if (routeEntry == NULL || !routeEntry->isValid) {
        EV << "Route to " << src_addr.S_addr.getIPv4().str()
                << " not Found. ERROR!\n";
        return NULL;
    }
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(forw_addr);

    PASER_TU_RREP *message = new PASER_TU_RREP(src_addr, dest_addr,
            paser_global->getSeqNr());
    message->keyNr = paser_configuration->getKeyNr();
    message->GFlag = isDestGW;

    in_addr WlanAddrStruct;
    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
    address_list myAddrList;
    myAddrList.ipaddr = WlanAddrStruct;
    myAddrList.range = paser_global->getAddL();
    message->AddressRangeList.push_back(myAddrList);
//    message->AddressRangeList.assign( AddL.begin(), AddL.end() );
//    message->routeFromQueryingToForwarding.push_front( WlanAddrStruct/*myAddrStruct*/ );
    message->metricBetweenQueryingAndForw = routeEntry->hopcnt;
    message->metricBetweenDestAndForw = 0;

    if (isDestGW) {
        message->kdc_data.GTK.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.GTK.len));
        memcpy(message->kdc_data.GTK.buf, kdcData.GTK.buf,
                (sizeof(u_int8_t) * kdcData.GTK.len));
        message->kdc_data.GTK.len = kdcData.GTK.len;

        message->kdc_data.nonce = kdcData.nonce;

        message->kdc_data.CRL.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.CRL.len));
        memcpy(message->kdc_data.CRL.buf, kdcData.CRL.buf,
                (sizeof(u_int8_t) * kdcData.CRL.len));
        message->kdc_data.CRL.len = kdcData.CRL.len;

        message->kdc_data.cert_kdc.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.cert_kdc.len));
        memcpy(message->kdc_data.cert_kdc.buf, kdcData.cert_kdc.buf,
                (sizeof(u_int8_t) * kdcData.cert_kdc.len));
        message->kdc_data.cert_kdc.len = kdcData.cert_kdc.len;

        message->kdc_data.sign.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.sign.len));
        memcpy(message->kdc_data.sign.buf, kdcData.sign.buf,
                (sizeof(u_int8_t) * kdcData.sign.len));
        message->kdc_data.sign.len = kdcData.sign.len;

        message->kdc_data.key_nr = kdcData.key_nr;

        message->kdc_data.sign_key.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * kdcData.sign_key.len));
        memcpy(message->kdc_data.sign_key.buf, kdcData.sign_key.buf,
                (sizeof(u_int8_t) * kdcData.sign_key.len));
        message->kdc_data.sign_key.len = kdcData.sign_key.len;
    } else {
        message->kdc_data.GTK.buf = NULL;
        message->kdc_data.GTK.len = 0;
        message->kdc_data.nonce = 0;
        message->kdc_data.CRL.buf = NULL;
        message->kdc_data.CRL.len = 0;
        message->kdc_data.cert_kdc.buf = NULL;
        message->kdc_data.cert_kdc.len = 0;
        message->kdc_data.sign.buf = NULL;
        message->kdc_data.sign.len = 0;
        message->kdc_data.key_nr = 0;
        message->kdc_data.sign_key.buf = NULL;
        message->kdc_data.sign_key.len = 0;
    }
//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
    geo_pos myGeo = paser_global->getGeoPosition();

    message->geoDestination.lat = myGeo.lat;
    message->geoDestination.lon = myGeo.lon;
    message->geoForwarding.lat = myGeo.lat;
    message->geoForwarding.lon = myGeo.lon;

    int next_iv = 0;
    u_int8_t *secret = (u_int8_t *) malloc(
            (sizeof(u_int8_t) * PASER_SECRET_LEN));
    message->auth = root->root_get_next_secret(&next_iv, secret);
    message->secret = secret;
    EV << "next iv: " << next_iv << "\n";

    crypto_hash->computeHmacTURREP(message, GTK);

    PASER_TU_RREP *messageToSend = new PASER_TU_RREP(*message);
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("one_hash_delay");
    paser_modul->send_message(messageToSend, forw_addr, nEntry->ifIndex);
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    return message;
}

PASER_TU_RREP_ACK * PASER_Message_Processing::send_tu_rrep_ack(
        struct in_addr src_addr, struct in_addr dest_addr) {
    PASER_TU_RREP_ACK *message = new PASER_TU_RREP_ACK(src_addr, dest_addr,
            paser_global->getSeqNr());
    message->keyNr = paser_configuration->getKeyNr();
    int next_iv = 0;
    u_int8_t *secret = (u_int8_t *) malloc(
            (sizeof(u_int8_t) * PASER_SECRET_LEN));
    message->auth = root->root_get_next_secret(&next_iv, secret);
    message->secret = secret;
    EV << "next iv: " << next_iv << "\n";

    crypto_hash->computeHmacTURREPACK(message, GTK);

    PASER_TU_RREP_ACK *messageToSend = new PASER_TU_RREP_ACK(*message);
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(dest_addr);
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("one_hash_delay");
    paser_modul->send_message(messageToSend, dest_addr, nEntry->ifIndex);
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    return message;
}

void PASER_Message_Processing::send_rerr(
        std::list<unreachableBlock> unreachableList) {
    if (unreachableList.size() == 1) {
        struct timeval now;
        paser_modul->MYgettimeofday(&now, NULL);

        EV << "unreachable.size = 1\n";
        if (!paser_global->getBlacklist()->setRerrTime(
                unreachableList.front().addr, now)) {
            return;
        }
    }

    for (u_int32_t i = 0; i < paser_configuration->getNetDeviceNumber(); i++) {
        PASER_TB_RERR *messageToSend = new PASER_TB_RERR(
                paser_global->getNetDevice()[i].ipaddr,
                paser_global->getSeqNr());
        messageToSend->keyNr = paser_configuration->getKeyNr();

        std::list<unreachableBlock> tempList(unreachableList);
        EV << "seqList: \n";
        for (std::list<unreachableBlock>::iterator it = tempList.begin();
                it != tempList.end(); it++) {
            unreachableBlock temp;
            temp.addr.S_addr = ((unreachableBlock) *it).addr.S_addr;
            temp.seq = ((unreachableBlock) *it).seq;
            messageToSend->UnreachableAdressesList.push_back(temp);
            EV << "IP: " << temp.addr.S_addr.getIPv4().str() << "\t: "
                    << temp.seq << "\n";
        }

        geo_pos myGeo = paser_global->getGeoPosition();
        messageToSend->geoForwarding.lat = myGeo.lat;
        messageToSend->geoForwarding.lon = myGeo.lon;

        int next_iv = 0;
        u_int8_t *secret = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * PASER_SECRET_LEN));
        messageToSend->auth = paser_global->getRoot()->root_get_next_secret(
                &next_iv, secret);
        messageToSend->secret = secret;
        EV << "next iv: " << next_iv << "\n";

        paser_global->getCrypto_hash()->computeHmacRERR(messageToSend,
                paser_configuration->getGTK());

        struct in_addr bcast_addr;
        bcast_addr.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("one_hash_delay");
        paser_modul->send_message(
                messageToSend,
                bcast_addr,
                paser_modul->MYgetWlanInterfaceIndexByAddress(
                        messageToSend->srcAddress_var.S_addr));
        //seqNr++
        paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    }
}

void PASER_Message_Processing::send_root() {
    for (u_int32_t i = 0; i < paser_configuration->getNetDeviceNumber(); i++) {
        PASER_UB_Root_Refresh *messageToSend = new PASER_UB_Root_Refresh(
                paser_global->getNetDevice()[i].ipaddr,
                paser_global->getSeqNr());

        struct timeval now;
        paser_modul->MYgettimeofday(&now, NULL);
        messageToSend->timestamp = now.tv_sec;

        geo_pos myGeo = paser_global->getGeoPosition();
        messageToSend->geoQuerying.lat = myGeo.lat;
        messageToSend->geoQuerying.lon = myGeo.lon;

        lv_block cert;
        if (!crypto_sign->getCert(&cert)) {
            EV << "certForw ERROR\n";
            return;
        }
        messageToSend->cert.buf = cert.buf;
        messageToSend->cert.len = cert.len;

        messageToSend->root = root->root_get_root();
        messageToSend->initVector = root->root_get_iv();

        crypto_sign->signB_ROOT(messageToSend);

        struct in_addr bcast_addr;
        bcast_addr.S_addr.set(IPv4Address::ALLONES_ADDRESS);
//        int i = 0;
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("sing_rsa_delay");
//        for(i=0; i<2; i++){
//            PASER_UB_Root_Refresh *messageCopy = new PASER_UB_Root_Refresh(*messageToSend);
//            paser_modul->send_message(messageCopy, bcast_addr, paser_modul->MYgetWlanInterfaceIndexByAddress(messageCopy->srcAddress_var.S_addr));
//        }
        paser_modul->send_message(
                messageToSend,
                bcast_addr,
                paser_modul->MYgetWlanInterfaceIndexByAddress(
                        messageToSend->srcAddress_var.S_addr));
        //seqNr++
        paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    }
}

void PASER_Message_Processing::send_reset() {
    EV << "send RESET\n";
    for (u_int32_t i = 0; i < paser_configuration->getNetDeviceNumber(); i++) {
        PASER_UB_Key_Refresh *messageToSend = new PASER_UB_Key_Refresh(
                paser_global->getNetDevice()[i].ipaddr);

        messageToSend->keyNr = paser_configuration->getKeyNr();

        lv_block tempCert;
        paser_configuration->getKDCCert(&tempCert);

        messageToSend->cert.len = tempCert.len;
        messageToSend->cert.buf = tempCert.buf;

        lv_block tempSign;
//        messageToSend->sign.len = paser_configuration->getRESET_sign().len;
//        messageToSend->sign.buf = (u_int8_t *)malloc(messageToSend->sign.len);
//        memcpy(messageToSend->sign.buf, paser_configuration->getRESET_sign().buf, sizeof(messageToSend->sign.len));

        paser_configuration->getRESETSign(&tempSign);
        messageToSend->sign.len = tempSign.len;
        messageToSend->sign.buf = tempSign.buf;
//printf("sign_getRESE_TEMP:");
//for (int n = 0; n < messageToSend->sign.len; n++)
//    printf("%02x", messageToSend->sign.buf[n]);
//putchar('\n');

        struct in_addr bcast_addr;
        bcast_addr.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        for (int i = 0; i < 1; i++) {
            PASER_UB_Key_Refresh *messageCopy = new PASER_UB_Key_Refresh(*messageToSend);
            paser_modul->send_message(
                    messageCopy,
                    bcast_addr,
                    paser_modul->MYgetWlanInterfaceIndexByAddress(
                            messageCopy->srcAddress_var.S_addr));
        }
        paser_modul->send_message(
                messageToSend,
                bcast_addr,
                paser_modul->MYgetWlanInterfaceIndexByAddress(
                        messageToSend->srcAddress_var.S_addr));
    }
}

PASER_UB_RREQ * PASER_Message_Processing::forward_ub_rreq(
        PASER_UB_RREQ *oldMessage) {
    //forward UB-REEQ on all interfaces
    PASER_UB_RREQ *message = NULL;
    for (u_int32_t i = 0; i < paser_configuration->getNetDeviceNumber(); i++) {
        if (i > 0) {
            delete message;
        }
        message = new PASER_UB_RREQ(*oldMessage);
        struct timeval now;
        paser_modul->MYgettimeofday(&now, NULL);
        message->timestamp = now.tv_sec;
        message->seqForw = paser_global->getSeqNr();
        message->AddressRangeList.assign(oldMessage->AddressRangeList.begin(),
                oldMessage->AddressRangeList.end());
        in_addr WlanAddrStruct;
        WlanAddrStruct.S_addr = netDevice[i].ipaddr.S_addr;
        address_list myAddrList;
        myAddrList.ipaddr = WlanAddrStruct;
        myAddrList.range = paser_global->getAddL();
        message->AddressRangeList.push_back(myAddrList);
//        message->AddressRangeList.assign( AddL.begin(), AddL.end() );
//        message->routeFromQueryingToForwarding.push_back( WlanAddrStruct/*myAddrStruct*/ );
        message->metricBetweenQueryingAndForw =
                message->metricBetweenQueryingAndForw + 1;

        lv_block certForw;
        free(message->certForw.buf);
        if (!crypto_sign->getCert(&certForw)) {
            EV << "certForw ERROR\n";
            return NULL;
        }
        message->certForw.buf = certForw.buf;
        message->certForw.len = certForw.len;
        free(message->root);
        message->root = root->root_get_root();
        message->initVector = root->root_get_iv();

//        ChannelControl *cc = ChannelControl::get();
//        ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//        myPos = cc->getHostPosition( myHostRef );
        geo_pos myGeo = paser_global->getGeoPosition();

        message->geoForwarding.lat = myGeo.lat;
        message->geoForwarding.lon = myGeo.lon;

        crypto_sign->signUBRREQ(message);

        struct in_addr bcast_addr;
        bcast_addr.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        PASER_UB_RREQ *messageToSend = new PASER_UB_RREQ(*message);
        paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
                + (double) paser_modul->par("sing_rsa_delay");
        paser_modul->send_message(messageToSend, bcast_addr,
                netDevice[i].ifindex);
    }
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    paser_global->resetHelloTimer();
    return message;
}

PASER_TU_RREQ * PASER_Message_Processing::forward_ub_rreq_to_tu_rreq(
        PASER_UB_RREQ *oldMessage, struct in_addr nxtHop_addr,
        struct in_addr dest_addr) {
    PASER_TU_RREQ *message = new PASER_TU_RREQ(oldMessage->srcAddress_var,
            dest_addr, oldMessage->seq);
    message->keyNr = paser_configuration->getKeyNr();
    message->seqForw = paser_global->getSeqNr();
    message->GFlag = oldMessage->GFlag;
//    message->routeFromQueryingToForwarding.assign(oldMessage->routeFromQueryingToForwarding.begin(), oldMessage->routeFromQueryingToForwarding.end());
    message->AddressRangeList.assign(oldMessage->AddressRangeList.begin(),
            oldMessage->AddressRangeList.end());
    in_addr WlanAddrStruct;
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(nxtHop_addr);
    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
    address_list myAddrList;
    myAddrList.ipaddr = WlanAddrStruct;
    myAddrList.range = paser_global->getAddL();
    message->AddressRangeList.push_back(myAddrList);
//    message->routeFromQueryingToForwarding.push_back( WlanAddrStruct/*myAddrStruct*/ );
    message->metricBetweenQueryingAndForw =
            oldMessage->metricBetweenQueryingAndForw + 1;

    if (message->GFlag) {
        //Falls auf GW sesucht wird, wird nonce und Zertifikat weitergeleitet
        EV << "Gflag is set\n";
        //nonce
        message->nonce = oldMessage->nonce;
        //cert
        message->cert.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->cert.len));
        memcpy(message->cert.buf, oldMessage->cert.buf,
                (sizeof(u_int8_t) * oldMessage->cert.len));
        message->cert.len = oldMessage->cert.len;

    }

//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
    geo_pos myGeo = paser_global->getGeoPosition();

    message->geoQuerying.lat = oldMessage->geoQuerying.lat;
    message->geoQuerying.lon = oldMessage->geoQuerying.lon;
    message->geoForwarding.lat = myGeo.lat;
    message->geoForwarding.lon = myGeo.lon;
    EV << "x: " << message->geoQuerying.lat << "\ny: " << message->geoQuerying.lon
            << "\n";
    EV << "x: " << message->geoForwarding.lat << "\ny: "
            << message->geoForwarding.lon << "\n";

    u_int8_t *secret = (u_int8_t *) malloc(
            (sizeof(u_int8_t) * PASER_SECRET_LEN));
    int next_iv = 0;
    message->auth = root->root_get_next_secret(&next_iv, secret);
    message->secret = secret;
    EV << "next iv: " << next_iv << "\n";
    crypto_hash->computeHmacTURREQ(message, GTK);
    EV << "x: " << message->geoQuerying.lat << "\ny: " << message->geoQuerying.lon
            << "\n";
    EV << "x: " << message->geoForwarding.lat << "\ny: "
            << message->geoForwarding.lon << "\n";

    PASER_TU_RREQ *messageToSend = new PASER_TU_RREQ(*message);
    if (messageToSend->GFlag) {
        EV << "Gflag is set by MessageToSend\n";
    }
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("one_hash_delay");
    paser_modul->send_message(messageToSend, nxtHop_addr, nEntry->ifIndex);
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    return message;
}

PASER_UU_RREP * PASER_Message_Processing::forward_uu_rrep(PASER_UU_RREP *oldMessage,
        struct in_addr nxtHop_addr) {
    PASER_UU_RREP *message = new PASER_UU_RREP(*oldMessage);
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    message->timestamp = now.tv_sec;
    message->AddressRangeList.assign(oldMessage->AddressRangeList.begin(),
            oldMessage->AddressRangeList.end());
    in_addr WlanAddrStruct;
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(nxtHop_addr);
    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
    address_list myAddrList;
    myAddrList.ipaddr = WlanAddrStruct;
    myAddrList.range = paser_global->getAddL();
    message->AddressRangeList.push_back(myAddrList);
//    message->AddressRangeList.assign( AddL.begin(), AddL.end() );
//    message->routeFromQueryingToForwarding.push_back( WlanAddrStruct/*myAddrStruct*/ );
    message->metricBetweenDestAndForw = message->metricBetweenDestAndForw + 1;

    free(message->root);
    message->root = root->root_get_root();
    message->initVector = root->root_get_iv();

//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
    geo_pos myGeo = paser_global->getGeoPosition();

    message->geoForwarding.lat = myGeo.lat;
    message->geoForwarding.lon = myGeo.lon;

    if (message->certForw.len > 0) {
        message->certForw.len = 0;
        free(message->certForw.buf);
    }
    lv_block certForw;
    if (!crypto_sign->getCert(&certForw)) {
        EV << "certForw ERROR\n";
        delete message;
        return NULL;
    }
    message->certForw.buf = certForw.buf;
    message->certForw.len = certForw.len;

    crypto_sign->signUURREP(message);
    PASER_UU_RREP *messageToSend = new PASER_UU_RREP(*message);
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("sing_rsa_delay");
    paser_modul->send_message(messageToSend, nxtHop_addr, nEntry->ifIndex);
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    return message;
}

PASER_TU_RREP * PASER_Message_Processing::forward_uu_rrep_to_tu_rrep(
        PASER_UU_RREP *oldMessage, struct in_addr nxtHop_addr) {
    PASER_TU_RREP *message = new PASER_TU_RREP(oldMessage->srcAddress_var,
            oldMessage->destAddress_var, oldMessage->seq);
    message->keyNr = paser_configuration->getKeyNr();
    message->GFlag = oldMessage->GFlag;
    message->AddressRangeList.assign(oldMessage->AddressRangeList.begin(),
            oldMessage->AddressRangeList.end());
//    message->AddressRangeList.assign( AddL.begin(), AddL.end() );
//    message->routeFromQueryingToForwarding.assign(oldMessage->routeFromQueryingToForwarding.begin(), oldMessage->routeFromQueryingToForwarding.end());
    message->AddressRangeList.assign(oldMessage->AddressRangeList.begin(),
            oldMessage->AddressRangeList.end());
    in_addr WlanAddrStruct;
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(nxtHop_addr);
    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
    address_list myAddrList;
    myAddrList.ipaddr = WlanAddrStruct;
    myAddrList.range = paser_global->getAddL();
    message->AddressRangeList.push_back(myAddrList);
//    message->routeFromQueryingToForwarding.push_back( WlanAddrStruct/*myAddrStruct*/ );
    message->metricBetweenQueryingAndForw =
            oldMessage->metricBetweenQueryingAndForw;
    message->metricBetweenDestAndForw = oldMessage->metricBetweenDestAndForw + 1;

    message->geoDestination.lat = oldMessage->geoDestination.lat;
    message->geoDestination.lon = oldMessage->geoDestination.lon;

//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
    geo_pos myGeo = paser_global->getGeoPosition();

    message->geoForwarding.lat = myGeo.lat;
    message->geoForwarding.lon = myGeo.lon;

    if (oldMessage->GFlag) {
        message->kdc_data.GTK.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.GTK.len));
        memcpy(message->kdc_data.GTK.buf, oldMessage->kdc_data.GTK.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.GTK.len));
        message->kdc_data.GTK.len = oldMessage->kdc_data.GTK.len;

        message->kdc_data.nonce = oldMessage->kdc_data.nonce;

        message->kdc_data.CRL.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.CRL.len));
        memcpy(message->kdc_data.CRL.buf, oldMessage->kdc_data.CRL.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.CRL.len));
        message->kdc_data.CRL.len = oldMessage->kdc_data.CRL.len;

        message->kdc_data.cert_kdc.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.cert_kdc.len));
        memcpy(message->kdc_data.cert_kdc.buf, oldMessage->kdc_data.cert_kdc.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.cert_kdc.len));
        message->kdc_data.cert_kdc.len = oldMessage->kdc_data.cert_kdc.len;

        message->kdc_data.sign.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.sign.len));
        memcpy(message->kdc_data.sign.buf, oldMessage->kdc_data.sign.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.sign.len));
        message->kdc_data.sign.len = oldMessage->kdc_data.sign.len;

        message->kdc_data.key_nr = oldMessage->kdc_data.key_nr;

        message->kdc_data.sign_key.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.sign_key.len));
        memcpy(message->kdc_data.sign_key.buf, oldMessage->kdc_data.sign_key.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.sign_key.len));
        message->kdc_data.sign_key.len = oldMessage->kdc_data.sign_key.len;
    } else {
        message->kdc_data.GTK.buf = NULL;
        message->kdc_data.GTK.len = 0;
        message->kdc_data.nonce = 0;
        message->kdc_data.CRL.buf = NULL;
        message->kdc_data.CRL.len = 0;
        message->kdc_data.cert_kdc.buf = NULL;
        message->kdc_data.cert_kdc.len = 0;
        message->kdc_data.sign.buf = NULL;
        message->kdc_data.sign.len = 0;
        message->kdc_data.key_nr = 0;
        message->kdc_data.sign_key.buf = NULL;
        message->kdc_data.sign_key.len = 0;
    }

    u_int8_t *secret = (u_int8_t *) malloc(
            (sizeof(u_int8_t) * PASER_SECRET_LEN));
    int next_iv = 0;
    message->auth = root->root_get_next_secret(&next_iv, secret);
    message->secret = secret;
    EV << "next iv: " << next_iv << "\n";
    crypto_hash->computeHmacTURREP(message, GTK);

    PASER_TU_RREP *messageToSend = new PASER_TU_RREP(*message);
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("one_hash_delay");
    paser_modul->send_message(messageToSend, nxtHop_addr, nEntry->ifIndex);
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    return message;
}

PASER_TU_RREQ * PASER_Message_Processing::forward_tu_rreq(PASER_TU_RREQ *oldMessage,
        struct in_addr nxtHop_addr) {
    PASER_TU_RREQ *message = new PASER_TU_RREQ(*oldMessage);
    message->seqForw = paser_global->getSeqNr();
    in_addr WlanAddrStruct;
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(nxtHop_addr);
    message->AddressRangeList.assign(oldMessage->AddressRangeList.begin(),
            oldMessage->AddressRangeList.end());
    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
    address_list myAddrList;
    myAddrList.ipaddr = WlanAddrStruct;
    myAddrList.range = paser_global->getAddL();
    message->AddressRangeList.push_back(myAddrList);
//    message->routeFromQueryingToForwarding.push_back( WlanAddrStruct/*myAddrStruct*/ );
    message->metricBetweenQueryingAndForw = message->metricBetweenQueryingAndForw
            + 1;

    if (message->GFlag) {
        free(message->cert.buf);
        message->cert.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->cert.len));
        memcpy(message->cert.buf, oldMessage->cert.buf,
                (sizeof(u_int8_t) * oldMessage->cert.len));
        message->cert.len = oldMessage->cert.len;
    }
//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
    geo_pos myGeo = paser_global->getGeoPosition();

    message->geoQuerying.lat = oldMessage->geoQuerying.lat;
    message->geoQuerying.lon = oldMessage->geoQuerying.lon;
    message->geoForwarding.lat = myGeo.lat;
    message->geoForwarding.lon = myGeo.lon;

    free(message->secret);
    for (std::list<u_int8_t *>::iterator it = message->auth.begin();
            it != message->auth.end(); it++) {
        u_int8_t *data = (u_int8_t *) *it;
        free(data);
    }
    message->auth.clear();
    free(message->hash);

    u_int8_t *secret = (u_int8_t *) malloc(
            (sizeof(u_int8_t) * PASER_SECRET_LEN));
    int next_iv = 0;
    message->auth = root->root_get_next_secret(&next_iv, secret);
    message->secret = secret;
    EV << "next iv: " << next_iv << "\n";
    crypto_hash->computeHmacTURREQ(message, GTK);

    PASER_TU_RREQ *messageToSend = new PASER_TU_RREQ(*message);
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("one_hash_delay");
    paser_modul->send_message(messageToSend, nxtHop_addr, nEntry->ifIndex);
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    return message;
}

PASER_UU_RREP * PASER_Message_Processing::forward_tu_rrep_to_uu_rrep(
        PASER_TU_RREP *oldMessage, struct in_addr nxtHop_addr) {

    PASER_UU_RREP *message = new PASER_UU_RREP(oldMessage->srcAddress_var,
            oldMessage->destAddress_var, oldMessage->seq);
    message->keyNr = paser_configuration->getKeyNr();
//    message->seqForw = seqNr;
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    message->timestamp = now.tv_sec;
    message->GFlag = oldMessage->GFlag;
    message->AddressRangeList.assign(oldMessage->AddressRangeList.begin(),
            oldMessage->AddressRangeList.end());
//    message->AddressRangeList.assign( AddL.begin(), AddL.end() );
//    message->routeFromQueryingToForwarding.assign( oldMessage->routeFromQueryingToForwarding.begin(), oldMessage->routeFromQueryingToForwarding.end() );
    message->AddressRangeList.assign(oldMessage->AddressRangeList.begin(),
            oldMessage->AddressRangeList.end());
    in_addr WlanAddrStruct;
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(nxtHop_addr);
    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
    address_list myAddrList;
    myAddrList.ipaddr = WlanAddrStruct;
    myAddrList.range = paser_global->getAddL();
    message->AddressRangeList.push_back(myAddrList);
//    message->routeFromQueryingToForwarding.push_back( WlanAddrStruct/*myAddrStruct*/ );
    message->metricBetweenQueryingAndForw =
            oldMessage->metricBetweenQueryingAndForw;
    message->metricBetweenDestAndForw = oldMessage->metricBetweenDestAndForw + 1;

    lv_block certForw;
    if (!crypto_sign->getCert(&certForw)) {
        EV << "certForw ERROR\n";
        return NULL;
    }
    message->certForw.buf = certForw.buf;
    message->certForw.len = certForw.len;
    message->root = root->root_get_root();
    message->initVector = root->root_get_iv();

//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
    geo_pos myGeo = paser_global->getGeoPosition();

    message->geoDestination.lat = oldMessage->geoDestination.lat;
    message->geoDestination.lon = oldMessage->geoDestination.lon;
    message->geoForwarding.lat = myGeo.lat;
    message->geoForwarding.lon = myGeo.lon;

    if (oldMessage->GFlag) {
        message->kdc_data.GTK.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.GTK.len));
        memcpy(message->kdc_data.GTK.buf, oldMessage->kdc_data.GTK.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.GTK.len));
        message->kdc_data.GTK.len = oldMessage->kdc_data.GTK.len;

        message->kdc_data.nonce = oldMessage->kdc_data.nonce;

        message->kdc_data.CRL.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.CRL.len));
        memcpy(message->kdc_data.CRL.buf, oldMessage->kdc_data.CRL.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.CRL.len));
        message->kdc_data.CRL.len = oldMessage->kdc_data.CRL.len;

        message->kdc_data.cert_kdc.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.cert_kdc.len));
        memcpy(message->kdc_data.cert_kdc.buf, oldMessage->kdc_data.cert_kdc.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.cert_kdc.len));
        message->kdc_data.cert_kdc.len = oldMessage->kdc_data.cert_kdc.len;

        message->kdc_data.sign.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.sign.len));
        memcpy(message->kdc_data.sign.buf, oldMessage->kdc_data.sign.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.sign.len));
        message->kdc_data.sign.len = oldMessage->kdc_data.sign.len;

        message->kdc_data.key_nr = oldMessage->kdc_data.key_nr;

        message->kdc_data.sign_key.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * oldMessage->kdc_data.sign_key.len));
        memcpy(message->kdc_data.sign_key.buf, oldMessage->kdc_data.sign_key.buf,
                (sizeof(u_int8_t) * oldMessage->kdc_data.sign_key.len));
        message->kdc_data.sign_key.len = oldMessage->kdc_data.sign_key.len;
    } else {
        message->kdc_data.GTK.buf = NULL;
        message->kdc_data.GTK.len = 0;
        message->kdc_data.nonce = 0;
        message->kdc_data.CRL.buf = NULL;
        message->kdc_data.CRL.len = 0;
        message->kdc_data.cert_kdc.buf = NULL;
        message->kdc_data.cert_kdc.len = 0;
        message->kdc_data.sign.buf = NULL;
        message->kdc_data.sign.len = 0;
        message->kdc_data.key_nr = 0;
        message->kdc_data.sign_key.buf = NULL;
        message->kdc_data.sign_key.len = 0;
    }

    crypto_sign->signUURREP(message);
    PASER_UU_RREP *messageToSend = new PASER_UU_RREP(*message);
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("sing_rsa_delay");
    paser_modul->send_message(messageToSend, nxtHop_addr, nEntry->ifIndex);
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    return message;
}

PASER_TU_RREP * PASER_Message_Processing::forward_tu_rrep(PASER_TU_RREP *oldMessage,
        struct in_addr nxtHop_addr) {
    PASER_TU_RREP *message = new PASER_TU_RREP(*oldMessage);
//    message->seqForw = seqNr;
    message->AddressRangeList.assign(oldMessage->AddressRangeList.begin(),
            oldMessage->AddressRangeList.end());
//    message->AddressRangeList.assign( AddL.begin(), AddL.end() );
    in_addr WlanAddrStruct;
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(nxtHop_addr);
    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
    address_list myAddrList;
    myAddrList.ipaddr = WlanAddrStruct;
    myAddrList.range = paser_global->getAddL();
    message->AddressRangeList.push_back(myAddrList);
//    message->routeFromQueryingToForwarding.push_back( WlanAddrStruct/*myAddrStruct*/ );
//    message->metricBetweenQueryingAndForw = message->metricBetweenQueryingAndForw;
    message->metricBetweenDestAndForw = message->metricBetweenDestAndForw + 1;

//    ChannelControl *cc = ChannelControl::get();
//    ChannelControl::HostRef myHostRef = cc->lookupHost(getParentModule());
//    myPos = cc->getHostPosition( myHostRef );
    geo_pos myGeo = paser_global->getGeoPosition();

    message->geoForwarding.lat = myGeo.lat;
    message->geoForwarding.lon = myGeo.lon;

    for (std::list<u_int8_t *>::iterator it = message->auth.begin();
            it != message->auth.end(); it++) {
        u_int8_t *temp = (u_int8_t *) *it;
        free(temp);
    }
    free(message->secret);
    message->auth.clear();
    free(message->hash);

    u_int8_t *secret = (u_int8_t *) malloc(
            (sizeof(u_int8_t) * PASER_SECRET_LEN));
    int next_iv = 0;
    message->auth = root->root_get_next_secret(&next_iv, secret);
    message->secret = secret;
    EV << "next iv: " << next_iv << "\n";
    crypto_hash->computeHmacTURREP(message, GTK);

    PASER_TU_RREP *messageToSend = new PASER_TU_RREP(*message);
    paser_modul->paketProcessingDelay = paser_modul->paketProcessingDelay
            + (double) paser_modul->par("one_hash_delay");
    paser_modul->send_message(messageToSend, nxtHop_addr, nEntry->ifIndex);
    paser_global->setSeqNr(paser_global->getSeqNr() + 1);
    return message;
}

void PASER_Message_Processing::deleteRouteRequestTimeout(
        struct in_addr dest_addr) {
    message_rreq_entry *rreq = rreq_list->pending_find(dest_addr);
    if (rreq) {
        rreq_list->pending_remove(rreq);
        PASER_Timer_Message *timeout = rreq->tPack;
        timer_queue->timer_remove(timeout);
        delete timeout;
        delete rreq;
    }
}

void PASER_Message_Processing::deleteRouteRequestTimeoutForAddList(
        std::list<address_list> AddList) {
    message_rreq_entry *rreq;
    for (std::list<address_list>::iterator it = AddList.begin();
            it != AddList.end(); it++) {
        address_list tempList = (address_list) *it;
        for (std::list<address_range>::iterator it2 = tempList.range.begin();
                it2 != tempList.range.end(); it2++) {
            address_range tempRange = (address_range) *it2;
            rreq = rreq_list->pending_find_addr_with_mask(tempRange.ipaddr,
                    tempRange.mask);
            if (rreq) {
                rreq_list->pending_remove(rreq);
                PASER_Timer_Message *timeout = rreq->tPack;
                timer_queue->timer_remove(timeout);
                delete timeout;
                delete rreq;
            }
        }
    }
}

void PASER_Message_Processing::sendKDCRequest(struct in_addr nodeAddr,
        struct in_addr nextHop, lv_block cert, int nonce) {
    if (paser_configuration->getNetEthDeviceNumber() < 1) {
        EV << "Cann't send KDC Request. NetEthDevice not Found!\n";
        return;
    }
//    if(simTime().dbl() > 30){
//        return;
//    }
    crl_message *message = new crl_message();
    message->setSrc(nodeAddr);
    message->setGwAddr(paser_configuration->getNetEthDevice()[0].ipaddr);
    message->setNextHopAddr(nextHop);
    message->setCert_len(cert.len);
    message->setCert_arrayArraySize(cert.len);
    for (u_int32_t i = 0; i < cert.len; i++) {
        message->setCert_array(i, cert.buf[i]);
    }
    message->setKdc_nonce(nonce);
    paser_modul->send_message(message, paser_configuration->getAddressOfKDC(), 1);
    return;
}

void PASER_Message_Processing::checkKDCReply(cPacket * msg) {
    crl_message *crlMssage = check_and_cast<crl_message *>(msg);
    EV << "checkKDCReply\n";
    //convert MessageData to KdcData
    kdc_block kdcData;
    kdcData.GTK.len = crlMssage->getKdc_gtk_len();
    kdcData.GTK.buf = (u_int8_t*) malloc(kdcData.GTK.len);
    for (u_int32_t i = 0; i < kdcData.GTK.len; i++) {
        kdcData.GTK.buf[i] = crlMssage->getKdc_gtk_array(i);
    }
    kdcData.nonce = crlMssage->getKdc_nonce();
    kdcData.CRL.len = crlMssage->getKdc_crl_len();
    kdcData.CRL.buf = (u_int8_t*) malloc(kdcData.CRL.len);
    for (u_int32_t i = 0; i < kdcData.CRL.len; i++) {
        kdcData.CRL.buf[i] = crlMssage->getKdc_crl_array(i);
    }
    kdcData.cert_kdc.len = crlMssage->getKdc_cert_len();
    kdcData.cert_kdc.buf = (u_int8_t*) malloc(kdcData.cert_kdc.len);
    for (u_int32_t i = 0; i < kdcData.cert_kdc.len; i++) {
        kdcData.cert_kdc.buf[i] = crlMssage->getKdc_cert_array(i);
    }
    kdcData.sign.len = crlMssage->getKdc_sign_len();
    kdcData.sign.buf = (u_int8_t*) malloc(kdcData.sign.len);
    for (u_int32_t i = 0; i < kdcData.sign.len; i++) {
        kdcData.sign.buf[i] = crlMssage->getKdc_sign_array(i);
    }
    kdcData.key_nr = crlMssage->getKdc_key_nr();
    kdcData.sign_key.len = crlMssage->getKdc_sign_key_len();
    kdcData.sign_key.buf = (u_int8_t*) malloc(kdcData.sign_key.len);
    for (u_int32_t i = 0; i < kdcData.sign_key.len; i++) {
        kdcData.sign_key.buf[i] = crlMssage->getKdc_sign_key_array(i);
    }

    if (paser_modul->isMyLocalAddress(crlMssage->getSrc())) {
        if (crypto_sign->checkSignKDC(kdcData) == 1
                && kdcData.nonce == paser_global->getLastGwSearchNonce()) {
            //KDC OK !!!
            EV << "checkSignKDC OK.";
           // EV <<  kdcData.nonce << paser_global->getLastGwSearchNonce()<<"\n" ;
            paser_global->getCrypto_sign()->checkAllCertInRoutingTable(
                    routing_table, neighbor_table, timer_queue);
            paser_global->setIsRegistered(true);
            paser_global->setWasRegistered(true);
            lv_block gtk;
            gtk.len = 0;
            gtk.buf = NULL;
            crypto_sign->rsa_dencrypt(kdcData.GTK, &gtk);
            paser_modul->paketProcessingDelay =
                    paser_modul->paketProcessingDelay
                            + (double) paser_modul->par("deryption_rsa_delay");
            paser_configuration->setGTK(gtk);
            paser_configuration->setKeyNr(kdcData.key_nr);
            if (gtk.len > 0) {
                free(gtk.buf);
            }
            GTK.buf = paser_configuration->getGTK().buf;
            GTK.len = paser_configuration->getGTK().len;
            free(kdcData.GTK.buf);
            free(kdcData.CRL.buf);
            free(kdcData.cert_kdc.buf);
            free(kdcData.sign.buf);
            free(kdcData.sign_key.buf);
            PASER_Timer_Message *timePack = new PASER_Timer_Message();
            timePack->handler = KDC_REQUEST;
            timer_queue->timer_remove(timePack);
            delete timePack;
            delete crlMssage;
            return;
        } else {
            // Error, send again a request to KDC
            //Fehler, sende KDC request nochmal
            EV << "SignError!\n";
//            lv_block cert;
//            if( !crypto_sign->getCert(&cert) ){
            EV << "cert ERROR\n";
            free(kdcData.GTK.buf);
            free(kdcData.CRL.buf);
            free(kdcData.cert_kdc.buf);
            free(kdcData.sign.buf);
            free(kdcData.sign_key.buf);
            delete crlMssage;
            return;
//            }
//            sendKDCRequest(paser_configuration->getNetDevice()[0].ipaddr, cert, paser_global->getLastGwSearchNonce());
//            free(cert.buf);
        }
    } else {
        // Send reply
        EV << "send RREP\n";
        struct in_addr nextHopAddr = crlMssage->getNextHopAddr();
        struct in_addr srcAddr = crlMssage->getSrc();
//        PASER_Routing_Entry *rEntry = routing_table->findDest(srcAddr);
//        if(rEntry == NULL){
//            free(kdcData.GTK.buf);
//            free(kdcData.CRL.buf);
//            free(kdcData.cert_kdc.buf);
//            free(kdcData.sign.buf);
//            free(kdcData.sign_key.buf);
//            delete crlMssage;
//            return;
//        }
//        PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(rEntry->nxthop_addr);
        PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(nextHopAddr);
        if (nEntry == NULL) {
            free(kdcData.GTK.buf);
            free(kdcData.CRL.buf);
            free(kdcData.cert_kdc.buf);
            free(kdcData.sign.buf);
            free(kdcData.sign_key.buf);
            delete crlMssage;
            return;
        }

        struct in_addr WlanAddrStruct;
        int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
        WlanAddrStruct.S_addr = netDevice[ifId].ipaddr.S_addr;
        if (!nEntry->neighFlag) {
            PASER_UU_RREP *message = send_uu_rrep(srcAddr, nEntry->neighbor_addr,
                    WlanAddrStruct, true, NULL, kdcData);
            if (message == NULL) {
                free(kdcData.GTK.buf);
                free(kdcData.CRL.buf);
                free(kdcData.cert_kdc.buf);
                free(kdcData.sign.buf);
                free(kdcData.sign_key.buf);
                delete crlMssage;
                return;
            }
            message_rreq_entry *rrep = rrep_list->pending_find(
                    nEntry->neighbor_addr);
            if (rrep) {
                timer_queue->timer_remove(rrep->tPack);
                delete rrep->tPack;
            } else {
                rrep = rrep_list->pending_add(nEntry->neighbor_addr);
            }
            rrep->tries = 0;
            struct timeval now;
            paser_modul->MYgettimeofday(&now, NULL);
            PASER_Timer_Message *tPack = new PASER_Timer_Message();
            tPack->data = (void *) message;
            tPack->destAddr.S_addr = nEntry->neighbor_addr.S_addr;
            tPack->handler = TU_RREP_ACK_TIMEOUT;
            tPack->timeout = timeval_add(now, PASER_UU_RREP_WAIT_TIME);

            EV << "now: " << now.tv_sec << "\ntimeout: "
                    << tPack->timeout.tv_sec << "\n";
            timer_queue->timer_add(tPack);
            rrep->tPack = tPack;
        } else {
            PASER_TU_RREP *message = send_tu_rrep(srcAddr, nEntry->neighbor_addr,
                    WlanAddrStruct, true, NULL, kdcData);
            if (message == NULL) {
                free(kdcData.GTK.buf);
                free(kdcData.CRL.buf);
                free(kdcData.cert_kdc.buf);
                free(kdcData.sign.buf);
                free(kdcData.sign_key.buf);
                delete crlMssage;
                return;
            }
            delete message;
        }

    }
    free(kdcData.GTK.buf);
    free(kdcData.CRL.buf);
    free(kdcData.cert_kdc.buf);
    free(kdcData.sign.buf);
    free(kdcData.sign_key.buf);
    delete crlMssage;
}
#endif
