//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
// @author Christian Huebsch
//

cplusplus {{

#include <CommonMessages_m.h>
#include <NodeHandle.h>
	
#define TRANSPORTADDRESS_L (IPADDR_L + UDPPORT_L)

static const int NICECOMMAND_L = 8;
static const int LAYER_L = 16;
static const int INT_L = 32;
static const int SIMTIME_L = 32;
static const int BOOL_L = 8;
static const int DOUBLE_L = 64;
static const int SHORT_L = 16;
	
#define NICEMESSAGE_L(msg) (BASEOVERLAY_L(msg) + NICECOMMAND_L + TRANSPORTADDRESS_L + LAYER_L)
#define NICEMEMBERMESSAGE_L(msg) (NICEMESSAGE_L(msg) + (msg->getMembersArraySize() * TRANSPORTADDRESS_L))
#define NICECLUSTERMERGE_L(msg) (NICEMEMBERMESSAGE_L(msg) + TRANSPORTADDRESS_L)
#define NICEHEARTBEAT_L(msg) (NICEMEMBERMESSAGE_L(msg) + (msg->getDistancesArraySize() * SIMTIME_L) + INT_L + INT_L + SIMTIME_L + INT_L)
#define NICELEADERHEARTBEAT_L(msg) (NICEHEARTBEAT_L(msg) + TRANSPORTADDRESS_L + (msg->getSupercluster_membersArraySize() * TRANSPORTADDRESS_L) + INT_L)
#define NICEMULTICAST_L(msg) (NICEMESSAGE_L(msg) + TRANSPORTADDRESS_L + DOUBLE_L + INT_L + INT_L)

}}

class BaseOverlayMessage;

//class noncobject McpoHost;
class noncobject TransportAddress;

enum NICECommand
{
    NICE_QUERY = 0; // Querying Memberships in Clusters
    NICE_QUERY_RESPONSE = 1; // Responding to a Query
    NICE_JOIN_CLUSTER = 2; // Join Cluster in specific layer
    NICE_HEARTBEAT = 3; // Periodic Heartbeat message
    NICE_LEADERHEARTBEAT = 4; // Periodic Heartbeat message
    NICE_LEADERTRANSFER = 5; // Leader Transfer 
    NICE_JOINEVAL = 6; // Distance evaluation when joining
    NICE_JOINEVAL_RESPONSE = 7; // Response to JoinEval
    NICE_REMOVE = 8; // Removes node from cluster
    NICE_PING_PROBE = 9; // Removes node from cluster
    NICE_PING_PROBE_RESPONSE = 10; // Removes node from cluster
    NICE_CLUSTER_MERGE_REQUEST = 11; // Requesting Cluster Merge
    NICE_PEER_TEMPORARY = 12; // Temporary peer while joining
    NICE_PEER_TEMPORARY_RELEASE = 13; // Temporary peering release
    NICE_POLL_RP = 14; // Poll existing RP
    NICE_POLL_RP_RESPONSE = 15; // Respond to RP Poll
    NICE_FORCE_MERGE = 16; // Force Merge to SourceNode
    NICE_MULTICAST = 17; // Multicast data
}

message NiceMessage extends BaseOverlayMessage
{
    int command enum(NICECommand);
    TransportAddress srcNode;
    short layer;
}

message NiceMemberMessage extends NiceMessage
{
    TransportAddress members[];
}

message NiceClusterMerge extends NiceMemberMessage
{
    TransportAddress newClusterLeader;
}

message NiceHeartbeat extends NiceMemberMessage
{
    unsigned int seqNo;
    unsigned int seqRspNo;
    double hb_delay;
    double distances[];
    double one_hop_distance;
    unsigned int sublayermembers;
}

message NiceLeaderHeartbeat extends NiceHeartbeat
{
    TransportAddress supercluster_leader;
    TransportAddress supercluster_members[];
    unsigned int k;
    unsigned int sc_tolerance;
}


message NiceMulticastMessage extends NiceMessage
{
    TransportAddress lastHop;
    double sendTime;
    unsigned int seqNo;
    unsigned int hopCount;
}

