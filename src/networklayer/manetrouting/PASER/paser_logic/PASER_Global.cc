/*
 *\class       PASER_Global
 *@brief       Class provides pointers to all PASER Modules.
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
#include "PASER_Global.h"
#include "ChannelAccess.h"

PASER_Global::PASER_Global(PASER_Configurations *pConfig, PASER_Socket *pModul) {

    paser_configuration = pConfig;
    paser_modul = pModul;

    isGW = paser_configuration->getIsGW();

    hello_message_interval = NULL;

    timer_queue = new PASER_Timer_Queue();
    neighbor_table = new PASER_Neighbor_Table(timer_queue, pModul);
    routing_table = new PASER_Routing_Table(timer_queue, neighbor_table, pModul,
            this);
    message_queue = new PASER_Message_Queue(pModul);
    rreq_list = new PASER_RREQ_List();
    rrep_list = new PASER_RREQ_List();

    blackList = new PASER_RERR_List();

    root = new PASER_Crypto_Root(this);
    root->root_init(PASER_Crypto_Root_param);

    EV << "isGW = " << isGW << "\n";
//    isRegistered = isGW;
//    wasRegistered = isRegistered;
    isRegistered = false;
    wasRegistered = false;

    crypto_sign = new PASER_Crypto_Sign();
    crypto_sign->init(paser_configuration->getCertfile(),
            paser_configuration->getKeyfile(),
            paser_configuration->getCAfile());

    crypto_hash = new PASER_Crypto_Hash();

//    GTK.buf = NULL;
//    GTK.len = 0;

//    if(isGW){
//        GTK.buf = paser_configuration->getGTK().buf;
//        GTK.len = paser_configuration->getGTK().len;
//    }

    seqNr = 1;

    netDevice = paser_configuration->getNetDevice();
    netAddDevice = paser_configuration->getNetAddDevice();
    EV << "AddL:\n";
    for (u_int32_t i = 0; i < paser_configuration->getNetAddDeviceNumber();
            i++) {
        address_range tempRange;
        tempRange.ipaddr = netAddDevice[i].ipaddr;
        tempRange.mask = netAddDevice[i].mask;
        AddL.push_back(tempRange);
        EV << "addr: " << tempRange.ipaddr.S_addr.getIPv4().str()
                << " mask: " << tempRange.mask.S_addr.getIPv4().str()
                << "\n";

        struct in_addr emptyAddr;
        emptyAddr.S_addr.set(IPv4Address::UNSPECIFIED_ADDRESS);
        routing_table->updateKernelRoutingTable(tempRange.ipaddr, emptyAddr,
                tempRange.mask, 1, false, netAddDevice[i].ifindex);
    }

    isHello = pModul->par("isHELLO").boolValue();
    isLinkLayer = pModul->par("isLinkLayer").boolValue();

    if (isHello) {
        activateHelloMessageTimer();
    }

    lastGwSearchNonce = 0;
}

PASER_Global::~PASER_Global() {
    delete crypto_sign;
    delete crypto_hash;
    delete root;
    delete timer_queue;
    delete neighbor_table;
    delete routing_table;
    delete message_queue;
    delete rreq_list;
    delete rrep_list;
}

std::list<address_range> PASER_Global::getAddL() {
    return AddL;
}

PASER_Crypto_Hash *PASER_Global::getCrypto_hash() {
    return crypto_hash;
}

PASER_Crypto_Sign *PASER_Global::getCrypto_sign() {
    return crypto_sign;
}

//lv_block PASER_Global::getGTK()
//{
//    return GTK;
//}

bool PASER_Global::getIsGW() {
    return isGW;
}

bool PASER_Global::getIsRegistered() {
    return isRegistered;
}

bool PASER_Global::getWasRegistered() {
    return wasRegistered;
}

//u_int32_t PASER_Global::getNS_IFINDEX()
//{
//    return NS_IFINDEX;
//}

PASER_Neighbor_Table *PASER_Global::getNeighbor_table() {
    return neighbor_table;
}

network_device *PASER_Global::getNetDevice() {
    return netDevice;
}

PASER_Message_Queue *PASER_Global::getMessage_queue() {
    return message_queue;
}

PASER_Crypto_Root *PASER_Global::getRoot() {
    return root;
}

PASER_Routing_Table *PASER_Global::getRouting_table() {
    return routing_table;
}

PASER_RREQ_List *PASER_Global::getRrep_list() {
    return rrep_list;
}

PASER_RREQ_List *PASER_Global::getRreq_list() {
    return rreq_list;
}

u_int32_t PASER_Global::getSeqNr() {
    return seqNr;
}

u_int32_t PASER_Global::getLastGwSearchNonce() {
    return lastGwSearchNonce;
}

PASER_Timer_Queue *PASER_Global::getTimer_queue() {
    return timer_queue;
}

PASER_Configurations *PASER_Global::getPaser_configuration() {
    return paser_configuration;
}

PASER_Socket *PASER_Global::getPASER_modul() {
    return paser_modul;
}

PASER_RERR_List *PASER_Global::getBlacklist() {
    return blackList;
}

void PASER_Global::setPaket_processing(PASER_Message_Processing *pProcessing) {
    paket_processing = pProcessing;
}

void PASER_Global::setRoute_findung(PASER_Route_Discovery *rFindung) {
    route_findung = rFindung;
}

void PASER_Global::setRoute_maintenance(PASER_Route_Maintenance *rMaintenance) {
    route_maintenance = rMaintenance;
}

PASER_Message_Processing *PASER_Global::getPaket_processing() {
    return paket_processing;
}

PASER_Route_Discovery *PASER_Global::getRoute_findung() {
    return route_findung;
}

PASER_Route_Maintenance *PASER_Global::getRoute_maintenance() {
    return route_maintenance;
}

void PASER_Global::setSeqNr(u_int32_t s) {
    seqNr = s;
}

void PASER_Global::setLastGwSearchNonce(u_int32_t s) {
    lastGwSearchNonce = s;
}

void PASER_Global::generateGwSearchNonce() {
    unsigned char buf[4];
    lastGwSearchNonce = 0;
    while (!RAND_bytes(buf, 4)) {
    };
    lastGwSearchNonce = (buf[0] | buf[1] <<8 | buf[2] <<16 | buf[3] <<24);
    return;

}

void PASER_Global::setIsGW(bool i) {
    isGW = i;
}

void PASER_Global::setWasRegistered(bool i) {
    wasRegistered = i;
}

void PASER_Global::setIsRegistered(bool i) {
    isRegistered = i;
}

geo_pos PASER_Global::getGeoPosition() {
    geo_pos myGeo;
    myGeo.lat = paser_modul->MY_getXPos();
    myGeo.lon = paser_modul->MY_getYPos();
    return myGeo;
}

PASER_Timer_Message* PASER_Global::getHelloMessageTimer() {
    return hello_message_interval;
}

void PASER_Global::activateHelloMessageTimer() {
    if (!isHello) {
        return;
    }
    if (hello_message_interval == NULL) {
        hello_message_interval = new PASER_Timer_Message();
        hello_message_interval->data = NULL;
        hello_message_interval->destAddr.S_addr = PASER_BROADCAST;
        hello_message_interval->handler = HELLO_SEND_TIMEOUT;
        paser_modul->MYgettimeofday(&(hello_message_interval->timeout), NULL);
        EV << "HelloTimeOut: " << hello_message_interval->timeout.tv_sec << "\n";
        hello_message_interval->timeout = timeval_add(
                hello_message_interval->timeout, PASER_TB_Hello_Interval);
        EV << "new HelloTimeOut: " << hello_message_interval->timeout.tv_sec
                << "\n";
        timer_queue->timer_add(hello_message_interval);
        timer_queue->timer_sort();
        EV << "hello timer wurde zu timerqueue zugefuegt\n";
    } else {
        EV << "hello timer wurde zu timerqueue NICHT zugefuegt\n";
    }
}

void PASER_Global::resetHelloTimer() {
    if (!isHello) {
        return;
    }
    if (hello_message_interval == NULL) {
        activateHelloMessageTimer();
        return;
    }
    paser_modul->MYgettimeofday(&(hello_message_interval->timeout), NULL);
    EV << "HelloTimeOut: " << hello_message_interval->timeout.tv_sec << "\n";
    hello_message_interval->timeout = timeval_add(hello_message_interval->timeout, 
    PASER_TB_Hello_Interval);
    EV << "new HelloTimeOut: " << hello_message_interval->timeout.tv_sec << "\n";
    timer_queue->timer_sort();
    EV << "hello timer wurde aktualisiert\n";
}

bool PASER_Global::isHelloActive() {
    return isHello;
}

bool PASER_Global::isLinkLayerActive() {
    return isLinkLayer;
}

/**
 * Reset all PASER Configuration and delete all PASER Tables
 */
void PASER_Global::resetPASER() {
    //Reset rreq_list
    for (std::map<ManetAddress, message_rreq_entry *>::iterator it =
            rreq_list->rreq_list.begin(); it != rreq_list->rreq_list.end();
            it++) {
        delete it->second;
    }
    rreq_list->rreq_list.clear();

    //Reset rrep_list
    for (std::map<ManetAddress, message_rreq_entry *>::iterator it =
            rrep_list->rreq_list.begin(); it != rrep_list->rreq_list.end();
            it++) {
        delete it->second;
    }
    rrep_list->rreq_list.clear();

    //Reset RoutinigTable
    for (std::map<ManetAddress, PASER_Routing_Entry*>::iterator it =
            routing_table->route_table.begin();
            it != routing_table->route_table.end(); it++) {
        PASER_Routing_Entry *temp = it->second;
        delete temp;
    }
    routing_table->route_table.clear();
    paser_modul->resetKernelRoutingTable();

    //Reset NeighborTable
    for (std::map<ManetAddress, PASER_Neighbor_Entry*>::iterator it =
            neighbor_table->neighbor_table_map.begin();
            it != neighbor_table->neighbor_table_map.end(); it++) {
        PASER_Neighbor_Entry *temp = it->second;
        delete temp;
    }
    neighbor_table->neighbor_table_map.clear();

    //Reset TimerQueue
    for (std::list<PASER_Timer_Message *>::iterator it =
            timer_queue->timer_queue.begin();
            it != timer_queue->timer_queue.end(); it++) {
        PASER_Timer_Message *temp = (PASER_Timer_Message*) *it;
        delete temp;
    }
    timer_queue->timer_queue.clear();

    //Reset HELLO
    hello_message_interval = NULL;
    resetHelloTimer();

    //Reset MessageQueue
    for (std::list<message_queue_entry>::iterator it =
            message_queue->message_queue_list.begin();
            it != message_queue->message_queue_list.end(); it++) {
        struct message_queue_entry temp = (message_queue_entry) *it;
        delete temp.p;
    }
    message_queue->message_queue_list.clear();

    //Clear Blacklists
    blackList->clearRerrList();

    //Delete GTK
    lv_block nullBlock;
    nullBlock.len = 0;
    nullBlock.buf = NULL;
    paser_configuration->setGTK(nullBlock);

    isRegistered = false;
    wasRegistered = false;
}

bool PASER_Global::isSeqNew(u_int32_t oldSeq, u_int32_t newSeq) {
    if (oldSeq == 0) {
        return true;
    }
    if (newSeq == 0) {
        return false;
    }
    if (newSeq > oldSeq && newSeq - oldSeq < (PASER_MAXSEQ / 2)) {
        return true;
    }
    return false;
}
#endif
