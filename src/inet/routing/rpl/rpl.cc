/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         ContikiRPL, an implementation of RPL: IPv6 Routing Protocol
 *         for Low-Power and Lossy Networks (IETF RFC 6550)
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

/**
 * \addtogroup uip6
 * @{
 */

#include "inet/routing/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ip/tcpip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/rpl/rpl-private.h"
#include "net/rpl/rpl-ns.h"
#include "net/ipv6/multicast/uip-mcast6.h"

#define DEBUG DEBUG_NONE

#include <limits.h>
#include <string.h>

namespace inet {

#if RPL_CONF_STATS
rpl_stats_t rpl_stats;
#endif


/*---------------------------------------------------------------------------*/
enum rpl_mode
RPL::rpl_get_mode(void)
{
  return mode;
}
/*---------------------------------------------------------------------------*/
enum rpl_mode
RPL::rpl_set_mode(enum rpl_mode m)
{
  enum rpl_mode oldmode = mode;

  /* We need to do different things depending on what mode we are
     switching to. */
  if(m == RPL_MODE_MESH) {

    /* If we switch to mesh mode, we should send out a DAO message to
       inform our parent that we now are reachable. Before we do this,
       we must set the mode variable, since DAOs will not be sent if
       we are in feather mode. */
    PRINTF("RPL: switching to mesh mode\n");
    mode = m;

    if(default_instance != NULL) {
      rpl_schedule_dao_immediately(default_instance);
    }
  } else if(m == RPL_MODE_FEATHER) {

    PRINTF("RPL: switching to feather mode\n");
    if(default_instance != NULL) {
      PRINTF("RPL: rpl_set_mode: RPL sending DAO with zero lifetime\n");
      if(default_instance->current_dag != NULL) {
        dao_output(default_instance->current_dag->preferred_parent, RPL_ZERO_LIFETIME);
      }
      rpl_cancel_dao(default_instance);
    } else {
      PRINTF("RPL: rpl_set_mode: no default instance\n");
    }

    mode = m;
  } else {
    mode = m;
  }

  return oldmode;
}
/*---------------------------------------------------------------------------*/
void
RPL::rpl_purge_routes(void)
{
    std::vector<IPv6RouteRpl *> delEntries;

    /* First pass, decrement lifetime */
    for (int i = 0 ;i < rtTable->getNumRoutes();i++) {
        IPv6RouteRpl *r = check_and_cast<IPv6RouteRpl *>(rtTable->getRoute(i));
        if(r->getLifetime() >= 1 && r->getLifetime() != RPL_ROUTE_INFINITE_LIFETIME) {
            /*
             * If a route is at lifetime == 1, set it to 0, scheduling it for
             * immediate removal below. This achieves the same as the original code,
             * which would delete lifetime <= 1
             */
            r->setLifetime(r->getLifetime()-1);
        }
        if (r->getLifetime() < 1) {
            delEntries.push_back(r);
        }
    }


#if RPL_WITH_MULTICAST
  uip_mcast6_route_t *mcast_route;
#endif


  /* Second pass, remove dead routes */
  r = uip_ds6_route_head();

  for (auto elem : delEntries) {
      EV << "RPL: No more routes to " << elem->getDestPrefix();
      uip_ipaddr_t prefix(elem->getDestPrefix());
      rtTable->deleteRoute(elem);

      rpl_dag_t * dag = default_instance->current_dag;
      /* Propagate this information with a No-Path DAO to preferred parent if we are not a RPL Root */
      if(dag->rank != ROOT_RANK(default_instance)) {
          EV << " -> generate No-Path DAO\n";
          dao_output_target(dag->preferred_parent, &prefix, RPL_ZERO_LIFETIME);
          /* Don't schedule more than 1 No-Path DAO, let next iteration handle that */
          return;
      }
  }
#if RPL_WITH_MULTICAST
  mcast_route = uip_mcast6_route_list_head();

  while(mcast_route != NULL) {
    if(mcast_route->lifetime <= 1) {
      uip_mcast6_route_rm(mcast_route);
      mcast_route = uip_mcast6_route_list_head();
    } else {
      mcast_route->lifetime--;
      mcast_route = list_item_next(mcast_route);
    }
  }
#endif
}
/*---------------------------------------------------------------------------*/
void
RPL::rpl_remove_routes(rpl_dag_t *dag)
{
    for (int i = 0 ;i < rtTable->getNumRoutes();i++) {
        IPv6RouteRpl *r = check_and_cast<IPv6RouteRpl *>(rtTable->getRoute(i));
        if(r->getDag() == dag) {
            delEntries.push_back(r);
        }
    }
    for (auto elem : delEntries) {
        rtTable->deleteRoute(elem);
    }

#if RPL_WITH_MULTICAST
  mcast_route = uip_mcast6_route_list_head();

  while(mcast_route != NULL) {
    if(mcast_route->dag == dag) {
      uip_mcast6_route_rm(mcast_route);
      mcast_route = uip_mcast6_route_list_head();
    } else {
      mcast_route = list_item_next(mcast_route);
    }
  }
#endif
}
/*---------------------------------------------------------------------------*/
void
RPL::rpl_remove_routes_by_nexthop(uip_ipaddr_t *nexthop, rpl_dag_t *dag)
{
    for (int i = 0 ;i < rtTable->getNumRoutes();i++) {
        IPv6RouteRpl *r = check_and_cast<IPv6RouteRpl *>(rtTable->getRoute(i));
        if(r->getDag() == dag && r->getNextHopAsGeneric() == *nexthop) {
            r->setlifetime(0);
        }
    }

 /* uip_ds6_route_t *r;

  r = uip_ds6_route_head();

  while(r != NULL) {
    if(uip_ipaddr_cmp(uip_ds6_route_nexthop(r), nexthop) &&
        r->state.dag == dag) {
      r->state.lifetime = 0;
    }
    r = uip_ds6_route_next(r);
  }
  ANNOTATE("#L %u 0\n", nexthop->u8[sizeof(uip_ipaddr_t) - 1]);*/
}
/*---------------------------------------------------------------------------*/
uip_ds6_route_t *
RPL::rpl_add_route(rpl_dag_t *dag, uip_ipaddr_t *prefix, int prefix_len,
              uip_ipaddr_t *next_hop)
{
    IPv6RouteRpl *rep = new IPv6RouteRpl(*prefix,prefix_len,MANET2);
    rep->setNextHop(next_hop);


    rep->setDag(dag);
    rep->setLifetime(RPL_LIFETIME(dag->instance, dag->instance->default_lifetime));
    /* always clear state flags for the no-path received when adding/refreshing */
    RPL_ROUTE_CLEAR_NOPATH_RECEIVED(rep);

    rtTable->addRoute(r);
    return rep;
}

void
RPL::uip_ds6_set_addr_iid(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr)
{
  /* We consider only links with IEEE EUI-64 identifier or
   * IEEE 48-bit MAC addresses */
    uip_lladdr_t add =  lladdr->getEui64();
#if (UIP_LLADDR_LEN == 8)
  memcpy(ipaddr->u8 + 8, lladdr, UIP_LLADDR_LEN);
  ipaddr->u8[8] ^= 0x02;
#elif (UIP_LLADDR_LEN == 6)
  memcpy(ipaddr->u8 + 8, lladdr, 3);
  ipaddr->u8[11] = 0xff;
  ipaddr->u8[12] = 0xfe;
  memcpy(ipaddr->u8 + 13, (uint8_t *)lladdr + 3, 3);
  ipaddr->u8[8] ^= 0x02;
#else
#error uip-ds6.c cannot build interface address when UIP_LLADDR_LEN is not 6 or 8
#endif
}

/*---------------------------------------------------------------------------*/
void
RPL::rpl_link_neighbor_callback(const linkaddr_t *addr, int status, int numtx)
{
  uip_ipaddr_t ipaddr;
  rpl_parent_t *parent;
  rpl_instance_t *instance;
  rpl_instance_t *end;

  uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, (uip_lladdr_t *)addr);

  for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES; instance < end; ++instance) {
    if(instance->used == 1 ) {
      parent = rpl_find_parent_any_dag(instance, &ipaddr);
      if(parent != NULL) {
        /* Trigger DAG rank recalculation. */
        PRINTF("RPL: rpl_link_neighbor_callback triggering update\n");
        parent->flags |= RPL_PARENT_FLAG_UPDATED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
rpl_ipv6_neighbor_callback(uip_ds6_nbr_t *nbr)
{
  rpl_parent_t *p;
  rpl_instance_t *instance;
  rpl_instance_t *end;

  PRINTF("RPL: Neighbor state changed for ");
  PRINT6ADDR(&nbr->ipaddr);
#if UIP_ND6_SEND_NS || UIP_ND6_SEND_RA
  PRINTF(", nscount=%u, state=%u\n", nbr->nscount, nbr->state);
#else /* UIP_ND6_SEND_NS || UIP_ND6_SEND_RA */
  PRINTF(", state=%u\n", nbr->state);
#endif /* UIP_ND6_SEND_NS || UIP_ND6_SEND_RA */
  for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES; instance < end; ++instance) {
    if(instance->used == 1 ) {
      p = rpl_find_parent_any_dag(instance, &nbr->ipaddr);
      if(p != NULL) {
        p->rank = INFINITE_RANK;
        /* Trigger DAG rank recalculation. */
        PRINTF("RPL: rpl_ipv6_neighbor_callback infinite rank\n");
        p->flags |= RPL_PARENT_FLAG_UPDATED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
rpl_purge_dags(void)
{
  rpl_instance_t *instance;
  rpl_instance_t *end;
  int i;

  for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES;
      instance < end; ++instance) {
    if(instance->used) {
      for(i = 0; i < RPL_MAX_DAG_PER_INSTANCE; i++) {
        if(instance->dag_table[i].used) {
          if(instance->dag_table[i].lifetime == 0) {
            if(!instance->dag_table[i].joined) {
              PRINTF("RPL: Removing dag ");
              PRINT6ADDR(&instance->dag_table[i].dag_id);
              PRINTF("\n");
              rpl_free_dag(&instance->dag_table[i]);
            }
          } else {
            instance->dag_table[i].lifetime--;
          }
        }
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
rpl_init(void)
{
  uip_ipaddr_t rplmaddr;
  PRINTF("RPL: RPL started\n");
  default_instance = NULL;

  rpl_dag_init();
  rpl_reset_periodic_timer();
  rpl_icmp6_register_handlers();

  /* add rpl multicast address */
  uip_create_linklocal_rplnodes_mcast(&rplmaddr);
  uip_ds6_maddr_add(&rplmaddr);

#if RPL_CONF_STATS
  memset(&rpl_stats, 0, sizeof(rpl_stats));
#endif

#if RPL_WITH_NON_STORING
  rpl_ns_init();
#endif /* RPL_WITH_NON_STORING */
}
/*---------------------------------------------------------------------------*/

}

/** @}*/
