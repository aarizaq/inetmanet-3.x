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
 * @file I3.cc
 * @author Antonio Zea
 */

#include "UDPSocket.h"

#include <IPvXAddressResolver.h>
#include <CommonMessages_m.h>
#include <GlobalNodeListAccess.h>
#include <UnderlayConfiguratorAccess.h>

#include <omnetpp.h>
#include <OverlayKey.h>
#include "SHA1.h"

#include "TriggerTable.h"
#include "I3Identifier.h"
#include "I3IPAddress.h"
#include "I3SubIdentifier.h"
#include "I3IdentifierStack.h"
#include "I3Trigger.h"
#include "I3Message_m.h"
#include "I3Message.h"
#include "I3.h"

using namespace std;

std::ostream& operator<<(std::ostream& os, const I3TriggerSet &t)
{
    os << endl;

    I3TriggerSet::iterator it;
    for (it = t.begin(); it != t.end(); it++) {
        os << *it << " in " << it->getInsertionTime() << "(" <<
        (simTime() - it->getInsertionTime()) << ")" << endl;
    }
    return os;
}

Define_Module(I3);

const I3Identifier *I3::findClosestMatch(const I3Identifier &t) const
{
    int prevDist = 0, nextDist = 0;


    if (triggerTable.size() == 0) return 0;

    /* find the closest identifier to t */
    /* if no match exists, it gets the next identifier with a bigger key */
    I3TriggerTable::const_iterator it = triggerTable.lower_bound(t);

    if (it == triggerTable.end()) {
        it--;               // if at the end, check last
    }

    if (it->first == t) return &it->first; // if we found an exact match, no need to bother

    if (it != triggerTable.begin()) { // if no smaller keys, begin() is the candidate itself

        /* no exact match, check which is closer: */
        /* either where the iterator points to (the next biggest) or the previous one. */
        /* see I3Identifier::distanceTo for distance definition */

        nextDist = it->first.distanceTo(t);
        it--;
        prevDist = it->first.distanceTo(t);

        // if the next one is closest, put iterator back in place
        if (nextDist < prevDist) {
            it++;
        }
    }

    /* now check if they match in the I3 sense (first prefixLength bits) */
    return (it->first.isMatch(t)) ? &it->first : 0;
}

void I3::insertTrigger(I3Trigger &t)
{

    if (t.getIdentifierStack().size() == 0) {
        /* don't bother */
        cout << "Warning: Got trigger " << t << " with size 0 in " << thisNode.getIp()<< endl;
        return;
    }

    t.setInsertionTime(simTime());

    /* insert packet in triggerTable; */
    /* if it was already there, remove and insert updated copy */

    triggerTable[t.getIdentifier()].erase(t);
    triggerTable[t.getIdentifier()].insert(t);

    updateTriggerTableString();
}

void I3::removeTrigger(I3Trigger &t)
{

    //cout << "Removing trigger at " << getId() << endl;
    //getParentModule()->getParentModule()->bubble("Removing trigger");

    if (triggerTable.count(t.getIdentifier()) == 0) return;

    set<I3Trigger> &s = triggerTable[t.getIdentifier()];

    s.erase(t);

    if (s.size() == 0) triggerTable.erase(t.getIdentifier());

    updateTriggerTableString();
}

void I3::sendToNode(I3SendPacketMessage *imsg)
{
    I3IPAddress address;
    /* re-route message to a client node */
    address = imsg->getIdentifierStack().peek().getIPAddress();
    imsg->getIdentifierStack().pop(); // pop ip address
    sendMessageToUDP(address, imsg);
}

void I3::sendPacket(I3SendPacketMessage *msg)
{

    I3IdentifierStack &idStack = msg->getIdentifierStack();

    if (idStack.size() == 0) {
        /* no identifiers left! drop packet */
        /* shouldn't happen (how'd it get here anyway?) */
        numDroppedPackets++;
        byteDroppedPackets += msg->getBitLength();
        delete msg;
        return;
    }

    I3SubIdentifier id = idStack.peek();

    if (id.getType() == I3SubIdentifier::IPAddress) {
        /* shouldn't happen (how'd they find us anyway?) but just in case */
        sendToNode(msg);
    } else {

        /* if we were asked to reply, send it now */
        if (msg->getSendReply()) {
            sendQueryReply(id.getIdentifier(), msg->getSource());
        }

        const I3Identifier *i3id = findClosestMatch(id.getIdentifier());

        /* eliminate top of the stack */
        idStack.pop();

        if (!i3id) {
            /* no matching ids found in this server, re-route to next id */
            if (idStack.size() == 0) {
                /* no identifiers left! drop packet */
                numDroppedPackets++;
                byteDroppedPackets += msg->getBitLength();
                cout << "Dropped packet at" << thisNode.getIp()<< " to unknown id " << id.getIdentifier() << endl;
                delete msg;
                return;
            } else {
                msg->setBitLength(SEND_PACKET_L(msg)); /* stack size changed, recalculate */
                if (idStack.peek().getType() == I3SubIdentifier::IPAddress) {
                    msg->getMatchedTrigger().clear(); // not trigger, but direct IP match
                    sendToNode(msg);
                } else {
                    OverlayKey key = idStack.peek().getIdentifier().asOverlayKey();
                    callRoute(key, msg);
                }
            }

        } else {
            /* some id found, send to all friends */
            set<I3Trigger> &s = triggerTable[*i3id];
            set<I3Trigger>::iterator it;

            for (it = s.begin(); it != s.end(); it++) {
                I3SendPacketMessage *newMsg;
                cPacket *dupMsg;

                newMsg = new I3SendPacketMessage();
                newMsg->setIdentifierStack(idStack); /* create copy */
                newMsg->getIdentifierStack().push(it->getIdentifierStack()); /* append our stuff to the top of the stack */
                dupMsg = check_and_cast<cPacket*>(msg->getEncapsulatedPacket()->dup()); /* dup msg */
                newMsg->setBitLength(SEND_PACKET_L(newMsg)); /* stack size changed, recalculate */
                newMsg->encapsulate(dupMsg);

                I3SubIdentifier &top = newMsg->getIdentifierStack().peek();

                if (top.getType() == I3SubIdentifier::IPAddress) {
                    newMsg->setMatchedTrigger(*it);
                    sendToNode(newMsg);
                } else {
                    OverlayKey key = top.getIdentifier().asOverlayKey();
                    callRoute(key, newMsg);
                }
            }

            /* copies sent, erase original */
            delete msg;

        }
    }
}

void I3::forward(OverlayKey *key, cPacket **msg, NodeHandle *hint)
{
    numForwardedPackets++;
    numForwardedBytes += (*msg)->getByteLength();

    BaseApp::forward(key, msg, hint);
}


void I3::handleUDPMessage(cMessage *msg)
{
    I3Message *i3msg;

    i3msg = dynamic_cast<I3Message*>(msg);

    if (!i3msg) {
        delete msg;
        return;
    }

    OverlayKey key;

    msg->removeControlInfo();
    switch (i3msg->getType()) {
    case INSERT_TRIGGER:
        I3InsertTriggerMessage *imsg;

        imsg = check_and_cast<I3InsertTriggerMessage*>(i3msg);
        key = imsg->getTrigger().getIdentifier().asOverlayKey();
        callRoute(key, imsg);

        /*            if ((imsg->getSource().address.d[0] & 0xff) == 56) {
                    cout << "UDP Server " << thisNode.getIp()<< " trigger " << imsg->getTrigger() << " key " << key << endl;
            }*/

        break;
    case REMOVE_TRIGGER:
        I3RemoveTriggerMessage *rmsg;

        rmsg = check_and_cast<I3RemoveTriggerMessage*>(i3msg);
        key = rmsg->getTrigger().getIdentifier().asOverlayKey();
        callRoute(key, rmsg);

        break;
    case SEND_PACKET:
    {
        I3SendPacketMessage *smsg;

        smsg = check_and_cast<I3SendPacketMessage*>(i3msg);
        if (smsg->getIdentifierStack().size() == 0) {
            /* got an empty identifier stack - nothing to do */
            delete msg;
            return;
        }
        I3SubIdentifier &subId = smsg->getIdentifierStack().peek();
        if (subId.getType() == I3SubIdentifier::IPAddress) {
            /* why didn't they send it directly?! */
            sendToNode(smsg);
        } else {
            key = subId.getIdentifier().asOverlayKey();
            callRoute(key, smsg);
        }
        break;
    }
    default:
        /* should't happen */
        delete msg;
        break;
    }
}

void I3::sendQueryReply(const I3Identifier &id, const I3IPAddress &add) {
    I3QueryReplyMessage *pmsg;
    I3IPAddress myAddress(thisNode.getIp(), par("serverPort"));

    pmsg = new I3QueryReplyMessage();
    pmsg->setSource(myAddress);
    pmsg->setSendingTime(simTime());
    pmsg->setIdentifier(id);
    pmsg->setBitLength(QUERY_REPLY_L(pmsg));
    sendMessageToUDP(add, pmsg);
}

void I3::deliver(OverlayKey& key, cMessage* msg)
{
    I3Message *i3msg;

    i3msg = dynamic_cast<I3Message*>(msg);
    if (!i3msg) {
        cout << "Delivered non I3 Message!" << endl;
        delete msg;
        return;
    }

    switch (i3msg->getType()) {
    case INSERT_TRIGGER:
        I3InsertTriggerMessage *imsg;

        imsg = check_and_cast<I3InsertTriggerMessage*>(i3msg);
        insertTrigger(imsg->getTrigger());

        if (imsg->getSendReply()) {
            sendQueryReply(imsg->getTrigger().getIdentifier(), imsg->getSource());
        }

        delete msg;
        break;
    case REMOVE_TRIGGER:
        I3RemoveTriggerMessage *rmsg;

        rmsg = check_and_cast<I3RemoveTriggerMessage*>(i3msg);
        removeTrigger(rmsg->getTrigger());
        delete msg;
        break;
    case SEND_PACKET:
        I3SendPacketMessage *smsg;

        smsg = check_and_cast<I3SendPacketMessage*>(i3msg);
        sendPacket(smsg);

        break;
    default:
        delete msg;
        break;
    }
}

int I3::numInitStages() const
{
    return MIN_STAGE_APP + 1;
}

void I3::initializeApp(int stage)
{
    if (stage != MIN_STAGE_APP)
        return;

    numDroppedPackets = 0;
    WATCH(numDroppedPackets);
    byteDroppedPackets = 0;
    WATCH(byteDroppedPackets);

    numForwardedPackets = 0;
    numForwardedBytes = 0;

    WATCH(numForwardedPackets);
    WATCH(numForwardedBytes);

    triggerTimeToLive = par("triggerTimeToLive");
    WATCH(triggerTimeToLive);

    expirationTimer = new cMessage("expiration timer");
    scheduleAt(simTime() + triggerTimeToLive, expirationTimer);

    getDisplayString() = "i=i3";

    thisNode.setPort(par("serverPort").longValue());
    bindToPort(thisNode.getPort());

}

void I3::handleTimerEvent(cMessage* msg)
{
    bool updateString = false;

    if (msg == expirationTimer) {
        scheduleAt(simTime() + triggerTimeToLive, expirationTimer);

        for (I3TriggerTable::iterator it = triggerTable.begin(); it != triggerTable.end(); it++) {
            set<I3Trigger> &triggerSet = it->second;
            for (set<I3Trigger>::const_iterator sit = triggerSet.begin(); sit != triggerSet.end(); sit++) {
                //cout << "Trigger " << *sit << " has
                if (simTime() - sit->getInsertionTime() > triggerTimeToLive) {
                    //if ((bool)par("debugOutput")) {
                    //	cout << "Erasing trigger " << *sit << " in " <<
                    //		thisNode.getIp()<< ", insertion time is " << sit->getInsertionTime()<< endl;
                    //}
                    triggerSet.erase(sit);
                    updateString = true;
                }
            }
            if (it->second.size() == 0) {
                triggerTable.erase(it);
            }
        }
        if (updateString) updateTriggerTableString();
    } else delete msg;
}

void I3::updateTriggerTableString()
{
    TriggerTable *table = check_and_cast<TriggerTable*>(getParentModule()->getSubmodule("triggerTable"));
    table->updateDisplayString();
}

I3TriggerTable &I3::getTriggerTable()
{
    return triggerTable;
}

void I3::finish()
{
    recordScalar("I3 Packets dropped", numDroppedPackets);
    recordScalar("I3 Bytes dropped", byteDroppedPackets);
}
