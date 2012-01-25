//
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
 * @file ALMTest.cc
 * @author Stephan Krause
 * @author Dimitar Toshev
 */

#include <assert.h>
#include "ALMTest.h"
#include "ALMTestTracedMessage_m.h"

Define_Module(ALMTest);

ALMTest::ALMTest()
{
    timer = new cMessage( "app_timer"); 
    joinGroups = true;
    sendMessages = true;
    observer = NULL;
}

ALMTest::~ALMTest()
{
    cancelAndDelete( timer ); 
}

void ALMTest::initializeApp(int stage)
{
    if( stage != (numInitStages()-1))
    {
        return;
    }
    observer = check_and_cast<MessageObserver*>(
            simulation.getModuleByPath("globalObserver.globalFunctions[0].function.observer"));
    joinGroups = par("joinGroups");
    msglen = par("messageLength");
    sendMessages = par("sendMessages");
}

void ALMTest::finishApp()
{
    cancelEvent(timer);
    observer->nodeDead(getId());
}

void ALMTest::handleTimerEvent( cMessage* msg )
{
    if( msg == timer ) {
        double random = uniform( 0, 1 );
        if( (random < 0.1 && joinGroups) || groupNum < 1 ) {
            joinGroup( ++groupNum );
        } else if( random < 0.2 && joinGroups ) {
            leaveGroup( groupNum-- );
        } else if ( sendMessages ) {
            sendDataToGroup( intuniform( 1, groupNum ));
        }
        scheduleAt( simTime() + 10, timer );
    }
}

void ALMTest::handleLowerMessage(cMessage* msg)
{
    ALMMulticastMessage* mcast = dynamic_cast<ALMMulticastMessage*>(msg);
    if ( mcast != 0 ) {
        handleMCast(mcast);
    }
}

void ALMTest::handleReadyMessage(CompReadyMessage* msg)
{
    if( (getThisCompType() - msg->getComp() == 1) && msg->getReady() ) {
	groupNum = 0;
	cancelEvent(timer);
	scheduleAt(simTime() + 1, timer);
    }
    delete msg;
}

void ALMTest::handleTransportAddressChangedNotification()
{
    //TODO: Implement
    assert(false);
}

void ALMTest::handleUDPMessage(cMessage* msg)
{
    //TODO: Implement
    assert(false);
}

void ALMTest::handleUpperMessage(cMessage* msg)
{
    //TODO: Implement
    assert(false);
}

void ALMTest::joinGroup(int i)
{
    ALMSubscribeMessage* msg = new ALMSubscribeMessage;
    msg->setGroupId(OverlayKey(i));
    send(msg, "to_lowerTier");

    observer->joinedGroup(getId(), OverlayKey(i));
}

void ALMTest::leaveGroup(int i)
{
    ALMLeaveMessage* msg = new ALMLeaveMessage;
    msg->setGroupId(OverlayKey(i));
    send(msg, "to_lowerTier");

    observer->leftGroup(getId(), OverlayKey(i));
}

void ALMTest::sendDataToGroup( int i )
{
    ALMMulticastMessage* msg = new ALMMulticastMessage("Multicast message");
    msg->setGroupId(OverlayKey(i));

    ALMTestTracedMessage* traced = new ALMTestTracedMessage("Traced message");
    traced->setTimestamp();
    traced->setGroupId(OverlayKey(i));
    traced->setMcastId(traced->getId());
    traced->setSenderId(getId());
    traced->setByteLength(msglen);

    msg->encapsulate(traced);

    send(msg, "to_lowerTier");

    observer->sentMessage(traced);
}

void ALMTest::handleMCast( ALMMulticastMessage* mcast )
{
    getParentModule()->getParentModule()->bubble("Received message!");
    EV << "[ALMTest::handleMCast()]\n"
       << "    App received data message for group: " << mcast->getGroupId()
       << endl;

    ALMTestTracedMessage* traced = check_and_cast<ALMTestTracedMessage*>(mcast->decapsulate());
    traced->setReceiverId(getId());
    observer->receivedMessage(traced);

    delete traced;

    delete mcast;
}
