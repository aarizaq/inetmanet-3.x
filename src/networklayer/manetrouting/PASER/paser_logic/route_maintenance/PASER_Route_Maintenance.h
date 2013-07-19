/**
 *\class       PASER_Route_Maintenance
 *@brief       Class provides functions to keep routes up-to-date. It manages PASER timers and link layer feedback
 *@ingroup     RM
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
class PASER_Route_Maintenance;

#ifndef PASER_ROUTE_MAINTENANCE_H_
#define PASER_ROUTE_MAINTENANCE_H_

#include "PASER_Global.h"
#include "PASER_Configurations.h"
#include "PASER_Socket.h"
#include "ManetAddress.h"
class PASER_Route_Maintenance {
public:
    PASER_Route_Maintenance(PASER_Global *pGlobal, PASER_Configurations *pConfig,
            PASER_Socket *pModul);

    /**
     *@brief This function is called when a timer expires.
     * It checks the expired timer's type and call the corresponding functions
     * to handle the latter.
     *
     *@param msg Pointer to the expired timer
     */
    void handleSelfMsg(cMessage *msg);

    /**
     * @brief Process a link layer feedback. Delete unreachable routes
     * from routing and neighbor tables.
     *
     *@param src Not used !!!
     *@param dest IP address of the node the route to which has been broken
     *@param sendRERR Decision whether to send a RERR message or not
     */
    void messageFailed(struct in_addr src, struct in_addr dest, bool sendRERR);

private:
    PASER_Global *paser_global;
    PASER_Configurations *paser_configuration;

    PASER_Socket *paser_modul;

    void timeout_KDC_request(PASER_Timer_Message *t);
    void timeout_ROUTE_DISCOVERY_UB(PASER_Timer_Message *t);
    void timeout_ROUTINGTABLE_DELETE_ENTRY(PASER_Timer_Message *t);
    void timeout_ROUTINGTABLE_NO_VALID_ENTRY(PASER_Timer_Message *t);
    void timeout_NEIGHBORTABLE_DELETE_ENTRY(PASER_Timer_Message *t);
    void timeout_NEIGHBORTABLE_NO_VALID_ENTRY(PASER_Timer_Message *t);
    void timeout_TU_RREP_ACK_TIMEOUT(PASER_Timer_Message *t);
    void timeout_HELLO_SEND_TIMEOUT(PASER_Timer_Message *t);
    void timeout_ROOT_TIMEOUT(PASER_Timer_Message *t);
};

#endif /* PASER_ROUTE_MAINTENANCE_H_ */
#endif
