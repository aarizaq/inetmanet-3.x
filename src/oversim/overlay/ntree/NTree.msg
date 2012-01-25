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
// @file NTree.msg
// @author Stephan Krause
//

cplusplus {{
#include <CommonMessages_m.h>
#include <Vector2D.h>

static const int NTREEPOSITION_L = 128;
static const int NTREESIZE_L = 64;
static const int NTREEQUADRANT_L = 2;
static const int NTREECHILDCOUNT_L = 16;

#define NTREEJOINCALL_L(msg) (NTREEPOSITION_L + BASECALL_L(msg))
#define NTREEJOINRESPONSE_L(msg) (NTREEPOSITION_L + NTREESIZE_L + msg->getMembersArraySize() * NODEHANDLE_L + BASERESPONSE_L(msg))
#define NTREEDIVIDECALL_L(msg) (NTREEPOSITION_L + NTREESIZE_L + NTREEQUADRANT_L + BASECALL_L(msg))
#define NTREEDIVIDERESPONSE_L(msg) (BASERESPONSE_L(msg))
#define NTREEPINGCALL_L(msg) (NTREEPOSITION_L + NTREESIZE_L + NODEHANDLE_L + BASECALL_L(msg))
#define NTREENODEPINGCALL_L(msg) (NTREEPINGCALL_L(msg) + 4*NODEHANDLE_L + NTREEQUADRANT_L)
#define NTREEPINGRESPONSE_L(msg) (BASERESPONSE_L(msg))
#define NTREENODEPINGRESPONSE_L(msg) (NTREEPINGRESPONSE_L(msg) + msg->getMembersArraySize() * NODEHANDLE_L + NTREECHILDCOUNT_L)
#define NTREEDELETE_L(msg) (NTREEPOSITION_L + NTREESIZE_L + 4*NODEHANDLE_L + BASEOVERLAY_L(msg))
#define NTREECOLLAPSE_L(msg) (NTREEPOSITION_L + NTREESIZE_L + NODEHANDLE_L + BASEOVERLAY_L(msg))
#define NTREELEAVE_L(msg) (NTREEPOSITION_L + NODEHANDLE_L + BASEOVERLAY_L(msg))
#define NTREEMOVE_L(msg) (NTREEPOSITION_L + NODEHANDLE_L + BASEOVERLAY_L(msg))
#define NTREEADD_L(msg) (NTREEPOSITION_L + NODEHANDLE_L + BASEOVERLAY_L(msg))
#define NTREEREPLACE_L(msg) (NTREEPOSITION_L + NTREESIZE_L + (msg->getChildrenArraySize() +1)*NODEHANDLE_L + TRANSPORTADDRESS_L + 1 + BASEOVERLAY_L(msg))
#define NTREETAKEOVER_L(msg) (NTREEPOSITION_L + NTREESIZE_L + NODEHANDLE_L + BASEOVERLAY_L(msg))
}}



class noncobject NodeHandle;

class noncobject TransportAddress;

class noncobject Vector2D;

class BaseOverlayMessage;

class BaseCallMessage;

class BaseResponseMessage;

packet NTreeJoinCall extends BaseCallMessage
{
        Vector2D position;
}

packet NTreeJoinResponse extends BaseResponseMessage
{
        Vector2D origin;
        double size;
        NodeHandle members[];
}

packet NTreeDivideCall extends BaseCallMessage
{
        Vector2D origin;
        double size;
        int quadrant;
}

packet NTreeDivideResponse extends BaseResponseMessage
{
        int quadrant;
}

packet NTreePingCall extends BaseCallMessage
{
        Vector2D origin;
        double size;
        NodeHandle parent;
}

packet NTreeNodePingCall extends NTreePingCall
{
        NodeHandle siblings[4];
        int quadrant;
}

packet NTreePingResponse extends BaseResponseMessage
{
}

packet NTreeNodePingResponse extends NTreePingResponse
{
        NodeHandle members[];
        int aggChildCount;
}

packet NTreeGroupDeleteMessage extends BaseOverlayMessage
{
        Vector2D origin;
        double size;
        NodeHandle newChild[4];
}

packet NTreeCollapseMessage extends BaseOverlayMessage
{
        Vector2D origin;
        double size;
        NodeHandle player;
}

packet NTreeLeaveMessage extends BaseOverlayMessage
{
        NodeHandle player;
        Vector2D position;
}

packet NTreeMoveMessage extends BaseOverlayMessage
{
        NodeHandle player;
        Vector2D position;
}

packet NTreeGroupAddMessage extends BaseOverlayMessage
{
        NodeHandle player;
        Vector2D origin;
}

packet NTreeReplaceNodeMessage extends BaseOverlayMessage
{
        Vector2D origin;
        double size;
        NodeHandle parent;
        NodeHandle children[];
        TransportAddress oldNode;
        bool isLeaf;
}

packet NTreeTakeOverMessage extends BaseOverlayMessage
{
        Vector2D origin;
        double size;
        NodeHandle player;
}
