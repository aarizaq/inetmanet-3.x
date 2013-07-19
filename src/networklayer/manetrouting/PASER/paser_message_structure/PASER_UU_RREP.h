/**
 *\class       PASER_UU_RREP
 *@brief       Class is an implementation of PASER_UU_RREP messages
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
#ifndef PASER_UU_RREP_H_
#define PASER_UU_RREP_H_

#include "PASER_Definitions.h"
#include <omnetpp.h>

#include "PASER_MSG.h"

class PASER_UU_RREP: public PASER_MSG {
public:
    u_int32_t keyNr; ///< Current number of GTK

    u_int32_t GFlag; ///< Gateway flag
    std::list<address_list> AddressRangeList; ///< Route list from querying node to forwarding node
    u_int32_t metricBetweenQueryingAndForw; ///< Metric of the route between querying node and forwarding node
    u_int32_t metricBetweenDestAndForw; ///< Metric of the route between destination node and forwarding node

    lv_block certForw; ///< Certificate of forwarding node
    u_int8_t * root; ///< Root element of forwarding node
    u_int32_t initVector; ///< IV of forwarding node
    geo_pos geoDestination; ///< Geographical position of destination node
    geo_pos geoForwarding; ///< Geographical position of forwarding node
    kdc_block kdc_data; ///< KDC data block
    lv_block sign; ///< Signature of the message
    long timestamp; ///< Sending or forwarding time

    /**
     * @brief  Constructor of PASER_UU_RREP messages. It duplicates an existing message
     *
     *@param m Pointer to the message to duplicate
     */

    PASER_UU_RREP(const PASER_UU_RREP &m);

    /**
     * @brief  Constructor of PASER_UU_RREP message. It creates a new message.
     *
     *@param src IP address of originator
     *@param dest IP address of querying node
     *@param seqNr Sequence number of originator
     */
    PASER_UU_RREP(struct in_addr src, struct in_addr dest, u_int32_t seqNr);

    ~PASER_UU_RREP();

    /**
     * @brief  Import the content of another message
     *
     *@param m Pointer to the message the content of which should be imported
     *@return Reference to myself
     */
    PASER_UU_RREP& operator=(const PASER_UU_RREP &m);
    virtual PASER_UU_RREP *dup() const {
        return new PASER_UU_RREP(*this);
    }

    /**
     *@brief  Produces a multi-line description of the message's content
     *
     *@return Description of the message content
     */
    std::string detailedInfo() const;

    /**
     * @brief  Creates and return an array of all fields that must be secured via hash or signature
     *
     *@param l Length of the created array
     *@return Array
     */
    u_int8_t * toByteArray(int *l);
    /**
     * @brief  Creates and return an array of all message fields
     *
     *@param l Length of the created array
     *@return Array
     */
    u_int8_t * getCompleteByteArray(int *l);

};

#endif /* PASER_UU_RREP_H_ */
#endif
