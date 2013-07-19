/**
 *\class       PASER_UB_RREQ
 *@brief       Class is an implementation of PASER_UB_RREQ messages
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
#ifndef PASER_UB_RREQ_H_
#define PASER_UB_RREQ_H_

#include "PASER_Definitions.h"
#include <omnetpp.h>

#include <list>
#include <string.h>

#include "PASER_MSG.h"

class PASER_UB_RREQ: public PASER_MSG {
public:
    u_int32_t keyNr; ///< Current number of GTK

    u_int32_t seqForw; ///< Sequence number of forwarding node

    u_int32_t GFlag; ///< Gateway flag
    std::list<address_list> AddressRangeList; ///< Route list from querying node to forwarding node
    u_int32_t metricBetweenQueryingAndForw; ///< Metric of the route between querying node and forwarding node

    lv_block cert; ///< Certificate of sending node
    u_int32_t nonce; ///< Registration nonce of sending node
    lv_block certForw; ///< Certificate of forwarding node
    u_int8_t * root; ///< RooT element of forwarding node
    u_int32_t initVector; ///< IV of forwarding node
    geo_pos geoQuerying; ///< Geographical position of sending node
    geo_pos geoForwarding; ///< Geographical position of forwarding node
    lv_block sign; ///< Signature of the message

    long timestamp; ///< Sending or forwarding time
    /**
     * @brief Constructor of PASER_UU_RREQ messages. It duplicates an existing message
     *
     *@param m Pointer to the message to duplicate
     */
    PASER_UB_RREQ(const PASER_UB_RREQ &m);
    /**
     * @brief  Constructor of PASER_UU_RREQ message. It creates a new message.
     *
     *@param src IP address of sending node
     *@param dest IP address of querying node
     *@param seqNr Sequence number of sending node
     */
    PASER_UB_RREQ(struct in_addr src, struct in_addr dest, u_int32_t seqNr);

    ~PASER_UB_RREQ();
    /**
     * @brief  Import the content of another message
     *
     *@param m Pointer to the message the content of which should be imported
     *@return Reference to myself
     */
    PASER_UB_RREQ& operator=(const PASER_UB_RREQ &m);
    virtual PASER_UB_RREQ *dup() const {
        return new PASER_UB_RREQ(*this);
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

#endif /* PASER_UB_RREQ_H_ */
#endif
