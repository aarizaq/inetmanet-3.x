/**
 *\class       PASER_Timer_Message
 *@brief       Class corresponds to an entry in the timer queue
 *@ingroup     TM
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
#ifndef PASER_TIMER_PACKET_H_
#define PASER_TIMER_PACKET_H_

//#include <sys/time.h>
#include "PASER_Definitions.h"
#include <compatibility.h>

enum timeout_var {
    KDC_REQUEST,
    ROUTE_DISCOVERY_UB,
    ROUTINGTABLE_DELETE_ENTRY,
    ROUTINGTABLE_VALID_ENTRY,
    NEIGHBORTABLE_DELETE_ENTRY,
    NEIGHBORTABLE_VALID_ENTRY,
    TU_RREP_ACK_TIMEOUT,
    HELLO_SEND_TIMEOUT,
    PASER_ROOT
};

class PASER_Timer_Message {
public:
    struct timeval timeout; ///< The time when the timeout expires
    struct in_addr destAddr; ///< IP address of the corresponding node
    timeout_var handler; ///< Type of the timeout
    void *data; ///< Pointer to auxiliary data

public:
    ~PASER_Timer_Message();

    bool operator==(PASER_Timer_Message *op2);

    std::ostream& operator<<(std::ostream& os) {
        os << "timer_queue.size: \n";
        return os;
    }
    ;
};

#endif /* PASER_TIMER_PACKET_H_ */
#endif
