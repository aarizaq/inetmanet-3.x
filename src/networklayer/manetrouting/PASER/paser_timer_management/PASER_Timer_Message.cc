/*
 *\class       PASER_Timer_Message
 *@brief       Class corresponds to an entry in the timer queue
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
#include "PASER_Timer_Message.h"
#include "PASER_Definitions.h"
#include "PASER_UB_RREQ.h"
#include "PASER_UU_RREP.h"

PASER_Timer_Message::~PASER_Timer_Message() {
    if (data) {
        PASER_UB_RREQ *pack0;
        PASER_UU_RREP *pack1;
        switch (handler) {
        case ROUTE_DISCOVERY_UB:
            pack0 = (PASER_UB_RREQ *) data;
            delete pack0;
            data = NULL;
            break;
        case TU_RREP_ACK_TIMEOUT:
            EV << "deleting PASER_UU_RREP\n";
            pack1 = (PASER_UU_RREP *) data;
            delete pack1;
            data = NULL;
        default:
            break;
        }
    }
}

//PASER_Timer_Message &PASER_Timer_Message::operator =(PASER_Timer_Message *op2){
//    PASER_Timer_Message * result = new PASER_Timer_Message();
//    result->data = op2->data;
//    result->destAddr.S_addr = op2->destAddr.S_addr;
//    result->handler = op2->handler;
//    result->timeout = op2->timeout;
//    result->used = op2->used;
//    return result;
//}

bool PASER_Timer_Message::operator==(PASER_Timer_Message *op2) {
    return (destAddr.S_addr == op2->destAddr.S_addr &&
//            timeout.tv_sec == op2->timeout.tv_sec &&
//            timeout.tv_usec == op2->timeout.tv_usec &&
            handler == op2->handler);
}

//bool PASER_Timer_Message::operator<(PASER_Timer_Message *op2){
//    bool result = false;
//    result = (
//            (timeout.tv_sec < op2->timeout.tv_sec) ||
//            (
//             (timeout.tv_sec == op2->timeout.tv_sec) &&
//             (timeout.tv_usec < op2->timeout.tv_usec)
//            )
//            );
//    return result;
//}
#endif
