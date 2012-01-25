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
 * @file I3LatencyStretch.cc
 * @author Antonio Zea
 */

#include "I3BaseApp.h"
#include "I3.h"

#define TRIGGER_TIMER     1234

#define STRETCH_HELLO     12345
#define STRETCH_HELLOACK  12346
#define STRETCH_I3MSG     12347
#define STRETCH_IPMSG     12348

#define USE_NO_SAMPLING   0
#define USE_QUERY_FLAG    1
#define USE_CLOSEST_ID    2
#define USE_SAMPLING      (USE_QUERY_FLAG | USE_CLOSEST_ID)

using namespace std;

enum Stats {
    STAT_IP,
    STAT_I3,
    STAT_RATIO,
    NUM_STATS
};

static cStdDev stats[NUM_STATS];
static bool statsDumped = false;

struct NodeIdentity {
    I3Identifier id;
    I3IPAddress address;
};

struct LatencyInfo {
    simtime_t i3Time;
    simtime_t ipTime;
};

struct MsgContent {
    I3Identifier sourceId;
    I3IPAddress sourceIp;
    simtime_t creationTime;
};

class I3LatencyStretch : public I3BaseApp {
    int samplingType;
    I3Identifier generalId;
    I3Identifier myId;
    NodeIdentity partner;
    bool foundPartner;
    cStdDev myStats[NUM_STATS];

    std::map<I3IPAddress, LatencyInfo> latencies;


    void initializeApp(int stage);
    void initializeI3();
    void handleTimerEvent(cMessage *msg);
    void handleUDPMessage(cMessage* msg);
    void deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg);
    void finish();
};

Define_Module(I3LatencyStretch);


void I3LatencyStretch::initializeApp(int stage) {
    statsDumped = false;
    I3BaseApp::initializeApp(stage);
}

void I3LatencyStretch::finish() {
    recordScalar("Number of samples", myStats[STAT_RATIO].getCount());

    recordScalar("IP Min  ", myStats[STAT_IP].getMin());
    recordScalar("IP Max  ", myStats[STAT_IP].getMax());
    recordScalar("IP Mean ", myStats[STAT_IP].getMean());
    recordScalar("IP Stdev", myStats[STAT_IP].getStddev());

    recordScalar("I3 Min  ", myStats[STAT_I3].getMin());
    recordScalar("I3 Max  ", myStats[STAT_I3].getMax());
    recordScalar("I3 Mean ", myStats[STAT_I3].getMean());
    recordScalar("I3 Stdev", myStats[STAT_I3].getStddev());

    recordScalar("Ratio Min  ", myStats[STAT_RATIO].getMin());
    recordScalar("Ratio Max  ", myStats[STAT_RATIO].getMax());
    recordScalar("Ratio Mean ", myStats[STAT_RATIO].getMean());
    recordScalar("Ratio Stdev", myStats[STAT_RATIO].getStddev());

    if (!statsDumped) {
        statsDumped = true;
        recordScalar("General Number of samples", stats[STAT_RATIO].getCount());

        recordScalar("General IP Min  ", stats[STAT_IP].getMin());
        recordScalar("General IP Max  ", stats[STAT_IP].getMax());
        recordScalar("General IP Mean ", stats[STAT_IP].getMean());
        recordScalar("General IP Stdev", stats[STAT_IP].getStddev());
        stats[STAT_IP].clearResult();

        recordScalar("General I3 Min  ", stats[STAT_I3].getMin());
        recordScalar("General I3 Max  ", stats[STAT_I3].getMax());
        recordScalar("General I3 Mean ", stats[STAT_I3].getMean());
        recordScalar("General I3 Stdev", stats[STAT_I3].getStddev());
        stats[STAT_I3].clearResult();

        recordScalar("General Ratio Min  ", stats[STAT_RATIO].getMin());
        recordScalar("General Ratio Max  ", stats[STAT_RATIO].getMax());
        recordScalar("General Ratio Mean ", stats[STAT_RATIO].getMean());
        recordScalar("General Ratio Stdev", stats[STAT_RATIO].getStddev());
        stats[STAT_RATIO].clearResult();
    }
}

void I3LatencyStretch::initializeI3() {
    foundPartner = false;
    samplingType = par("useSampling");

    generalId.createFromHash("LatencyStretch");
    generalId.setName("LatencyStretch");
    generalId.createRandomSuffix();
    insertTrigger(generalId);

    if (samplingType & USE_CLOSEST_ID) {
        myId = retrieveClosestIdentifier();
    } else {
        myId.createRandomKey();
    }
    insertTrigger(myId);

    cMessage *msg = new cMessage();
    msg->setKind(TRIGGER_TIMER);
    scheduleAt(simTime() + 5, msg);
}

void I3LatencyStretch::handleUDPMessage(cMessage *msg) {
    if (msg->getContextPointer() != 0) {
        MsgContent *mc = (MsgContent*)msg->getContextPointer();

        if (!latencies.count(mc->sourceIp)) opp_error("Unknown Id!");

        LatencyInfo &info = latencies[mc->sourceIp];

        info.ipTime = simTime() - mc->creationTime;
        delete mc;
        delete msg;
    } else {
        I3BaseApp::handleUDPMessage(msg);
    }
}

void I3LatencyStretch::deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg)
{
    if (msg->getKind() == STRETCH_HELLO) {
        NodeIdentity *nid = (NodeIdentity*)msg->getContextPointer();
        I3Identifier otherId = nid->id;

        latencies[nid->address] = LatencyInfo();
        latencies[nid->address].i3Time = 0;
        latencies[nid->address].ipTime = 0;

        msg->setKind(STRETCH_HELLOACK);
        nid->id = myId;
        nid->address = I3IPAddress(nodeIPAddress, par("clientPort"));
        msg->setContextPointer(nid);
        sendPacket(otherId, msg, samplingType & USE_QUERY_FLAG);

    } else if (msg->getKind() == STRETCH_HELLOACK) {

        NodeIdentity *nid = (NodeIdentity*)msg->getContextPointer();
        partner = *nid;
        foundPartner = true;
        delete nid;
        delete msg;

    } else if (msg->getKind() == STRETCH_I3MSG) {

        MsgContent *mc = (MsgContent*)msg->getContextPointer();

        if (!latencies.count(mc->sourceIp)) opp_error("Unknown Id!");

        LatencyInfo &info = latencies[ mc->sourceIp ];

        info.i3Time = simTime() - mc->creationTime;

        if (info.ipTime != 0) {
            if (simTime() > 100) {
                stats[STAT_IP].collect(info.ipTime);
                stats[STAT_I3].collect(info.i3Time );
                stats[STAT_RATIO].collect(info.i3Time / info.ipTime);

                myStats[STAT_IP].collect(info.ipTime);
                myStats[STAT_I3].collect(info.i3Time );
                myStats[STAT_RATIO].collect(info.i3Time / info.ipTime);
            }
            info.i3Time = 0;
            info.ipTime = 0;
        }


        delete mc;
        delete msg;
    }
}

void I3LatencyStretch::handleTimerEvent(cMessage *msg) {
    if (msg->getKind() == TRIGGER_TIMER) {
        if (!foundPartner) {
            I3Identifier otherId;

            otherId.createFromHash("LatencyStretch");
            otherId.createRandomSuffix();

            cPacket *hmsg = new cPacket();
            NodeIdentity *nid = new NodeIdentity();

            nid->id = myId;
            nid->address = I3IPAddress(nodeIPAddress, par("clientPort"));

            hmsg->setKind(STRETCH_HELLO);
            hmsg->setContextPointer(nid);
            sendPacket(otherId, hmsg, samplingType & USE_QUERY_FLAG);

        } else {
            int length = (intrand(512) + 32) * 8;

            MsgContent *mc1 = new MsgContent();
            mc1->creationTime = simTime();
            mc1->sourceId = myId;
            mc1->sourceIp = I3IPAddress(nodeIPAddress, par("clientPort"));

            cPacket *i3msg = new cPacket();
            i3msg->setKind(STRETCH_I3MSG);
            i3msg->setContextPointer(mc1);
            i3msg->setBitLength(length);

            MsgContent *mc2 = new MsgContent(*mc1);
            cPacket *ipmsg = new cPacket();
            ipmsg->setKind(STRETCH_IPMSG);
            ipmsg->setContextPointer(mc2);
            ipmsg->setBitLength(length);

            sendPacket(partner.id, i3msg, samplingType & USE_QUERY_FLAG);
            sendThroughUDP(ipmsg, partner.address);

        }

        scheduleAt(simTime() + truncnormal(15, 5), msg);

    }
}
