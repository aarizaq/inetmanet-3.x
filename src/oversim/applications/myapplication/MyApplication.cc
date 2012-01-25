//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @author Antonio Zea
 */

#include <string>

#include "UnderlayConfigurator.h"
#include "GlobalStatistics.h"

#include "MyMessage_m.h"

#include "MyApplication.h"

Define_Module(MyApplication);

// initializeApp() is called when the module is being created.
// Use this function instead of the constructor for initializing variables.
void MyApplication::initializeApp(int stage)
{
    // initializeApp will be called twice, each with a different stage.
    // stage can be either MIN_STAGE_APP (this module is being created),
    // or MAX_STAGE_APP (all modules were created).
    // We only care about MIN_STAGE_APP here.

    if (stage != MIN_STAGE_APP) return;

    // copy the module parameter values to our own variables
    sendPeriod = par("sendPeriod");
    numToSend = par("numToSend");
    largestKey = par("largestKey");

    // initialize our statistics variables
    numSent = 0;
    numReceived = 0;

    // tell the GUI to display our variables
    WATCH(numSent);
    WATCH(numReceived);

    // start our timer!
    timerMsg = new cMessage("MyApplication Timer");
    scheduleAt(simTime() + sendPeriod, timerMsg);

    bindToPort(2000);
}


// finish is called when the module is being destroyed
void MyApplication::finishApp()
{
    // finish() is usually used to save the module's statistics.
    // We'll use globalStatistics->addStdDev(), which will calculate min, max, mean and deviation values.
    // The first parameter is a name for the value, you can use any name you like (use a name you can find quickly!).
    // In the end, the simulator will mix together all values, from all nodes, with the same name.

    globalStatistics->addStdDev("MyApplication: Sent packets", numSent);
    globalStatistics->addStdDev("MyApplication: Received packets", numReceived);
}


// handleTimerEvent is called when a timer event triggers
void MyApplication::handleTimerEvent(cMessage* msg)
{
    // is this our timer?
    if (msg == timerMsg) {
        // reschedule our message
        scheduleAt(simTime() + sendPeriod, timerMsg);

        // if the simulator is still busy creating the network,
        // let's wait a bit longer
        if (underlayConfigurator->isInInitPhase()) return;

        for (int i = 0; i < numToSend; i++) {

            // let's create a random key
            OverlayKey randomKey(intuniform(1, largestKey));

            MyMessage *myMessage; // the message we'll send
            myMessage = new MyMessage();
            myMessage->setType(MYMSG_PING); // set the message type to PING
            myMessage->setSenderAddress(thisNode); // set the sender address to our own
            myMessage->setByteLength(100); // set the message length to 100 bytes

            numSent++; // update statistics

            EV << thisNode.getIp() << ": Sending packet to "
               << randomKey << "!" << std::endl;

            callRoute(randomKey, myMessage); // send it to the overlay
        }
    } else {
        // unknown message types are discarded
        delete msg;
    }
}

// deliver() is called when we receive a message from the overlay.
// Unknown packets can be safely deleted here.
void MyApplication::deliver(OverlayKey& key, cMessage* msg)
{
    // we are only expecting messages of type MyMessage, throw away any other
    MyMessage *myMsg = dynamic_cast<MyMessage*>(msg);
    if (myMsg == NULL) {
        delete msg; // type unknown!
        return;
    }

    // are we a PING? send a PONG!
    if (myMsg->getType() == MYMSG_PING) {
        myMsg->setType(MYMSG_PONG); // change type

        EV << thisNode.getIp() << ": Got packet from "
           << myMsg->getSenderAddress() << ", sending back!"
           << std::endl;

        // send it back to its owner
        sendMessageToUDP(myMsg->getSenderAddress(), myMsg);
    } else {
        // only handle PING messages
        delete msg;
    }
}

// handleUDPMessage() is called when we receive a message from UDP.
// Unknown packets can be safely deleted here.
void MyApplication::handleUDPMessage(cMessage* msg)
{
    // we are only expecting messages of type MyMessage
    MyMessage *myMsg = dynamic_cast<MyMessage*>(msg);

    if (myMsg && myMsg->getType() == MYMSG_PONG) {
        EV << thisNode.getIp() << ": Got reply!" << std::endl;
        numReceived++;
    }

    // Message isn't needed any more -> delete it
    delete msg;
}
