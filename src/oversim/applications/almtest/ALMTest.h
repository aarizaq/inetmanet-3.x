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
 * @file ALMTest.h
 * @author Stephan Krause
 * @author Dimitar Toshev
 */


#ifndef __ALMTEST_H_
#define __ALMTEST_H_

#include <omnetpp.h>
#include "BaseApp.h"
#include "CommonMessages_m.h"
#include "MessageObserver.h"

class ALMTest : public BaseApp {
    public:
        ALMTest();
        ~ALMTest();

        void initializeApp(int stage);
	void finishApp();
    protected:
	// Handlers for message types
	void handleLowerMessage(cMessage* msg);
	//void handleNodeGracefulLeaveNotification();
	//void handleNodeLeaveNotification();
	void handleReadyMessage(CompReadyMessage* msg);
	//void handleTraceMessage(cMessage* msg);
	void handleTransportAddressChangedNotification();
	void handleUDPMessage(cMessage* msg);
	void handleUpperMessage(cMessage* msg);
	void handleTimerEvent(cMessage* msg);

        void joinGroup(int i);
        void leaveGroup(int i);
        void sendDataToGroup(int i);
        void handleMCast( ALMMulticastMessage* mcast );
        cMessage* timer;
        int groupNum;

    private:
        // Controls if we'll try joining groups other than 1.
        // True by default.
        // Set to false for multicast protocols that do not support
        // more than one multicast group.
        bool joinGroups;

	bool sendMessages;

        MessageObserver* observer;
        int msglen;
};

#endif
