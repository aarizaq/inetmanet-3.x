//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Author: Benjamin Seregi
//

#ifndef __INET_MPDUGEN_H
#define __INET_MPDUGEN_H

#include "inet/common/INETDefs.h"
#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {
/*
 * A very simple MPDU generator class.
 */
class MpduGen : public ApplicationBase
{
protected:
    int localPort = -1, destPort = -1;
    UDPSocket socket;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

    cMessage *selfMsg = nullptr;
    int numSent = 0;
    int numReceived = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void sendPackets();
    void processPacket(cPacket *pk);
    virtual void handleMessageWhenUp(cMessage* msg) override;

  public:
    MpduGen() {}
    ~MpduGen() { cancelAndDelete(selfMsg); }
};

}

#endif
