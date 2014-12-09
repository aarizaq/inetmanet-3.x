/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_SRT_H
#define _DSR_SRT_H

#include "dsr.h"
#include "debug_dsr.h"

#ifdef NS2
#ifndef OMNETPP
#include "endian.h"
#else
#include "vector"
#endif
#endif

#ifndef NO_GLOBALS
#include "inet/routing/extras/dsr/dsr-uu/dsr_options.h"
namespace inet {

namespace inetmanet {

/* Source route options header */
/* TODO: This header is not byte order correct... is there a simple way to fix
 * it? */
/* Flags: */
#define SRT_FIRST_HOP_EXT 0x1
#define SRT_LAST_HOP_EXT  0x2

#define DSR_SRT_OPT_LEN(srt) (DSR_SRT_HDR_LEN + (srt->addrs.size()*DSR_ADDRESS_SIZE))

/* Flags */
#define SRT_BIDIR 0x1

/* Internal representation of a source route */

struct dsr_srt
{
    struct in_addr src;
    struct in_addr dst;
    unsigned short flags;
    unsigned short index;
    unsigned int laddrs;    /* length in bytes if addrs */
    std::vector<unsigned int> cost;
    VectorAddressIn addrs;  /* Intermediate nodes */
    dsr_srt &  operator= (const dsr_srt &m)
    {
        if (this==&m) return *this;
        src = m.src;
        dst = m.dst;
        flags = m.flags;
        index = m.index;
        laddrs = m.laddrs;
        addrs = m.addrs;
        cost = m.cost;
        return *this;
    }
    dsr_srt()
    {
        src.s_addr = 0;
        dst = src;
        flags = 0;
        index = 0;
        laddrs = 0;
        addrs.clear();
        cost.clear();
    }
    ~dsr_srt()
    {
        addrs.clear();
        cost.clear();
    }
};

static inline char *print_srt(struct dsr_srt *srt)
{
#define BUFLEN 256
    static char buf[BUFLEN];
    unsigned int len;

    if (!srt)
        return nullptr;

    len = sprintf(buf, "%s<->", print_ip(srt->src));

    for (unsigned int i = 0; i < (srt->addrs.size()) &&
            (len + 16) < BUFLEN; i++)
        len += sprintf(buf + len, "%s<->", print_ip(srt->addrs[i]));

    if ((len + 16) < BUFLEN)
        len = sprintf(buf + len, "%s", print_ip(srt->dst));
    return buf;
}
struct in_addr dsr_srt_next_hop(struct dsr_srt *srt, int sleft);
struct in_addr dsr_srt_prev_hop(struct dsr_srt *srt, int sleft);
struct dsr_srt_opt *dsr_srt_opt_add(struct dsr_opt_hdr *opt_hdr, int len, int flags, int salvage, struct dsr_srt *srt);
struct dsr_srt_opt *dsr_srt_opt_add_char(char *buffer, int len, int flags, int salvage, struct dsr_srt *srt);

struct dsr_srt *dsr_srt_new(struct in_addr,struct in_addr,unsigned int, const std::vector<uint32_t>&,const std::vector<EtxCost> & = std::vector<EtxCost>());
void dsr_srt_split_both(struct dsr_srt *,struct in_addr,struct in_addr,struct dsr_srt **,struct dsr_srt **);


struct dsr_srt *dsr_srt_new_rev(struct dsr_srt *srt);
void dsr_srt_del(struct dsr_srt *srt);
struct dsr_srt *dsr_srt_concatenate(struct dsr_srt *srt1, struct dsr_srt *srt2); int dsr_srt_check_duplicate(struct dsr_srt *srt);
struct dsr_srt *dsr_srt_new_split(struct dsr_srt *srt, struct in_addr addr);

} // namespace inetmanet

} // namespace inet
#endif              /* NO_GLOBALS */

#ifndef NO_DECLS

int dsr_srt_add(struct dsr_pkt *dp);
int dsr_srt_opt_recv(struct dsr_pkt *dp, struct dsr_srt_opt *srt_opt);

#endif              /* NO_DECLS */

#endif              /* _DSR_SRT_H */
