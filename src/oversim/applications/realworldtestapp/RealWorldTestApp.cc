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
 * @file RealWorldTestApp.cc
 * @author Bernhard Heep
 */

#include "IPvXAddressResolver.h"
#include <CommonMessages_m.h>


#include "RealWorldTestApp.h"
#include "RealWorldTestMessage_m.h"


Define_Module(RealWorldTestApp);


void RealWorldTestApp::initializeApp(int stage)
{
    if (stage != MIN_STAGE_APP)
        return;

    displayMsg = new cMessage("DISPLAY");
}

void RealWorldTestApp::finishApp()
{
    cancelAndDelete(displayMsg);
}

void RealWorldTestApp::deliver(OverlayKey& key, cMessage* msg)
{
    RealWorldTestMessage* testMsg = check_and_cast<RealWorldTestMessage*>(msg);
    OverlayCtrlInfo* overlayCtrlInfo =
        check_and_cast<OverlayCtrlInfo*>(msg->removeControlInfo());

    if(std::string(testMsg->getName()) == "CALL") {
	// bubble
	//std::string tempString = "Call for key (" + key.toString() +
	//") with message: \"" + testMsg->getMessage() + "\"";

	std::string tempString = "Message received: \"" + std::string(testMsg->getMsg()) + "\"";

	getParentModule()->getParentModule()->bubble(tempString.c_str());

	// change color
	getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, "green");

	if(displayMsg->isScheduled())
	    cancelEvent(displayMsg);
	scheduleAt(simTime() + 2, displayMsg);

	// send back
	RealWorldTestMessage* answerMsg = new RealWorldTestMessage("ANSWER");
	//tempString = "Reply to: \"" + std::string(testMsg->getMessage()) + "\" from "
	//    + overlayCtrlInfo->getThisNode().getKey().toString();

	tempString = "Reply to: \"" + std::string(testMsg->getMsg()) + "\" from "
	    + overlay->getThisNode().getIp().str();

	answerMsg->setMsg(tempString.c_str());
	callRoute(overlayCtrlInfo->getSrcNode().getKey(), answerMsg, overlayCtrlInfo->getSrcNode());

	delete testMsg;
    } else if(std::string(testMsg->getName()) == "ANSWER") {
	if(gate("to_upperTier")->getNextGate()->isConnectedOutside())
	    send(msg, "to_upperTier");
	else
	    delete msg;
    }

    delete overlayCtrlInfo;
}

void RealWorldTestApp::handleUpperMessage(cMessage* msg)
{
    RealWorldTestMessage* callMsg = check_and_cast<RealWorldTestMessage*>(msg);
    callMsg->setName("CALL");
    callRoute(OverlayKey::sha1(const_cast<char*>(callMsg->getMsg())), callMsg);
}

void RealWorldTestApp::handleTimerEvent(cMessage* msg)
{
    getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, "");
}
