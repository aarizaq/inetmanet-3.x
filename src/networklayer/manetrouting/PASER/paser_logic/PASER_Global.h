/**
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
class PASER_Global;

#ifndef PASER_GLOBAL_H_
#define PASER_GLOBAL_H_

#include "PASER_Message_Queue.h"
#include "PASER_Message_Processing.h"
#include "PASER_Route_Discovery.h"
#include "PASER_Route_Maintenance.h"
#include "PASER_Configurations.h"
#include "PASER_Crypto_Hash.h"
#include "PASER_Crypto_Sign.h"
#include "PASER_Crypto_Root.h"
#include "PASER_RREQ_List.h"
#include "PASER_RERR_List.h"
#include "PASER_Definitions.h"

class PASER_Global {
public:
    PASER_Global(PASER_Configurations *pConfig, PASER_Socket *pModul);
    ~PASER_Global();

    std::list<address_range> getAddL();
    PASER_Crypto_Hash *getCrypto_hash();
    PASER_Crypto_Sign *getCrypto_sign();
    PASER_Neighbor_Table *getNeighbor_table();
    network_device *getNetDevice();
    PASER_Message_Queue *getMessage_queue();
    PASER_Crypto_Root *getRoot();
    PASER_Routing_Table *getRouting_table();
    PASER_RREQ_List *getRrep_list();
    PASER_RREQ_List *getRreq_list();
    PASER_Timer_Queue *getTimer_queue();
    PASER_Configurations *getPaser_configuration();
    PASER_Socket *getPASER_modul();
    PASER_RERR_List *getBlacklist();

    bool getIsGW();
    void setIsGW(bool i);
    bool getIsRegistered();
    void setIsRegistered(bool i);
    bool getWasRegistered();
    void setWasRegistered(bool i);
    u_int32_t getSeqNr();
    void setSeqNr(u_int32_t s);
    u_int32_t getLastGwSearchNonce();
    void generateGwSearchNonce();
    void setLastGwSearchNonce(u_int32_t s);

    PASER_Timer_Message* getHelloMessageTimer();
    void activateHelloMessageTimer();
    void resetHelloTimer();

    void setPaket_processing(PASER_Message_Processing *pProcessing);
    void setRoute_findung(PASER_Route_Discovery *rFindung);
    void setRoute_maintenance(PASER_Route_Maintenance *rMaintenance);
    PASER_Message_Processing *getPaket_processing();
    PASER_Route_Discovery *getRoute_findung();
    PASER_Route_Maintenance *getRoute_maintenance();

    geo_pos getGeoPosition();

    bool isHelloActive();
    bool isLinkLayerActive();

    /**
     * @brief  Reset all PASER configurations and delete all PASER tables
     */
    void resetPASER();

    bool isSeqNew(u_int32_t oldSeq, u_int32_t newSeq);

private:
    PASER_Timer_Queue *timer_queue;
    PASER_Routing_Table *routing_table;
    PASER_Neighbor_Table *neighbor_table;
    PASER_Message_Queue *message_queue;
    PASER_Configurations *paser_configuration;
    PASER_Message_Processing *paket_processing;
    PASER_Route_Discovery *route_findung;
    PASER_Route_Maintenance *route_maintenance;

    PASER_RREQ_List *rreq_list;
    PASER_RREQ_List *rrep_list;

    PASER_Crypto_Root *root;
    PASER_Crypto_Sign *crypto_sign;
    PASER_Crypto_Hash *crypto_hash;

    PASER_RERR_List *blackList;

    std::list<address_range> AddL;

    u_int32_t seqNr;

    uint32_t lastGwSearchNonce;

    bool isGW;
    bool isRegistered;
    bool wasRegistered;

    bool isHello;
    bool isLinkLayer;

    network_device *netDevice;
    network_device *netAddDevice;

    PASER_Timer_Message *hello_message_interval;

    PASER_Socket *paser_modul;
};

#endif /* PASER_GLOBAL_H_ */
#endif
