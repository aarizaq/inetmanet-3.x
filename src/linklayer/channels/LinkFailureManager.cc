// Copyright (C) 2009 Juan-Carlos Maureira
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

#include "LinkFailureManager.h"
#include "LinkFailureMessage_m.h"
#include "ProgramedFailureChannel.h"

Define_Module(LinkFailureManager);


std::ostream& operator << (std::ostream& os, const LinkState& ls)
{
    if (ls == UP)
    {
        os << "UP";
    }
    else
    {
        os << "DOWN";
    }
    return os;
}


void LinkFailureManager::initialize()
{
    EV << "Module LinkFailureManager initialized in " << this->getOwner()  << endl;
}

void LinkFailureManager::handleMessage(cMessage *msg)
{
    if (dynamic_cast<LinkFailureMessage*>(msg))
    {
        EV << "Link Failure Msg Arrived" << endl;
        LinkFailureMessage* event = dynamic_cast<LinkFailureMessage*>(msg);
        cModule* module = simulation.getModule(event->getModule_id());
        if (module!=NULL)
        {
            cGate* gate = module->gate(event->getPort_id());
            if (gate!=NULL)
            {
                ProgramedFailureChannel* ch = dynamic_cast<ProgramedFailureChannel*>(gate->getChannel());
                if (ch!=NULL)
                {
                    ch->setState(event->getState());
                }
            }
            else
            {
                EV << "Gate does not exist. discarding this event" << endl;
            }
        }
        else
        {
            EV << "Source Module does not exists. discarding this event" << endl;
        }
    }
    delete(msg);
}

void LinkFailureManager::scheduleLinkStateChange(cGate* gate, simtime_t when, LinkState state)
{

    Enter_Method_Silent();

    LinkFailureMessage* msg = new LinkFailureMessage("Link Failure");
    msg->setPort_id(gate->getId());
    msg->setState(state);
    msg->setModule_id(gate->getOwnerModule()->getId());

    std::stringstream desc;
    desc << "Module " << gate->getOwnerModule()->getFullName() << " port " << gate->getFullName() << " State " << state;
    msg->setDescription(desc.str().c_str());

    EV << "Link Change for " << gate->getOwnerModule()->getFullName() << " in port " << gate->getFullName() << " port id " << gate->getId() << " state scheduled at " << when << " state " << state << endl;
    scheduleAt(when,msg);
}
