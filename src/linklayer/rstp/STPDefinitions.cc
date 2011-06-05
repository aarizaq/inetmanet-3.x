/*
 * STPDefinitions.cc
 *
 *  Created on: Dec 18, 2009
 *      Author: jcm
 */

#include "STPDefinitions.h"


std::ostream& operator << (std::ostream& os, const BPDUType& t )
{

    switch (t)
    {
    case CONF_BPDU:
        os << "CONF_BPDU";
        break;
    case TCN_BPDU:
        os << "TCN_BPDU";
        break;;
    }
    return os;
}

std::ostream& operator << (std::ostream& os, const PortRole& r )
{

    switch (r)
    {
    case ROOT_PORT:
        os << "ROOT_PORT";
        break;
    case DESIGNATED_PORT:
        os << "DESIGNATED_PORT";
        break;;
    case NONDESIGNATED_PORT:
        os << "NONDESIGNATED_PORT";
        break;;
    case ALTERNATE_PORT:
        os << "ALTERNATE_PORT";
        break;;
    case BACKUP_PORT:
        os << "BACKUP_PORT";
        break;;
    case EDGE_PORT:
        os << "EDGE_PORT";
        break;;
    }

    return os;
}

std::ostream& operator << (std::ostream& os, const BridgeID& bid )
{
    os << bid.priority << "." << bid.address;
    return os;
}

std::ostream& operator << (std::ostream& os, const PriorityVector& pr )
{
    os << "RootID: " << pr.root_id << " Cost: " << pr.root_path_cost << " BridgeID: " << pr.bridge_id << " Port: " << pr.port_id;
    return os;
}

bool operator < (MACAddress mac1,MACAddress mac2)
{

    for (int i=0; i<6; i++)
    {
        long int l1 = mac1.getAddressByte(i) & 0xFF;
        long int l2 = mac2.getAddressByte(i) & 0xFF;

        if (l1 < l2)
        {
            return true;
        }
        else if (l1 > l2)
        {
            return false;
        }
    }
    return false;
}

bool operator > (MACAddress mac1,MACAddress mac2)
{

    for (int i=0; i<6; i++)
    {
        long int l1 = mac1.getAddressByte(i) & 0xFF;
        long int l2 = mac2.getAddressByte(i) & 0xFF;

        if (l1 > l2)
        {
            return true;
        }
        else if (l1 < l2)
        {
            return false;
        }
    }

    return false;
}

bool operator < (BridgeID bid1,BridgeID bid2)
{

    if (bid1.priority > bid2.priority)
    {
        return true;
    }
    else
    {
        // only compare the mac address if the priority are equals
        if (bid1.priority == bid2.priority)
        {
            if (bid1.address > bid2.address)
            {
                return true;
            }
        }
    }

    return false;
}

bool operator > (BridgeID bid1, BridgeID bid2)
{

    if (bid1.priority < bid2.priority)
    {
        return true;
    }
    else
    {
        // only compare the mac address if the priority are equals

        if (bid1.priority == bid2.priority)
        {
            if (bid1.address < bid2.address)
            {
                return true;
            }
        }
    }

    return false;
}

bool operator == (const BridgeID bid1, const BridgeID bid2)
{

    if (bid1.priority == bid2.priority && bid1.address == bid2.address)
    {
        return true;
    }
    return false;
}

bool operator != (const BridgeID bid1, const BridgeID bid2)
{
    return !(bid1==bid2);
}

bool operator == (const PriorityVector v1, const PriorityVector v2)
{
    if (v1.root_id == v2.root_id)
    {
        if (v1.root_path_cost == v2.root_path_cost)
        {
            if (v1.bridge_id == v2.bridge_id)
            {
                if (v1.port_id == v2.port_id)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool operator < (const PriorityVector v1, const PriorityVector v2)
{
    if (v1.root_id < v2.root_id)
    {
        return true;
    }
    else if (v1.root_id == v2.root_id)
    {
        if (v1.root_path_cost > v2.root_path_cost)
        {
            return true;
        }
        else if (v1.root_path_cost == v2.root_path_cost)
        {
            if (v1.bridge_id < v2.bridge_id)
            {
                return true;
            }
            else if (v1.bridge_id == v2.bridge_id)
            {
                if (v1.port_id > v2.port_id)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool operator > (const PriorityVector v1, const PriorityVector v2)
{
    if (v1.root_id > v2.root_id)
    {
        return true;
    }
    else if (v1.root_id == v2.root_id)
    {
        if (v1.root_path_cost < v2.root_path_cost)
        {
            return true;
        }
        else if (v1.root_path_cost == v2.root_path_cost)
        {
            if (v1.bridge_id > v2.bridge_id)
            {
                return true;
            }
            else if (v1.bridge_id == v2.bridge_id)
            {
                if (v1.port_id < v2.port_id)
                {
                    return true;
                }
            }
        }
    }
    return false;
}
