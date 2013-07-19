/**
 *\class       PASER_Timer_Queue
 *@brief       Class provides a list of node's timers
 *@ingroup     TM
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
#ifndef PASER_TIMER_QUEUE_H_
#define PASER_TIMER_QUEUE_H_

//#include <sys/time.h>
#include "PASER_Timer_Message.h"
#include <omnetpp.h>
#include <list>

class PASER_Timer_Queue {

public:
    /**
     * List of node's timers.
     */
    std::list<PASER_Timer_Message *> timer_queue;

public:
    ~PASER_Timer_Queue();

    void init();
    /**
     * @brief Sort the list by time
     */
    void timer_sort();

    /**
     *@brief Add a new timer to the list (shortest to longest timeout order)
     */
    int timer_add(PASER_Timer_Message *t);

    /**
     *@brief Remove a timer from the list
     */
    int timer_remove(PASER_Timer_Message *t);

    /**
     *@brief Return the timer with the shortest timeout period from the list
     */
    PASER_Timer_Message *timer_get_next_timer();

    /**
     *@brief Get time difference
     *
     *@param t1 Pointer to timer 1
     *@param t2 Pointer to timer 2
     *
     *@return t1 - t2
     */
    long timeval_diff(struct timeval *t1, struct timeval *t2);

    /**
     *@brief Get size of the timer list
     */
    int getTimerQueueSize() {
        return timer_queue.size();
    }
    ;

    std::ostream& operator<<(std::ostream& os) {
        os << "timer_queue.size: \n";
        return os;
    }
    ;

};

#endif /* PASER_TIMER_QUEUE_H_ */
#endif
