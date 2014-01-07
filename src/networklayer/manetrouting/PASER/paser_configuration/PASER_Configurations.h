/**
 *\class       PASER_Configurations
 *@brief       Class is a parser of the PASER configuration in the NED file
 *@ingroup     Configuration
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
class PASER_Configurations;

#ifndef PASER_CONFIG_H_
#define PASER_CONFIG_H_

#include "PASER_Definitions.h"
#include "PASER_Socket.h"

class PASER_Configurations {
public:
    /**
     * @brief Constructor
     */
    PASER_Configurations(char *configFile, PASER_Socket *pModul);
    ~PASER_Configurations();

    bool getIsGW();
    char *getCertfile();
    char *getKeyfile();
    char *getCAfile();
    bool isGWsearch();

    void setGTK(lv_block _GTK);
    lv_block getGTK();

    void setKDC_cert(lv_block _KDC_cert);
    void getKDCCert(lv_block *t);
    lv_block getKDC_cert();

    void setRESET_sign(lv_block _RESET_sign);
    void getRESETSign(lv_block *s);
    lv_block getRESET_sign();

    void setKeyNr(u_int32_t k);
    u_int32_t getKeyNr();

    u_int32_t getRootRepetitionsTimeout();
    u_int32_t getRootRepetitions();

    u_int32_t getNetEthDeviceNumber();
    network_device *getNetEthDevice();
    u_int32_t getNetDeviceNumber();
    network_device *getNetDevice();
    u_int32_t getNetAddDeviceNumber();
    network_device *getNetAddDevice();
    struct in_addr getAddressOfKDC();

    bool isSetLinkLayerFeeback();

    bool isSetLocalRepair();
    u_int32_t getMaxLocalRepairHopCount();

    /**
     *@brief Test if a given address <b>Addr</b> is in the own subnetworks
     *
     *@param Addr IP address that is going to be tested
     */
    bool isAddInMySubnetz(struct in_addr Addr);
    bool isRootReady;
private:
    bool isGW; ///< Is this node a gateway
    char *certfile, *keyfile, *cafile; ///< Path to node's certificate, key and CA file

    lv_block GTK; ///< Current GTK
    u_int32_t key_nr; ///< Current number of GTK
    lv_block KDC_cert; ///< Certificate of KDC
    lv_block RESET_sign; ///< Signature of UB_Key_Refresh (RESET) message

    u_int32_t netEthDeviceNumber; ///< Number of Ethernet cards
    u_int32_t netDeviceNumber; ///< Number of wireless cards PASER is going to run on
    u_int32_t netAddDeviceNumber; ///< Number of own subnetworks

    network_device *netEthDevice; ///< Array of Ethernet cards
    network_device *netDevice; ///< Array of wireless cards PASER is going to run on
    network_device *netAddDevice; ///< Array of subnetworks

    u_int32_t root_repetitions_timeout; ///< Root message sending interval
    u_int32_t root_repetitions; ///< Number of Root message sending repetitions

    struct in_addr addressOfKDC; ///< Address of KDC

    bool setGWsearch; ///< Enable proactive search for the gateway
    bool isLinkLayerFeeback; ///< Enable link layer feedback

    bool isLocalRepair; ///< Enable local repair
    u_int32_t maxHopCountForLocalRepair; ///< Maximum allowed number of hops towards the node for which the route should be locally repaired

    PASER_Socket *paser_modul;

    /**
     * @brief Initialize the array of node's subnetworks
     */
    void intAddlList();
};

#endif /* PASER_CONFIG_H_ */
#endif
