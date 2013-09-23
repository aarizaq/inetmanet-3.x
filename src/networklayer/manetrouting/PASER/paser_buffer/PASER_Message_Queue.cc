/*
 *class       PASER_Message_Queue
 *brief       Class is a buffer of all data messages that must be forwarded to an unknown destination
 *
 *Authors     Eugen.Paul | Mohamad.Sbeiti \@paser.info
 *
 *copyright   (C) 2012 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
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
#include "PASER_Message_Queue.h"
#include "ManetAddress.h"
#include "compatibility.h"
PASER_Message_Queue::~PASER_Message_Queue() {
    for (std::list<message_queue_entry>::iterator it = message_queue_list.begin();
            it != message_queue_list.end(); it++) {
        struct message_queue_entry temp = (struct message_queue_entry) *it;
        delete temp.p;
    }
    message_queue_list.clear();
}

PASER_Message_Queue::PASER_Message_Queue(PASER_Socket *pModul) {
    paser_modul = pModul;
}

void PASER_Message_Queue::message_add(cPacket * p, struct in_addr dest_addr) {
    struct message_queue_entry temp;
    temp.dest_addr = dest_addr;
    temp.p = p;

    message_queue_list.push_back(temp);
}

void PASER_Message_Queue::getAllPaketsTo(struct in_addr dest_addr,
        std::list<message_queue_entry> *datagrams) {
    bool tryAgain = true;
    while (tryAgain) {
        tryAgain = false;
        for (std::list<message_queue_entry>::iterator it =
                message_queue_list.begin(); it != message_queue_list.end();
                it++) {
            struct message_queue_entry temp = (struct message_queue_entry) *it;
            if (temp.dest_addr.S_addr == dest_addr.S_addr) {
                message_queue_list.erase(it);
                datagrams->push_back(temp);
                tryAgain = true;
                break;
            }
        }
    }
}

void PASER_Message_Queue::getAllPaketsToAddWithMask(struct in_addr dest_addr,
        struct in_addr mask_addr, std::list<message_queue_entry> *datagrams) {
    bool tryAgain = true;
    while (tryAgain) {
        tryAgain = false;
        for (std::list<message_queue_entry>::iterator it =
                message_queue_list.begin(); it != message_queue_list.end();
                it++) {
            struct message_queue_entry temp = (struct message_queue_entry) *it;
            if (IPv4Address::maskedAddrAreEqual( temp.dest_addr.S_addr.getIPv4(), dest_addr.S_addr.getIPv4(),   mask_addr.S_addr.getIPv4())) {
                message_queue_list.erase(it);
                datagrams->push_back(temp);
                tryAgain = true;
                break;
            }
        }
    }
}

void PASER_Message_Queue::send_queued_messages(struct in_addr dest_addr) {
    EV << "send all messages to: " << dest_addr.S_addr.getIPv4().str()
            << "\n";
    std::list<message_queue_entry> datagrams;
    getAllPaketsTo(dest_addr, &datagrams);
    for (std::list<message_queue_entry>::iterator it = datagrams.begin();
            it != datagrams.end(); it++) {
        struct message_queue_entry temp = (message_queue_entry) *it;
//        paser_modul->send(temp.p, "to_ip");
        if (temp.p->isScheduled()) {
            opp_error(
                    "the message that you want to send is scheduled. It is unpossible.");
        } else {
            paser_modul->sendDelayed(temp.p,
                    paser_modul->data_message_send_total_delay, "to_ip");
            paser_modul->data_message_send_total_delay +=
                    PASER_DATA_PACKET_SEND_DELAY;
        }
    }
    datagrams.clear();
}

void PASER_Message_Queue::send_queued_messages_for_AddList(
        std::list<address_list> AddList) {
    for (std::list<address_list>::iterator it = AddList.begin();
            it != AddList.end(); it++) {
        address_list tempList = (address_list) *it;
        for (std::list<address_range>::iterator it = tempList.range.begin();
                it != tempList.range.end(); it++) {
            address_range destRange = (address_range) *it;
            struct in_addr dest_addr = destRange.ipaddr;
            struct in_addr mask_addr = destRange.mask;
            EV << "send all messages to ip: "
                    << dest_addr.S_addr.getIPv4().str() << " mask: "
                    << mask_addr.S_addr.getIPv4().str() << "\n";
            std::list<message_queue_entry> datagrams;
            getAllPaketsToAddWithMask(dest_addr, mask_addr, &datagrams);
            for (std::list<message_queue_entry>::iterator it = datagrams.begin();
                    it != datagrams.end(); it++) {
                struct message_queue_entry temp = (message_queue_entry) *it;
//                paser_modul->send(temp.p, "to_ip");
                paser_modul->sendDelayed(temp.p,
                        paser_modul->data_message_send_total_delay, "to_ip");
                paser_modul->data_message_send_total_delay +=
                        PASER_DATA_PACKET_SEND_DELAY;
            }
            datagrams.clear();
        }
    }
}

void PASER_Message_Queue::deleteMessages(struct in_addr dest_addr) {
    EV << "delete all messages to: " << dest_addr.S_addr.getIPv4().str()
            << "\n";
    std::list<message_queue_entry> datagrams;
    getAllPaketsTo(dest_addr, &datagrams);
    for (std::list<message_queue_entry>::iterator it = datagrams.begin();
            it != datagrams.end(); it++) {
        struct message_queue_entry temp = (message_queue_entry) *it;
        paser_modul->MY_sendICMP(temp.p);
//        delete temp.p;
    }
    datagrams.clear();
}
#endif
