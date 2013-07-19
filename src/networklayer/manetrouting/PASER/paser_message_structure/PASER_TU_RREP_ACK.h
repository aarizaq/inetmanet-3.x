/**
 *\class       PASER_TU_RREP_ACK
 *@brief       Class is an implementation of PASER_TU_RREP_ACK messages
 *@ingroup     MS
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
#ifndef PASER_TU_RREP_ACK_H_
#define PASER_TU_RREP_ACK_H_

#include "PASER_Definitions.h"
#include <omnetpp.h>
#include <compatibility.h>

#include <list>

#include "PASER_MSG.h"

class PASER_TU_RREP_ACK: public PASER_MSG {
public:
    u_int32_t keyNr; ///< Current number of GTK
    u_int8_t *secret; ///< Secret of forwarding node
    std::list<u_int8_t *> auth; ///< Authentication path of forwarding node
    u_int8_t *hash; ///< Hash of the message

    /**
     *@brief  Constructor of PASER_TU_RREP_ACK messages. It duplicates an existing message
     *
     *@param m Pointer to the message to duplicate
     */
    PASER_TU_RREP_ACK(const PASER_TU_RREP_ACK &m);
    /**
     *@brief  Constructor of PASER_TU_RREP_ACK message. It creates a new message.
     *
     *@param src IP address of sending node
     *@param dest IP address of querying node
     *@param seqNr Sequence number of sending node
     */
    PASER_TU_RREP_ACK(struct in_addr src, struct in_addr dest, u_int32_t seqNr);

    ~PASER_TU_RREP_ACK();
    /**
     *@brief Import the content of another message
     *
     *@param m Pointer to the message the content of which should be imported
     *@return Reference to myself
     */
    PASER_TU_RREP_ACK& operator=(const PASER_TU_RREP_ACK &m);
    virtual PASER_TU_RREP_ACK *dup() const {
        return new PASER_TU_RREP_ACK(*this);
    }
    /**
     * @brief Produces a multi-line description of the message's content
     *
     *@return Description of the message content
     */
    std::string detailedInfo() const;
    /**
     * @brief Creates and return an array of all fields that must be secured via hash or signature
     *
     *@param l Length of the created array
     *@return Array
     */
    u_int8_t * toByteArray(int *l);
    /**
     * @brief Creates and return an array of all message fields
     *
     *@param l Length of the created array
     *@return Array
     */
    u_int8_t * getCompleteByteArray(int *l);

};

#endif /* PASER_TU_RREP_ACK_H_ */
#endif
