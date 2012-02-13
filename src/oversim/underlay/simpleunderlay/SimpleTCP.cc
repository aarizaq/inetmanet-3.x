//
// Copyright (C) 2004 Andras Varga
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
 * @file SimpleTCP.cc
 * @author Bernhard Mueller
 *
 * @brief Modification of the TCP module for using in SimpleUnderlay.
 * Modification of the original TCP module to send data directly to the destination gate.
 * The main modifications were made in both sendToIP() methods of the SimpleTCPConnection class.
 * They are similar to the modifications of the SimpleUDP module. All additional changes were
 * made to let the SimpleTCP module create SimpleTCPConnection.
 */

#include <omnetpp.h>

#include <CommonMessages_m.h>
#include <GlobalNodeListAccess.h>
#include <GlobalStatisticsAccess.h>

#include <SimpleInfo.h>
#include <SimpleUDP.h>
#include "IPv4Datagram_m.h"
#include "IPv6Datagram_m.h"
#include "TCPSegment.h"
#include "SimpleTCP.h"
#include "TCPCommand_m.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"
#include "ICMPMessage_m.h"
#include "ICMPv6Message_m.h"
#include "IPvXAddressResolver.h"
#include "TCPSendQueue.h"
#include "TCPSACKRexmitQueue.h"
#include "TCPReceiveQueue.h"
#include "TCPAlgorithm.h"

#define EPHEMERAL_PORTRANGE_START 1024
#define EPHEMERAL_PORTRANGE_END   5000

Define_Module( SimpleTCP );



static std::ostream& operator<<(std::ostream& os, const TCP::SockPair& sp)
{
    os << "loc=" << IPvXAddress(sp.localAddr) << ":" << sp.localPort << " "
       << "rem=" << IPvXAddress(sp.remoteAddr) << ":" << sp.remotePort;
    return os;
}

static std::ostream& operator<<(std::ostream& os, const TCP::AppConnKey& app)
{
    os << "connId=" << app.connId << " appGateIndex=" << app.appGateIndex;
    return os;
}

static std::ostream& operator<<(std::ostream& os, const TCPConnection& conn)
{
    os << "connId=" << conn.connId << " " << TCPConnection::stateName(conn.getFsmState())
       << " state={" << const_cast<TCPConnection&>(conn).getState()->info() << "}";
    return os;
}

void SimpleTCP::initialize(int stage)
{
    if (stage == MIN_STAGE_UNDERLAY) {
        lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
        WATCH(lastEphemeralPort);

        WATCH_PTRMAP(tcpConnMap);
        WATCH_PTRMAP(tcpAppConnMap);

        recordStatistics = par("recordStats");

        cModule *netw = simulation.getSystemModule();
        testing = netw->hasPar("testing") && netw->par("testing").boolValue();
        logverbose = !testing && netw->hasPar("logverbose") && netw->par("logverbose").boolValue();

        // start of modifications

        sad.numSent = 0;
        sad.numQueueLost = 0;
        sad.numPartitionLost = 0;
        sad.numDestUnavailableLost = 0;
        WATCH(sad.numQueueLost);
        WATCH(sad.numPartitionLost);
        WATCH(sad.numDestUnavailableLost);

        sad.globalNodeList = GlobalNodeListAccess().get();
        sad.globalStatistics = GlobalStatisticsAccess().get();
        sad.constantDelay = par("constantDelay");
        sad.useCoordinateBasedDelay = par("useCoordinateBasedDelay");

        sad.delayFaultTypeString = par("delayFaultType").stdstringValue();
        sad.delayFaultTypeMap["live_all"] = sad.delayFaultLiveAll;
        sad.delayFaultTypeMap["live_planetlab"] = sad.delayFaultLivePlanetlab;
        sad.delayFaultTypeMap["simulation"] = sad.delayFaultSimulation;

        switch (sad.delayFaultTypeMap[sad.delayFaultTypeString]) {
        case StatisticsAndDelay::delayFaultLiveAll:
        case StatisticsAndDelay::delayFaultLivePlanetlab:
        case StatisticsAndDelay::delayFaultSimulation:
            sad.faultyDelay = true;
            break;
        default:
            sad.faultyDelay = false;
        }

        sad.jitter = par("jitter");
        sad.nodeEntry = NULL;
        WATCH_PTR(sad.nodeEntry);
    }
}

void SimpleTCP::finish()
{
    sad.globalStatistics->addStdDev("SimpleTCP: Packets sent",
                                sad.numSent);
    sad.globalStatistics->addStdDev("SimpleTCP: Packets dropped due to queue overflows",
                                sad.numQueueLost);
    sad.globalStatistics->addStdDev("SimpleTCP: Packets dropped due to network partitions",
                                sad.numPartitionLost);
    sad.globalStatistics->addStdDev("SimpleTCP: Packets dropped due to unavailable destination",
                                sad.numDestUnavailableLost);
}

void SimpleTCP::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        SimpleTCPConnection *conn = (SimpleTCPConnection *) msg->getContextPointer();
        bool ret = conn->processTimer(msg);
        if (!ret)
            removeConnection(conn);
    }
    else if (msg->arrivedOn("ipIn") || msg->arrivedOn("ipv6In"))
    {
        if (dynamic_cast<ICMPMessage *>(msg) || dynamic_cast<ICMPv6Message *>(msg))
        {
            tcpEV << "ICMP error received -- discarding\n"; // FIXME can ICMP packets really make it up to TCP???
            delete msg;
        }
        else
        {
            // must be a TCPSegment
            TCPSegment *tcpseg = check_and_cast<TCPSegment *>(msg);

            // get src/dest addresses
            IPvXAddress srcAddr, destAddr;
            if (dynamic_cast<IPv4ControlInfo *>(tcpseg->getControlInfo())!=NULL)
            {
                IPv4ControlInfo *controlInfo = (IPv4ControlInfo *)tcpseg->removeControlInfo();
                srcAddr = controlInfo->getSrcAddr();
                destAddr = controlInfo->getDestAddr();
                delete controlInfo;
            }
            else if (dynamic_cast<IPv6ControlInfo *>(tcpseg->getControlInfo())!=NULL)
            {
                IPv6ControlInfo *controlInfo = (IPv6ControlInfo *)tcpseg->removeControlInfo();
                srcAddr = controlInfo->getSrcAddr();
                destAddr = controlInfo->getDestAddr();
                delete controlInfo;
            }
            else
            {
                error("(%s)%s arrived without control info", tcpseg->getClassName(), tcpseg->getName());
            }

            // process segment
            SimpleTCPConnection *conn = dynamic_cast<SimpleTCPConnection*>(findConnForSegment(tcpseg, srcAddr, destAddr));

            if (conn)
            {
                bool ret = conn->processTCPSegment(tcpseg, srcAddr, destAddr);
                if (!ret)
                    removeConnection(conn);
            }
            else
            {
                segmentArrivalWhileClosed(tcpseg, srcAddr, destAddr);
            }
        }
    }
    else // must be from app
    {
        TCPCommand *controlInfo = check_and_cast<TCPCommand *>(msg->getControlInfo());
        int appGateIndex = msg->getArrivalGate()->getIndex();
        int connId = controlInfo->getConnId();

        SimpleTCPConnection *conn = (SimpleTCPConnection*)findConnForApp(appGateIndex, connId);

        if (!conn)
        {
            conn = createConnection(appGateIndex, connId);

            // add into appConnMap here; it'll be added to connMap during processing
            // the OPEN command in SimpleTCPConnection's processAppCommand().
            AppConnKey key;
            key.appGateIndex = appGateIndex;
            key.connId = connId;
            tcpAppConnMap[key] = conn;

            tcpEV << "TCP connection created for " << msg << "\n";
        }
        bool ret = conn->processAppCommand(msg);
        if (!ret)
            removeConnection(conn);
    }

    if (ev.isGUI())
        updateDisplayString();
}

void SimpleTCP::setNodeEntry(SimpleNodeEntry* entry)
{
    sad.nodeEntry = entry;
}

SimpleTCPConnection *SimpleTCP::createConnection(int appGateIndex, int connId)
{
    return new SimpleTCPConnection(this, appGateIndex, connId);
}

void SimpleTCP::segmentArrivalWhileClosed(TCPSegment *tcpseg, IPvXAddress srcAddr, IPvXAddress destAddr)
{
    SimpleTCPConnection *tmp = new SimpleTCPConnection();
    tmp->segmentArrivalWhileClosed(tcpseg, srcAddr, destAddr);
    delete tmp;
    delete tcpseg;
}

SimpleTCPConnection *SimpleTCPConnection::cloneListeningConnection()
{
    SimpleTCPConnection *conn = new SimpleTCPConnection(tcpMain,appGateIndex,connId);

    // following code to be kept consistent with initConnection()
    const char *sendQueueClass = sendQueue->getClassName();
    conn->sendQueue = check_and_cast<TCPSendQueue *>(createOne(sendQueueClass));
    conn->sendQueue->setConnection(conn);

    const char *receiveQueueClass = receiveQueue->getClassName();
    conn->receiveQueue = check_and_cast<TCPReceiveQueue *>(createOne(receiveQueueClass));
    conn->receiveQueue->setConnection(conn);

    // create SACK retransmit queue
    rexmitQueue = new TCPSACKRexmitQueue();
    rexmitQueue->setConnection(this);

    const char *tcpAlgorithmClass = tcpAlgorithm->getClassName();
    conn->tcpAlgorithm = check_and_cast<TCPAlgorithm *>(createOne(tcpAlgorithmClass));
    conn->tcpAlgorithm->setConnection(conn);

    conn->state = conn->tcpAlgorithm->getStateVariables();
    configureStateVariables();
    conn->tcpAlgorithm->initialize();

    // put it into LISTEN, with our localAddr/localPort
    conn->state->active = false;
    conn->state->fork = true;
    conn->localAddr = localAddr;
    conn->localPort = localPort;
    FSM_Goto(conn->fsm, TCP_S_LISTEN);

    return conn;
}


void SimpleTCPConnection::sendRst(uint32 seq, IPvXAddress src, IPvXAddress dest, int srcPort, int destPort)
{
    TCPSegment *tcpseg = createTCPSegment("RST");

    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);

    tcpseg->setRstBit(true);
    tcpseg->setSequenceNo(seq);

    // send it
    SimpleTCPConnection::sendToIP(tcpseg, src, dest);
}

void SimpleTCPConnection::sendRstAck(uint32 seq, uint32 ack, IPvXAddress src, IPvXAddress dest, int srcPort, int destPort)
{
    TCPSegment *tcpseg = createTCPSegment("RST+ACK");

    tcpseg->setSrcPort(srcPort);
    tcpseg->setDestPort(destPort);

    tcpseg->setRstBit(true);
    tcpseg->setAckBit(true);
    tcpseg->setSequenceNo(seq);
    tcpseg->setAckNo(ack);

    // send it
    SimpleTCPConnection::sendToIP(tcpseg, src, dest);
}

void SimpleTCPConnection::sendToIP(TCPSegment *tcpseg)
{
    StatisticsAndDelay& sad = dynamic_cast<SimpleTCP*>(tcpMain)->sad;
    // record seq (only if we do send data) and ackno
    if (sndNxtVector && tcpseg->getPayloadLength()!=0)
        sndNxtVector->record(tcpseg->getSequenceNo());
    if (sndAckVector)
        sndAckVector->record(tcpseg->getAckNo());

    // final touches on the segment before sending
    tcpseg->setSrcPort(localPort);
    tcpseg->setDestPort(remotePort);
    ASSERT(tcpseg->getHeaderLength() >= TCP_HEADER_OCTETS);     // TCP_HEADER_OCTETS = 20 (without options)
    ASSERT(tcpseg->getHeaderLength() <= TCP_MAX_HEADER_OCTETS); // TCP_MAX_HEADER_OCTETS = 60

    // add header byte length for the skipped IP header
    int ipHeaderBytes = 0;
    if (remoteAddr.isIPv6()) {
        ipHeaderBytes = IPv6_HEADER_BYTES;
    } else {
        ipHeaderBytes = IP_HEADER_BYTES;
    }
    tcpseg->setByteLength(tcpseg->getHeaderLength() +
                          tcpseg->getPayloadLength() + ipHeaderBytes);

    tcpEV << "Sending: ";
    printSegmentBrief(tcpseg);

    // TBD reuse next function for sending


    /* main modifications for SimpleTCP start here */

    const IPvXAddress& src = IPvXAddressResolver().addressOf(tcpMain->getParentModule());
    //const IPvXAddress& src = localAddr;
    const IPvXAddress& dest = remoteAddr;

    SimpleInfo* info = dynamic_cast<SimpleInfo*>(sad.globalNodeList->getPeerInfo(dest));
    sad.numSent++;

    if (info == NULL) {
        EV << "[SimpleTCPConnection::sendToIP() @ " << src << "]\n"
           << "    No route to host " << dest
           << endl;

        delete tcpseg;
        sad.numDestUnavailableLost++;
        return;
    }

    SimpleNodeEntry* destEntry = info->getEntry();

    // calculate delay
    simtime_t totalDelay = 0;
    if (src != dest) {
        SimpleNodeEntry::SimpleDelay temp;
        if (sad.faultyDelay) {
            SimpleInfo* thisInfo = static_cast<SimpleInfo*>(sad.globalNodeList->getPeerInfo(src));
            temp = sad.nodeEntry->calcDelay(tcpseg, *destEntry,
                                        !(thisInfo->getNpsLayer() == 0 ||
                                          info->getNpsLayer() == 0)); //TODO
        } else {
            temp = sad.nodeEntry->calcDelay(tcpseg, *destEntry);
        }
        if (sad.useCoordinateBasedDelay == false) {
            totalDelay = sad.constantDelay;
        } else if (temp.second == false) {
            EV << "[SimpleTCPConnection::sendToIP() @ " << src << "]\n"
               << "    Send queue full: packet " << tcpseg << " dropped"
               << endl;
            delete tcpseg;
            sad.numQueueLost++;
            return;
        } else {
            totalDelay = temp.first;
        }
    }

    SimpleInfo* thisInfo = dynamic_cast<SimpleInfo*>(sad.globalNodeList->getPeerInfo(src));

    if (!sad.globalNodeList->areNodeTypesConnected(thisInfo->getTypeID(), info->getTypeID())) {
        EV << "[SimpleTCPConnection::sendToIP() @ " << src << "]\n"
                   << "    Partition " << thisInfo->getTypeID() << "->" << info->getTypeID()
                   << " is not connected"
                   << endl;
        delete tcpseg;
        sad.numPartitionLost++;
        return;
    }

    if (sad.jitter) {
        // jitter
        //totalDelay += truncnormal(0, SIMTIME_DBL(totalDelay) * jitter);

        //workaround (bug in truncnormal(): sometimes returns inf)
        double temp = truncnormal(0, SIMTIME_DBL(totalDelay) * sad.jitter);
        while (temp == INFINITY || temp != temp) { // reroll if temp is INF or NaN
            std::cerr << "\n******* SimpleTCPConnection: truncnormal() -> inf !!\n"
                      << std::endl;
            temp = truncnormal(0, SIMTIME_DBL(totalDelay) * sad.jitter);
        }

        totalDelay += temp;
    }



    EV << "[SimpleTCPConnection::sendToIP() @ " << src << "]\n"
       << "    Packet " << tcpseg << " sent with delay = " << totalDelay
       << endl;

    //RECORD_STATS(globalStatistics->addStdDev("SimpleTCP: delay", totalDelay));


    /* main modifications for SimpleTCP end here */

    if (!remoteAddr.isIPv6())
    {
        // send over IPv4
        IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(src.get4());
        controlInfo->setDestAddr(dest.get4());
        tcpseg->setControlInfo(controlInfo);

        tcpMain->sendDirect(tcpseg, totalDelay, 0, destEntry->getTcpIPv4Gate());
    }
    else
    {
        // send over IPv6
        IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(src.get6());
        controlInfo->setDestAddr(dest.get6());
        tcpseg->setControlInfo(controlInfo);

        tcpMain->sendDirect(tcpseg, totalDelay, 0, destEntry->getTcpIPv6Gate());
    }
}

void SimpleTCPConnection::sendToIP(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest)
{

    /* main modifications for SimpleTCP start here */
    StatisticsAndDelay& sad = dynamic_cast<SimpleTCP*>(tcpMain)->sad;

    SimpleInfo* info = dynamic_cast<SimpleInfo*>(sad.globalNodeList->getPeerInfo(dest));
    sad.numSent++;

    if (info == NULL) {
        EV << "[SimpleTCPConnection::sendToIP() @ " << src << "]\n"
           << "    No route to host " << dest
           << endl;

        delete tcpseg;
        sad.numDestUnavailableLost++;
        return;
    }

    SimpleNodeEntry* destEntry = info->getEntry();

    // calculate delay
    simtime_t totalDelay = 0;
    if (src != dest) {
        SimpleNodeEntry::SimpleDelay temp;
        if (sad.faultyDelay) {
            SimpleInfo* thisInfo = static_cast<SimpleInfo*>(sad.globalNodeList->getPeerInfo(src));
            temp = sad.nodeEntry->calcDelay(tcpseg, *destEntry,
                                        !(thisInfo->getNpsLayer() == 0 ||
                                          info->getNpsLayer() == 0)); //TODO
        } else {
            temp = sad.nodeEntry->calcDelay(tcpseg, *destEntry);
        }
        if (sad.useCoordinateBasedDelay == false) {
            totalDelay = sad.constantDelay;
        } else if (temp.second == false) {
            EV << "[SimpleTCPConnection::sendToIP() @ " << src << "]\n"
               << "    Send queue full: packet " << tcpseg << " dropped"
               << endl;
            delete tcpseg;
            sad.numQueueLost++;
            return;
        } else {
            totalDelay = temp.first;
        }
    }

    SimpleInfo* thisInfo = dynamic_cast<SimpleInfo*>(sad.globalNodeList->getPeerInfo(src));

    if (!sad.globalNodeList->areNodeTypesConnected(thisInfo->getTypeID(), info->getTypeID())) {
        EV << "[SimpleTCPConnection::sendToIP() @ " << src << "]\n"
                   << "    Partition " << thisInfo->getTypeID() << "->" << info->getTypeID()
                   << " is not connected"
                   << endl;
        delete tcpseg;
        sad.numPartitionLost++;
        return;
    }

    if (sad.jitter) {
        // jitter
        //totalDelay += truncnormal(0, SIMTIME_DBL(totalDelay) * jitter);

        //workaround (bug in truncnormal(): sometimes returns inf)
        double temp = truncnormal(0, SIMTIME_DBL(totalDelay) * sad.jitter);
        while (temp == INFINITY || temp != temp) { // reroll if temp is INF or NaN
            std::cerr << "\n******* SimpleTCPConnection: truncnormal() -> inf !!\n"
                      << std::endl;
            temp = truncnormal(0, SIMTIME_DBL(totalDelay) * sad.jitter);
        }

        totalDelay += temp;
    }



    EV << "[SimpleTCPConnection::sendToIP() @ " << src << "]\n"
       << "    Packet " << tcpseg << " sent with delay = " << totalDelay
       << endl;

    //RECORD_STATS(globalStatistics->addStdDev("SimpleTCP: delay", totalDelay));


    /* main modifications for SimpleTCP end here */

    tcpEV << "Sending: ";
    printSegmentBrief(tcpseg);

    if (!dest.isIPv6())
    {
        // send over IPv4
        IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(src.get4());
        controlInfo->setDestAddr(dest.get4());
        tcpseg->setControlInfo(controlInfo);

        check_and_cast<TCP *>(simulation.getContextModule())->sendDirect(tcpseg, totalDelay, 0, destEntry->getTcpIPv4Gate());
    }
    else
    {
        // send over IPv6
        IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
        controlInfo->setProtocol(IP_PROT_TCP);
        controlInfo->setSrcAddr(src.get6());
        controlInfo->setDestAddr(dest.get6());
        tcpseg->setControlInfo(controlInfo);

        check_and_cast<TCP *>(simulation.getContextModule())->sendDirect(tcpseg, totalDelay, 0, destEntry->getTcpIPv6Gate());
    }
}
