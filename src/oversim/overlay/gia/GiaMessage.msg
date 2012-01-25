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
// @author Robert Palmer, Bernhard Heep
//
cplusplus {{
#include <IPvXAddress.h>
#include <OverlayKey.h>
#include <GiaNode.h>
#include <NodeHandle.h>
#include <CommonMessages_m.h>

// constants for message length in bit
static const int GIACOMMAND_L = 8;
static const int CAPACITY_L = 32;
static const int DEGREE_L = 16;
static const int TOKENNR_L = 16;
static const int MAXRESPONSES_L = 16;
#define GIANODE_L (CAPACITY_L + DEGREE_L + NODEHANDLE_L + 2 * TOKENNR_L)

#define GIA_L(msg) (BASEOVERLAY_L(msg) + NODEHANDLE_L + HOPCOUNT_L + \
	            GIACOMMAND_L + CAPACITY_L + DEGREE_L)
#define GIANEIGHBOR_L(msg) (GIA_L(msg) + msg->getNeighborsArraySize() * GIANODE_L)
#define TOKEN_L(msg) (GIA_L(msg) + 2 * TOKENNR_L)
#define GIAID_L(msg) (GIA_L(msg) + KEY_L)
#define GIAROUTE_L(msg) (GIAID_L(msg) + KEY_L + IPADDR_L + UDPPORT_L)
#define KEYLIST_L(msg) (GIA_L(msg) + (msg->getKeysArraySize() * KEY_L))
#define SEARCH_L(msg) (GIAID_L(msg) + KEY_L + MAXRESPONSES_L + \
	               msg->getReversePathArraySize() * KEY_L + \
		       msg->getFoundNodeArraySize() * KEY_L)
#define SEARCHRESPONSE_L(msg) (GIAID_L(msg) + KEY_L + msg->getReversePathArraySize() * KEY_L + \
	                       GIANODE_L + HOPCOUNT_L)
}};


class noncobject IPvXAddress;
class noncobject OverlayKey;
class noncobject GiaNode;
class noncobject NodeHandle;
class BaseOverlayMessage;

//
// Some constants for Command-Field
// @author Robert Palmer
// @todo change names in enum
//
enum GIACommand
{
    JOIN_REQUEST = 0;    // JOIN_REQUEST-Message
    JOIN_RESPONSE = 1;    // JOIN_RESPONSE-Message
    JOIN_ACK = 2;    // JOIN_ACK-Message
    JOIN_DENY = 3;    // JOIN_DENY-Message
    DISCONNECT = 4;    // DISCONNECT-Message
    ROUTE = 5;    // ROUTE-Message
    SEARCH = 6;    // SEARCH-Message
    ANSWER = 7;    // ANSWER-Message
    UPDATE = 8;    // UPDATE-Message
    TOKEN = 9;    // TOKEN-Message
    KEYLIST= 10;    // KEYLIST-Message
};

//
// Main GIA-Message definition
// @author Robert Palmer
//
message GiaMessage extends BaseOverlayMessage
{
       NodeHandle srcNode;    // source node
       int hopCount = 0;    // hop count, increased by BaseOverlay
       int command enum(GIACommand);    // COMMAND-Field
       double srcCapacity;    // capacity of source node
       int srcDegree;    // neighbor count of source node
};

message GiaNeighborMessage extends GiaMessage
{
        GiaNode neighbors[];
};
 
//
// Used for sending a token to a destination node
// @author Robert Palmer
//
message TokenMessage extends GiaMessage
{
       int srcTokenNr;    // how many tokens received from destination node
       int dstTokenNr;    // how many tokens sent to destination node
};

//
// A GiaMessage extended by an unique ID-Field
// @author Robert Palmer
//
message GiaIDMessage extends GiaMessage
{
        OverlayKey destKey;    // destination key, may be undefined
        OverlayKey ID;    // unique message id
};

//
// Send a route message to a destination node
// @author Robert Palmer
//
message GiaRouteMessage extends GiaIDMessage
{
        OverlayKey originatorKey;    // unique key (id) of the originator of this route message
        IPvXAddress originatorIP;    // IP of the originator of this route message
        int originatorPort;    // UDP-Port of the originator of this route message
};

//
// Send a keylist to a neighbor node
// @author Robert Palmer
//
message KeyListMessage extends GiaMessage
{
       OverlayKey keys[];    // vector of search keys
};

//
// Search message 
// @author Robert Palmer
//
message SearchMessage extends GiaIDMessage
{
        OverlayKey searchKey;    // searched key
        int maxResponses;    // maximum responses to this search message
        OverlayKey reversePath[];    // vector containing all nodes which this search message passes
        OverlayKey foundNode[];    // vector containing all nodes which contain the searched key
};

//
// Search response message 
// @author Robert Palmer
//
message SearchResponseMessage extends GiaIDMessage
{
       OverlayKey searchKey;	    // searched key
       OverlayKey reversePath[];    // vector containing all remaining nodes which the search message passed
       GiaNode foundNode;    // this node contains the searched key
       int searchHopCount;    // how many hops from foundNode to the originator of the search message were necessary
};
