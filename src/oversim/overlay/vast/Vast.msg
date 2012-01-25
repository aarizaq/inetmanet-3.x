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

// @file Vast.msg
// @author Helge Backhaus
//

cplusplus {{
#include <NodeHandle.h>
#include <OverlayKey.h>
#include <Vector2D.h>
#include <CommonMessages_m.h>

// constants for message length in bit
static const int VASTCOMMAND_L = 8;
static const int POSITION_L = 128;
static const int ENTRYCOUNT_L = 32;
static const int COUNT_L = 32;
static const int FLAG_L = 8;

#define VAST_L(msg) (VASTCOMMAND_L + NODEHANDLE_L + KEY_L + POSITION_L + COUNT_L)
#define VASTLIST_L(msg) (VAST_L(msg) + ENTRYCOUNT_L + FLAG_L + msg->getNeighborNodeArraySize() * (NODEHANDLE_L + POSITION_L))
#define VASTMOVE_L(msg) (VAST_L(msg) + POSITION_L + 2 * FLAG_L)
#define VASTDISCARD_L(msg) (VAST_L(msg) + NODEHANDLE_L)
}}

class noncobject NodeHandle;
class noncobject OverlayKey;
class noncobject Vector2D;
class BaseOverlayMessage;

enum VASTCommand
{
    JOIN_REQUEST = 0;
    JOIN_ACKNOWLEDGE = 1;
    NODE_MOVE = 2;
    NEW_NEIGHBORS = 3;
    NODE_LEAVE = 4;
    ENCLOSING_NEIGHBORS_REQUEST = 5;
    BACKUP_NEIGHBORS = 6;
    PING = 7;
    PONG = 8;
    DISCARD_NODE = 9;
    VAST_EVENT = 10;
}

//
// Base message for all messages used by Vast
// JOIN_REQUEST, ENCLOSING_NEIGHBORS_REQUEST, PING, PONG
//
packet VastMessage extends BaseOverlayMessage
{
    int command enum(VASTCommand);
    NodeHandle sourceNode;
    OverlayKey destKey;
    Vector2D pos;
    int neighborCount;
}

packet VastListMessage extends VastMessage // JOIN_ACKNOWLEDGE, NEW_NEIGHBORS, NODE_LEAVE, BACKUP_NEIGHBORS
{
    NodeHandle neighborNode[];
    Vector2D neighborPos[];
    bool requestEnclosingNeighbors = false;
}

packet VastMoveMessage extends VastMessage // NODE_MOVE
{
    Vector2D newPos;
    bool is_boundary = false;
    bool request_list = false;
}

packet VastDiscardMessage extends VastMessage // DISCARD_NODE
{
    NodeHandle discardNode;
}

//FIXME: Length definition
packet VastEventMessage extends VastMessage
{
}

