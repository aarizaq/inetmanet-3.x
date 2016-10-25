/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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

#ifndef __INET_ETHERENCAPMESH_H
#define __INET_ETHERENCAPMESH_H

#include <stdio.h>
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ethernet/EtherMACBase.h"

/**
 * Performs Ethernet II encapsulation/decapsulation. More info in the NED file.
 */
namespace inet {

class INET_API EtherEncapMesh : public EtherEncap
{
  protected:
    long totalFromWifi;
    long totalToWifi;
    virtual void initialize() override;
    virtual void refreshDisplay() const override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void processFrameFromMAC(EtherFrame *msg) override;
    virtual void processFrameFromWifiMesh(ieee80211::Ieee80211Frame *);
};

}
#endif


