//
// Copyright (C) 2006 Andras Varga
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

#ifndef SECURITYIEEE80211_MGMT_BASE_H
#define SECURITYIEEE80211_MGMT_BASE_H

#include "inet/common/INETDefs.h"

#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/queue/PassiveQueueBase.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h"
#include "inet/securityModule/message/securityPkt_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"
#include "inet/securityModule/SecurityWPA2.h"

/**
 * Abstract base class for 802.11 infrastructure mode management components.
 * Performs queueing for MAC, and dispatching incoming frames by frame type.
 * Also keeps some simple statistics (frame counts).
 *
 * @author Andras Varga
 *
 */

namespace inet {

namespace ieee80211 {


class INET_API SecurityIeee80211MgmtBase : public Ieee80211MgmtBase
{
  protected:
        bool hasSecurity;
    virtual void initialize(int);

    /** Dispatches incoming messages to handleTimer(), handleUpperMessage() or processFrame(). */
    virtual void handleMessage(cMessage *msg);

    /** Redefined from PassiveQueueBase: send message to MAC */
    virtual void sendOut(cMessage *msg);
    virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame) = 0;
    virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame) = 0;
    virtual void handleCCMPFrame(CCMPFrame *frame);
    //@}
};
}
}
#endif
