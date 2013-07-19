/**
 *\class       PASER_Message_Queue
 *@brief       Class is a buffer of all data messages that must be forwarded to an unknown destination
 *@ingroup     Buffer
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
class PASER_Message_Queue;

#ifndef PASER_PACKET_QUEUE_H_
#define PASER_PACKET_QUEUE_H_

//#include <IPv4Datagram.h>

#include <list>
#include "ManetAddress.h"
#include "PASER_Socket.h"

#include <omnetpp.h>
#include "compatibility.h"

struct message_queue_entry {
    cPacket *p;
    struct in_addr dest_addr;
};

class PASER_Message_Queue {
private:
    PASER_Socket *paser_modul; ///< pointer to the PASER module

public:
    /// Container for data messages
    std::list<message_queue_entry> message_queue_list;

public:
    ~PASER_Message_Queue();
    /**
     *@brief Constructor of PASER_Message_Queue.
     *
     *@param pModul Pointer to the PASER module
     */
    PASER_Message_Queue(PASER_Socket *pModul);

    /**
     *@brief Add a new data message to Container
     *
     *@param p Pointer to the message
     *@param dest_addr IP address of the message destination
     *
     */
    void message_add(cPacket * p, struct in_addr dest_addr);

    /**
     *@brief Get a list of all buffered messages for the destination with the IP address <b>dest_addr</b>
     * delete these messages from container
     *
     *@param dest_addr IP address of the message destination
     *@param datagrams List of all buffered messages for that destination
     *
     */
    void getAllPaketsTo(struct in_addr dest_addr,
            std::list<message_queue_entry> *datagrams);

    /**
     *@brief Get a list of all buffered messages for the destination with the IP address <b>dest_addr</b>
     * and the network mask <b>mask_addr</b>
     * delete these messages from container
     *
     *@param dest_addr IP address of the message destination
     *@param mask_addr Network mask of the destination IP address
     *@param datagrams List of all buffered messages for that destination
     *
     */
    void getAllPaketsToAddWithMask(struct in_addr dest_addr,
            struct in_addr mask_addr, std::list<message_queue_entry> *datagrams);

    /**
     *@brief Send all messages in container to the <b>dest_addr</b> IP address
     *
     *@param dest_addr IP address of the messages destination
     *
     */
    void send_queued_messages(struct in_addr dest_addr);

    /**
     *@brief Send all messages in container to the destinations listed in <b>AddList</b>
     *
     *@param AddList List IP addresses for which the messages should be sent
     *
     */
    void send_queued_messages_for_AddList(std::list<address_list> AddList);

    /**
     *@brief Delete all messages for destination <b>dest_addr</b> from container
     *
     *@param dest_addr IP address for the messages destination
     *
     */
    void deleteMessages(struct in_addr dest_addr);

};

#endif /* PASER_PACKET_QUEUE_H_ */
#endif
