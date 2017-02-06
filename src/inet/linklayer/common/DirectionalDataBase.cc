//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include <sstream>

#include "inet/common/LayeredProtocolBase.h"
#include "inet/linklayer/common/DirectionalDataBase.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"

using namespace inet::physicallayer;

#include "inet/common/ModuleAccess.h"

#ifdef WITH_IDEALWIRELESS
#include "inet/linklayer/ideal/IdealMacFrame_m.h"
#endif // ifdef WITH_IDEALWIRELESS


#ifdef WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#endif // ifdef WITH_IEEE80211

#ifdef WITH_CSMA
#include "inet/linklayer/csma/CSMAFrame_m.h"
#endif // ifdef WITH_CSMA

#ifdef WITH_LMAC
#include "inet/linklayer/lmac/LMacFrame_m.h"
#endif // ifdef WITH_LMAC

#ifdef WITH_BMAC
#include "inet/linklayer/bmac/BMacFrame_m.h"
#endif // ifdef WITH_BMAC

#ifdef WITH_XMAC
#include "inet/linklayer/xmac/XMacPkt_m.h"
#endif // ifdef WITH_XMAC

namespace inet {

Define_Module(DirectionalDataBase);

DirectionalDataBase::DirectionalDataBase()
{
}

DirectionalDataBase::~DirectionalDataBase()
{
    dataMap.clear();
}

void DirectionalDataBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // get a pointer to the host module
        cModule *mod = getContainingNode(this);
        mod->subscribe(LayeredProtocolBase::packetSentToUpperSignal,this);
    }
}

void DirectionalDataBase::updateDisplayString()
{
    if (!hasGUI())
        return;
}

void DirectionalDataBase::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void DirectionalDataBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    // nothing needed here at the moment
    Enter_Method_Silent();
    if (LayeredProtocolBase::packetSentToUpperSignal != signalID)
    return;

    cPacket * frame = dynamic_cast<cPacket*>(obj);


    if (frame)
    {

        EV_DETAIL << "Received directional signal" << endl;
        // XXX: This is a hack for supporting both IdealMac and Ieee80211Mac. etc
        MACAddress addr;
#ifdef WITH_IEEE80211
        if (dynamic_cast<ieee80211::Ieee80211TwoAddressFrame *>(frame))
        {
            ieee80211::Ieee80211TwoAddressFrame *f = dynamic_cast<ieee80211::Ieee80211TwoAddressFrame *>(frame);
            addr = f->getTransmitterAddress();
        }
#endif // ifdef WITH_IEEE80211
#ifdef WITH_IDEALWIRELESS
        if (dynamic_cast<IdealMacFrame *>(frame))
        {
            IdealMacFrame *f = dynamic_cast<IdealMacFrame *>(frame);
            addr = f->getSrc();
        }
#endif // ifdef WITH_IDEALWIRELESS
#ifdef WITH_CSMA
        if (dynamic_cast<CSMAFrame *>(frame))
        {
            CSMAFrame *f = dynamic_cast<CSMAFrame *>(frame);
            addr = f->getSrcAddr();
        }
#endif // ifdef WITH_CSMA
#ifdef WITH_LMAC
        if (dynamic_cast<LMacFrame *>(frame))
        {
            LMacFrame *f = dynamic_cast<LMacFrame *>(frame);
            addr = f->getSrcAddr();
        }
#endif // ifdef WITH_LMAC
#ifdef WITH_BMAC
        if (dynamic_cast<BMacFrame *>(frame))
        {
            BMacFrame *f = dynamic_cast<BMacFrame *>(frame);
            addr = f->getSrcAddr();
        }
#endif // ifdef WITH_BMAC
#ifdef WITH_XMAC
        if (dynamic_cast<XMacPkt *>(frame))
        {
            XMacPkt *f = dynamic_cast<XMacPkt *>(frame);
            addr = f->getSrcAddr();
        }
#endif // ifdef WITH_XMAC

        ReceptionIndication * indication = dynamic_cast<ReceptionIndication *>(frame->getControlInfo());
        if (indication && !addr.isUnspecified() && !addr.isBroadcast() && !addr.isMulticast()) {
            setDirection(addr,indication->getDirection());
        }
    }
}

//---

void DirectionalDataBase::setDirection(const MACAddress &addr,const EulerAngles &angle) {
    Data auxData;
    auxData.angle = angle;
    auxData.time = simTime();
    auto it = dataMap.find(addr);
    if (it == dataMap.end()) {
        DataVector auxVec;
        auxVec.push_back(auxData);
        dataMap.insert(std::make_pair(addr,auxVec));
    }
    else
    {
        it->second.push_back(auxData);
        if (it->second.size() > maxDataSize)
            it->second.pop_front();
    }
}

EulerAngles DirectionalDataBase::getDirection(const MACAddress &addr)
{
    auto it = dataMap.find(addr);
    if (it == dataMap.end())
        return -1;
    return it->second.back().angle;
}


} // namespace inet
