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
 * @file I3HostMobility.cc
 * @author Antonio Zea
 */

#include "I3BaseApp.h"
#include "I3.h"

#define NUM_PARTNERS 5

using namespace std;

enum MsgType {
    MSG_TIMER,
    MSG_TIMER_RESET_ID,
    MSG_TIMER_REDISCOVER,
    MSG_QUERY_ID,
    MSG_REPLY_ID,
    MSG_PING,
    MSG_REPLY
};

struct MessageContent {
    int id;
    I3Identifier source;
};

class I3HostMobility : public I3BaseApp {
    bool checkedPartners;

    int numSentPackets;
    std::set<int> packets;
    std::vector<I3Identifier> partners;
    I3Identifier poolId;
    I3Identifier closestId;

    void initializeApp(int stage);
    void initializeI3();
    void handleTimerEvent(cMessage *msg);
    void handleUDPMessage(cMessage* msg);
    void deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg);
    void doMobilityEvent(I3MobilityStage stage);
    void discoverPartners();
    void finish();
};

Define_Module(I3HostMobility);


void I3HostMobility::initializeApp(int stage) {
    numSentPackets = 0;

    I3BaseApp::initializeApp(stage);
}

void I3HostMobility::finish() {
    recordScalar("Number of sent packets", numSentPackets);
    recordScalar("Number of lost packets", packets.size());
}

void I3HostMobility::initializeI3() {
    checkedPartners = false;

    poolId.createFromHash("HostMobility");
    poolId.setName("HostMobility");
    poolId.createRandomSuffix();
    insertTrigger(poolId);

    closestId = retrieveClosestIdentifier();
    insertTrigger(closestId);

    cMessage *msg = new cMessage();
    msg->setKind(MSG_TIMER);
    scheduleAt(simTime() + 5, msg);

    cMessage *nmsg = new cMessage();
    msg->setKind(MSG_TIMER_REDISCOVER);
    scheduleAt(simTime() + 60, nmsg);
}

void I3HostMobility::handleUDPMessage(cMessage *msg) {
    I3BaseApp::handleUDPMessage(msg);
}

void I3HostMobility::deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg)
{
    MessageContent *mc = (MessageContent*)msg->getContextPointer();
    switch (msg->getKind()) {
    case MSG_QUERY_ID:
    {
        I3Identifier otherId = mc->source;
        mc->source = closestId;
        msg->setKind(MSG_REPLY_ID);
        sendPacket(otherId, msg);
        break;
    }
    case MSG_REPLY_ID:
        partners.push_back(mc->source);
        delete mc;
        delete msg;
        break;
    case MSG_PING:
        msg->setKind(MSG_REPLY);
        sendPacket(mc->source, msg);
        break;
    case MSG_REPLY:
        packets.erase(mc->id);
        delete mc;
        delete msg;
        break;
    default:
        break;
    }
}

void I3HostMobility::handleTimerEvent(cMessage *msg) {
    switch (msg->getKind()) {
    case MSG_TIMER:
        if (checkedPartners) {
            // partners.size() != NUM_PARTNERS in the unlikely event
            // that not all id queries have returned
            //if (partners.size() == 0) {
            //	opp_error("Wtf?!");
            //}
            for (unsigned int i = 0; i < partners.size(); i++) {
                cPacket *cmsg = new cPacket();
                MessageContent *mc = new MessageContent();

                packets.insert(numSentPackets);
                mc->id = numSentPackets++;
                mc->source = closestId;
                cmsg->setContextPointer(mc);
                cmsg->setKind(MSG_PING);
                cmsg->setBitLength((32 + intrand(512)) * 8);

                sendPacket(partners[i], cmsg, true);
            }
            scheduleAt(simTime() + truncnormal(0.5, 0.1), msg);
        } else {
            discoverPartners();
            scheduleAt(simTime() + 10, msg);
        }
        break;
    case MSG_TIMER_RESET_ID:
        closestId = retrieveClosestIdentifier();
        insertTrigger(closestId);
        delete msg;
        break;
    case MSG_TIMER_REDISCOVER:
        checkedPartners = false;
        scheduleAt(simTime() + 60, msg);
    default:
        break;
    }
}

void I3HostMobility::discoverPartners()
{
    partners.clear();
    for (int i = 0; i < NUM_PARTNERS; i++) {
    	cPacket *cmsg = new cPacket();
        MessageContent *mc = new MessageContent();
        I3Identifier partner;

        mc->source = closestId;
        mc->id = -1;
        cmsg->setContextPointer(mc);
        cmsg->setKind(MSG_QUERY_ID);

        partner.createFromHash("HostMobility");
        partner.createRandomSuffix();
        sendPacket(partner, cmsg);
    }
    checkedPartners = true;
}

void I3HostMobility::doMobilityEvent(I3MobilityStage stage) {
    if (stage == I3_MOBILITY_UPDATED) {
        cMessage *msg = new cMessage();
        msg->setKind(MSG_TIMER_RESET_ID);
        scheduleAt(simTime() + 10, msg);
    }
}

