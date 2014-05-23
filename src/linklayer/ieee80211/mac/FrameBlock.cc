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

#include "FrameBlock.h"

#include "FrameBlock.h"
//#define SHAREDBLOCK

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

FrameBlock::~FrameBlock() {
    // TODO Auto-generated destructor stub
    _deleteEncapVector();
}

FrameBlock::FrameBlock(const char *name, int kind):Ieee80211TwoAddressFrame(name,kind)
{
    encapsulateVector.clear();
}

FrameBlock::FrameBlock(FrameBlock &other):Ieee80211TwoAddressFrame()
{
    encapsulateVector.clear();
    setName(other.getName());
    operator=(other);
}

void FrameBlock::forEachChild(cVisitor *v)
{
    FrameBlock::forEachChild(v);
    if (!encapsulateVector.empty())
    {
        for (unsigned int i=0;i<encapsulateVector.size();i++)
        {
            _detachShareVector(i); // see method comment why this is needed
            v->visit(encapsulateVector[i]->pkt);
        }
    }
}

void FrameBlock::parsimPack(cCommBuffer *buffer)
{
cPacket::parsimPack(buffer);
    doPacking(buffer,this->encapsulateVector);
}

void FrameBlock::parsimUnpack(cCommBuffer *buffer)
{
cPacket::parsimUnpack(buffer);
    doUnpacking(buffer,this->encapsulateVector);
}


void FrameBlock::_deleteEncapVector()
{
    while(!encapsulateVector.empty())
    {
#ifdef SHAREDBLOCK
        if (encapsulateVector.back()->shareCount>0)
        {
            encapsulateVector.back()->shareCount--;
        }
        else
        {
            if (encapsulateVector.back()->pkt->getOwner()!=this)
                take (encapsulateVector.back()->pkt);
            delete encapsulateVector.back()->pkt;
        }
#else
        delete encapsulateVector.back()->pkt;
#endif
        encapsulateVector.pop_back();
    }
}

cPacket *FrameBlock::popBack()
{
    if (encapsulateVector.empty())
        return NULL;
    if (getBitLength()>0)
        setBitLength(getBitLength()-encapsulateVector.back()->pkt->getBitLength());
    if (getBitLength()<0)
        throw cRuntimeError(this,"popBack(): packet length is smaller than encapsulated packet");
    if (encapsulateVector.back()->pkt->getOwner()!=this)
     take (encapsulateVector.back()->pkt);
#ifdef SHAREDBLOCK
    if (encapsulateVector.back()->shareCount>0)
    {
        encapsulateVector.back()->shareCount--;
        cPacket * msg = encapsulateVector.front()->pkt->dup();
        encapsulateVector.pop_back();
        return msg;
    }
#endif
    cPacket *msg = encapsulateVector.back()->pkt;
    encapsulateVector.pop_back ();
    if (msg) drop(msg);
    return msg;
}


cPacket *FrameBlock::popFrom()
{
    if (encapsulateVector.empty())
        return NULL;
    if (getBitLength()>0)
        setBitLength(getBitLength()-encapsulateVector.front()->pkt->getBitLength());
    if (getBitLength()<0)
        throw cRuntimeError(this,"popFrom(): packet length is smaller than encapsulated packet");
    if (encapsulateVector.front()->pkt->getOwner()!=this)
     take (encapsulateVector.front()->pkt);
#ifdef SHAREDBLOCK
    if (encapsulateVector.front()->shareCount>0)
    {
        encapsulateVector.front()->shareCount--;
        cPacket *msg = encapsulateVector.front()->pkt->dup();
        encapsulateVector.erase (encapsulateVector.begin());
        if (msg) drop(msg);
        return msg;
    }
#endif
    cPacket *msg = encapsulateVector.front()->pkt;
    encapsulateVector.erase (encapsulateVector.begin());
    if (msg) drop(msg);
    return msg;
}

void FrameBlock::pushBack(cPacket *pkt)
{
    if (pkt==NULL)
        return;

    // Sanity check, check if the packet is already in the vector
    for (unsigned int i=0;i<encapsulateVector.size();i++)
    {
        if (encapsulateVector[i]->pkt==pkt)
            throw cRuntimeError(this,"pushBack(): packet already in the vector (%s)%s, owner is (%s)%s",
                pkt->getClassName(), pkt->getFullName(),
                pkt->getOwner()->getClassName(), pkt->getOwner()->getFullPath().c_str());

    }
    setBitLength(getBitLength()+pkt->getBitLength());
    ShareStruct * shareStructPtr = new ShareStruct();
    if (pkt->getOwner()!=simulation.getContextSimpleModule())
        throw cRuntimeError(this,"pushBack(): not owner of message (%s)%s, owner is (%s)%s",
            pkt->getClassName(), pkt->getFullName(),
            pkt->getOwner()->getClassName(), pkt->getOwner()->getFullPath().c_str());
    take(shareStructPtr->pkt=pkt);
    encapsulateVector.push_back(shareStructPtr);
}

void FrameBlock::pushFrom(cPacket *pkt)
{
    if (pkt==NULL)
        return;
    // Sanity check, check if the packet is already in the vector
    for (unsigned int i=0;i<encapsulateVector.size();i++)
    {
        if (encapsulateVector[i]->pkt==pkt)
            throw cRuntimeError(this,"pushBack(): packet already in the vector (%s)%s, owner is (%s)%s",
                pkt->getClassName(), pkt->getFullName(),
                pkt->getOwner()->getClassName(), pkt->getOwner()->getFullPath().c_str());

    }
    setBitLength(getBitLength()+pkt->getBitLength());
    ShareStruct * shareStructPtr = new ShareStruct();
    if (pkt->getOwner()!=simulation.getContextSimpleModule())
          throw cRuntimeError(this,"pushFrom(): not owner of message (%s)%s, owner is (%s)%s",
              pkt->getClassName(), pkt->getFullName(),
              pkt->getOwner()->getClassName(), pkt->getOwner()->getFullPath().c_str());
    take(shareStructPtr->pkt=pkt);
    encapsulateVector.insert (encapsulateVector.begin(),shareStructPtr);
 }

void FrameBlock::_detachShareVector(unsigned int i)
{
    if (i<encapsulateVector.size())
    {
#ifdef SHAREDBLOCK
        if (encapsulateVector[i]->shareCount>0)
        {
            ShareStruct *share = new ShareStruct;
            if (encapsulateVector.front()->pkt->getOwner()!=this)
             take (encapsulateVector[i]->pkt);
            share->shareCount=0;
            take (share->pkt=encapsulateVector[i]->pkt->dup());
            encapsulateVector[i]->shareCount--;
            encapsulateVector[i]=share;
        }
#endif
    }
}

cPacket *FrameBlock::getPacket(unsigned int i) const
{

    if (i>=encapsulateVector.size())
        return NULL;
    const_cast<FrameBlock*>(this)->_detachShareVector(i);
    return encapsulateVector[i]->pkt;
}

cPacket *FrameBlock::decapsulatePacket(unsigned int i)
{

    if (i>=encapsulateVector.size())
        return NULL;
    const_cast<FrameBlock*>(this)->_detachShareVector(i);
    cPacket * pkt = encapsulateVector[i]->pkt;
    if (getBitLength()>0)
           setBitLength(getBitLength()-encapsulateVector.front()->pkt->getBitLength());
    if (pkt->getOwner()!=this)
     take (pkt);
#ifdef SHAREDBLOCK
    if (pkt->shareCount>0)
    {
        pkt->shareCount--;
        cPacket *msg = encapsulateVector.front()->pkt->dup();
        encapsulateVector.erase (encapsulateVector.begin()+i);
        if (msg) drop(msg);
        return msg;
    }
#endif
    encapsulateVector.erase (encapsulateVector.begin()+i);
    if (pkt) drop(pkt);
    return pkt;
}


void FrameBlock::setPacketKind(unsigned int i,int kind)
{
    if (i>=encapsulateVector.size())
        return;
    this->_detachShareVector(i);
    encapsulateVector[i]->pkt->setKind(kind);
}

FrameBlock& FrameBlock::operator=(const FrameBlock& msg)
{
    if (this==&msg) return *this;
    cPacket::operator=(msg);
    if (encapsulateVector.size()>0)
    {
        _deleteEncapVector();
    }
    if (msg.encapsulateVector.size()>0)
    {
#ifdef SHAREDBLOCK
        encapsulateVector = msg.encapsulateVector;
        for (unsigned int i=0;i<msg.encapsulateVector.size();i++)
        {
            encapsulateVector[i]->shareCount++;
        }
#else
        for (unsigned int i=0;i<msg.encapsulateVector.size();i++)
        {
         ShareStruct * shareStructPtr = new ShareStruct();
         shareStructPtr->pkt=encapsulateVector[i]->pkt->dup();
            encapsulateVector.push_back(shareStructPtr);
        }
#endif
    }
    return *this;
}


