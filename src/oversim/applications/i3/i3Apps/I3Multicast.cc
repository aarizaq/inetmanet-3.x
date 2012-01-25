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
 * @file I3Multicast.cc
 * @author Antonio Zea
 */


#include "I3BaseApp.h"

using namespace std;

/** Simple module to test I3 multicast capabilities. All nodes register the same 
* identifier, then one node sends a message to that identifier. All participating
* nodes receive the packet .
*/

class I3Multicast : public I3BaseApp
{
public:
    cMessage *sendPacketTimer;

    void initializeApp(int stage);
    void initializeI3();
    void deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg);
    void handleTimerEvent(cMessage* msg);
};

void I3Multicast::initializeApp(int stage)
{
}

void I3Multicast::initializeI3()
{
    sendPacketTimer = new cMessage("packet timer");
    scheduleAt(simTime() + 20, sendPacketTimer);

    I3Identifier identifier("whee");
    insertTrigger(identifier);
}


void I3Multicast::deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg)
{
    getParentModule()->bubble("Got a message!");
    delete msg;
}

void I3Multicast::handleTimerEvent(cMessage* msg)
{
    if (msg == sendPacketTimer) {
    	cPacket *cmsg = new cPacket("woot");
        I3Identifier id("whee");

        getParentModule()->bubble("Sending message!");
        sendPacket(id, cmsg);
        scheduleAt(simTime() + 20, sendPacketTimer);
    } else delete msg;
}

Define_Module(I3Multicast);


