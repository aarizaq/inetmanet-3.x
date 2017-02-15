//**************************************************************************
// * file:        DMAMACSink main file
// *
// * author:      A. Ajith Kumar S.
// * copyright:   (c) A. Ajith Kumar S. 
// * homepage:    www.hib.no/ansatte/aaks
// * email:       aji3003 @ gmail.com
// **************************************************************************
// * part of:     A dual mode adaptive MAC (DMAMAC) protocol for WSAN.
// * Refined on:  25-Apr-2015
// **************************************************************************
// *This file is part of DMAMAC (DMAMAC Protocol Implementation on MiXiM-OMNeT).
// *
// *DMAMAC is free software: you can redistribute it and/or modify
// *it under the terms of the GNU General Public License as published by
// *the Free Software Foundation, either version 3 of the License, or
// *(at your option) any later version.
// *
// *DMAMAC is distributed in the hope that it will be useful,
// *but WITHOUT ANY WARRANTY; without even the implied warranty of
// *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *GNU General Public License for more details.
// *
// *You should have received a copy of the GNU General Public License
// *along with DMAMAC.  If not, see <http://www.gnu.org/licenses/>./
// **************************************************************************

#include "inet/linklayer/dmamac/DMAMACSink.h"
#include "inet/common/INETUtils.h"
#include "inet/common/INETMath.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/FindModule.h"
#include <cstdlib>

namespace inet {

#define myId ((myMacAddr.getInt() & 0xFFFF)-1-baseAddress)

Define_Module(DMAMACSink);

/* EV or print statements can be changed to EV_DEBUG incase you need to hide them */

/* @brief To set the nodeId for the current node  */
/* @brief Initialize the mac using omnetpp.ini variables and initializing other necessary variables  */

void DMAMACSink::initialize(int stage)
{
    DMAMAC::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        /* @brief Initializing to false to not let it take random value */
        changeMacModeInNextSuperFrame = false;

        /* @brief Initializing the actuator sending part. */
        sinkInitialize();
        isSincronized = true;
    }
    else if (stage == INITSTAGE_LINK_LAYER)
    {    }
}

/* @brief Handles the messages sent to self, mainly timers to specify slots */
void DMAMACSink::handleSelfMessage(cMessage* msg)
{
    if (hoppingTimer && msg == hoppingTimer) {
        scheduleAt(simTime()+slotDuration, hoppingTimer);
        // change channel
        setRandSeq(simTime().raw());
        setNextSequenceChannel();
        return;
    }

    EV << "Self-Message Arrived with type : " << msg->getKind() << "Current mode of operation is :" << currentMacMode << endl;

    /* @brief To check if collision has resulted in a switch failure */
    if(checkForSuperframeChange && !changeMacMode && currentSlot == 0)
    {
       EV << "Switch failed due to collision" << endl;
       nbFailedSwitch++;
       checkForSuperframeChange = false;
    }
    /* @brief To set checkForSuperframeChange as false when superframe actually changes */
    if(checkForSuperframeChange && changeMacMode && currentSlot == 0)
        checkForSuperframeChange = false;




    /* @brief To change superframe when currentSlot == 0, Transient to steady, 
	 * and at multiples of numSlotsTransient (transient parts) if required for emergency transient switch
	 */
    if (currentSlot == 0 || (currentSlot % numSlotsTransient) == 0)
    {
        /* @brief If we need to change to new operational mode */
        if (changeMacMode)
        {
            EV << " MAC to change operational mode : " << currentMacMode << " to : " << previousMacMode << endl;
            if (currentMacMode == TRANSIENT)
            {
               previousMacMode = TRANSIENT;
               currentMacMode = STEADY;
               nbTransientToSteady++;
            }
            else
            {
               previousMacMode = STEADY;
               currentMacMode = TRANSIENT;
               nbSteadyToTransient++;
            }
            
			EV << " Current slot at change of operational modes " << (currentSlot % numSlots) << endl;

            /* €brief For the case of emergency switch resetting currentSlot counter to 0 */
            if (currentSlot % numSlots == numSlotsTransient)
            {
               currentSlot = 0;
               EV << " Emergency switch made before steady state completion " << endl;
               nbMidSwitch++;
            }
            changeSuperFrame(currentMacMode);
        }
    }

    /* @brief For printing and creating packets till actuators 
	 * Needs modification if topology is changed, since creating here is partly manual	
  	 */
    if (currentSlot == 0)
    {
        EV << "New Superframe is starting " << endl;
        //setNextSequenceChannel();
        macPeriod = DATA;
        /* @Statistics DMAMACSink */
        if (currentMacMode == TRANSIENT)
            nbTransient++;
        else
            nbSteady++;

        /* @brief Preparing packets for actuators set with destination address.*/
        int i=0;

        while(macPktQueue.size() < queueLength)
        {
            DMAMACPkt* actuatorData = new DMAMACPkt();
            destAddr = MACAddress(actuatorNodes[i]);
            actuatorData->setDestAddr(destAddr);
            macPktQueue.push_back(actuatorData);
            actuatorData->setBitLength(headerLength);
            i++;
        }
    }
    /* To mark Alert period for MAC */
    if (currentSlot >= numSlotsTransient)
        macPeriod = ALERT;

    /* @brief Printing number of slots for debugging */
    EV << "nbSlots = " << numSlots << ", currentSlot = " << currentSlot << ", mySlot = " << mySlot << endl;
    EV << "In this slot transmitting node is : " << transmitSlot[currentSlot] << endl;
    EV << "Current RadioMode : " << radio->getRadioMode() << "Current MacState " << currentMacState << endl;
    EV << "Number of steady superframes until now :" << nbSteady << endl;

    switch (msg->getKind())
    {
        /* @brief Nodes enter SETUP phase to start the MAC protocol  */
        case DMAMAC_STARTUP:

                currentMacState = STARTUP;
                currentSlot = 0;

                /* @brief We start with sending notification message from the sink */
                scheduleAt(simTime(), sendNotification);
                if (hoppingTimer)
                    scheduleAt(simTime()+slotDuration, hoppingTimer);
                break;

        /* @brief Sleep state definition */
        case DMAMAC_SLEEP:

              currentMacState = MAC_SLEEP;
              /* Finds the next slot to wake up */
              findDistantNextSlot();
              break;

        /* @brief Waiting for ACK packet state definition */
        case DMAMAC_WAIT_ACK:

              currentMacState = WAIT_ACK;
              EV << "Switching radio to RX, waiting for ACK" << endl;
              radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);

              /* @brief Wait only until ACK timeout */
              scheduleAt(simTime() + ackTimeoutValue, ackTimeout);
              break;

        /* @brief Waiting for DATA packet state definition */
        case DMAMAC_WAIT_DATA:

                currentMacState = WAIT_DATA;
                EV << "<SINK> receive slot\n";
                EV << "Scheduling data timeout with value : " << dataTimeoutValue << endl;   
                scheduleAt(simTime() + dataTimeoutValue, dataTimeout);                  

                /* @brief Checking if the radio is already in receive mode */
                if (radio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER)
                {
                    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                    EV << "Switching Radio to receive mode" << endl;
                    findImmediateNextSlot(currentSlot, slotDuration);
                }
                else
                {
                    EV << "Radio already in receive mode" << endl;
                    findImmediateNextSlot(currentSlot, slotDuration);
                }

				/* @brief increments currentlot varaible after next slot is found also currentslot is kept in bounds */
                currentSlot++;
                currentSlot %= numSlots;
                break;

        /* @brief Waiting for DATA packet state definition */
        case DMAMAC_WAIT_ALERT:

                currentMacState = WAIT_ALERT;
                EV << "<SINK> Alert receive slot\n";

                if(macType == TDMA)
                {
                    EV << "Scheduling alert timeout with value : " << alertTimeoutValue << endl;   // 9 Dec
                    scheduleAt(simTime() + alertTimeoutValue, alertTimeout);
                }

                /* @brief Checking if the radio is already in receive mode */
                if (radio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER)
                {
                    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                    EV << "Switching Radio to receive mode" << endl;
                    findImmediateNextSlot(currentSlot, slotDuration);
                }
                else
                {
                    EV << "Radio already in receive mode" << endl;
                    findImmediateNextSlot(currentSlot, slotDuration);
                }

                currentSlot++;
                currentSlot %= numSlots;
                break;

        /* @brief Handling receiving of ACK */
           case DMAMAC_ACK_RECEIVED:

				  /* @brief Radio put to sleep until next slot or operation */	
                  EV << "ACK Received" << endl;
                  if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
                      radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                  break;

           /* @brief Handling ACK timeout possibility (ACK packet lost)  */
           case DMAMAC_ACK_TIMEOUT:

                  /* @brief Calculating number of transmission failures  */
                  nbTxDataFailures++;
                  EV << "Data <failed>" << endl;

                  if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
                      radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);         // 23 Dec

                  /* @brief Checking if next slot is transmission slot */
                  if (transmitSlot[currentSlot + 1] == mySlot)
                      EV << "ACK timeout received re-sending DATA" << endl;
                  else
                  {
                      EV << "Maximum re-transmissions attempt done, DATA transmission <failed>. Deleting packet from que" << endl;
                      EV_DEBUG << " Deleting packet from DMAMAC queue";
					  /* @brief DATA Packet deleted */
                      delete macPktQueue.front();     
                      macPktQueue.pop_front();
                      EV << "My Packet queue size" << macPktQueue.size() << endl;
                  }
                  break;

          /* @brief Handling data timeout (required when in sender is in re-transmission slot and no data to send) */
          case DMAMAC_DATA_TIMEOUT:

                  EV << "No data transmission detected stopping RX" << endl;
                  if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
                  {
                      radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                      EV << "Switching Radio to SLEEP" << endl;
                  }
                  nbTimeouts++;
                  break;

          /* @brief Handling alert timeout required by TDMA version */
          case DMAMAC_ALERT_TIMEOUT:

                 EV << "No alert transmission detected stopping alert RX" << endl;
                 if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
                 {
                     radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                     EV << "Switching Radio to SLEEP" << endl;
                 }
                 break;

         /* @brief Send DATA definition */
          case DMAMAC_SEND_DATA:

                currentMacState = SEND_DATA;
                nbTxSlots++;
                /* @brief Checking if the current slot is also the node's transmit slot  */
                if (mySlot == transmitSlot[currentSlot])
                {
                    /* @brief Checking if packets are available to send */
                    if(macPktQueue.empty())
                    {
                        EV_DEBUG << "No Packet to Send exiting" << endl;
                        findImmediateNextSlot(currentSlot, slotDuration);
                    }
                    else
                    {
                        /* @brief Setting the radio state to transmit mode */
                        radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                        EV_DEBUG << "Waking up in my slot. Radio switch to TX command Sent.\n";
                        findImmediateNextSlot(currentSlot, slotDuration);
                    }
                }
                else
                    EV_DEBUG << "ERROR: Send data message received, but we are not in our slot!!! Repair.\n";

                currentSlot++;
                currentSlot %= numSlots;
                break;

            /* @brief Send ACK definition */
            case DMAMAC_SEND_ACK:

                currentMacState = SEND_ACK;
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                break;

            /* @brief Send NOTIFICATION definition */
            case DMAMAC_SEND_NOTIFICATION:

                EV << "Sending Notification as sink" << endl;
                currentMacState = SEND_NOTIFICATION;

                /* @brief To put in delay of waking up at notification slot equal to other nodes waking up */
                if(radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER || radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);

                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);

                /* @brief if State switch is called for then we ask nodes to make switch here
				 * and makes its own switch here. Should not happen in the first slot of sink 
				 */
                if (changeMacMode && currentMacMode == STEADY && currentSlot != 0)
                {
                    EV << "Initiating switch procedure immediately &aaks 5-May" << endl;
                    /* @brief Scheduling notification message to start on slot 0 */
                    scheduleAt(simTime() + slotDuration,sendNotification);

                    /* @brief For mid switch purpose if sleep was scheduled we remove it since notification is next
                     * Applies only if SteadySuperframe is at least thrice the transient superframe
					 */
                    if (setSleep->isScheduled())
                        cancelEvent(setSleep);           

                    /* €brief For the case of emergency switch resetting currentSlot counter to 0 */
					currentSlot=0;
					
					/* @brief if it is a mid switch through steady we count statistics */
                    if (currentSlot != (numSlotsSteady-1))
                    {
                       EV << " Emergency switch made between steady state completion !Sink" << endl;
                       nbMidSwitch++;
                    }                   
                }
                else
                {
                    /* @brief For regular procedure without changeMacMode or if not in Transient when changeMacMode is required*/
                    findImmediateNextSlot(currentSlot,slotDuration);
                    currentSlot++;
                    currentSlot %= numSlots;
                }
                break;

        default:{
            EV << "WARNING: unknown timer callback " << msg->getKind() << endl;
        }
    }
}

/* @brief
 * Handles and forwards the different control messages
 * we get from the physical layer.
 * This comes from basic MAC tutorial for MiXiM
 */
void DMAMACSink::receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::radioModeChangedSignal) {
            IRadio::RadioMode radioMode = (IRadio::RadioMode)value;
            if (radioMode == IRadio::RADIO_MODE_TRANSMITTER) {
                handleRadioSwitchedToTX();
            }
            else if (radioMode == IRadio::RADIO_MODE_RECEIVER) {
                handleRadioSwitchedToRX();
            }
    }
    else if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            EV_DEBUG << "Packet transmission completed" << endl;
            if(currentMacState == SEND_DATA)
            {
                EV_DEBUG << "Actuator data Packet transmission completed awaiting ACK" << endl;
                scheduleAt(simTime(), waitAck);
            }
            /* @brief Setting radio back to receive to prevent some issues where radio is already in TX*/
            if(currentMacState == SEND_ACK) {
                radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
            }
            /* @brief Setting radio to sleep after notification is sent */
            else if(currentMacState == SEND_NOTIFICATION) {
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
            }
        }
        transmissionState = newRadioTransmissionState;
    }
    else if (signalID == IRadio::receptionStateChangedSignal) {
        IRadio::ReceptionState newRadioReceptionState = (IRadio::ReceptionState)value;
        if (newRadioReceptionState == IRadio::RECEPTION_STATE_RECEIVING) {
            EV << "Message being received" << endl;
            /* @brief cancel timeouts on reception */
            if(dataTimeout->isScheduled())
            {
                cancelEvent(dataTimeout);
                EV << "Receiving Data, hence scheduled data timeout is cancelled" << endl;
            }
            if(alertTimeout->isScheduled())
            {
                cancelEvent(alertTimeout);
                EV << "Receiving alert, hence scheduled alert timeout is cancelled" << endl;
            }
        }
    }
}

/* @brief
 * Called after the radio is switched to TX mode. Encapsulates
 * and sends down the packet to the physical layer.
 * This comes from basic MAC tutorial for MiXiM
 */
void DMAMACSink::handleRadioSwitchedToTX() {
    /* @brief Radio is set to Transmit state thus next job is to send the packet to physical layer  */
    EV << "Radio transmission (TX) handler module " << endl;

    if(currentMacState == SEND_DATA)
       {
            EV << "Sending Data packet down " << endl;
            DMAMACPkt* data = macPktQueue.front()->dup();
            data->setSrcAddr(sinkAddress);
            data->setKind(DMAMAC_ACTUATOR_DATA);
            data->setMySlot(mySlot);
            attachSignal(data);
            EV << "Sending down data packet\n";
            EV << "MAC queue size " << macPktQueue.size() << endl;
            EV << "Packet Destination " << data->getDestAddr() << endl;
            sendDown(data);

            /* @statistics */
            nbTxActuatorData++;
            nbTxData++;
       }
       else if (currentMacState == SEND_ACK)
       {
            EV << "Creating and sending ACK packet down " << endl;
            /* @brief Create ACK packet on reception of DATA packet */
            DMAMACPkt* ack = new DMAMACPkt();
            ack->setDestAddr(lastDataPktSrcAddr);
            ack->setSrcAddr(myMacAddr);
            ack->setKind(DMAMAC_ACK);
            ack->setMySlot(mySlot);
			/* Setting byte length to 11 bytes */
            ack->setByteLength(11);
            attachSignal(ack);
            EV << "Sending ACK packet\n";
            sendDown(ack);

            /* @Statistics */
            nbTxAcks++;
       }
       else if (currentMacState == SEND_NOTIFICATION)
       {
            EV << "Creating and sending Notification packet down " << endl;
            /* @brief Create SINK notification packet */
            DMAMACSinkPkt* notification = new DMAMACSinkPkt();
            timeRef = simTime();
            if (randomGenerator != nullptr) {
                randomGenerator->setRandSeq(simTime().raw());
                int c = randomGenerator->iRandom(11,26);
                notification->setChannel(c);
            }
            lastDataPktSrcAddr = MACAddress::BROADCAST_ADDRESS;
            notification->setDestAddr(lastDataPktSrcAddr);
            notification->setSrcAddr(myMacAddr);
            notification->setKind(DMAMAC_NOTIFICATION);
            notification->setTimeRef(timeRef);
            notification->setByteLength(11);

            attachSignal(notification);
            /* @brief if transient, check for stateProbability otherwise state-switch will be initiated by sensor nodes. */
            if(currentMacMode == TRANSIENT)
            {
                randomNumber = uniform(0,100,0);
                if (randomNumber > stateProbability)
                {
                   EV << "!aaks Time to change to steady mode, number generated :" << randomNumber << " with stateProbability " << stateProbability << endl;
                   changeMacMode = true;
                }
            }
            EV << "ChangeMacMode Value is :" << changeMacMode << endl;
            /* @brief The sensor nodes will be notified either because of stateProbability or because of previous alert */
            notification->setChangeMacMode(changeMacMode);
            sendDown(notification);
            if (notification->getChannel() != -1)
                setChannel(notification->getChannel());

            /* @Statistics */
            nbTxNotifications++;
       }
}

/* @brief
 * Called when the radio switched back to RX.
 * To add procedures to do when radio switched to RX
 */
void DMAMACSink::handleRadioSwitchedToRX() {

    EV << "Radio switched to Receiving state" << endl;

    /* @brief
     * No operations defined currently MAC just waits for packet from
     * physical layer which will be handled by handleLowerMsg function
     */
}

/* @brief
 * Handles received Mac packets from Physical layer. ASserts the packet
 * was received correct and checks if it was meant for us. 
 */
void DMAMACSink::handleLowerPacket(cPacket* msg) {

    if (msg->hasBitError()) {
        EV << "Received " << msg << " contains bit errors or collision, dropping it\n";
        if (currentMacMode == STEADY)
            checkForSuperframeChange = true;
        delete msg;
        return;
    }

    if(currentMacState == WAIT_DATA)
    {
        DMAMACPkt *const mac  = static_cast<DMAMACPkt *>(msg);
        const MACAddress& dest = mac->getDestAddr();

        /* @brief Check if the packet is for me (TDMA So it has to be me in general so just for testing) */
        if(dest == myMacAddr)
        {
            EV << " DATA Packet received with length :" << mac->getByteLength() << ", Sending to Upper Layer" << endl;
            lastDataPktSrcAddr = mac->getSrcAddr();

            /* @brief Not sending to application layer but sending to Actuators */
            if (!mac->getDestinationAddress().isUnspecified() && mac->getDestinationAddress() != myMacAddr) {
                if (isRelayNode)
                    sendUp(mac);
                else
                    throw cRuntimeError("Packet to other destination but this node is not relay");
            }
            else if (sendUppperLayer)
                sendUp(decapsMsg(mac));
            else
                delete mac;

            /* @brief Packet received for myself thus sending an ACK */
            scheduleAt(simTime(), sendAck);
        }
        else
        {
            EV << "DATA Packet not for me, deleting";
            delete mac;
        }
        /* @statistics */
        nbRxData++;
    }
    else if (currentMacState == WAIT_ACK)
    {
        DMAMACPkt *const mac  = static_cast<DMAMACPkt *>(msg);

        EV << "ACK Packet received with length : " << mac->getByteLength() << ", DATA Transmission successful"  << endl;

        EV_DEBUG << " Deleting packet from DMAMAC queue";
        delete macPktQueue.front();     // DATA Packet deleted
        macPktQueue.pop_front();
        delete mac;                     // received ACK Packet is deleted

        /* @brief ACK received thus need to cancel ACK timeout event (can also be cancelled on control message for reception) */
        if(ackTimeout->isScheduled())
            cancelEvent(ackTimeout);

        /* @brief Returns self message that just prints an extra ACK received message preserved for future use */
        scheduleAt(simTime(), ackReceived);

        /* @Statistics */
        nbRxAcks++;
    }
    else if(currentMacState == WAIT_ALERT)
    {
        AlertPkt *const alert  = static_cast<AlertPkt *>(msg);

        EV << " Alert Packet received with length :" << alert->getByteLength() << endl;

        /* @brief Setting changeMacMode to true!! */
        changeMacMode = true;
        EV << "ChangeMacMode Value is :" << changeMacMode << endl;

        /* @Statistics */
        nbRxAlert++;

        /* @brief Deleting alert */
        delete alert;
    }
}

/* @brief
 * Finding the next slot after a sleep slot, when sleep exists for more than one slot.
 */
void DMAMACSink::findDistantNextSlot()
{
    EV << "Current slot before calculating next wakeup :" << currentSlot << endl;
    int i,x; i = 1;
    x = (currentSlot + i) % numSlots;
	/* @brief checking time to next event */
    while(transmitSlot[x] != mySlot && receiveSlot[x] != mySlot && transmitSlot[x] != NOTIFICATION && receiveSlot[x] != ALERT_SINK)
    {
        i++;
        x = (currentSlot + i) % numSlots;
    }

    /* @brief
     * Setting radio to sleep if not already in sleep
     */
    if (radio->getRadioMode() != IRadio::RADIO_MODE_SLEEP)
        radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);

    nextEvent = slotDuration*(i);
    currentSlot +=i;

    /* @Statistics */
    nbSleepSlots +=i;

    /* @brief Printing number of sleep slots */
    EV << "Number of slots :" << numSlots << endl;

    currentSlot %= numSlots;

    EV_DEBUG << "Time for RADIO Sleep for " << nextEvent << " Seconds" << endl;
    EV << "Waking up in slot :" << currentSlot << endl;

    /* @brief
    * Checking if next active slot found is a transmit slot or receive slot. Then scheduling
    * the next event for the time when the node will enter into transmit or receive.
    */
    if(transmitSlot[(currentSlot) % numSlots] == mySlot)
    {
        scheduleAt(simTime() + nextEvent, sendData);
        EV << "My next slot after sleep is transmit slot" << endl;
    }
    else if (transmitSlot[(currentSlot) % numSlots] == NOTIFICATION)
    {
        EV << "My next slot after sleep is notification slot" << endl;
        scheduleAt(simTime() + nextEvent, sendNotification);
    }
    else if (receiveSlot[(currentSlot) % numSlots] == ALERT_SINK)
    {
        EV << "Next slot after sleep is alert slot" << endl;
        scheduleAt(simTime() + nextEvent, waitAlert);
    }
    else if (receiveSlot[(currentSlot) % numSlots] == mySlot)
    {
        if(currentSlot < numSlotsTransient)
        {
            EV << "My next slot after sleep is receive slot" << endl;
            scheduleAt(simTime() + nextEvent, waitData);
        }
        else if (currentSlot >= numSlotsTransient)
        {
            EV << "Next slot after sleep is alert slot" << endl;
            scheduleAt(simTime() + nextEvent, waitAlert);
        }
    }
    else
        EV << " Undefined MAC state <ERROR>" << endl;
}


/*
 * Finding immediate next Slot
 */
void DMAMACSink::findImmediateNextSlot(int currentSlotLocal,simtime_t nextSlot)
{
    EV << "Finding immediate next slot" << endl;

    if (transmitSlot[(currentSlotLocal + 1) % numSlots] == mySlot)
    {
        EV << "Immediate next Slot is my Send Slot, getting ready to transmit.\n";
        scheduleAt(simTime() + nextSlot, sendData);
    }
    else if (transmitSlot[(currentSlotLocal + 1) % numSlots] == NOTIFICATION)
    {
       EV << "Immediate next slot is notification slot" << endl;
       scheduleAt(simTime() + nextSlot, sendNotification);
    }
    else if (receiveSlot[(currentSlotLocal + 1) % numSlots] == mySlot)
    {
        if(currentSlotLocal < numSlotsTransient)
        {
            EV << "Immediate next Slot is my Receive Slot, getting ready to receive.\n";
            scheduleAt(simTime() + nextSlot, waitData);
        }
        else if (currentSlotLocal >= numSlotsTransient)
        {
            EV << "Immediate next Slot is Alert Slot, getting ready to receive.\n";
            scheduleAt(simTime() + nextSlot, waitAlert);
        }

    }
    else if (receiveSlot[(currentSlotLocal + 1) % numSlots] == ALERT_SINK)
    {
        EV << "Immediate next Slot is Alert Slot, getting ready to receive.\n";
        scheduleAt(simTime() + nextSlot, waitAlert);
    }
    else
    {
        EV << "Immediate next Slot is Sleep slot.\n";
        scheduleAt(simTime() + nextSlot, setSleep);
    }
}

/*
 * @brief Sink initializing, to add in addresses of actuators for which the data has to be
 * sent to from the sink
 */
void DMAMACSink::sinkInitialize()
{
    cXMLElement *xmlBuffer1,*xmlBuffer2;

    cXMLElement* rootElement = par("neighborData").xmlValue();

    char id[maxNodes];
    int j;
    cXMLElementList::iterator xmlListIterator1;
    cXMLElementList nListBuffer1;

    for(j=0;j<maxNodes;j++)
        actuatorNodes[j]=-1;

    sprintf(id, "%d", myId);

    xmlBuffer = rootElement->getElementById(id);

    xmlBuffer1 = xmlBuffer->getFirstChildWithTag("actuators");

    nListBuffer1 = xmlBuffer1->getChildren();

    j=0;
    for(xmlListIterator1 = nListBuffer1.begin();xmlListIterator1!=nListBuffer1.end();xmlListIterator1++)
    {
      xmlBuffer2 = (*xmlListIterator1);
      EV << " Actuator ID <aaks> " << xmlBuffer2->getNodeValue() << endl;
      actuatorNodes[j]= atoi(xmlBuffer2->getNodeValue());
      j++;
    }
}


}
