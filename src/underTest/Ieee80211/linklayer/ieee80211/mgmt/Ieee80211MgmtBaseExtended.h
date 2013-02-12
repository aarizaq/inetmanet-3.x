//
// Copyright (C) 2006 Juan-Carlos Maureira
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

#ifndef IEEE80211_MGMT_BASE_H
#define IEEE80211_MGMT_BASE_H

#include <omnetpp.h>
#include "MACAddress.h"
#include "PassiveQueueBase.h"
#include "NotificationBoard.h"
#include "Ieee80211Frame_m.h"
#include "Ieee80211MgmtFrames_m.h"
#include "IInterfaceTable.h"


/**
 * Abstract base class for 802.11 management components.
 * Performs queueing for MAC, and dispatching incoming frames by frame type.
 * Also keeps some simple statistics (frame counts).
 * Provides all the basic functionality to implement an Managed, Master or
 * WDS management module.
 *
 * @author Juan-Carlos Maureira
 * based on the code from Andras Varga
 */
class INET_API Ieee80211MgmtBaseExtended : public PassiveQueueBase, public INotifiable
{
  protected:
    // configuration
    int frameCapacity;
    MACAddress myAddress;
    InterfaceEntry * myEntry;

    // state
    cQueue dataQueue; // queue for data frames
    cQueue mgmtQueue; // queue for management frames (higher priority than data frames)

    // statistics
    long numDataFramesReceived;
    long numMgmtFramesReceived;
    long numMgmtFramesDropped;
    long numDataFramesDropped;

    // queue statistics
    cOutVector dataQueueLenVec;
    cOutVector dataQueueDropVec;

  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    /** Dispatches incoming messages to handleTimer(), handleUpperMessage() or processFrame(). */
    virtual void handleMessage(cMessage *msg);

    /** Can be redefined to deal with self-messages */
    virtual void handleTimer(cMessage *frame) {return;};

    /** Can be redefined to encapsulate and enqueue msgs from higher layers */
    virtual void handleUpperMessage(cPacket *msg);

    /** Can be redefined to handle commands from the "agent" (if present) */
    virtual void handleCommand(int msgkind, cPolymorphic *ctrl) {return;};

    /** Utility method for implementing handleUpperMessage(): gives the message to PassiveQueueBase */
    virtual void sendOrEnqueue(cPacket *frame);

    /** Redefined from PassiveQueueBase. */
    virtual cMessage * enqueue(cMessage *msg);

    /** Redefined from PassiveQueueBase. */
    virtual cMessage *dequeue();

    /** Redefined from PassiveQueueBase. */
    virtual bool isEmpty();

    /** Redefined from PassiveQueueBase: send message to MAC */
    virtual void sendOut(cMessage *msg);

    /** Utility method to dispose of an unhandled frame, and log the drop operation */
    virtual void dropFrame(Ieee80211Frame *frame);

    /** Utility method to decapsulate a data frame */
    virtual cPacket *decapsulate(Ieee80211DataFrame *frame);

    /** Utility method to encapsulate a packet. It must be redefined by each Mgmt **/
    virtual Ieee80211DataFrame* encapsulate(cPacket* pkt) = 0;

    /** Utility method: sends the packet to the upper layer */
    virtual void sendUp(cMessage *msg);

    /** Dispatch to frame processing methods according to frame type */
    virtual void processFrame(Ieee80211DataOrMgmtFrame *frame);

    /** @name Processing of different frame types */
    //@{
    virtual void handleDataFrame(Ieee80211DataFrame *frame) {return;};
    virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame) {return;};
    virtual void handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame) {return;};
    virtual void handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame) {return;};
    virtual void handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame) {return;};
    virtual void handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame) {return;};
    virtual void handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame) {return;};
    virtual void handleDisassociationFrame(Ieee80211DisassociationFrame *frame) {return;};
    virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame) {return;};
    virtual void handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame) {return;};
    virtual void handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame) {return;};
    //@}
};

#endif


