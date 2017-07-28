/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Callback timer implementation
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup ctimer
 * @{
 */

#include "rpl.h"

static char initialized;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

namespace inet {
/*---------------------------------------------------------------------------*/
void RPL::ctimer_reschedule()
{
    if (rplTimerMap.empty()) {
        if (nextEvent.isScheduled())
            cancelEvent(&nextEvent);
        return;
    }

    if (nextEvent.getArrivalTime() > rplTimerMap.begin()->first)
        cancelEvent(&nextEvent);

    if (!nextEvent.isScheduled()) {
        scheduleAt(rplTimerMap.begin()->first, &nextEvent);
    }
}
/*---------------------------------------------------------------------------*/
void RPL::ctimer_init(void)
{
    while (!rplTimerMap.empty()){
        delete  rplTimerMap.begin()->second;
        rplTimerMap.erase(rplTimerMap.begin());
    }
    cancelEvent(&nextEvent);
}

/*---------------------------------------------------------------------------*/
void RPL::ctimer_set(ctimer *c, simtime_t t,
        timeout_func_t f, void *ptr)
{
    for (auto it = rplTimerMap.begin();it != rplTimerMap.end();) {
        if (it->second == c)
            rplTimerMap.erase(it++);
        else
            ++it;
    }

    c->timeout = t;
    c->handler = f;
    c->data = ptr;

    rplTimerMap.insert(std::make_pair(simTime()+t,c));
    ctimer_reschedule();
}

/*---------------------------------------------------------------------------*/
void
RPL::ctimer_reset(ctimer *c)
{

    for (auto it = rplTimerMap.begin();it != rplTimerMap.end();) {
         if (it->second == c)
             rplTimerMap.erase(it++);
         else
             ++it;
     }
     rplTimerMap.insert(std::make_pair(simTime()+c->timeout,c));
     ctimer_reschedule();
}

/*---------------------------------------------------------------------------*/
void
RPL::ctimer_restart(struct ctimer *c)
{
    ctimer_reset(c);
}

/*---------------------------------------------------------------------------*/
void
RPL::ctimer_stop(struct ctimer *c)
{
    for (auto it = rplTimerMap.begin();it != rplTimerMap.end();) {
        if (it->second == c)
            rplTimerMap.erase(it++);
        else
            ++it;
    }
}

/*---------------------------------------------------------------------------*/
int
RPL::ctimer_expired(ctimer *c)
{
    for (auto it = rplTimerMap.begin();it != rplTimerMap.end();) {
        if (it->second == c)
            return 0;
        else
            ++it;
    }
    return 1;
}
}
/*---------------------------------------------------------------------------*/
/** @} */
