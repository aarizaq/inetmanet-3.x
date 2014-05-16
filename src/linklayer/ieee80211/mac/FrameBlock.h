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

#ifndef FRAMEBLOCK_H_
#define FRAMEBLOCK_H_
#include <omnetpp.h>
#include "Ieee80211Frame_m.h"
class FrameBlock : public Ieee80211TwoAddressFrame
{
private:
    struct ShareStruct{
        cPacket * pkt;
        unsigned int shareCount;
        ShareStruct(){
            pkt=NULL;
            shareCount =0;
        }
    };
    std::vector<ShareStruct*> encapsulateVector;
    void _deleteEncapVector();
    bool _checkIfShare();
    void _detachShareVector(unsigned int i);
public:
    FrameBlock(const char *name=NULL, int kind=0);
    FrameBlock(FrameBlock &);
    virtual FrameBlock * dup(){return new FrameBlock(*this);}
    virtual ~FrameBlock();
    FrameBlock& operator=(const FrameBlock& msg);
    virtual cPacket *getPacket(unsigned int i) const;
    virtual void setPacketKind(unsigned int i,int kind);
    virtual unsigned int getNumEncap() const {return encapsulateVector.size();}
    uint64_t getPktLength(unsigned int i) const
    {
        if (i<encapsulateVector.size())
            return encapsulateVector[i]->pkt->getBitLength();
        return 0;
    }
    cPacket *decapsulatePacket(unsigned int i);

    virtual void pushFrom(cPacket *);
    virtual void pushBack(cPacket *);
    virtual cPacket *popFrom();
    virtual cPacket *popBack();
    virtual bool haveBlock(){return !encapsulateVector.empty();}
    virtual void forEachChild(cVisitor *v);

    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);


    virtual void encapsulate(cPacket *packet)
    {
        opp_error("operation not supported");
    }


    virtual cPacket *decapsulate()
    {
        opp_error("operation not supported");
        return NULL;
    }

    /**
     * Returns a pointer to the encapsulated packet, or NULL if there
     * is no encapsulated packet.
     *
     * IMPORTANT: see notes at encapsulate() about reference counting
     * of encapsulated packets.
     */
    virtual cPacket *getEncapsulatedPacket() const
    {
        opp_error("operation not supported");
        return NULL;
    }


    /**
     * Returns true if the packet contains an encapsulated packet, and false
     * otherwise. This method is potentially more efficient than
     * <tt>getEncapsulatedPacket()!=NULL</tt>, because it does not need to
     * unshare a shared encapsulated packet (see note at encapsulate()).
     */
    virtual bool hasEncapsulatedPacket() const
    {
        opp_error("operation not supported");
        return false;
    }
};

inline void doPacking(cCommBuffer *b, FrameBlock& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, FrameBlock& obj) {obj.parsimUnpack(b);}

#endif /* FRAMEBLOCK_H_ */
