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

/**
 * @file SimpleNodeEntry.cc
 * @author Bernhard Heep
 */

#include <sstream>
#include <cassert>

#include "SimpleNodeEntry.h"
#include "SimpleUDP.h"
#include "SimpleTCP.h"
#include "SHA1.h"
#include "OverlayKey.h"
#include "BinaryValue.h"


uint8_t NodeRecord::dim;

NodeRecord::NodeRecord()
{
    coords = new double[dim];
}

NodeRecord::~NodeRecord()
{
    delete[] coords;
    coords = NULL;
}

NodeRecord::NodeRecord(const NodeRecord& nodeRecord)
{
    coords = new double[dim];
    for (uint32_t i = 0; i < dim; ++i)
        coords[i] = nodeRecord.coords[i];
}

NodeRecord& NodeRecord::operator=(const NodeRecord& nodeRecord)
{
    delete[] coords;
    coords = new double[dim];
    for (uint32_t i = 0; i < dim; ++i)
        coords[i] = nodeRecord.coords[i];

    return *this;
}

SimpleNodeEntry::SimpleNodeEntry(cModule* node,
                                 cChannelType* typeRx,
                                 cChannelType* typeTx,
                                 uint32_t sendQueueLength,
                                 uint32_t fieldSize)
{
    assert(NodeRecord::dim == 2);
    cModule* udpModule = node->getSubmodule("udp");
    UdpIPv4ingate = udpModule->gate("ipIn");
    UdpIPv6ingate = udpModule->gate("ipv6In");
    cModule* tcpModule = node->getSubmodule("tcp", 0);
    if (tcpModule) {
        TcpIPv4ingate = tcpModule->gate("ipIn");
        TcpIPv6ingate = tcpModule->gate("ipv6In");
    }

    nodeRecord = new NodeRecord;
    index = -1;

    //use random values as coordinates
    nodeRecord->coords[0] = uniform(0, fieldSize) - fieldSize / 2;
    nodeRecord->coords[1] = uniform(0, fieldSize) - fieldSize / 2;

    cDatarateChannel* tempRx = dynamic_cast<cDatarateChannel*>(typeRx->create("temp"));
    cDatarateChannel* tempTx = dynamic_cast<cDatarateChannel*>(typeTx->create("temp"));

    rx.bandwidth = tempRx->par("datarate");
    rx.errorRate = tempRx->par("ber");
    rx.accessDelay = tempRx->par("delay");
    rx.maxQueueTime = 0;
    rx.finished = simTime();

    tx.bandwidth = tempTx->par("datarate");
    tx.errorRate = tempTx->par("ber");
    tx.accessDelay = tempTx->par("delay");
    tx.maxQueueTime = (sendQueueLength * 8.) / tx.bandwidth;
    tx.finished = simTime();

    delete tempRx;
    delete tempTx;
}

SimpleNodeEntry::SimpleNodeEntry(cModule* node,
                                 cChannelType* typeRx,
                                 cChannelType* typeTx,
                                 uint32_t sendQueueLength,
                                 NodeRecord* nodeRecord, int index)
{
    cModule* udpModule = node->getSubmodule("udp");
    UdpIPv4ingate = udpModule->gate("ipIn");
    UdpIPv6ingate = udpModule->gate("ipv6In");
    cModule* tcpModule = node->getSubmodule("tcp", 0);
    if (tcpModule) {
        TcpIPv4ingate = tcpModule->gate("ipIn");
        TcpIPv6ingate = tcpModule->gate("ipv6In");
    }

    this->nodeRecord = nodeRecord;
    this->index = index;

    cDatarateChannel* tempRx = dynamic_cast<cDatarateChannel*>(typeRx->create("temp"));
    cDatarateChannel* tempTx = dynamic_cast<cDatarateChannel*>(typeTx->create("temp"));

    rx.bandwidth = tempRx->par("datarate");
    rx.errorRate = tempRx->par("ber");
    rx.accessDelay = tempRx->par("delay");
    rx.maxQueueTime = 0;
    rx.finished = simTime();

    tx.bandwidth = tempTx->par("datarate");
    tx.errorRate = tempTx->par("ber");
    tx.accessDelay = tempTx->par("delay");
    tx.maxQueueTime = (sendQueueLength * 8.) / tx.bandwidth;
    tx.finished = simTime();

    delete tempRx;
    delete tempTx;
}

float SimpleNodeEntry::operator-(const SimpleNodeEntry& entry) const
{
    double sum_of_squares = 0;
    for (uint32_t i = 0; i < nodeRecord->dim; i++) {
        sum_of_squares += pow(nodeRecord->coords[i] -
                              entry.nodeRecord->coords[i], 2);
    }
    return sqrt(sum_of_squares);
}

SimpleNodeEntry::SimpleDelay SimpleNodeEntry::calcDelay(cPacket* msg,
                                                        const SimpleNodeEntry& dest,
                                                        bool faultyDelay)
{
    if ((pow(1 - tx.errorRate, msg->getByteLength() * 8) <= uniform(0, 1)) ||
        (pow(1 - dest.rx.errorRate, msg->getByteLength() * 8) <= uniform(0, 1))) {
        msg->setBitError(true);
    }

    simtime_t now = simTime();
    simtime_t bandwidthDelay= ((msg->getByteLength() * 8) / tx.bandwidth);
    simtime_t newTxFinished = std::max(tx.finished, now) + bandwidthDelay;

    // send queue
    if ((newTxFinished > now + tx.maxQueueTime) && (tx.maxQueueTime != 0)) {
        EV << "[SimpleNodeEntry::calcDelay()]\n"
           << "    Send queue overrun"
           << "\n    newTxFinished = fmax(txFinished, now) + bandwidthDelay"
           << "\n    newTxFinished = " << newTxFinished
           << "\n    tx.finished = " << tx.finished
           << "\n    now = " << now
           << "\n    bandwidthDelay = " << bandwidthDelay
           << "\n    (newTxFinished > now + txMaxQueueTime) == true"
           << "\n    tx.maxQueueTime = " << tx.maxQueueTime
           << endl;
        return SimpleDelay(0, false);
    }

    tx.finished = newTxFinished;

    simtime_t destBandwidthDelay = (msg->getByteLength() * 8) / dest.rx.bandwidth;
    simtime_t coordDelay = 0.001 * (*this - dest);

    if (faultyDelay)
        coordDelay = getFaultyDelay(coordDelay);

    return SimpleDelay(tx.finished - now
                       + tx.accessDelay
                       + coordDelay
                       + destBandwidthDelay + dest.rx.accessDelay, true);
}

simtime_t SimpleNodeEntry::getFaultyDelay(simtime_t oldDelay) {

    // hash over string of oldDelay
    char delaystring[35];
    sprintf(delaystring, "%.30f", SIMTIME_DBL(oldDelay));

    CSHA1 sha1;
    uint8_t hashOverDelays[20];
    sha1.Reset();
    sha1.Update((uint8_t*)delaystring, 32);
    sha1.Final();
    sha1.GetHash(hashOverDelays);

    // get the hash's first 4 bytes == 32 bits as one unsigned integer
    unsigned int decimalhash = 0;
    for (int i = 0; i < 4; i++) {
        decimalhash += (unsigned int) hashOverDelays[i] * (2 << (8*(3 - i) - 1));
    }

    // normalize decimal hash value onto 0..1 (decimal number / 2^32-1)
    double fraction = (double) decimalhash / (unsigned int) ((2 << 31) - 1);

    // flip a coin if faulty rtt is larger or smaller
    char sign = (decimalhash % 2 == 0) ? 1 : -1;

    // get the error ratio according to the distributions in
    // "Network Coordinates in the Wild", Figure 7
    double errorRatio = 0;
    switch (SimpleUDP::delayFaultTypeMap[SimpleUDP::delayFaultTypeString]) {
        case SimpleUDP::delayFaultLiveAll:
            // Kumaraswamy, a=2.03, b=14, moved by 0.04 to the right
            errorRatio = pow((1.0 - pow(fraction, 1.0/14.0)), 1.0/2.03) + 0.04;
            break;

        case SimpleUDP::delayFaultLivePlanetlab:
            // Kumaraswamy, a=1.95, b=50, moved by 0.105 to the right
            errorRatio = pow((1.0 - pow(fraction, 1.0/50.0)), 1.0/1.95) + 0.105;
            break;

        case SimpleUDP::delayFaultSimulation:
            // Kumaraswamy, a=1.96, b=23, moved by 0.02 to the right
            errorRatio = pow((1.0 - pow(fraction, 1.0/23.0)), 1.0/1.96) + 0.02;
            std::cout << "ErrorRatio: " << errorRatio << std::endl;
            break;

        default:
            break;
    }
    /*
    std::cout << "decimalhash: " << decimalhash << " -- fraction: " << fraction << " -- errorRatio: " << errorRatio << std::endl;
    std::cout << SimpleUDP::delayFaultTypeString << " -- " << "old: " << oldDelay << " -- new: " << oldDelay + sign * errorRatio * oldDelay << std::endl;
    */

    // If faulty rtt is smaller, set errorRatio to max 0.6
    errorRatio = (sign == -1 && errorRatio > 0.6) ? 0.6 : errorRatio;

    return oldDelay + sign * errorRatio * oldDelay;
}

std::string SimpleNodeEntry::info() const
{
    std::ostringstream str;
    str << *this;
    return str.str();
}

std::ostream& operator<<(std::ostream& out, const SimpleNodeEntry& entry)
{
    out << "(";
    for (uint8_t i = 0; i < NodeRecord::dim; i++) {
        out << "dim" << i <<": " << entry.nodeRecord->coords[i];
        out << ", ";
    }
    // out << ", y:" << entry.nodeRecord->coords[1]
    out << ")\n[rx]"
        << "\nbandwidth = " << entry.rx.bandwidth
        << ",\ndelay = " << entry.rx.accessDelay
        << ",\nerrorRate = " << entry.rx.errorRate
        << ",\ntxMaxQueueTime = " << entry.rx.maxQueueTime
        << ",\ntxFinished = " << entry.rx.finished;
    out << "\n[tx]"
        << "\nbandwidth = " << entry.tx.bandwidth
        << ",\ndelay = " << entry.tx.accessDelay
        << ",\nerrorRate = " << entry.tx.errorRate
        << ",\ntxMaxQueueTime = " << entry.tx.maxQueueTime
        << ",\ntxFinished = " << entry.tx.finished;

    return out;
}
