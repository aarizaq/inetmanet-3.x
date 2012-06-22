/* -*- mode:c++ -*- ********************************************************
 * file:        Decider80216.cc
 *
 * author:      Marc Loebbers, Andreas Koepke
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#include "Decider80216.h"

//#include "AirFrame_m.h"
//#include "FWMath.h"

Define_Module(Decider80216);

void Decider80216::initialize(int stage)
{
    BasicModule::initialize(stage);
    if (stage == 0)
    {
        snrThresholdLevel = FWMath::dBm2mW(par("snrThresholdLevel"));
        upperLayerOut = findGate("upperLayerOut");
        lowerLayerIn = findGate("lowerLayerIn");
        numRcvd = 0;
        numSentUp = 0;
        WATCH(numRcvd);
        WATCH(numSentUp);
    }
}

/**
 *  Checks the received sduList (from the PhySDU header) if it contains an
 *  snr level above the threshold
 */
bool Decider80216::snrOverThreshold(SnrList& snrlist) const
{
    //check the entries in the sduList if a level is lower than the
    //acceptable minimum:

    //check
    for (SnrList::const_iterator iter = snrlist.begin(); iter != snrlist.end(); iter++)
    {
        if (iter->snr <= snrThresholdLevel)
        {
            EV << "Message got lost. MessageSnr: " << iter->snr << " Threshold: " <<
                snrThresholdLevel << endl;
            return false;
        }
    }
    return true;
}

void Decider80216::getSnrList(AirFrame* af, SnrList& receivedList)
{
#if 0
    SnrListEntry listEntry;
    listEntry = receivedList.front();

    ChannelControl *cc;
    ChannelControl::HostRef myHostRef;
    cModule *hostModule = findHost();

    cc = dynamic_cast<ChannelControl*>(simulation.getModuleByPath("channelcontrol"));
    myHostRef = cc->lookupHost(hostModule);

    const Coord& myPos = cc->getHostPosition(myHostRef);
    const Coord& framePos = af->getSenderPos();
    double distance = myPos.distance(framePos);
    EV << "Nachricht ";
    EV << "mit dem Time: " << listEntry.time << ", ";
    EV << "mit der Distanz: " << distance << ", ";
    EV << "mit dem SNR: " << listEntry.snr << "\n";

    for (cModule::SubmoduleIterator iter(subParent); !iter.end(); iter++)
    {
        ev << "Roland Parent Module: " << iter()->getFullName() << "\n";
    }

    cModule* parent = getParentModule(); //Uebergeordnetes Module
    cModule* subParent = parent->getParentModule(); //Übergeordnetes Module

    cModule* plane = subParent->getSubmodule("controlPlane");
    //ControlPlaneBase *ControlPlane = check_and_cast<ControlPlaneBase *> (plane);

    //if (ControlPlane)
    //{
    //ControlPlane->setSNR(listEntry.snr);
    //ControlPlane->setDistance(distance);
    //}
#endif
}

void Decider80216::handleLowerMsg(AirFrame* af, SnrList& receivedList)
{
    getSnrList(af, receivedList);
    if (snrOverThreshold(receivedList))
    {
        EV << "Message handed on to Mac\n";

        sendUp(af);
    }
    else
    {
        delete af;
    }
}


/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * The decider module only handles messages from lower layers. All
 * messages from upper layers are directly passed to the snrEval layer
 * and cannot be processed in the decider module
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleLowerMsg, handleSelfMsg
 */
void Decider80216::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == lowerLayerIn)
    {
        numRcvd++;

        //remove the control info from the AirFrame
        SnrControlInfo *cInfo = static_cast<SnrControlInfo *>(msg->removeControlInfo());

        // read in the snrList from the control info
        handleLowerMsg(check_and_cast<AirFrame *>(msg), cInfo->getSnrList());

        // delete the control info
        delete cInfo;

    }
    else if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
    }
}


/**
 * Decapsulate and send message to the upper layer.
 *
 * to be called within @ref handleLowerMsg.
 */
void Decider80216::sendUp(AirFrame * frame)
{
    numSentUp++;
    cPacket *macMsg = frame->decapsulate();
    send(macMsg, upperLayerOut);
    EV << "sending up msg " << frame->getName() << endl;
    delete frame;
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * In this function it has to be decided whether this message got lost
 * or not. This can be done with a simple SNR threshold or with
 * transformations of SNR into bit error probabilities...
 *
 * If you want to forward the message to upper layers please use @ref
 * sendUp which will decapsulate the MAC frame before sending
 */

