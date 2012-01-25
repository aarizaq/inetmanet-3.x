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
 * @file I3BaseApp.cc
 * @author Antonio Zea
 */

#include <omnetpp.h>
#include <IPvXAddressResolver.h>
#include <GlobalNodeListAccess.h>
#include <InitStages.h>
#include <NotificationBoard.h>
#include <UnderlayConfigurator.h>
#include <UDPControlInfo_m.h>
#include <NodeHandle.h>
#include <BootstrapList.h>

#include "I3Trigger.h"
#include "I3IdentifierStack.h"
#include "I3Message.h"
#include "I3BaseApp.h"

#include <iostream>
#include <sstream>
#include <cfloat>

using namespace std;


I3BaseApp::I3CachedServer::I3CachedServer() :
        lastReply(0),
        roundTripTime(MAXTIME)
{
}

std::ostream &operator<<(std::ostream &os, const I3BaseApp::I3CachedServer &server) {
    os << server.address << " rt=" << server.roundTripTime;
    return os;
}



I3BaseApp::I3BaseApp()
{
}

I3BaseApp::~I3BaseApp()
{
}

int I3BaseApp::numInitStages() const
{
    return MAX_STAGE_APP + 1;
}

void I3BaseApp::initialize(int stage)
{
    if (stage != MIN_STAGE_APP) return;

    nodeIPAddress = IPvXAddressResolver().addressOf(getParentModule());
    socket.setOutputGate(gate("udpOut"));
    socket.bind(par("clientPort").longValue());

    /*    NotificationBoardAccess().get()->subscribe(this, NF_HOSTPOSITION_BEFOREUPDATE);
        NotificationBoardAccess().get()->subscribe(this, NF_HOSTPOSITION_UPDATED);*/

    getDisplayString().setTagArg("i", 0, "i3c");
    getParentModule()->getDisplayString().removeTag("i2");

    if (int(par("bootstrapTime")) >= int(par("initTime"))) {
        opp_error("Parameter bootstrapTime must be smaller than initTime");
    }

    bootstrapTimer = new cMessage();
    scheduleAt(simTime() + int(par("bootstrapTime")), bootstrapTimer);

    initializeTimer = new cMessage();
    scheduleAt(simTime() + int(par("initTime")), initializeTimer);

    numSent = 0;
    sentBytes = 0;
    numReceived = 0;
    receivedBytes = 0;
    numIsolations = 0;
    mobilityInStages = false;

    WATCH(nodeIPAddress);
    WATCH(numSent);
    WATCH(sentBytes);
    WATCH(numReceived);
    WATCH(receivedBytes);
    WATCH(numIsolations);


    WATCH_SET(insertedTriggers);
    WATCH(gateway);
    WATCH_MAP(samplingCache);
    WATCH_MAP(identifierCache);

    initializeApp(stage);
}

void I3BaseApp::initializeApp(int stage)
{
}

void I3BaseApp::bootstrapI3()
{
    I3IPAddress myAddress(nodeIPAddress, par("clientPort"));

    // TODO: use BootstrapList instead of GlobalNodeList
    const NodeHandle handle = GlobalNodeListAccess().get()->getBootstrapNode();
    gateway.address = I3IPAddress(handle.getIp(), par("serverPort"));

    int cacheSize = par("cacheSize");
    for (int i = 0; i < cacheSize; i++) {
        I3Identifier id;

        id.createRandomKey();

        ostringstream os;
        os << myAddress << " sample" << i;
        id.setName(os.str());

        samplingCache[id] = I3CachedServer(); // placeholder

        insertTrigger(id, false);
    }

    refreshTriggersTimer = new cMessage();
    refreshTriggersTime = par("triggerRefreshTime");
    scheduleAt(simTime() + truncnormal(refreshTriggersTime, refreshTriggersTime / 10),
               refreshTriggersTimer);

    refreshSamplesTimer = new cMessage();
    refreshSamplesTime = par("sampleRefreshTime");
    scheduleAt(simTime() + truncnormal(refreshSamplesTime, refreshSamplesTime / 10),
               refreshSamplesTimer);
}

void I3BaseApp::initializeI3()
{

}

void I3BaseApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == bootstrapTimer) {
            bootstrapI3();
            delete msg;
            bootstrapTimer = 0;
        } else if (msg == initializeTimer)  {
            initializeI3();
            delete msg;
            initializeTimer = 0;
        } else if (msg == refreshTriggersTimer) {
            refreshTriggers();
            scheduleAt(simTime() + truncnormal(refreshTriggersTime, refreshTriggersTime / 10),
                       refreshTriggersTimer);
        } else if (msg == refreshSamplesTimer) {
            refreshSamples();
            scheduleAt(simTime() + truncnormal(refreshSamplesTime, refreshSamplesTime / 10),
                       refreshSamplesTimer);
        } else {
            handleTimerEvent(msg);
        }
    } else if (msg->arrivedOn("udpIn")) {
        handleUDPMessage(msg);
    } else {
        delete msg;
    }
}

void I3BaseApp::deliver(I3Trigger &matchingTrigger, I3IdentifierStack &stack, cPacket *msg)
{
    delete msg;
}

void I3BaseApp::handleTimerEvent(cMessage *msg)
{
    delete msg;
}

void I3BaseApp::handleUDPMessage(cMessage *msg)
{
    I3Message *i3msg;

    i3msg = dynamic_cast<I3Message*>(msg);
    if (i3msg) {
        switch (i3msg->getType()) {
        case SEND_PACKET:
        {
            I3SendPacketMessage *smsg;

            smsg = check_and_cast<I3SendPacketMessage*>(msg);
            numReceived++;
            receivedBytes += smsg->getByteLength();

            /* deliver to app */
            cPacket *newMessage = smsg->decapsulate();
            deliver(smsg->getMatchedTrigger(), smsg->getIdentifierStack(), newMessage);

            break;
        }
        case QUERY_REPLY:
        {
            I3QueryReplyMessage *pmsg;
            pmsg = check_and_cast<I3QueryReplyMessage*>(msg);
            I3Identifier &id = pmsg->getIdentifier();

            identifierCache[id].address = pmsg->getSource();
            identifierCache[id].lastReply = simTime();
            identifierCache[id].roundTripTime = simTime() - pmsg->getSendingTime();

            if (samplingCache.count(id) != 0) {
                samplingCache[id] = identifierCache[id];
            }
            break;
        }
        default:
            /* shouldn't get here */
            break;
        }
    }
    delete msg;
}

void I3BaseApp::sendToI3(I3Message *msg)
{
    sendThroughUDP(msg, gateway.address);
}

void I3BaseApp::sendThroughUDP(cMessage *msg, const I3IPAddress &add)
{
    msg->removeControlInfo();
    socket.sendTo(PK(msg), add.getIp(), add.getPort());
    send(msg, "udpOut");
}

void I3BaseApp::refreshTriggers()
{
    I3IPAddress myAddress(nodeIPAddress, par("clientPort"));
    map<I3Identifier, I3CachedServer>::iterator mit;


    // pick fastest I3 server as gateway
    int serverTimeout = par("serverTimeout");
    gateway.roundTripTime = serverTimeout;
    I3Identifier gatewayId;
    for (mit = samplingCache.begin(); mit != samplingCache.end(); mit++) {
        if (gateway.roundTripTime > mit->second.roundTripTime) {
            gatewayId = mit->first;
            gateway = mit->second;
        }
    }

    // check if gateway has timeout'ed
    if (simTime() - gateway.lastReply >= serverTimeout) {
        // We have a small problem here: if the fastest server has timeout,
        // that means the previous gateway had stopped responding some time before and no triggers were refreshed.
        // Since all servers have timeout'ed by now and we can't trust return times, pick a random server and hope that one is alive.
        int random = intrand(samplingCache.size()), i;

        EV << "I3BaseApp::refreshTriggers()]\n"
           << "    Gateway timeout at " << nodeIPAddress
           << ", time " << simTime()
           << "; expired gateway is " << gateway << "(" << gatewayId << ") "
           << " with last reply at " << gateway.lastReply
           << endl;

        for (i = 0, mit = samplingCache.begin(); i < random; i++, mit++);
        gateway = mit->second;
        EV << "I3BaseApp::refreshTriggers()]\n"
           << "    New gateway for " << nodeIPAddress << " is " << gateway
           << endl;

        if (gateway.roundTripTime > 2 * serverTimeout) {
            EV << "I3BaseApp::refreshTriggers()]\n"
               << "    New gateway's (" << gateway << ") rtt for " << nodeIPAddress
               << " too high... marking as isolated!"
               << endl;
            numIsolations++;
            const NodeHandle handle = GlobalNodeListAccess().get()->getBootstrapNode();
            gateway.address = I3IPAddress(handle.getIp(), par("serverPort"));
        }
    }

    /* ping gateway */
    insertTrigger(gatewayId, false);
//    cout << "Client " << nodeIPAddress << " pings " << gatewayId << endl;

    /* reinsert stored triggers */
    set<I3Trigger>::iterator it;
    for (it = insertedTriggers.begin(); it != insertedTriggers.end(); it++) {
        insertTrigger(*it, false);
    }

    /* now that we are refreshing stuff, might as well erase old identifier cache entries */
    int idStoreTime = par("idStoreTime");
    for (mit = identifierCache.begin(); mit != identifierCache.end(); mit++) {
        if (mit->second.lastReply - simTime() > idStoreTime) {
            identifierCache.erase(mit);
        }
    }

}

void I3BaseApp::refreshSamples() {
    map<I3Identifier, I3CachedServer>::iterator mit;

    EV << "I3BaseApp::refreshSamples()]\n"
       << "    Refresh samples!"
       << endl;
    /* reinsert sample triggers */
    for (mit = samplingCache.begin(); mit != samplingCache.end(); mit++) {
        insertTrigger(mit->first, false);
    }
}

I3Identifier I3BaseApp::retrieveClosestIdentifier()
{
    simtime_t time;
    I3Identifier id;
    map<I3Identifier, I3CachedServer>::iterator mit;
    I3IPAddress myAddress(nodeIPAddress, par("clientPort"));

    time = MAXTIME;
    for (mit = samplingCache.begin(); mit != samplingCache.end(); mit++) {
        if (time > mit->second.roundTripTime) {
            time = mit->second.roundTripTime;
            id = mit->first;
        }
    }
    samplingCache.erase(id);

    I3Identifier rid;
    rid.createRandomKey();

    ostringstream os;
    os << myAddress << " sample";
    rid.setName(os.str());

    samplingCache[rid] = I3CachedServer(); // placeholder
    insertTrigger(rid, false);

    return id;
}

void I3BaseApp::sendPacket(const I3Identifier &id, cPacket *msg, bool useHint)
{
    I3IdentifierStack stack;

    stack.push(id);
    sendPacket(stack, msg, useHint);
}

void I3BaseApp::sendPacket(const I3IdentifierStack &stack, cPacket *msg, bool useHint)
{
    I3SendPacketMessage *smsg;

    smsg = new I3SendPacketMessage();
    smsg->setBitLength(SEND_PACKET_L(smsg));
    smsg->encapsulate(msg);
    smsg->setIdentifierStack(stack);

    smsg->setSendReply(useHint);
    if (useHint) {
        I3IPAddress add(nodeIPAddress, par("clientPort"));
        smsg->setSource(add);
    }

    numSent++;
    sentBytes += smsg->getByteLength();

    I3SubIdentifier subid = stack.peek(); // first check where the packet should go
    if (subid.getType() == I3SubIdentifier::IPAddress) { // if it's an IP address
        smsg->getIdentifierStack().pop(); // pop it
        sendThroughUDP(smsg, subid.getIPAddress()); // and send directly to host
    } else { // else if it's an identifier
        // check if we have the I3 server cached
        I3IPAddress address = (useHint && identifierCache.count(subid.getIdentifier()) != 0) ?
                              identifierCache[subid.getIdentifier()].address :
                              gateway.address;
        sendThroughUDP(smsg, address); // send it directly
    }
}

void I3BaseApp::insertTrigger(const I3Identifier &identifier, bool store)
{
    I3Trigger trigger;
    I3IPAddress add(nodeIPAddress, par("clientPort"));;

    trigger.getIdentifierStack().push(add);
    trigger.setIdentifier(identifier);
    insertTrigger(trigger, store);
}

void I3BaseApp::insertTrigger(const I3Identifier &identifier, const I3IdentifierStack &stack, bool store)
{
    I3Trigger trigger;

    trigger.setIdentifier(identifier);
    trigger.getIdentifierStack() = stack;
    insertTrigger(trigger, store);
}

void I3BaseApp::insertTrigger(const I3Trigger &t, bool store) {

    if (store) {
        if (insertedTriggers.count(t) != 0) return;
        insertedTriggers.insert(t);
    }

    I3InsertTriggerMessage *msg = new I3InsertTriggerMessage();
    I3IPAddress myAddress(nodeIPAddress, par("clientPort"));

    msg->setTrigger(t);
    msg->setSendReply(true);
    msg->setSource(myAddress);
    msg->setBitLength(INSERT_TRIGGER_L(msg));

    sendThroughUDP(msg, gateway.address);
}

void I3BaseApp::removeTrigger(const I3Identifier &identifier)
{
    I3Trigger dummy;
    dummy.setIdentifier(identifier);

    set<I3Trigger>::iterator it = insertedTriggers.lower_bound(dummy);
    if (it == insertedTriggers.end()) return; /* no matches */

    for (; it != insertedTriggers.end() && it->getIdentifier() == identifier; it++) {
        removeTrigger(*it);
    }
}

void I3BaseApp::removeTrigger(const I3Trigger &t)
{
    I3RemoveTriggerMessage *msg = new I3RemoveTriggerMessage();
    msg->setTrigger(t);
    msg->setBitLength(REMOVE_TRIGGER_L(msg));
    sendThroughUDP(msg, gateway.address);

    insertedTriggers.erase(t);
}

set<I3Trigger> &I3BaseApp::getInsertedTriggers()
{
    return insertedTriggers;
}

void I3BaseApp::receiveChangeNotification (int category, const cObject *details)
{
    Enter_Method_Silent();

    /* Mobility is happening (the only event we are subscribed to). We have two things to do:
    * 1) Insert triggers with new IP
    * 2) Delete triggers with old IP
    * If it's one staged mobility, we just get told the IP after it's changed, and we need to make sure
    * step 1 and 2 are done. If it's two staged mobility, we need to make sure we do step 1 first and then
    * step 2. */

//     if (!mobilityInStages) { /* if the flag isn't set, mobility isn't done in stages or this is stage 1 */
//         if (category == NF_HOSTPOSITION_BEFOREUPDATE) {
//             mobilityInStages = true; /* set the flag so we don't land here in stage 2 again */
//         }
//         /* do part 1! */
//         cMessage *msg = check_and_cast<cMessage*>(details);
//         IPvXAddress *ipAddr = (IPvXAddress*)msg->getContextPointer();
//
//         ostringstream os;
//         os << "Mobility first stage - actual IP is " << nodeIPAddress << ", future IP is " << *ipAddr << endl;
//         getParentModule()->bubble(os.str().c_str());
//
//         std::cout << "In advance from " << nodeIPAddress << " to " << *ipAddr << endl;
//         I3IPAddress oldAddress(nodeIPAddress, par("clientPort"));
//         I3IPAddress newAddress(*ipAddr, par("clientPort"));
//
//         delete ipAddr;
//         delete msg;
//
//         for (set<I3Trigger>::iterator it = insertedTriggers.begin(); it != insertedTriggers.end(); it++) {
//             I3Trigger trigger(*it); /* create copy */
//             trigger.getIdentifierStack().replaceAddress(oldAddress, newAddress); /* replace old address with new */
//             insertTrigger(trigger, false); /* insert trigger in I3, but don't store it in our list yet - that's done in part 2 */
//         }
//
//         doMobilityEvent(I3_MOBILITY_BEFORE_UPDATE);
//     }
//     if (category == NF_HOSTPOSITION_UPDATED) { /* part 2: both for 1-stage and stage 2 of 2-stage mobility */
//         I3IPAddress oldAddress(nodeIPAddress, par("clientPort"));
//         nodeIPAddress = IPAddressResolver().addressOf(getParentModule()).get4();
//         I3IPAddress newAddress(nodeIPAddress, par("clientPort"));
//
//         cout << "After from " << oldAddress << " to " << newAddress << endl;
//
//         ostringstream os;
//         os << "Mobility second stage - setting IP as " << newAddress << endl;
//         getParentModule()->bubble(os.str().c_str());
//
//         set<I3Trigger> newSet; /* list of new triggers (that we already inserted in I3 in stage 1) */
//
//         for (set<I3Trigger>::iterator it = insertedTriggers.begin(); it != insertedTriggers.end(); it++) {
//             I3Trigger trigger(*it); /* create copy */
//
//             trigger.getIdentifierStack().replaceAddress(oldAddress, newAddress); /* replace old address with new */
//             newSet.insert(trigger); /* insert in new list */
//
//             removeTrigger(*it);  /* remove trigger from I3 and out list */
//
//         }
//         insertedTriggers = newSet; /* replace old list with updated one */
//
//         mobilityInStages = false; /* reset variable */
//         refreshTriggers(); /* to get new trigger round-trip times, new cache list */
// 	refreshSamples();
//
//         doMobilityEvent(I3_MOBILITY_UPDATED);
//     }
}

void I3BaseApp::doMobilityEvent(I3MobilityStage category)
{
}
