//
// Copyright (C) 2006 Andras Varga
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

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"

#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211eClassifier.h"
#include <string>

namespace inet {

namespace ieee80211 {

simsignal_t Ieee80211MgmtBase::dataQueueLenSignal = registerSignal("dataQueueLen");

static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

void Ieee80211MgmtBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        PassiveQueueBase::initialize();

        numQueues = 1;
        if (par("EDCA"))
        {
            const char *classifierClass = par("classifier");
            classifier = check_and_cast<IQoSClassifier*>(createOne(classifierClass));
            numQueues = classifier->getNumQueues();
        }
        dataQueue.resize(numQueues);
        packetRequestedCat.resize(numQueues-1);
        mgmtQueue.setName("wlanMgmtQueue");
        int length = 0;
        if (numQueues == 1)
        {
            dataQueue[0].setName("wlanDataQueue");
            length = dataQueue[0].length();
        }
        else
        {

            std::string str;
            for (int i = 0; i < numQueues; i++)
            {
                str = "wlanDataQueue" + std::to_string(i);
                dataQueue[i].setName(str.c_str());
                length += dataQueue[i].length();
            }
            for (int i = 0; i < numQueues -1 ; i++)
                packetRequestedCat[i] = 0;
        }
        emit(dataQueueLenSignal, length);

        numDataFramesReceived = 0;
        numMgmtFramesReceived = 0;
        numMgmtFramesDropped = 0;
        WATCH(numDataFramesReceived);
        WATCH(numMgmtFramesReceived);
        WATCH(numMgmtFramesDropped);

        // configuration
        frameCapacity = par("frameCapacity");


    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        // obtain our address from MAC
        cModule *mac = getParentModule()->getSubmodule("mac");
        if (!mac)
            throw cRuntimeError("MAC module not found; it is expected to be next to this submodule and called 'mac'");
        myAddress.setAddress(mac->par("address").stringValue());
    }
}

void Ieee80211MgmtBase::handleMessage(cMessage *msg)
{
    if (!isOperational)
        throw cRuntimeError("Message '%s' received when module is OFF", msg->getName());

    if (msg->isSelfMessage()) {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
    }
    else if (msg->arrivedOn("macIn")) {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
        processFrame(frame);
    }
    else if (msg->arrivedOn("agentIn")) {
        // process command from agent
        EV << "Command arrived from agent: " << msg << "\n";
        int msgkind = msg->getKind();
        cObject *ctrl = msg->removeControlInfo();
        delete msg;

        handleCommand(msgkind, ctrl);
    }
    else {
        // packet from upper layers, to be sent out
        cPacket *pk = PK(msg);
        EV << "Packet arrived from upper layers: " << pk << "\n";
        if (pk->getByteLength() > 2312)
            throw cRuntimeError("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
                    pk->getClassName(), pk->getName(), (int)(pk->getByteLength()));

        handleUpperMessage(pk);
    }
}

void Ieee80211MgmtBase::sendOrEnqueue(cPacket *frame, const int &cat)
{
    if (frame->getBitLength() > 0)
        frame->setKind(cat);

    if (cat == 0)
    {
        PassiveQueueBase::handleMessage(frame);
        return;
    }

    if (packetRequestedCat[cat-1] > 0)
    {
        packetRequestedCat[cat-1]--;
        emit(enqueuePkSignal, frame);
        emit(dequeuePkSignal, frame);
        emit(queueingTimeSignal, SIMTIME_ZERO);
        sendOut (frame);
    }
    else
    {
        frame->setArrivalTime(simTime());
        cMessage *droppedMsg = enqueue(frame, cat);
        if (frame != droppedMsg)
            emit(enqueuePkSignal, frame);
        if (droppedMsg)
        {
            numQueueDropped++;
            emit(dropPkByQueueSignal, droppedMsg);
            delete droppedMsg;
        }
        else
            notifyListeners();
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived, numQueueDropped);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void Ieee80211MgmtBase::sendOrEnqueue(cPacket *frame)
{
    ASSERT(isOperational);
    if (numQueues == 1)
        PassiveQueueBase::handleMessage(frame);
    else
    {
        int cat = classifier->classifyPacket(frame);
        sendOrEnqueue(frame, cat);
    }
}



cMessage *Ieee80211MgmtBase::enqueue(cMessage *msg)
{
    ASSERT(dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg) != NULL);
    bool isDataFrame = dynamic_cast<Ieee80211DataFrame *>(msg) != NULL;
    int cat = 0;

    if (numQueues > 1 && isDataFrame)
    {
        cat = classifier->classifyPacket(msg);
    }

    if (!isDataFrame) {
        // management frames are inserted into mgmtQueue
        mgmtQueue.insert(msg);
        return NULL;
    }
    else if (frameCapacity && dataQueue[cat].length() >= frameCapacity) {
        EV << "Queue full, dropping packet.\n";
        return msg;
    }
    else {
        dataQueue[cat].insert(msg);
        int length = 0;
        for (int i = 0; i < numQueues; i++)
            length += dataQueue[i].length();
        emit(dataQueueLenSignal, length);
        return NULL;
    }
}


cMessage *Ieee80211MgmtBase::enqueue(cMessage *msg, const int &cat)
{
    ASSERT(dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg) != NULL);
    bool isDataFrame = dynamic_cast<Ieee80211DataFrame *>(msg) != NULL;

    if (!isDataFrame) {
        // management frames are inserted into mgmtQueue
        mgmtQueue.insert(msg);
        return NULL;
    }
    else if (frameCapacity && dataQueue[cat].length() >= frameCapacity) {
        EV << "Queue full, dropping packet.\n";
        return msg;
    }
    else {
        dataQueue[cat].insert(msg);
        int length = 0;
        for (int i = 0; i < numQueues; i++)
            length += dataQueue[i].length();
        emit(dataQueueLenSignal, length);
        return NULL;
    }
}

bool Ieee80211MgmtBase::isEmpty()
{
    if (!mgmtQueue.empty())
        return false;
    return dataQueue.empty();
}

cMessage *Ieee80211MgmtBase::dequeue()
{
    // management frames have priority
    if (!mgmtQueue.empty())
        return (cMessage *)mgmtQueue.pop();

    // return a data frame if we have one
    if (dataQueue[0].empty())
        return NULL;

    cMessage *pk = (cMessage *)dataQueue[0].pop();

    // statistics
    int length = 0;
    for (int i = 0; i < numQueues; i++)
        length += dataQueue[i].length();
    emit(dataQueueLenSignal, length);
    return pk;
}

void Ieee80211MgmtBase::requestPacket(const int &cat)
{
    Enter_Method("requestPacket(int)");

    cMessage *msg = dequeue(cat);
    if (msg == NULL) {
        if (cat == 0)
            packetRequested++;
        else
            packetRequestedCat[cat - 1]++;
    }
    else {
        emit(dequeuePkSignal, msg);
        emit(queueingTimeSignal, simTime() - msg->getArrivalTime());
        sendOut(msg);
    }
}

cMessage *Ieee80211MgmtBase::dequeue(const int & cat)
{
    // management frames have priority
    if (!mgmtQueue.empty())
        return (cMessage *)mgmtQueue.pop();

    // return a data frame if we have one
    if (dataQueue[cat].empty())
        return NULL;

    cMessage *pk = (cMessage *)dataQueue[cat].pop();

    // statistics
    int length = 0;
    for (int i = 0; i < numQueues; i++)
        length += dataQueue[i].length();
    emit(dataQueueLenSignal, length);
    return pk;
}

void Ieee80211MgmtBase::sendOut(cMessage *msg)
{
    ASSERT(isOperational);
    send(msg, "macOut");
}

void Ieee80211MgmtBase::dropManagementFrame(Ieee80211ManagementFrame *frame)
{
    EV << "ignoring management frame: " << (cMessage *)frame << "\n";
    delete frame;
    numMgmtFramesDropped++;
}

void Ieee80211MgmtBase::sendUp(cMessage *msg)
{
    ASSERT(isOperational);
    send(msg, "upperLayerOut");
}

void Ieee80211MgmtBase::processFrame(Ieee80211DataOrMgmtFrame *frame)
{
    switch (frame->getType()) {
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
            handleReassociationResponseFrame(check_and_cast<Ieee80211ReassociationResponseFrame *>(frame));
            break;

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
            throw cRuntimeError("Unexpected frame type (%s)%s", frame->getClassName(), frame->getName());
    }
}

bool Ieee80211MgmtBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_PHYSICAL_LAYER)
            start();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_PHYSICAL_LAYER)
            stop();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_LOCAL) // crash is immediate
            stop();
    }
    else
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    return true;
}

void Ieee80211MgmtBase::start()
{
    isOperational = true;
}

void Ieee80211MgmtBase::stop()
{
    clear();
    dataQueue.clear();
    int length = 0;
    for (int i = 0; i < numQueues; i++)
    {
        dataQueue[i].clear();
        length += dataQueue[i].length();
    }
    emit(dataQueueLenSignal, length);
    mgmtQueue.clear();
    isOperational = false;
}

} // namespace ieee80211

} // namespace inet

