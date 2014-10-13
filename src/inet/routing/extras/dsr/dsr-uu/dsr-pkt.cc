/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#define OMNETPP

#ifdef __KERNEL_
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#endif

#ifndef OMNETPP
#ifdef NS2
#include "inet/routing/extras/dsr/dsr-uu/ns-agent.h"
#endif
#else
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#endif

#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-opt.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr.h"

namespace inet {

namespace inetmanet {

struct dsr_opt_hdr * dsr_pkt_alloc_opts(struct dsr_pkt *dp)
{
    if (!dp)
        return NULL;

    dp->dh.opth.resize(dp->dh.opth.size()+1);
    return &(dp->dh.opth.back());
}

void dsr_pkt_free_opts(struct dsr_pkt *dp)
{
    dp->dh.opth.clear();
}

#ifndef OMNETPP
#ifdef NS2
struct dsr_pkt *dsr_pkt_alloc(Packet * p)
{
    struct dsr_pkt *dp;
    struct hdr_cmn *cmh;
    int dsr_opts_len = 0;

    dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);

    if (!dp)
        return NULL;

    memset(dp, 0, sizeof(struct dsr_pkt));

    if (p)
    {
        cmh = hdr_cmn::access(p);

        dp->p = p;
        dp->mac.raw = p->access(hdr_mac::offset_);
        dp->nh.iph = HDR_IP(p);

        dp->src.s_addr =
            Address::getInstance().get_nodeaddr(dp->nh.iph->saddr());
        dp->dst.s_addr =
            Address::getInstance().get_nodeaddr(dp->nh.iph->daddr());

        if (cmh->ptype() == PT_DSR)
        {
            struct dsr_opt_hdr *opth;

            opth = hdr_dsr::access(p);

            dsr_opts_len = ntohs(opth->p_len) + DSR_OPT_HDR_LEN;

            if (!dsr_pkt_alloc_opts(dp, dsr_opts_len))
            {
                FREE(dp);
                return NULL;
            }

            memcpy(dp->dh.raw, (char *)opth, dsr_opts_len);

            dsr_opt_parse(dp);

            if ((DATA_PACKET(dp->dh.opth->nh) ||
                    dp->dh.opth->nh == PT_PING) &&
                    ConfVal(UseNetworkLayerAck))
                dp->flags |= PKT_REQUEST_ACK;
        }
        else if ((DATA_PACKET(cmh->ptype()) ||
                  cmh->ptype() == PT_PING) &&
                 ConfVal(UseNetworkLayerAck))
            dp->flags |= PKT_REQUEST_ACK;

        /* A trick to calculate payload length... */
        dp->payload_len = cmh->size() - dsr_opts_len - IP_HDR_LEN;
    }
    return dp;
}

#else

struct dsr_pkt *dsr_pkt_alloc(struct sk_buff *skb)
{
    struct dsr_pkt *dp;
    int dsr_opts_len = 0;

    dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);

    if (!dp)
        return NULL;

    memset(dp, 0, sizeof(struct dsr_pkt));

    if (skb)
    {
        /*  skb_unlink(skb); */

        dp->skb = skb;

        dp->mac.raw = skb->mac.raw;
        dp->nh.iph = skb->nh.iph;

        dp->src.s_addr = skb->nh.iph->saddr;
        dp->dst.s_addr = skb->nh.iph->daddr;

        if (dp->nh.iph->protocol == IPPROTO_DSR)
        {
            struct dsr_opt_hdr *opth;
            int n;

            opth = (struct dsr_opt_hdr *)(dp->nh.raw + (dp->nh.iph->ihl << 2));
            dsr_opts_len = ntohs(opth->p_len) + DSR_OPT_HDR_LEN;

            if (!dsr_pkt_alloc_opts(dp, dsr_opts_len))
            {
                FREE(dp);
                return NULL;
            }

            memcpy(dp->dh.raw, (char *)opth, dsr_opts_len);

            n = dsr_opt_parse(dp);

            DEBUG("Packet has %d DSR option(s)\n", n);
        }

        dp->payload = dp->nh.raw +
                      (dp->nh.iph->ihl << 2) + dsr_opts_len;

        dp->payload_len = ntohs(dp->nh.iph->tot_len) -
                          (dp->nh.iph->ihl << 2) - dsr_opts_len;

        if (dp->payload_len)
            dp->flags |= PKT_REQUEST_ACK;
    }
    return dp;
}

#endif

void dsr_pkt_free(struct dsr_pkt *dp)
{

    if (!dp)
        return;
#ifndef NS2
    if (dp->skb)
        dev_kfree_skb_any(dp->skb);
#endif
    dsr_pkt_free_opts(dp);

    if (dp->srt)
        FREE(dp->srt);

    FREE(dp);

    return;
}

#else

struct dsr_pkt * dsr_pkt::dup()
{
    struct dsr_pkt *dp;
    // int dsr_opts_len = 0;

    // dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);
    if (DSRUU::lifoDsrPkt!=NULL)
    {
        dp=DSRUU::lifoDsrPkt;
        DSRUU::lifoDsrPkt = dp->next;
        DSRUU::lifo_token++;
    }
    else
        dp = new dsr_pkt;

    if (!dp)
        return NULL;
    dp->clear();
    dp->mac.raw = dp->mac_data;
    dp->nh.iph = (struct iphdr *) dp->ip_data;
    memcpy(dp->mac_data,this->mac_data,sizeof(this->mac_data));
    memcpy(dp->ip_data,this->ip_data,sizeof(this->ip_data));

    dp->src.s_addr = this->src.s_addr;
    dp->dst.s_addr = this->dst.s_addr;

    dp->moreFragments = this->moreFragments;
    dp->fragmentOffset = this->moreFragments;

    dp->totalPayloadLength = this->totalPayloadLength;

    dp->payload_len = this->payload_len;
    if (this->payload)
        dp->payload = this->payload->dup();

    if (this->srt)
    {
        dp->srt = new dsr_srt;
        *dp->srt = *this->srt;
    }

    dp->encapsulate_protocol = this->encapsulate_protocol;
    dp->dh.opth = this->dh.opth;
    dp->costVector = this->costVector;
    dsr_opt_parse(dp);
    dp->flags = this->flags;
    return dp;
}

dsr_pkt * dsr_pkt_alloc(cPacket  * p)
{
    struct dsr_pkt *dp;
   // int dsr_opts_len = 0;

    // dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);
    if (DSRUU::lifoDsrPkt!=NULL)
    {
        dp=DSRUU::lifoDsrPkt;
        DSRUU::lifoDsrPkt = dp->next;
        DSRUU::lifo_token++;
    }
    else
        dp = new dsr_pkt;

    if (!dp)
        return NULL;
    dp->clear();
    if (p)
    {
        IPv4Datagram *dgram = dynamic_cast <IPv4Datagram *> (p);
        dp->encapsulate_protocol=0;
        dp->mac.raw = dp->mac_data;
        cObject * ctrl = dgram->removeControlInfo();
        if (ctrl!=NULL)
        {
            Ieee802Ctrl * ctrlmac = check_and_cast<Ieee802Ctrl *> (ctrl);
            ctrlmac->getDest().getAddressBytes(dp->mac.ethh->h_dest);    /* destination eth addr */
            ctrlmac->getSrc().getAddressBytes(dp->mac.ethh->h_source);   /* destination eth addr */
            delete ctrl;
        }

        // IPv4Address dest = dgram->getDestAddress();
        // IPv4Address src = dgram->getSrcAddress();

        dp->src.s_addr = dgram->getSrcAddress().getInt();
        dp->dst.s_addr =dgram->getDestAddress().getInt();
        dp->nh.iph = (struct iphdr *) dp->ip_data;
        dp->nh.iph->ihl= dgram->getHeaderLength(); // Header length
        dp->nh.iph->version= dgram->getVersion(); // Ip version
        dp->nh.iph->tos= dgram->getTypeOfService(); // ToS
        dp->nh.iph->tot_len= dgram->getByteLength(); // Total length
        dp->nh.iph->id= dgram->getIdentification(); // Identification
        dp->nh.iph->frag_off= 0x1FFF & dgram->getFragmentOffset(); //
        if (dgram->getMoreFragments())
            dp->nh.iph->frag_off |= 0x2000;
        if (dgram->getDontFragment())
            dp->nh.iph->frag_off |= 0x4000;

        dp->moreFragments=dgram->getMoreFragments();
        dp->fragmentOffset=dgram->getFragmentOffset();

        dp->nh.iph->ttl= dgram->getTimeToLive(); // TTL
        dp->nh.iph->protocol= dgram->getTransportProtocol(); // Transport protocol
        // dp->nh.iph->check= p->;                          // Check sum
        dp->nh.iph->saddr= dgram->getSrcAddress().getInt();
        dp->nh.iph->daddr= dgram->getDestAddress().getInt();
#ifdef NEWFRAGMENT
        dp->totalPayloadLength = dgram->getTotalPayloadLength();
#endif
        //if (dgram->getFragmentOffset()==0 && !dgram->getMoreFragments())
        dp->payload = p->decapsulate();
        //else
        //  dp->payload = NULL;
        dp->encapsulate_protocol = 0;

        if (dp->nh.iph->protocol == IP_PROT_DSR)
        {
            int n;
            if (dynamic_cast<DSRPkt*> (p))
            {
                DSRPkt * dsrpkt = dynamic_cast<DSRPkt*> (p);

                dp->dh.opth = dsrpkt->getOptions();
                dsrpkt->clearOptions();
                // dsr_opts_len = dp->dh.opth.begin()->p_len + DSR_OPT_HDR_LEN;

                if (dp->payload)
                    dp->encapsulate_protocol=dsrpkt->getEncapProtocol();

                n = dsr_opt_parse(dp);
                DEBUG("Packet has %d DSR option(s)\n", n);
                dp->ip_pkt = dsrpkt;
                dp->costVector = dsrpkt->getCostVector();

                dsrpkt->resetCostVector();
                p=NULL;
            }
        }
        else
        {
            if (dp->payload)
                dp->encapsulate_protocol = dgram->getTransportProtocol();
        }

        if (dp->payload)
            dp->payload_len = dp->payload->getByteLength();
        if (dp->payload_len && ConfVal(UseNetworkLayerAck))
            dp->flags |= PKT_REQUEST_ACK;

    }
    if (p)
    {
        delete p;
        p = NULL;
    }
    return dp;
}


dsr_pkt * dsr_pkt_alloc2(cPacket  * p, cObject *ctrl)
{
    struct dsr_pkt *dp;
   // int dsr_opts_len = 0;


    // dp = (struct dsr_pkt *)MALLOC(sizeof(struct dsr_pkt), GFP_ATOMIC);
    if (DSRUU::lifoDsrPkt!=NULL)
    {
        dp=DSRUU::lifoDsrPkt;
        DSRUU::lifoDsrPkt = dp->next;
        DSRUU::lifo_token++;
    }
    else
        dp = new dsr_pkt;

    if (!dp)
        return NULL;
    dp->clear();

    if (p)
    {
        IPv4Datagram *dgram = dynamic_cast <IPv4Datagram *> (p);
        dp->encapsulate_protocol=0;
        dp->mac.raw = dp->mac_data;

        dp->src.s_addr = dgram->getSrcAddress().getInt();
        dp->dst.s_addr =dgram->getDestAddress().getInt();
        dp->nh.iph = (struct iphdr *) dp->ip_data;
        dp->nh.iph->ihl= dgram->getHeaderLength(); // Header length
        dp->nh.iph->version= dgram->getVersion(); // Ip version
        dp->nh.iph->tos= dgram->getTypeOfService(); // ToS
        dp->nh.iph->tot_len= dgram->getByteLength(); // Total length
        dp->nh.iph->id= dgram->getIdentification(); // Identification
        dp->nh.iph->frag_off= 0x1FFF & dgram->getFragmentOffset(); //
        if (dgram->getMoreFragments())
            dp->nh.iph->frag_off |= 0x2000;
        if (dgram->getDontFragment())
            dp->nh.iph->frag_off |= 0x4000;

        if (ctrl!=NULL)
        {
            Ieee802Ctrl * ctrlmac = check_and_cast<Ieee802Ctrl *> (ctrl);
            ctrlmac->getDest().getAddressBytes(dp->mac.ethh->h_dest);    /* destination eth addr */
            ctrlmac->getSrc().getAddressBytes(dp->mac.ethh->h_source);   /* destination eth addr */
            delete ctrl;
        }

        dp->moreFragments=dgram->getMoreFragments();
        dp->fragmentOffset=dgram->getFragmentOffset();

        dp->nh.iph->ttl= dgram->getTimeToLive(); // TTL
        dp->nh.iph->protocol= dgram->getTransportProtocol(); // Transport protocol
        // dp->nh.iph->check= p->;                          // Check sum
        dp->nh.iph->saddr= dgram->getSrcAddress().getInt();
        dp->nh.iph->daddr= dgram->getDestAddress().getInt();
#ifdef NEWFRAGMENT
        dp->totalPayloadLength = dgram->getTotalPayloadLength();
#endif

        if (dp->nh.iph->protocol == IP_PROT_DSR)
        {
            //struct dsr_opt_hdr *opth;
            int n;
            if (dynamic_cast<DSRPkt*> (p))
            {
                DSRPkt * dsrpkt = dynamic_cast<DSRPkt*> (p);
                dp->dh.opth = dsrpkt->getOptions();
                dsrpkt->clearOptions();
                // dsr_opts_len = dp->dh.opth.begin()->p_len + DSR_OPT_HDR_LEN;
                dp->dh.opth = dsrpkt->getOptions();

                if (dp->payload)
                    dp->encapsulate_protocol=dsrpkt->getEncapProtocol();
                n = dsr_opt_parse(dp);
                DEBUG("Packet has %d DSR option(s)\n", n);
                dp->costVector = dsrpkt->getCostVector();
                dsrpkt->resetCostVector();
            }
        }
    }
    return dp;
}

void dsr_pkt_free(dsr_pkt *dp)
{

    if (!dp)
        return;
    dsr_pkt_free_opts(dp);

    if (dp->srt)
        delete dp->srt;

    if (dp->payload)
        delete dp->payload;

    if (dp->ip_pkt)
        delete dp->ip_pkt;

    if (!dp->costVector.empty())
           dp->costVector.clear();

    if (!dp->dh.opth.empty())
        dp->dh.opth.clear();

    if (DSRUU::lifo_token>0)
    {
        DSRUU::lifo_token--;
        dp->next=DSRUU::lifoDsrPkt;
        DSRUU::lifoDsrPkt=dp;
    }
    else
        delete dp;
    dp=NULL;
    return;

}

} // namespace inetmanet

} // namespace inet

#endif
