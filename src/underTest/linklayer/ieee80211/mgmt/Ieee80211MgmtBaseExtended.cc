//
// Copyright (C) 2006 Juan-Carlos Maureira
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "Ieee80211MgmtBaseExtended.h"
#include "Ieee802Ctrl_m.h"
#include "InterfaceTableAccess.h"

static std::ostream& operator<< (std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

void Ieee80211MgmtBaseExtended::initialize(int stage)
{
    if (stage==0)
    {
        PassiveQueueBase::initialize();

        dataQueue.setName("wlanDataQueue");
        mgmtQueue.setName("wlanMgmtQueue");
        dataQueueLenVec.setName("queue length");
        dataQueueDropVec.setName("queue drop count");

        numDataFramesReceived = 0;
        numMgmtFramesReceived = 0;
        numMgmtFramesDropped = 0;
        numDataFramesDropped = 0;

        WATCH(numDataFramesReceived);
        WATCH(numMgmtFramesReceived);
        WATCH(numMgmtFramesDropped);
        WATCH(numDataFramesDropped);
        // configuration
        frameCapacity = par("frameCapacity");


    }
    else if (stage==1)
    {
        // obtain our address from MAC
        cModule *mac = getParentModule()->getSubmodule("mac");
        if (!mac)
            error("MAC module not found; it is expected to be next to this submodule and called 'mac'");
        myAddress.setAddress(mac->par("address").stringValue());
        myEntry = NULL;
        IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
        if (ift)
        {
            for (int i = 0; i < ift->getNumInterfaces(); i++)
            {
                if (ift->getInterface(i)->getMacAddress()==myAddress)
                {
                    myEntry = ift->getInterface(i);
                    EV << "Interface Entry: " << myEntry->getFullName() << endl;
                }
            }
        }
        else
        {
            EV << "No interface Table, we can not determine our Interface Entry" << endl;
            myEntry = new InterfaceEntry(this);
            myEntry->setMACAddress(myAddress);
            myEntry->setName(getParentModule()->getName());
        }
    }
}

void Ieee80211MgmtBaseExtended::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
    }
    else if (msg->arrivedOn("macIn"))
    {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
        processFrame(frame);
    }
    else if (msg->arrivedOn("agentIn"))
    {
        // process command from agent
        EV << "Command arrived from agent: " << msg << "\n";
        int msgkind = msg->getKind();
        cPolymorphic *ctrl = msg->removeControlInfo();
        delete msg;

        handleCommand(msgkind, ctrl);
    }
    else
    {
        // packet from upper layers, to be sent out
        cPacket *pk = PK(msg);
        EV << "Packet arrived from upper layers: " << pk << "\n";
        if (pk->getByteLength() > 2312)
            error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
                  pk->getClassName(), pk->getName(), pk->getByteLength());

        handleUpperMessage(pk);

    }
}

void Ieee80211MgmtBaseExtended::handleUpperMessage(cPacket *msg)
{
    return;
}

void Ieee80211MgmtBaseExtended::sendOrEnqueue(cPacket *frame)
{
    PassiveQueueBase::handleMessage(frame);
}

cMessage* Ieee80211MgmtBaseExtended::enqueue(cMessage *msg)
{
    ASSERT(dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg)!=NULL);
    bool isDataFrame = dynamic_cast<Ieee80211DataFrame *>(msg)!=NULL;

    if (!isDataFrame)
    {
        // management frames are inserted into mgmtQueue
        mgmtQueue.insert(msg);
        return NULL;
    }
    else if (frameCapacity && dataQueue.length() >= frameCapacity)
    {
        EV << "Queue full, dropping packet.\n";
        dataQueueDropVec.record(1);
        return msg;
    }
    else
    {
        dataQueue.insert(msg);
        dataQueueLenVec.record(dataQueue.length());
        return NULL;
    }
}

cMessage *Ieee80211MgmtBaseExtended::dequeue()
{
    // management frames have priority
    if (!mgmtQueue.empty())
        return (cMessage *)mgmtQueue.pop();

    // return a data frame if we have one
    if (dataQueue.empty())
        return NULL;

    cMessage *pk = (cMessage *)dataQueue.pop();

    // statistics
    dataQueueLenVec.record(dataQueue.length());
    return pk;
}

bool Ieee80211MgmtBaseExtended::isEmpty()
{

    if (dataQueue.empty() && mgmtQueue.empty())
        return true;
    return false;
}

void Ieee80211MgmtBaseExtended::sendOut(cMessage *msg)
{
    send(msg, "macOut");
}

void Ieee80211MgmtBaseExtended::dropFrame(Ieee80211Frame *frame)
{
    EV << "dropping frame: " << (cMessage *)frame << "\n";
    delete frame;
    if (dynamic_cast<Ieee80211ManagementFrame*>(frame))
    {
        numMgmtFramesDropped++;
    }
    else
    {
        numDataFramesDropped++;
    }
}

cPacket *Ieee80211MgmtBaseExtended::decapsulate(Ieee80211DataFrame *frame)
{
    cPacket *payload = frame->decapsulate();

    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
    ctrl->setSrc(frame->getAddress3());
    // ctrl->setSrc(frame->getTransmitterAddress());
    ctrl->setDest(frame->getReceiverAddress());
    payload->setControlInfo(ctrl);

    delete frame;
    return payload;
}

void Ieee80211MgmtBaseExtended::sendUp(cMessage *msg)
{
    send(msg, "upperLayerOut");
}

void Ieee80211MgmtBaseExtended::processFrame(Ieee80211DataOrMgmtFrame *frame)
{
    switch (frame->getType())
    {
    case ST_DATA:
        numDataFramesReceived++;
        handleDataFrame(check_and_cast<Ieee80211DataFrame *>(frame));
        break;
    case ST_AUTHENTICATION:
        numMgmtFramesReceived++;
        handleAuthenticationFrame(check_and_cast<Ieee80211AuthenticationFrame *>(frame));
        break;
    case ST_DEAUTHENTICATION:
        numMgmtFramesReceived++;
        handleDeauthenticationFrame(check_and_cast<Ieee80211DeauthenticationFrame *>(frame));
        break;
    case ST_ASSOCIATIONREQUEST:
        numMgmtFramesReceived++;
        handleAssociationRequestFrame(check_and_cast<Ieee80211AssociationRequestFrame *>(frame));
        break;
    case ST_ASSOCIATIONRESPONSE:
        numMgmtFramesReceived++;
        handleAssociationResponseFrame(check_and_cast<Ieee80211AssociationResponseFrame *>(frame));
        break;
    case ST_REASSOCIATIONREQUEST:
        numMgmtFramesReceived++;
        handleReassociationRequestFrame(check_and_cast<Ieee80211ReassociationRequestFrame *>(frame));
        break;
    case ST_REASSOCIATIONRESPONSE:
        numMgmtFramesReceived++;
        handleReassociationResponseFrame(check_and_cast<Ieee80211ReassociationResponseFrame *>(frame)); break;
    case ST_DISASSOCIATION:
        numMgmtFramesReceived++;
        handleDisassociationFrame(check_and_cast<Ieee80211DisassociationFrame *>(frame));
        break;
    case ST_BEACON:
        numMgmtFramesReceived++;
        handleBeaconFrame(check_and_cast<Ieee80211BeaconFrame *>(frame));
        break;
    case ST_PROBEREQUEST:
        numMgmtFramesReceived++;
        handleProbeRequestFrame(check_and_cast<Ieee80211ProbeRequestFrame *>(frame));
        break;
    case ST_PROBERESPONSE:
        numMgmtFramesReceived++;
        handleProbeResponseFrame(check_and_cast<Ieee80211ProbeResponseFrame *>(frame));
        break;
    default:
        error("unexpected frame type (%s)%s", frame->getClassName(), frame->getName());
    }
}

