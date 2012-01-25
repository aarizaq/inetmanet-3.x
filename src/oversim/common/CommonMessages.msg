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

cplusplus {{
#include <OverlayKey.h>
#include <NodeHandle.h>
#include <IPvXAddress.h>
#include <TransportAddress.h>
#include <BinaryValue.h>
#include <Vector2D.h>
#include <OverSimMessage.h>

#define KEY_L OverlayKey::getLength()

static const int TYPE_L = 8;
static const int IPADDR_L = 32;
static const int UDPPORT_L = 16;
static const int HOPCOUNT_L = 16;
static const int NONCE_L = 32;
static const int COMP_L = 16;
static const int NUMSIBLINGS_L = 8;
static const int NUMREDNODES_L = 8;
static const int EXHAUSTIVEFLAG_L = 8;
static const int NEIGHBORSFLAG_L = 8;
static const int RESPONSEFLAG_L = 8;
static const int TRYAGAINFLAG_L = 8;
static const int TIER_L = 8;
static const int ARRAYSIZE_L = 8;
static const int ROUTINGTYPE_L = 8;
static const int SIGNATURE_L = 40*8;
static const int CERT_L = 40*8;
static const int PUBKEY_L = 20*8;
static const int NCSINFO_L = 32;

#define NODEHANDLE_L (IPADDR_L + UDPPORT_L + KEY_L)
// TODO: TRANSPORTADDRESS_L doesn't contain NAT type at the moment, since this
// is work in progress and currently not used
#define TRANSPORTADDRESS_L (IPADDR_L + UDPPORT_L)

#define AUTHBLOCK_L (overlay->getMeasureAuthBlock() ? (SIGNATURE_L + CERT_L + PUBKEY_L) : 0)

#define BASEOVERLAY_L(msg) TYPE_L
#define BASEROUTE_L(msg) (BASEOVERLAY_L(msg) + NODEHANDLE_L + KEY_L + \
                          HOPCOUNT_L + ROUTINGTYPE_L +\
                          ARRAYSIZE_L + (msg->getVisitedHopsArraySize() *\
                          TRANSPORTADDRESS_L) +\
                          ARRAYSIZE_L + (msg->getNextHopsArraySize() *\
                          TRANSPORTADDRESS_L) +\
                          ARRAYSIZE_L + (msg->getHintsArraySize() *\
                          NODEHANDLE_L))
#define BASEAPPDATA_L(msg) (BASEOVERLAY_L(msg) + 2*COMP_L)
#define BASERPC_L(msg) (BASEOVERLAY_L(msg) + NONCE_L + NODEHANDLE_L + \
	                TIER_L)
#define BASECALL_L(msg) BASERPC_L(msg)
//TODO add field for call hop count ?
#define BASERESPONSE_L(msg) (BASERPC_L(msg) + AUTHBLOCK_L + \
                             ((msg->getNcsInfoArraySize()) ? \
                              (msg->getNcsInfoArraySize() * NCSINFO_L + 8) : \
                              0))
#define FINDNODECALL_L(msg) (BASECALL_L(msg) + KEY_L + NUMSIBLINGS_L + \
                             NUMREDNODES_L + EXHAUSTIVEFLAG_L)
//TODO add field for closestNodesArraySize
#define FINDNODERESPONSE_L(msg) (BASERESPONSE_L(msg) + NEIGHBORSFLAG_L + \
                  (msg->getClosestNodesArraySize() * NODEHANDLE_L))
#define FAILEDNODECALL_L(msg) (BASECALL_L(msg) + IPADDR_L + UDPPORT_L)
#define FAILEDNODERESPONSE_L(msg) (BASERESPONSE_L(msg) + TRYAGAINFLAG_L)
#define PINGCALL_L(msg) BASECALL_L(msg)
#define PINGRESPONSE_L(msg) BASERESPONSE_L(msg)
#define BOOTSTRAPPINGCALL_L(msg) BASECALL_L(msg)
#define BOOTSTRAPPINGRESPONSE_L(msg) BASERESPONSE_L(msg)
#define NEXTHOPCALL_L(msg) BASECALL_L(msg)
#define NEXTHOPRESPONSE_L(msg) BASERESPONSE_L(msg)
}}

class OverSimMessage;
class BaseOverlayMessage;
class BaseRpcMessage;
class BaseCallMessage;
class BaseResponseMessage;

class noncobject NodeHandle;
class noncobject TransportAddress;
class noncobject OverlayKey;
class noncobject IPvXAddress;
class noncobject Vector2D;
class noncobject BinaryValue;
class noncobject CompType;

enum BaseOverlayMessageType
{

    OVERLAYSIGNALING = 0;
    RPC = 1;
    APPDATA = 2;
    OVERLAYROUTE = 3;
}

enum CompType
{

    INVALID_COMP = 0;
    OVERLAY_COMP = 1;
    TIER1_COMP = 2;
    TIER2_COMP = 3;
    TIER3_COMP = 4;
    BOOTSTRAPLIST_COMP = 5;
    NEIGHBORCACHE_COMP = 6;
}

enum RoutingType
{

    NO_OVERLAY_ROUTING = 0;
    DEFAULT_ROUTING = 1;
    ITERATIVE_ROUTING = 2;
    EXHAUSTIVE_ITERATIVE_ROUTING = 3;
    SEMI_RECURSIVE_ROUTING = 4;
    FULL_RECURSIVE_ROUTING = 5;
    RECURSIVE_SOURCE_ROUTING = 6;
    PROX_AWARE_ITERATIVE_ROUTING = 7;
}

enum TransportType
{

    INVALID_TRANSPORT = 0;
    INTERNAL_TRANSPORT = 1;
    UDP_TRANSPORT = 2;
    ROUTE_TRANSPORT = 3;
}

enum CommonAPIMessageType
{

    COMMON_API_UNDEFINED = 0;
    KBR_ROUTE = 1;
    KBR_DELIVER = 2;
    KBR_FORWARD = 3;
    KBR_FORWARD_RESPONSE = 4;
    KBR_UPDATE = 5;
}

enum StatType
{

    MAINTENANCE_STAT = 0;
    APP_DATA_STAT = 1;
    APP_LOOKUP_STAT = 2;

}

class AuthBlock
{
    BinaryValue signature;
    BinaryValue pubKey;
    BinaryValue cert;
}

//
// Base class for all messages handled by overlay modules
//
// @author Bernhard Heep
// @see ChordMessage
//
packet BaseOverlayMessage extends OverSimMessage
{
    int type enum(BaseOverlayMessageType) = OVERLAYSIGNALING;  // message type
    int statType enum(StatType) = MAINTENANCE_STAT; // message type for stats
}

packet BaseRouteMessage extends BaseOverlayMessage
{
    OverlayKey destKey;             // destination key
    NodeHandle srcNode;             // source node
    int routingType enum(RoutingType); // routing type
    int hopCount = 0;               // hop count, increased by BaseOverlay
    TransportAddress visitedHops[]; // hops for source routing
    TransportAddress nextHops[];    // hops for source routing
    NodeHandle hints[];             // hints for next hop (optional)
    simtime_t hopStamp;             // timestamp of processing at last hop
    simtime_t hopDelay[];           // vector of single-hop latency (for stats)
}

packet BaseAppDataMessage extends BaseOverlayMessage
{
    int destComp enum(CompType); // destination component
    int srcComp enum(CompType);  // source component
}

//
// A basic Remote-Procedure-Call message used for calls and return values
//
packet BaseRpcMessage extends BaseOverlayMessage
{
    unsigned int nonce;  // nonce to match RPC responses to corresponding calls
    NodeHandle srcNode;  // source node
    AuthBlock authBlock[] @sizetype(uint8_t); // optional authentication block containing a signature
}

//
// A basic Remote-Procedure-Call message
//
packet BaseCallMessage extends BaseRpcMessage
{
}

//
// A basic Remote-Procedure-Response message
//
packet BaseResponseMessage extends BaseRpcMessage
{
    int callHopCount = 0;
    double ncsInfo[]; // info block for network coordinate systems
}

//
// The RPC Timeout self-message
//
packet RpcTimeoutMessage extends BaseRpcMessage
{
}

//
// A basic find node rpc response
//
packet FindNodeCall extends BaseCallMessage
{
    OverlayKey lookupKey;   // request nextHops for this key
    int numRedundantNodes;  // number of redundant nodes to return
    int numSiblings;        // number of siblings to return
    bool exhaustiveIterative = false; // for exhaustive-iterative always request numRedudantNode, even if node is a sibling
}

//
// A basic find node rpc call
//
packet FindNodeResponse extends BaseResponseMessage
{
    // the closestNodes[] vector contains all sibling for the lookup key
    bool siblings;
    NodeHandle closestNodes[];  // vector of known next hops to the lookup key
}

//
// A basic failed node notification
//
packet FailedNodeCall extends BaseCallMessage
{
    TransportAddress failedNode;
}

//
// A basic failed node response
//
packet FailedNodeResponse extends BaseResponseMessage
{
    bool tryAgain;
}

//
// A basic ping rpc call
//
packet PingCall extends BaseCallMessage
{
}

//
// A basic ping response
//
packet PingResponse extends BaseResponseMessage
{
}

//
// Notification that a component finished its init phase
//
packet CompReadyMessage
{
    bool ready;              // true, if the component is ready
    int comp enum(CompType); // the component, which is ready 
}

//
// Internal message for the BootstrapList lookup listener
//
packet BootstrapLookupMessage extends BaseOverlayMessage
{
}

//
// A next hop rpc call
//
packet NextHopCall extends BaseCallMessage
{
}

//
// A next hop response / acknowledgement
//
packet NextHopResponse extends BaseResponseMessage
{
}



// -------------------------------------------------------------
// Common API messages (internal messages)
// -------------------------------------------------------------


//
// Base class for all common API message types
//
// @author Bernhard Heep
//
packet CommonAPIMessage
{
    int type enum(CommonAPIMessageType) = COMMON_API_UNDEFINED;  // message type
}

//
// Message type for KBR route calls
//
// @author Bernhard Heep
//
packet KBRroute extends CommonAPIMessage
{
    OverlayKey destKey;                // the destination key
    int destComp enum(CompType);       // destination component
    int srcComp enum(CompType);        // source component
    TransportAddress sourceRoute[];    // source route or hint
    int routingType enum(RoutingType); // routing type
}

//
// Message type for KBR forward calls
//
// @author Bernhard Heep
//
packet KBRforward extends CommonAPIMessage
{
    OverlayKey destKey;     // the destination key
    NodeHandle nextHopNode; // the proposed/changed next hop node
}

//
// Message type for information sent from the overlay to the application
//
// @author Bernhard Heep
//
packet KBRdeliver extends CommonAPIMessage
{
    OverlayKey destKey; // destination overlay key
}

//
// Message type to inform overlay nodes about new or left nodes
//
// @author Bernhard Heep
//
packet KBRupdate extends CommonAPIMessage
{
    NodeHandle node; // the node that has joined or left the overlay
    bool joined; // true if node has joined, false if node has left
}

//
// Data type for DHT CAPI messages
//
// @author Ingmar Baumgart
//
class DhtDumpEntry
{
    OverlayKey key;
    uint32_t kind;
    uint32_t id;
    BinaryValue value;
    int ttl;
    NodeHandle ownerNode;
    bool is_modifiable;
    bool responsible;
}

//
// Message type to store value at the keys root
//
// @author Ingmar Baumgart
//
packet DHTputCAPICall extends BaseCallMessage
{
    OverlayKey key; // overlayKey that corresponds with value
    uint32_t kind = 1; // the kind of the stored data item
    uint32_t id = 1; // the id to identify multiple date items with same key and kind 
    BinaryValue value; // value that has to be stored on keys root
    int ttl; // time to life
    bool isModifiable; // true, if the data item may be overwritten by other nodes
}

//
// Message type to order the value corresponding to the OverlayKey key from the node responsible of key
//
// @author Ingmar Baumgart
//
packet DHTgetCAPICall extends BaseCallMessage
{
    OverlayKey key; // corresponding OverlayKey to the requested value
    uint32_t kind = 1; // the kind of the stored data item
    uint32_t id = 1; // the id to identify multiple date items with same key and kind 
}

//
// Message type to respond to a DHTput request @see DHTput
//
// @author Ingmar Baumgart
//
packet DHTputCAPIResponse extends BaseResponseMessage
{
    bool isSuccess;
}

//
// Message type to respond to a DHTget request @see DHTget
//
// @author Ingmar Baumgart
//
packet DHTgetCAPIResponse extends BaseResponseMessage
{
    DhtDumpEntry result[]; 
    bool isSuccess;
}

//
// Request to dump all local DHT records
//
// @author Ingmar Baumgart
//
packet DHTdumpCall extends BaseCallMessage
{
}

//
// Response to a DHTdumpCall
//
// @author Ingmar Baumgart
//
packet DHTdumpResponse extends BaseResponseMessage
{
    DhtDumpEntry record[]; // array of local dht entries
}



//
// Internal RPC to ask overlay to start a lookup
// @author Ingmar Baumgart
//
packet LookupCall extends BaseCallMessage
{
    OverlayKey key;
    int numSiblings;
    int routingType enum(RoutingType) = DEFAULT_ROUTING;
}

//
// Internal RPC response from overlay containing lookup results
// @author Ingmar Baumgart
//
packet LookupResponse extends BaseResponseMessage
{
    OverlayKey key;        // the lookup key
    int hopCount = 0;      // the accumulated hop count for this lookup
    bool isValid;          // true if this lookup finished successfully
    NodeHandle siblings[]; // the siblings for the key (closest nodes=
}

//
// P2PNS Register RPC Call
//
// @author Ingmar Baumgart
//
packet P2pnsRegisterCall extends BaseCallMessage
{
    BinaryValue p2pName; // the name to register
    uint32_t kind = 1; // the kind of the stored data item
    uint32_t id = 1;  // the id of the stored data item
    BinaryValue address; // the current address of the name
    int ttl;             // time to life
}

//
// P2PNS Register RPC Response
//
// @author Ingmar Baumgart
//
packet P2pnsRegisterResponse extends BaseResponseMessage
{
    BinaryValue p2pName;  // the name to register
    BinaryValue address;  // the current address of the name
    bool isSuccess;       // true, if the name was successfully registered
}

//
// P2PNS Resolve RPC Call
//
// @author Ingmar Baumgart
//
packet P2pnsResolveCall extends BaseCallMessage
{
    BinaryValue p2pName;    // the name to resolve
    uint32_t kind = 1; // the kind of the name
    uint32_t id = 1; // the id of the name 
}

//
// P2PNS Resolve RPC Response
//
// @author Ingmar Baumgart
//
packet P2pnsResolveResponse extends BaseResponseMessage
{
    BinaryValue p2pName;  // the name to resolve
    BinaryValue address[];  // the current address of the name
    uint32_t kind[];
    uint32_t id[];
    bool isSuccess;       // true, if the name was successfully resolved
}


//
// Control info for internal handling BaseOverlaMessages.
//
// @author Bernhard Heep
//
class OverlayCtrlInfo
{
    TransportAddress lastHop;    // last hop node
    NodeHandle srcNode;          // the message's source node IP/port/nodeID
    NodeHandle srcRoute;         // like srcNode, but contains the source route to the destination (if available) 
    int hopCount = 0;            // hop count
    TransportAddress visitedHops[];     // hops on route
    int srcComp enum(CompType);  // source component
    int destComp enum(CompType); // destination component
    int transportType enum(TransportType) = INVALID_TRANSPORT; // transport type of the received message
    int routingType enum(RoutingType);
}

//
// Base message for ALM communication
//
// @author Stephan Krause
//
packet ALMMessage
{
    OverlayKey groupId;
}

//
// Create a multicast group
//
// @author Stephan Krause
//
packet ALMCreateMessage extends ALMMessage
{
}

//
// Delete a multicast group
//
// @author Stephan Krause
//
packet ALMDeleteMessage extends ALMMessage
{
}

//
// Subscribe to a multicast group
//
// @author Stephan Krause
//
packet ALMSubscribeMessage extends ALMMessage
{
}

//
// Leave a multicast group
//
// @author Stephan Krause
//
packet ALMLeaveMessage extends ALMMessage
{
}

//
// Send a multicast message to all group members
//
// @author Stephan Krause
//
packet ALMMulticastMessage extends ALMMessage
{
}

//
// Send a message to a (random) member in the group
//
// @author Stephan Krause
//
packet ALMAnycastMessage extends ALMMessage
{
}

//
// Base classes for all overlay / gameclient communication
//
// @author Helge Backhaus
//
enum GameAPIMessageCommand
{

    MOVEMENT_INDICATION = 0;
    MOVEMENT_REQUEST = 1;
    NEIGHBOR_UPDATE = 2;
    RESIZE_AOI = 3;
    GAMEEVENT_CHAT = 4;
    GAMEEVENT_SNOW = 5;
    GAMEEVENT_FROZEN = 6;
}

packet GameAPIMessage extends BaseOverlayMessage // MOVEMENT_REQUEST
{
    int command enum(GameAPIMessageCommand);
}

packet GameAPIListMessage extends GameAPIMessage // NEIGHBOR_UPDATE
{
    NodeHandle removeNeighbor[];
    NodeHandle addNeighbor[];
    Vector2D neighborPosition[];
}

packet GameAPIPositionMessage extends GameAPIMessage // MOVEMENT_INDICATION
{
    Vector2D position;
}

packet GameAPIResizeAOIMessage extends GameAPIMessage
{
    double AOIsize;
}

packet GameAPIChatMessage extends GameAPIMessage
{
    NodeHandle src;
    string msg;
}

packet GameAPISnowMessage extends GameAPIMessage
{
    NodeHandle src;
    Vector2D start;
    Vector2D end;
    int timeSec;
    int timeUsec;
}

packet GameAPIFrozenMessage extends GameAPIMessage
{
    NodeHandle src;
    int thrower;	
    int timeSec;
    int timeUsec;
}

