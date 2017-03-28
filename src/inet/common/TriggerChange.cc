//
// Copyright (C) 2012 Alfonso Ariza Universidad de M�laga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/TriggerChange.h"



namespace inet{

simsignal_t TriggerChange::triggerChange = registerSignal("triggerChange");


Define_Module(TriggerChange);

TriggerChange::TriggerChange()
{

}

TriggerChange::~TriggerChange()
{

    cancelAndDelete(timer);
}

void TriggerChange::initialize()
{
    timer = new cMessage("TriggerTimer");
    scheduleAt(simTime()+par("init"),timer);
}

void TriggerChange::handleMessage(cMessage *msg)
{
    if (timer == msg) {
        emit(triggerChange,simTime().dbl());
        scheduleAt(simTime()+par("interval"),timer);
    }
    throw cRuntimeError ("GlobalWirelessWirelessLinkInspector has received a packet");
}


}

