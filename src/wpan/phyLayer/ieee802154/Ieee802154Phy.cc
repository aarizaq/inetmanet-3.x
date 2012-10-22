#include "Ieee802154Phy.h"
#include "BasicBattery.h"
#include "PhyControlInfo_m.h"
#include "Radio80211aControlInfo_m.h"

#define MIN_DISTANCE 0.001 // minimum distance 1 millimeter
// #undef EV
//#define EV (ev.isDisabled() || !m_debug) ? std::cout : ev  ==> EV is now part of <omnetpp.h>

simsignal_t Ieee802154Phy::bitrateSignal = SIMSIGNAL_NULL;
simsignal_t Ieee802154Phy::radioStateSignal = SIMSIGNAL_NULL;
simsignal_t Ieee802154Phy::channelNumberSignal = SIMSIGNAL_NULL;
simsignal_t Ieee802154Phy::lossRateSignal = SIMSIGNAL_NULL;

Define_Module(Ieee802154Phy);


uint16_t Ieee802154Phy::aMaxPHYPacketSize  = 127;      //max PSDU size (in bytes) the PHY shall be able to receive
uint16_t Ieee802154Phy::aTurnaroundTime    = 12;       //Rx-to-Tx or Tx-to-Rx max turnaround time (in symbol period)
uint16_t Ieee802154Phy::aMaxBeaconOverhead             = 75;       //max # of octets added by the MAC sublayer to the payload of its beacon frame
uint16_t Ieee802154Phy::aMaxBeaconPayloadLength = aMaxPHYPacketSize - aMaxBeaconOverhead;       //max size, in octets, of a beacon payload
uint16_t Ieee802154Phy::aMaxFrameOverhead              = 25;       //max # of octets added by the MAC sublayer to its payload w/o security.//max # of octets that can be transmitted in the MAC frame payload field
uint16_t Ieee802154Phy::aMaxMACFrameSize  = aMaxPHYPacketSize - aMaxFrameOverhead;


Ieee802154Phy::Ieee802154Phy() : rs(this->getId())
{
    radioModel = NULL;
    receptionModel = NULL;
    CCA_timer = NULL;
    ED_timer = NULL;
    TRX_timer = NULL;
    TxOver_timer = NULL;
    updateString =NULL;
    transceiverConnect = true;
    receiverConnect = true;
    numReceivedCorrectly = 0;
    numGivenUp = 0;

}

void Ieee802154Phy::registerBattery()
{
    BasicBattery *bat = BatteryAccess().getIfExists();
    if (bat)
    {
        //int id,double mUsageRadioIdle,double mUsageRadioRecv,double mUsageRadioSend,double mUsageRadioSleep)=0;
        // read parameters
        double mUsageRadioIdle      = par("usage_radio_idle");
        double mUsageRadioRecv      = par("usage_radio_recv");
        double mUsageRadioSleep     = par("usage_radio_sleep");
        double mTransmitterPower        = par("transmitterPower");

        double trans_power_dbm = 10*log10(mTransmitterPower);
        // calculation of usage_radio_send
        // based on the values in Olaf Landsiedel's AEON paper
        /*if (trans_power_dbm <= -18)
            mUsageRadioSend = 8.8;
        else if (trans_power_dbm <= -13)
            mUsageRadioSend = 9.8;
        else if (trans_power_dbm <= -10)
            mUsageRadioSend = 10.4;
        else if (trans_power_dbm <= -6)
            mUsageRadioSend = 11.3;
        else if (trans_power_dbm <= -2)
            mUsageRadioSend = 15.6;
        else if (trans_power_dbm <= 0)
            mUsageRadioSend = 17;
        else if (trans_power_dbm <= 3)
            mUsageRadioSend = 20.2;
        else if (trans_power_dbm <= 4)
            mUsageRadioSend = 22.5;
        else if (trans_power_dbm <= 5)
            mUsageRadioSend = 26.9;
        else
            error("[Battery]: transmitter Power too high!");*/

        // based on the values for CC2420 in howitt paper
        double mUsageRadioSend;
        if (trans_power_dbm <= -25)
            mUsageRadioSend = 8.53;
        else if (trans_power_dbm <= -15)
            mUsageRadioSend = 9.64;
        else if (trans_power_dbm <= -10)
            mUsageRadioSend = 10.68;
        else if (trans_power_dbm <= -7)
            mUsageRadioSend = 11.86;
        else if (trans_power_dbm <= -5)
            mUsageRadioSend = 13.11;
        else if (trans_power_dbm <= -3)
            mUsageRadioSend = 14.09;
        else if (trans_power_dbm <= -1)
            mUsageRadioSend = 15.07;
        else if (trans_power_dbm <= 0)
            mUsageRadioSend = 16.24;
        else
            error("[Battery]: transmitter Power too high!");
        bat->registerWirelessDevice(rs.getRadioId(),mUsageRadioIdle,mUsageRadioRecv,mUsageRadioSend,mUsageRadioSleep);
    }
}

Ieee802154Phy::~Ieee802154Phy()
{
    cancelAndDelete(CCA_timer);
    cancelAndDelete(ED_timer);
    cancelAndDelete(TRX_timer);
    cancelAndDelete(TxOver_timer);
    delete radioModel;
    delete receptionModel;
    if (txPktCopy != NULL)
        delete txPktCopy;

    // delete messages being received
    for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
        delete it->first;
}

void Ieee802154Phy::initialize(int stage)
{
    ChannelAccess::initialize(stage);

    EV << getParentModule()->getParentModule()->getFullName() << ": initializing Ieee802154Phy, stage=" << stage << endl;

    if (stage == 0)
    {
        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        gate("radioIn")->setDeliverOnReceptionStart(true);

        // The following parameters to be specified in omnetpp.ini
        m_debug             = par("debug");
        rs.setChannelNumber(par("channelNumber").longValue()); // default: 11, 2.4G
        // carrierFrequency     = cc->par("carrierFrequency");  // taken from ChannelControl
        carrierFrequency = par("carrierFrequency");
        sensitivity             = FWMath::dBm2mW(par("sensitivity").doubleValue()); // -85 dBm for 2450 MHz, -92 dBm for 868/915 MHz
        thermalNoise            = FWMath::dBm2mW(par("thermalNoise").doubleValue());
        transmitterPower        = par("transmitterPower");  // in mW
        if (transmitterPower > (double) getChannelControlPar("pMax"))
            error("[PHY]: transmitterPower cannot be bigger than pMax in ChannelControl!");

        // initialize noiseLevel
        noiseLevel = thermalNoise;
        snrInfo.ptr = NULL;
        txPktCopy = NULL;
        //rxPkt = NULL;
        for (int i=0; i<27; i++)
        {
            rxPower[i] = 0;     // remember to clear after channel switching
        }
        rxPeakPower = 0;
        numCurrRx = 0;

        phyRadioState = phy_RX_ON;
        PLME_SET_TRX_STATE_confirm(phyRadioState);
        rs.setState(RadioState::IDLE);
        //rs.setChannelNumber((int)def_phyCurrentChannel); // default: 11, 2.4G
        rs.setBitrate(getRate('b'));
        rs.setRadioId(this->getId());

        newState = phy_IDLE;
        newState_turnaround = phy_IDLE;
        isCCAStartIdle = false;

        // initalize self messages (timer)
        CCA_timer   = new cMessage("CCA_timer",     PHY_CCA_TIMER);
        ED_timer    = new cMessage("ED_timer",  PHY_ED_TIMER);
        TRX_timer   = new cMessage("TRX_timer", PHY_TRX_TIMER);
        TxOver_timer    = new cMessage("TxOver_timer",  PHY_TX_OVER_TIMER);


        obstacles = ObstacleControlAccess().getIfExists();
        if (obstacles) EV << "Found ObstacleControl" << endl;

        // this is the parameter of the channel controller (global)
        std::string propModel = getChannelControlPar("propagationModel").stdstringValue();
        if (propModel == "")
            propModel = "FreeSpaceModel";

        receptionModel = (IReceptionModel *) createOne(propModel.c_str());
        receptionModel->initializeFrom(this);

        // radio model to handle frame length and reception success calculation (modulation, error correction etc.)
        std::string rModel =  par("radioModel").stdstringValue();
        if (rModel=="")
            rModel = "GenericRadioModel";

        radioModel = (IRadioModel *) createOne(rModel.c_str());
        radioModel->initializeFrom(this);

        radioModel = createRadioModel();
        radioModel->initializeFrom(this);
        bitrateSignal = registerSignal("bitrate");
        radioStateSignal = registerSignal("radioState");
        channelNumberSignal = registerSignal("channelNo");
        lossRateSignal = registerSignal("lossRate");
        bool change=false;
        if (par("aMaxPHYPacketSize").longValue()!=aMaxPHYPacketSize)
        {
            change=true;
            aMaxPHYPacketSize=par("aMaxPHYPacketSize").longValue();
        }

        if (aTurnaroundTime!=par("aTurnaroundTime").longValue())
        {
            change=true;
            aTurnaroundTime=par("aTurnaroundTime").longValue();
        }

        if (aMaxBeaconOverhead!=par("aMaxBeaconOverhead").longValue())
        {
            change=true;
            aMaxBeaconOverhead=par("aMaxBeaconOverhead").longValue();
        }

        if (aMaxFrameOverhead!=par("aMaxFrameOverhead").longValue())
        {
            change=true;
            aMaxFrameOverhead=par("aMaxFrameOverhead").longValue();
        }
        if (change)
        {
            aMaxBeaconPayloadLength = aMaxPHYPacketSize - aMaxBeaconOverhead;       //max size, in octets, of a beacon payload
            aMaxMACFrameSize  = aMaxPHYPacketSize - aMaxFrameOverhead;
        }


    }
    else if (stage == 1)
    {
        // tell initial values to MAC; must be done in stage 1, because they
        // subscribe in stage 0
        nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
        nb->fireChangeNotification(NF_RADIO_CHANNEL_CHANGED, &rs);
    }
    else if (stage == 2)
    {
        // tell initial channel number to ChannelControl; should be done in
        // stage==2 or later, because base class initializes myHostRef in that stage

       	cc->setRadioChannel(myRadioRef, rs.getChannelNumber());

            // statistics
        emit(bitrateSignal, rs.getBitrate());
        emit(radioStateSignal, rs.getState());
        emit(channelNumberSignal, rs.getChannelNumber());

        if (this->hasPar("drawCoverage"))
            drawCoverage = par("drawCoverage");
        else
            drawCoverage = false;
        registerBattery();
        this->updateDisplayString();
        if (this->hasPar("refresCoverageInterval"))
        	updateStringInterval = par("refresCoverageInterval");
        else
        	updateStringInterval = 0;
        WATCH(rs);
        WATCH(phyRadioState);
    }
}

void Ieee802154Phy::finish()
{
}

bool Ieee802154Phy::processAirFrame(AirFrame *airframe)
{

    int chnum = airframe->getChannelNumber();
    return (chnum == getChannelNumber());
}

void Ieee802154Phy::handleMessage(cMessage *msg)
{
    // handle primitives
    if (updateString && updateString==msg)
    {
        this->updateDisplayString();
        return;
    }
    if (!msg->isSelfMessage())
    {
        if (msg->getArrivalGateId()==upperLayerIn && (dynamic_cast<cPacket*>(msg)==NULL))
        {
            if (msg->getKind()==0)
                error("[PHY]: Message '%s' with length==0 is supposed to be a primitive, but msg kind is also zero", msg->getName());
            EV << "[PHY]: a primitive received from MAC layer, processing ..." << endl;
            handlePrimitive(msg->getKind(), msg);
            return;
        }
    }
    if (msg->getArrivalGateId() == upperLayerIn)
    {
        if (transceiverConnect)
        {
            EV << "[PHY]: a MAC frame " << msg->getName()  << " received from MAC layer" << endl;
            AirFrame *airframe = encapsulatePacket(msg);
            handleUpperMsg(airframe);
        }
        else
        {
            EV << "[PHY]: a MAC frame " << msg->getName()  << " received from MAC layer but transceiver is disconnected, delete packet" << endl;
            delete msg;
        }
    }
    else if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
    }
    else if (processAirFrame (check_and_cast<AirFrame*>(msg)))
    {
        if (this->isEnabled() && receiverConnect)
        {
            // must be an AirFrame
            AirFrame *airframe = (AirFrame *) msg;
            handleLowerMsgStart(airframe);
            bufferMsg(airframe);
        }
        else
        {
            EV << "[PHY]: message undetectable while receiver disenabled -- dropping it\n";
            delete msg;
        }
    }
    else
    {
        EV << "[PHY]: listening to a different channel when receiving message -- dropping it\n";
        delete msg;
    }
}

void Ieee802154Phy::handlePrimitive(int msgkind, cMessage *msg)
{
    Ieee802154MacPhyPrimitives *primitive = check_and_cast<Ieee802154MacPhyPrimitives *>(msg);
    switch (msgkind)
    {
    case PLME_CCA_REQUEST:
        EV <<"[PHY]: this is a PLME_CCA_REQUEST" << endl;
        if (phyRadioState == phy_RX_ON)
        {
            // perform CCA, delay 8 symbols
            if (CCA_timer->isScheduled())
                error("[CCA]: received a PLME_CCA_REQUEST from MAC layer while CCA is running");
            // check if it's idle at start
            isCCAStartIdle = (rs.getState() == RadioState::IDLE);
            EV <<"[CCA]: performing CCA ..., lasting 8 symbols" << endl;
            scheduleAt(simTime() + 8.0/getRate('s'), CCA_timer);
        }
        else
        {
            EV <<"[CCA]: received a PLME_CCA_REQUEST from MAC layer while receiver is off, reporting to MAC layer" << endl;
            PLME_CCA_confirm(phyRadioState);
        }
        delete primitive;
        break;

    case PLME_ED_REQUEST:
        EV <<"[PHY]: this is a PLME_ED_REQUEST" << endl;
        if (phyRadioState == phy_RX_ON)
        {
            rxPeakPower = rxPower[getChannelNumber()];
            ASSERT(!ED_timer->isScheduled());
            scheduleAt(simTime() + 8.0/getRate('s'), ED_timer);
        }
        else
            PLME_ED_confirm(phyRadioState, 0);
        delete primitive;
        break;

    case PLME_SET_TRX_STATE_REQUEST:
        EV <<"[PHY]: this is a PLME_SET_TRX_STATE_REQUEST" << endl;
        handle_PLME_SET_TRX_STATE_request(PHYenum(primitive->getStatus()));
        delete primitive;
        break;

    case PLME_SET_REQUEST:
        EV <<"[PHY]: this is a PLME_SET_REQUEST" << endl;
        handle_PLME_SET_request(primitive);
        break;
    case PLME_GET_BITRATE:
        PLME_bitRate(rs.getBitrate());
        break;
    default:
        error("[PHY]: unknown primitive received (msgkind=%d)", msgkind);
        break;
    }
}

AirFrame* Ieee802154Phy::encapsulatePacket(cMessage *frame)
{
    //PhyControlInfo *ctrl = dynamic_cast<PhyControlInfo *>(frame->removeControlInfo());
    //ASSERT(!ctrl || ctrl->getChannelNumber()==-1); // per-packet channel switching not supported

    // Note: we don't set length() of the AirFrame, because duration will be used everywhere instead
    PhyControlInfo *ctrl = dynamic_cast<PhyControlInfo *>(frame->removeControlInfo());
    AirFrame*   airframe = new AirFrame();
    airframe->setName(frame->getName());
    airframe->setPSend(transmitterPower);
    airframe->setChannelNumber(getChannelNumber());
    airframe->setBitrate(rs.getBitrate());
    if (ctrl)
    {
        if (ctrl->getChannelNumber()>=0)
            airframe->setChannelNumber(ctrl->getChannelNumber());
        if (ctrl->getBitrate()>=0)
        {
            airframe->setBitrate(ctrl->getBitrate());
            if (rs.getBitrate()!=ctrl->getBitrate())
                rs.setBitrate(ctrl->getBitrate());
        }
        if (ctrl->getTransmitterPower()>=0)
        {
            if (ctrl->getTransmitterPower() <= (double)getChannelControlPar("pMax"))
               airframe->setPSend(ctrl->getTransmitterPower());
        }
        delete ctrl;
    }
    airframe->encapsulate(PK(frame));
    airframe->setDuration(radioModel->calculateDuration(airframe));
    airframe->setSenderPos(getRadioPosition());
    airframe->setCarrierFrequency(carrierFrequency);

    //delete ctrl;
    EV << "[PHY]: encapsulating " << frame->getName()  << " into an airframe" << endl;
    return airframe;
}

void Ieee802154Phy::handleUpperMsg(AirFrame *airframe)
{
    if (phyRadioState == phy_TX_ON)
    {
        if (par("forceIdle").boolValue())
           ASSERT(rs.getState() != RadioState::TRANSMIT);
        EV << "[PHY]: transmitter is on, start sending message ..." << endl;
        setRadioState(RadioState::TRANSMIT);
        ASSERT(txPktCopy == NULL);
        txPktCopy = (AirFrame *) airframe->dup();

        if (TxOver_timer->isScheduled())
            error("[PHY]: try to transmit a pkt whihe radio is Txing");
        scheduleAt(simTime() + airframe->getDuration(), TxOver_timer);
        EV << "[PHY]: the transmission needs " << airframe->getDuration() << " s" << endl;
        sendDown(airframe);
    }
    else
    {
        error("[PHY]: transmitter is not ON while trying to send a %s msg to channel, radio is in %d state!", airframe->getName(), phyRadioState);
        ; //send a confirm
    }
}

void Ieee802154Phy::sendUp(cMessage *msg)
{
    if (receiverConnect)
    {
        EV << "[PHY]: sending received " << msg->getName() << " frame to MAC layer" << endl;
        send(msg, upperLayerOut);
    }
    else
    {
        EV << "[PHY]: a MAC frame " << msg->getName()  << " received from the radio layer but receiver is disconnected, delete packet" << endl;
        delete msg;
    }

}

void Ieee802154Phy::sendDown(AirFrame *airframe)
{
    sendToChannel(airframe);
}

void Ieee802154Phy::bufferMsg(AirFrame *airframe) //FIXME: add explicit simtime_t atTime arg?
{
    // set timer to indicate transmission is complete
    cMessage *endRxTimer = new cMessage("endRx", PHY_RX_OVER_TIMER);
    endRxTimer->setContextPointer(airframe);
    airframe->setContextPointer(endRxTimer);

    // NOTE: use arrivalTime instead of simTime, because we might be calling this
    // function during a channel change, when we're picking up ongoing transmissions
    // on the channel -- and then the message's arrival time is in the past!
    scheduleAt(airframe->getArrivalTime() + airframe->getDuration(), endRxTimer);
}

AirFrame* Ieee802154Phy::unbufferMsg(cMessage *msg)
{
    AirFrame* airframe = (AirFrame *) msg->getContextPointer();
    //delete the self message
    delete msg;
    return airframe;
}

void Ieee802154Phy::handleLowerMsgStart(AirFrame * airframe)
{
    // Calculate the receive power of the message

    // calculate distance
    const Coord& framePos = airframe->getSenderPos();
    double distance = getRadioPosition().distance(framePos);
    if (distance<MIN_DISTANCE)
        distance = MIN_DISTANCE;

    // calculate receive power

    AirFrame * airframeExt = dynamic_cast<AirFrame * >(airframe);
    double frequency = carrierFrequency;
    if (airframeExt)
    {
        if (airframeExt->getCarrierFrequency()>0.0)
            frequency = airframeExt->getCarrierFrequency();

    }

    double rcvdPower = receptionModel->calculateReceivedPower(airframe->getPSend(), frequency, distance);
    if (obstacles && distance > MIN_DISTANCE)
        rcvdPower = obstacles->calculateReceivedPower(rcvdPower, carrierFrequency, framePos, 0, getRadioPosition(), 0);
    airframe->setPowRec(rcvdPower);

    // accumulate receive power for each pkt received in current channel, no matter real pkt or noise
    rxPower[getChannelNumber()] += rcvdPower;

    // store the receive power in the recvBuff
    recvBuff[airframe] = rcvdPower;

    if (ED_timer->isScheduled()) // all packets received during ED measurement will be discarded by MAC layer, not here
        if (rxPeakPower < rxPower[getChannelNumber()])
            rxPeakPower = rxPower[getChannelNumber()];

    // if receive power is bigger than sensitivity and if not sending
    // and currently not receiving another message and the message has
    // arrived in time
    // NOTE: a message may have arrival time in the past here when we are
    // processing ongoing transmissions during a channel change
    if (airframe->getArrivalTime() == simTime() && rcvdPower >= sensitivity && rs.getState() != RadioState::TRANSMIT && snrInfo.ptr == NULL)
    {
        EV << "[PHY]: start receiving " << airframe->getName() << " frame ...\n";

        // Put frame and related SnrList in receive buffer
        SnrList snrList;
        snrInfo.ptr = airframe;
        snrInfo.rcvdPower = rcvdPower;
        snrInfo.sList = snrList;

        // add initial snr value
        addNewSnr();
        if (rs.getState() != RadioState::RECV)
        {
            // publish new RadioState
            EV << "publish new RadioState:RECV\n";
            setRadioState(RadioState::RECV);
        }
    }
    // receive power is too low or another message is being received
    else
    {
        EV << "[PHY]: frame " << airframe->getName() << " is just noise\n";
        //add receive power to the noise level
        noiseLevel += rcvdPower;

        // if a message is being received, add a new snr value
        if (snrInfo.ptr != NULL)
        {
            // update snr info for currently being received message
            EV << "[PHY]: adding new snr value to snr list of message being received\n";
            addNewSnr();
        }

        // update the RadioState if the noiseLevel exceeded the threshold
        // and the radio is currently not in receive or in send mode
        if (noiseLevel >= sensitivity &&  rs.getState() == RadioState::IDLE)
        {
            EV << "[PHY]: noise level is high, setting radio state to RECV\n";
            setRadioState(RadioState::RECV);
        }
    }
}

void Ieee802154Phy::handleLowerMsgEnd(AirFrame * airframe)
{
    bool isCorrupt = false;
    bool isCollision = false;
    rxPower[getChannelNumber()] -= recvBuff[airframe];

    if (snrInfo.ptr == airframe)
    {
        // my receiver was turned off during reception
        if (airframe->getKind() == BITERROR_FORCE_TRX_OFF)
        {
            EV << "[PHY]: reception of r/ieee802154/Ieee802154Phy.cc line 432." << airframe->getName() << " frame failed because MAC layer turned off the receiver forcibly during the reception, drop it \n";
            noiseLevel -= recvBuff[airframe];
            isCorrupt = true;
        }
        // the sender of this pkt turned off the transmitter during transmission
        else if (airframe->getEncapsulatedPacket()->getKind() == BITERROR_FORCE_TRX_OFF)
        {
            EV << "[PHY]: reception of " << airframe->getName() << " frame failed because the sender turned off its transmitter during the tranmission, drop it \n";
            isCorrupt = true;
        }
        else
        {
            EV << "[PHY]: reception of " << airframe->getName() << " frame is over, preparing to send it to MAC layer\n";
            // get Packet and list out of the receive buffer:
            SnrList list;
            list = snrInfo.sList;
            double snirMin = list.begin()->snr;
            for (SnrList::const_iterator iter = list.begin(); iter != list.end(); iter++)
                if (iter->snr < snirMin)
                    snirMin = iter->snr;
            airframe->setSnr(10*log10(snirMin));
            if (!radioModel->isReceivedCorrectly(airframe, list))
            {
                isCollision = true;
                // we cannot do this before decapsulation, because we only detect this packet collided, not all, refer to encapsulation msg
                //airframe->getEncapsulatedMsg()->setKind(COLLISION);
            }

            // we decapsulate here to set some flag
            cMessage *frame = airframe->decapsulate();
            frame->setKind(PACKETOK);
            Radio80211aControlInfo * cinfo = new Radio80211aControlInfo;
            cinfo->setSnr(airframe->getSnr());
            cinfo->setLossRate(-1);
            cinfo->setRecPow(airframe->getPowRec());
            frame->setControlInfo(cinfo);

            if (isCollision)
                frame->setKind(COLLISION);
            else if (CCA_timer->isScheduled())  // during CCA, tell MAC layer to discard this pkt
                frame->setKind(RX_DURING_CCA);

            if (frame->getKind() == PACKETOK)
                numReceivedCorrectly++;
            else
                numGivenUp++;

            if ( (numReceivedCorrectly+numGivenUp)%50 == 0)
            {
                lossRate = (double)numGivenUp/((double)numReceivedCorrectly+(double)numGivenUp);
                emit(lossRateSignal, lossRate);
                numReceivedCorrectly = 0;
                numGivenUp = 0;
            }
            sendUp(frame);
        }

        // delete the pointer to indicate that no message is currently
        // being received and clear the list
        snrInfo.ptr = NULL;
        snrInfo.sList.clear();

        // delete the frame from the recvBuff
        recvBuff.erase(airframe);
        delete airframe;
    }
    // all other messages are noise
    else
    {
        EV << "[PHY]: reception of noise message " << airframe->getName() <<" is over, removing recvdPower from noiseLevel....\n";
        noiseLevel -= recvBuff[airframe]; // get the rcvdPower and subtract it from the noiseLevel
        recvBuff.erase(airframe);       // delete message from the recvBuff
        if (recvBuff.empty())
            noiseLevel = thermalNoise;

        // update snr info for message currently being received if any
        if (snrInfo.ptr != NULL)    addNewSnr();

        delete airframe;    // message should be deleted
    }

    // check the RadioState and update if necessary
    // change to idle if noiseLevel smaller than threshold and state was
    // not idle before
    // do not change state if currently sending or receiving a message!!!
    if (noiseLevel < sensitivity && phyRadioState == phy_RX_ON && rs.getState() == RadioState::RECV && snrInfo.ptr == NULL)
    {
        setRadioState(RadioState::IDLE);
        EV << "[PHY]: radio finishes receiving\n";
        if (newState != phy_IDLE)
        {
            newState_turnaround = newState;
            newState = phy_IDLE;
            if (newState_turnaround == phy_TRX_OFF)
            {
                phyRadioState = phy_TRX_OFF;
                setRadioState(RadioState::SLEEP);
                PLME_SET_TRX_STATE_confirm(phyRadioState);
            }
            else
            {
                // dely <aTurnaroundTime> symbols for Rx2Tx
                phyRadioState = phy_TRX_OFF;
                PLME_SET_TRX_STATE_confirm(phyRadioState);
                setRadioState(RadioState::SLEEP); // radio disabled during TRx turnaround
                if (TRX_timer->isScheduled())    cancelEvent(TRX_timer);
                scheduleAt(simTime() + aTurnaroundTime/getRate('s'), TRX_timer);
            }
        }
    }
}

void Ieee802154Phy::handleSelfMsg(cMessage *msg)
{
    switch (msg->getKind())
    {
    case PHY_RX_OVER_TIMER:     // Rx over, dynamic timer
    {
        EV << "[PHY]: frame is completely received now\n";
        AirFrame* airframe = unbufferMsg(msg);  // unbuffer the message
        handleLowerMsgEnd(airframe);
        break;
    }
    case PHY_TX_OVER_TIMER:     // Tx over
    {
        EV << "[PHY]: transmitting of " << txPktCopy->getName() << " completes!" << endl;
        if (par("forceIdle").boolValue())
            setRadioState(RadioState::IDLE);
        delete txPktCopy;
        txPktCopy = NULL;
        EV << "[PHY]: send a PD_DATA_confirm with success to MAC layer" << endl;
        PD_DATA_confirm(phy_SUCCESS,TX_OVER);//


        // process radio and channel state switch
        if (newState != phy_IDLE)
        {
            newState_turnaround = newState;
            newState = phy_IDLE;
            if (newState_turnaround == phy_TRX_OFF)
            {
                phyRadioState = phy_TRX_OFF;
                PLME_SET_TRX_STATE_confirm(phyRadioState);
                setRadioState(RadioState::SLEEP);
            }
            else
            {
                // dely <aTurnaroundTime> symbols for Rx2Tx
                phyRadioState = phy_TRX_OFF;
                PLME_SET_TRX_STATE_confirm(phyRadioState);
                setRadioState(RadioState::SLEEP);       // radio disabled during TRx turnaround
                if (TRX_timer->isScheduled())    cancelEvent(TRX_timer);
                scheduleAt(simTime() + aTurnaroundTime/getRate('s'), TRX_timer);
            }
        }
        break;
    }
    case PHY_CCA_TIMER:     // perform CCA after delay 8 symbols
    {
        if (rs.getState() ==  RadioState::IDLE && isCCAStartIdle)
        {
            EV <<"[CCA]: CCA completes, channel is IDLE, reporting to MAC layer" << endl;
            PLME_CCA_confirm(phy_IDLE);
        }
        else
        {
            EV <<"[CCA]: CCA completes, channel is BUSY, reporting to MAC layer" << endl;
            PLME_CCA_confirm(phy_BUSY);
        }
        break;
    }
    case PHY_ED_TIMER:
    {
        PLME_ED_confirm(phy_SUCCESS, calculateEnergyLevel());
        break;
    }
    case PHY_TRX_TIMER:     // TRx turnaround over
    {
        phyRadioState = newState_turnaround;
        setRadioState(RadioState::IDLE);
        PLME_SET_TRX_STATE_confirm(phyRadioState);
        PLME_SET_TRX_STATE_confirm(phy_SUCCESS);
        break;
    }
    default:
        error("[PHY]: unknown PHY timer type!");
        break;
    }
}

//*******************************************************************
// PHY primitives processing
//*******************************************************************

void Ieee802154Phy::PD_DATA_confirm(PHYenum status)
{
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PD_DATA_CONFIRM);
    primitive->setStatus(status);
    primitive->setBitRate(rs.getBitrate());
    EV << "[PHY]: sending a PD_DATA_confirm with " << status << " to MAC" << endl;
    send(primitive, upperLayerOut);
}

void Ieee802154Phy::PD_DATA_confirm(PHYenum status,short additional)
{
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PD_DATA_CONFIRM);
    primitive->setStatus(status);
    primitive->setBitRate(rs.getBitrate());
    primitive->setAdditional(additional);
    EV << "[PHY]: sending a PD_DATA_confirm with " << status << " to MAC" << endl;
    send(primitive, upperLayerOut);
}


void Ieee802154Phy::PLME_CCA_confirm(PHYenum status)
{
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_CCA_CONFIRM);
    primitive->setStatus(status);
    primitive->setBitRate(rs.getBitrate());
    EV << "[PHY]: sending a PLME_CCA_confirm with " << status << " to MAC" << endl;
    send(primitive, upperLayerOut);
}

void Ieee802154Phy::PLME_bitRate(double bitRate)
{
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_GET_BITRATE);
    primitive->setBitRate(bitRate);
    primitive->setBitRate(rs.getBitrate());
    EV << "[PHY]: sending a PLME_bitRate with " << bitRate << " to MAC" << endl;
    send(primitive, upperLayerOut);
}

void Ieee802154Phy::PLME_ED_confirm(PHYenum status, uint16_t energyLevel)
{
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_ED_CONFIRM);
    primitive->setStatus(status);
    primitive->setEnergyLevel(energyLevel);
    primitive->setBitRate(rs.getBitrate());
    EV << "[PHY]: sending a PLME_ED_confirm with " << status << " to MAC" << endl;
    send(primitive, upperLayerOut);
}

void Ieee802154Phy::PLME_SET_TRX_STATE_confirm(PHYenum status)
{
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_SET_TRX_STATE_CONFIRM);
    primitive->setStatus(status);
    primitive->setBitRate(rs.getBitrate());
    if (status == phy_SUCCESS)
    	EV << "phy_SUCCESS";
    EV << "[PHY]: sending a PLME_SET_TRX_STATE_confirm with " << status << " to MAC" << endl;
    send(primitive, upperLayerOut);
}

void Ieee802154Phy::PLME_SET_confirm(PHYenum status, PHYPIBenum attribute)
{
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_SET_CONFIRM);
    primitive->setStatus(status);
    primitive->setAttribute(attribute);
    primitive->setBitRate(rs.getBitrate());
    EV << "[PHY]: sending a PLME_SET_confirm with " << status << " and attr " <<  attribute << " to MAC" << endl;
    send(primitive, upperLayerOut);
}

void Ieee802154Phy::handle_PLME_SET_TRX_STATE_request(PHYenum setState)
{
    bool delay;
    PHYenum tmp_state;
    PHYenum curr_state = phyRadioState;

    switch (setState)
    {
    case phy_BUSY:
    	EV << "request Busy \n";
    	break;
    case phy_BUSY_RX:
    	EV << "request Busy RX\n";
    	break;
    case phy_BUSY_TX:
    	EV << "request Busy TX\n";
    	break;
    case phy_FORCE_TRX_OFF:
    	EV << "request FORCE TX OFF \n";
    	break;

    case phy_IDLE:
    	EV << "request idle \n";
    	break;

    case phy_INVALID_PARAMETER:
    	EV << "request INVALID_PARAMETER \n";
    	break;

    case phy_RX_ON:
    	EV << "request RX ON \n";
    	break;

    case phy_SUCCESS:
    	EV << "request success \n";
    	break;

    case phy_TRX_OFF:
    	EV << "request tx Off \n";
    	break;

    case phy_TX_ON:
    	EV << "request TX ON \n";
    	break;

    case phy_UNSUPPORT_ATTRIBUTE:
    	EV << "request UNSUPPORT_ATTRIBUTE \n";
        break;
    }

    //ignore any pending request
    if (newState != phy_IDLE)
        newState = phy_IDLE;
    else if (TRX_timer->isScheduled())
    {
        cancelEvent(TRX_timer);
    }

    tmp_state = curr_state;
    if (setState != curr_state) // case A: desired state is different from current state
    {
        delay = false;
        // case A1
        if (((setState == phy_RX_ON)||(setState == phy_TRX_OFF)) && rs.getState() == RadioState::TRANSMIT)
        {
            tmp_state = phy_BUSY_TX;
            newState = setState;
        }
        // case A2
        else if (((setState == phy_TX_ON)||(setState == phy_TRX_OFF)) &&  rs.getState() == RadioState::RECV)
        {
            tmp_state = phy_BUSY_RX;
            newState = setState;
        }
        // case A3
        else if (setState == phy_FORCE_TRX_OFF)
        {
            tmp_state = (curr_state == phy_TRX_OFF)? phy_TRX_OFF:phy_SUCCESS;
            phyRadioState = phy_TRX_OFF; // turn off radio immediately
            PLME_SET_TRX_STATE_confirm(phyRadioState);
            setRadioState(RadioState::SLEEP);

            // a packet is being received, force it terminated
            // We do not clear the Rx buffer here and will let the rx end timer decide what to do
            if (snrInfo.ptr != NULL)
            {
                snrInfo.ptr->setKind(BITERROR_FORCE_TRX_OFF);   //incomplete reception -- force packet discard
                noiseLevel += snrInfo.rcvdPower;    // the rest reception becomes noise
            }

            // a packet is being transmitted, force it terminated
            if ( rs.getState() == RadioState::TRANSMIT && TxOver_timer->isScheduled())
            {
                ASSERT(txPktCopy);
                txPktCopy->getEncapsulatedPacket()->setKind(BITERROR_FORCE_TRX_OFF);
                cancelEvent(TxOver_timer);
                delete txPktCopy;
                txPktCopy = NULL;
                PD_DATA_confirm(phy_TRX_OFF);
            }

            // ****************************** important! *********************************
            // phy_FORCE_TRX_OFF is usually called by MAC followed with a phy_TX_ON or phy_RX_ON,
            // since phy_FORCE_TRX_OFF will always succeed, no PLME_SET_TRX_STATE_confirm is necessary
            // if PLME_SET_TRX_STATE_confirm is sent, error will occur in trx_state_req at MAC
            return;
        }
        // case A4
        else
        {
            tmp_state = phy_SUCCESS;
            if (((setState == phy_RX_ON)&&(curr_state == phy_TX_ON))
                    ||((setState == phy_TX_ON)&&(curr_state == phy_RX_ON)))
            {
                newState_turnaround = setState;
                delay = true;
            }
            else
                // three cases:
                // curr: RX_ON && IDLE, set: TRX_OFF
                // curr: TX_ON && IDLE, set: TRX_OFF    (probability >> 0)
                // curr: TRX_OFF,   set: RX_ON or TX_ON
            {
                phyRadioState = setState;
                PLME_SET_TRX_STATE_confirm(phyRadioState);
                if (setState == phy_TRX_OFF)
                    setRadioState(RadioState::SLEEP);
                else
                    setRadioState(RadioState::IDLE);
            }
        }
        //we need to delay <aTurnaroundTime> symbols if Tx2Rx or Rx2Tx
        if (delay)
        {
            phyRadioState = phy_TRX_OFF;
            PLME_SET_TRX_STATE_confirm(phyRadioState);
            setRadioState(RadioState::SLEEP);   //should be disabled immediately (further transmission/reception will not succeed)
            scheduleAt(simTime() + aTurnaroundTime/getRate('s'), TRX_timer);
            return; // send back a confirm when turnaround finishes
        }
        else
            PLME_SET_TRX_STATE_confirm(tmp_state);
    }
    // case B: desired state already set (setState == curr_state)
    else
        PLME_SET_TRX_STATE_confirm(tmp_state);
}

void Ieee802154Phy::handle_PLME_SET_request(Ieee802154MacPhyPrimitives *primitive)
{
    PHYenum status = phy_SUCCESS;
    PHYPIBenum attribute = PHYPIBenum(primitive->getAttribute());

    switch (attribute)
    {
    case PHY_CURRENT_CHANNEL:
        // actually checking the channel number validity has been done by MAC layer before sending this primitive
        if ((primitive->getChannelNumber() < 0) || (primitive->getChannelNumber() > 26))
            error("[PHY]: channel not supported by IEEE802.15.4");

        if (!channelSupported(primitive->getChannelNumber()))
        {
            status = phy_INVALID_PARAMETER;
            break;
        }

        if (primitive->getChannelNumber() != getChannelNumber())
        {
            changeChannel(primitive->getChannelNumber());
            nb->fireChangeNotification(NF_RADIO_CHANNEL_CHANGED, &rs);
        }
        break;

        /*case PHY_CHANNELS_SUPPORTED:
            if ((primitive->getPhyChannelsSupported() & 0xf8000000) != 0)
                status = phy_INVALID_PARAMETER;
            else
            {
                rs.setPhyChannelsSupported(primitive->getPhyChannelsSupported());
                nb->fireChangeNotification(NF_CHANNELS_SUPPORTED_CHANGED, &rs);
            }
            break;

        case PHY_TRANSMIT_POWER:
            rs.setPhyTransmitPower(primitive->getPhyTransmitPower());
            nb->fireChangeNotification(NF_TRANSMIT_POWER_CHANGED, &rs);
            break;

        case PHY_CCA_MODE:
            if ((primitive->getPhyCCAMode() < 1) || (primitive->getPhyCCAMode()> 3))
                status = phy_INVALID_PARAMETER;
            else
            {
                rs.setPhyCCAMode(primitive->getPhyCCAMode());
                nb->fireChangeNotification(NF_CCA_MODE_CHANGED, &rs);
            }
            break;*/

    default:
        status = phy_UNSUPPORT_ATTRIBUTE;
        break;
    }

    PLME_SET_confirm(status, attribute);
    delete primitive;
}


void Ieee802154Phy::changeChannel(int channel)
{
    if (channel == rs.getChannelNumber())
        return;
    if (rs.getState() == RadioState::TRANSMIT)
        error("changing channel while transmitting is not allowed");

   // Clear the recvBuff
   for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
   {
        AirFrame *airframe = it->first;
        cMessage *endRxTimer = (cMessage *)airframe->getContextPointer();
        delete airframe;
        delete cancelEvent(endRxTimer);
    }
    recvBuff.clear();

    // clear snr info
    snrInfo.ptr = NULL;
    snrInfo.sList.clear();
    rxPower[getChannelNumber()] = 0; // clear power accumulator on current channel

    if (rs.getState()!=RadioState::IDLE)
        rs.setState(RadioState::IDLE);// Force radio to Idle

    // do channel switch
    EV << "Changing to channel #" << channel << "\n";

    emit(channelNumberSignal, channel);
    rs.setChannelNumber(channel);

    rxPower[channel] = 0; // clear power accumulator on new channel
    noiseLevel = thermalNoise; // reset noise level

    // do channel switch
    rs.setBitrate(getRate('b'));  // bitrate also changed
    emit(bitrateSignal, getRate('b'));

    cc->setRadioChannel(myRadioRef, rs.getChannelNumber());

    cModule *myHost = findHost();

    //cGate *radioGate = myHost->gate("radioIn");

    cGate* radioGate = this->gate("radioIn")->getPathStartGate();

    EV << "RadioGate :" << radioGate->getFullPath() << " " << radioGate->getFullName() << endl;

    // pick up ongoing transmissions on the new channel
    EV << "Picking up ongoing transmissions on new channel:\n";
    IChannelControl::TransmissionList tlAux = cc->getOngoingTransmissions(channel);
    for (IChannelControl::TransmissionList::const_iterator it = tlAux.begin(); it != tlAux.end(); ++it)
    {
    	AirFrame *airframe = check_and_cast<AirFrame *> (*it);
    	// time for the message to reach us
    	double distance = getRadioPosition().distance(airframe->getSenderPos());
    	simtime_t propagationDelay = distance / 3.0E+8;

    	// if this transmission is on our new channel and it would reach us in the future, then schedule it
    	if (channel == airframe->getChannelNumber())
    	{
    		EV << " - (" << airframe->getClassName() << ")" << airframe->getName() << ": ";
    	}

    	// if there is a message on the air which will reach us in the future
    	if (airframe->getTimestamp() + propagationDelay >= simTime())
    	{
    		EV << "will arrive in the future, scheduling it\n";

    		// we need to send to each radioIn[] gate of this host
    		//for (int i = 0; i < radioGate->size(); i++)
    		//    sendDirect(airframe->dup(), airframe->getTimestamp() + propagationDelay - simTime(), airframe->getDuration(), myHost, radioGate->getId() + i);

    		// JcM Fix: we need to this radio only. no need to send the packet to each radioIn
    		// since other radios might be not in the same channel
    		sendDirect(airframe->dup(), airframe->getTimestamp() + propagationDelay - simTime(), airframe->getDuration(), myHost, radioGate->getId() );
    	}
    	// if we hear some part of the message
    	else if (airframe->getTimestamp() + airframe->getDuration() + propagationDelay > simTime())
    	{
    		EV << "missed beginning of frame, processing it as noise\n";

    		AirFrame *frameDup = airframe->dup();
    		frameDup->setArrivalTime(airframe->getTimestamp() + propagationDelay);
    		handleLowerMsgStart(frameDup);
    		bufferMsg(frameDup);
    	}
    	else
    	{
    		EV << "in the past\n";
    	}
    }

    // notify other modules about the channel switch; and actually, radio state has changed too
    nb->fireChangeNotification(NF_RADIO_CHANNEL_CHANGED, &rs);
    nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
}


bool Ieee802154Phy::channelSupported(int channel)
{
    return ((def_phyChannelsSupported & (1 << channel)) != 0);
}

double Ieee802154Phy::getRate(char bitOrSymbol)
{
    double rate;

    if (getChannelNumber() == 0)
    {
        if (bitOrSymbol == 'b')
            rate = BR_868M;
        else
            rate = SR_868M;
    }
    else if (getChannelNumber() <= 10)
    {
        if (bitOrSymbol == 'b')
            rate = BR_915M;
        else
            rate = SR_915M;
    }
    else if (getChannelNumber() <= 26)
    {
        if (bitOrSymbol == 'b')
            rate = BR_2_4G;
        else
            rate = SR_2_4G;
    }
    else
        error("[PHY]: channel number ", getChannelNumber(), " is not supported");
    return (rate*1000);     // return bit/s
}

uint16_t Ieee802154Phy::calculateEnergyLevel()
{
    int energy;
    uint16_t t_EnergyLevel;

    //refer to sec 6.7.7 for ED implementation details
    //ED is somewhat simulation/implementation specific; here's our way:

    /* Linux floating number compatibility
    energy = (int)((rxEDPeakPower/RXThresh_)*128);
    */

    double tmpf;
    tmpf = rxPeakPower/sensitivity;
    energy = (int)(tmpf * 128);

    t_EnergyLevel = (energy > 255)?255:energy;
    return t_EnergyLevel;
}

void Ieee802154Phy::setRadioState(RadioState::State newState)
{
    if (rs.getState() != newState)
    {
        emit(radioStateSignal, newState);
        if (newState == RadioState::SLEEP)
        {
            disconnectTransceiver();
            disconnectReceiver();
        }
        else if (rs.getState() == RadioState::SLEEP)
        {
            connectTransceiver();
            connectReceiver();
            if (rs.getState() == newState)
            {
                rs.setState(newState);
                nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
                return;
            }
        }
    }
    rs.setState(newState);
    nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
}

void Ieee802154Phy::addNewSnr()
{
    SnrListEntry listEntry;     // create a new entry
    listEntry.time = simTime();
    listEntry.snr = snrInfo.rcvdPower / noiseLevel;
    snrInfo.sList.push_back(listEntry);
}

// TODO: change the parent to AbstractRadioExtended and remove this methods
void Ieee802154Phy::updateDisplayString() {
    // draw the interference area and sensitivity area
    // according pathloss propagation only
    // we use the channel controller method to calculate interference distance
    // it should be the methods provided by propagation models, but to
    // avoid a big modification, we reuse those methods.

    if (!ev.isGUI() || !drawCoverage) // nothing to do
        return;
    if (myRadioRef) {
    	cDisplayString& d = hostModule->getDisplayString();

    	// communication area (up to sensitivity)
    	// FIXME this overrides the ranges if more than one radio is present is a host
    	double sensitivity_limit = cc->getInterferenceRange(myRadioRef);
    	d.removeTag("r1");
    	d.insertTag("r1");
    	d.setTagArg("r1",0,(long) sensitivity_limit);
    	d.setTagArg("r1",2,"gray");
    	d.removeTag("r2");
    	d.insertTag("r2");
    	d.setTagArg("r2",0,(long) calcDistFreeSpace());
    	d.setTagArg("r2",2,"blue");
    }
    if (updateString==NULL && updateStringInterval>0)
    	updateString = new cMessage("refress timer");
    if (updateStringInterval>0)
        scheduleAt(simTime()+updateStringInterval,updateString);
}

double Ieee802154Phy::calcDistFreeSpace()
{
    //the carrier frequency used
    double carrierFrequency = getChannelControlPar("carrierFrequency");
    //signal attenuation threshold
    //path loss coefficient
    double alpha = getChannelControlPar("alpha");

    double waveLength = (SPEED_OF_LIGHT / carrierFrequency);
    //minimum power level to be able to physically receive a signal
    double minReceivePower = sensitivity;

    double interfDistance = pow(waveLength * waveLength * transmitterPower /
                         (16.0 * M_PI * M_PI * minReceivePower), 1.0 / alpha);
    return interfDistance;
}

void Ieee802154Phy::disconnectReceiver()
{
    receiverConnect = false;
    cc->disableReception(this->myRadioRef);
    // Clear the recvBuff
    for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
    {
         AirFrame *airframe = it->first;
         cMessage *endRxTimer = (cMessage *)airframe->getContextPointer();
         delete airframe;
         delete cancelEvent(endRxTimer);
     }
     recvBuff.clear();

     // clear snr info
     snrInfo.ptr = NULL;
     snrInfo.sList.clear();
     rxPower[getChannelNumber()] = 0; // clear power accumulator on current channel
}

void Ieee802154Phy::connectReceiver()
{
   cc->enableReception(this->myRadioRef);
   receiverConnect = true;
   if (rs.getState()!=RadioState::IDLE)
        rs.setState(RadioState::IDLE);// Force radio to Idle

    rxPower[getChannelNumber()] = 0; // clear power accumulator on new channel
    noiseLevel = thermalNoise; // reset noise level

    // do channel switch
    rs.setBitrate(getRate('b'));  // bitrate also changed
    emit(bitrateSignal, getRate('b'));

    cc->setRadioChannel(myRadioRef, rs.getChannelNumber());

    cModule *myHost = findHost();

    //cGate *radioGate = myHost->gate("radioIn");

    cGate* radioGate = this->gate("radioIn")->getPathStartGate();

    EV << "RadioGate :" << radioGate->getFullPath() << " " << radioGate->getFullName() << endl;

    // pick up ongoing transmissions on the new channel
    EV << "Picking up ongoing transmissions on new channel:\n";
    IChannelControl::TransmissionList tlAux = cc->getOngoingTransmissions(getChannelNumber());
    for (IChannelControl::TransmissionList::const_iterator it = tlAux.begin(); it != tlAux.end(); ++it)
    {
        AirFrame *airframe = check_and_cast<AirFrame *> (*it);
        // time for the message to reach us
        double distance = getRadioPosition().distance(airframe->getSenderPos());
        simtime_t propagationDelay = distance / 3.0E+8;


        // if there is a message on the air which will reach us in the future
        if (airframe->getTimestamp() + propagationDelay >= simTime())
        {
            // if this transmission is on our new channel and it would reach us in the future, then schedule it
            EV << " - (" << airframe->getClassName() << ")" << airframe->getName() << ": ";
            EV << "will arrive in the future, scheduling it\n";

            // we need to send to each radioIn[] gate of this host
            //for (int i = 0; i < radioGate->size(); i++)
            //    sendDirect(airframe->dup(), airframe->getTimestamp() + propagationDelay - simTime(), airframe->getDuration(), myHost, radioGate->getId() + i);

            // JcM Fix: we need to this radio only. no need to send the packet to each radioIn
            // since other radios might be not in the same channel
            sendDirect(airframe->dup(), airframe->getTimestamp() + propagationDelay - simTime(), airframe->getDuration(), myHost, radioGate->getId() );
        }
        // if we hear some part of the message
        else if (airframe->getTimestamp() + airframe->getDuration() + propagationDelay > simTime())
        {
            EV << "missed beginning of frame, processing it as noise\n";

            AirFrame *frameDup = airframe->dup();
            frameDup->setArrivalTime(airframe->getTimestamp() + propagationDelay);
            handleLowerMsgStart(frameDup);
            bufferMsg(frameDup);
        }
    }
    // notify other modules about the channel switch; and actually, radio state has changed too
    nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
}
