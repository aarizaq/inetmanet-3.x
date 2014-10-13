/*****************************************************************************
 *
 * Copyright (C) 2002 Uppsala University.
 * Copyright (C) 2007 Malaga University.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Erik Nordstr� <erik.nordstrom@it.uu.se>
 * Authors: Alfonso Ariza Quintana.<aarizaq@uma.ea>
 *
 *****************************************************************************/
#include "inet/routing/extras/dsr/dsr-pkt_omnet.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"

namespace inet {

namespace inetmanet {

#define SIZE_COST_BITS 16
DSRPkt::~DSRPkt()
{
    clean();
}

void DSRPkt::clean()
{
    options.clear();
    costVector.clear();
    options.clear();
}

DSRPkt::DSRPkt(const DSRPkt& m) : IPv4Datagram(m)
{
    costVector.clear();
    options.clear();
    copy(m);
}

DSRPkt& DSRPkt::operator=(const DSRPkt& m)
{
    if (this==&m) return *this;
    clean();
    IPv4Datagram::operator=(m);
    copy(m);
    return *this;
}

void DSRPkt::copy(const DSRPkt& m)
{
    encap_protocol = m.encap_protocol;
    previous = m.previous;
    next = m.next;
    options = m.options;
    costVector = m.costVector;
}
// Constructor
DSRPkt::DSRPkt(struct dsr_pkt *dp, int interface_id) : IPv4Datagram()
{
    costVector.clear();
    options.clear();

    setEncapProtocol((IPProtocolId)0);

    if (dp)
    {
        IPv4Address destAddress_var((uint32_t)dp->dst.s_addr);
        setDestAddress(destAddress_var);
        IPv4Address srcAddress_var((uint32_t)dp->src.s_addr);
        setSrcAddress(srcAddress_var);
        setHeaderLength(dp->nh.iph->ihl); // Header length
        setVersion(dp->nh.iph->version); // Ip version
        setTypeOfService(dp->nh.iph->tos); // ToS
        setIdentification(dp->nh.iph->id); // Identification
        setMoreFragments(dp->nh.iph->frag_off & 0x2000);
        setDontFragment(dp->nh.iph->frag_off & 0x4000);
        setTimeToLive(dp->nh.iph->ttl); // TTL
        setTransportProtocol(IP_PROT_DSR); // Transport protocol
        setBitLength(getHeaderLength()*8);
        // ¿como gestionar el MAC
        // dp->mac.raw = p->access(hdr_mac::offset_);


        options = dp->dh.opth;
        //int dsr_opts_len = opth->p_len + DSR_OPT_HDR_LEN;
        setBitLength(getBitLength()+((DSR_OPT_HDR_LEN+options.begin()->p_len)*8));
        setHeaderLength(getByteLength());
#ifdef NEWFRAGMENT
        setTotalPayloadLength(dp->totalPayloadLength);
#endif
        if (dp->payload)
        {
            encapsulate(dp->payload);
            dp->payload = NULL;
            setEncapProtocol((IPProtocolId)dp->encapsulate_protocol);

        }
        if (interface_id>=0)
        {
            IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
            //ipControlInfo->setProtocol(IP_PROT_UDP);
            ipControlInfo->setProtocol(IP_PROT_DSR);
            ipControlInfo->setInterfaceId(interface_id); // If broadcast packet send to interface
            ipControlInfo->setSrcAddr(srcAddress_var);
            ipControlInfo->setDestAddr(destAddress_var);
            ipControlInfo->setTimeToLive(dp->nh.iph->ttl);
            setControlInfo(ipControlInfo);
        }
        if (dp->costVector.size()>0)
        {
            setCostVector(dp->costVector);
            dp->costVector.clear();
        }
    }
}

void DSRPkt::ModOptions(struct dsr_pkt *dp, int interface_id)
{
    setEncapProtocol((IPProtocolId)0);
    if (dp)
    {
        IPv4Address destAddress_var((uint32_t)dp->dst.s_addr);
        setDestAddress(destAddress_var);
        IPv4Address srcAddress_var((uint32_t)dp->src.s_addr);
        setSrcAddress(srcAddress_var);
        // ¿como gestionar el MAC
        // dp->mac.raw = p->access(hdr_mac::offset_);
        setHeaderLength(dp->nh.iph->ihl); // Header length
        setVersion(dp->nh.iph->version); // Ip version
        setTypeOfService(dp->nh.iph->tos); // ToS
        setIdentification(dp->nh.iph->id); // Identification

        setMoreFragments(dp->nh.iph->frag_off & 0x2000);
        setDontFragment(dp->nh.iph->frag_off & 0x4000);

        setTimeToLive(dp->nh.iph->ttl); // TTL
        setTransportProtocol(IP_PROT_DSR); // Transport protocol
        options.clear();

        options = dp->dh.opth;

        setBitLength((DSR_OPT_HDR_LEN+IP_HDR_LEN+options.front().p_len)*8);

        if (dp->payload)
        {
            cPacket *msg = this->decapsulate();
            if (msg)
                delete msg;
            encapsulate(dp->payload);
            dp->payload = NULL;
            setEncapProtocol((IPProtocolId)dp->encapsulate_protocol);

        }

        if (interface_id>=0)
        {
            IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
            //ipControlInfo->setProtocol(IP_PROT_UDP);
            ipControlInfo->setProtocol(IP_PROT_DSR);

            ipControlInfo->setInterfaceId(interface_id); // If broadcast packet send to interface

            ipControlInfo->setSrcAddr(srcAddress_var);
            ipControlInfo->setDestAddr(destAddress_var);
            ipControlInfo->setTimeToLive(dp->nh.iph->ttl);
            setControlInfo(ipControlInfo);
        }
        costVector.clear();

        if (dp->costVector.size() > 0)
        {
            setCostVector(dp->costVector);
            dp->costVector.clear();
        }

    }
}



std::string DSRPkt::detailedInfo() const
{
    std::stringstream out;
    struct dsr_opt *dopt;
    int l = DSR_OPT_HDR_LEN;
    out << " DSR Options "  << "\n"; // Khmm...

    for (unsigned int i = 0; i < options.size(); i++)
    {
        for (unsigned int j = 0; j < options[i].option.size(); j++)
        {
            dopt = options[i].option[j];
            //DEBUG("dsr_len=%d l=%d\n", dsr_len, l);
            switch (dopt->type)
            {
                case DSR_OPT_PADN:
                    out << " DSR_OPT_PADN " << "\n"; // Khmm...
                    break;
                case DSR_OPT_RREQ:
                {
                    out << " DSR_OPT_RREQ " << "\n"; // Khmm...
                    dsr_rreq_opt *rreq_opt = check_and_cast<dsr_rreq_opt*>(dopt);
                    IPv4Address add(rreq_opt->target);
                    out << " Target :" << add << "\n"; // Khmm
                    for (unsigned int m = 0; m < rreq_opt->addrs.size(); m++)
                    {
                        IPv4Address add(rreq_opt->addrs[m]);
                        out << add << "\n"; // Khmm
                    }
                }
                    break;
                case DSR_OPT_RREP:
                {
                    out << " DSR_OPT_RREP " << "\n"; // Khmm...Q
                    dsr_rrep_opt *rrep_opt = check_and_cast<dsr_rrep_opt*>(dopt);
                    for (unsigned int m = 0; m < rrep_opt->addrs.size(); m++)
                    {
                        IPv4Address add(rrep_opt->addrs[m]);
                        out << add << "\n"; // Khmm
                    }

                }
                    break;
                case DSR_OPT_RERR:
                    out << " DSR_OPT_RERR " << "\n"; // Khmm...

                    break;
                case DSR_OPT_PREV_HOP:
                    out << " DSR_OPT_PREV_HOP " << "\n"; // Khmm...
                    break;
                case DSR_OPT_ACK:
                    out << " DSR_OPT_ACK " << "\n"; // Khmm...
                    break;
                case DSR_OPT_SRT:
                {
                    out << " DSR_OPT_SRT " << "\n"; // Khmm...
                    dsr_srt_opt *srt_opt = check_and_cast<dsr_srt_opt*>(dopt);
                    out << "next hop : " << next << "  previous : " << previous << "\n Route \n";
                    for (unsigned int j = 0; j < srt_opt->addrs.size(); j++)
                    {
                        IPv4Address add(srt_opt->addrs[j]);
                        out << add << "\n"; // Khmm
                    }
                }
                    break;
                case DSR_OPT_TIMEOUT:
                    out << " DSR_OPT_TIMEOUT " << "\n"; // Khmm...
                    break;
                case DSR_OPT_FLOWID:
                    out << " DSR_OPT_FLOWID " << "\n"; // Khmm...
                    break;
                case DSR_OPT_ACK_REQ:
                    out << " DSR_OPT_ACK_REQ " << "\n"; // Khmm...
                    break;
                case DSR_OPT_PAD1:
                    out << " DSR_OPT_PAD1 " << "\n"; // Khmm...
                    l++;
                    dopt++;
                    continue;
                default:
                    out << " Unknown DSR option type " << "\n"; // Khmm...
            }
        }
    }
    return out.str();
}





void DSRPkt::getCostVector(std::vector<EtxCost> &cost) // Copy
{
    cost = costVector;
}

// The pointer *cost is the new pointer
void DSRPkt::setCostVector(std::vector<EtxCost> &cost)
{
    if (costVector.size() > 0)
    {
        setBitLength(getBitLength()-(costVector.size() * SIZE_COST_BITS));
        costVector.clear();
    }
    costVector = cost;
    setBitLength(getBitLength()+(costVector.size()*SIZE_COST_BITS));
}


void DSRPkt::setCostVectorSize(EtxCost newLinkCost)
{
    setBitLength(getBitLength()+SIZE_COST_BITS);
    costVector.push_back(newLinkCost);
}

void DSRPkt::setCostVectorSize(u_int32_t addr, double cost)
{
    EtxCost newLinkCost;
    IPv4Address address(addr);
    newLinkCost.address = address;
    newLinkCost.cost = cost;
    setCostVectorSize(newLinkCost);
}

void DSRPkt::resetCostVector()
{
    setBitLength(getBitLength()-(costVector.size()*SIZE_COST_BITS));
    costVector.clear();
}

DSRPktExt::DSRPktExt(const DSRPktExt& m) : IPv4Datagram(m)
{
    copy(m);
}


DSRPktExt& DSRPktExt::operator=(const DSRPktExt& msg)
{
    if (this==&msg) return *this;
    clean();
    IPv4Datagram::operator=(msg);
    copy(msg);
    return *this;
}

void DSRPktExt::copy(const DSRPktExt& msg)
{
    size = msg.size;
    if (size==0)
    {
        extension = NULL;
        return;
    }
    extension = new EtxList[size];
    memcpy(extension, msg.extension, size*sizeof(EtxList));
}

void DSRPktExt:: clearExtension()
{
    if (size==0)
    {
        return;
    }
    delete [] extension;
    size = 0;
}

DSRPktExt::~DSRPktExt()
{
    clearExtension();
}

EtxList * DSRPktExt::addExtension(int len)
{
    EtxList * extension_aux;
    if (len<0)
    {
        return NULL;
    }
    extension_aux = new EtxList [size+len];
    memcpy(extension_aux, extension, size*sizeof(EtxList));
    delete [] extension;
    extension = extension_aux;
    size += len;
    setBitLength(getBitLength()+(len*8*8)); // 4 ip-address 4 cost
    return extension;
}

EtxList * DSRPktExt::delExtension(int len)
{
    EtxList * extension_aux;
    if (len<0)
    {
        return NULL;
    }
    if (size-len<=0)
    {
        delete [] extension;
        extension = NULL;
        size = 0;
        return extension;
    }

    extension_aux = new EtxList [size-len];
    memcpy(extension_aux, extension, (size-len)*sizeof(EtxList));
    delete [] extension;
    extension = extension_aux;

    size -= len;
    setBitLength(getBitLength()-(len*8*8));
    return extension;
}

} // namespace inetmanet

} // namespace inet

