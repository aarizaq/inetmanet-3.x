//
// Copyright (C) 2010 Institut fuer Telematik, Karlsruher Institut fuer Technologie (KIT)
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
 * @file SimpleTCP.h
 * @author Bernhard Mueller
 *
 * @brief Modification of the TCP module for using in SimpleUnderlay.
 * Modification of the original TCP module to send data directly to the destination gate.
 * The main modifications were made in both sendToIP() methods of the SimpleTCPConnection class.
 * They are similar to the modifications of the SimpleUDP module. All additional changes were
 * made to let the SimpleTCP module create SimpleTCPConnection.
 */

#ifndef __SIMPLETCP_H
#define __SIMPLETCP_H

//#include <map>
//#include <set>
#include <omnetpp.h>
#include "IPvXAddress.h"
#include <TCPConnection.h>
#include <InitStages.h>

class SimpleNodeEntry;


struct StatisticsAndDelay {

public:

    // delay fault type string and corresponding map for switch..case
    std::string delayFaultTypeString;
    enum delayFaultTypeNum {
        delayFaultUndefined,
        delayFaultLiveAll,
        delayFaultLivePlanetlab,
        delayFaultSimulation
    };
    std::map<std::string, delayFaultTypeNum> delayFaultTypeMap;

    // statistics
    int numSent; /**< number of sent packets */
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
};

class SimpleTCPConnection : public TCPConnection {

public:
    SimpleTCPConnection():TCPConnection(){};
    SimpleTCPConnection(TCP* mod, int appGateIndex, int connId):TCPConnection(mod, appGateIndex, connId){};

    /** Utility: adds control info to segment and sends it to the destination node */
    virtual void sendToIP(TCPSegment *tcpseg);

    static StatisticsAndDelay sad;

    /** Utility: sends RST; does not use connection state */
    void sendRst(uint32 seq, IPvXAddress src, IPvXAddress dest, int srcPort, int destPort);
    /** Utility: sends RST+ACK; does not use connection state */
    void sendRstAck(uint32 seq, uint32 ack, IPvXAddress src, IPvXAddress dest, int srcPort, int destPort);

protected:
    /** Utility: send IP packet to destination node */
    virtual void sendToIP(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest);

    /** Utility: clone a listening connection. Used for forking. */
    SimpleTCPConnection *cloneListeningConnection();

};

class SimpleTCP : public TCP {

public:
  SimpleTCP() { sad.globalStatistics = NULL; };
  virtual ~SimpleTCP() {};

  void setNodeEntry (SimpleNodeEntry* entry);

  StatisticsAndDelay sad;

protected:
  // utility methods
  void segmentArrivalWhileClosed(TCPSegment *tcpseg, IPvXAddress srcAddr, IPvXAddress destAddr);
  SimpleTCPConnection *createConnection(int appGateIndex, int connId);
  virtual void initialize(int stage);
  virtual void handleMessage(cMessage *msg);

  virtual int numInitStages() const
  {
      return MAX_STAGE_UNDERLAY + 1;
  }

  void finish();
};

#endif
