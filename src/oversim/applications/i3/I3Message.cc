// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file I3Message.cc
 * @author Antonio Zea
 */


#include "I3Message.h"

Register_Class(I3SendPacketMessage);
Register_Class(I3RemoveTriggerMessage);
Register_Class(I3InsertTriggerMessage);
Register_Class(I3QueryReplyMessage);


I3InsertTriggerMessage::I3InsertTriggerMessage(const char *name, int kind) :
        I3InsertTriggerMessage_Base(name,kind)
{
    setType(INSERT_TRIGGER);
    setSendReply(false);
    setName("I3::InsertTrigger");
}

I3InsertTriggerMessage::I3InsertTriggerMessage(const I3InsertTriggerMessage& other) :
        I3InsertTriggerMessage_Base(other.getName())
{
    setType(INSERT_TRIGGER);
    setSendReply(false);
    operator=(other);
}

I3InsertTriggerMessage& I3InsertTriggerMessage::operator=(const I3InsertTriggerMessage& other)
{
    I3InsertTriggerMessage_Base::operator=(other);
    return *this;
}

I3InsertTriggerMessage *I3InsertTriggerMessage::dup() const
{
    return new I3InsertTriggerMessage(*this);
}

I3RemoveTriggerMessage::I3RemoveTriggerMessage(const char *name, int kind) :
        I3RemoveTriggerMessage_Base(name,kind)
{
    setType(REMOVE_TRIGGER);
    setName("I3::RemoveTrigger");
}

I3RemoveTriggerMessage::I3RemoveTriggerMessage(const I3RemoveTriggerMessage& other) :
        I3RemoveTriggerMessage_Base(other.getName())
{
    setType(REMOVE_TRIGGER);
    operator=(other);
}

I3RemoveTriggerMessage& I3RemoveTriggerMessage::operator=(const I3RemoveTriggerMessage& other)
{
    I3RemoveTriggerMessage_Base::operator=(other);
    return *this;
}

I3RemoveTriggerMessage *I3RemoveTriggerMessage::dup() const
{
    return new I3RemoveTriggerMessage(*this);
}

I3SendPacketMessage::I3SendPacketMessage(const char *name, int kind)
        : I3SendPacketMessage_Base(name,kind)
{
    setType(SEND_PACKET);
    setName("I3::SendPacket");
}

I3SendPacketMessage::I3SendPacketMessage(const I3SendPacketMessage& other) :
        I3SendPacketMessage_Base(other.getName())
{
    setType(SEND_PACKET);
    operator=(other);
}

I3SendPacketMessage& I3SendPacketMessage::operator=(const I3SendPacketMessage& other)
{
    I3SendPacketMessage_Base::operator=(other);
    return *this;
}

I3SendPacketMessage *I3SendPacketMessage::dup() const
{
    return new I3SendPacketMessage(*this);
}

I3QueryReplyMessage::I3QueryReplyMessage(const char *name, int kind)
        : I3QueryReplyMessage_Base(name,kind)
{
    setType(QUERY_REPLY);
    setName("I3::QueryReply");
}

I3QueryReplyMessage::I3QueryReplyMessage(const I3QueryReplyMessage& other) :
        I3QueryReplyMessage_Base(other.getName())
{
    setType(QUERY_REPLY);
    operator=(other);
}

I3QueryReplyMessage& I3QueryReplyMessage::operator=(const I3QueryReplyMessage& other)
{
    I3QueryReplyMessage_Base::operator=(other);
    return *this;
}

I3QueryReplyMessage *I3QueryReplyMessage::dup() const
{
    return new I3QueryReplyMessage(*this);
}

