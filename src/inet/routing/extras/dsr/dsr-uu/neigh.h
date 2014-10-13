/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _NEIGH_H
#define _NEIGH_H

#include "dsr.h"

#ifndef NO_DECLS

int neigh_tbl_add(struct in_addr neigh_addr, struct ethhdr *ethh);

int neigh_tbl_del(struct in_addr neigh_addr);
int neigh_tbl_query(struct in_addr neigh_addr,
                    struct neighbor_info *neigh_info);
int neigh_tbl_id_inc(struct in_addr neigh_addr);
int neigh_tbl_set_rto(struct in_addr neigh_addr, struct neighbor_info *neigh_info);
int neigh_tbl_set_ack_req_time(struct in_addr neigh_addr);
void neigh_tbl_garbage_timeout(unsigned long data);

int neigh_tbl_init(void);
void neigh_tbl_cleanup(void);

#endif              /* NO_DECLS */

#endif              /* _NEIGH_H */
