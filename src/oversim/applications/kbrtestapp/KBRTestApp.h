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
 * @file KBRTestApp.h
 * @author Bernhard Heep, Ingmar Baumgart
 */

#ifndef __KBRTESTAPP_H_
#define __KBRTESTAPP_H_

#include <omnetpp.h>

#include <OverlayKey.h>

#include "BaseApp.h"

class KBRTestMessage;
class KbrTestCall;

/**
 * Test application for KBR interface
 *
 * Test application for KBR interface that sends periodically
 * test messages to random keys or existing nodeIDs. The receiver does
 * not send back any answer, but sender::evaluateDate() is called.
 */
class KBRTestApp : public BaseApp
{
public:
    KBRTestApp();
    ~KBRTestApp();

private:

    /**
     * type for storing seen messages in a circular buffer, holds
     * OverlayKey of the sender and SequenceNumber
     */
    struct MsgHandle
    {
        OverlayKey key;
        int seqNum;

        MsgHandle(void) :
            key(OverlayKey::UNSPECIFIED_KEY), seqNum(-1) {};
        MsgHandle(const OverlayKey& key, int seqNum) :
            key(key), seqNum(seqNum) {};
        bool operator==(const MsgHandle& rhs) const {
            return ((key == rhs.key) && (seqNum == rhs.seqNum));
        };
        MsgHandle& operator=(const MsgHandle& rhs) {
            key = rhs.key;
            seqNum = rhs.seqNum;
            return (*this);
        };
    };
    typedef std::vector<MsgHandle> MsgHandleBuf;

    void initializeApp(int stage);
    void finishApp();
    void handleTimerEvent(cMessage* msg);

    void deliver(OverlayKey& key, cMessage* msg);

    void forward(OverlayKey* key, cPacket** msg, NodeHandle* nextHopNode);

    /**
     * Checks if a message was already seen before. If the sequence
     * number of the message is new for the given sender key, it is stored
     * in a circular buffer and false is returned.
     *
     * @param key the OverlayKey of the sender
     * @param seqNum sequence number of the message to check
     * @return true if the message was seen before
     */
    bool checkSeen(const OverlayKey& key, int seqNum);

    /**
     * Analyses and records measuring data handed over from
     * nodes that previously
     * had been the destination for a test message from this module,
     * called by receiver::handleMessage() at sendermodule
     *
     * @param timeDelay packet-delay
     * @param hopCount packet hop-count
     * @param bytes packet size in bytes
     */
    void evaluateData(simtime_t timeDelay, int hopCount, long int bytes);

    bool handleRpcCall(BaseCallMessage* msg);
    void kbrTestCall(KbrTestCall* call);

    void handleRpcResponse(BaseResponseMessage* msg, cPolymorphic* context,
                           int rpcId, simtime_t rtt);

    void handleRpcTimeout(BaseCallMessage* msg,
                          const TransportAddress& dest,
                          cPolymorphic* context, int rpcId,
                          const OverlayKey& destKey);

    void handleLookupResponse(LookupResponse* msg,
                              cObject* context, simtime_t latency);

    void pingResponse(PingResponse* response, cPolymorphic* context,
                      int rpcId, simtime_t rtt);

    // see BaseApp.h
    virtual void handleNodeLeaveNotification();

    std::pair<OverlayKey, TransportAddress> createDestKey();

    bool kbrOneWayTest;
    bool kbrRpcTest;
    bool kbrLookupTest;

    int testMsgSize;
    double mean; //!< mean time interval between sending test messages
    double deviation; //!< deviation of time interval
    bool activeNetwInitPhase; //!< is app active in network init phase?
    bool lookupNodeIds;  //!< lookup only existing nodeIDs
    bool nodeIsLeavingSoon; //!< true if the node is going to be killed shortly
    bool onlyLookupInoffensiveNodes; //!< if true only search for inoffensive nodes (use together with lookupNodeIds)

    uint32_t numSent;
    uint32_t bytesSent;
    uint32_t numDelivered; //!< number of delivered packets
    uint32_t bytesDelivered; //!< number of delivered bytes
    uint32_t numDropped;
    uint32_t bytesDropped;

    uint32_t numRpcSent;
    uint32_t bytesRpcSent;
    uint32_t numRpcDelivered; //!< number of delivered packets
    uint32_t bytesRpcDelivered; //!< number of delivered bytes
    uint32_t numRpcDropped;
    uint32_t bytesRpcDropped;

    simtime_t rpcSuccLatencySum;
    uint32_t rpcSuccLatencyCount;

    simtime_t rpcTotalLatencySum;
    uint32_t rpcTotalLatencyCount;

    uint32_t numLookupSent;
    uint32_t numLookupSuccess;
    uint32_t numLookupFailed;

    // TODO lookup stats

    cMessage* onewayTimer;
    cMessage* rpcTimer;
    cMessage* lookupTimer;

    simtime_t failureLatency; /**< this latency is recorded for failed lookups and RPCs */

    uint32_t sequenceNumber;
    int msgHandleBufSize; //!< how many MsgHandles to store in circular buffer
    MsgHandleBuf mhBuf; //!< circular buffer of MsgHandles
    MsgHandleBuf::iterator mhBufBegin; //!< begin of circular buffer
    MsgHandleBuf::iterator mhBufNext; //!< next element to insert
    MsgHandleBuf::iterator mhBufEnd; //!< end of circular buffer
};

#endif
