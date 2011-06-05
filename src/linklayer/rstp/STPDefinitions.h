/*
 * STPDefinitions.h
 *
 *  Created on: Dec 18, 2009
 *      Author: jcm
 */

#ifndef STPDEFINITIONS_H_
#define STPDEFINITIONS_H_

#include "MACAddress.h"

enum BPDUType
{
    CONF_BPDU, // Configuration Bridge Protocol Data Unit (BPDU)
    TCN_BPDU  // Topology Change Notification BPDU
};

enum PortState
{
    BLOCKING,
    LISTENING,
    LEARNING,
    FORWARDING
};

enum PortRole
{
    ROOT_PORT,           // port to reach the root bridge
    DESIGNATED_PORT,     // port to reach child bridges
    NONDESIGNATED_PORT,  // port no nowhere, usually blocked
    ALTERNATE_PORT,      // alternate port to reach bridges
    BACKUP_PORT,         // backup (redundancy) port to reach bridges
    EDGE_PORT            // Edge port. port connected to a Station
};

struct BridgeID
{
    int priority;
    MACAddress address;
};

struct PriorityVector
{
    BridgeID root_id;
    int root_path_cost;
    BridgeID bridge_id;
    int port_id;

    PriorityVector()
    {
        root_path_cost = 0;
        port_id = 0;
    }

    PriorityVector(BridgeID rid, int cost, BridgeID bid, int port)
    {
        root_id = rid;
        root_path_cost = cost;
        bridge_id = bid;
        port_id = port;
    }
};

std::ostream& operator << (std::ostream& os, const BPDUType& t );
std::ostream& operator << (std::ostream& os, const PortRole& r );

std::ostream& operator << (std::ostream& os, const BridgeID& bid );
std::ostream& operator << (std::ostream& os, const PriorityVector& pr );

bool operator < (const MACAddress mac1, const MACAddress mac2);
bool operator > (const MACAddress mac1, const MACAddress mac2);

bool operator < (BridgeID bid1,BridgeID bid2);
bool operator > (BridgeID bid1,BridgeID bid2);
bool operator == (const BridgeID bid1, const BridgeID bid2);
bool operator != (const BridgeID bid1, const BridgeID bid2);
bool operator == (const PriorityVector v1, const PriorityVector v2);
bool operator < (const PriorityVector v1, const PriorityVector v2);
bool operator > (const PriorityVector v1, const PriorityVector v2);

#endif /* STPDEFINITIONS_H_ */
