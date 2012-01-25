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
// @author Ingmar Baumgart, Markus Mauch
//

cplusplus {{
#include <IPvXAddress.h>
#include <NodeHandle.h>
#include <TransportAddress.h>
#include <CommonMessages_m.h>

// constants for message length in bit
static const int CHORDCOMMAND_L = 8;
static const int SUCNUM_L = 8;
static const int FINGER_L = 8;
static const int STEP_L = 8;
static const int PRENODESET_L = 1;

#define NEWSUCCESSORHINT_L(msg) (BASEOVERLAY_L(msg) + CHORDCOMMAND_L + \
                                 2*NODEHANDLE_L)
#define NOTIFYCALL_L(msg) (BASECALL_L(msg))
#define NOTIFYRESPONSE_L(msg) (BASERESPONSE_L(msg) + SUCNUM_L + NODEHANDLE_L +\
			      (msg->getSucNodeArraySize() * NODEHANDLE_L) +\
                  PRENODESET_L)
#define JOINCALL_L(msg) BASECALL_L(msg)
#define JOINRESPONSE_L(msg) (BASERESPONSE_L(msg) + SUCNUM_L + NODEHANDLE_L +\
                             (msg->getSucNodeArraySize() * NODEHANDLE_L))
#define STABILIZECALL_L(msg) BASECALL_L(msg)
#define STABILIZERESPONSE_L(msg) (BASERESPONSE_L(msg) + NODEHANDLE_L)
#define FIXFINGERSCALL_L(msg) (BASECALL_L(msg) + FINGER_L)
#define FIXFINGERSRESPONSE_L(msg) (BASERESPONSE_L(msg) + FINGER_L + \
                                   NODEHANDLE_L + SUCNUM_L + \
                                   (msg->getSucNodeArraySize() * NODEHANDLE_L))
#define DEBRUIJNCALL_L(msg) (BASECALL_L(msg) + KEY_L)
#define DEBRUIJNRESPONSE_L(msg) (BASERESPONSE_L(msg) + SUCNUM_L + NODEHANDLE_L + \
				 (msg->getSucNodeArraySize() * NODEHANDLE_L)) 

#define KOORDEFINDNODEEXTMESSAGE_L (KEY_L + STEP_L)

}}
//todo add more Koorde message lengths

class noncobject IPvXAddress;
class noncobject NodeHandle;
class noncobject OverlayKey;
class noncobject TransportAddress;
class BaseOverlayMessage;
class BaseRpcMessage;
class BaseCallMessage;
class BaseResponseMessage;

enum ChordCommand
{

    NEWSUCCESSORHINT = 0;
}

//
// Base message for all messages used by Chord
//
packet ChordMessage extends BaseOverlayMessage
{
    int command enum(ChordCommand);    // message type (NEWSUCCESSORHINT, ...)
}

//
// Message used to inform about a new successor node
//
packet NewSuccessorHintMessage extends ChordMessage
{
    NodeHandle srcNode;    // nodehandle of the message originator
    NodeHandle preNode;    // the predecessor of the message originator
}

//
// Message used to inform about existing predecessors
//
packet NotifyCall extends BaseCallMessage
{
    TransportAddress failed = TransportAddress::UNSPECIFIED_NODE;
}

//
// Message used to answer to a NotifyCall message
//
packet NotifyResponse extends BaseResponseMessage
{
    int sucNum;    // number of entries in the successor list
    NodeHandle sucNode[];    // successor list of the message originator
    NodeHandle preNode;    // in aggressiveJoinMode: predecessor hint
    bool preNodeSet; // indicates if the dest node has been set as new predecessor (for merging)
}

//
// Message used to join a chord overlay
//
packet JoinCall extends BaseCallMessage
{
}

//
// Message used to answer to a JoinCall message
//
packet JoinResponse extends BaseResponseMessage
{
    int sucNum;    // number of entries in the successor list
    NodeHandle sucNode[];    // successor list of the message originator
    NodeHandle preNode;    // in aggressiveJoinMode: predecessor hint
}

//
// Message used for the  stabilization protocol as mentioned in 
// "Chord: A Scalable Peer-to-Peer Lookup Protocol for Internet Applications"
//
packet StabilizeCall extends BaseCallMessage
{
}

//
// Message used to answer to a StabilizeCall message
//
packet StabilizeResponse extends BaseResponseMessage
{
    NodeHandle preNode;    // the predecessor of the message originator
}

//
// Message used to verify our ChordFingerTable
//
packet FixfingersCall extends BaseCallMessage
{
    int finger;    // the number of the finger to update
}

//
// Message used to answer to a FixfingersCall message
//
packet FixfingersResponse extends BaseResponseMessage
{
    int finger;    // the number of the finger to update
    NodeHandle sucNode[];    // the successor of the message originator
}

// ----------- Koorde -------------------

//
// Message used to find nodes in the Koorde overlay
//
packet KoordeFindNodeExtMessage
{
    OverlayKey routeKey;    // the OverlayKey of the next hop node
    int step;             // the de bruijn routing step
}

packet DeBruijnCall extends BaseCallMessage
{
    OverlayKey destKey;    // the OverlayKey of the destination node
}

//
// Message used to answer to a DeBruijnCall message
//
packet DeBruijnResponse extends BaseResponseMessage
{
    NodeHandle dBNode;
    int sucNum;    // number of entries in the successor list
    NodeHandle sucNode[];   // successor list of the message originator
}

