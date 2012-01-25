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

// @file Quon.msg
// @author Helge Backhaus
// @author Stephan Krause
//

cplusplus {{
#include <NodeHandle.h>
#include <QuonDefs.h>
#include <QuonHelper.h>
#include <CommonMessages_m.h>

// constants for message length in bit
static const int QUONCOMMAND_L = 8;
static const int QUONPOSITION_L = 128;
static const int QUONAOI_L = 64;
static const int QUONENTRYCOUNT_L = 32;
static const int QUONFLAG_L = 1;

#define QUON_L(msg) (QUONCOMMAND_L + QUONENTRY_L + KEY_L + QUONAOI_L)
#define QUONLIST_L(msg) (QUON_L(msg) + QUONENTRYCOUNT_L + msg->getNeighborHandleArraySize() * QUONENTRY_L)
#define QUONMOVE_L(msg) (QUONLIST_L(msg) + QUONPOSITION_L + QUONFLAG_L)
#define QUONENTRY_L (NODEHANDLE_L + QUONPOSITION_L)
}}

class noncobject NodeHandle;
class noncobject OverlayKey;
class noncobject Vector2D;
class BaseOverlayMessage;

enum QUONCommand
{
    JOIN_REQUEST = 0;
    JOIN_ACKNOWLEDGE = 1;
    NODE_MOVE = 2;
    NEW_NEIGHBORS = 3;
    NODE_LEAVE = 4;
    QUON_EVENT = 5;
}

// Quon self-messages

message QuonSelfMessage
{
    OverlayKey key;
}

// Quon overlay-messages

packet QuonMessage extends BaseOverlayMessage // JOIN_REQUEST
{
    int command enum(QUONCommand);
    NodeHandle sender;
    Vector2D position;
    double AOIsize;
}

packet QuonListMessage extends QuonMessage // JOIN_ACKNOWLEDGE, NEW_NEIGHBORS, NODE_LEAVE
{
    NodeHandle neighborHandle[];
    Vector2D neighborPosition[];
}

packet QuonMoveMessage extends QuonListMessage // NODE_MOVE
{
    Vector2D newPosition;
    bool isBinding = false;
}

packet QuonEventMessage extends QuonMessage
{
}
