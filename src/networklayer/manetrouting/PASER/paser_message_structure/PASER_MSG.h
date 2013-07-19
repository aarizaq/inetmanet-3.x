/**
 *\class       PASER_MSG
 *@brief       Class is a generic implementation of common PASER messages fields/features
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
#ifndef PASER_MSG_H_
#define PASER_MSG_H_

#include "PASER_Definitions.h"
#include <omnetpp.h>
#include "compatibility.h"

/**
 * Type of PASER messages
 */
enum message_type {
    UB_RREQ = 0,
    UU_RREP = 1,
    TU_RREQ = 2,
    TU_RREP = 3,
    TU_RREP_ACK = 4,
    B_RERR = 5,
    B_HELLO = 6,
    B_ROOT = 7,
    B_RESET = 8
};

class PASER_MSG: public cPacket {
public:
    message_type type; ///< Type of PASER messages
    struct in_addr srcAddress_var; ///< IP address of source node
    struct in_addr destAddress_var; ///< IP address of destination node
    u_int32_t seq; ///< Current sequence number of sending node

public:
    PASER_MSG();
    ~PASER_MSG();

    /**
     * @brief Produces a multi-line description of the message's content
     *
     *@return Description of the message content
     */
    virtual std::string detailedInfo() const=0;

    /**
     *@brief  Duplicates a PASER message
     */
    virtual PASER_MSG *dup() const=0;
    /**
     * @brief Creates and return an array of all fields that must be secured via hash or signature
     *
     *@param l Length of the created array
     *@return Array
     */
    virtual u_int8_t *toByteArray(int *l)=0;

    /**
     *@brief  Creates and return an array of all message fields
     *
     *@param l Length of the created array
     *@return Array
     */
    virtual u_int8_t *getCompleteByteArray(int *l)=0;
};

#endif /* PASER_MSG_H_ */
#endif
