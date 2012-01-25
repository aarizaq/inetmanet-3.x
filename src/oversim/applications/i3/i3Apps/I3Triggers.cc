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
 * @file I3Triggers.cc
 * @author Antonio Zea
 */

#include "I3BaseApp.h"
#include "I3TriggersMessage_m.h"

using namespace std;

class I3Triggers : public I3BaseApp
{
private:
    static int index; // HACK Change to use index module when it's done
public:
    int myIndex;

    // for both
    I3Identifier myIdentifier;

    // for server
    struct Client {
        I3Identifier clientId;
        I3Identifier privateId;
        int sentValue;
    };
    map<I3Identifier, Client> clients;

    // for client
    I3Identifier publicIdentifier;
    I3Identifier privateIdentifier;

    cMessage *handShakeTimer;
    cMessage *sendPacketTimer;

    void initializeApp(int stage);
    void initializeI3();
    void deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg);
    void handleTimerEvent(cMessage *msg);
    void createMessage();
};

int I3Triggers::index = 0;

void I3Triggers::initializeApp(int stage)
{
    myIndex = index++;

    WATCH(myIndex);
    WATCH(myIdentifier);
}

void I3Triggers::initializeI3() {
    publicIdentifier.createFromHash("Triggers 0");

    ostringstream os;
    os << "Triggers " << myIndex;

    myIdentifier.createFromHash(os.str());
    insertTrigger(myIdentifier);

    // handshake timer must be set before the packet timer!
    handShakeTimer = new cMessage("handshake timer");
    scheduleAt(simTime() + 5, handShakeTimer);

    sendPacketTimer = new cMessage("packet timer");
    scheduleAt(simTime() + 10, sendPacketTimer);
}

void I3Triggers::handleTimerEvent(cMessage *msg)
{
    if (myIndex != 0) {
        if (msg == handShakeTimer) {

            // start handshake
            TriggersHandshakeMsg *msg = new TriggersHandshakeMsg();

            msg->setValue(myIndex);
            msg->setTriggerId(myIdentifier);
            sendPacket(publicIdentifier, msg);
            getParentModule()->bubble("Started handshake");

        } else if (msg == sendPacketTimer) {

            //send a packet
            TriggersMsg *msg = new TriggersMsg();
            msg->setValue(0);
            sendPacket(privateIdentifier, msg);

            // reset timer
            sendPacketTimer = new cMessage("packet timer");
            scheduleAt(simTime() + 5, sendPacketTimer);

            getParentModule()->bubble("Sending packet");
        }
    }
    delete msg;
}

void I3Triggers::deliver(I3Trigger &matchingTrigger, I3IdentifierStack &stack, cPacket *msg)
{
    TriggersHandshakeMsg *hmsg = dynamic_cast<TriggersHandshakeMsg*>(msg);
    TriggersMsg *tmsg = NULL;

    if (!hmsg) tmsg = dynamic_cast<TriggersMsg*>(msg);

    if (myIndex == 0) {
        // act as server

        if (hmsg) {
            getParentModule()->bubble("Got handshake!");

            // this is a handshake message
            TriggersHandshakeMsg *newMsg = new TriggersHandshakeMsg();

            // create a new private trigger
            I3Identifier privateId;
            privateId.createRandomKey();

            // insert it into i3
            insertTrigger(privateId);

            // store the client's value
            Client client;
            client.clientId = hmsg->getTriggerId();
            client.privateId = privateId;
            client.sentValue = hmsg->getValue();
            clients[privateId] = client;

            // notify the client back
            newMsg->setValue(0);
            newMsg->setTriggerId(privateId);
            sendPacket(hmsg->getTriggerId(), newMsg);



        } else if (tmsg) {

            getParentModule()->bubble("Got normal message!");

            // this is a normal message. just reply with sent value
            TriggersMsg *newMsg = new TriggersMsg();

            Client &client = clients[matchingTrigger.getIdentifier()];
            newMsg->setValue(client.sentValue);
            sendPacket(client.clientId, newMsg);
        }

    } else {
        //act as client

        if (hmsg) {

            getParentModule()->bubble("Finished handshaking!");

            // store the private trigger
            privateIdentifier = hmsg->getTriggerId();
            WATCH(privateIdentifier);

        } else {

            // check if the value is valid
            if (tmsg->getValue() == myIndex) {
                getParentModule()->bubble("Got packet - Got my id!");
            } else {
                getParentModule()->bubble("Got packet - Got an unknown id");
            }

        }
    }
}


Define_Module(I3Triggers);


