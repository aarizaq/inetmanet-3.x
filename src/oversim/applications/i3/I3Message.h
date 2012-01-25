//
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
 * @file I3Message.h
 * @author Antonio Zea
 */


#ifndef __I3MESSAGE_H__
#define __I3MESSAGE_H__

#include "I3Message_m.h"

class I3InsertTriggerMessage : public I3InsertTriggerMessage_Base
{
public:
    I3InsertTriggerMessage(const char *name=NULL, int kind=0);
    I3InsertTriggerMessage(const I3InsertTriggerMessage& other);
    I3InsertTriggerMessage& operator=(const I3InsertTriggerMessage& other);
    virtual I3InsertTriggerMessage *dup() const;
};

class I3RemoveTriggerMessage : public I3RemoveTriggerMessage_Base
{
public:
    I3RemoveTriggerMessage(const char *name=NULL, int kind=0);
    I3RemoveTriggerMessage(const I3RemoveTriggerMessage& other);
    I3RemoveTriggerMessage& operator=(const I3RemoveTriggerMessage& other);
    virtual I3RemoveTriggerMessage *dup() const;

};

class I3SendPacketMessage : public I3SendPacketMessage_Base
{
public:
    I3SendPacketMessage(const char *name=NULL, int kind=0);
    I3SendPacketMessage(const I3SendPacketMessage& other);
    I3SendPacketMessage& operator=(const I3SendPacketMessage& other);
    virtual I3SendPacketMessage *dup() const;

};

class I3QueryReplyMessage : public I3QueryReplyMessage_Base
{
public:
    I3QueryReplyMessage(const char *name=NULL, int kind=0);
    I3QueryReplyMessage(const I3QueryReplyMessage& other);
    I3QueryReplyMessage& operator=(const I3QueryReplyMessage& other);
    virtual I3QueryReplyMessage *dup() const ;

};

#endif
