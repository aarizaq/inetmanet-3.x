/**
 *\class       PASER_Route_Discovery
 *@brief       Class provides functions to manage a node registration at a gateway or to handle a route discovery
 *@ingroup     RD
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
class PASER_Route_Discovery;

#ifndef PASER_ROUTE_FINDUNG_H_
#define PASER_ROUTE_FINDUNG_H_
#include "IPv4Datagram.h"
#include "PASER_Global.h"
#include "PASER_Configurations.h"
#include "PASER_Socket.h"

class PASER_Route_Discovery {

public:
    PASER_Route_Discovery(PASER_Global *pGlobal, PASER_Configurations *pConfig,
            PASER_Socket *pModul, bool setGWsearch);

    /**
     * @brief Start registration of the node at a gateway (only if the node is not registered).
     *
     */
    void tryToRegister();

    /**
     * @brief Start route discovery (only if not already started)
     *
     *@param dest_addr IP address of destination node or broadcast if <b>isDestGW</b> is set.
     *@param isDestGW Flag is set if a route to a gateway must be found.
     */
    void route_discovery(struct in_addr dest_addr, int isDestGW);

    /**
     * @brief Process a data message. If the route to the message destination is not known,
     * then start a route discovery
     *
     *@param datagram Pointer to the data message
     */
    void processMessage(IPv4Datagram* datagram);

private:
    PASER_Global *paser_global;
    PASER_Configurations *paser_configuration;
    PASER_Socket *paser_modul;

    bool isGWsearch;
};

#endif /* PASER_ROUTE_FINDUNG_H_ */
#endif
