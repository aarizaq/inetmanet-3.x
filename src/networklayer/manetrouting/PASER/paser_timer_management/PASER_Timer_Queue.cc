/*
 *\class       PASER_Timer_Queue
 *@brief       Class provides a list of node's timers
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
#include "PASER_Timer_Queue.h"

#include "PASER_UB_RREQ.h"

PASER_Timer_Queue::~PASER_Timer_Queue() {
    for (std::list<PASER_Timer_Message *>::iterator it = timer_queue.begin();
            it != timer_queue.end(); it++) {
        PASER_Timer_Message *temp = (PASER_Timer_Message*) *it;
        delete temp;
    }
    timer_queue.clear();
}

bool compare_list(PASER_Timer_Message *op1, PASER_Timer_Message *op2) {
    bool result = false;
    result = ((op1->timeout.tv_sec < op2->timeout.tv_sec)
            || ((op1->timeout.tv_sec == op2->timeout.tv_sec)
                    && (op1->timeout.tv_usec < op2->timeout.tv_usec)));
    return result;
}

void PASER_Timer_Queue::timer_sort() {
    timer_queue.sort(compare_list);
}

int PASER_Timer_Queue::timer_add(PASER_Timer_Message *t) {
    timer_remove(t);
    timer_queue.push_front(t);
    timer_queue.sort(compare_list);
    return 1;
}

int PASER_Timer_Queue::timer_remove(PASER_Timer_Message *t) {
    if (!t) {
        EV << "timer == NULL\n";
        return 0;
    }
    if (t->handler == KDC_REQUEST) {
        for (std::list<PASER_Timer_Message *>::iterator it = timer_queue.begin();
                it != timer_queue.end(); it++) {
            PASER_Timer_Message *temp = (PASER_Timer_Message *) *it;
            if (temp->handler == KDC_REQUEST) {
                timer_queue.erase(it);
                delete temp;
                return 1;
            }
        }
    } else {
        for (std::list<PASER_Timer_Message *>::iterator it = timer_queue.begin();
                it != timer_queue.end(); it++) {
            PASER_Timer_Message *temp = (PASER_Timer_Message *) *it;
            if (temp->handler == ROUTINGTABLE_DELETE_ENTRY
                    || temp->handler == ROUTINGTABLE_VALID_ENTRY
                    || temp->handler == NEIGHBORTABLE_DELETE_ENTRY
                    || temp->handler == NEIGHBORTABLE_VALID_ENTRY) {
                if (temp->destAddr.S_addr == t->destAddr.S_addr
                        && temp->handler == t->handler) {
                    timer_queue.erase(it);
                    return 1;
                }
            } else {
                if (temp->destAddr.S_addr == t->destAddr.S_addr
                        && temp->handler == t->handler) {
                    timer_queue.erase(it);
                    return 1;
                }
            }
        }
        EV << "timer not found\n";
        return 0;
    }
    return 0;
}

PASER_Timer_Message *PASER_Timer_Queue::timer_get_next_timer() {
    if (timer_queue.size() == 0) {
        return NULL;
    }
    return timer_queue.front();
}

long PASER_Timer_Queue::timeval_diff(struct timeval *t1, struct timeval *t2) {
    long long res;
    if (t1 && t2) {
        res = t1->tv_sec;
        res = ((res - t2->tv_sec) * 1000000 + t1->tv_usec - t2->tv_usec) / 1000;
        return (long) res;
    }
    return -1;
}

#endif
