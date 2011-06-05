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
    BasicDecider::initialize(stage);

    if (stage == 0)
    {
        snrThresholdLevel = FWMath::dBm2mW(par("snrThresholdLevel"));
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
