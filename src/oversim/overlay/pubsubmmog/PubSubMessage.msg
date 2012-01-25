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
// @file PubSubMessage.msg
// @author Stephan Krause
//

cplusplus {{
#include <CommonMessages_m.h>
#include <Vector2D.h>

static const int PUBSUB_FLAG_L = 8;
static const int PUBSUB_VECTOR_L = 128;
static const int PUBSUB_RESS_L = 32;
static const int PUBSUB_SUBSPACE_L = 32;
static const int PUBSUB_CHILDPOS_L = 16;
static const int PUBSUB_TIMESTAMP_L = 32;

#define PUBSUB_JOINCALL_L(msg) (BASECALL_L(msg) + PUBSUB_VECTOR_L + PUBSUB_RESS_L)
#define PUBSUB_JOINRESPONSE_L(msg) (BASECALL_L(msg) + NODEHANDLE_L)
#define PUBSUB_RESPONSIBLENODECALL_L(msg) (BASECALL_L(msg) + PUBSUB_VECTOR_L)
#define PUBSUB_RESPONSIBLENODERESPONSE_L(msg) (BASECALL_L(msg) + NODEHANDLE_L)
#define PUBSUB_TAKEOVERSUBSPACECALL_L(msg) (BASECALL_L(msg) + PUBSUB_VECTOR_L)
#define PUBSUB_TAKEOVERSUBSPACERESPONSE_L(msg) (BASECALL_L(msg))
#define PUBSUB_SUBSCRIPTIONCALL_L(msg) (BASECALL_L(msg) + PUBSUB_SUBSPACE_L)
#define PUBSUB_SUBSCRIPTIONRESPONSE_L(msg) (BASECALL_L(msg) + PUBSUB_FLAG_L)
#define PUBSUB_UNSUBSCRIPTION_L(msg) (NODEHANDLE_L + PUBSUB_SUBSPACE_L)
#define PUBSUB_HELPCALL_L(msg) (BASECALL_L(msg) + PUBSUB_SUBSPACE_L + PUBSUB_FLAG_L)
#define PUBSUB_HELPRESPONSE_L(msg) (BASECALL_L(msg) + NODEHANDLE_L)
#define PUBSUB_INTERMEDIATECALL_L(msg) (BASECALL_L(msg) + PUBSUB_SUBSPACE_L)
#define PUBSUB_INTERMEDIATERESPONSE_L(msg) (BASECALL_L(msg))
#define PUBSUB_BACKUPCALL_L(msg) (BASECALL_L(msg) + msg->getIntermediatesArraySize()*NODEHANDLE_L + msg->getChildrenArraySize()*(NODEHANDLE_L+PUBSUB_CHILDPOS_L))
#define PUBSUB_BACKUPRESPONSE_L(msg) (BASECALL_L(msg))
#define PUBSUB_PINGCALL_L(msg) (BASECALL_L(msg) + PUBSUB_SUBSPACE_L + PUBSUB_FLAG_L)
#define PUBSUB_PINGRESPONSE_L(msg) (BASECALL_L(msg))
#define PUBSUB_REPLACEMENT_L(msg) (PUBSUB_SUBSPACE_L + NODEHANDLE_L)
#define PUBSUB_FAILEDNODE_L(msg) (IPADDR_L + UDPPORT_L)
#define PUBSUB_NODELEFT_L(msg) (PUBSUB_SUBSPACE_L + NODEHANDLE_L)
#define PUBSUB_ADOPTCHILDCALL_L(msg) (BASECALL_L(msg) + PUBSUB_SUBSPACE_L + NODEHANDLE_L)
#define PUBSUB_ADOPTCHILDRESPONSE_L(msg) (BASECALL_L(msg))
#define PUBSUB_RELEASEINTERMEDIATE_L(msg) (PUBSUB_SUBSPACE_L)
#define PUBSUB_HELPRELEASE_L(msg) (PUBSUB_SUBSPACE_L + NODEHANDLE_L)
#define PUBSUB_BACKUPSUBSCRIPTION_L(msg) (PUBSUB_SUBSPACE_L + NODEHANDLE_L*3)
#define PUBSUB_BACKUPINTERMEDIATE_L(msg) (PUBSUB_SUBSPACE_L + NODEHANDLE_L + PUBSUB_CHILDPOS_L)
#define PUBSUB_BACKUPUNSUBSCRIBE_L(msg) (PUBSUB_SUBSPACE_L + NODEHANDLE_L*2)
#define PUBSUB_MOVE_L(msg) (PUBSUB_SUBSPACE_L + NODEHANDLE_L + PUBSUB_VECTOR_L + PUBSUB_TIMESTAMP_L)
#define PUBSUB_MOVELIST_L(msg) (PUBSUB_SUBSPACE_L + PUBSUB_TIMESTAMP_L + msg->getPlayerArraySize()*(NODEHANDLE_L+PUBSUB_VECTOR_L))

}}



class noncobject NodeHandle;

class noncobject TransportAddress;

class noncobject Vector2D;

class BaseOverlayMessage;


class BaseCallMessage;


class BaseResponseMessage;



enum PubSubTimerType 
{

    PUBSUB_HEARTBEAT = 0;
    PUBSUB_CHILDPING = 1;
    PUBSUB_PARENT_TIMEOUT = 2;
    PUBSUB_TAKEOVER_GRACE_TIME = 3;
    PUBSUB_EVENTDELIVERY = 4;
}

enum PubSubHelpType
{

    PUBSUB_BACKUP = 0;
    PUBSUB_INTERMEDIATE = 1;
}

enum PubSubPingType
{

    PUBSUB_PING_BACKUP = 0;
    PUBSUB_PING_INTERMEDIATE = 1;
    PUBSUB_PING_CHILD = 2;
}

message PubSubTimer
{
        int type enum(PubSubTimerType);
        int subspaceId;
}

packet PubSubJoinCall extends BaseCallMessage
{
        Vector2D position;
        int ressources;
}

packet PubSubJoinResponse extends BaseResponseMessage
{
        NodeHandle responsibleNode;
        // FIXME: Position's missing
}

packet PubSubResponsibleNodeCall extends BaseCallMessage
{
        Vector2D subspacePos; //FIXME: send subspaceId as int
}

packet PubSubResponsibleNodeResponse extends BaseResponseMessage
{
        int subspaceId; // Not counted in length: could easily be stored locally!
        NodeHandle responsibleNode;
}

packet PubSubTakeOverSubspaceCall extends BaseCallMessage
{
        Vector2D subspacePos; //FIXME: send subspaceId as int
}

packet PubSubTakeOverSubspaceResponse extends BaseResponseMessage
{
        Vector2D subspacePos; //FIXME: send subspaceId as int // Not counted in length: could easily be stored locally!
}

packet PubSubSubscriptionCall extends BaseCallMessage
{
        int subspaceId;
}

packet PubSubSubscriptionResponse extends BaseResponseMessage
{
        bool failed = false;
}

packet PubSubUnsubscriptionMessage extends BaseOverlayMessage
{
        int subspaceId;
        NodeHandle src;
}

packet PubSubHelpCall extends BaseCallMessage
{
        int subspaceId; // Not counted in length: could easily be stored locally!
        int helpType enum(PubSubHelpType);
}

packet PubSubHelpResponse extends BaseResponseMessage
{
        int subspaceId; // Not counted in length: could easily be stored locally!
        int helpType enum(PubSubHelpType); // Not counted in length: could easily be stored locally!
        NodeHandle node;
}

packet PubSubIntermediateCall extends BaseCallMessage
{
        int subspaceId;
}

packet PubSubIntermediateResponse extends BaseResponseMessage
{
        int subspaceId; // Not counted in length: could easily be stored locally!
}

packet PubSubBackupCall extends BaseCallMessage
{
        int subspaceId;
        NodeHandle intermediates[];
        NodeHandle children[];
        int childrenPos[];
}

packet PubSubBackupResponse extends BaseResponseMessage
{
        int subspaceId; // Not counted in length: could easily be stored locally!
}

packet PubSubPingCall extends BaseCallMessage
{
        int subspaceId;
        int pingType enum(PubSubPingType);
}

packet PubSubPingResponse extends BaseResponseMessage
{
        int subspaceId; // Not counted in length: could easily be stored locally!
}

packet PubSubReplacementMessage extends BaseOverlayMessage
{
        int subspaceId;
        NodeHandle newResponsibleNode;
}

packet PubSubFailedNodeMessage extends BaseOverlayMessage
{
        TransportAddress failedNode;
}

packet PubSubNodeLeftMessage extends BaseOverlayMessage
{
        int subspaceId;
        NodeHandle node;
}

packet PubSubAdoptChildCall extends BaseCallMessage
{
        int subspaceId;
        NodeHandle child;
}

packet PubSubAdoptChildResponse extends BaseResponseMessage
{
        int subspaceId; // Not counted in length: could easily be stored locally!
        NodeHandle child; // Not counted in length: could easily be stored locally!
}

packet PubSubReleaseIntermediateMessage extends BaseOverlayMessage
{
        int subspaceId;
}

packet PubSubHelpReleaseMessage extends BaseOverlayMessage
{
        int subspaceId;
        NodeHandle node;
}

packet PubSubBackupSubscriptionMessage extends BaseOverlayMessage
{
        int subspaceId;
        NodeHandle child;
        NodeHandle parent;
        NodeHandle oldParent;
}

packet PubSubBackupIntermediateMessage extends BaseOverlayMessage
{
        int subspaceId;
        NodeHandle node;
        int pos;
}

packet PubSubBackupUnsubscribeMessage extends BaseOverlayMessage
{
        int subspaceId;
        NodeHandle child;
        NodeHandle intermediate;
}

packet PubSubMoveMessage extends BaseOverlayMessage
{
        int subspaceId;
        simtime_t timestamp;
        NodeHandle player;
        Vector2D position;
}

packet PubSubMoveListMessage extends BaseOverlayMessage
{
        int subspaceId;
        simtime_t timestamp;
        NodeHandle player[];
        Vector2D position[];
        simtime_t positionAge[];
}

