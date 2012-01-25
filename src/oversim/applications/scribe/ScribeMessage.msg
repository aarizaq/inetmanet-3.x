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

//
// @file ScribeMessage.msg
// @author Stephan Krause
//

cplusplus {{
#include <CommonMessages_m.h>

class ScribeTimer;

#include "ScribeGroup.h"

// Length definition
static const int SCRIBE_WRONGROOT_L = 8;
static const int SCRIBE_EMPTYFLAG_L = 8;

// HACK: groupId field nor counted in length calculation (see message definition)
#define SCRIBE_JOINCALL_L(msg) (BASECALL_L(msg))
#define SCRIBE_JOINRESPONSE_L(msg) (BASECALL_L(msg))
#define SCRIBE_PUBLISHCALL_L(msg) (BASECALL_L(msg))
#define SCRIBE_PUBLISHRESPONSE_L(msg) (BASECALL_L(msg) + SCRIBE_WRONGROOT_L)

#define SCRIBE_SUBSCRIPTIONREFRESH_L(msg) (NODEHANDLE_L + KEY_L)
#define SCRIBE_LEAVE_L(msg) (NODEHANDLE_L + KEY_L)
#define SCRIBE_DATA_L(msg) (KEY_L + SCRIBE_EMPTYFLAG_L)
}}



class noncobject ScribeGroupPointer;

class noncobject ChildPointer;

class noncobject NodeHandle;

class noncobject OverlayKey;

class BaseCallMessage;


class BaseResponseMessage;



enum ScribeTimerType 
{

    SCRIBE_HEARTBEAT = 0; // Send heartbeat to all children of a group
    SCRIBE_SUBSCRIPTION_REFRESH = 1; // refresh subscriptions for all groups
    SCRIBE_PARENT_TIMEOUT = 2; // Parent failed to send heartbeat
    SCRIBE_CHILD_TIMEOUT = 3; // Child faild to refresh subscription
}

packet ScribeJoinCall extends BaseCallMessage
{
        OverlayKey groupId; // HACK. The key is already included in BaseRouteMessage,
                            //   but it gets lost in handleRpc
}

packet ScribeJoinResponse extends BaseResponseMessage
{
        OverlayKey groupId; // HACK. The key is already included in BaseRouteMessage,
                            //   but it gets lost in handleRpc
}

packet ScribePublishCall extends BaseCallMessage
{
        OverlayKey groupId; // HACK. The key is already included in BaseRouteMessage,
                            //   but it gets lost in handleRpc
}

packet ScribePublishResponse extends BaseResponseMessage
{
        OverlayKey groupId; // HACK. The key is already included in BaseRouteMessage,
                            //   but it gets lost in handleRpc
        bool wrongRoot = false;
}

packet ScribeSubscriptionRefreshMessage
{
        NodeHandle src;
        OverlayKey groupId;
}

packet ScribeLeaveMessage
{
        NodeHandle src;
        OverlayKey groupId;
}

message ScribeTimer
{
        int timerType enum(ScribeTimerType);
        NodeHandle child;
        OverlayKey group;
}

packet ScribeDataMessage
{
        OverlayKey groupId;
        bool empty = false;
}
