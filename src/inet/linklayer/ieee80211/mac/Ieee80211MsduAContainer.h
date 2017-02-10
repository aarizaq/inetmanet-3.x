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

#ifndef __IEEE80211MSDUA_H__
#define __IEEE80211MSDUA_H__
#include <deque>

#include <omnetpp.h>
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {


class Ieee80211MsduAContainer : public Ieee80211MeshFrame
{
private:

    std::deque<Ieee80211DataFrame *> encapsulateVector;
    void _deleteEncapVector();
    bool _checkIfShare();
protected:
  // protected and unimplemented operator==(), to prevent accidental usage
  bool operator==(const Ieee80211MsduAContainer&);

public:
    Ieee80211MsduAContainer(const char *name=nullptr, int kind=0);
    Ieee80211MsduAContainer(const Ieee80211MsduAContainer &);
    virtual Ieee80211MsduAContainer * dup() const {return new Ieee80211MsduAContainer(*this);}
    virtual ~Ieee80211MsduAContainer();
    Ieee80211MsduAContainer& operator=(const Ieee80211MsduAContainer& msg);
    virtual Ieee80211DataFrame *getPacket(unsigned int i) const;
    virtual void setPacketKind(unsigned int i,int kind);
    virtual unsigned int getNumEncap() const {return encapsulateVector.size();}
    uint64_t getPktLength(unsigned int i) const
    {
        if (i<encapsulateVector.size())
            return encapsulateVector[i]->getBitLength();
        return 0;
    }
    cPacket *decapsulatePacket(unsigned int i) ;
    virtual unsigned int getEncapSize() {return encapsulateVector.size();}

    virtual void pushFrom(Ieee80211DataFrame *);
    virtual void pushBack(Ieee80211DataFrame *);
    virtual Ieee80211DataFrame *popFrom();
    virtual Ieee80211DataFrame *popBack();
    virtual Ieee80211DataFrame* getFrom() {return encapsulateVector.front();}
    virtual Ieee80211DataFrame* getBack() {return encapsulateVector.back();}
    virtual bool hasBlock(){return !encapsulateVector.empty();}
    virtual void forEachChild(cVisitor *v);

    virtual void encapsulate(cPacket *packet) override
    {
        throw cRuntimeError("operation not supported");
    }


    virtual cPacket *decapsulate() override
    {
        throw cRuntimeError("operation not supported");
        return nullptr;
    }

    /**
     * Returns a pointer to the encapsulated packet, or nullptr if there
     * is no encapsulated packet.
     *
     * IMPORTANT: see notes at encapsulate() about reference counting
     * of encapsulated packets.
     */
    virtual cPacket *getEncapsulatedPacket() const override
    {
        throw cRuntimeError("operation not supported");
        return nullptr;
    }


    /**
     * Returns true if the packet contains an encapsulated packet, and false
     * otherwise. This method is potentially more efficient than
     * <tt>getEncapsulatedPacket()!=nullptr</tt>, because it does not need to
     * unshare a shared encapsulated packet (see note at encapsulate()).
     */
    virtual bool hasEncapsulatedPacket() const override
    {
        throw cRuntimeError("operation not supported");
        return false;
    }

    void parsimPack(cCommBuffer *buffer) const
    {
        throw cRuntimeError(this, "ParSim not supported");
    }
    void parsimUnpack(cCommBuffer *buffer)
    {
        throw cRuntimeError(this, "ParSim not supported");
    }

};


}
}

#endif /* FRAMEBLOCK_H_ */
