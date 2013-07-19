/**
 *\class       PASER_Message_Processing
 *@brief       Class provides functions to manage all PASER messages
 *@ingroup     MP
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
class PASER_Message_Processing;

#ifndef PASER_PAKET_PROCESSING_H_
#define PASER_PAKET_PROCESSING_H_

#include "PASER_Socket.h"
#include "PASER_Configurations.h"
#include "PASER_Global.h"
#include "PASER_Message_Queue.h"
#include "PASER_Route_Discovery.h"
#include "PASER_Route_Maintenance.h"
#include "PASER_RREQ_List.h"
#include "PASER_Crypto_Root.h"
#include "PASER_Crypto_Hash.h"
#include "PASER_Crypto_Sign.h"
#include "ManetAddress.h"
#include "crl_message_m.h"
#include "PASER_UU_RREP.h"
//#include "kdcReset_m.h"

class PASER_Message_Processing {
private:
    PASER_Socket *paser_modul;
    PASER_Global *paser_global;
    PASER_Configurations *paser_configuration;
    PASER_Timer_Queue *timer_queue;
    PASER_Routing_Table *routing_table;
    PASER_Neighbor_Table *neighbor_table;
    PASER_Message_Queue *message_queue;
    PASER_Crypto_Root *root;
    PASER_Crypto_Sign *crypto_sign;
    PASER_Crypto_Hash *crypto_hash;
    PASER_Route_Discovery *route_findung;
    PASER_Route_Maintenance *route_maintenance;

    PASER_RREQ_List *rreq_list; ///< List of IP addresses to which a route discovery has been started
    PASER_RREQ_List *rrep_list; ///< List of IP addresses from which a TU-RREP-ACK is expected

    bool isGW; ///< This flag is set if the node is a Gateway
    network_device *netDevice; ///< Pointer to an Array of wireless cards on which PASER is running
    lv_block GTK; ///< Current GTK

public:
    PASER_Message_Processing(PASER_Global *pGlobal, PASER_Configurations *pConfig,
            PASER_Socket *pModul);
    ~PASER_Message_Processing();

    /**
     *
     * @brief Process a received PASER message. Checks its type and call the
     * corresponding functions to handle the latter
     *
     */
    void handleLowerMsg(cPacket * msg, u_int32_t ifIndex);

    /**
     * @brief Check sequence number of the PASER message
     *
     *@param paser_msr Pointer to the message
     *@param forwarding IP address of the node which forwarded the message
     *
     *@return 1 if the sequence number is fresh
     *        else 0
     */
    int check_seq_nr(PASER_MSG *paser_msg, struct in_addr forwarding);

    /**
     * @brief Check if the specified position is in the range of own wireless card.
     *
     *@param position Geo Position
     *
     *@return 1 if the <b>position</b> is in the range of own wireless card
     *        else 0.
     */
    int check_geo(geo_pos position);

    /**
     * @brief Check if IP address of own wireless card is in the list.
     *
     *@param rList List of IP addresses
     *
     *@return 1 IP address of own wireless card is in the list
     *        else 0.
     */
    int checkRouteList(std::list<address_list> rList);

    /**
     * @brief Functions to process a received PASER message. Check the message,
     * edit routing and neighbor tables and send a reply or forward the message,
     * if necessary.
     */
    void handleUBRREQ(cPacket * msg, u_int32_t ifIndex);
    void handleUURREP(cPacket * msg, u_int32_t ifIndex);
    void handleTURREQ(cPacket * msg, u_int32_t ifIndex);
    void handleTURREP(cPacket * msg, u_int32_t ifIndex);
    void handleTURREPACK(cPacket * msg, u_int32_t ifIndex);
    void handleRERR(cPacket * msg, u_int32_t ifIndex);
    void handleHELLO(cPacket * msg, u_int32_t ifIndex);
    void handleB_ROOT(cPacket * msg, u_int32_t ifIndex);
    void handleB_RESET(cPacket * msg, u_int32_t ifIndex);
    /*--------------------------------------------------------*/

    /**
     * @brief Functions to send a PASER message
     */
    PASER_UB_RREQ * send_ub_rreq(struct in_addr src_addr,
            struct in_addr dest_addr, int isDestGW);
    PASER_UU_RREP * send_uu_rrep(struct in_addr src_addr,
            struct in_addr forw_addr, struct in_addr dest_addr, int isDestGW,
            X509 *cert, kdc_block kdcData);
    PASER_TU_RREP * send_tu_rrep(struct in_addr src_addr,
            struct in_addr forw_addr, struct in_addr dest_addr, int isDestGW,
            X509 *cert, kdc_block kdcData);
    PASER_TU_RREP_ACK * send_tu_rrep_ack(struct in_addr src_addr,
            struct in_addr dest_addr);
    void send_rerr(std::list<unreachableBlock> unreachableList);
    void send_root();
    void send_reset();
    /*--------------------------------------------------------*/

    /**
     * Functions to forward a PASER message
     */
    PASER_UB_RREQ * forward_ub_rreq(PASER_UB_RREQ *oldMessage);
    PASER_TU_RREQ * forward_ub_rreq_to_tu_rreq(PASER_UB_RREQ *oldMessage,
            struct in_addr nxtHop_addr, struct in_addr dest_addr);
    PASER_UU_RREP * forward_uu_rrep(PASER_UU_RREP *oldMessage,
            struct in_addr nxtHop_addr);
    PASER_TU_RREP * forward_uu_rrep_to_tu_rrep(PASER_UU_RREP *oldMessage,
            struct in_addr nxtHop_addr);
    PASER_TU_RREQ * forward_tu_rreq(PASER_TU_RREQ *oldMessage,
            struct in_addr nxtHop_addr);
    PASER_UU_RREP * forward_tu_rrep_to_uu_rrep(PASER_TU_RREP *oldMessage,
            struct in_addr nxtHop_addr);
    PASER_TU_RREP * forward_tu_rrep(PASER_TU_RREP *oldMessage,
            struct in_addr nxtHop_addr);
    /*--------------------------------------------------------*/

    /**
     *  @brief Delete an IP address from <b>rreq_list</b> as well as the corresponding timer from timer_queue.
     *
     * @param dest_addr IP address to delete
     */
    void deleteRouteRequestTimeout(struct in_addr dest_addr);

    /**
     *  @brief Delete a list of IP addresses from <b>rreq_list</b> as well as the corresponding timers from timer_queue
     *
     * @param AddList List of IP addresses to delete
     */
    void deleteRouteRequestTimeoutForAddList(std::list<address_list> AddList);

    /**
     * @brief Send a registration request to KDC. The request will be sent over Ethernet.
     * This function is only called by a gateway.
     *
     *@param nodeAddr IP address of the registered node
     *@param nextHop IP address of the next hop node to the registered node
     *@param cert certificate of the registered node
     *@param nonce nonce of the registered node
     */
    void sendKDCRequest(struct in_addr nodeAddr, struct in_addr nextHop,
            lv_block cert, int nonce);

    /**
     * @brief Check the KDC registration reply and forward it to the destination node
     */
    void checkKDCReply(cPacket * msg);

//    /**
//     *
//     */
//    void checkKDCReset(cPacket * msg);

};

#endif /* PASER_PAKET_PROCESSING_H_ */
#endif
