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
 * @file I3Session.cc
 * @author Antonio Zea
 */


#include "I3BaseApp.h"
#include "I3SessionMessage_m.h"

using namespace std;

#define DONT_REMOVE         0
#define REMOVE_AT_ONCE      1
#define WAIT_STATIC         2
#define WAIT_CONFIRMATION   3

#define TYPE_CHANGE_SESSION 0
#define TYPE_REMOVE_TRIGGER 1

enum Stats {
    STAT_CHANGE,
    STAT_RX,
    STAT_WRONG,
    NUM_STATS
};

class I3SessionServer : public I3BaseApp
{
public:
    int numExchanged;

    I3Identifier myIdentifier;
    I3Identifier clientIdentifier;

    void initializeI3();
    void deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg);
    void finish();
};

Define_Module(I3SessionServer);

void I3SessionServer::initializeI3()
{
    numExchanged = 0;
    clientIdentifier.createFromHash("Client");
    myIdentifier.createFromHash("Server");
    insertTrigger(myIdentifier);
}

void I3SessionServer::deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg)
{
    SessionMsg *smsg = check_and_cast<SessionMsg*>(msg);
    smsg->setValue(smsg->getValue() + 1);
    numExchanged++;
    sendPacket(clientIdentifier, smsg);
}

void I3SessionServer::finish() {
    recordScalar("Server packets exchanged", numExchanged);
}






class I3SessionClient : public I3BaseApp
{
public:
    cStdDev myStats[NUM_STATS];

    int numForeignPackets;
    int numSessions;
    int numExchanged;
    bool holdsSession;
    double actualValue;
    I3Identifier clientIdentifier;
    I3Identifier serverIdentifier;
    I3Identifier poolIdentifier;

    void initializeApp(int stage);
    virtual void initializeI3();
    void deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg);
    void handleTimerEvent(cMessage *msg);
    void finish();
};

Define_Module(I3SessionClient);


void I3SessionClient::initializeApp(int stage)
{
    holdsSession = false;
    numForeignPackets = 0;
    numSessions = 0;
    numExchanged = 0;
    WATCH(numForeignPackets);
    clientIdentifier.createFromHash("Client");
    serverIdentifier.createFromHash("Server");
}

void I3SessionClient::initializeI3() {
    poolIdentifier.createFromHash("Pool");
    poolIdentifier.createRandomSuffix();
    insertTrigger(poolIdentifier);
}

void I3SessionClient::deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg)
{
    SessionMsg *smsg = check_and_cast<SessionMsg*>(msg);

    if (smsg->getType() == PAYLOAD) {
        if (holdsSession) {
            //std::cout << "Got value " << smsg->getValue() << ", resending..." << endl;
            numExchanged++;
            actualValue = smsg->getValue();
            sendPacket(serverIdentifier, msg);
        } else {
            numForeignPackets++;
            //std::cout << "Foreign packet at " << nodeIPAddress << endl;
            delete msg;
        }

    } else if (smsg->getType() == CHANGE_SESSION) {

        //cout << "Insert new trigger" << nodeIPAddress << endl;
        /* resume session */
        insertTrigger(clientIdentifier, int(par("sessionMobilityType")) != DONT_REMOVE); // renew only if type != DONT_REMOVE
        holdsSession = true;

        SessionMsg *newMsg = new SessionMsg();
        newMsg->setType(PAYLOAD);
        newMsg->setValue(smsg->getValue());
        sendPacket(serverIdentifier, newMsg);

        if (int(par("sessionMobilityType")) == WAIT_CONFIRMATION) {
            // send confirmation
            SessionMsg *newMsg = new SessionMsg();
            newMsg->setType(TRIGGER_CONFIRMATION);
            newMsg->setValue(0);
            newMsg->setSource(poolIdentifier);
            sendPacket(smsg->getSource(), newMsg);
        }
        delete smsg;

        cMessage *msg = new cMessage();
        msg->setKind(TYPE_CHANGE_SESSION);
        scheduleAt(simTime() + int(par("sessionTime")), msg);
        numSessions++;

        getParentModule()->bubble("Got session!");

    } else if (smsg->getType() == TRIGGER_CONFIRMATION) { // only for WAIT_CONFIRMATION
        removeTrigger(clientIdentifier);
        getParentModule()->bubble("Got confirmation for erase.");
        delete smsg;

    } else {
        // ??
        delete smsg;
    }
}

void I3SessionClient::finish() {
    recordScalar("Client packets received", numExchanged);
    recordScalar("Client wrong received  ", numForeignPackets);
    recordScalar("Client session changed ", numSessions);

}

void I3SessionClient::handleTimerEvent(cMessage *msg) {
    if (msg->getKind() == TYPE_CHANGE_SESSION) {
        myStats[STAT_CHANGE].collect(simTime());
        switch (int(par("sessionMobilityType"))) {
        case DONT_REMOVE:
        case WAIT_CONFIRMATION:
            break;
        case REMOVE_AT_ONCE:
            removeTrigger(clientIdentifier);
            break;
        case WAIT_STATIC:
            cMessage *msg = new cMessage();
            msg->setKind(TYPE_REMOVE_TRIGGER);
            scheduleAt(simTime() + int(par("sessionMobilityWait")), msg);
            break;
        }
        holdsSession = false;

        /* cede session */
        I3Identifier sessionId;

        sessionId.createFromHash("Pool");
        sessionId.createRandomSuffix();

        SessionMsg *newMsg = new SessionMsg();
        newMsg->setType(CHANGE_SESSION);
        newMsg->setValue(actualValue);
        newMsg->setSource(poolIdentifier);
        sendPacket(sessionId, newMsg);

        getParentModule()->bubble("Ceding session...");
        delete msg;

    } else if (msg->getKind() == TYPE_REMOVE_TRIGGER) { // for WAIT_STATIC only
        getParentModule()->bubble("Timer ticked for erase.");
        removeTrigger(clientIdentifier);
        //cout << "Delete old trigger " << nodeIPAddress << endl;
        delete msg;
    }
}










class I3SessionClientStarter : public I3SessionClient
{
public:
    void initializeI3();
};

Define_Module(I3SessionClientStarter);

void I3SessionClientStarter::initializeI3() {
    I3SessionClient::initializeI3();

    /* start session */
    insertTrigger(clientIdentifier, int(par("sessionMobilityType")) != DONT_REMOVE); // renew only if type != DONT_REMOVE
    holdsSession = true;

    SessionMsg *newMsg = new SessionMsg();
    newMsg->setType(PAYLOAD);
    newMsg->setValue(0);
    sendPacket(serverIdentifier, newMsg);

    cMessage *msg = new cMessage();
    msg->setKind(TYPE_CHANGE_SESSION);

    std::cout << "Started starts" << endl;
    scheduleAt(simTime() + int(par("sessionTime")), msg);
}
