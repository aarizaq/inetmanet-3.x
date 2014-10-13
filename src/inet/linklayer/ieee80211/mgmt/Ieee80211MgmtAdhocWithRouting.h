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

#ifndef IEEE80211_MGMT_ADHOC_H
#define IEEE80211_MGMT_ADHOC_H

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"
#include <map>
#include <deque>
#include "inet/routing/extras/base/ManetRoutingBase.h"

namespace inet {

namespace ieee80211 {

/**
 * Used in 802.11 ad-hoc mode. See corresponding NED file for a detailed description.
 * This implementation ignores many details.
 *
 * @author Andras Varga
 */
class INET_API Ieee80211MgmtAdhocWithRouting : public Ieee80211MgmtBase
{

    typedef std::deque<uint16_t> SeqNumberVector;
    typedef std::map<uint64_t, SeqNumberVector> SeqNumberInfo;
    SeqNumberInfo mySeqNumberInfo;
    uint16_t mySeqNumber;
    inetmanet::ManetRoutingBase *routingModule;
    int maxTTL;



  protected:
    virtual void actualizeReactive(cPacket *pkt,bool out);
    virtual void handleRoutingMessage(cPacket*);
    virtual bool forwardMessage(Ieee80211DataFrame *frame);
    bool macLabelBasedSend(Ieee80211DataFrame *frame);
    Ieee80211DataFrame *encapsulate(cPacket *msg,MACAddress dest);
    virtual void handleMessage(cMessage *msg);
    virtual void startRouting();

    virtual int numInitStages() const {return NUM_INIT_STAGES;}
    virtual void initialize(int);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleUpperMessage(cPacket *msg);

    /** Implements abstract Ieee80211MgmtBase method -- throws an error (no commands supported) */
    virtual void handleCommand(int msgkind, cObject *ctrl);

    /** Utility function for handleUpperMessage() */
    virtual Ieee80211DataFrame *encapsulate(cPacket *msg);

    /** @name Processing of different frame types */
    //@{
    virtual void handleDataFrame(Ieee80211DataFrame *frame);
    virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame);
    virtual void handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame);
    virtual void handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame);
    virtual void handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame);
    virtual void handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame);
    virtual void handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame);
    virtual void handleDisassociationFrame(Ieee80211DisassociationFrame *frame);
    virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame);
    virtual void handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame);
    virtual void handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame);
    virtual cPacket *decapsulate(Ieee80211DataFrame *frame);
    //@}
};

}
}
#endif


