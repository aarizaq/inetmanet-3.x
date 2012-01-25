//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file I3.h
 * @author Antonio Zea
 */

#ifndef __I3_H__
#define __I3_H__

#include "I3Trigger.h"
#include "I3Identifier.h"
#include <omnetpp.h>
#include <OverlayKey.h>
#include "I3Message.h"
#include <BaseApp.h>

/** Simple wrapper aroung set<I3Trigger> to implement string stream operations. */
class I3TriggerSet : public std::set<I3Trigger>
{
    friend std::ostream& operator<<(std::ostream& os, const I3TriggerSet &t);
};

typedef std::map< I3Identifier, I3TriggerSet > I3TriggerTable;

/** Main Omnet module for the implementation of Internet Indirection Infrastructure.
 *
 */

class I3 : public BaseApp
{
protected:
    /** Number of dropped packets */
    int numDroppedPackets;
    int byteDroppedPackets;

    int numForwardedPackets;
    int numForwardedBytes;

    /** Time before inserted triggers expire */
    int triggerTimeToLive;

    /** Table containing inserted triggers */
    I3TriggerTable triggerTable;

    /** Timer to check for trigger expiration */
    cMessage *expirationTimer;

    /** Returns number of required init stages */
    int numInitStages() const;

    /** Actual initialization function
         * @param stage Actual stage
         */
    virtual void initializeApp(int stage);

    /** Delivers a packet from the overlay
      * @param key Key from the overlay
      * @param msg Message to deliver
      */
    virtual void deliver(OverlayKey &key, cMessage *msg);

    /** Handles a message from UDP
         * @param msg Incoming message
         */
    virtual void handleUDPMessage(cMessage *msg);

    /** Handles timers
     * @param msg Timer
     */
    virtual void handleTimerEvent(cMessage* msg);

    /** Replies to a query of which server is responsible for the given identifier (this server)
         * @param id I3 identifier of the query
         * @param add IP address of requester
         */
    void sendQueryReply(const I3Identifier &id, const I3IPAddress &add);

    virtual void forward(OverlayKey *key, cPacket **msg, NodeHandle* nextHopNode);

    /** Updates TriggerTable's module display string */
    void updateTriggerTableString();

    virtual void finish();

public:
    /** Returns the table of inserted triggers */
    I3TriggerTable &getTriggerTable();

    /** Finds the closest match to t from the stored trigger identifiers.
         * Note that, in the case that there are many biggest prefix matches, it
         * will only return the first one (bug! - couldn't find efficient way to do)
         * @param t Identifier to be matchesd
         * @return Pointer to the biggest prefix match, or NULL if none was found
        */
    const I3Identifier *findClosestMatch(const I3Identifier &t) const;

    /** Inserts a trigger into I3
         * @param t Trigger to be inserted
        */
    void insertTrigger(I3Trigger &t);

    /** Removes a trigger from I3
         * @param t Trigger to be removed
        */
    void removeTrigger(I3Trigger &t);

    /** Sends a packet through I3.
         * It checks the trigger table for triggers matching the message's first I3SubIdentifier from the stack.
         * If none are found, the subidentifier is dropped and the packet is routed based on the next remaining one
         * (if no subidentifiers remain, the packet itself is dropped).
         * The matching trigger's own subidentifier stack is then appended to the message stack, and then
         * routed to the first subidentifier of the resulting stack (or sent through UDP if it's an IP address).
         * @param msg Message to be sent
        */
    void sendPacket(I3SendPacketMessage *msg);

    /** Sends packet to matching IP address (used by sendPacket)
     * @param imsg Message to be sent
    */
    void sendToNode(I3SendPacketMessage *imsg);

};

#endif
