/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 * Copyright (C) 2011 Zoltan Bojthe
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include "inet/linklayer/ethernet/EtherMAC.h"

#include "inet/common/queue/IPassiveQueue.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

// TODO: there is some code that is pretty much the same as the one found in EtherMACFullDuplex.cc (e.g. EtherMAC::beginSendFrames)
// TODO: refactor using a statemachine that is present in a single function
// TODO: this helps understanding what interactions are there and how they affect the state

static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

Define_Module(EtherMAC);

simsignal_t EtherMAC::collisionSignal = registerSignal("collision");
simsignal_t EtherMAC::backoffSignal = registerSignal("backoff");

EtherMAC::~EtherMAC()
{
    delete frameBeingReceived;
    cancelAndDelete(endRxMsg);
    cancelAndDelete(endBackoffMsg);
    cancelAndDelete(endJammingMsg);
}

void EtherMAC::initialize(int stage)
{
    EtherMACBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        endRxMsg = new cMessage("EndReception", ENDRECEPTION);
        endBackoffMsg = new cMessage("EndBackoff", ENDBACKOFF);
        endJammingMsg = new cMessage("EndJamming", ENDJAMMING);

        // initialize state info
        backoffs = 0;
        numConcurrentTransmissions = 0;
        currentSendPkTreeID = 0;

        WATCH(backoffs);
        WATCH(numConcurrentTransmissions);
    }
}

void EtherMAC::initializeStatistics()
{
    EtherMACBase::initializeStatistics();

    framesSentInBurst = 0;
    bytesSentInBurst = 0;

    WATCH(framesSentInBurst);
    WATCH(bytesSentInBurst);

    // initialize statistics
    totalCollisionTime = 0.0;
    totalSuccessfulRxTxTime = 0.0;
    numCollisions = numBackoffs = 0;

    WATCH(numCollisions);
    WATCH(numBackoffs);
}

void EtherMAC::initializeFlags()
{
    EtherMACBase::initializeFlags();

    duplexMode = par("duplexMode").boolValue();
    frameBursting = !duplexMode && par("frameBursting").boolValue();
    physInGate->setDeliverOnReceptionStart(true);
}

void EtherMAC::processConnectDisconnect()
{
    if (!connected) {
        delete frameBeingReceived;
        frameBeingReceived = nullptr;
        cancelEvent(endRxMsg);
        cancelEvent(endBackoffMsg);
        cancelEvent(endJammingMsg);
        bytesSentInBurst = 0;
        framesSentInBurst = 0;
    }

    EtherMACBase::processConnectDisconnect();

    if (connected) {
        if (!duplexMode) {
            // start RX_RECONNECT_STATE
            receiveState = RX_RECONNECT_STATE;
            emit(receiveStateSignal, RX_RECONNECT_STATE);
            simtime_t reconnectEndTime = simTime() + 8 * (MAX_ETHERNET_FRAME_BYTES + JAM_SIGNAL_BYTES) / curEtherDescr->txrate;
            endRxTimeList.clear();
            addReceptionInReconnectState(-1, reconnectEndTime);
        }
    }
}

void EtherMAC::readChannelParameters(bool errorWhenAsymmetric)
{
    EtherMACBase::readChannelParameters(errorWhenAsymmetric);

    if (connected && !duplexMode) {
        if (curEtherDescr->halfDuplexFrameMinBytes < 0.0)
            throw cRuntimeError("%g bps Ethernet only supports full-duplex links", curEtherDescr->txrate);
    }
}

void EtherMAC::handleSelfMessage(cMessage *msg)
{
    // Process different self-messages (timer signals)
    EV_TRACE << "Self-message " << msg << " received\n";

    switch (msg->getKind()) {
        case ENDIFG:
            handleEndIFGPeriod();
            break;

        case ENDTRANSMISSION:
            handleEndTxPeriod();
            break;

        case ENDRECEPTION:
            handleEndRxPeriod();
            break;

        case ENDBACKOFF:
            handleEndBackoffPeriod();
            break;

        case ENDJAMMING:
            handleEndJammingPeriod();
            break;

        case ENDPAUSE:
            handleEndPausePeriod();
            break;

        default:
            throw cRuntimeError("Self-message with unexpected message kind %d", msg->getKind());
    }
}

void EtherMAC::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        handleMessageWhenDown(msg);
        return;
    }

    if (channelsDiffer)
        readChannelParameters(true);

    printState();

    // some consistency check
    if (!duplexMode && transmitState == TRANSMITTING_STATE && receiveState != RX_IDLE_STATE)
        throw cRuntimeError("Inconsistent state -- transmitting and receiving at the same time");

    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else if (msg->getArrivalGate() == upperLayerInGate)
        processFrameFromUpperLayer(check_and_cast<EtherFrame *>(msg));
    else if (msg->getArrivalGate() == physInGate)
        processMsgFromNetwork(check_and_cast<cPacket *>(msg));
    else
        throw cRuntimeError("Message received from unknown gate");

    printState();
}

void EtherMAC::processFrameFromUpperLayer(EtherFrame *frame)
{
    ASSERT(frame->getByteLength() >= MIN_ETHERNET_FRAME_BYTES);

    EV_INFO << "Received " << frame << " from upper layer." << endl;

    emit(packetReceivedFromUpperSignal, frame);

    if (frame->getDest().equals(address)) {
        throw cRuntimeError("Logic error: frame %s from higher layer has local MAC address as dest (%s)",
                frame->getFullName(), frame->getDest().str().c_str());
    }

    if (frame->getByteLength() > MAX_ETHERNET_FRAME_BYTES) {
        throw cRuntimeError("Packet from higher layer (%d bytes) exceeds maximum Ethernet frame size (%d)",
                (int)(frame->getByteLength()), MAX_ETHERNET_FRAME_BYTES);
    }

    if (!connected || disabled) {
        EV_WARN << (!connected ? "Interface is not connected" : "MAC is disabled") << " -- dropping packet " << frame << endl;
        emit(dropPkFromHLIfaceDownSignal, frame);
        numDroppedPkFromHLIfaceDown++;
        delete frame;

        requestNextFrameFromExtQueue();
        return;
    }

    // fill in src address if not set
    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    bool isPauseFrame = (dynamic_cast<EtherPauseFrame *>(frame) != nullptr);

    if (!isPauseFrame) {
        numFramesFromHL++;
        emit(rxPkFromHLSignal, frame);
    }

    if (txQueue.extQueue) {
        ASSERT(curTxFrame == nullptr);
        curTxFrame = frame;
        fillIFGIfInBurst();
    }
    else {
        if (txQueue.innerQueue->isFull())
            throw cRuntimeError("txQueue length exceeds %d -- this is probably due to "
                                "a bogus app model generating excessive traffic "
                                "(or if this is normal, increase txQueueLimit!)",
                    txQueue.innerQueue->getQueueLimit());

        // store frame and possibly begin transmitting
        EV_DETAIL << "Frame " << frame << " arrived from higher layer, enqueueing\n";
        txQueue.innerQueue->insertFrame(frame);

        if (!curTxFrame && !txQueue.innerQueue->isEmpty())
            curTxFrame = (EtherFrame *)txQueue.innerQueue->pop();
    }

    if ((duplexMode || receiveState == RX_IDLE_STATE) && transmitState == TX_IDLE_STATE) {
        EV_DETAIL << "No incoming carrier signals detected, frame clear to send\n";
        startFrameTransmission();
    }
}

void EtherMAC::addReceptionInReconnectState(long packetTreeId, simtime_t endRxTime)
{
    // note: packetTreeId==-1 is legal, and represents a special entry that marks the end of the reconnect state

    // housekeeping: remove expired entries from endRxTimeList
    simtime_t now = simTime();
    while (!endRxTimeList.empty() && endRxTimeList.front().endTime <= now)
        endRxTimeList.pop_front();

    // remove old entry with same packet tree ID (typically: a frame reception
    // doesn't go through but is canceled by a jam signal)
    auto i = endRxTimeList.begin();
    for ( ; i != endRxTimeList.end(); i++) {
        if (i->packetTreeId == packetTreeId) {
            endRxTimeList.erase(i);
            break;
        }
    }

    // find insertion position and insert new entry (list is ordered by endRxTime)
    for (i = endRxTimeList.begin(); i != endRxTimeList.end() && i->endTime <= endRxTime; i++)
        ;
    PkIdRxTime item(packetTreeId, endRxTime);
    endRxTimeList.insert(i, item);

    // adjust endRxMsg if needed (we'll exit reconnect mode when endRxMsg expires)
    simtime_t maxRxTime = endRxTimeList.back().endTime;
    if (endRxMsg->getArrivalTime() != maxRxTime) {
        cancelEvent(endRxMsg);
        scheduleAt(maxRxTime, endRxMsg);
    }
}

void EtherMAC::addReception(simtime_t endRxTime)
{
    numConcurrentTransmissions++;

    if (endRxMsg->getArrivalTime() < endRxTime) {
        cancelEvent(endRxMsg);
        scheduleAt(endRxTime, endRxMsg);
    }
}

void EtherMAC::processReceivedJam(EtherJam *jam)
{
    simtime_t endRxTime = simTime() + jam->getDuration();
    delete jam;

    numConcurrentTransmissions--;
    if (numConcurrentTransmissions < 0)
        throw cRuntimeError("Received JAM without message");

    if (numConcurrentTransmissions == 0 || endRxMsg->getArrivalTime() < endRxTime) {
        cancelEvent(endRxMsg);
        scheduleAt(endRxTime, endRxMsg);
    }

    processDetectedCollision();
}

void EtherMAC::processMsgFromNetwork(cPacket *msg)
{
    EV_DETAIL << "Received " << msg << " from network.\n";

    if (!connected || disabled) {
        EV_WARN << (!connected ? "Interface is not connected" : "MAC is disabled") << " -- dropping msg " << msg << endl;
        if (EtherPhyFrame *phyFrame = dynamic_cast<EtherPhyFrame *>(msg)) {    // do not count JAM and IFG packets
            EtherFrame *frame = decapsulate(phyFrame);
            emit(dropPkIfaceDownSignal, frame);
            delete frame;
            numDroppedIfaceDown++;
        }
        else
            delete msg;

        return;
    }

    // detect cable length violation in half-duplex mode
    if (!duplexMode) {
        simtime_t propagationTime = simTime() - msg->getSendingTime();
        if (propagationTime >= curEtherDescr->maxPropagationDelay) {
            throw cRuntimeError("Very long frame propagation time detected, maybe cable exceeds "
                                "maximum allowed length? (%s corresponds to an approx. %lgm cable)",
                    SIMTIME_STR(propagationTime),
                    SIMTIME_DBL(propagationTime) * SPEED_OF_LIGHT_IN_CABLE);
        }
    }

    simtime_t endRxTime = simTime() + msg->getDuration();
    EtherJam *jamMsg = dynamic_cast<EtherJam *>(msg);

    if (duplexMode && jamMsg) {
        throw cRuntimeError("Stray jam signal arrived in full-duplex mode");
    }
    else if (!duplexMode && receiveState == RX_RECONNECT_STATE) {
        long treeId = jamMsg ? jamMsg->getAbortedPkTreeID() : msg->getTreeId();
        addReceptionInReconnectState(treeId, endRxTime);
        delete msg;
    }
    else if (!duplexMode && (transmitState == TRANSMITTING_STATE || transmitState == SEND_IFG_STATE)) {
        // since we're half-duplex, receiveState must be RX_IDLE_STATE (asserted at top of handleMessage)
        if (jamMsg)
            throw cRuntimeError("Stray jam signal arrived while transmitting (usual cause is cable length exceeding allowed maximum)");

        // set receive state and schedule end of reception
        receiveState = RX_COLLISION_STATE;
        emit(receiveStateSignal, RX_COLLISION_STATE);

        addReception(endRxTime);
        delete msg;

        EV_DETAIL << "Transmission interrupted by incoming frame, handling collision\n";
        cancelEvent((transmitState == TRANSMITTING_STATE) ? endTxMsg : endIFGMsg);

        EV_DETAIL << "Transmitting jam signal\n";
        sendJamSignal();    // backoff will be executed when jamming finished

        numCollisions++;
        emit(collisionSignal, 1L);
    }
    else if (receiveState == RX_IDLE_STATE) {
        if (jamMsg)
            throw cRuntimeError("Stray jam signal arrived (usual cause is cable length exceeding allowed maximum)");

        channelBusySince = simTime();
        EV_INFO << "Reception of " << msg << " started.\n";
        scheduleEndRxPeriod(msg);
    }
    else if (receiveState == RECEIVING_STATE
             && !jamMsg
             && endRxMsg->getArrivalTime() - simTime() < curEtherDescr->halfBitTime)
    {
        // With the above condition we filter out "false" collisions that may occur with
        // back-to-back frames. That is: when "beginning of frame" message (this one) occurs
        // BEFORE "end of previous frame" event (endRxMsg) -- same simulation time,
        // only wrong order.

        EV_DETAIL << "Back-to-back frames: completing reception of current frame, starting reception of next one\n";

        // complete reception of previous frame
        cancelEvent(endRxMsg);
        frameReceptionComplete();

        // calculate usability
        totalSuccessfulRxTxTime += simTime() - channelBusySince;
        channelBusySince = simTime();

        // start receiving next frame
        scheduleEndRxPeriod(msg);
    }
    else {    // (receiveState==RECEIVING_STATE || receiveState==RX_COLLISION_STATE)
              // handle overlapping receptions
        if (jamMsg) {
            processReceivedJam(jamMsg);
        }
        else {    // EtherFrame or EtherPauseFrame
            EV_DETAIL << "Overlapping receptions -- setting collision state\n";
            addReception(endRxTime);
            // delete collided frames: arrived frame as well as the one we're currently receiving
            delete msg;
            processDetectedCollision();
        }
    }
}

void EtherMAC::processDetectedCollision()
{
    if (receiveState != RX_COLLISION_STATE) {
        delete frameBeingReceived;
        frameBeingReceived = nullptr;

        numCollisions++;
        emit(collisionSignal, 1L);
        // go to collision state
        receiveState = RX_COLLISION_STATE;
        emit(receiveStateSignal, RX_COLLISION_STATE);
    }
}

void EtherMAC::handleEndIFGPeriod()
{
    if (transmitState != WAIT_IFG_STATE && transmitState != SEND_IFG_STATE)
        throw cRuntimeError("Not in WAIT_IFG_STATE at the end of IFG period");

    currentSendPkTreeID = 0;

    EV_DETAIL << "IFG elapsed\n";

    if (frameBursting && (transmitState != SEND_IFG_STATE)) {
        bytesSentInBurst = 0;
        framesSentInBurst = 0;
    }

    // End of IFG period, okay to transmit, if Rx idle OR duplexMode ( checked in startFrameTransmission(); )

    // send frame to network
    beginSendFrames();
}

void EtherMAC::startFrameTransmission()
{
    ASSERT(curTxFrame);

    EV_INFO << "Transmission of " << curTxFrame << " started.\n";

    EtherFrame *frame = curTxFrame->dup();

    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    bool inBurst = frameBursting && framesSentInBurst;
    int64 minFrameLength = duplexMode ? curEtherDescr->frameMinBytes : (inBurst ? curEtherDescr->frameInBurstMinBytes : curEtherDescr->halfDuplexFrameMinBytes);

    if (frame->getByteLength() < minFrameLength)
        frame->setByteLength(minFrameLength);

    // add preamble and SFD (Starting Frame Delimiter), then send out
    EtherPhyFrame *phyFrame = encapsulate(frame);

    currentSendPkTreeID = phyFrame->getTreeId();
    int64_t sentFrameByteLength = phyFrame->getByteLength();
    send(phyFrame, physOutGate);

    // check for collisions (there might be an ongoing reception which we don't know about, see below)
    if (!duplexMode && receiveState != RX_IDLE_STATE) {
        // During the IFG period the hardware cannot listen to the channel,
        // so it might happen that receptions have begun during the IFG,
        // and even collisions might be in progress.
        //
        // But we don't know of any ongoing transmission so we blindly
        // start transmitting, immediately collide and send a jam signal.
        //
        EV_DETAIL << "startFrameTransmission(): sending JAM signal.\n";
        printState();

        sendJamSignal();
        // numConcurrentRxTransmissions stays the same: +1 transmission, -1 jam

        if (receiveState == RECEIVING_STATE) {
            delete frameBeingReceived;
            frameBeingReceived = nullptr;

            numCollisions++;
            emit(collisionSignal, 1L);
        }
        // go to collision state
        receiveState = RX_COLLISION_STATE;
        emit(receiveStateSignal, RX_COLLISION_STATE);
    }
    else {
        // no collision
        scheduleEndTxPeriod(sentFrameByteLength);

        // only count transmissions in totalSuccessfulRxTxTime if channel is half-duplex
        if (!duplexMode)
            channelBusySince = simTime();
    }
}

void EtherMAC::handleEndTxPeriod()
{
    // we only get here if transmission has finished successfully, without collision
    if (transmitState != TRANSMITTING_STATE || (!duplexMode && receiveState != RX_IDLE_STATE))
        throw cRuntimeError("End of transmission, and incorrect state detected");

    currentSendPkTreeID = 0;

    if (curTxFrame == nullptr)
        throw cRuntimeError("Frame under transmission cannot be found");

    emit(packetSentToLowerSignal, curTxFrame);    //consider: emit with start time of frame

    if (EtherPauseFrame *pauseFrame = dynamic_cast<EtherPauseFrame *>(curTxFrame)) {
        numPauseFramesSent++;
        emit(txPausePkUnitsSignal, pauseFrame->getPauseTime());
    }
    else {
        unsigned long curBytes = curTxFrame->getByteLength();
        numFramesSent++;
        numBytesSent += curBytes;
        emit(txPkSignal, curTxFrame);
    }

    EV_INFO << "Transmission of " << curTxFrame << " successfully completed.\n";
    delete curTxFrame;
    curTxFrame = nullptr;
    lastTxFinishTime = simTime();
    getNextFrameFromQueue();  // note: cannot be moved into handleEndIFGPeriod(), because in burst mode we need to know whether to send filled IFG or not

    // only count transmissions in totalSuccessfulRxTxTime if channel is half-duplex
    if (!duplexMode) {
        simtime_t dt = simTime() - channelBusySince;
        totalSuccessfulRxTxTime += dt;
    }

    backoffs = 0;

    // check for and obey received PAUSE frames after each transmission
    if (pauseUnitsRequested > 0) {
        // if we received a PAUSE frame recently, go into PAUSE state
        EV_DETAIL << "Going to PAUSE mode for " << pauseUnitsRequested << " time units\n";
        scheduleEndPausePeriod(pauseUnitsRequested);
        pauseUnitsRequested = 0;
    }
    else {
        EV_DETAIL << "Start IFG period\n";
        scheduleEndIFGPeriod();
        if (!txQueue.extQueue)
            fillIFGIfInBurst();
    }
}

void EtherMAC::scheduleEndRxPeriod(cPacket *frame)
{
    ASSERT(frameBeingReceived == nullptr);
    ASSERT(!endRxMsg->isScheduled());

    frameBeingReceived = frame;
    receiveState = RECEIVING_STATE;
    emit(receiveStateSignal, RECEIVING_STATE);
    addReception(simTime() + frame->getDuration());
}

void EtherMAC::handleEndRxPeriod()
{
    simtime_t dt = simTime() - channelBusySince;

    switch (receiveState) {
        case RECEIVING_STATE:
            frameReceptionComplete();
            totalSuccessfulRxTxTime += dt;
            break;

        case RX_COLLISION_STATE:
            EV_DETAIL << "Incoming signals finished after collision\n";
            totalCollisionTime += dt;
            break;

        case RX_RECONNECT_STATE:
            EV_DETAIL << "Incoming signals finished or reconnect time elapsed after reconnect\n";
            endRxTimeList.clear();
            break;

        default:
            throw cRuntimeError("model error: invalid receiveState %d", receiveState);
    }

    receiveState = RX_IDLE_STATE;
    emit(receiveStateSignal, RX_IDLE_STATE);
    numConcurrentTransmissions = 0;

    if (transmitState == TX_IDLE_STATE)
        scheduleEndIFGPeriod();
}

void EtherMAC::handleEndBackoffPeriod()
{
    if (transmitState != BACKOFF_STATE)
        throw cRuntimeError("At end of BACKOFF and not in BACKOFF_STATE");

    if (curTxFrame == nullptr)
        throw cRuntimeError("At end of BACKOFF and no frame to transmit");

    if (receiveState == RX_IDLE_STATE) {
        EV_DETAIL << "Backoff period ended, wait IFG\n";
        scheduleEndIFGPeriod();
    }
    else {
        EV_DETAIL << "Backoff period ended but channel is not free, idling\n";
        transmitState = TX_IDLE_STATE;
        emit(transmitStateSignal, TX_IDLE_STATE);
    }
}

void EtherMAC::sendJamSignal()
{
    if (currentSendPkTreeID == 0)
        throw cRuntimeError("Model error: sending JAM while not transmitting");

    EtherJam *jam = new EtherJam("JAM_SIGNAL");
    jam->setByteLength(JAM_SIGNAL_BYTES);
    jam->setAbortedPkTreeID(currentSendPkTreeID);

    transmissionChannel->forceTransmissionFinishTime(SIMTIME_ZERO);
    //emit(packetSentToLowerSignal, jam);
    send(jam, physOutGate);

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endJammingMsg);
    transmitState = JAMMING_STATE;
    emit(transmitStateSignal, JAMMING_STATE);
}

void EtherMAC::handleEndJammingPeriod()
{
    if (transmitState != JAMMING_STATE)
        throw cRuntimeError("At end of JAMMING but not in JAMMING_STATE");

    EV_DETAIL << "Jamming finished, executing backoff\n";
    handleRetransmission();
}

void EtherMAC::handleRetransmission()
{
    if (++backoffs > MAX_ATTEMPTS) {
        EV_DETAIL << "Number of retransmit attempts of frame exceeds maximum, cancelling transmission of frame\n";
        delete curTxFrame;
        curTxFrame = nullptr;
        transmitState = TX_IDLE_STATE;
        emit(transmitStateSignal, TX_IDLE_STATE);
        backoffs = 0;
        getNextFrameFromQueue();
        beginSendFrames();
        return;
    }

    EV_DETAIL << "Executing backoff procedure\n";
    int backoffRange = (backoffs >= BACKOFF_RANGE_LIMIT) ? 1024 : (1 << backoffs);
    int slotNumber = intuniform(0, backoffRange - 1);

    scheduleAt(simTime() + slotNumber * curEtherDescr->slotTime, endBackoffMsg);
    transmitState = BACKOFF_STATE;
    emit(transmitStateSignal, BACKOFF_STATE);

    numBackoffs++;
    emit(backoffSignal, 1L);
}

void EtherMAC::printState()
{
#define CASE(x)    case x: \
        EV_DETAIL << #x; break

    EV_DETAIL << "transmitState: ";
    switch (transmitState) {
        CASE(TX_IDLE_STATE);
        CASE(WAIT_IFG_STATE);
        CASE(SEND_IFG_STATE);
        CASE(TRANSMITTING_STATE);
        CASE(JAMMING_STATE);
        CASE(BACKOFF_STATE);
        CASE(PAUSE_STATE);
    }

    EV_DETAIL << ",  receiveState: ";
    switch (receiveState) {
        CASE(RX_IDLE_STATE);
        CASE(RECEIVING_STATE);
        CASE(RX_COLLISION_STATE);
        CASE(RX_RECONNECT_STATE);
    }

    EV_DETAIL << ",  backoffs: " << backoffs;
    EV_DETAIL << ",  numConcurrentRxTransmissions: " << numConcurrentTransmissions;

    if (txQueue.innerQueue)
        EV_DETAIL << ",  queueLength: " << txQueue.innerQueue->getLength();

    EV_DETAIL << endl;

#undef CASE
}

void EtherMAC::finish()
{
    EtherMACBase::finish();

    simtime_t t = simTime();
    simtime_t totalChannelIdleTime = t - totalSuccessfulRxTxTime - totalCollisionTime;
    recordScalar("rx channel idle (%)", 100 * (totalChannelIdleTime / t));
    recordScalar("rx channel utilization (%)", 100 * (totalSuccessfulRxTxTime / t));
    recordScalar("rx channel collision (%)", 100 * (totalCollisionTime / t));
    recordScalar("collisions", numCollisions);
    recordScalar("backoffs", numBackoffs);
}

void EtherMAC::handleEndPausePeriod()
{
    if (transmitState != PAUSE_STATE)
        throw cRuntimeError("At end of PAUSE and not in PAUSE_STATE");

    EV_DETAIL << "Pause finished, resuming transmissions\n";
    beginSendFrames();
}

void EtherMAC::frameReceptionComplete()
{
    EtherTraffic *msg = check_and_cast<EtherTraffic *>(frameBeingReceived);
    frameBeingReceived = nullptr;

    if (dynamic_cast<EtherFilledIFG *>(msg) != nullptr) {
        delete msg;
        return;
    }

    bool hasBitError = msg->hasBitError();

    EtherFrame *frame = decapsulate(check_and_cast<EtherPhyFrame *>(msg));
    emit(packetReceivedFromLowerSignal, frame);

    if (hasBitError) {
        numDroppedBitError++;
        emit(dropPkBitErrorSignal, frame);
        delete frame;
        return;
    }

    if (dropFrameNotForUs(frame))
        return;

    if (dynamic_cast<EtherPauseFrame *>(frame) != nullptr) {
        processReceivedPauseFrame((EtherPauseFrame *)frame);
    }
    else {
        EV_INFO << "Reception of " << frame << " successfully completed.\n";
        processReceivedDataFrame(check_and_cast<EtherFrame *>(frame));
    }
}

void EtherMAC::processReceivedDataFrame(EtherFrame *frame)
{
    // statistics
    unsigned long curBytes = frame->getByteLength();
    numFramesReceivedOK++;
    numBytesReceivedOK += curBytes;
    emit(rxPkOkSignal, frame);

    numFramesPassedToHL++;
    emit(packetSentToUpperSignal, frame);
    // pass up to upper layer
    EV_INFO << "Sending " << frame << " to upper layer.\n";
    send(frame, "upperLayerOut");
}

void EtherMAC::processReceivedPauseFrame(EtherPauseFrame *frame)
{
    int pauseUnits = frame->getPauseTime();
    delete frame;

    numPauseFramesRcvd++;
    emit(rxPausePkUnitsSignal, pauseUnits);

    if (transmitState == TX_IDLE_STATE) {
        EV_DETAIL << "PAUSE frame received, pausing for " << pauseUnitsRequested << " time units\n";
        if (pauseUnits > 0)
            scheduleEndPausePeriod(pauseUnits);
    }
    else if (transmitState == PAUSE_STATE) {
        EV_DETAIL << "PAUSE frame received, pausing for " << pauseUnitsRequested
                  << " more time units from now\n";
        cancelEvent(endPauseMsg);

        if (pauseUnits > 0)
            scheduleEndPausePeriod(pauseUnits);
    }
    else {
        // transmitter busy -- wait until it finishes with current frame (endTx)
        // and then it'll go to PAUSE state
        EV_DETAIL << "PAUSE frame received, storing pause request\n";
        pauseUnitsRequested = pauseUnits;
    }
}

void EtherMAC::scheduleEndIFGPeriod()
{
    transmitState = WAIT_IFG_STATE;
    emit(transmitStateSignal, WAIT_IFG_STATE);
    simtime_t endIFGTime = simTime() + (INTERFRAME_GAP_BITS / curEtherDescr->txrate);
    scheduleAt(endIFGTime, endIFGMsg);
}

void EtherMAC::fillIFGIfInBurst()
{
    if (!frameBursting)
        return;

    EV_TRACE << "fillIFGIfInBurst(): t=" << simTime() << ", framesSentInBurst=" << framesSentInBurst << ", bytesSentInBurst=" << bytesSentInBurst << endl;

    if (curTxFrame
        && endIFGMsg->isScheduled()
        && (transmitState == WAIT_IFG_STATE)
        && (simTime() == lastTxFinishTime)
        && (simTime() == endIFGMsg->getSendingTime())
        && (framesSentInBurst > 0)
        && (framesSentInBurst < curEtherDescr->maxFramesInBurst)
        && (bytesSentInBurst + (INTERFRAME_GAP_BITS / 8) + PREAMBLE_BYTES + SFD_BYTES + curTxFrame->getByteLength()
            <= curEtherDescr->maxBytesInBurst)
        )
    {
        EtherFilledIFG *gap = new EtherFilledIFG("FilledIFG");
        bytesSentInBurst += gap->getByteLength();
        currentSendPkTreeID = gap->getTreeId();
        send(gap, physOutGate);
        transmitState = SEND_IFG_STATE;
        emit(transmitStateSignal, SEND_IFG_STATE);
        cancelEvent(endIFGMsg);
        scheduleAt(transmissionChannel->getTransmissionFinishTime(), endIFGMsg);
    }
    else {
        bytesSentInBurst = 0;
        framesSentInBurst = 0;
    }
}

void EtherMAC::scheduleEndTxPeriod(int64_t sentFrameByteLength)
{
    // update burst variables
    if (frameBursting) {
        bytesSentInBurst += sentFrameByteLength;
        framesSentInBurst++;
    }

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endTxMsg);
    transmitState = TRANSMITTING_STATE;
    emit(transmitStateSignal, TRANSMITTING_STATE);
}

void EtherMAC::scheduleEndPausePeriod(int pauseUnits)
{
    // length is interpreted as 512-bit-time units
    simtime_t pausePeriod = pauseUnits * PAUSE_UNIT_BITS / curEtherDescr->txrate;
    scheduleAt(simTime() + pausePeriod, endPauseMsg);
    transmitState = PAUSE_STATE;
    emit(transmitStateSignal, PAUSE_STATE);
}

void EtherMAC::beginSendFrames()
{
    if (curTxFrame) {
        // Other frames are queued, therefore wait IFG period and transmit next frame
        EV_DETAIL << "Will transmit next frame in output queue after IFG period\n";
        startFrameTransmission();
    }
    else {
        // No more frames, set transmitter to idle
        transmitState = TX_IDLE_STATE;
        emit(transmitStateSignal, TX_IDLE_STATE);
        EV_DETAIL << "No more frames to send, transmitter set to idle\n";
    }
}

} // namespace inet

