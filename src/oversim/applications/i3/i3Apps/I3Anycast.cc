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
 * @file I3Anycast.cc
 * @author Antonio Zea
 */

#include "I3Anycast.h"

using namespace std;

/** Random ping test application for I3. First, all nodes insert a trigger in which the identifier has a constant prefix
* but a random postfix. Then the first node sends a message with an identifier that has the previous prefix, but
* a different random suffix. That makes that, when the packet is matched within I3 and sent to the closest match,
* it will land each time on a random node. The fact that it retains the same prefix guarantees that a match will always exist
* and the packet won't be dropped. After arrival in each node, the process is repeated.
*/


int I3Anycast::index = 0;


void I3Anycast::initializeApp(int stage) {
    myIndex = index++;
}


void I3Anycast::initializeI3()
{
    sendPacketTimer = new cMessage("packet timer");
    scheduleAt(simTime() + 20, sendPacketTimer);

    I3Identifier identifier("I3 Anycast test"); // create an identifier with the given string hash as prefix
    identifier.createRandomSuffix(); // set the suffix as random
    insertTrigger(identifier); // and insert a trigger with that id

}

void I3Anycast::handleTimerEvent(cMessage *msg)
{
    if (myIndex == 0) { // only the first node
        cPacket *cmsg = new cPacket("woot");

        I3Identifier id("I3 Anycast test"); // create an identifier with the previously used hash as prefix
        id.createRandomSuffix(); // and a random suffix

        //cout << "Send message!" << endl;
        getParentModule()->bubble("Sending message!");
        sendPacket(id, cmsg); // send the first message with that identifier
    }
    delete msg;
}

void I3Anycast::deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg)
{
    // after arrival, repeat the same process
    I3Identifier id("I3 Anycast test"); // create an identifier with the previously used hash as prefix
    id.createRandomSuffix(); // and a random suffix

    getParentModule()->bubble("Got message - sending back!");
    sendPacket(id, msg); // and send back to I3
}


Define_Module(I3Anycast);


