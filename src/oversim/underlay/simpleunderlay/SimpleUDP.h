//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 Andras Varga
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

/**
 * @file SimpleUDP.h
 * @author Jochen Reber
 */

//
// Author: Jochen Reber
// Rewrite: Andras Varga 2004,2005
// Modifications: Stephan Krause, Bernhard Mueller
//

#ifndef __SIMPLEUDP_H__
#define __SIMPLEUDP_H__

#include <map>
#include <list>

#include <InitStages.h>
#include <UDP.h>

#include "UDPControlInfo_m.h"
#include "IRoutingTable.h"
#include "RoutingTable6.h"

class GlobalNodeList;
class SimpleNodeEntry;
class GlobalStatistics;

class IPControlInfo;
class IPv6ControlInfo;
class ICMP;
class ICMPv6;
class UDPPacket;


/**
 * Implements the UDP protocol: encapsulates/decapsulates user data into/from UDP.
 *
 * More info in the NED file.
 */
class SimpleUDP : public UDP
{
private:
   IRoutingTable *inet_rt;
   RoutingTable6 *inetv6_rt;
   IPvXAddress myAddr;
public:

    // delay fault type string and corresponding map for switch..case
    static std::string delayFaultTypeString;
    enum delayFaultTypeNum {
        delayFaultUndefined,
        delayFaultLiveAll,
        delayFaultLivePlanetlab,
        delayFaultSimulation
    };
    static std::map<std::string, delayFaultTypeNum> delayFaultTypeMap;

protected:

    // statistics
    int numQueueLost; /**< number of lost packets due to queue full */
    int numPartitionLost; /**< number of lost packets due to network partitions */
    int numDestUnavailableLost; /**< number of lost packets due to unavailable destination */
    simtime_t delay; /**< simulated delay between sending and receiving udp module */

    simtime_t constantDelay; /**< constant delay between two peers */
    bool useCoordinateBasedDelay; /**< delay should be calculated from euklidean distance between two peers */
    double jitter; /**< amount of jitter in % of total delay */
    bool faultyDelay; /** violate the triangle inequality?*/
    GlobalNodeList* globalNodeList; /**< pointer to GlobalNodeList */
    GlobalStatistics* globalStatistics; /**< pointer to GlobalStatistics */
    SimpleNodeEntry* nodeEntry; /**< nodeEntry of the overlay node this module belongs to */

public:
    /**
     * set or change the nodeEntry of this module
     *
     * @param entry the new nodeEntry
     */
    void setNodeEntry(SimpleNodeEntry* entry);

protected:
    /**
     * utility: show current statistics above the icon
     */
    void updateDisplayString();

    /**
     * process packets from application
     *
     * @param appData the data that has to be sent
     */
    virtual void processMsgFromApp(cPacket *appData);

    // process UDP packets coming from IP
    // virtual void processUDPPacket(UDPPacket *udpPacket);

    virtual void processUndeliverablePacket(UDPPacket *udpPacket, cPolymorphic *ctrl);

public:
    SimpleUDP();

    /** destructor */
    virtual ~SimpleUDP();

protected:
    /**
     * initialise the SimpleUDP module
     *
     * @param stage stage of initialisation phase
     */
    virtual void initialize(int stage);

    /**
     * returns the number of init stages
     *
     * @return the number of init stages
     */
    virtual int numInitStages() const
    {
        return MAX_STAGE_UNDERLAY + 1;
    }

    void finish();
};

#endif

