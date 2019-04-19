/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_PKT_H
#define _DSR_PKT_H

#ifndef OMNETPP

#ifdef NS2
#include <packet.h>
#include <ip.h>
#else
#include <linux/in.h>
#endif
#else

#include <omnetpp.h>
#include <vector>
#include "compatibility_dsr.h"

class DSRPkt;
class EtxCost;
#undef NS2

#endif

#define MAX_RREP_OPTS 10
#define MAX_RERR_OPTS 10
#define MAX_ACK_OPTS  10

#define DEFAULT_TAILROOM 128

/* Internal representation of a packet. For portability */
struct dsr_pkt
{
    struct in_addr src; /* IP level data */
    struct in_addr dst;
    struct in_addr nxt_hop;
    struct in_addr prv_hop;
    int flags;
    int salvage;
    int numRetries;
#ifdef NS2
    union
    {
        struct hdr_mac *ethh;
        unsigned char *raw;
    } mac;
    struct hdr_ip ip_data;
    union
    {
        struct hdr_ip *iph;
        char *raw;
    } nh;
#else
    union
    {
        struct ethhdr *ethh;
        char *raw;
    } mac;
#ifdef OMNETPP
    char mac_data[16];
#endif
    union
    {
        struct iphdr *iph;
        char *raw;
    } nh;
    char ip_data[70];
#endif
    struct
    {
         std::vector<struct dsr_opt_hdr>opth;
    } dh;

    struct dsr_srt_opt *srt_opt;
    std::vector<struct dsr_rreq_opt *> rreq_opt;  /* Can only be one */
    std::vector<struct dsr_rrep_opt *> rrep_opt;
    std::vector<struct dsr_rerr_opt *> rerr_opt;
    std::vector<struct dsr_ack_opt *> ack_opt;
    struct dsr_ack_req_opt *ack_req_opt;
    struct dsr_srt *srt;    /* Source route */
    int payload_len;
#ifndef OMNETPP
#ifdef NS2
    AppData *payload;
    Packet *p;
#else
    char *payload;
    struct sk_buff *skb;
#endif
#else
    bool moreFragments;
    int fragmentOffset;
    int totalPayloadLength;

    cPacket *payload;
    DSRPkt   *ip_pkt;
    int encapsulate_protocol;
    // Etx cost

    std::vector<EtxCost> costVector;
    struct dsr_pkt * next;
#endif
    void clear()
    {
        costVector.clear();
        dh.opth.clear();
        src.s_addr = 0; /* IP level data */
        dst = nxt_hop = prv_hop = src;
        flags = salvage = numRetries = 0;
        mac.raw = NULL;
        memset(mac_data,0,sizeof(mac_data));
        nh.raw = NULL;
        memset(ip_data,0,sizeof(ip_data));
        srt_opt = NULL;
        ack_req_opt = NULL;
        srt = NULL;
        srt_opt = NULL;
        payload_len = 0;
        moreFragments = false;
        fragmentOffset = totalPayloadLength = 0;
        payload = NULL;
        ip_pkt = NULL;
        encapsulate_protocol = 0;
        next = NULL;

        rreq_opt.clear();  /* Can only be one */
        rrep_opt.clear();
        rerr_opt.clear();
        ack_opt.clear();

    }
    struct dsr_pkt *dup();
};


/* Flags: */
#define PKT_PROMISC_RECV 0x01
#define PKT_REQUEST_ACK  0x02
#define PKT_PASSIVE_ACK  0x04
#define PKT_XMIT_JITTER  0x08

/* Packet actions: */
#define DSR_PKT_NONE           1
#define DSR_PKT_SRT_REMOVE     (DSR_PKT_NONE << 2)
#define DSR_PKT_SEND_ICMP      (DSR_PKT_NONE << 3)
#define DSR_PKT_SEND_RREP      (DSR_PKT_NONE << 4)
#define DSR_PKT_SEND_BUFFERED  (DSR_PKT_NONE << 5)
#define DSR_PKT_SEND_ACK       (DSR_PKT_NONE << 6)
#define DSR_PKT_FORWARD        (DSR_PKT_NONE << 7)
#define DSR_PKT_FORWARD_RREQ   (DSR_PKT_NONE << 8)
#define DSR_PKT_DROP           (DSR_PKT_NONE << 9)
#define DSR_PKT_ERROR          (DSR_PKT_NONE << 10)
#define DSR_PKT_DELIVER        (DSR_PKT_NONE << 11)
#define DSR_PKT_ACTION_LAST    (12)

struct dsr_pkt *dsr_pkt_alloc(cPacket *p);
struct dsr_pkt * dsr_pkt_alloc2(cPacket  * p, cObject *ctrl);


struct dsr_opt_hdr * dsr_pkt_alloc_opts(struct dsr_pkt *dp);
//struct dsr_opt_hdr * dsr_pkt_alloc_opts_expand(struct dsr_pkt *dp, int len);
void dsr_pkt_free(struct dsr_pkt *dp);
void dsr_pkt_free_opts(struct dsr_pkt *dp);

#endif              /* _DSR_PKT_H */
