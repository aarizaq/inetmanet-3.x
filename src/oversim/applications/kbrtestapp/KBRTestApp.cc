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
 * @file KBRTestApp.cc
 * @author Bernhard Heep, Ingmar Baumgart
 */

#include "IPvXAddressResolver.h"
#include <CommonMessages_m.h>
#include <GlobalStatistics.h>
#include <UnderlayConfigurator.h>
#include <GlobalNodeList.h>

#include "KBRTestApp.h"
#include "KBRTestMessage_m.h"

Define_Module(KBRTestApp);

KBRTestApp::KBRTestApp()
{
    onewayTimer = NULL;
}

KBRTestApp::~KBRTestApp()
{
    cancelAndDelete(onewayTimer);
    cancelAndDelete(rpcTimer);
    cancelAndDelete(lookupTimer);
}

void KBRTestApp::initializeApp(int stage)
{
    if (stage != MIN_STAGE_APP) {
        return;
    }

    kbrOneWayTest = par("kbrOneWayTest");
    kbrRpcTest = par("kbrRpcTest");
    kbrLookupTest = par("kbrLookupTest");

    if (!kbrOneWayTest && !kbrRpcTest && !kbrLookupTest) {
        throw cRuntimeError("KBRTestApp::initializeApp(): "
                            "no tests are configured!");
    }

    failureLatency = par("failureLatency");

    testMsgSize = par("testMsgSize");
    lookupNodeIds = par("lookupNodeIds");
    mean = par("testMsgInterval");
    deviation = mean / 10;
    activeNetwInitPhase = par("activeNetwInitPhase");
    msgHandleBufSize = par("msgHandleBufSize");
    onlyLookupInoffensiveNodes = par("onlyLookupInoffensiveNodes");

    numSent = 0;
    bytesSent = 0;
    numDelivered = 0;
    bytesDelivered = 0;
    numDropped = 0;
    bytesDropped = 0;
    WATCH(numSent);
    WATCH(bytesSent);
    WATCH(numDelivered);
    WATCH(bytesDelivered);
    WATCH(numDropped);
    WATCH(bytesDropped);

    numRpcSent = 0;
    bytesRpcSent = 0;
    numRpcDelivered = 0;
    bytesRpcDelivered = 0;
    numRpcDropped = 0;
    bytesRpcDropped = 0;
    rpcSuccLatencyCount = 0;
    rpcSuccLatencySum = 0;
    rpcTotalLatencyCount = 0;
    rpcTotalLatencySum = 0;
    WATCH(numRpcSent);
    WATCH(bytesRpcSent);
    WATCH(numRpcDelivered);
    WATCH(bytesRpcDelivered);
    WATCH(numRpcDropped);
    WATCH(bytesRpcDropped);

    numLookupSent = 0;
    numLookupSuccess = 0;
    numLookupFailed = 0;
    WATCH(numLookupSent);
    WATCH(numLookupSuccess);
    WATCH(numLookupFailed);

    sequenceNumber = 0;

    nodeIsLeavingSoon = false;

    // initialize circular buffer
    if (msgHandleBufSize > 0) {
        mhBuf.resize(msgHandleBufSize);
        mhBufBegin = mhBuf.begin();
        mhBufEnd = mhBuf.end();
        mhBufNext = mhBufBegin;
    }

#if 0
    bindToPort(1025);
    thisNode.setPort(1025);
#endif

    // start periodic timer
    onewayTimer = new cMessage("onewayTimer");
    rpcTimer = new cMessage("rpcTimer");
    lookupTimer = new cMessage("lookupTimer");

    if (kbrOneWayTest) {
        scheduleAt(simTime() + truncnormal(mean, deviation), onewayTimer);
    }
    if (kbrRpcTest) {
        scheduleAt(simTime() + truncnormal(mean, deviation), rpcTimer);
    }
    if (kbrLookupTest) {
        scheduleAt(simTime() + truncnormal(mean, deviation), lookupTimer);
    }
}

void KBRTestApp::handleTimerEvent(cMessage* msg)
{
    // schedule next timer event
    scheduleAt(simTime() + truncnormal(mean, deviation), msg);

    // do nothing if the network is still in the initialization phase
    if ((!activeNetwInitPhase && underlayConfigurator->isInInitPhase())
            || underlayConfigurator->isSimulationEndingSoon()
            || nodeIsLeavingSoon) {
        return;
    }

    std::pair<OverlayKey,TransportAddress> dest = createDestKey();

    if (msg == onewayTimer) {
        // TEST 1: route a test message to a key (one-way)
        // do nothing if there are currently no nodes in the network
        if (!dest.first.isUnspecified()) {
            // create a 100 byte test message
            KBRTestMessage* testMsg = new KBRTestMessage("KBRTestMessage");
            testMsg->setId(getId());
            testMsg->setSeqNum(sequenceNumber++);
            testMsg->setByteLength(testMsgSize);
            testMsg->setMeasurementPhase(globalStatistics->isMeasuring());

            RECORD_STATS(globalStatistics->sentKBRTestAppMessages++;
                         numSent++; bytesSent += testMsg->getByteLength());

            callRoute(dest.first, testMsg);
        }
    } else if (msg == rpcTimer) {
        // TEST 2: send a remote procedure call to a specific key and wait for a response
        // do nothing if there are currently no nodes in the network
        if (!dest.first.isUnspecified()) {
            KbrTestCall* call = new KbrTestCall;
            call->setByteLength(testMsgSize);
            KbrRpcContext* context = new KbrRpcContext;
            context->setDestKey(dest.first);
            if (lookupNodeIds) {
                context->setDestAddr(dest.second);
            }
            context->setMeasurementPhase(globalStatistics->isMeasuring());

            RECORD_STATS(numRpcSent++;
                         bytesRpcSent += call->getByteLength());

            sendRouteRpcCall(TIER1_COMP, dest.first, call, context);
        }
    } else /*if (msg == lookupTimer &&)*/ {
        // TEST 3: perform a lookup of a specific key
        // do nothing if there are currently no nodes in the network
        if (!dest.first.isUnspecified()) {
            LookupCall* call = new LookupCall();
            call->setKey(dest.first);
            call->setNumSiblings(overlay->getMaxNumSiblings());
            KbrRpcContext* context = new KbrRpcContext;
            context->setDestKey(dest.first);
            if (lookupNodeIds) {
                context->setDestAddr(dest.second);
            }
            context->setMeasurementPhase(globalStatistics->isMeasuring());
            sendInternalRpcCall(OVERLAY_COMP, call, context);

            RECORD_STATS(numLookupSent++);
        }
    }

#if 0
    thisNode.setPort(1025);
    NodeHandle handle = globalNodeList->getBootstrapNode();
    handle.setPort(1025);
    pingNode(handle, -1, -1, NULL, "TestPING", NULL, -1, UDP_TRANSPORT);
#endif

}
void KBRTestApp::pingResponse(PingResponse* response, cPolymorphic* context,
                              int rpcId, simtime_t rtt)
{
    //std::cout << rtt << std::endl;
}
bool KBRTestApp::handleRpcCall(BaseCallMessage* msg)
{
    RPC_SWITCH_START( msg );
        RPC_DELEGATE( KbrTest, kbrTestCall );
    RPC_SWITCH_END( );

    return RPC_HANDLED;
}
void KBRTestApp::kbrTestCall(KbrTestCall* call)
{
    KbrTestResponse* response = new KbrTestResponse;
    response->setByteLength(call->getByteLength());
    sendRpcResponse(call, response);
}

void KBRTestApp::handleRpcResponse(BaseResponseMessage* msg,
                                   cPolymorphic* context, int rpcId,
                                   simtime_t rtt)
{
    RPC_SWITCH_START(msg)
    RPC_ON_RESPONSE(Lookup) {
        EV << "[KBRTestApp::handleRpcResponse() @ " << overlay->getThisNode().getIp()
           << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
           << "    Lookup RPC Response received: id=" << rpcId << "\n"
           << "    msg=" << *_LookupResponse << " rtt=" << rtt
           << endl;
        handleLookupResponse(_LookupResponse, context, rtt);
        break;
    }
    RPC_ON_RESPONSE(KbrTest) {
        KbrRpcContext* kbrRpcContext = check_and_cast<KbrRpcContext*>(context);
        if (kbrRpcContext->getMeasurementPhase() == true) {
            if (!lookupNodeIds ||
                (kbrRpcContext->getDestKey() == msg->getSrcNode().getKey() &&
                 kbrRpcContext->getDestAddr() == msg->getSrcNode())) {

                RECORD_STATS(numRpcDelivered++;
                             bytesRpcDelivered += msg->getByteLength());
                RECORD_STATS(globalStatistics->recordOutVector(
                        "KBRTestApp: RPC Success Latency", SIMTIME_DBL(rtt)));
                RECORD_STATS(globalStatistics->recordOutVector(
                        "KBRTestApp: RPC Total Latency", SIMTIME_DBL(rtt)));
                RECORD_STATS(rpcSuccLatencyCount++;
                             rpcSuccLatencySum += SIMTIME_DBL(rtt));
                RECORD_STATS(rpcTotalLatencyCount++;
                             rpcTotalLatencySum += SIMTIME_DBL(rtt));
                OverlayCtrlInfo* overlayCtrlInfo =
                        dynamic_cast<OverlayCtrlInfo*>(msg->getControlInfo());

                uint16_t hopSum = msg->getCallHopCount();
                hopSum += (overlayCtrlInfo ? overlayCtrlInfo->getHopCount() : 1);
                RECORD_STATS(globalStatistics->recordOutVector(
                        "KBRTestApp: RPC Hop Count", hopSum));
//              RECORD_STATS(globalStatistics->recordHistogram(
//                      "KBRTestApp: RPC Hop Count Histogram", hopSum));
            } else {
                RECORD_STATS(numRpcDropped++;
                             bytesRpcDropped += msg->getByteLength());
                // for failed RPCs add failureLatency to latency statistics vector
                RECORD_STATS(globalStatistics->recordOutVector(
                        "KBRTestApp: RPC Total Latency",
                        SIMTIME_DBL(failureLatency)));
                RECORD_STATS(rpcTotalLatencyCount++;
                             rpcTotalLatencySum += SIMTIME_DBL(failureLatency));
            }
        }
        delete kbrRpcContext;
        break;
    }
    RPC_SWITCH_END( )
}

void KBRTestApp::handleRpcTimeout(BaseCallMessage* msg,
                                  const TransportAddress& dest,
                                  cPolymorphic* context, int rpcId,
                                  const OverlayKey& destKey)
{
    RPC_SWITCH_START(msg)
    RPC_ON_CALL(KbrTest) {
        KbrRpcContext* kbrRpcContext = check_and_cast<KbrRpcContext*>(context);
        if (kbrRpcContext->getMeasurementPhase() == true) {
             RECORD_STATS(numRpcDropped++;
                          bytesRpcDropped += msg->getByteLength());
             // for failed RPCs add failureLatency to latency statistics vector
             RECORD_STATS(globalStatistics->recordOutVector(
                         "KBRTestApp: RPC Total Latency",
                         SIMTIME_DBL(failureLatency)));
             RECORD_STATS(rpcTotalLatencyCount++;
                          rpcTotalLatencySum += SIMTIME_DBL(failureLatency));

        }
        break;
    }
    RPC_ON_CALL(Lookup) {
        KbrRpcContext* kbrRpcContext = check_and_cast<KbrRpcContext*>(context);
        if (kbrRpcContext->getMeasurementPhase() == true) {
            RECORD_STATS(numLookupFailed++);
            // for failed lookups add failureLatency to latency statistics vector
            RECORD_STATS(globalStatistics->recordOutVector(
                         "KBRTestApp: Lookup Total Latency",
                         SIMTIME_DBL(failureLatency)));
        }
        break;
    }
    RPC_SWITCH_END()

    delete context;
}

void KBRTestApp::handleLookupResponse(LookupResponse* msg,
                                      cObject* context, simtime_t latency)
{
    EV << "[KBRTestApp::handleLookupResponse() @ " << overlay->getThisNode().getIp()
       << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
       << "    Lookup response for key " << msg->getKey()<< " : ";

    KbrRpcContext* kbrRpcContext = check_and_cast<KbrRpcContext*>(context);

    if (kbrRpcContext->getMeasurementPhase() == true) {
        if (msg->getIsValid() && (!lookupNodeIds ||
                ((msg->getSiblingsArraySize() > 0) &&
                 (msg->getSiblings(0).getKey() == msg->getKey()) &&
                 (kbrRpcContext->getDestAddr() == msg->getSiblings(0))))) {
            RECORD_STATS(numLookupSuccess++);
            RECORD_STATS(globalStatistics->recordOutVector(
                   "KBRTestApp: Lookup Success Latency", SIMTIME_DBL(latency)));
            RECORD_STATS(globalStatistics->recordOutVector(
                   "KBRTestApp: Lookup Total Latency", SIMTIME_DBL(latency)));
            RECORD_STATS(globalStatistics->recordOutVector(
                    "KBRTestApp: Lookup Hop Count", msg->getHopCount()));
        } else {
#if 0
            if (!msg->getIsValid()) {
                std::cout << "invalid" << std::endl;
            } else if (msg->getSiblingsArraySize() == 0) {
                std::cout << "empty" << std::endl;
            } else {
                std::cout << "wrong key" << std::endl;
            }
#endif
            RECORD_STATS(numLookupFailed++);
            // for failed lookups add failureLatency to latency statistics vector
            RECORD_STATS(globalStatistics->recordOutVector(
                         "KBRTestApp: Lookup Total Latency",
                         SIMTIME_DBL(failureLatency)));
            RECORD_STATS(globalStatistics->recordOutVector(
                    "KBRTestApp: Failed Lookup Hop Count", msg->getHopCount()));
        }
    }

    delete kbrRpcContext;
}

void KBRTestApp::handleNodeLeaveNotification()
{
    nodeIsLeavingSoon = true;
}

void KBRTestApp::deliver(OverlayKey& key, cMessage* msg)
{
    KBRTestMessage* testMsg = check_and_cast<KBRTestMessage*>(msg);
    OverlayCtrlInfo* overlayCtrlInfo =
        check_and_cast<OverlayCtrlInfo*>(msg->removeControlInfo());

    if (overlay->getThisNode().getKey().isUnspecified())
        error("key");

    // check for duplicate
    if ((msgHandleBufSize > 0 )
            && checkSeen(overlayCtrlInfo->getSrcNode().getKey(), testMsg->getSeqNum())) {
        EV << "[KBRTestApp::deliver() @ " << overlay->getThisNode().getIp()
           << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
           << "    Duplicate dropped."
           << endl;
        delete overlayCtrlInfo;
        delete testMsg;
        return;
    }

    // Return statistical data to the sender.
    if (cModule* mod = simulation.getModule(testMsg->getId())) {
        if (KBRTestApp* sender = dynamic_cast<KBRTestApp*>(mod)) {
            if ((!lookupNodeIds) || (overlay->getThisNode().getKey() == key)) {
                if (testMsg->getMeasurementPhase() == true) {
                        sender->evaluateData((simTime() - testMsg->getCreationTime()),
                                             overlayCtrlInfo->getHopCount(),
                                             testMsg->getByteLength());
                }
            } else if(lookupNodeIds) {
                if (testMsg->getMeasurementPhase() == true) {
                    RECORD_STATS(numDropped++;
                                 bytesDropped += testMsg->getByteLength());
                }
                EV << "[KBRTestApp::deliver() @ " << overlay->getThisNode().getIp()
                   << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
                   << "    Error: Lookup of NodeIDs and KBRTestMessage"
                   << " received with different destKey!"
                   << endl;
            }
        }
    }

    EV << "[KBRTestApp::deliver() @ " << overlay->getThisNode().getIp()
       << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
       << "    Received \"" << testMsg->getName() << "(seqNr: "
       << testMsg->getSeqNum() << ")\n"
       << "    with destination key: " << key.toString(16)
       << endl;

    delete overlayCtrlInfo;
    delete testMsg;
}

void KBRTestApp::forward(OverlayKey* key, cPacket** msg,
                         NodeHandle* nextHopNode)
{
    KBRTestMessage* tempMsg = dynamic_cast<KBRTestMessage*>(*msg);

    if (tempMsg == NULL) return;

    tempMsg->setVisitedNodesArraySize(tempMsg->getVisitedNodesArraySize() + 1);
    tempMsg->setVisitedNodes(tempMsg->getVisitedNodesArraySize() - 1,
                             overlay->getThisNode().getIp());
}

std::pair<OverlayKey, TransportAddress> KBRTestApp::createDestKey()
{
    if (lookupNodeIds) {
        const NodeHandle& handle = globalNodeList->getRandomNode(0, true,
                                                   onlyLookupInoffensiveNodes);
        return std::make_pair(handle.getKey(), handle);
    }
    // generate random destination key
    return std::make_pair(OverlayKey::random(), TransportAddress::UNSPECIFIED_NODE);
}

bool KBRTestApp::checkSeen(const OverlayKey& key, int seqNum)
{
    MsgHandle hdl(key, seqNum);

    for (MsgHandleBuf::iterator it = mhBufBegin; it != mhBufEnd; ++it) {
        if (it->key.isUnspecified()) {
            continue;
        }
        if (*it == hdl) {
            return true;
        }
    }

    *(mhBufNext++) = hdl;
    if (mhBufNext == mhBufEnd) {
        mhBufNext = mhBufBegin;
    }

    return false;
}

void KBRTestApp::evaluateData(simtime_t latency, int hopCount, long int bytes)
{
    // count the number and size of successfully delivered messages
    RECORD_STATS(numDelivered++; bytesDelivered += bytes;
                 globalStatistics->deliveredKBRTestAppMessages++);

    if (numSent < numDelivered) {
        std::ostringstream tempString;
        tempString << "KBRTestApp::evaluateData(): numSent ("
                   << numSent << ") < numDelivered (" << numDelivered << ")!";
        throw cRuntimeError(tempString.str().c_str());
    }

    RECORD_STATS(globalStatistics->recordOutVector("KBRTestApp: One-way Hop "
                                                   "Count", hopCount));
    RECORD_STATS(globalStatistics->recordOutVector("KBRTestApp: One-way Latency",
                                                   SIMTIME_DBL(latency)));
}

void KBRTestApp::finishApp()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);

    if (time >= GlobalStatistics::MIN_MEASURED) {
        if (kbrOneWayTest) {
            globalStatistics->addStdDev("KBRTestApp: One-way Delivered Messages/s",
                                        numDelivered / time);
            globalStatistics->addStdDev("KBRTestApp: One-way Delivered Bytes/s",
                                        bytesDelivered / time);
            globalStatistics->addStdDev("KBRTestApp: One-way Dropped Messages/s",
                                        numDropped / time);
            globalStatistics->addStdDev("KBRTestApp: One-way Dropped Bytes/s",
                                        bytesDropped / time);
            if (numSent > 0) {
                globalStatistics->addStdDev("KBRTestApp: One-way Delivery Ratio",
                                            (float)numDelivered /
                                            (float)numSent);
            }
        }

        if (kbrRpcTest) {
            globalStatistics->addStdDev("KBRTestApp: RPC Delivered Messages/s",
                                        numRpcDelivered / time);
            globalStatistics->addStdDev("KBRTestApp: RPC Delivered Bytes/s",
                                        bytesRpcDelivered / time);
            globalStatistics->addStdDev("KBRTestApp: RPC Dropped Messages/s",
                                        numRpcDropped / time);
            globalStatistics->addStdDev("KBRTestApp: RPC Dropped Bytes/s",
                                        bytesRpcDropped / time);
#if 0
            if (rpcSuccLatencyCount > 0) {
                globalStatistics->addStdDev("KBRTestApp: RPC Success Session Latency",
                                        SIMTIME_DBL(rpcSuccLatencySum) / rpcSuccLatencyCount);
            }

            if (rpcTotalLatencyCount > 0) {
                globalStatistics->addStdDev("KBRTestApp: RPC Total Session Latency",
                                        SIMTIME_DBL(rpcTotalLatencySum) / rpcTotalLatencyCount);
            }
#endif

            if (numRpcSent > 0) {
                globalStatistics->addStdDev("KBRTestApp: RPC Delivery Ratio",
                                            (float)numRpcDelivered /
                                            (float)numRpcSent);
            }
        }

        if (kbrLookupTest) {
            globalStatistics->addStdDev("KBRTestApp: Successful Lookups/s",
                                        numLookupSuccess / time);
            globalStatistics->addStdDev("KBRTestApp: Failed Lookups/s",
                                        numLookupFailed / time);
            if (numLookupSent > 0) {
                globalStatistics->addStdDev("KBRTestApp: Lookup Success Ratio",
                                            (float)numLookupSuccess /
                                            (float)numLookupSent);
            }
        }
    }
}
