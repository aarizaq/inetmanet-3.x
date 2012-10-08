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

#include "SCTPSendStream.h"

Register_Class(SCTPDataMsgQueue);


SCTPDataMsgQueue::SCTPDataMsgQueue(const char *name, CompareFunc cmp)
   : cQueue(name, cmp)
{
   totalLength = 0;
}

SCTPDataMsgQueue::SCTPDataMsgQueue(const SCTPDataMsgQueue& queue)
{
   setName(queue.getName());
   operator=(queue);
}

SCTPDataMsgQueue& SCTPDataMsgQueue::operator=(const SCTPDataMsgQueue& queue)
{
   cQueue::operator=(queue);
   totalLength = queue.totalLength;
   return *this;
}

std::string SCTPDataMsgQueue::info() const
{
   if (empty()) {
      return std::string("empty");
   }
   std::stringstream out;
   out << "len=" << getLength() << ", " << getByteLength()
       << " bits (" << getByteLength() << " bytes)";
   return out.str();
}

void SCTPDataMsgQueue::insert(SCTPDataMsg* pkt)
{
   addLen(pkt);
   cQueue::insert(pkt);
}

void SCTPDataMsgQueue::insertBefore(SCTPDataMsg* where, SCTPDataMsg* pkt)
{
   addLen(pkt);
   cQueue::insertBefore(where, pkt);
}

void SCTPDataMsgQueue::insertAfter(SCTPDataMsg* where, SCTPDataMsg* pkt)
{
   addLen(pkt);
   cQueue::insertAfter(where, pkt);
}

SCTPDataMsg* SCTPDataMsgQueue::remove(SCTPDataMsg* pkt)
{
   SCTPDataMsg* pkt1 = (SCTPDataMsg*)cQueue::remove(pkt);
   subLen(pkt1);
   return pkt1;
}

SCTPDataMsg* SCTPDataMsgQueue::pop()
{
   SCTPDataMsg* pkt = (SCTPDataMsg*)cQueue::pop();
   subLen(pkt);
   return pkt;
}

void SCTPDataMsgQueue::addLen(const SCTPDataMsg* pkt)
{
   if(pkt) {
      const cPacket* encapsulatedPacket = pkt->getEncapsulatedPacket();
      assert(encapsulatedPacket != NULL);
      addLen(encapsulatedPacket->getByteLength());
   }
}

void SCTPDataMsgQueue::subLen(const SCTPDataMsg* pkt)
{
   if(pkt) {
      const cPacket* encapsulatedPacket = pkt->getEncapsulatedPacket();
      assert(encapsulatedPacket != NULL);
      subLen(encapsulatedPacket->getByteLength());
   }
}

void SCTPDataMsgQueue::addLen(const uint32 length)
{
   assert(length >= 0);
   totalLength += (uint64)length;
}

void SCTPDataMsgQueue::subLen(const uint32 length)
{
   assert(length >= 0);
   if(totalLength < (uint64)length) {
      assert(false);
   }
   totalLength -= (uint64)length;
}


SCTPSendStream::SCTPSendStream(const uint16 id)
{
   streamId         = id;
   nextStreamSeqNum = 0;

   char queueName[64];
   snprintf(queueName, sizeof(queueName), "OrderedSendQueue ID %d", id);
   streamQ = new SCTPDataMsgQueue(queueName);
   snprintf(queueName, sizeof(queueName), "UnorderedSendQueue ID %d", id);
   uStreamQ = new SCTPDataMsgQueue(queueName);
}

SCTPSendStream::~SCTPSendStream()
{
   SCTPDataMsg*       datMsg;
   SCTPSimpleMessage* smsg;
   int32              count = streamQ->length();
   while (!streamQ->empty()) {
      datMsg = check_and_cast<SCTPDataMsg*>(streamQ->pop());
      smsg   = check_and_cast<SCTPSimpleMessage*>(datMsg->decapsulate());
      delete smsg;
      delete datMsg;
      count--;
   }
   while (!uStreamQ->empty()) {
      datMsg = check_and_cast<SCTPDataMsg*>(uStreamQ->pop());
      smsg   = check_and_cast<SCTPSimpleMessage*>(datMsg->decapsulate());
      delete smsg;
      delete datMsg;
   }
   delete streamQ;
   delete uStreamQ;
}
