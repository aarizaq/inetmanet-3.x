//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef __SCTPSENDSTREAM_H
#define __SCTPSENDSTREAM_H

#include <omnetpp.h>
#include <assert.h>
#include "SCTPQueue.h"
#include "SCTPAssociation.h"
#include "SCTPMessage_m.h"

class SCTPMessage;
class SCTPCommand;
class SCTPDataVariables;


class SIM_API SCTPDataMsgQueue : public cQueue
{
  public:
    SCTPDataMsgQueue(const char *name = NULL, CompareFunc cmp = NULL);
    SCTPDataMsgQueue(const SCTPDataMsgQueue& queue);
    SCTPDataMsgQueue& operator=(const SCTPDataMsgQueue& queue);
    virtual SCTPDataMsgQueue *dup() const  { return new SCTPDataMsgQueue(*this); }
    virtual std::string info() const;
    virtual void insert(SCTPDataMsg* pkt);
    virtual void insertBefore(SCTPDataMsg* where, SCTPDataMsg* pkt);
    virtual void insertAfter(SCTPDataMsg* where, SCTPDataMsg* pkt);
    virtual SCTPDataMsg* remove(SCTPDataMsg* pkt);
    virtual SCTPDataMsg* pop();
    uint64 getBitLength() const         { return (8 * totalLength);             }
    uint64 getByteLength() const        { return (totalLength);                 }
    virtual SCTPDataMsg* front() const  { return (SCTPDataMsg*)cQueue::front(); }
    virtual SCTPDataMsg* back() const   { return (SCTPDataMsg*)cQueue::back();  }
    SCTPDataMsg* get(const int i) const { return (SCTPDataMsg*)cQueue::get(i);  }
    void addLen(const uint32 length);
    void subLen(const uint32 length);

  private:
    void addLen(const SCTPDataMsg* pkt);
    void subLen(const SCTPDataMsg* pkt);

    uint64 totalLength;
};


class INET_API SCTPSendStream : public cPolymorphic
{
   protected:
      uint16            streamId;
      uint16            nextStreamSeqNum;
      SCTPDataMsgQueue* streamQ;
      SCTPDataMsgQueue* uStreamQ;
      int32             ssn;

   public:
      SCTPSendStream(const uint16 id);
      ~SCTPSendStream();

      inline SCTPDataMsgQueue* getStreamQ() const          { return streamQ; };
      inline SCTPDataMsgQueue* getUnorderedStreamQ() const { return uStreamQ; };
      inline uint32 getNextStreamSeqNum() const            { return nextStreamSeqNum; };
      inline void setNextStreamSeqNum(const uint16 num)    { nextStreamSeqNum = num; };
      inline uint16 getStreamId() const                    { return streamId; };
      inline void setStreamId(const uint16 id)             { streamId = id; };

      inline unsigned int getQueuedMsgs() const {
         return(streamQ->getLength() + uStreamQ->getLength());
      }
      inline unsigned long long getQueuedBytes() const {
         return(streamQ->getByteLength() + uStreamQ->getByteLength());
      }

};

#endif
