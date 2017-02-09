/*
 * Ieee80211MsduAContainer.cc
 *
 *  Created on: 3 feb. 2017
 *      Author: Alfonso
 */
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/linklayer/ieee80211/mac/Ieee80211MsduAContainer.h"


namespace inet {
namespace ieee80211 {

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out, const T&)
{
    return out;
}

Ieee80211MsduAContainer::~Ieee80211MsduAContainer()
{
    // TODO Auto-generated destructor stub
    _deleteEncapVector();
}

Register_Class(Ieee80211MsduAContainer);

Ieee80211MsduAContainer::Ieee80211MsduAContainer(const char *name, int kind) :
        Ieee80211MeshFrame(name, kind)
{
    encapsulateVector.clear();
}

Ieee80211MsduAContainer::Ieee80211MsduAContainer(const Ieee80211MsduAContainer &other) :
        Ieee80211MeshFrame()
{
    encapsulateVector.clear();
    setName(other.getName());
    operator=(other);
}

void Ieee80211MsduAContainer::forEachChild(cVisitor *v)
{
    cPacket::forEachChild(v);
    if (!encapsulateVector.empty())
    {
        for (unsigned int i = 0; i < encapsulateVector.size(); i++)
            v->visit(encapsulateVector[i]);
    }
}

void Ieee80211MsduAContainer::_deleteEncapVector()
{
    while (!encapsulateVector.empty())
    {
        Ieee80211DataFrame *pkt =  encapsulateVector.back();
        encapsulateVector.pop_back();
        if (pkt)
        {
            drop(pkt);
            delete pkt;
        }
    }
}

Ieee80211DataFrame *Ieee80211MsduAContainer::popBack()
{
    if (encapsulateVector.empty())
        return nullptr;
    if (getBitLength() > 0)
        setBitLength(getBitLength() - encapsulateVector.back()->getBitLength());
    if (getBitLength() < 0)
        throw cRuntimeError(this, "popBack(): packet length is smaller than encapsulated packet");
    if (encapsulateVector.back()->getOwner() != this)
        take(encapsulateVector.back());
    Ieee80211DataFrame *msg = encapsulateVector.back();
    encapsulateVector.pop_back();
    if (msg)
        drop(msg);
    return msg;
}

Ieee80211DataFrame *Ieee80211MsduAContainer::popFrom()
{
    if (encapsulateVector.empty())
        return nullptr;
    if (getBitLength() > 0)
        setBitLength(getBitLength() - encapsulateVector.front()->getBitLength());
    if (getBitLength() < 0)
        throw cRuntimeError(this, "popFrom(): packet length is smaller than encapsulated packet");
    if (encapsulateVector.front()->getOwner() != this)
        take(encapsulateVector.front());
    Ieee80211DataFrame *msg = encapsulateVector.front();
    encapsulateVector.erase(encapsulateVector.begin());
    if (msg)
        drop(msg);
    return msg;
}

void Ieee80211MsduAContainer::pushBack(Ieee80211DataFrame *pkt)
{
    if (pkt == nullptr)
        return;

    // Sanity check, check if the packet is already in the vector
    for (unsigned int i = 0; i < encapsulateVector.size(); i++)
    {
        if (encapsulateVector[i] == pkt)
            throw cRuntimeError(this, "pushBack(): packet already in the vector (%s)%s, owner is (%s)%s",
                    pkt->getClassName(), pkt->getFullName(), pkt->getOwner()->getClassName(),
                    pkt->getOwner()->getFullPath().c_str());

    }
    // the previous must be
    // 8.3.2.2 A-MSDU format Each A-MSDU subframe (except the last) is padded so that its length is a multiple of 4 octets
    // padding the last
    if (!encapsulateVector.empty())
    {
        cPacket * pktAux = encapsulateVector.back();
        uint64_t size = pktAux->getByteLength();

        if (size % 4)
        {
            size++;
            while (size % 4) size++;
            setByteLength(getByteLength() - pktAux->getByteLength() + size);
            pktAux->setByteLength(size);
        }
    }

    setBitLength(getBitLength() + pkt->getBitLength());
    if (pkt->getOwner() != getSimulation()->getContextSimpleModule())
        throw cRuntimeError(this, "pushBack(): not owner of message (%s)%s, owner is (%s)%s", pkt->getClassName(),
                pkt->getFullName(), pkt->getOwner()->getClassName(), pkt->getOwner()->getFullPath().c_str());
    take(pkt);
    //drop(pkt);
    encapsulateVector.push_back(pkt);
}

void Ieee80211MsduAContainer::pushFrom(Ieee80211DataFrame *pkt)
{
    if (pkt == nullptr)
        return;
    // Sanity check, check if the packet is already in the vector
    for (unsigned int i = 0; i < encapsulateVector.size(); i++)
    {
        if (encapsulateVector[i] == pkt)
            throw cRuntimeError(this, "pushBack(): packet already in the vector (%s)%s, owner is (%s)%s",
                    pkt->getClassName(), pkt->getFullName(), pkt->getOwner()->getClassName(),
                    pkt->getOwner()->getFullPath().c_str());

    }
    setBitLength(getBitLength() + pkt->getBitLength());
    if (pkt->getOwner() != getSimulation()->getContextSimpleModule())
        throw cRuntimeError(this, "pushFrom(): not owner of message (%s)%s, owner is (%s)%s", pkt->getClassName(),
                pkt->getFullName(), pkt->getOwner()->getClassName(), pkt->getOwner()->getFullPath().c_str());
    take(pkt);
    //drop(pkt);
    encapsulateVector.insert(encapsulateVector.begin(), pkt);
}

Ieee80211DataFrame *Ieee80211MsduAContainer::getPacket(unsigned int i) const
{

    if (i >= encapsulateVector.size())
        return nullptr;
    return encapsulateVector[i];
}

cPacket *Ieee80211MsduAContainer::decapsulatePacket(unsigned int i)
{

    if (i >= encapsulateVector.size())
        return nullptr;
    cPacket * pkt = encapsulateVector[i];
    if (getBitLength() > 0)
        setBitLength(getBitLength() - pkt->getBitLength());
    if (pkt->getOwner() != this)
        take(pkt);
    encapsulateVector.erase(encapsulateVector.begin() + i);
    if (pkt)
        drop(pkt);
    return pkt;
}

void Ieee80211MsduAContainer::setPacketKind(unsigned int i, int kind)
{
    if (i >= encapsulateVector.size())
        return;
    encapsulateVector[i]->setKind(kind);
}

Ieee80211MsduAContainer& Ieee80211MsduAContainer::operator=(const Ieee80211MsduAContainer& msg)
{
    if (this == &msg)
        return *this;
    Ieee80211MeshFrame::operator=(msg);

    if (encapsulateVector.size() > 0)
    {
        _deleteEncapVector();
    }
    if (msg.encapsulateVector.size() > 0)
    {
        for (unsigned int i = 0; i < msg.encapsulateVector.size(); i++)
        {
            Ieee80211DataFrame *pkt = msg.encapsulateVector[i]->dup();
            take(pkt);
            encapsulateVector.push_back(pkt);
        }
    }
    return *this;
}


}

}



