
#include "Ieee80216Radio.h"
//#include "omnetpp.h"

Define_Module(Ieee80216Radio);

void Ieee80216Radio::initialize(int stage)
{
    Radio::initialize(stage);

    if (stage == 3 && !par("isInput").boolValue())
    {
    	disconnectReceiver();
    }
    isCollision = false;
    collisions = 0;
}

void Ieee80216Radio::finish()
{
    recordScalar("# of collisions", collisions);
}

void Ieee80216Radio::handleLowerMsgStart(AirFrame *airframe)
{
    // Calculate the receive power of the message
    EV << "(in Ieee80216Radio::handleLowerMsgStart) " << airframe->getClassName() <<
        airframe->getName() << " eingetroffen.\n";
    // calculate distance
    const Coord& myPos = getRadioPosition();
    const Coord& framePos = airframe->getSenderPos();
    double distance = myPos.distance(framePos);

    // calculate receive power
    //receptionModel->setAbweichung(abweichung);
    double rcvdPower =
        receptionModel->calculateReceivedPower(airframe->getPSend(), carrierFrequency, distance);

    // store the receive power in the recvBuff
    recvBuff[airframe] = rcvdPower;

    // if receive power is bigger than sensitivity and if not sending
    // and currently not receiving another message and the message has
    // arrived in time
    // NOTE: a message may have arrival time in the past here when we are
    // processing ongoing transmissions during a channel change
    ev << "\n\nrcvdPower: " << rcvdPower << "    sensitivity: " << sensitivity << "\n";
    ev << "radio state: " << rs.getState() << "   snrptr: " << snrInfo.ptr << "\n\n";

    if (airframe->getArrivalTime() == simTime() && rcvdPower >= sensitivity
        && rs.getState() != RadioState::TRANSMIT && snrInfo.ptr == NULL)
    {
        EV << "receiving frame " << airframe->getName() << endl;

        // Put frame and related SnrList in receive buffer
        SnrList snrList;
        snrInfo.ptr = airframe;
        snrInfo.rcvdPower = rcvdPower;
        snrInfo.sList = snrList;

        // add initial snr value
        addNewSnr();

        //char buf[90];
        //sprintf(buf, "rcvdPower: %f", snrInfo.rcvdPower);
        //displayString().setTagArg("t",0,buf);
        ev << "Achtung RS Radio rcvdPower:" << rcvdPower << " SNR:" <<
            log10(snrInfo.rcvdPower / noiseLevel) << ".\n";
        if (rs.getState() != RadioState::RECV)
        {
            // publish new RadioState
            EV << "publish new RadioState:RECV\n";
            setRadioState(RadioState::RECV);
        }
    }
    // receive power is too low or another message is being sent or received
    else
    {
        EV << "frame " << airframe->getName() << " is just noise\n";
        //add receive power to the noise level
        noiseLevel += rcvdPower;

        // if a message is being received add a new snr value
        if (snrInfo.ptr != NULL)
        {
            // update snr info for currently being received message
            EV << "adding new snr value to snr list of message being received\n";
            addNewSnr();
        }

        // update the RadioState if the noiseLevel exceeded the threshold
        // and the radio is currently not in receive or in send mode
        if (noiseLevel >= sensitivity && rs.getState() == RadioState::IDLE)
        {
            EV << "noiselevel > sensivity && state=IDLE\n";
            EV << "setting radio state to RECV\n";
            setRadioState(RadioState::RECV);
        }
    }
}

void Ieee80216Radio::handleLowerMsgEnd(AirFrame *airframe)
{
    EV << "(in Ieee80216Radio::handleLowerMsgEnd) " << airframe->getClassName() <<
        airframe->getName() << " eingetroffen.\n";
    // check if message has to be send to the decider
    if (snrInfo.ptr == airframe)
    {
        EV << "reception of frame over, preparing to send packet to upper layer\n";
        // get Packet and list out of the receive buffer:
        SnrList list;
        list = snrInfo.sList;

        double rcvdPower = snrInfo.rcvdPower;

        // delete the pointer to indicate that no message is currently
        // being received and clear the list
        snrInfo.ptr = NULL;
        snrInfo.sList.clear();

        // delete the frame from the recvBuff
        recvBuff.erase(airframe);

        //XXX send up the frame:
        //if (radioModel->isReceivedCorrectly(airframe, list))
        //    sendUp(airframe);
        //else
        //    delete airframe;
        if (getRadioPosition().distance(airframe->getSenderPos())<0.000001)
        {
            delete airframe;
        }
        else
        {
            if (!radioModel->isReceivedCorrectly(airframe, list))
            {
                airframe->getEncapsulatedPacket()->setKind(list.size() > 1 ? COLLISION : BITERROR);
                airframe->setName(list.size() > 1 ? "COLLISION" : "BITERROR");
                isCollision = true;
                collisions++;
            }
            /*if(isCollision == false)
               sendUp(airframe, list);
               if(isCollision == true)
               {
               isCollision=false;
               delete airframe;
               } */
            sendUp(airframe, list, rcvdPower);
        }
    }
    // all other messages are noise
    else
    {
        EV << "reception of noise message over, removing recvdPower from noiseLevel....\n";
        // get the rcvdPower and subtract it from the noiseLevel
        noiseLevel -= recvBuff[airframe];

        // delete message from the recvBuff
        recvBuff.erase(airframe);

        // update snr info for message currently being received if any
        if (snrInfo.ptr != NULL)
        {
            addNewSnr();
        }

        // message should be deleted
        delete airframe;
        EV << "message deleted\n";
    }

    // check the RadioState and update if necessary
    // change to idle if noiseLevel smaller than threshold and state was
    // not idle before
    // do not change state if currently sending or receiving a message!!!
    if (noiseLevel < sensitivity && rs.getState() == RadioState::RECV && snrInfo.ptr == NULL)
    {
        // publish the new RadioState:
        EV << "new RadioState is IDLE\n";
        setRadioState(RadioState::IDLE);
    }
}

void Ieee80216Radio::sendUp(AirFrame *airframe, SnrList list, double rcvdPower)
{
    EV << "(in Ieee80216Radio::sendUp) " << airframe->getClassName() << airframe->getName() <<
        " eingetroffen.\n";
    const Coord& myPos = getRadioPosition();
    const Coord& framePos = airframe->getSenderPos();
    double distance = myPos.distance(framePos);
    //double rcvdPower = receptionModel->calculateReceivedPower(airframe->getPSend(), carrierFrequency, distance);
    //double sendPower= airframe->getPSend();
    cMessage *frame = airframe->decapsulate();
    delete airframe;

    // berechnet mittleren SNR eines AirFrame über die gesamte Bit-Anzahl
    //double snrMin = list.begin()->snr;
    double snrGes = 0;
    double snrMit = 0;
    int Zaehler = 0;
    for (SnrList::const_iterator iter = list.begin(); iter != list.end(); iter++)
    {
        //if (iter->snr < snrMin)
        //    snrMin = iter->snr;
        ++Zaehler;
        EV << "Radio Module iter" << Zaehler << " snr:" << iter->snr << "\n";
        snrGes = snrGes + iter->snr;
    }
    snrMit = snrGes / Zaehler;
    EV << "Radio Module sntMit" << snrMit << " snr Ges:" << snrGes << " Z�hler:" << Zaehler << "\n";

    Ieee80216MacHeader *macFrame = dynamic_cast<Ieee80216MacHeader *>(frame); // Empfangendes Paket ist eine IEEE802.16e Frame
    if (macFrame)
    {
        macFrame->setSNR(10 * log10(snrMit));
        macFrame->setRcvdPower(rcvdPower);
        macFrame->setThermNoise(noiseLevel);
        macFrame->setAbstand(distance);
        macFrame->setXPos(myPos.x);
        macFrame->setYPos(myPos.y);
    }
    EV << "sending up frame " << frame->getName() << endl;
    send(frame, upperLayerOut);
}

SnrList Ieee80216Radio::getSNRlist()
{
    return snrInfo.sList;
}
