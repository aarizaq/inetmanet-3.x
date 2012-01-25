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
 * @file I3Composite.cc
 * @author Antonio Zea
 */

#include "I3BaseApp.h"

using namespace std;

struct I3CompositeMessage : public cPacket {
    string sentence;

    I3CompositeMessage *dup() const {
        return new I3CompositeMessage(*this);
    }
};

/** Composite test application for I3. This tests the ability of I3 to realize service composition in which
* the receving node decides the routing of a packet. First, all nodes insert their own trigger. Then,
* node 0 creates an packet containing an empty sentence (""), and sends it to I3 with an identifier stack containing the
* triggers of service nodes 1, 2, 3, 4 and then node 0. I3 then routes the packet to each of those triggers in that order.
* Each service node adds some words to the sentence ands sends it back to I3 with the same identifier stack it
* received. When it returns to node 0 it displays the sentence it received and starts again.
*/

class I3Composite : public I3BaseApp
{
private:
    static int index; // HACK Change to use index module when it's done
public:
    int myIndex;
    cMessage *sendPacketTimer;

    void initializeApp(int stage);
    void initializeI3();
    void deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg);
    void handleTimerEvent(cMessage *msg);
    void createMessage();
};

int I3Composite::index = 0;

void I3Composite::initializeApp(int stage)
{
    myIndex = index++;

}

void I3Composite::initializeI3()
{
    if (myIndex == 0) {
        sendPacketTimer = new cMessage("packet timer");
        scheduleAt(simTime() + 50, sendPacketTimer);
    }

    ostringstream os;
    os << "Composite " << myIndex;

    I3Identifier identifier(os.str());
    insertTrigger(identifier);
}

void I3Composite::createMessage() {
    I3CompositeMessage *cmsg = new I3CompositeMessage();

    cmsg->sentence = "";

    I3Identifier id0("Composite 0"),
    id1("Composite 1"),
    id2("Composite 2"),
    id3("Composite 3"),
    id4("Composite 4");

    I3IdentifierStack stack;
    stack.push(id0);
    stack.push(id4);
    stack.push(id3);
    stack.push(id2);
    stack.push(id1);

    sendPacket(stack, cmsg);
}

void I3Composite::handleTimerEvent(cMessage *msg)
{
    if (myIndex == 0) { // only the first node
        getParentModule()->bubble("Starting chain!");
        createMessage();
    }
    delete msg;
}

void I3Composite::deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg)
{
    I3CompositeMessage *cmsg = check_and_cast<I3CompositeMessage*>(msg);

    switch (myIndex) {
    case 0:
    {
        string final = "Final sentence: " + cmsg->sentence;
        getParentModule()->bubble(final.c_str());
        delete msg;
        createMessage();
        return;
    }
    case 1:
        getParentModule()->bubble("Adding 'He pounds'");
        cmsg->sentence += "He pounds ";
        break;
    case 2:
        getParentModule()->bubble("Adding 'his fists'");
        cmsg->sentence += "his fists ";
        break;
    case 3:
        getParentModule()->bubble("Adding 'against'");
        cmsg->sentence += "against ";
        break;
    case 4:
        getParentModule()->bubble("Adding 'the posts'");
        cmsg->sentence += "the posts ";
        break;
    default:
        delete msg;
        return;
    }
    sendPacket(stack, cmsg);
}


Define_Module(I3Composite);


