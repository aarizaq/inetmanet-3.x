/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
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
 * Author: Kirill Andreev <andreev@iitp.ru>
 *
 * adapted to inetmanet 2011, by Alfonso Ariza
 */

#include "hwmp-rtable.h"

HwmpRtable::HwmpRtable()
{
    DeleteProactivePath();
}
HwmpRtable::~HwmpRtable()
{
    DeleteProactivePath();
    m_routes.clear();
}

void HwmpRtable::AddReactivePath(MACAddress destination, MACAddress retransmitter, uint32_t interface, uint32_t metric,
        simtime_t lifetime, uint32_t seqnum, uint8_t hops, bool actualizeSeqnum)
{
    //uint64_t dest= MacToUint64(destination);
    std::map<MACAddress, ReactiveRoute>::iterator i = m_routes.find(destination);
    if (i == m_routes.end())
    {
        ReactiveRoute newroute;
        m_routes[destination] = newroute;
    }
    i = m_routes.find(destination);
    ASSERT(i != m_routes.end());
    i->second.retransmitter = retransmitter;
    i->second.interface = interface;
    i->second.metric = metric;
    i->second.whenExpire = simTime() + lifetime;
    if (actualizeSeqnum)
        i->second.seqnum = seqnum;
    i->second.hops = hops;
}

void HwmpRtable::AddProactivePath(uint32_t metric, MACAddress root, MACAddress retransmitter, uint32_t interface,
        simtime_t lifetime, uint32_t seqnum, uint8_t hops)
{
    m_root.root = root;
    m_root.retransmitter = retransmitter;
    m_root.metric = metric;
    m_root.whenExpire = simTime() + lifetime;
    m_root.seqnum = seqnum;
    m_root.interface = interface;
    m_root.hops = hops;
}

void HwmpRtable::AddPrecursor(MACAddress destination, uint32_t precursorInterface, MACAddress precursorAddress,
        simtime_t lifetime)
{
    Precursor precursor;
    precursor.interface = precursorInterface;
    precursor.address = precursorAddress;
    precursor.whenExpire = simTime() + lifetime;
    // uint64_t dest= MacToUint64(destination);
    std::map<MACAddress, ReactiveRoute>::iterator i = m_routes.find(destination);
    if (i != m_routes.end())
    {
        bool should_add = true;
        for (unsigned int j = 0; j < i->second.precursors.size(); j++)
        {
            //NB: Only one active route may exist, so do not check
            //interface ID, just address
            if (i->second.precursors[j].address == precursorAddress)
            {
                should_add = false;
                i->second.precursors[j].whenExpire = precursor.whenExpire;
                break;
            }
        }
        if (should_add)
        {
            i->second.precursors.push_back(precursor);
        }
    }
}

void HwmpRtable::DeleteProactivePath()
{
    m_root.precursors.clear();
    m_root.interface = INTERFACE_ANY;
    m_root.metric = MAX_METRIC;
    m_root.retransmitter = (MACAddress) MACAddress::UNSPECIFIED_ADDRESS;
    m_root.seqnum = 0;
    m_root.hops = MAX_HOPS;
    m_root.whenExpire = simTime();
}

void HwmpRtable::DeleteProactivePath(MACAddress root)
{
    if (m_root.root == root)
    {
        DeleteProactivePath();
    }
}

void HwmpRtable::DeleteReactivePath(MACAddress destination)
{
    // uint64_t dest= MacToUint64(destination);
    std::map<MACAddress, ReactiveRoute>::iterator i = m_routes.find(destination);
    if (i != m_routes.end())
    {
        m_routes.erase(i);
    }
}

HwmpRtable::LookupResult HwmpRtable::LookupReactive(MACAddress destination)
{
    // uint64_t dest= MacToUint64(destination);
    std::map<MACAddress, ReactiveRoute>::iterator i = m_routes.find(destination);
    if (i == m_routes.end())
    {
        return LookupResult();
    }
    if ((i->second.whenExpire < simTime()) && (i->second.whenExpire != 0))
    {
        EV << "Reactive route has expired, sorry." << endl;
        return LookupResult();
    }
    return LookupReactiveExpired(destination);
}

HwmpRtable::LookupResult HwmpRtable::LookupReactiveExpired(MACAddress destination)
{
    // uint64_t dest= MacToUint64(destination);
    std::map<MACAddress, ReactiveRoute>::iterator i = m_routes.find(destination);
    if (i == m_routes.end())
    {
        return LookupResult();
    }
    if (i->second.whenExpire <= simTime())
        return LookupResult(i->second.retransmitter, i->second.interface, i->second.metric, i->second.seqnum, 0);
    return LookupResult(i->second.retransmitter, i->second.interface, i->second.metric, i->second.seqnum,
            i->second.whenExpire - simTime());
}

HwmpRtable::LookupResult HwmpRtable::LookupProactive()
{
    if (m_root.whenExpire < simTime())
    {
        EV << "Proactive route has expired and will be deleted, sorry." << endl;
        DeleteProactivePath();
    }
    return LookupProactiveExpired();
}

HwmpRtable::LookupResult HwmpRtable::LookupProactiveExpired()
{
    if (m_root.whenExpire <= simTime())
        return LookupResult(m_root.retransmitter, m_root.interface, m_root.metric, m_root.seqnum, 0);
    return LookupResult(m_root.retransmitter, m_root.interface, m_root.metric, m_root.seqnum,
            m_root.whenExpire - simTime());
}

std::vector<HwmpFailedDestination> HwmpRtable::GetUnreachableDestinations(MACAddress peerAddress)
{
    HwmpFailedDestination dst;
    std::vector < HwmpFailedDestination > retval;
    for (std::map<MACAddress, ReactiveRoute>::iterator i = m_routes.begin(); i != m_routes.end(); i++)
    {
        if (i->second.retransmitter == peerAddress)
        {
            dst.destination = i->first; // Uint64ToMac(i->first);
            i->second.seqnum++;
            dst.seqnum = i->second.seqnum;
            bool notFind = true;
            for (unsigned int i = 0; i < retval.size(); i++)
            {
                if (retval[i].destination == dst.destination)
                    notFind = false;
            }
            if (notFind)
                retval.push_back(dst);
        }
    }
    //Lookup a path to root
    if (m_root.retransmitter == peerAddress)
    {
        dst.destination = m_root.root;
        dst.seqnum = m_root.seqnum;
        bool notFind = true;
        for (unsigned int i = 0; i < retval.size(); i++)
        {
            if (retval[i].destination == dst.destination)
                notFind = false;
        }
        if (notFind)
            retval.push_back(dst);
    }
    return retval;
}

HwmpRtable::PrecursorList HwmpRtable::GetPrecursors(MACAddress destination)
{
    //We suppose that no duplicates here can be
    PrecursorList retval;
    // uint64_t dest= MacToUint64(destination);
    std::map<MACAddress, ReactiveRoute>::iterator route = m_routes.find(destination);
    if (route != m_routes.end())
    {
        for (std::vector<Precursor>::const_iterator i = route->second.precursors.begin();
                i != route->second.precursors.end(); i++)
        {
            if (i->whenExpire > simTime())
            {
                retval.push_back(std::make_pair(i->interface, i->address));
            }
        }
    }
    return retval;
}

bool HwmpRtable::LookupResult::operator==(const HwmpRtable::LookupResult & o) const
{
    return (retransmitter == o.retransmitter && ifIndex == o.ifIndex && metric == o.metric && seqnum == o.seqnum
            && hops == o.hops);
}

HwmpRtable::ReactiveRoute *
HwmpRtable::getLookupReactivePtr(MACAddress destination)
{
    // uint64_t dest= MacToUint64(destination);
    if (destination.isUnspecified())
        return NULL;
    std::map<MACAddress, ReactiveRoute>::iterator i = m_routes.find(destination);
    if (i == m_routes.end())
        return NULL;
    if ((i->second.whenExpire < simTime()) && (i->second.whenExpire != 0))
        return NULL;
    return &(i->second);
}

HwmpRtable::ProactiveRoute *
HwmpRtable::getLookupProactivePtr()
{
    if (m_root.whenExpire <= simTime())
        return NULL;
    return &m_root;
}

void HwmpRtable::deleteNeighborRoutes(MACAddress nextHop)
{

    for (std::map<MACAddress, ReactiveRoute>::iterator it = m_routes.begin(); it != m_routes.end();)
    {
        if (it->second.retransmitter == nextHop)
        {
            std::map<MACAddress, ReactiveRoute>::iterator aux = it;
            it++;
            m_routes.erase(aux);
        }
        else
            it++;
    }
}
