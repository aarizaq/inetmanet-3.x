//**************************************************************************
// * file:        DMAMAC main file
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

#include <cstdlib>
#include "inet/linklayer/dmamac/DMAMAC.h"

#include "../../applications/udpapp/UDPDmaMacRelay.h"
#include "inet/linklayer/dmamac/DMAMACSink.h"
#include "inet/common/INETUtils.h"
#include "inet/common/INETMath.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/FindModule.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"
#include <cinttypes>

namespace inet {

Define_Module(DMAMAC);

/* @brief To set the nodeId for the current node  */
#define myId ((myMacAddr.getInt() & 0xFFFF)-1-baseAddress)
DMAMAC::LocatorTable DMAMAC::globalLocatorTable;
DMAMAC::LocatorTable DMAMAC::sinkToClientAddress; //
DMAMAC::LocatorTable DMAMAC::clientToSinkAddress;
std::map<uint64_t,DMAMAC::nodeStatus> DMAMAC::slotMap;
std::map<uint64_t,int> DMAMAC::slotInfo;

bool DMAMAC::twoLevels = false;

simsignal_t DMAMAC::rcvdPkSignalDma = registerSignal("rcvdPkDma");
simsignal_t DMAMAC::DmaMacChangeChannel = registerSignal("DmaMacChangeChannel");


/* @brief Got this from BaseLayer file, to catch the signal for hostState change */
// const simsignal_t DMAMAC::catHostStateSignal = simsignal_t(MIXIM_SIGNAL_HOSTSTATE_NAME);

/* @brief Initialize the MAC protocol using omnetpp.ini variables and initializing other necessary variables  */

void DMAMAC::discoverIfNodeIsRelay() {
    cModule * mod = gate("upperLayerOut")->getPathEndGate()->getOwnerModule();

    if (mod == nullptr) {
        sendUppperLayer = false;
        return;
    }
    UDPDmaMacRelay *upper  = dynamic_cast<UDPDmaMacRelay *>(mod);

    DMAMACSink * dmacSinkThis = dynamic_cast<DMAMACSink *>(this);
    DMAMAC * dmacNeig = dynamic_cast<DMAMAC *>(mod);
    DMAMACSink * dmacSinkNeigh = dynamic_cast<DMAMACSink *>(mod);

    if (dmacNeig != nullptr) {
        // relay node.
        isRelayNode = true;
    }

    if (dmacSinkThis != nullptr)
        isSink = true;

    if (!isRelayNode) {
        if (upper) {
           isRelayNode = true;
           isUpperRelayNode = true;
        }
        return; // nothing more to-do
    }


    // seach errors.
    if (dmacSinkThis != nullptr && dmacSinkNeigh !=nullptr)
        cRuntimeError("Relay node with two sinks, it must be a sink plus a client");
    if (dmacSinkThis == nullptr && dmacNeig == nullptr)
        cRuntimeError("Relay node with two clients, it must be a sink plus a client");

    // search the address

    if (isSink)
        sinkToClientAddress[myMacAddr] = dmacNeig->myMacAddr;
    else
        clientToSinkAddress[myMacAddr] = dmacNeig->myMacAddr;
    twoLevels = true;
}

void DMAMAC::initializeMACAddress()
{
    const char *addrstr = par("address");

    if (!strcmp(addrstr, "auto")) {
        // assign automatic address
        myMacAddr = MACAddress(MACAddress::generateAutoAddress().getInt() & 0xFFFF);
        // change module parameter from "auto" to concrete address
        par("address").setStringValue(myMacAddr.str().c_str());
    }
    else {
        int add = atoi(addrstr);
        myMacAddr = MACAddress(add);

        if (par("baseAddress").longValue() > 0) {
            baseAddress = par("baseAddress").longValue();
        }
    }
    int add = atoi(par("StartAddressRange"));
    startAddressRange = MACAddress(add);
    add = atoi(par("EndAddressRange"));
    endAddressRange = MACAddress(add);
    if (startAddressRange == endAddressRange) {
        endAddressRange = MACAddress::UNSPECIFIED_ADDRESS;
        startAddressRange = MACAddress::UNSPECIFIED_ADDRESS;
    }
    else if ((startAddressRange.isUnspecified() && !endAddressRange.isUnspecified()) || (!startAddressRange.isUnspecified() && endAddressRange.isUnspecified())) {
        endAddressRange = MACAddress::UNSPECIFIED_ADDRESS;
        startAddressRange = MACAddress::UNSPECIFIED_ADDRESS;
    }
}

InterfaceEntry *DMAMAC::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // data rate
    e->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    e->setMACAddress(myMacAddr);
    e->setInterfaceToken(myMacAddr.formInterfaceIdentifier());

    // capabilities
    e->setMtu(par("mtu").longValue());
    e->setMulticast(false);
    e->setBroadcast(true);

    return e;
}

void DMAMAC::initialize(int stage)
{
    MACProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL)
    {
        EV << "¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤MAC Initialization in process¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤ " << endl;

        /* @brief For Droppedpacket code used */

        /* @brief Signal handling for node death indication as HostState */
        // catHostStateSignal.initialize();

        /* @brief Getting parameters from ini file and setting MAC variables DMA-MAC  */
        /*@{*/
        queueLength         = par("queueLength");
        slotDuration        = par("slotDuration");
        bitrate             = par("bitrate");
        headerLength        = par("headerLength");
        numSlotsTransient   = par("numSlotsTransient");
        numSlotsSteady      = par("numSlotsSteady");
        txPower             = par("txPower");
        ackTimeoutValue     = par("ackTimeout");
        dataTimeoutValue    = par("dataTimeout");
        alertTimeoutValue   = par("alertTimeout");                  
        xmlFileSteady       = par("xmlFileSteadyN");
        xmlFileTransient    = par("xmlFileTransientN");
        stateProbability    = par("stateProbability");
        alertProbability    = par("alertProbability");
        stats               = par("stats");                         // @Statistics recording switch ON/OFF
        alertDelayMax       = par("alertDelayMax");                 // Tune the max delay in sending alert packets
        maxRadioSwitchDelay = par("maxRadioSwitchDelay");
        sinkAddress         = MACAddress( par("sinkAddress").longValue());
        sinkAddressGlobal   = MACAddress( par("sinkAddressGlobal").longValue());
        isActuator          = par("isActuator");                    
        maxNodes            = par("maxNodes");
        maxChildren         = par("maxChildren");
        hasSensorChild      = par("hasSensorChild");                                     
        double temp         = (par("macTypeInput").doubleValue());
        disableChecks = par("disableChecks");
        checkDup = par("checkDup");

        networkId = par("subnetworkId");

        if(temp == 0)
            macType = DMAMAC::HYBRID;
        else
            macType = DMAMAC::TDMA;

        /*@}*/

        /* @brief @Statistics collection */
        /*@{*/
        nbTxData            = 0;
        nbTxDataFailures    = 0;
        nbTxAcks            = 0;
        nbTxAlert           = 0;
        nbTxSlots           = 0;
        nbTxNotifications   = 0;
        nbRxData            = 0;
        nbRxAlert    = 0;
        nbRxNotifications   = 0;
        //nbMissedAcks        = 0;
        nbRxAcks            = 0;
        nbSleepSlots        = 0;
        nbCollisions        = 0;
        nbDroppedDataPackets= 0;
        nbTransient         = 0;
        nbSteady            = 0;
        nbSteadyToTransient = 0;
        nbTransientToSteady = 0;
        nbFailedSwitch      = 0;
        nbMidSwitch         = 0;
        nbSkippedAlert      = 0;
        nbForwardedAlert    = 0;      
        nbDiscardedAlerts   = 0;      
        randomNumber        = 0;
        nbTimeouts          = 0;
        nbAlertRxSlots      = 0;        
        /*@}*/

        /* @brief Other initializations */
        /*@{*/
        nextSlot                = 0;
        txAttempts              = 0;                        // Number of retransmission attempts
        currentMacState         = STARTUP;
        previousMacMode         = STEADY;
        currentMacMode          = TRANSIENT;
        changeMacMode           = false;
        sendAlertMessage        = false;
        alertMessageFromDown    = false;                    // Indicates alert received from children
        checkForSuperframeChange= false;
        forChildNode            = false;
        /*@}*/

        /* @brief How long does it take to send/receive a control packet */
        controlDuration = ((double)headerLength + (double)numSlots + 16) / (double)bitrate;
        EV_DETAIL << "Control packets take : " << controlDuration << " seconds to transmit\n";

        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

        if (par ("useSignalsToChangeChannel").boolValue())
            getSimulation()->getSystemModule()->subscribe(DmaMacChangeChannel, this);



        initializeMACAddress();

        registerInterface();

        /* @brief copies all xml data for slots in steady and transient and neighbor data */
        slotInitialize();


        if (numSlotsTransient != (int)receiveSlotTransient.size())
            numSlotsTransient = (int)receiveSlotTransient.size();
        if (numSlotsSteady   != (int)receiveSlotSteady.size())
            numSlotsSteady   = (int)receiveSlotSteady.size();
        maxNumSlots  = numSlotsSteady;

        WATCH(actualChannel);
        WATCH(currentSlot);
        WATCH(mySlot);
    }
    else if(stage == INITSTAGE_LINK_LAYER) {
        discoverIfNodeIsRelay(); // configure relay information
    }
    else if(stage == INITSTAGE_LINK_LAYER_2) { // configure the rest of the link information

        if (sinkAddress.isUnspecified() && !isSink)
            throw cRuntimeError("Sink address undefined");

        EV << "My Mac address is" << myMacAddr << endl;
        globalLocatorTable[myMacAddr] = sinkAddress;

        EV << " QueueLength  = " << queueLength
        << " slotDuration  = "  << slotDuration
        << " controlDuration= " << controlDuration
        << " numSlots       = " << numSlots
        << " bitrate        = " << bitrate
        << " stateProbability = " << stateProbability
        << " alertProbability = " << alertProbability
        << " macType = "  << macType
        << " Is Actuator = " << isActuator << endl;

        /* @brief initialize the timers with enumerated values  */
        /*@{*/
        startup             = new cMessage("DMAMAC_STARTUP");
        setSleep            = new cMessage("DMAMAC_SLEEP");
        waitData            = new cMessage("DMAMAC_WAIT_DATA");
        waitAck             = new cMessage("DMAMAC_WAIT_ACK");
        waitAlert           = new cMessage("DMAMAC_WAIT_ALERT");
        waitNotification    = new cMessage("DMAMAC_WAIT_NOTIFICATION");
        sendData            = new cMessage("DMAMAC_SEND_DATA");
        sendAck             = new cMessage("DMAMAC_SEND_ACK");
        scheduleAlert       = new cMessage("DMAMAC_SCHEDULE_ALERT");
        sendAlert           = new cMessage("DMAMAC_SEND_ALERT");
        sendNotification    = new cMessage("DMAMAC_SEND_NOTIFICATION");
        ackReceived         = new cMessage("DMAMAC_ACK_RECEIVED");
        ackTimeout          = new cMessage("DMAMAC_ACK_TIMEOUT");
        dataTimeout         = new cMessage("DMAMAC_DATA_TIMEOUT");
        alertTimeout        = new cMessage("DMAMAC_ALERT_TIMEOUT");

        /* @brief msgKind for identification in integer */
        startup->setKind(DMAMAC_STARTUP);
        setSleep->setKind(DMAMAC_SLEEP);
        waitData->setKind(DMAMAC_WAIT_DATA);
        waitAck->setKind(DMAMAC_WAIT_ACK);
        waitAlert->setKind(DMAMAC_WAIT_ALERT);
        waitNotification->setKind(DMAMAC_WAIT_NOTIFICATION);
        sendData->setKind(DMAMAC_SEND_DATA);
        sendAck->setKind(DMAMAC_SEND_ACK);
        scheduleAlert->setKind(DMAMAC_SCHEDULE_ALERT);
        sendAlert->setKind(DMAMAC_SEND_ALERT);
        sendNotification->setKind(DMAMAC_SEND_NOTIFICATION);
        ackReceived->setKind(DMAMAC_ACK_RECEIVED);
        ackTimeout->setKind(DMAMAC_ACK_TIMEOUT);
        dataTimeout->setKind(DMAMAC_DATA_TIMEOUT);
        alertTimeout->setKind(DMAMAC_ALERT_TIMEOUT);

        startup->setSchedulingPriority(-1);
        setSleep->setSchedulingPriority(-1);
        waitData->setSchedulingPriority(-1);
        waitAck->setSchedulingPriority(-1);
        waitAlert->setSchedulingPriority(-1);
        waitNotification->setSchedulingPriority(-1);
        sendData->setSchedulingPriority(-1);
        sendAck->setSchedulingPriority(-1);
        scheduleAlert->setSchedulingPriority(-1);
        sendAlert->setSchedulingPriority(-1);
        sendNotification->setSchedulingPriority(-1);
        ackReceived->setSchedulingPriority(-1);
        ackTimeout->setSchedulingPriority(-1);
        dataTimeout->setSchedulingPriority(-1);
        alertTimeout->setSchedulingPriority(-1);
        /*@}*/

        /* @brief My slot is same as myID assigned used for slot scheduling  */
        mySlot = myId;

        sendUppperLayer = par("sendUppperLayer"); // if false the module deletes the packet, other case, it sends the packet to the upper layer.
        procUppperLayer = par("procUppperLayer");

        if (twoLevels) {
            reserveChannel = par("reserveChannel");
            if (reserveChannel != -1) {
                if (isSink) {
                    if (myMacAddr == sinkAddressGlobal) {
                        setChannel(reserveChannel);
                    }
                    else
                        initializeRandomSeq();
                }
                else {
                    if (sinkAddress == sinkAddressGlobal)
                        setChannel(reserveChannel);
                    else
                        initializeRandomSeq();
                }
            }
        }
        else {
            reserveChannel = -1;
            initializeRandomSeq();
        }

        frequentHopping = par("frequentHopping");

        if (frequentHopping && randomGenerator ) {
            if (isSink || !par ("useSignalsToChangeChannel").boolValue()) {
                hoppingTimer = new cMessage("DMAMAC_HOPPING_TIMEOUT");
                hoppingTimer->setSchedulingPriority(-2);
                hoppingTimer->setKind(DMAMAC_HOPPING_TIMEOUT);
                scheduleAt(par("initTime").doubleValue(), hoppingTimer);
            }
            if (channelSinc)
                scheduleAt(par("initTime").doubleValue(), startup);
        }
        else {
            /* @brief Schedule a self-message to start superFrame  */
            scheduleAt(par("initTime").doubleValue(), startup);
        }

        nodeStatus status;
        status.currentMacMode = currentMacMode;
        status.currentMacState = currentMacState;
        status.previousMacMode = previousMacMode;
        status.currentSlot = currentSlot;

        slotMap[myMacAddr.getInt()] = status;
        slotInfo[myMacAddr.getInt()] = mySlot;


        if (radio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER && !channelSinc)
        {
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
            EV << "Switching Radio to RX mode" << endl;
        }
    }
}


/* @brief Module destructor */

DMAMAC::~DMAMAC() {

    /* @brief Clearing Self Messages */
    /*@{*/
    cancelAndDelete(startup);
    cancelAndDelete(setSleep);
    cancelAndDelete(waitData);
    cancelAndDelete(waitAck);
    cancelAndDelete(waitAlert);
    cancelAndDelete(waitNotification);
    cancelAndDelete(sendData);
    cancelAndDelete(sendAck);
    cancelAndDelete(scheduleAlert);
    cancelAndDelete(sendAlert);
    cancelAndDelete(sendNotification);
    cancelAndDelete(ackReceived);
    cancelAndDelete(ackTimeout);
    cancelAndDelete(dataTimeout);
    cancelAndDelete(alertTimeout); 
    if (hoppingTimer)
        cancelAndDelete(hoppingTimer);
    /*@}*/

    while (!macPktQueue.empty()) {
        delete macPktQueue.back().pkt;
        macPktQueue.pop_back();
    }
    while (!alertPktQueue.empty()) {
        delete alertPktQueue.back();
        alertPktQueue.pop_back();
    }
}

/* @brief Recording statistical data fro analysis */
void DMAMAC::finish() {

    /* @brief Record statistics for plotting before finish */
    if (stats)
    {
        recordScalar("nbCreatePkt", nbCreatePkt);
        recordScalar("nbTxData", nbTxData);
        recordScalar("nbTxActuatorData", nbTxActuatorData);
        recordScalar("nbTxDataFailures", nbTxDataFailures);
        recordScalar("nbTxAcks", nbTxAcks);
        recordScalar("nbTxAlert", nbTxAlert);
        recordScalar("nbTxSlots", nbTxSlots);
        recordScalar("nbTxNotifications", nbTxNotifications);
        recordScalar("nbRxData", nbRxData);
        recordScalar("nbRxActuatorData", nbRxActuatorData);
        recordScalar("nbRxAcks", nbRxAcks);
        recordScalar("nbRxAlert", nbRxAlert);
        recordScalar("nbRxNotifications", nbRxNotifications);
        //recordScalar("nbMissedAcks", nbMissedAcks);
        recordScalar("nbSleepSlots", nbSleepSlots);
        recordScalar("nbCollisions",nbCollisions);
        recordScalar("nbDroppedDataPackets", nbDroppedDataPackets);
        recordScalar("nbTransient", nbTransient);
        recordScalar("nbSteady", nbSteady);
        recordScalar("nbSteadyToTransient", nbSteadyToTransient);
        recordScalar("nbTransientToSteady", nbTransientToSteady);
        recordScalar("nbFailedSwitch", nbFailedSwitch);
        recordScalar("nbMidSwitch", nbMidSwitch);
        recordScalar("nbSkippedAlert", nbSkippedAlert);
        recordScalar("nbForwardedAlert", nbForwardedAlert);
        recordScalar("nbDiscardedAlerts", nbDiscardedAlerts);
        recordScalar("nbTimeouts", nbTimeouts); 
        recordScalar("nbAlertRxSlots", nbAlertRxSlots); 
        recordScalar("nbRxDataErroneous", nbRxDataErroneous);
        recordScalar("nbRxNotificationErroneous", nbRxNotificationErroneous);

    }
}

/* @brief
 * Handles packets from the upper layer and starts the process
 * to send them down.
 */
void DMAMAC::handleUpperPacket(cPacket* msg){

    if (shutDown) {
        delete msg;
        return;
    }

    EV_DEBUG << "Packet from Network layer" << endl;
    if (!procUppperLayer) {
        delete msg;
        return;
    }
    if (dynamic_cast<AlertPkt *> (msg)) {
        if (isRelayNode && !isUpperRelayNode)
            alertPktQueue.push_back(dynamic_cast<AlertPkt *> (msg));
        else
            delete msg;
        return;
    }


    DMAMACPkt *mac = nullptr;
    DMAMACPkt *macUpper = dynamic_cast<DMAMACPkt *>(msg);
    if (macUpper == nullptr) {
        /* Creating Mac packets here itself to have periodic style in our method
         *              * Since application packet generated period had certain issues
         *                           * Folliwing code segement can be used for application layer generated packets
         *                                        * @brief Casting upper layer message to mac packet format
         *                                                     * */
        mac = static_cast<DMAMACPkt *>(encapsMsg(static_cast<cPacket*>(msg)));
        // @brief Sensor data goes to sink and is of type DMAMAC_DATA, setting it here 20 May
        if (!isSink) {
            mac->setDestAddr(sinkAddress);
            if (twoLevels) {
                mac->setDestinationAddress(sinkAddressGlobal);
            }
        }
        else {
            if (twoLevels) {
                destAddr = mac->getDestAddr();
                mac->setDestinationAddress(destAddr);
                // search for the appropriate sink
                auto it = globalLocatorTable.find(destAddr);
                if (it == globalLocatorTable.end())
                    throw cRuntimeError("Node not found in the list globalLocatorTable");
                if (it->second != myMacAddr) { //
                    // search for the address of the relay
                    auto itAux = sinkToClientAddress.find(destAddr);
                    if (itAux == sinkToClientAddress.end())
                        throw cRuntimeError("Node not found in the list of sinkToClientAddress");
                    // check that this client address sink is this node
                    auto itAux2 = globalLocatorTable.find(itAux->second);
                    if (itAux2 == globalLocatorTable.end())
                        throw cRuntimeError("Node not found in the list of globalLocatorTable");
                    mac->setDestAddr(itAux->second);
                }
            }
        }
    }
    else {
        // the node must be relay to accept this
        if (!isRelayNode)
            throw cRuntimeError("Packet type received from upper layer not allowed");
        // actualize the address and enque
        mac = macUpper;
        mac->setDestAddr(mac->getDestinationAddress());
        mac->setSrcAddr(myMacAddr);
        mac->setNetworkId(networkId);
        if (checkDup) {
            mac->setBitLength(headerLength+8);
        }
        else
            mac->setBitLength(headerLength);
    }

    if (mac->getKind() != DMAMAC_ACTUATOR_DATA)
        mac->setKind(DMAMAC_ACTUATOR_DATA);

    mac->setMySlot(mySlot);
    mac->setSourceAddress(myMacAddr);
    sequence++;
    mac->setSequence(sequence);

    // @brief Check if packet queue is full s
    if (macPktQueue.size() < queueLength) {
        packetSend aux;
        aux.pkt = mac;
        macPktQueue.push_back(aux);
        EV_DEBUG << " Data packet put in MAC queue with queueSize: " << macPktQueue.size() << endl;
    }
    else {
        // @brief Queue is full, message has to be deleted DMA-MAC
        EV_DEBUG << "New packet arrived, but queue is FULL, so new packet is deleted\n";
        // @brief Network layer unable to handle MAC_ERROR for now so just deleting
        delete mac;
    }
    refreshDisplay();
}

/*  @brief Handles the messages sent to self (mainly slotting messages)  */
void DMAMAC::handleSelfMessage(cMessage* msg)
{
    if (shutDown) {
        delete msg;
        return;
    }

    if (!channelSinc && frequentHopping) {
        if (hoppingTimer && msg == hoppingTimer) {
            currentSlot++;
            currentSlot %= numSlots;
            if (currentSlot == 0) {
                // change to the next channel
                int channel = actualChannel+1;
                if (channel > 26)
                    channel = 11;
                setChannel(channel);
            }
            if (timeRef+slotDuration < simTime())
                timeRef = simTime();
            scheduleAt(timeRef+slotDuration, hoppingTimer);
            timeRef += slotDuration;
        }
    }
    else if (hoppingTimer && msg == hoppingTimer) {
        scheduleAt(timeRef+slotDuration, hoppingTimer);
        // change channel
        setRandSeq(timeRef.raw() + initialSeed);
        setNextSequenceChannel();
        timeRef += slotDuration;
        refreshDisplay();
        return;
    }
    else if (hoppingTimer && msg == hoppingTimer && !isSincronized) {
        throw cRuntimeError("It is necessary to implement search sink");
    }


    EV << "Self-Message Arrived with type : " << msg->getKind() << "Current mode of operation is :" << currentMacMode << endl;

    /* @brief To check if collision(alert packet dropped) has resulted in a switch failure */
    if(checkForSuperframeChange && !changeMacMode && currentSlot == 0)
    {
        EV << "Switch failed due to collision" << endl;
        nbFailedSwitch++;
        checkForSuperframeChange = false;
    }
    /* @brief To set checkForSuperframeChange as false when superframe actually changes */
    if(checkForSuperframeChange && changeMacMode && currentSlot == 0)
        checkForSuperframeChange = false;

    /* @brief To change superframe when in slot 0 or at multiples of numSlotsTransient 
	 * between transient parts, if required for emergency transient switch.
	 */
    if (currentSlot == 0 || (currentSlot % receiveSlotTransient.size()) == 0)
    {
        /* @brief If we need to change to new superframe operational mode */
        if (changeMacMode)
        {
           EV << " MAC to change operational mode : " << currentMacMode << " to : " << previousMacMode << endl;
           if (currentMacMode == TRANSIENT)
           {
               previousMacMode = TRANSIENT;
               currentMacMode = STEADY;
           }
           else
           {
              previousMacMode = STEADY;
              currentMacMode = TRANSIENT;
           }
           EV << "The currentSlot is on switch is " << currentSlot << endl;
           changeSuperFrame(currentMacMode);
        }
    }

	/* @brief Collecting statistcs and creating packets after every superframe */
    if (currentSlot == 0)
    {
       EV << "¤¤¤¤¤¤¤¤New Superframe is starting¤¤¤¤¤¤¤¤" << endl;
       //setNextSequenceChannel();
       macPeriod = DATA;
       /* @Statistics */
       if (currentMacMode == TRANSIENT)
           nbTransient++;
       else
           nbSteady++;

       /* @brief Creating the data packet for this round */
       if(!isActuator && macPktQueue.empty())
       {
              DMAMACPkt* mac = new DMAMACPkt();
              destAddr = MACAddress(sinkAddress);
              mac->setDestAddr(destAddr);
              mac->setKind(DMAMAC_DATA);
              mac->setByteLength(45);
              if (checkDup)
                  mac->setByteLength(mac->getByteLength()+1);
              mac->setSourceAddress(myMacAddr);
              nbCreatePkt++;
              sequence++;
              mac->setSequence(sequence);
              packetSend aux;
              aux.pkt = mac;
              macPktQueue.push_back(aux);

              //mac->setBitLength(headerLength);
         }
    }

    /* @brief To mark Alert period for mark */
/*    if (currentSlot >= numSlotsTransient)
        macPeriod = ALERT;*/

    EV << "Current <MAC> period = " << macPeriod << endl;
    /* @brief Printing number of slots for check  */
    EV << "nbSlots = " << numSlots << ", currentSlot = " << currentSlot << ", mySlot = " << mySlot << endl;
    //EV << "In this slot transmitting node is : " << transmitSlot[currentSlot] << endl;
    EV << "Current RadioState : " << radio->getRadioModeName(radio->getRadioMode())  << endl;

    bool recIsIdle = (radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE);
    bool generateAlert = false;
    switch (msg->getKind())
    {
        /* @brief SETUP phase enters to start the MAC protocol operation */
        case DMAMAC_STARTUP:

                currentMacState = STARTUP;
                currentSlot = 0;
                /* @brief Starting with notification (TRANSIENT state) */
                scheduleAt(simTime(), waitNotification);
                break;

        /* @brief Sleep state definition  */
        case DMAMAC_SLEEP:

                currentMacState = MAC_SLEEP;
                /* Finds the next slot after getting up */
                findDistantNextSlot();
                break;

        /* @brief Waiting for data packet state definition */
        case DMAMAC_WAIT_DATA:

                currentMacState = WAIT_DATA;
                EV << "My data receive slot " << endl;

				/* @brief Data tiemout is scheduled */
                EV << "Scheduling timeout with value : " << dataTimeoutValue << endl;   
                scheduleAt(simTime() + dataTimeoutValue, dataTimeout);                  

                /* @brief Checking if the radio is already in receive mode */
                if (radio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER)
                {
                    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                    EV << "Switching Radio to RX" << endl;
                }
                else
                    EV << "Radio already in RX" << endl;

				/* @brief Find next slot and increment current slot counter after sending self message */
                findImmediateNextSlot(currentSlot, slotDuration);
                currentSlot++;
                currentSlot %= numSlots;
                break;

        /* @brief Waiting for ACK packet state definition */
        case DMAMAC_WAIT_ACK:

                currentMacState = WAIT_ACK;
                EV << "Switching radio to RX, waiting for ACK" << endl;
                radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);

                /* @brief Wait only until ACK timeout */
                scheduleAt(simTime() + ackTimeoutValue, ackTimeout);
                break;

        /* @brief Waiting for ALERT packet state definition */
        case DMAMAC_WAIT_ALERT:

                currentMacState = WAIT_ALERT;
                EV << "Switching radio to RX, waiting for Alert" << endl;

                /* @brief If no sensor children no wait alert.*/
                if (!alertPktQueue.empty()) {
                    // propagate alert
                    handleLowerPacket(alertPktQueue.front());
                    alertPktQueue.pop_front();

                }
                else if(!hasSensorChild)
                {
                    EV << " No children are <Sensors> so no waiting" << endl;
                }
                else
                {
                    nbAlertRxSlots++;

					/* @brief The only place where DMAMAC types Hybrid and TDMA are different is in the wait alert and send alert */
                    if(macType == DMAMAC::TDMA) // 1 = TDMA, 0 = CSMA.
                    {
                        EV << "Scheduling alert timeout with value : " << alertTimeoutValue << endl; 
                        scheduleAt(simTime() + alertTimeoutValue, alertTimeout);
                    }

                    /* @brief Checking if the radio is already in receive mode, preventing RX->RX */
                    if (radio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER)
                    {
                        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                        EV << "Switching Radio to RX mode" << endl;
                    }
                    else
                        EV << "Radio already in RX mode" << endl;
                }

                findImmediateNextSlot(currentSlot, slotDuration);
                currentSlot++;
                currentSlot %= numSlots;
                break;

        /* @brief Waiting for ACK packet state definition */
        case DMAMAC_WAIT_NOTIFICATION:

                currentMacState = WAIT_NOTIFICATION;
                EV << "Switching radio to RX, waiting for Notification packet from sink" << endl;

                /* @brief Checking if the radio is already in receive mode, preventing RX->to RX */
                if (radio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER)
                {
                    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                    EV << "Switching Radio to RX mode" << endl;
                }
                else
                    EV << "Radio already in RX mode" << endl;


                findImmediateNextSlot(currentSlot, slotDuration);
                currentSlot++;
                currentSlot %= numSlots;
                break;

        /* @brief Handling receiving of ACK, more of a representative message */
        case DMAMAC_ACK_RECEIVED:

                EV << "ACK Received, setting radio to sleep" << endl;

                if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
                    if (!alwaysListening)
                        radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                break;

        /* @brief Handling ACK timeout possibility (ACK packet lost)  */
        case DMAMAC_ACK_TIMEOUT:

                /* @brief Calculating number of transmission failures  */
                nbTxDataFailures++;
                EV << "Data <failed>" << endl;

                if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
                    if (!alwaysListening) {
                        radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                    }

                /* @brief Checking if we have re-transmission slots left */
                if (transmitSlot[currentSlot + 1] == mySlot)
                    EV << "ACK timeout received re-sending DATA" << endl;
                else
                {
                    EV_INFO << "Maximum re-transmissions attempt done, DATA transmission <failed>. Deleting packet from que" << endl;
                    EV_DEBUG << " Deleting packet from DMAMAC queue";
                    delete macPktQueue.front().pkt;     // DATA Packet deleted in case re-transmissions are done
                    macPktQueue.pop_front();
                    EV_INFO << "My Packet queue size" << macPktQueue.size() << endl;
                }
                break;

        /* @brief Handling data timeout (required when in re-transmission slot nothing is sent) */
        case DMAMAC_DATA_TIMEOUT:

                EV << "No data transmission detected stopping RX" << endl;
                if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
                {
                    if (!alwaysListening) {
                        radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                        EV << "Switching Radio to SLEEP" << endl;
                    }
                }
                nbTimeouts++;
                break;

        /* @brief Handling alert timeout (required when in alert slot nothing is being received) */
        case DMAMAC_ALERT_TIMEOUT:

               EV << "No alert transmission detected stopping alert RX, macType : " << macType << endl;
               if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
               {
                   if (!alwaysListening) {
                       radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                       EV << "Switching Radio to SLEEP" << endl;
                   }
               }
               break;

        /* @brief Sending data definition */
        case DMAMAC_SEND_DATA:

                currentMacState = SEND_DATA;
                nbTxSlots++;

                /* @brief Checking if the current slot is also the node's transmit slot  */
                assert(mySlot == transmitSlot[currentSlot]);

                /* @brief Checking if packets are available to send in the mac queue  */
                randomNumber = uniform(0,1000,0);
                EV << "Generated randomNumber is : " << randomNumber << endl;

                /* @brief Checking if we need to send Alert message at all */
                /*@{*/

                if (randomNumber < alertProbability)
                {
                   EV << "Threshold crossed need to send Alert message, number generated :" << randomNumber << endl;
                   generateAlert = true;
                }

                if (generateAlert && macPktQueue.empty()) {
                    // generate message of data for transmit the alert.
                    /* @brief Creating the data packet for this round */
                    if(!isActuator && macPktQueue.empty()) {
                        DMAMACPkt* mac = new DMAMACPkt();
                        destAddr = MACAddress(sinkAddress);
                        mac->setDestAddr(destAddr);
                        mac->setKind(DMAMAC_DATA);
                        mac->setByteLength(45);
                        if (checkDup)
                            mac->setByteLength(mac->getByteLength()+1);
                        mac->setSourceAddress(myMacAddr);
                        nbCreatePkt++;
                        sequence++;
                        mac->setSequence(sequence);
                        packetSend aux;
                        aux.pkt = mac;
                        macPktQueue.push_back(aux);
                        //mac->setBitLength(headerLength);
                    }
                }

                if(macPktQueue.empty()) {
                    EV << "No Packet to Send exiting" << endl;
                }
                else
                {
                    if (generateAlert) {
                        nbTxAlert++;
                        // search a message in the queue and activate the alert.
                        DMAMACPkt *pktAux = macPktQueue.front().pkt;
                        if (pktAux->getSourceAddress() == myMacAddr) {
                            pktAux->setAlarms(pktAux->getAlarms()+1);
                        }
                        else {
                            bool find = false;
                            for (int i = 0; i < pktAux->getAlarmsArrayArraySize(); i++) {
                                Alarms alr = pktAux->getAlarmsArray(i);
                                if (alr.getAddress() == myMacAddr)  {
                                    find = true;
                                    alr.setAlarms(alr.getAlarms()+1);
                                    pktAux->setAlarmsArray(i,alr);
                                }
                            }
                            if (!find) {
                                Alarms alr;
                                alr.setAddress(myMacAddr);
                                pktAux->setAlarmsArrayArraySize(pktAux->getAlarmsArrayArraySize()+1);
                                alr.setAlarms(1);
                                pktAux->setAlarmsArray(pktAux->getAlarmsArrayArraySize()-1,alr);
                                pktAux->setByteLength(pktAux->getByteLength()+2);
                            }
                        }
                    }
                    /* @brief Setting the radio state to transmit mode */
                    if(radio->getRadioMode() != IRadio::RADIO_MODE_TRANSMITTER)
                        radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                    EV_DEBUG << "Radio switch to TX command Sent.\n";
                }

                findImmediateNextSlot(currentSlot, slotDuration);
                currentSlot++;
                currentSlot %= numSlots;
                break;

        /* @brief Sending ACK packets when DATA is received */
        case DMAMAC_SEND_ACK:

                currentMacState = SEND_ACK;
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                break;

        /* @brief Scheduling the alert with a random Delay for Hybrid and without delay fro TDMA */
        case DMAMAC_SCHEDULE_ALERT:

                currentMacState = SCHEDULE_ALERT;
                /* @brief Radio set to sleep to prevent missing messages due to time required to switch 
				 */
                if(macType == DMAMAC::HYBRID || macType == DMAMAC::TDMA) // 0 = Hybrid
                {
                    if((radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER) || (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER))
                        if (!alwaysListening) {
                            radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                        }
                }

                randomNumber = uniform(0,1000,0);   
                EV << "Generated randomNumber is : " << randomNumber << endl;                

                /* @brief Checking if we need to send Alert message at all */
                /*@{*/
                if (alertMessageFromDown)
                {
                   EV << "Forwarding Alert packets from children " << endl;
                   sendAlertMessage = true;
                }
                /* @brief if we have alert message generated randomly */
                else if (randomNumber < alertProbability) 
                {
                   EV << "Threshold crossed need to send Alert message, number generated :" << randomNumber << endl;
                   sendAlertMessage = true;
                }
                /*@}*/

                /* @brief if we have alert message to be sent it is scheduled here with random Delay
 				 * random delay is only used for Hybrid version, for TDMA it it sent without delay
				 */
                if(macType == DMAMAC::HYBRID) // 0 = Hybrid
                {
                    if(sendAlertMessage)
                    {
                        /* @brief Scheduling the first Alert message */
                        EV << "Alert Message being scheduled !alert " << endl;
                        double alertDelay = ((double)(intrand(alertDelayMax)))/ 10000;
                        EV_DEBUG << "Delay generated !alert, alertDelay: " << alertDelay << endl;
                        
                        scheduleAt(simTime() + alertDelay, sendAlert);											
                        sendAlertMessage = false;
                    }
                    else
                    {
                        /* @brief If we have no alert to send check if radio is in RX, if yes set to SLEEP */
                        EV << "No alert to send sleeping <NoAlert>" << endl;
                        if (!alwaysListening) {
                            if(radio->getRadioMode() != IRadio::RADIO_MODE_SLEEP)
                                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                        }
                        else {
                            if(radio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER)
                                radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                        }
                    }
                }
                else if (macType == DMAMAC::TDMA) // 1 = TDMA
                {
                    if(sendAlertMessage)
                    {
                        scheduleAt(simTime(), sendAlert);
                        sendAlertMessage = false;
                    }
                    else
                    {
                        EV << "No alert to send sleeping <NoAlert>" << endl;
                        if (!alwaysListening) {
                            if(radio->getRadioMode() != IRadio::RADIO_MODE_SLEEP)
                                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                        }
                        else {
                            if(radio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER)
                                radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                        }
                    }
                }

                /* @brief Changed to in any case find next slot */
                findImmediateNextSlot(currentSlot, slotDuration);
                currentSlot++;
                currentSlot %= numSlots;
                break;

        /* @brief Sending alert packets after set delay */
        case DMAMAC_SEND_ALERT:
                currentMacState = SEND_ALERT;
                EV << "Sending Alert message " << endl;
                if (macType == DMAMAC::HYBRID) // 0 = Hybrid
                {
                    /*@{ Hybrid part*/
                    /* @brief Send only if channel is idle otherwise ignore since there is alert message being sent already */
                    if(recIsIdle)
                    {
                        radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                    }
                    else
                    {
                       EV << "Channel busy indicating alert message is being sent: thus we skip " << endl;
                       nbSkippedAlert++;
                    }
                    /*@}*/
                }
                else if (macType == DMAMAC::TDMA)  // 1 = TDMA
                {
                    /* Start sending alert */
                    if (radio->getRadioMode() != IRadio::RADIO_MODE_TRANSMITTER)
                        radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                }
                break;

        default:{
            EV << "WARNING: unknown timer callback at Self-Message" << msg->getKind() << endl;
        }
    }
    refreshDisplay();
}

/* @brief
 * Handles and forwards different control messages we get from the physical layer.
 * This comes from basic MAC tutorial for MiXiM
 */
void DMAMAC::receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details)
{

    Enter_Method_Silent();
    if (shutDown) {
        return;
    }

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
            /* @brief Should not wait for ACK for the control packet sent by node 0 */
            if(currentMacState == SEND_DATA)
            {
                EV_DEBUG << "Packet transmission completed awaiting ACK" << endl;
                scheduleAt(simTime(), waitAck);
            }
            /* @brief Setting radio to sleep to save energy after ACK is sent */
            if(currentMacState == SEND_ACK && radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER) {

                if (!alwaysListening) {
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                 }
                 else {
                     radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                 }
            }

            /* @brief Setting radio to sleep to save energy after Alert is sent */
            if(currentMacState == SEND_ALERT && radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER) {
                if (!alwaysListening) {
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                 }
                 else {
                     radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                 }
            }
        }
        transmissionState = newRadioTransmissionState;
    }
    else if (signalID == IRadio::receptionStateChangedSignal) {
        IRadio::ReceptionState newRadioReceptionState = (IRadio::ReceptionState)value;
        if (newRadioReceptionState == IRadio::RECEPTION_STATE_RECEIVING) {
            EV_DEBUG << "Message being received" << endl;
            /* @brief Data received thus need to cancel data timeout event */
            if(dataTimeout->isScheduled() && currentMacState == WAIT_DATA)
            {
                cancelEvent(dataTimeout);
                EV << "Receiving Data, hence scheduled data timeout is cancelled" << endl;
            }
            /* @brief ACK received thus need to cancel ACK timeout event */
            else if(ackTimeout->isScheduled() && currentMacState == WAIT_ACK)
            {
                cancelEvent(ackTimeout);
                EV << "Receiving ACK, hence scheduled ACK timeout is cancelled" << endl;
            }
            /* @brief Alert received thus need to cancel Alert timeout event */
            else if(alertTimeout->isScheduled() && currentMacState == WAIT_ALERT)
            {
                cancelEvent(alertTimeout);
                EV << "Receiving alert, hence scheduled alert timeout is cancelled" << endl;
            }
        }
    }
    else if (signalID == DmaMacChangeChannel){
        if (details == nullptr)
            return;
        DetailsChangeChannel *de = dynamic_cast<DetailsChangeChannel *> (details);
        if (de == nullptr)
            return;
        if (networkId != de->networkId || sinkAddress != de->sinkId)
            return;
        setChannel(value);
    }
}

/* @brief
 * Called after the radio is switched to TX mode. Encapsulates
 * and sends down the packet to the physical layer.
 * This comes from basic MAC tutorial for MiXiM
 */
void DMAMAC::handleRadioSwitchedToTX() {
    /* @brief Radio is set to Transmit state thus next job is to send the packet to physical layer  */
    EV_INFO << " TX handler module ";

        if(currentMacState == SEND_DATA)
        {
            if (par("sendDisorganized") && !alwaysListeningReception) {
                // check reception slot
                int rslot = receiveSlot[currentSlot];
                for (auto it = macPktQueue.begin(); it != macPktQueue.end(); ++it) {
                    // it is cheat, but it is possible to know the slot of every node beacuse is ofline configuration
                    auto itAux = slotInfo.find((*it).pkt->getDestAddr().getInt());
                    if (itAux->second == rslot) {
                        if (it != macPktQueue.begin()) {
                            // move to the front
                            packetSend aux = (*it);
                            it = macPktQueue.erase(it);
                            macPktQueue.push_front(aux);
                            break;
                        }
                    }
                }
            }

            EV << "Sending Data packet down " << endl;
            if (macPktQueue.front().ret == 0) {
                seqMod256++;
                macPktQueue.front().pkt->setSeq(seqMod256);
            }
            DMAMACPkt* data = macPktQueue.front().pkt->dup();

            /* @brief Other fields are set, setting slot field 
			 * Both for self data and forward data
			 */
            data->setSrcAddr(myMacAddr);
            data->setNetworkId(networkId);
            data->setMySlot(mySlot);
            attachSignal(data);
            EV_INFO << "Sending down data packet\n";
            macPktQueue.front().ret++;
            if (channelSinc && frequentHopping) {
                data->setTimeRef(simTime());
                data->setInitialSeed(initialSeed);
            }
            sendDown(data);

            /* @statistics */
            nbTxData++;
        }
        else if (currentMacState == SEND_ACK)
        {
            EV_INFO << "Creating and sending ACK packet down " << endl;
            /* @brief ACK packet has to be created */
            DMAMACPkt* ack = new DMAMACPkt();
            ack->setDestAddr(lastDataPktSrcAddr);
            ack->setSrcAddr(myMacAddr);
            ack->setNetworkId(networkId);
            ack->setKind(DMAMAC_ACK);
            ack->setMySlot(mySlot);
            ack->setByteLength(11);
            attachSignal(ack);
            EV_INFO << "Sending ACK packet\n";
            if (channelSinc && frequentHopping) {
                ack->setTimeRef(simTime());
                ack->setInitialSeed(initialSeed);
            }

            sendDown(ack);

            /* @Statistics */
            nbTxAcks++;
            EV_INFO << "#TxAcks : " << nbTxAcks << endl;
        }
        else if (currentMacState == SEND_ALERT)
        {
		   /* @brief Forwarding alert packets received from children */
           if(alertMessageFromDown)
            {
               EV_INFO << "Forwarding Alert from Down " << endl;
                AlertPkt* alert = new AlertPkt();
                destAddr = MACAddress(parent);
                alert->setDestAddr(destAddr);
                alert->setSrcAddr(myMacAddr);
                alert->setNetworkId(networkId);
                alert->setKind(DMAMAC_ALERT);
                alert->setByteLength(11);

                attachSignal(alert);
                EV_INFO << "Forwarding Alert packet\n";
                if (channelSinc && frequentHopping) {
                    alert->setTimeRef(simTime());
                    alert->setInitialSeed(initialSeed);
                }
                sendDown(alert);

                /* @Statistics */
                nbForwardedAlert++;
            }
			/* @brief Sending its own alert packet */
            else
            {
                EV_INFO << "Creating and sending Alert packet down " << endl;
                /* @brief Alert packet has to be created */
                AlertPkt* alert = new AlertPkt();
                destAddr = MACAddress(parent);
                alert->setDestAddr(destAddr);
                alert->setSrcAddr(myMacAddr);
                alert->setNetworkId(networkId);
                alert->setKind(DMAMAC_ALERT);

                if (twoLevels){
                    alert->setSourceAddress(myMacAddr);
                    alert->setDestinationAddress(sinkAddressGlobal);
                }
                alert->setByteLength(11);
                attachSignal(alert);
                EV_INFO << "Sending #new Alert packet\n";
                if (channelSinc && frequentHopping) {
                    alert->setTimeRef(simTime());
                    alert->setInitialSeed(initialSeed);
                }
                sendDown(alert);

                /* @Statistics */
                nbTxAlert++;
            }
        }
}

/* @brief
 * Encapsulates the packet from the upper layer and
 * creates and attaches signal to it.
 */
MACFrameBase* DMAMAC::encapsMsg(cPacket* msg) {

    DMAMACPkt *pkt = new DMAMACPkt(msg->getName(), msg->getKind());
    pkt->setBitLength(headerLength);

    /* @brief Copy dest address from the Control Info (if attached to the network message by the network layer) */
    IMACProtocolControlInfo *const cInfo = check_and_cast<IMACProtocolControlInfo *>(msg->removeControlInfo());

    EV_DEBUG << "CInfo removed, mac addr=" << cInfo->getDestinationAddress() << endl;
    pkt->setDestAddr(cInfo->getDestinationAddress());

    /* @brief Delete the control info */
    delete cInfo;

    /* @brief Set the src address to own mac address (nic module getId()) */
    pkt->setSrcAddr(myMacAddr);
    pkt->setNetworkId(networkId);
    pkt->setSourceAddress(myMacAddr);
    sequence++;
    pkt->setSequence(sequence);
    if (checkDup)
        pkt->setByteLength(pkt->getByteLength()+1);

    /* @brief Encapsulate the MAC packet */
    pkt->encapsulate(check_and_cast<cPacket *>(msg));
    EV_DEBUG <<"pkt encapsulated\n";

    return pkt;
}

/* @brief
 * Called when the radio switched back to RX.
 * Sets the MAC state to RX
 * This comes from basic MAC tutorial for MiXiM
 */
void DMAMAC::handleRadioSwitchedToRX() {

    EV << "Radio switched to Receiving state" << endl;

    /* @brief
     * No operations defined currently MAC just waits for packet from
     * physical layer which will be handled by handleLowerMsg function
     */
}

/* @brief
 * Handles received Mac packets from Physical layer. Asserts if the packet
 * was received correct and checks if it was meant for us. 
 */
void DMAMAC::resyncr(const int &slot,const macMode &mode, const bool &changeMacMode)
{

    if (!disableChecks) return;

    bool resync = false;
    if (currentMacMode != mode && !changeMacMode)
    {
       // if (testing)
       //     throw cRuntimeError("Synchronization problem, Mac Mode are different");

        currentMacMode = mode;
        if (currentMacMode == TRANSIENT) {
            numSlots = numSlotsTransient;
            transmitSlot = transmitSlotTransient;         // 150 normal slots with configuration 1x, 2x, 3x and 4x we have upto 600 slots
            receiveSlot = receiveSlotTransient;
        }
        else {
            transmitSlot = transmitSlotSteady;         // 150 normal slots with configuration 1x, 2x, 3x and 4x we have upto 600 slots
            receiveSlot = receiveSlotSteady;
            numSlots = numSlotsSteady;
        }
        resync = true;
    }

    int val = (slot+1)%numSlots;
    if (val != currentSlot || resync) {
        //if (testing)
        //    throw cRuntimeError("Synchronization problem, slots number mismatch");
        // resync
        resync = true;
        currentSlot = val;
        currentMacState = WAIT_NOTIFICATION;
        if (waitAck->isScheduled())
            cancelEvent(waitAck);
        if (waitData->isScheduled())
            cancelEvent(waitData);
        if (waitData->isScheduled())
            cancelEvent(setSleep);
        if (waitAlert->isScheduled())
            cancelEvent(waitAlert);
        if (waitNotification->isScheduled())
            cancelEvent(waitNotification);
        if (sendData->isScheduled())
            cancelEvent(sendData);
        if (sendAck->isScheduled())
            cancelEvent(sendAck);
        if (scheduleAlert->isScheduled())
            cancelEvent(scheduleAlert);
        if (sendAlert->isScheduled())
            cancelEvent(sendAlert);
        if (sendNotification->isScheduled())
            cancelEvent(sendNotification);
        if (ackReceived->isScheduled())
            cancelEvent(ackReceived);
        if (ackTimeout->isScheduled())
            cancelEvent(ackTimeout);
        if (dataTimeout->isScheduled())
            cancelEvent(dataTimeout);
        if (alertTimeout->isScheduled())
            cancelEvent(alertTimeout);
        if (setSleep->isScheduled())
            cancelEvent(setSleep);

    }
    if (!resync)
        return;

    if (transmitSlot[val] == mySlot)
    {
        scheduleAt(simTime(), sendData);
        /*
        if(slot < numSlotsTransient)
        {
            EV << "Immediate next Slot is my Send Slot, getting ready to transmit" << endl;
            scheduleAt(simTime(), sendData);
        }
        else if(slot >= numSlotsTransient)
        {
            EV << "Immediate next Slot is my alert Transmit Slot, getting ready to send" << endl;
            scheduleAt(simTime(), scheduleAlert);
        }
        */
    }
    else if (transmitSlot[val] == alertLevel && !isActuator)
    {
        EV << "Immediate next Slot is my alert Transmit Slot, getting ready to send" << endl;
        scheduleAt(simTime(), scheduleAlert);
    }
    else if (receiveSlot[val] == mySlot)
    {
        EV << "Immediate next Slot is my Receive Slot, getting ready to receive" << endl;
        scheduleAt(simTime(), waitData);
        /*
        if(slot < numSlotsTransient)
        {
            EV << "Immediate next Slot is my Receive Slot, getting ready to receive" << endl;
            scheduleAt(simTime(), waitData);
        }
        else if(slot >= numSlotsTransient)
        {
            EV << "Immediate next Slot is my alert Receive Slot, getting ready to receive" << endl;
            scheduleAt(simTime(), waitAlert);
        }
        */
    }
    else if (receiveSlot[val] == alertLevel)
    {
        EV << "Immediate next Slot is my alert Receive Slot, getting ready to receive" << endl;
        scheduleAt(simTime(), waitAlert);
    }
    else if (receiveSlot[val] == BROADCAST_RECEIVE)
    {

        EV << "Immediate next Slot is Notification Slot, getting ready to receive" << endl;
        scheduleAt(simTime(), waitNotification);
    }
    else if (alwaysListening) {
        EV << "Always listening, getting ready to receive" << endl;
        scheduleAt(simTime(), waitData);
    }
    else
    {
        EV << "Immediate next Slot is Sleep slot.\n";
        scheduleAt(simTime(), setSleep);
    }
}

void DMAMAC::handleLowerPacket(cPacket* msg) {

    if (msg->hasBitError()) {
        EV << "Received " << msg << " contains bit errors or collision, dropping it\n";
        if (currentMacMode == STEADY)
            checkForSuperframeChange = true;
        delete msg;
        return;
    }

    if (dynamic_cast<DMAMACSyncPkt*>(msg))
    {
        // TODO: sync packet, syncronize channels and time slots
        DMAMACSyncPkt * syncPk = dynamic_cast<DMAMACSyncPkt*>(msg);
        currentSlot = syncPk->getTimeSlot();
        numSlots = syncPk->getTdmaNumSlots();
        delete msg;
        return;
    }

    DMAMACPkt * dmapkt = dynamic_cast<DMAMACPkt *>(msg);

    if (!disableChecks) {
        DMAMACSinkPkt *notification = dynamic_cast<DMAMACSinkPkt *>(msg);
        if (notification != nullptr) {
            if (networkId != -1 && networkId != notification->getNetworkId()) {
                // ignore
                EV << "Notification not for me delete \n";
                delete msg;
                return;
            }

            EV << "Received notification from :" << notification->getSrcAddr()
                      << "\n";
            if (notification->getSrcAddr() != sinkAddress) {
                // ignore
                EV << "Notification not for me delete \n";
                delete msg;
                return;
            }
            resyncr(notification->getNumSlot(), static_cast<macMode>(notification->getMacMode()), notification->getChangeMacMode());
        }

        if (dmapkt && (networkId != -1 && networkId != dmapkt->getNetworkId())) {
            // ignore
            EV << "Data of other subnetwork delete \n";
            delete msg;
            return;
        }

        if (dmapkt != nullptr && !endAddressRange.isUnspecified()) {
            if (dmapkt->getSrcAddr() < startAddressRange
                    || dmapkt->getSrcAddr() > endAddressRange) {
                EV << "Sender address out of range Received notification from :"
                          << dmapkt->getSrcAddr() << "\n";
                delete msg;
                return;
            }
        }

        if (auto alert = dynamic_cast<AlertPkt *>(msg)) {
            if (networkId != -1 && networkId != alert->getNetworkId()) {
                // ignore
                EV << "Alarm of other subnetwork delete \n";
                delete msg;
                return;
            }
        }
    }
    if (dmapkt && networkId != dmapkt->getNetworkId())
        nbRxDataErroneous ++;

    bool isDup = false;
    if (dmapkt && dmapkt->getKind() != DMAMAC_ACK) {
        emit(rcvdPkSignalDma,msg);
        MACAddress addr = dmapkt->getSourceAddress();
        if (dmapkt->getKind() == DMAMAC_DATA) {
            bool isDupli = false;
            auto it = longSeqMap.find(addr);
            if (it == longSeqMap.end() || (it != longSeqMap.end() && it->second < dmapkt->getSequence())) {
                longSeqMap[addr] = dmapkt->getSequence();
            }
            else {
//                double t = simTime().dbl();
//                double cre = dmapkt->getCreationTime().dbl();
                isDupli = true;
            }
            isDup = isDupli;
        }

      /*  if (checkDup) {
            auto it = seqMap.find(dmapkt->getSrcAddr());
            if (it != seqMap.end()) {
//                uint8_t distance = ((uint16_t)(dmapkt->getSeq() - (uint16_t)it->second) + 256) % 256;
//                if (distance >= 128) {
//                    isDup = true;
//                }
                if (dmapkt->getSeq() == (uint16_t)it->second) isDup = true;
            }
            seqMap[dmapkt->getSrcAddr()] = dmapkt->getSeq();
        }*/
    }

    if (frequentHopping && !channelSinc) {
        // Synchronize channel hopping
        if (dmapkt && dmapkt->getTimeRef() != SimTime::ZERO) {
            timeRef = dmapkt->getTimeRef();
            initialSeed = dmapkt->getInitialSeed();
            timeRef += slotDuration;
            if (hoppingTimer->isScheduled())
                cancelEvent(hoppingTimer);
            scheduleAt(timeRef, hoppingTimer);
            channelSinc = true;
            // check if this message is a notification
            DMAMACSinkPkt *notification  = dynamic_cast<DMAMACSinkPkt *>(msg);
            if (notification == nullptr)
                scheduleAt(simTime(), startup);
        }
        else {
            auto alert = dynamic_cast<AlertPkt *>(msg);
            if (alert && alert->getTimeRef() != SimTime::ZERO) {
                timeRef = alert->getTimeRef();
                initialSeed = alert->getInitialSeed();
                timeRef += slotDuration;
                if (hoppingTimer->isScheduled())
                    cancelEvent(hoppingTimer);
                scheduleAt(timeRef, hoppingTimer);
                channelSinc = true;
                // check if this message is a notification
                DMAMACSinkPkt *notification  = dynamic_cast<DMAMACSinkPkt *>(msg);
                if (notification == nullptr)
                    scheduleAt(simTime(), startup);
            }
        }
        if (!channelSinc) {
            delete msg;
            return;
        }
    }

    if(currentMacState == WAIT_DATA)
    {
        DMAMACPkt *mac  = dmapkt;
        if (mac == nullptr)
        {
            delete msg;
            return;
        }
        const MACAddress& dest = mac->getDestAddr();

        EV << " DATA Packet received with length :" << mac->getByteLength() << " destined for: " << mac->getDestAddr() << endl;

        /* @brief Checking done for actuator data packets if it is for one of the children */
        if( mac->getKind() == DMAMAC_ACTUATOR_DATA)
        {
            for(int i=0;i<4;i++)
            {
                for(int j=0;j<10;j++)
                {
                    MACAddress childNode =  MACAddress(downStream[i].reachableAddress[j]);
                    if (dest == childNode)
                       forChildNode = true;
                }
            }
            /* @brief Error prevention clause to make space for actuator packets even if there is no space */
            if(macPktQueue.size() == queueLength && !isDup)
            {
                EV_DEBUG << " Deleting packet from DMAMAC queue";
                delete macPktQueue.front().pkt;     // DATA Packet deleted
                macPktQueue.pop_front();
            }
        }

        /* @brief Check if the packet is for me or sink (TDMA So it has to be me in general so just for testing) */
        if(dest == myMacAddr || dest == sinkAddress || forChildNode)
        {
            lastDataPktSrcAddr = mac->getSrcAddr();
            /* @brief not sending forwarding packets up to application layer */
            if (isDup)
                delete mac;
            else {
                if (dest == myMacAddr && sendUppperLayer) {
                    if (isRelayNode)
                        sendUp(mac);
                    else
                        sendUp(decapsMsg(mac));
                } else {
                    if (macPktQueue.size() < queueLength) {
                        packetSend aux;
                        aux.pkt = mac;
                        macPktQueue.push_back(aux);
                        EV_DEBUG
                                        << " DATA packet from child node put in queue : "
                                        << macPktQueue.size() << endl;
                    } else {
                        EV_DEBUG
                                        << " No space for forwarding packets queue size :"
                                        << macPktQueue.size() << endl;
                        delete mac;
                    }
                }
                /* @brief Resetting forChildNode value to false */
                if (forChildNode) {
                    forChildNode = false;
                    /* @statistics */
                    nbRxActuatorData++;
                }
                //if(dest == myMacAddr)
                nbRxData++;
            }

            /* @brief Packet received for myself thus sending an ACK */
            scheduleAt(simTime(), sendAck);

        }
        else
        {
            EV << this->getFullName() << " DATA Packet not for me, deleting";
            EV << " Destination: " << mac->getDestAddr() << " Sender: " << mac->getSrcAddr() << " My address: " << myMacAddr << endl;
            delete mac;
        }
    }
    else if (currentMacState == WAIT_ACK)
    {
        DMAMACPkt *mac  = dynamic_cast<DMAMACPkt *>(msg);
        if (mac == nullptr)
        {
            delete msg;
            return;
        }

        EV_INFO << "ACK Packet received with length : " << mac->getByteLength() << ", DATA Transmission successful"  << endl;

        EV_DEBUG << " Deleting packet from DMAMAC queue";
        delete macPktQueue.front().pkt;     // DATA Packet deleted
        macPktQueue.pop_front();
        delete mac;                     // ACK Packet deleted

        /* @brief Returns self message that just prints an extra ACK received message preserved for future use */
        scheduleAt(simTime(), ackReceived);

        /* @Statistics */
        nbRxAcks++;
    }
    else if (currentMacState == WAIT_NOTIFICATION)
    {
        DMAMACSinkPkt *notification  = dynamic_cast<DMAMACSinkPkt *>(msg);
        if (notification && networkId != notification->getNetworkId())
            nbRxNotificationErroneous++;
        if (notification == nullptr)
        {
            //throw cRuntimeError(" msg is not a notification message");
            delete msg;
            return;
        }
        if (frequentHopping && randomGenerator && !par("useSignalsToChangeChannel").boolValue()) {
            timeRef = notification->getTimeRef();
/*            initialSeed = notification->getInitialSeed();
            if (hoppingTimer->isScheduled())
                cancelEvent(hoppingTimer);
            scheduleAt(timeRef+slotDuration,hoppingTimer);*/
            isSincronized = true;
        }

        changeMacMode = notification->getChangeMacMode();
        EV << " NOTIFICATION packet received with length : " << notification->getByteLength() << " and changeMacMode = " << changeMacMode << endl;

        if (changeMacMode == true)
        {
            EV << " Sink has asked to switch states " << endl;
            if(currentMacMode == STEADY)
            {
                EV << "Initiating switch procedure (STEADY to TRANSIENT) immediately " << endl;
                /* @brief if alert is successful delete saved alert */
                if(alertMessageFromDown && currentMacMode == STEADY)
                    alertMessageFromDown = false;

                /* @brief After last notification of the steady superframe, new Superframe again has notification.
                 * Otherwise in between there is always sleep after notification 
                 * Because before we come here next slot is already found and scheduled
				 * Thus we need to cancel the next event based on the result here.
                 */
                EV << "Current slot value at this time :" << currentSlot << endl;
                if(currentSlot == 0)
                {
					/* @brief This part only re-instates wait notification (Have not tried with ommitting this one) Might not be required */
                    EV << " Notification message arrival time " << waitNotification->getArrivalTime() << endl;
                    const simtime_t nextDiscreteEvent = waitNotification->getArrivalTime();
                    cancelEvent(waitNotification);
                    scheduleAt(nextDiscreteEvent,waitNotification);
                }
                else
                {
					/* @brief Schedules sleep instead of waitNotification */
                    EV << " Sleep message arrival time " << setSleep->getArrivalTime() << endl;

                    if (waitNotification->isScheduled()) {
                        scheduleAt(waitNotification->getArrivalTime(),setSleep);
                        cancelEvent(waitNotification);
                    }
                }

                /* €brief For the case of emergency switch resetting currentSlot counter to 0 */
                if (currentSlot != 0)
                    EV << " Switch asked by !Sink based on Alert" << endl;
            }
            else
                EV << "Initiating switch : TRANSIENT to STEADY in the next Superframe " << endl;
        }

        /* @Statistics */
        nbRxNotifications++;
        if (notification->getChannel() != -1)
            setChannel(notification->getChannel());
        delete notification;

        /* @brief Notification received hence sleeping briefly until next work */
        if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
            if (!alwaysListening) {
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
             }
    }
    else if (currentMacState == WAIT_ALERT)
    {
        AlertPkt *alert  = dynamic_cast<AlertPkt *>(msg);
        if (alert == nullptr)
        {
            delete msg;
            return;
        }


        EV << "I have received an Alert packet from fellow sensor " << endl;
        EV << "Alert Packet length is : " << alert->getByteLength() << " detailed info " << alert->str() << endl;

        EV << " Alert message sent to : " << alert->getDestAddr() << " my address is " << myMacAddr << endl;

        const MACAddress& dest = alert->getDestAddr();

        if (twoLevels && isRelayNode && !alertMessageFromDown){
            EV << "Alert message received from down will be forwarded" << endl;
            alertMessageFromDown = true;

        }
        else if(!alertMessageFromDown && dest == myMacAddr)
        {
            EV << "Alert message received from down will be forwarded" << endl;
            alertMessageFromDown = true;
            /* @Statistics */
            nbRxAlert++;
        }
        else
        {
            EV << "Alert for different node" << endl;
            nbDiscardedAlerts++;
        }

        delete alert;

        /* If in TDMA or Hybrid mode then stop RX process once alert is received. */
        if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
            if (!alwaysListening)
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
    }
    refreshDisplay();
}

/* @brief
 * Finding the distant slot generally after sleep for some slots.
 */
void DMAMAC::findDistantNextSlot()
{
    EV << "Current slot before calculating next wakeup :" << currentSlot << endl;
    int i,x; i = 1;
    x = (currentSlot + i) % numSlots;
    /* @brief if Actuator, only check if node has to receive otherwise check about transmit as well. */
    if(isActuator)
    {
        while(transmitSlot[x] != mySlot && receiveSlot[x] != mySlot && receiveSlot[x] != BROADCAST_RECEIVE && receiveSlot[x] != alertLevel)
        {
          i++;
          x = (currentSlot + i) % numSlots;
        }
    }
    else
    {
        while(transmitSlot[x] != mySlot && receiveSlot[x] != mySlot && receiveSlot[x] != BROADCAST_RECEIVE && transmitSlot[x] != alertLevel && receiveSlot[x] != alertLevel)
        {
          i++;
          x = (currentSlot + i) % numSlots;
        }
    }

    /* @brief
     * Here you could decide if you want to let the node sleep,
     * if the next transmit or receive slot is to appear soon
     * We set it to 1 for the time being.
	 * This means atleast one sleep slot
     *    if ( i > 1)
      {
     */
    /* @brief if not sleeping already then set sleep */
    if (!alwaysListening) {
        if (radio->getRadioMode() != IRadio::RADIO_MODE_SLEEP)
            radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
        else
            EV << "Radio already in sleep just making wakeup calculations " << endl;
    }
    else {
        if (radio->getRadioMode() != IRadio::RADIO_MODE_RECEIVER)
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
    }

	/* @brief Time to next event */
    nextEvent = slotDuration*(i);
    currentSlot +=i;

    /* @Statistics */
    nbSleepSlots +=i;

    currentSlot %= numSlots;

    EV << "Time for RADIO Sleep for " << nextEvent << " Seconds" << endl;
    EV << "Waking up in slot :" << currentSlot << endl;

    /* @brief
    * Checking if next active slot found is a transmit slot or receive slot. Then scheduling
    * the next event for the time when the node will enter into transmit or receive.
    */
    if(transmitSlot[(currentSlot) % numSlots] == mySlot)
    {
        scheduleAt(simTime() + nextEvent, sendData);
        /*
        if(currentSlot < numSlotsTransient)
        {
            EV << "My next slot after sleep is transmit slot" << endl;
            scheduleAt(simTime() + nextEvent, sendData);
        }
        else if(currentSlot >= numSlotsTransient)
        {
            EV << "My next slot after sleep is alert Transmit Slot" << endl;
            scheduleAt(simTime() + nextEvent, scheduleAlert);
        }
        */
    }
    else if (transmitSlot[(currentSlot) % numSlots] == alertLevel && !isActuator)
    {
        EV << "My next slot after sleep is alert Transmit Slot" << endl;
        scheduleAt(simTime() + nextEvent, scheduleAlert);
    }
    else if (receiveSlot[(currentSlot) % numSlots] == alertLevel)
    {
        EV << "My next slot after sleep is alert receive Slot" << endl;
        scheduleAt(simTime() + nextEvent, waitAlert);
    }
    else if (receiveSlot[(currentSlot) % numSlots] == BROADCAST_RECEIVE)
    {
        EV << "My next slot after sleep is notification slot" << endl;
        scheduleAt(simTime() + nextEvent, waitNotification);
    }
    else if (receiveSlot[(currentSlot) % numSlots] == mySlot)
    {
        scheduleAt(simTime() + nextEvent, waitData);
        /*
        if(currentSlot < numSlotsTransient)
        {
            EV << "My next slot after sleep is receive slot" << endl;
            scheduleAt(simTime() + nextEvent, waitData);
        }
        else if(currentSlot >= numSlotsTransient)
        {
            EV << "My next slot after sleep is alert receive Slot" << endl;
            scheduleAt(simTime() + nextEvent, waitAlert);
        }
        */
    }
    else if (alwaysListening) {
        scheduleAt(simTime() + nextEvent, waitData);
    }
    else
            EV << " Undefined MAC state <ERROR>" << endl;

}

/* @brief Finding immediate next Slot */
void DMAMAC::findImmediateNextSlot(int currentSlotLocal,simtime_t nextSlot)
{
    EV << "Finding immediate next slot" << endl;
    if (transmitSlot[(currentSlotLocal + 1) % numSlots] == mySlot)
    {
        if(currentSlotLocal < numSlotsTransient)
        {
            EV << "Immediate next Slot is my Send Slot, getting ready to transmit" << endl;
            scheduleAt(simTime() + nextSlot, sendData);
        }
        else if(currentSlotLocal >= numSlotsTransient)
        {
            EV << "Immediate next Slot is my alert Transmit Slot, getting ready to send" << endl;
            scheduleAt(simTime() + nextSlot, scheduleAlert);
        }
    }
    else if (transmitSlot[(currentSlotLocal + 1) % numSlots] == alertLevel && !isActuator)
    {
        EV << "Immediate next Slot is my alert Transmit Slot, getting ready to send" << endl;
        scheduleAt(simTime() + nextSlot, scheduleAlert);
    }
    else if (receiveSlot[(currentSlotLocal + 1) % numSlots] == mySlot)
    {
        scheduleAt(simTime() + nextSlot, waitData);
        /*
        if(currentSlotLocal < numSlotsTransient)
        {
            EV << "Immediate next Slot is my Receive Slot, getting ready to receive" << endl;
            scheduleAt(simTime() + nextSlot, waitData);
        }
        else if(currentSlotLocal >= numSlotsTransient)
        {
            EV << "Immediate next Slot is my alert Receive Slot, getting ready to receive" << endl;
            scheduleAt(simTime() + nextSlot, waitAlert);
        }
        */
    }
    else if (receiveSlot[(currentSlotLocal + 1) % numSlots] == alertLevel)
    {
        EV << "Immediate next Slot is my alert Receive Slot, getting ready to receive" << endl;
        scheduleAt(simTime() + nextSlot, waitAlert);
    }
    else if (receiveSlot[(currentSlotLocal + 1) % numSlots] == BROADCAST_RECEIVE)
    {
        EV << "Immediate next Slot is Notification Slot, getting ready to receive" << endl;
        scheduleAt(simTime() + nextSlot, waitNotification);
    }
    else
    {
        EV << "Immediate next Slot is Sleep slot.\n";
        scheduleAt(simTime() + nextSlot, setSleep);
    }
}

/* @brief Used by encapsulation function */
void DMAMAC::attachSignal(MACFrameBase* macPkt)
{
    simtime_t duration = macPkt->getBitLength() / bitrate;
    macPkt->setDuration(duration);
}

/* @brief To change main superframe to the new mac mode superframe */
void DMAMAC::changeSuperFrame(macMode mode)
{
    /* @brief Switching superframe slots to new mode */

    EV << "Switching superframe slots to new mode " << endl;

    if (mode == TRANSIENT)
    {
        transmitSlot = transmitSlotTransient;
        receiveSlot = receiveSlotTransient;

        numSlots = numSlotsTransient;
        EV << "In mode : " << mode << " , and number of slots : " << numSlots << endl;
    }
    else if (mode == STEADY)
    {
        transmitSlot = transmitSlotSteady;
        receiveSlot = receiveSlotSteady;
        numSlots = numSlotsSteady;
        EV << "In mode : " << mode << " , and number of slots : " << numSlots << endl;
    }

    /* @brief Set change mac mode back to false */
    changeMacMode = false;

    EV << "Transmit Slots and Receive Slots" << endl;
    for(int i=0; i < numSlots; i ++)
    {
        EV_DEBUG << " Send Slot "<< i << " = " << transmitSlot[i] << endl;
    }
    for(int i=0; i < numSlots; i ++)
    {
        EV_DEBUG << " Receive Slot "<< i << " = " << receiveSlot[i] << endl;
    }
    /* @brief Current Slot is set to Zero with start of new superframe */
    currentSlot=0;

    /* @brief period definition */
    macPeriod = DATA;
}

/* @brief Extracting the static slot schedule here from the xml files */
void DMAMAC::slotInitialize()
{
    EV << " Starting in transient state operation (just to be safe) " << endl;

    currentMacMode = TRANSIENT;

    /* @brief Intializing slotting arrays */

    /* @brief Steady superframe copied */
    xmlBuffer = xmlFileSteady->getFirstChildWithTag("transmitSlots");
    nListBuffer = xmlBuffer->getChildren(); //Gets all children (slot values)

    /* @brief Steady superframe listed continuously as values listing the slot indicator */
    xmlListIterator = nListBuffer.begin();
    for(int i=0;xmlListIterator!=nListBuffer.end();xmlListIterator++,i++)
    {
       xmlBuffer = (*xmlListIterator);
       EV_DEBUG << " XML stuff " << xmlBuffer->getNodeValue() << endl;
       transmitSlotSteady.push_back(atoi(xmlBuffer->getNodeValue()));
    }

    xmlBuffer = xmlFileSteady->getFirstChildWithTag("receiveSlots");
    nListBuffer = xmlBuffer->getChildren();
    xmlListIterator = nListBuffer.begin();

    for(int i=0;xmlListIterator!=nListBuffer.end();xmlListIterator++,i++)
    {
       xmlBuffer = (*xmlListIterator);
       EV_DEBUG << " XML stuff " << xmlBuffer->getNodeValue() << endl;
       receiveSlotSteady.push_back(atoi(xmlBuffer->getNodeValue()));
    }

    if (receiveSlotSteady.size() != transmitSlotSteady.size())
        throw cRuntimeError("receiveSlotSteady.size() != transmitSlotSteady.size()");

    /* @brief Transient superframe copied */
    xmlBuffer = xmlFileTransient->getFirstChildWithTag("transmitSlots");
    nListBuffer = xmlBuffer->getChildren();
    xmlListIterator = nListBuffer.begin();

    for(int i=0;xmlListIterator!=nListBuffer.end();xmlListIterator++,i++)
    {
       xmlBuffer = (*xmlListIterator);
       EV_DEBUG << " XML stuff " << xmlBuffer->getNodeValue() << endl;
       transmitSlotTransient.push_back(atoi(xmlBuffer->getNodeValue()));
    }

    xmlBuffer = xmlFileTransient->getFirstChildWithTag("receiveSlots");
    nListBuffer = xmlBuffer->getChildren();
    xmlListIterator = nListBuffer.begin();

    for(int i=0;xmlListIterator!=nListBuffer.end();xmlListIterator++,i++)
    {
       xmlBuffer = (*xmlListIterator);
       EV_DEBUG << " XML stuff " << xmlBuffer->getNodeValue() << endl;
       receiveSlotTransient.push_back(atoi(xmlBuffer->getNodeValue()));
    }
    if (receiveSlotTransient.size() != transmitSlotTransient.size())
        throw cRuntimeError("receiveSlotTransient.size() != transmitSlotTransient.size()");

// resize
    if ((int)transmitSlotTransient.size() > numSlotsTransient) {
        receiveSlotTransient.resize(numSlotsTransient);
        transmitSlotTransient.resize(numSlotsTransient);
    }
    else if ((int)transmitSlotTransient.size() < numSlotsTransient)
        numSlotsTransient = transmitSlotTransient.size();

    if ((int)receiveSlotSteady.size() > numSlotsSteady) {
        receiveSlotSteady.resize(numSlotsSteady);
        receiveSlotSteady.resize(numSlotsSteady);
    }
    else if ((int)receiveSlotSteady.size() < numSlotsSteady)
        numSlotsSteady = receiveSlotSteady.size();

    /* @brief Initializing to prevent random values */
    transmitSlot = transmitSlotTransient;
    receiveSlot = receiveSlotTransient;

    for (unsigned int i = 0; i < receiveSlotSteady.size();i++) {
            if ((receiveSlotSteady[i] == transmitSlotSteady[i]) &&  transmitSlotSteady[i] != 102) {
                throw cRuntimeError("receiveSlotSteady[i] == transmitSlotSteady[i] i = %i",i);
            }
        }

    for (unsigned int i = 0; i < receiveSlotTransient.size();i++) {
        if ((receiveSlotTransient[i] == transmitSlotTransient[i]) &&  transmitSlotTransient[i] != 102) {
            throw cRuntimeError("receiveSlotTransient[i] == transmitSlotTransient[i] i = %i",i);
        }
    }


    /*
    for(int i=0; i < maxNumSlots; i ++)
    {
        transmitSlot[i] = -1;
        receiveSlot[i] = -1;
    }
*/
    /* @brief Starting with transient superframe */
    /*
    for(int i=0; i < maxNumSlots; i ++)
    {
       transmitSlot[i] = transmitSlotTransient[i];
       receiveSlot[i] = receiveSlotTransient[i];
    }
    */

    numSlots = numSlotsTransient;

    EV_DEBUG << "Transmit Slots and Receive Slots" << endl;
    for(int i=0; i < maxNumSlots; i ++)
    {
        EV_DEBUG << " Send Slot "<< i << " = " << transmitSlot[i] << endl;
    }
    for(int i=0; i < maxNumSlots; i ++)
    {
        EV_DEBUG << " Receive Slot "<< i << " = " << receiveSlot[i] << endl;
    }

    /* @brief Extracting neighbor data from the input xml file */
    //if (myId != sinkAddress)

    cXMLElement *xmlBuffer1,*xmlBuffer2;

    cXMLElement* rootElement = par("neighborData").xmlValue();

    char id[maxNodes];
    sprintf(id,"%"  PRIu64, myId);
    EV << " My ID is : " << myId << endl;


    xmlBuffer = rootElement->getElementById(id);
    if (xmlBuffer == nullptr)
        throw cRuntimeError("Error in configuration file neighborData, id = %s not found",id);

    alertLevel = atoi(xmlBuffer->getFirstChildWithTag("level")->getNodeValue());
    EV << " Node is at alertLevel " << alertLevel << endl;

    parent = long(atoi(xmlBuffer->getFirstChildWithTag("parent")->getNodeValue()));
    EV << "My Parent is " << parent << endl;
    int reachableNodes[maxNodes],nextHopEntry;
    int i=0,j=0;

    for(j=0;j<maxNodes;j++)
       reachableNodes[j]=-1;

    /* @brief Initialization */
    for(int i=0;i<maxChildren;i++)
    {
        for(int j=0;j<maxNodes;j++)
        {
            downStream[i].nextHop = -1;
            downStream[i].reachableAddress[j] = -1;
        }
    }

    cXMLElementList::iterator xmlListIterator1;
    cXMLElementList::iterator xmlListIterator2;
    cXMLElementList nListBuffer1,nListBuffer2;

    nListBuffer1 = xmlBuffer->getElementsByTagName("nextHop");

    j=0;
    for(xmlListIterator1 = nListBuffer1.begin();xmlListIterator1!=nListBuffer1.end();xmlListIterator1++)
    {
      xmlBuffer1 = (*xmlListIterator1);
      EV<< " nList buffer " << xmlBuffer1->getAttribute("address") << endl;
      nextHopEntry = atoi(xmlBuffer1->getAttribute("address"));
      EV << "next hop entry is " << nextHopEntry << endl;
      if(xmlBuffer1->hasChildren())
      {
          nListBuffer2 = xmlBuffer1->getChildren();
          for(xmlListIterator2 = nListBuffer2.begin();xmlListIterator2!=nListBuffer2.end();xmlListIterator2++)
          {
              xmlBuffer2 = (*xmlListIterator2);
              EV << " XML stuff " << xmlBuffer2->getNodeValue() << endl;
              reachableNodes[i] = atoi(xmlBuffer2->getNodeValue());
              i++;
          }
          downStream[j].nextHop = nextHopEntry;
          for(int x=0;x<i;x++)
              downStream[j].reachableAddress[x] = reachableNodes[x];
          /* @brief Resetting reachableNodes to input next values */
          for(int x=0;x<maxNodes;x++)
              reachableNodes[x] = -1;
          i=0;
          j++;
      }
      else
      {
          EV << "Children are leaf nodes" << endl;
          reachableNodes[0] = nextHopEntry;
          downStream[j].nextHop = nextHopEntry;
          for(int x=0;x<maxNodes;x++)
              downStream[j].reachableAddress[x] = reachableNodes[x];
          j++;
      }
    }
    /* @brief All nodes can have max 3 children so we list only 3 for their next hops. */
    for(int j=0;j<maxChildren;j++)
    {
        EV << " The downstream possibilities at :" << downStream[j].nextHop << " are :";
        for(int x=0;x<maxNodes;x++)
        {
          if(downStream[j].reachableAddress[x] != -1)
              EV << "Node: " << downStream[j].reachableAddress[x];
        }
        EV <<  endl;
    }
}


bool DMAMAC::handleNodeShutdown(IDoneCallback *doneCallback)
{
    cancelEvent(startup);
    cancelEvent(setSleep);
    cancelEvent(waitData);
    cancelEvent(waitAck);
    cancelEvent(waitAlert);
    cancelEvent(waitNotification);
    cancelEvent(sendData);
    cancelEvent(sendAck);
    cancelEvent(scheduleAlert);
    cancelEvent(sendAlert);
    cancelEvent(sendNotification);
    cancelEvent(ackReceived);
    cancelEvent(ackTimeout);
    cancelEvent(dataTimeout);
    cancelEvent(alertTimeout);
    if (hoppingTimer)
        cancelEvent(hoppingTimer);
    MacPktQueue::iterator it1;
    for(it1 = macPktQueue.begin(); it1 != macPktQueue.end(); ++it1) {
        delete (*it1).pkt;
    }
    macPktQueue.clear();
    shutDown = true;
    radio->setRadioMode(IRadio::RADIO_MODE_OFF);
    return true;
}

void DMAMAC::handleNodeCrash()
{
    cancelEvent(startup);
    cancelEvent(setSleep);
    cancelEvent(waitData);
    cancelEvent(waitAck);
    cancelEvent(waitAlert);
    cancelEvent(waitNotification);
    cancelEvent(sendData);
    cancelEvent(sendAck);
    cancelEvent(scheduleAlert);
    cancelEvent(sendAlert);
    cancelEvent(sendNotification);
    cancelEvent(ackReceived);
    cancelEvent(ackTimeout);
    cancelEvent(dataTimeout);
    cancelEvent(alertTimeout);
    if (hoppingTimer)
        cancelEvent(hoppingTimer);
    MacPktQueue::iterator it1;
    for(it1 = macPktQueue.begin(); it1 != macPktQueue.end(); ++it1) {
        delete (*it1).pkt;
    }
    macPktQueue.clear();
    shutDown = true;
    radio->setRadioMode(IRadio::RADIO_MODE_OFF);
}


cPacket *DMAMAC::decapsMsg(MACFrameBase *macPkt)
{
    cPacket *msg = macPkt->decapsulate();
    setUpControlInfo(msg, macPkt->getSrcAddr());
    return msg;
}

/**
 * Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
 */
cObject *DMAMAC::setUpControlInfo(cMessage *const pMsg, const MACAddress& pSrcAddr)
{
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setSrc(pSrcAddr);
    pMsg->setControlInfo(cCtrlInfo);
    return (cCtrlInfo);
}


void DMAMAC::initializeRandomSeq() {
    // initialize the random generator.
    if (par("initialSeed").longValue() != -1)
    {
        randomGenerator = new CRandomMother(par("initialSeed").longValue());
        setNextSequenceChannel();
        initialSeed = par("initialSeed").longValue();
    }
    else
    {
        setChannel(par("initialChannel"));
        randomGenerator = nullptr;
    }
}

void DMAMAC::refreshDisplay() {
    nodeStatus status;
    status.currentMacMode = currentMacMode;
    status.currentMacState = currentMacState;
    status.previousMacMode = previousMacMode;
    status.currentSlot = currentSlot;

    slotMap[myMacAddr.getInt()] = status;

    // check sink slot
 /*   auto it = slotMap.find(sinkAddress.getInt());
    if (it->second.currentSlot != currentSlot) {
        // check status
        printf("");
    }*/

    char buf[150];
    sprintf(buf, "Current Mac mode Mode :%i Current mac State %d Channel: %d Slot: %d MySlot %d",currentMacMode,currentMacState, actualChannel,currentSlot,mySlot);
    getDisplayString().setTagArg("t", 0, buf);
}

void DMAMAC::setChannel(const int &channel) {
    if (channel < 11 || channel > 26)
        return;
    if (actualChannel == channel)
        return;

    EV << "Hop to channel :" << channel << endl;
    bubble("Changing channel");
    actualChannel = channel;

    ConfigureRadioCommand *configureCommand = new ConfigureRadioCommand();
    configureCommand->setBandwidth(Hz(channels[actualChannel-11].bandwith));
    configureCommand->setCarrierFrequency(Hz(channels[actualChannel-11].mean));

    cMessage *message = new cMessage("configureRadioMode", RADIO_C_CONFIGURE);
    message->setControlInfo(configureCommand);
    sendDown(message);
}

double DMAMAC::getCarrierChannel(const int &channel) {
    if (channel < 11 || channel > 26)
        return (-1);
    return (channels[channel-11].mean);
}

double DMAMAC::getBandwithChannel(const int &channel) {
    if (channel < 11 || channel > 26)
        return (-1);
    return (channels[channel-11].bandwith);
}

void DMAMAC::setNextSequenceChannel()
{
    if (randomGenerator == nullptr)
       return;

    int c = randomGenerator->iRandom(11,26);
    while (c == reserveChannel)
        c = randomGenerator->iRandom(11,26);

    setChannel(c);
}

void DMAMAC::setChannelWithSeq(const uint32_t *v)
{
    if (randomGenerator == nullptr)
        return;
    int c = randomGenerator->iRandom(11,26,v);
    while (c == reserveChannel)
        c = randomGenerator->iRandom(11,26);
    setChannel(c);
}

void DMAMAC::setRandSeq(const uint64_t &v)
{
    if (randomGenerator == nullptr)
        return;
    randomGenerator->setRandSeq(v);
}

// Output random bits
uint32_t DMAMAC::CRandomMother::bRandom() {
  uint64_t sum;
  for (int i = 0; i < 5 ; i++)
      Prevx[i] = x[i];
  sum = (uint64_t)2111111111UL * (uint64_t)x[3] +
     (uint64_t)1492 * (uint64_t)(x[2]) +
     (uint64_t)1776 * (uint64_t)(x[1]) +
     (uint64_t)5115 * (uint64_t)(x[0]) +
     (uint64_t)x[4];
  x[3] = x[2];  x[2] = x[1];  x[1] = x[0];
  x[4] = (uint32_t)(sum >> 32);                  // Carry
  x[0] = (uint32_t)sum;                          // Low 32 bits of sum
  return (x[0]);
}

uint32_t DMAMAC::CRandomMother::bRandom(const uint32_t *v) {
  uint64_t sum;

  sum = (uint64_t)2111111111UL * (uint64_t)v[3] +
     (uint64_t)1492 * (uint64_t)(v[2]) +
     (uint64_t)1776 * (uint64_t)(v[1]) +
     (uint64_t)5115 * (uint64_t)(v[0]) +
     (uint64_t)v[4];                         // Low 32 bits of sum
  return ((uint32_t)sum);
}

// returns a random number between 0 and 1:
double DMAMAC::CRandomMother::random() {
   return ((double)bRandom() * (1./(65536.*65536.)));
}

// returns integer random number in desired interval:
int DMAMAC::CRandomMother::iRandom(int min, int max) {
   // Output random integer in the interval min <= x <= max
   // Relative error on frequencies < 2^-32
   if (max <= min) {
      if (max == min) return (min); else return (0x80000000);
   }
   // Assume 64 bit integers supported. Use multiply and shift method
   uint32_t interval;                  // Length of interval
   uint64_t longran;                   // Random bits * interval
   uint32_t iran;                      // Longran / 2^32

   interval = (uint32_t)(max - min + 1);
   longran  = (uint64_t)bRandom() * interval;
   iran = (uint32_t)(longran >> 32);
   // Convert back to signed and return result
   return ((int32_t)iran + min);
}

// returns integer random number in desired interval:
int DMAMAC::CRandomMother::iRandom(int min, int max, const uint32_t *v) {
    // Output random integer in the interval min <= x <= max
    // Relative error on frequencies < 2^-32
    if (max <= min) {
        if (max == min) return (min); else return (0x80000000);
    }
    // Assume 64 bit integers supported. Use multiply and shift method
    uint32_t interval;                  // Length of interval
    uint64_t longran;                   // Random bits * interval
    uint32_t iran;                      // Longran / 2^32
    interval = (uint32_t)(max - min + 1);
    longran  = (uint64_t)bRandom(v) * interval;
    iran = (uint32_t)(longran >> 32);
    // Convert back to signed and return result
    return ((int32_t)iran + min);
}

// this function initializes the random number generator:
void DMAMAC::CRandomMother::randomInit (int seed) {
  int i;
  uint32_t s = seed;
  // make random numbers and put them into the buffer
  for (i = 0; i < 5; i++) {
    s = s * 29943829 - 1;
    x[i] = s;
  }
  // randomize some more
  for (i=0; i<19; i++) bRandom();
}

void DMAMAC::CRandomMother::getRandSeq(uint32_t *v) {
    for (int i = 0 ; i < 5; i++)
        v[i] = x[i];
}

void DMAMAC::CRandomMother::getPrevSeq(uint32_t *v) {
    for (int i = 0 ; i < 5; i++)
        v[i] = Prevx[i];
}

void DMAMAC::CRandomMother::setRandSeq(const uint32_t *v) {
    for (int i = 0 ; i < 5; i++)
        x[i] = v[i];
}

void DMAMAC::CRandomMother::setRandSeq(const uint64_t &v) {

    for (int i = 0 ; i < 5; i++)
            x[i] = 0;
    x[0] = v & 0xFFFFFFFF;
    x[1] = (v >>32) & 0xFFFFFFFF;
    // randomize some more
    for (int i=0; i<19; i++) bRandom();
  }
}
