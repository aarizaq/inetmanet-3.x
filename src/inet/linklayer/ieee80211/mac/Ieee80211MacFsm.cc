
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211MpduA.h"

namespace inet {

namespace ieee80211 {

bool Ieee80211Mac::initFsm(cMessage *msg,bool &receptionError, Ieee80211Frame *& frame)
{

    removeOldTuplesFromDuplicateMap();
    // skip those cases where there's nothing to do, so the switch looks simpler
    if (isUpperMessage(msg) && fsm.getState() != IDLE) {
        if (fsm.getState() == WAITAIFS && endDIFS->isScheduled()) {
            // a difs was schedule because all queues ware empty
            // change difs for aifs
            simtime_t remaint = getAIFS(currentAC) - getDIFS();
            scheduleAt(endDIFS->getArrivalTime() + remaint, endAIFS(currentAC));
            cancelEvent(endDIFS);
        }
        else if (fsm.getState() == BACKOFF && endBackoff(numCategories() - 1)->isScheduled() && transmissionQueue(numCategories() - 1)->empty()) {
            // a backoff was schedule with all the queues empty
            // reschedule the backoff with the appropriate AC
            backoffPeriod(currentAC) = backoffPeriod(numCategories() - 1);
            backoff(currentAC) = backoff(numCategories() - 1);
            backoff(numCategories() - 1) = false;
            scheduleAt(endBackoff(numCategories() - 1)->getArrivalTime(), endBackoff(currentAC));
            cancelEvent(endBackoff(numCategories() - 1));
        }
        EV_DEBUG << "deferring upper message transmission in " << fsm.getStateName() << " state\n";
        return false;
    }

// Special case, is  endTimeout ACK and the radio state  is RECV, the system must wait until end reception (9.3.2.8 ACK procedure)
    if (msg == endTimeout && isMediumRecv() && useModulationParameters && fsm.getState() == WAITACK)
    {
        EV << "Re-schedule WAITACK timeout \n";
        scheduleAt(simTime() + controlFrameTxTime(LENGTH_ACK), endTimeout);
        return false;
    }

    logState();
    stateVector.record(fsm.getState());

    receptionError = false;
    frame = dynamic_cast<Ieee80211Frame*>(msg);
    if (frame && isLowerMessage(frame)) {
        lastReceiveFailed = receptionError = frame ? frame->hasBitError() : false;
        scheduleReservePeriod(frame);
    }
    return true;
}

void Ieee80211Mac::endFsm(cMessage *msg)
{
    EV_TRACE << "leaving handleWithFSM\n\t";
    logState();
    stateVector.record(fsm.getState());
    if (simTime() - last > 0.1)
    {
        for (int i = 0; i<numCategories(); i++)
        {
            throughput(i)->record(bits(i)/(simTime()-last));
            bits(i) = 0;
            if (maxJitter(i) > SIMTIME_ZERO && minJitter(i) > SIMTIME_ZERO)
            {
                jitter(i)->record(maxJitter(i)-minJitter(i));
                maxJitter(i) = SIMTIME_ZERO;
                minJitter(i) = SIMTIME_ZERO;
            }
        }
        last = simTime();
    }
}

void Ieee80211Mac::handleWithFSM(cMessage *msg)
{

    bool receptionError;
    Ieee80211Frame *frame;
    if (!initFsm(msg, receptionError, frame))
        return;

    int frameType = frame ? frame->getType() : -1;

    // TODO: fix bug according to the message: [omnetpp] A possible bug in the Ieee80211's FSM.
    FSMA_Switch(fsm) {
        FSMA_State(IDLE) {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            /*
               if (fixFSM)
               {
               FSMA_Event_Transition(Data-Ready,
                                  // isUpperMessage(msg),
                                  isUpperMessage(msg) && backoffPeriod[currentAC] > 0,
                                  DEFER,
                //ASSERT(isInvalidBackoffPeriod() || backoffPeriod == 0);
                //invalidateBackoffPeriod();
               ASSERT(false);

               );
               FSMA_No_Event_Transition(Immediate-Data-Ready,
                                     //!transmissionQueue.empty(),
                !transmissionQueueEmpty(),
                                     DEFER,
               //  invalidateBackoffPeriod();
                ASSERT(backoff[currentAC]);

               );
               }
             */
            FSMA_Event_Transition(Data - Ready,
                    isUpperMessage(msg),
                    DEFER,
                    ASSERT(isInvalidBackoffPeriod() || backoffPeriod() == SIMTIME_ZERO);
                    invalidateBackoffPeriod();
                    );
            FSMA_No_Event_Transition(Immediate - Data - Ready,
                    !transmissionQueueEmpty(),
                    DEFER,
                    );
            FSMA_Event_Transition(Receive,
                    isLowerMessage(msg),
                    RECEIVE,
                    );
        }
        FSMA_State(DEFER) {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Wait - AIFS,
                    isMediumStateChange(msg) && isMediumFree(),
                    WAITAIFS,
                    ;
                    );
            FSMA_No_Event_Transition(Immediate - Wait - AIFS,
                    isMediumFree() || (!isBackoffPending()),
                    WAITAIFS,
                    ;
                    );
            FSMA_Event_Transition(Receive,
                    isLowerMessage(msg),
                    RECEIVE,
                    ;
                    );
        }
        FSMA_State(WAITAIFS) {
            FSMA_Enter(scheduleAIFSPeriod());

            FSMA_Event_Transition(EDCAF - Do - Nothing,
                    isMsgAIFS(msg) && transmissionQueue()->empty(),
                    WAITAIFS,
                    ASSERT(0 == 1);
                    ;
                    );
            FSMA_Event_Transition(Immediate - Transmit - RTS,
                    isMsgAIFS(msg) && !transmissionQueue()->empty() && !isMulticast(getCurrentTransmission())
                    && dynamic_cast<Ieee80211MpduA*>(getCurrentTransmission())  && !backoff() && useRtsMpduA,
                    WAITCTS,
                    sendRTSFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    );
            FSMA_Event_Transition(Immediate - Transmit - MPDU,
                    isMsgAIFS(msg) && !transmissionQueue()->empty() && !isMulticast(getCurrentTransmission())
                    && isMpduA(getCurrentTransmission())  && !backoff() && !useRtsMpduA,
                    WAITBLOCKACK,
                    sendRTSFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    );
            FSMA_Event_Transition(Immediate - Transmit - RTS,
                    isMsgAIFS(msg) && !transmissionQueue()->empty() && !isMulticast(getCurrentTransmission())
                    && getCurrentTransmission()->getByteLength() >= rtsThreshold && !backoff(),
                    WAITCTS,
                    sendRTSFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    );
            FSMA_Event_Transition(Immediate - Transmit - Multicast,
                    isMsgAIFS(msg) && isMulticast(getCurrentTransmission()) && !backoff(),
                    WAITMULTICAST,
                    sendMulticastFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    );
            FSMA_Event_Transition(Immediate - Transmit - Data,
                    isMsgAIFS(msg) && !isMulticast(getCurrentTransmission()) && !backoff(),
                    WAITACK,
                    sendDataFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    cancelAIFSPeriod();
                    );
            /*FSMA_Event_Transition(AIFS-Over,
                                  isMsgAIFS(msg) && backoff[currentAC],
                                  BACKOFF,
                if (isInvalidBackoffPeriod())
                    generateBackoffPeriod();
               );*/
            FSMA_Event_Transition(AIFS - Over,
                    isMsgAIFS(msg),
                    BACKOFF,
                    if (isInvalidBackoffPeriod())
                        generateBackoffPeriod();
                    );
            // end the difs and no other packet has been received
            FSMA_Event_Transition(DIFS - Over,
                    msg == endDIFS && transmissionQueueEmpty(),
                    BACKOFF,
                    currentAC = numCategories() - 1;
                    if (isInvalidBackoffPeriod())
                        generateBackoffPeriod();
                    );
            FSMA_Event_Transition(DIFS - Over,
                    msg == endDIFS,
                    BACKOFF,
                    for (int i = numCategories() - 1; i >= 0; i--) {
                        if (!transmissionQueue(i)->empty()) {
                            currentAC = i;
                        }
                    }
                    if (isInvalidBackoffPeriod())
                        generateBackoffPeriod();
                    );
            FSMA_Event_Transition(Busy,
                    isMediumStateChange(msg) && !isMediumFree(),
                    DEFER,
                    for (int i = 0; i < numCategories(); i++) {
                        if (endAIFS(i)->isScheduled())
                            backoff(i) = true;
                    }
                    if (endDIFS->isScheduled())
                        backoff(numCategories() - 1) = true;
                    cancelAIFSPeriod();
                    );
            FSMA_No_Event_Transition(Immediate - Busy,
                    !isMediumFree(),
                    DEFER,
                    for (int i = 0; i < numCategories(); i++) {
                        if (endAIFS(i)->isScheduled())
                            backoff(i) = true;
                    }
                    if (endDIFS->isScheduled())
                        backoff(numCategories() - 1) = true;
                    cancelAIFSPeriod();

                    );
            // radio state changes before we actually get the message, so this must be here
            FSMA_Event_Transition(Receive,
                    isLowerMessage(msg),
                    RECEIVE,
                    cancelAIFSPeriod();
                    ;
                    );
        }
        FSMA_State(BACKOFF) {
            FSMA_Enter(scheduleBackoffPeriod());
            if (getCurrentTransmission())
            {
                FSMA_Event_Transition(Transmit-RTS,
                                      msg == endBackoff() && !isMulticast(getCurrentTransmission())
                                      && getCurrentTransmission()->getByteLength() >= rtsThreshold,
                                      WAITCTS,
                                      sendRTSFrame(getCurrentTransmission());
                                      oldcurrentAC = currentAC;
                                      cancelAIFSPeriod();
                                      decreaseBackoffPeriod();
                                      cancelBackoffPeriod();
                                     );
                FSMA_Event_Transition(Transmit-Multicast,
                                      msg == endBackoff() && isMulticast(getCurrentTransmission()),
                                      WAITMULTICAST,
                                      sendMulticastFrame(getCurrentTransmission());
                                      oldcurrentAC = currentAC;
                                      cancelAIFSPeriod();
                                      decreaseBackoffPeriod();
                                      cancelBackoffPeriod();
                                     );
                FSMA_Event_Transition(Transmit-Data,
                                      msg == endBackoff() && !isMulticast(getCurrentTransmission()),
                                      WAITACK,
                                      sendDataFrame(getCurrentTransmission());
                                      oldcurrentAC = currentAC;
                                      cancelAIFSPeriod();
                                      decreaseBackoffPeriod();
                                      cancelBackoffPeriod();
                                     );
            }
            FSMA_Event_Transition(AIFS-Over-backoff,
                                  isMsgAIFS(msg) && backoff(),
                                  BACKOFF,
                                  if (isInvalidBackoffPeriod())
                                  generateBackoffPeriod();
                                 );
            FSMA_Event_Transition(AIFS-Immediate-Transmit-RTS,
                                  isMsgAIFS(msg) && !transmissionQueue()->empty() && !isMulticast(getCurrentTransmission())
                                  && getCurrentTransmission()->getByteLength() >= rtsThreshold && !backoff(),
                                  WAITCTS,
                                  sendRTSFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                  cancelAIFSPeriod();
                                  decreaseBackoffPeriod();
                                  cancelBackoffPeriod();
                                 );
            FSMA_Event_Transition(AIFS-Immediate-Transmit-Multicast,
                                  isMsgAIFS(msg) && isMulticast(getCurrentTransmission()) && !backoff(),
                                  WAITMULTICAST,
                                  sendMulticastFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                  cancelAIFSPeriod();
                                  decreaseBackoffPeriod();
                                  cancelBackoffPeriod();
                                 );
            FSMA_Event_Transition(AIFS-Immediate-Transmit-Data,
                                  isMsgAIFS(msg) && !isMulticast(getCurrentTransmission()) && !backoff(),
                                  WAITACK,
                                  sendDataFrame(getCurrentTransmission());
                                  oldcurrentAC = currentAC;
                                  cancelAIFSPeriod();
                                  decreaseBackoffPeriod();
                                  cancelBackoffPeriod();
                                 );
            FSMA_Event_Transition(Backoff-Idle,
                                  isBackoffMsg(msg) && transmissionQueueEmpty(),
                                  IDLE,
                                  resetStateVariables();
                                  );
            FSMA_Event_Transition(Backoff-Busy,
                                  isMediumStateChange(msg) && !isMediumFree(),
                                  DEFER,
                                  cancelAIFSPeriod();
                                  decreaseBackoffPeriod();
                                  cancelBackoffPeriod();
                                 );

        }
        FSMA_State(WAITACK)
        {
            FSMA_Enter(scheduleDataTimeoutPeriod(getCurrentTransmission()));
#ifndef DISABLEERRORACK
            FSMA_Event_Transition(Reception-ACK-failed,
                                  isLowerMessage(msg) && receptionError && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                                  IDLE,
                                  currentAC=oldcurrentAC;
                                  cancelTimeoutPeriod();
                                  giveUpCurrentTransmission();
                                  txop = false;
                                  if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
            FSMA_Event_Transition(Reception-ACK-error,
                                  isLowerMessage(msg) && receptionError,
                                  DEFER,
                                  currentAC=oldcurrentAC;
                                  cancelTimeoutPeriod();
                                  retryCurrentTransmission();
                                  txop = false;
                                  if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
#endif
            FSMA_Event_Transition(Receive-ACK-TXOP-Empty,
                                  isLowerMessage(msg) && isForUs(frame) && frameType == ST_ACK && txop && transmissionQueue(oldcurrentAC)->size() == 1,
                                  DEFER,
                                  sendNotification(NF_TX_ACKED);// added by aaq
                                  currentAC = oldcurrentAC;
                                  if (retryCounter() == 0) numSentWithoutRetry()++;
                                  numSent()++;
                                  fr = getCurrentTransmission();
                                  numBits += fr->getBitLength();
                                  bits() += fr->getBitLength();
                                  macDelay()->record(simTime() - fr->getMACArrive());
                                  if (maxJitter() == SIMTIME_ZERO || maxJitter() < (simTime() - fr->getMACArrive()))
                                      maxJitter() = simTime() - fr->getMACArrive();
                                  if (minJitter() == SIMTIME_ZERO || minJitter() > (simTime() - fr->getMACArrive()))
                                      minJitter() = simTime() - fr->getMACArrive();
                                  EV_DEBUG << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() <<endl;
                                  numSentTXOP++;
                                  cancelTimeoutPeriod();
                                  finishCurrentTransmission();
                                  resetCurrentBackOff();
                                  txop = false;
                                  if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                  );
            FSMA_Event_Transition(Receive-ACK-TXOP,
                                  isLowerMessage(msg) && isForUs(frame) && frameType == ST_ACK && txop,
                                  WAITSIFS,
                                  sendNotification(NF_TX_ACKED);// added by aaq
                                  currentAC = oldcurrentAC;
                                  if (retryCounter() == 0) numSentWithoutRetry()++;
                                  numSent()++;
                                  fr = getCurrentTransmission();
                                  numBits += fr->getBitLength();
                                  bits() += fr->getBitLength();


                                  macDelay()->record(simTime() - fr->getMACArrive());
                                  if (maxJitter() == SIMTIME_ZERO || maxJitter() < (simTime() - fr->getMACArrive()))
                                      maxJitter() = simTime() - fr->getMACArrive();
                                  if (minJitter() == SIMTIME_ZERO || minJitter() > (simTime() - fr->getMACArrive()))
                                      minJitter() = simTime() - fr->getMACArrive();
                                  EV_DEBUG << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() <<endl;
                                  numSentTXOP++;
                                  cancelTimeoutPeriod();
                                  finishCurrentTransmission();
                                  );
/*
            FSMA_Event_Transition(Receive-ACK,
                                  isLowerMessage(msg) && isForUs(frame) && frameType == ST_ACK,
                                  IDLE,
                                  currentAC=oldcurrentAC;
                                  if (retryCounter[currentAC] == 0) numSentWithoutRetry[currentAC]++;
                                  numSent[currentAC]++;
                                  fr=getCurrentTransmission();
                                  numBites += fr->getBitLength();
                                  bits[currentAC] += fr->getBitLength();

                                  macDelay[currentAC].record(simTime() - fr->getMACArrive());
                                  if (maxjitter[currentAC] == 0 || maxjitter[currentAC] < (simTime() - fr->getMACArrive())) maxjitter[currentAC]=simTime() - fr->getMACArrive();
                                      if (minjitter[currentAC] == 0 || minjitter[currentAC] > (simTime() - fr->getMACArrive())) minjitter[currentAC]=simTime() - fr->getMACArrive();
                                          EV_DEBUG << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() <<endl;

                                          cancelTimeoutPeriod();
                                          finishCurrentTransmission();
                                         );

             */
             /*Ieee 802.11 2007 9.9.1.2 EDCA TXOPs*/
             FSMA_Event_Transition(Receive-ACK,
                                  isLowerMessage(msg) && isForUs(frame) && frameType == ST_ACK,
                                  DEFER,
                                  sendNotification(NF_TX_ACKED);// added by mhn **************************************
                                  currentAC = oldcurrentAC;
                                  if (retryCounter() == 0)
                                      numSentWithoutRetry()++;
                                  numSent()++;
                                  fr = getCurrentTransmission();
                                  numBits += fr->getBitLength();
                                  bits() += fr->getBitLength();

                                  macDelay()->record(simTime() - fr->getMACArrive());
                                  if (maxJitter() == SIMTIME_ZERO || maxJitter() < (simTime() - fr->getMACArrive()))
                                      maxJitter() = simTime() - fr->getMACArrive();
                                  if (minJitter() == SIMTIME_ZERO || minJitter() > (simTime() - fr->getMACArrive()))
                                      minJitter() = simTime() - fr->getMACArrive();
                                  EV_DEBUG << "record macDelay AC" << currentAC << " value " << simTime() - fr->getMACArrive() <<endl;
                                  cancelTimeoutPeriod();
                                  finishCurrentTransmission();
                                  resetCurrentBackOff();
                                  );
            FSMA_Event_Transition(Transmit-Data-Failed,
                                  msg == endTimeout && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                                  IDLE,
                                  currentAC = oldcurrentAC;
                                  giveUpCurrentTransmission();
                                  txop = false;
                                  if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
            FSMA_Event_Transition(Receive-ACK-Timeout,
                                  msg == endTimeout,
                                  DEFER,
                                  currentAC = oldcurrentAC;
                                  retryCurrentTransmission();
                                  txop = false;
                                  if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
            FSMA_Event_Transition(Interrupted-ACK-Failure,
                                  isLowerMessage(msg) && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                                  RECEIVE,
                                  currentAC=oldcurrentAC;
                                  cancelTimeoutPeriod();
                                  giveUpCurrentTransmission();
                                  txop = false;
                                  if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
            FSMA_Event_Transition(Retry-Interrupted-ACK,
                                 isLowerMessage(msg),
                                 RECEIVE,
                                 currentAC=oldcurrentAC;
                                 cancelTimeoutPeriod();
                                 retryCurrentTransmission();
                                 txop = false;
                                 if (endTXOP->isScheduled()) cancelEvent(endTXOP);
                                 );
        }

        FSMA_State(WAITBLOCKACK)
        {

        }
        // wait until multicast is sent
        FSMA_State(WAITMULTICAST)
        {
            FSMA_Enter(scheduleMulticastTimeoutPeriod(getCurrentTransmission()));
            /*
                        FSMA_Event_Transition(Transmit-Multicast,
                                              msg == endTimeout,
                                              IDLE,
                            currentAC=oldcurrentAC;
                            finishCurrentTransmission();
                            numSentMulticast++;
                        );
            */
            ///changed
            FSMA_Event_Transition(Transmit-Multicast,
                                  msg == endTimeout,
                                  DEFER,
                                  currentAC = oldcurrentAC;
                                  fr = getCurrentTransmission();
                                  numBits += fr->getBitLength();
                                  bits() += fr->getBitLength();
                                  finishCurrentTransmission();
                                  numSentMulticast++;
                                  resetCurrentBackOff();
                                 );
        }
        // accoriding to 9.2.5.7 CTS procedure
        FSMA_State(WAITCTS)
        {
            FSMA_Enter(scheduleCTSTimeoutPeriod());
#ifndef DISABLEERRORACK
            FSMA_Event_Transition(Reception-CTS-Failed,
                                   isLowerMessage(msg) && receptionError && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                                   IDLE,
                                   cancelTimeoutPeriod();
                                   currentAC = oldcurrentAC;
                                   giveUpCurrentTransmission();
                                  );
            FSMA_Event_Transition(Reception-CTS-error,
                                   isLowerMessage(msg) && receptionError,
                                   DEFER,
                                   cancelTimeoutPeriod();
                                   currentAC = oldcurrentAC;
                                   retryCurrentTransmission();
                                  );
#endif
            FSMA_Event_Transition(Receive-CTS,
                                  isLowerMessage(msg) && isForUs(frame) && frameType == ST_CTS,
                                  WAITSIFS,
                                  cancelTimeoutPeriod();
                                 );
            FSMA_Event_Transition(Transmit-RTS-Failed,
                                  msg == endTimeout && retryCounter(oldcurrentAC) == transmissionLimit - 1,
                                  IDLE,
                                  currentAC = oldcurrentAC;
                                  giveUpCurrentTransmission();
                                 );
            FSMA_Event_Transition(Receive-CTS-Timeout,
                                  msg == endTimeout,
                                  DEFER,
                                  currentAC = oldcurrentAC;
                                  retryCurrentTransmission();
                                 );
        }
        FSMA_State(WAITSIFS)
        {
            FSMA_Enter(scheduleSIFSPeriod(frame));
            FSMA_Event_Transition(Transmit-Data-TXOP,
                    msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_ACK,
                    WAITACK,
                    sendDataFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    );
            FSMA_Event_Transition(Transmit-CTS,
                    msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_RTS,
                    IDLE,
                    sendCTSFrameOnEndSIFS();
                    finishReception();
                    );
            FSMA_Event_Transition(Transmit-DATA,
                    msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_CTS && isMpduA(getCurrentTransmission()),
                    WAITBLOCKACK,
                    sendDataFrameOnEndSIFS(getCurrentTransmission());
                    oldcurrentAC = currentAC;
            );
            FSMA_Event_Transition(Transmit-DATA,
                    msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_CTS,
                    WAITACK,
                    sendDataFrameOnEndSIFS(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    );

            FSMA_Event_Transition(Transmit-ACK,
                    msg == endSIFS && isDataOrMgmtFrame(getFrameReceivedBeforeSIFS()),
                    IDLE,
                    sendACKFrameOnEndSIFS();
                    finishReception();
                    );
        }
        // this is not a real state
        FSMA_State(RECEIVE)
        {
            FSMA_No_Event_Transition(Immediate-Receive-MPDUA,
                    isLowerMessage(msg)  && isForUs(frame) && isMpduA(frame),
                    WAITSIFS,
                    processMpduA(frame);
                    finishReception();
                    );
            FSMA_No_Event_Transition(Immediate-Receive-Error,
                    isLowerMessage(msg) && receptionError,
                    IDLE,
                    EV << "received frame contains bit errors or collision, next wait period is EIFS\n";
                    numCollision++;
                    finishReception();
                    );
            FSMA_No_Event_Transition(Immediate-Receive-Multicast,
                    isLowerMessage(msg) && isMulticast(frame) && !isSentByUs(frame) && isDataOrMgmtFrame(frame),
                    IDLE,
                    sendUp(frame);
                    numReceivedMulticast++;
                    finishReception();
                    );
            FSMA_No_Event_Transition(Immediate-Receive-Data,
                    isLowerMessage(msg) && isForUs(frame) && isDataOrMgmtFrame(frame),
                    WAITSIFS,
                    sendUp(frame);
                    numReceived++;
                    );
            FSMA_No_Event_Transition(Immediate-Receive-RTS,
                    isLowerMessage(msg) && isForUs(frame) && frameType == ST_RTS,
                    WAITSIFS,
                    );
            FSMA_No_Event_Transition(Immediate-Receive-Other-backtobackoff,
                    isLowerMessage(msg) && isBackoffPending(), //(backoff[0] || backoff[1] || backoff[2] || backoff[3]),
                    DEFER,
                    );

            FSMA_No_Event_Transition(Immediate-Promiscuous-Data,
                    isLowerMessage(msg) && !isForUs(frame) && isDataOrMgmtFrame(frame),
                    IDLE,
                    promiscousFrame(frame);
                    finishReception();
                    numReceivedOther++;
                    );
            FSMA_No_Event_Transition(Immediate-Receive-Other,
                    isLowerMessage(msg),
                    IDLE,
                    finishReception();
                    numReceivedOther++;
                    );
        }
    }
    endFsm(msg);
}


}

}
