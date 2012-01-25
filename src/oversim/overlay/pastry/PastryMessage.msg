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
// @author Felix Palmen
//

cplusplus {{
#include <IPvXAddress.h>
#include <NodeHandle.h>
#include <OverlayKey.h>
#include <CommonMessages_m.h>

// constants for message length in bit
static const int PASTRYTYPE_L = 8;
static const int PASTRYSTATETYPE_L = 8;
static const int PASTRYREQUESTTYPE_L = 0;
static const int LASTHOPFLAG_L = 8;
static const int TIMESTAMP_L = 32;
static const int PASTRYFINDNODEEXTDATA_L = NODEHANDLE_L + HOPCOUNT_L;

#define PASTRY_L(msg) (BASEOVERLAY_L(msg) + PASTRYTYPE_L)
#define PASTRYJOIN_L(msg) (PASTRY_L(msg) + TRANSPORTADDRESS_L + HOPCOUNT_L)
#define PASTRYREQ_L(msg) (PASTRY_L(msg) + TRANSPORTADDRESS_L + PASTRYREQUESTTYPE_L)
#define PASTRYSTATE_L(msg) (PASTRY_L(msg) + TRANSPORTADDRESS_L \
        + NODEHANDLE_L * \
        (msg->getRoutingTableArraySize() + msg->getLeafSetArraySize() \
        + msg->getNeighborhoodSetArraySize()) + ARRAYSIZE_L + HOPCOUNT_L + \
        LASTHOPFLAG_L + TIMESTAMP_L)
#define PASTRYNEWLEAFS_L(msg) (msg->getLeafsArraySize() * NODEHANDLE_L + \
        ARRAYSIZE_L)
#define PASTRYLEAFSET_L(msg) (PASTRY_L(msg) + TRANSPORTADDRESS_L \
        + NODEHANDLE_L * msg->getLeafSetArraySize() + ARRAYSIZE_L)
#define PASTRYRTREQ_L(msg) (PASTRY_L(msg) + TRANSPORTADDRESS_L)
#define PASTRYRTABLE_L(msg) (PASTRY_L(msg) + TRANSPORTADDRESS_L \
        + NODEHANDLE_L * msg->getRoutingTableArraySize() + ARRAYSIZE_L)
}}


class noncobject IPvXAddress;
class noncobject TransportAddress;
class noncobject NodeHandle;
class noncobject OverlayKey;

class BaseOverlayMessage;
class BaseCallMessage;
class BaseResponseMessage;
class BaseAppDataMessage;


enum PastryMsgType
{
    PASTRY_MSG_STD = 0;
    PASTRY_MSG_JOIN = 1;
    PASTRY_MSG_STATE = 2;
    PASTRY_MSG_REQ = 3;
    PASTRY_MSG_LEAFSET = 4;	
    PASTRY_MSG_ROWREQ = 5;
    PASTRY_MSG_RROW = 6;
    PASTRY_MSG_LEAFSET_PULL = 7;
    PASTRY_REQ_REPAIR = 8;
    PASTRY_REQ_STATE = 9;
    PASTRY_REQ_LEAFSET = 10;
}

enum PastryStateMsgType
{
    PASTRY_STATE_STD = 0;
    PASTRY_STATE_JOIN = 1;
    PASTRY_STATE_MINJOIN = 2;
    PASTRY_STATE_UPDATE = 3;
    PASTRY_STATE_REPAIR = 4;
    PASTRY_STATE_JOINUPDATE = 5;
}


//
// Base message for all messages used by Pastry
//
packet PastryMessage extends BaseOverlayMessage
{
        int pastryMsgType = PASTRY_MSG_STD;    // the PastryMsgType
}

//
// Message used to join a pastry overlay
//
packet PastryJoinMessage extends PastryMessage
{
        TransportAddress sendStateTo = TransportAddress::UNSPECIFIED_NODE;    // TransportAddress of the node that wants to join
}


//
// Message used to send a PastryState
//
packet PastryStateMessage extends PastryMessage
{
        int pastryStateMsgType = PASTRY_STATE_STD;    // the type of the PastryStateMessage
        NodeHandle sender = NodeHandle::UNSPECIFIED_NODE;    // NodeHandle of the node sending this message
        NodeHandle routingTable[];    // the routingTable of the sender
        NodeHandle leafSet[];    // the leafSet of the sender
        NodeHandle neighborhoodSet[];    // the neighborhoodSet of the sender
        int joinHopCount = 0;    // counts the hops this message takes
        bool lastHop = false;    // is this node the destination node?
        simtime_t timestamp;    // simTime when sending this message
}

//
// Message used to find a Pastry node
//
packet PastryFindNodeExtData
{
        TransportAddress sendStateTo = TransportAddress::UNSPECIFIED_NODE;    // the sender of this message
        int joinHopCount = 0;    // counts the hops this message takes
}

//
// Message used to inform about new PastryLeafs
//
packet PastryNewLeafsMessage
{
        NodeHandle leafs[];    // the new PastryLeafs
}


packet PastrySendState
{
        TransportAddress dest = TransportAddress::UNSPECIFIED_NODE;
}

packet PastryLeafsetMessage extends PastryMessage
{
        NodeHandle sender = NodeHandle::UNSPECIFIED_NODE;
        TransportAddress sendStateTo = TransportAddress::UNSPECIFIED_NODE;
        NodeHandle leafSet[];
        simtime_t timestamp = 0;
}

packet PastryRoutingRowRequestMessage extends PastryMessage
{
        TransportAddress sendStateTo = TransportAddress::UNSPECIFIED_NODE;
        int row;

}
packet PastryRoutingRowMessage extends PastryMessage
{
        NodeHandle sender = NodeHandle::UNSPECIFIED_NODE;
        NodeHandle routingTable[];
        int row;
        simtime_t timestamp = 0;
}

//
// Message used to request PastryState or a Leafset
//
packet PastryRequestMessage extends PastryMessage
{
        TransportAddress sendStateTo = TransportAddress::UNSPECIFIED_NODE;    // TransportAddress of the node that requests the state
        int pastryReqType = PASTRY_MSG_STD;    // the PastryMsgType
}


