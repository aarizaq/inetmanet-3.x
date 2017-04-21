//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_UDPDMASINK_H
#define __INET_UDPDMASINK_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

/**
 * UDP application. See NED for more info.
 */
class INET_API UDPDmaMacSink : public ApplicationBase
{
  protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };

    // parameters
    L3Address sinkAddress;

    std::vector<L3Address> destAddresses;
    int localPort = -1, destPort = -1;
    simtime_t startTime;
    simtime_t stopTime;
    const char *packetName = nullptr;

    struct NodeId {
        int networkId;
        MACAddress addr;
        bool operator==(const NodeId& other) const { return (networkId == other.networkId && addr == other.addr);}
          /**
           * @brief Returns true if the id of the other dimension is
           * greater then the id of this dimension.
           *
           * This is needed to be able to use Dimension as a key in std::map.
           */
        bool operator<(const NodeId& other) const { if (networkId == other.networkId) return addr < other.addr; else return (networkId < other.networkId);  }

          /** @brief Sorting operator by dimension ID.*/
        bool operator>(const NodeId& other) const { if (networkId == other.networkId) return addr > other.addr; else return (networkId > other.networkId);}
          /** @brief Sorting operator by dimension ID.*/
    };

    std::map<NodeId,unsigned long> sequences;



    // state
    UDPSocket socket;
    cMessage *selfMsg = nullptr;

    // statistics
    int numSent = 0;
    int numReceived = 0;
    int numReceivedDmaMac = 0;
    int totalRec = 0;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t rcvdPkSignalDma;

    typedef std::map<int,L3Address> RelayId;
    RelayId relayId;

    enum DISTRUBUTIONTYPE {
        uniformDist,
        constantDist,
        exponentialDist,
        pareto_shiftedDist,
        normalDist
    };

    struct ActuatorInfo {
        int networkId;
        int address;
        DISTRUBUTIONTYPE disType;
        double a;
        double b;
        double c;
        simtime_t startTime;
        cMessage *timer;
    };
    std::vector<ActuatorInfo> actuators;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    // chooses random destination address
    virtual L3Address chooseDestAddr();
    virtual void processPacket(cPacket *msg);
    virtual void parseXMLConfigFile();
    virtual void setSocketOptions();

    virtual void processStart();
    virtual void processStop();

    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;

  public:
    UDPDmaMacSink() {}
    ~UDPDmaMacSink();
};

} // namespace inet

#endif // ifndef __INET_UDPBASICAPP_H

