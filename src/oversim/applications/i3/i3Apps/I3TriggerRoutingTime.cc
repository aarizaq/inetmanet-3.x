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
 * @file I3TriggerRoutingTime.cc
 * @author Antonio Zea
 */

#include "I3BaseApp.h"
#include "I3.h"

using namespace std;

static cStdDev stats;
static bool statsDumped = false;

class I3TRTServer : public I3 {

    void initializeApp(int stage);
    void deliver(OverlayKey &key, cMessage *msg);
    void finish();
};

Define_Module(I3TRTServer);

class I3TRTClient : public I3BaseApp {
    void initializeI3();
    void handleTimerEvent(cMessage *msg);
};

Define_Module(I3TRTClient);


void I3TRTServer::initializeApp(int stage) {
    statsDumped = false;
    I3::initializeApp(stage);
}

void I3TRTServer::deliver(OverlayKey &key, cMessage *msg) {
    I3InsertTriggerMessage *i3msg;

    i3msg = dynamic_cast<I3InsertTriggerMessage*>(msg);
    if (i3msg) {
        simtime_t *pt = (simtime_t*)i3msg->getContextPointer();
        if (pt) {
            stats.collect(simTime() - *pt);
            //cout << "Trigger reach time " << simTime() - *pt << endl;
            delete pt;
            i3msg->setContextPointer(0);
        }
    }
    I3::deliver(key, msg);
}

void I3TRTServer::finish() {
    if (!statsDumped) {
        statsDumped = true;
        recordScalar("I3Sim Number of samples", stats.getCount());
        recordScalar("I3Sim Min time", stats.getMin());
        recordScalar("I3Sim Max time", stats.getMax());
        recordScalar("I3Sim Mean time", stats.getMean());
        recordScalar("I3Sim Stardard dev", stats.getStddev());
        stats.clearResult();
    }
}

#define TRIGGER_TIMER   12345

void I3TRTClient::initializeI3() {
    cMessage *msg = new cMessage();
    msg->setKind(TRIGGER_TIMER);
    scheduleAt(simTime() + int(par("triggerDelay")), msg);
}

void I3TRTClient::handleTimerEvent(cMessage *msg) {
    if (msg->getKind() == TRIGGER_TIMER) {

        I3Identifier id;
        I3Trigger t;
        I3InsertTriggerMessage *imsg = new I3InsertTriggerMessage();
        I3IPAddress myAddress(nodeIPAddress, par("clientPort"));

        id.createRandomKey();
        t.setIdentifier(id);
        t.getIdentifierStack().push(myAddress);


        imsg->setTrigger(t);
        imsg->setSendReply(true);
        imsg->setSource(myAddress);
        imsg->setBitLength(INSERT_TRIGGER_L(imsg));
        imsg->setContextPointer(new simtime_t(simTime()));

        sendThroughUDP(imsg, gateway.address);
        scheduleAt(simTime() + int(par("triggerDelay")), msg);
    }
}
