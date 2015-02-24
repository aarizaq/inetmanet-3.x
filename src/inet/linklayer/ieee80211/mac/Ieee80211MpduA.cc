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

#include "inet/linklayer/ieee80211/mac/Ieee80211MpduA.h"

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t)
{
    throw cRuntimeError(
            "Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",
            opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t)
{
    throw cRuntimeError(
            "Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",
            opp_typename(typeid(t)));
}

namespace inet {
namespace ieee80211 {

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out, const T&)
{
    return out;
}

Ieee80211MpduA::~Ieee80211MpduA()
{
    // TODO Auto-generated destructor stub
    _deleteEncapVector();
}

Ieee80211MpduA::Ieee80211MpduA(Ieee80211DataOrMgmtFrame *frame) :
        Ieee80211DataOrMgmtFrame(*frame)
{
    encapsulateVector.clear();
    pushBack(frame);
}

Ieee80211MpduA::Ieee80211MpduA(const char *name, int kind) :
        Ieee80211DataOrMgmtFrame(name, kind)
{
    encapsulateVector.clear();
}

Ieee80211MpduA::Ieee80211MpduA(Ieee80211MpduA &other) :
        Ieee80211DataOrMgmtFrame()
{
    encapsulateVector.clear();
    setName(other.getName());
    operator=(other);
}

void Ieee80211MpduA::forEachChild(cVisitor *v)
{
    cPacket::forEachChild(v);
    if (!encapsulateVector.empty())
    {
        for (unsigned int i = 0; i < encapsulateVector.size(); i++)
            v->visit(encapsulateVector[i]->pkt);
    }
}

void Ieee80211MpduA::parsimPack(cCommBuffer *buffer)
{
    cPacket::parsimPack(buffer);
    doPacking(buffer, this->encapsulateVector);
}

void Ieee80211MpduA::parsimUnpack(cCommBuffer *buffer)
{
    cPacket::parsimUnpack(buffer);
    doUnpacking(buffer, this->encapsulateVector);
}

void Ieee80211MpduA::_deleteEncapVector()
{
    while (!encapsulateVector.empty())
    {
        if (encapsulateVector.back()->pkt->getOwner()!=this)
            take(encapsulateVector.back()->pkt);
        delete encapsulateVector.back()->pkt;
        encapsulateVector.pop_back();
    }
}

Ieee80211DataOrMgmtFrame *Ieee80211MpduA::popBack()
{
    if (encapsulateVector.empty())
        return nullptr;
    if (getBitLength() > 0)
        setBitLength(getBitLength() - encapsulateVector.back()->pkt->getBitLength());
    if (getBitLength() < 0)
        throw cRuntimeError(this, "popBack(): packet length is smaller than encapsulated packet");
    if (encapsulateVector.back()->pkt->getOwner() != this)
        take(encapsulateVector.back()->pkt);
    Ieee80211DataOrMgmtFrame *msg = encapsulateVector.back()->pkt;
    encapsulateVector.pop_back();
    if (msg)
        drop(msg);
    return msg;
}

Ieee80211DataOrMgmtFrame *Ieee80211MpduA::popFront()
{
    if (encapsulateVector.empty())
        return nullptr;
    if (getBitLength() > 0)
        setBitLength(getBitLength() - encapsulateVector.front()->pkt->getBitLength());
    if (getBitLength() < 0)
        throw cRuntimeError(this, "popFrom(): packet length is smaller than encapsulated packet");
    if (encapsulateVector.front()->pkt->getOwner() != this)
        take(encapsulateVector.front()->pkt);
    Ieee80211DataOrMgmtFrame *msg = encapsulateVector.front()->pkt;
    encapsulateVector.erase(encapsulateVector.begin());
    if (msg)
        drop(msg);
    return msg;
}

void Ieee80211MpduA::pushBack(Ieee80211DataOrMgmtFrame *pkt)
{
    pushBack(pkt, 0);
}

void Ieee80211MpduA::pushFront(Ieee80211DataOrMgmtFrame *pkt)
{
    pushFront(pkt, 0);
}



void Ieee80211MpduA::pushBack(Ieee80211DataOrMgmtFrame *pkt, int retries)
{
    if (pkt == nullptr)
        return;

    // Sanity check, check if the packet is already in the vector
    for (unsigned int i = 0; i < encapsulateVector.size(); i++)
    {
        if (encapsulateVector[i]->pkt == pkt)
            throw cRuntimeError(this, "pushBack(): packet already in the vector (%s)%s, owner is (%s)%s",
                    pkt->getClassName(), pkt->getFullName(), pkt->getOwner()->getClassName(),
                    pkt->getOwner()->getFullPath().c_str());

    }
    // the previous must be
    // 8.3.2.2 A-MSDU format Each A-MSDU subframe (except the last) is padded so that its length is a multiple of 4 octets
    // padding the last
    if (!encapsulateVector.empty())
    {
        cPacket * pktAux = encapsulateVector.back()->pkt;
        uint64_t size = pktAux->getByteLength();

        if (size % 4)
        {
            size++;
            while (size % 4) size++;
            setBitLength(getBitLength() - pktAux->getByteLength() + size);
            pktAux->setBitLength(size);
        }
    }

    setBitLength(getBitLength() + pkt->getBitLength());
    PacketStruct * shareStructPtr = new PacketStruct();
    if (pkt->getOwner() != simulation.getContextSimpleModule())
        throw cRuntimeError(this, "pushBack(): not owner of message (%s)%s, owner is (%s)%s", pkt->getClassName(),
                pkt->getFullName(), pkt->getOwner()->getClassName(), pkt->getOwner()->getFullPath().c_str());
    take(pkt);
    shareStructPtr->pkt = pkt;
    shareStructPtr->numRetries = retries;
    encapsulateVector.push_back(shareStructPtr);
}

void Ieee80211MpduA::pushFront(Ieee80211DataOrMgmtFrame *pkt, int retries)
{
    if (pkt == nullptr)
        return;
    // Sanity check, check if the packet is already in the vector
    for (unsigned int i = 0; i < encapsulateVector.size(); i++)
    {
        if (encapsulateVector[i]->pkt == pkt)
            throw cRuntimeError(this, "pushBack(): packet already in the vector (%s)%s, owner is (%s)%s",
                    pkt->getClassName(), pkt->getFullName(), pkt->getOwner()->getClassName(),
                    pkt->getOwner()->getFullPath().c_str());

    }
    setBitLength(getBitLength() + pkt->getBitLength());
    PacketStruct * shareStructPtr = new PacketStruct();
    if (pkt->getOwner() != simulation.getContextSimpleModule())
        throw cRuntimeError(this, "pushFrom(): not owner of message (%s)%s, owner is (%s)%s", pkt->getClassName(),
                pkt->getFullName(), pkt->getOwner()->getClassName(), pkt->getOwner()->getFullPath().c_str());
    take(shareStructPtr->pkt = pkt);
    shareStructPtr->numRetries = retries;

    encapsulateVector.insert(encapsulateVector.begin(), shareStructPtr);
}


Ieee80211DataOrMgmtFrame *Ieee80211MpduA::getPacket(unsigned int i) const
{

    if (i >= encapsulateVector.size())
        return nullptr;
    return encapsulateVector[i]->pkt;
}

int Ieee80211MpduA::getNumRetries(const unsigned int &i) const
{

    if (i >= encapsulateVector.size())
        return 0;
    return encapsulateVector[i]->numRetries;
}

void Ieee80211MpduA::setNumRetries(const unsigned int &i, const int &val)
{

    if (i >= encapsulateVector.size())
        return;
    encapsulateVector[i]->numRetries = val;
}

Ieee80211DataOrMgmtFrame *Ieee80211MpduA::decapsulatePacket(unsigned int i)
{

    if (i >= encapsulateVector.size())
        return nullptr;
    Ieee80211DataOrMgmtFrame * pkt = encapsulateVector[i]->pkt;
    if (getBitLength() > 0)
        setBitLength(getBitLength() - encapsulateVector.front()->pkt->getBitLength());
    if (pkt->getOwner() != this)
        take(pkt);
    encapsulateVector.erase(encapsulateVector.begin() + i);
    if (pkt)
        drop(pkt);
    return pkt;
}

void Ieee80211MpduA::setPacketKind(unsigned int i, int kind)
{
    if (i >= encapsulateVector.size())
        return;
    encapsulateVector[i]->pkt->setKind(kind);
}

Ieee80211MpduA& Ieee80211MpduA::operator=(const Ieee80211MpduA& msg)
{
    if (this == &msg)
        return *this;
    cPacket::operator=(msg);
    if (encapsulateVector.size() > 0)
    {
        _deleteEncapVector();
    }
    if (msg.encapsulateVector.size() > 0)
    {
        for (unsigned int i = 0; i < msg.encapsulateVector.size(); i++)
        {
            PacketStruct * shareStructPtr = new PacketStruct();
            shareStructPtr->pkt = encapsulateVector[i]->pkt->dup();
            encapsulateVector.push_back(shareStructPtr);
        }
    }
    return *this;
}

}

}

