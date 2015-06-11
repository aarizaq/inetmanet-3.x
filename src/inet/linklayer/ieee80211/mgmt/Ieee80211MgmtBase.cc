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
#include "inet/linklayer/ieee80211/mgmt/MpduAggregateHandler.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211MsduAContainer.h"

#include <string>

namespace inet {

namespace ieee80211 {

simsignal_t Ieee80211MgmtBase::dataQueueLenSignal = registerSignal("dataQueueLen");

static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

Ieee80211MgmtBase::~Ieee80211MgmtBase()
{
    clear();
    if (mpduAggregateHandler)
        delete mpduAggregateHandler;
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
        packetRequestedCat.resize(numQueues);
        // mgmtQueue.setName("wlanMgmtQueue");
        int length = 0;
        if (numQueues == 1)
        {
            // dataQueue[0].setName("wlanDataQueue");
            length = dataQueue[0].size();
        }
        else
        {

            std::string str;
            for (int i = 0; i < numQueues; i++)
            {
                str = "wlanDataQueue" + std::to_string(i);
                // dataQueue[i].setName(str.c_str());
                length += dataQueue[i].size();
            }
            for (int i = 0; i < numQueues - 1 ; i++)
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
        if (par("aggregateHandler").boolValue())
        {
            mpduAggregateHandler = new MpduAggregateHandler();
            // for testing
            mpduAggregateHandler->setAllAddress(true);
            mpduAggregateHandler->setRequestProcedure(false); // disable request procedure for testing
            mpduAggregateHandler->setAutomaticMimimumAddress(minMpduASize);
        }
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        // obtain our address from MAC
        cModule *mac = getModuleFromPar<cModule>(par("macModule"), this);
        myAddress.setAddress(mac->par("address").stringValue());
    }
}

Ieee80211DataOrMgmtFrame *Ieee80211MgmtBase::fromMsduAFrameToMsduA(Ieee80211DataOrMgmtFrame *frameToProc)
{
    // check first encapsulate packet

    if (!frameToProc)
        return nullptr;
    if (dynamic_cast<Ieee80211MsduAFrame *>(frameToProc) == nullptr && dynamic_cast<Ieee80211MsduAMeshFrame *>(frameToProc) == nullptr)
        return nullptr;

    std::vector<cPacket *> stack;
    std::vector<cPacket *> stackAux;

    cPacket *pkt = frameToProc->decapsulate();
    while (pkt)
    {
        cPacket *pktAux =  pkt->decapsulate();
        stack.push_back(pkt);
        pkt =  pktAux;
    }

    while (!stack.empty())
    {
        cPacket *pktAux =  stack.back();
        stack.pop_back();
        if (dynamic_cast<Ieee80211MsduAMeshSubframe *>(pktAux) == nullptr && dynamic_cast<Ieee80211MsduASubframe *>(pktAux) == nullptr)
        {
            stack.back()->encapsulate(pktAux);
        }
        else
            stackAux.push_back(pktAux);
    }

    Ieee80211MsduAContainer *mpduA = new Ieee80211MsduAContainer();
    bool isMesh = dynamic_cast<Ieee80211MeshFrame *>(frameToProc) != nullptr;

     if (isMesh)
        *((Ieee80211MeshFrame*)mpduA) = *((Ieee80211MeshFrame*)frameToProc);
    else
        *((Ieee80211DataFrameWithSNAP*)mpduA) = *((Ieee80211DataFrameWithSNAP*)frameToProc);
    while (!stackAux.empty())
    {
        mpduA->pushBack((Ieee80211DataFrame *)stackAux.back());
        stackAux.pop_back();
    }
    delete frameToProc;
    return mpduA;
}

Ieee80211DataFrame *Ieee80211MgmtBase::fromMsduAToMsduAFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211MsduAContainer *msduA = dynamic_cast<Ieee80211MsduAContainer *>(frameToSend);
    if (msduA == nullptr)
    {
        return nullptr;
    }

    std::vector<cPacket *> stack;
    std::vector<cPacket *> stackAux;
    bool isMesh = dynamic_cast<Ieee80211MsduASubframe *>(msduA->getPacket(0)) == nullptr;

    while (msduA->getNumEncap() > 0)
        stackAux.push_back(msduA->popBack());

    cPacket *pkt;
    while (!stackAux.empty())
    {
        pkt = stackAux.back();
        stackAux.pop_back();
        do{
            cPacket *pktAux =  pkt->decapsulate();
            stack.push_back(pkt);
            pkt =  pktAux;
        } while (pkt != nullptr);
    }
    // recapsulate

    while(!stack.empty())
    {
        pkt = stack.back();
        stack.pop_back();
        if (!stack.empty())
            stack.back()->encapsulate(pkt);
    }

    Ieee80211DataFrame *frame = nullptr;
    if (!isMesh)
    {
        // normal data
        Ieee80211MsduAFrame *frameAux = new Ieee80211MsduAFrame();
        uint64_t l = frameAux->getByteLength();
        *frameAux = *(Ieee80211MsduAFrame *)msduA;
        frameAux->setByteLength(l);
        frame = frameAux;
    }
    else
    {
        Ieee80211MsduAMeshFrame *frameAux = new Ieee80211MsduAMeshFrame();
        uint64_t l = frameAux->getByteLength();
        *frameAux = *(Ieee80211MsduAMeshFrame *)msduA;
        frameAux->setByteLength(l);
        frame = frameAux;
        // mesh data
    }
    frame->encapsulate(pkt);
    return frame;
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
        frame->setKind(cat);
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

    if (hasGUI())
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
    Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
    ASSERT(frame != nullptr);
    bool isDataFrame = dynamic_cast<Ieee80211DataFrame *>(msg) != nullptr;
    int cat = 0;

    if (numQueues > 1 && isDataFrame)
    {
        cat = classifier->classifyPacket(msg);
    }

    if (!isDataFrame) {
        // management frames are inserted into mgmtQueue
        mgmtQueue.push_back(frame);
        return nullptr;
    }
    else if (frameCapacity && (int) dataQueue[cat].size() >= frameCapacity) {
        EV << "Queue full, dropping packet.\n";
        return msg;
    }
    else {
        bool includedInMsduA = false;
        // if broadcast and musticast frames must be transmit before
        if ((frame->getReceiverAddress().isBroadcast() || frame->getReceiverAddress().isMulticast()))
        {
            if (!dataQueue[cat].empty() && !dataQueue[cat].back()->getReceiverAddress().isBroadcast() && !dataQueue[cat].back()->getReceiverAddress().isMulticast())
            {
                // search for the first non multicast frame
                auto it = dataQueue[cat].begin();
                for (;it != dataQueue[cat].end();++it)
                {
                    if (!(*it)->getReceiverAddress().isBroadcast() && !(*it)->getReceiverAddress().isMulticast())
                        break;
                }
                dataQueue[cat].insert(it,frame);
            }
            else
                dataQueue[cat].push_back(frame);
        }
        else
        {
            if (mpduAggregateHandler)
            {
                includedInMsduA = mpduAggregateHandler->setMsduA((Ieee80211DataFrame *)frame,cat); // it is safe, the frame must be Ieee80211DataFrame if the core arrive here
                if (!includedInMsduA)
                    dataQueue[cat].push_back(frame);
            }
            else
                dataQueue[cat].push_back(frame);
        }
        int length = 0;
        for (int i = 0; i < numQueues; i++)
            length += dataQueue[i].size();
        if (mpduAggregateHandler && !includedInMsduA)
            mpduAggregateHandler->increaseSize(frame,cat);

        emit(dataQueueLenSignal, length);
        return nullptr;
    }
}


cMessage *Ieee80211MgmtBase::enqueue(cMessage *msg, const int &cat)
{
    Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
    ASSERT(frame != nullptr);
    bool isDataFrame = dynamic_cast<Ieee80211DataFrame *>(msg) != nullptr;

    if (!isDataFrame) {
        // management frames are inserted into mgmtQueue
        mgmtQueue.push_back(frame);
        return nullptr;
    }
    else if (frameCapacity && (int) dataQueue[cat].size() >= frameCapacity) {
        EV << "Queue full, dropping packet.\n";
        return msg;
    }
    else {
        bool includedInMsduA = false;
        // if broadcast and musticast frames must be transmit before
        if ((frame->getReceiverAddress().isBroadcast() || frame->getReceiverAddress().isMulticast()))
        {
            if (!dataQueue[cat].empty() && !dataQueue[cat].back()->getReceiverAddress().isBroadcast() && !dataQueue[cat].back()->getReceiverAddress().isMulticast())
            {
                // search for the first non multicast frame
                auto it = dataQueue[cat].begin();
                for (;it != dataQueue[cat].end();++it)
                {
                    if (!(*it)->getReceiverAddress().isBroadcast() && !(*it)->getReceiverAddress().isMulticast())
                        break;
                }
                dataQueue[cat].insert(it,frame);
            }
            else
                dataQueue[cat].push_back(frame);
        }
        else
        {
            if (mpduAggregateHandler)
            {
                includedInMsduA = mpduAggregateHandler->setMsduA((Ieee80211DataFrame *)frame,cat); // it is safe, the frame must be Ieee80211DataFrame if the core arrive here, return true if the frame is included in a msdu-A frame.
                if (!includedInMsduA)
                    dataQueue[cat].push_back(frame);
            }
            else
                dataQueue[cat].push_back(frame);
        }
        int length = 0;
        for (int i = 0; i < numQueues; i++)
            length += dataQueue[i].size();
        if (mpduAggregateHandler && !includedInMsduA)
            mpduAggregateHandler->increaseSize(frame,cat);

        emit(dataQueueLenSignal, length);
        return nullptr;
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
    Ieee80211DataOrMgmtFrame *pk = nullptr;
    // management frames have priority
    if (!mgmtQueue.empty())
    {
        pk = mgmtQueue.front();
        mgmtQueue.pop_front();
    }
    else if (!dataQueue[0].empty())
    {
        // return a data frame if we have one
        pk =  dataQueue[0].front();
        dataQueue[0].pop_front();
        // statistics
        int length = 0;
        for (int i = 0; i < numQueues; i++)
            length += dataQueue[i].size();
        emit(dataQueueLenSignal, length);
        if (mpduAggregateHandler)
            mpduAggregateHandler->decreaseSize(pk,0);


    }
    if (mpduAggregateHandler && pk != nullptr)
    {
        mpduAggregateHandler->checkState(pk);
    }
    return pk;
}

void Ieee80211MgmtBase::requestPacket()
{
    Enter_Method("requestPacket(int)");

    bool dataFrame = mgmtQueue.empty();
    cMessage *msg = dequeue();
    if (msg == nullptr) {
        packetRequested++;
    }
    else {
        emit(dequeuePkSignal, msg);
        emit(queueingTimeSignal, simTime() - msg->getArrivalTime());
        if (mpduAggregateHandler && dataFrame)
        {
            Ieee80211DataFrame * frame = dynamic_cast<Ieee80211DataFrame*>(msg);
            if (frame)
            {
                if (mpduAggregateHandler->isAllowAddress(frame->getReceiverAddress()))
                {
                    cMessage * aux = mpduAggregateHandler->getBlock(frame,64,-1,0,minMpduASize);
                    if (aux)
                        msg = aux;
                }

            }
        }
        msg->setKind(0);
        sendOut(msg);
    }
}


void Ieee80211MgmtBase::requestPacket(const int &cat)
{
    Enter_Method("requestPacket(int)");

    bool dataFrame = mgmtQueue.empty();
    cMessage *msg = dequeue(cat);
    if (msg == nullptr) {
        if (cat == 0)
            packetRequested++;
        else
            packetRequestedCat[cat - 1]++;
    }
    else {
        emit(dequeuePkSignal, msg);
        emit(queueingTimeSignal, simTime() - msg->getArrivalTime());
        if (mpduAggregateHandler && dataFrame)
        {
            Ieee80211DataFrame * frame = dynamic_cast<Ieee80211DataFrame*>(msg);
            if (frame)
            {
                if (mpduAggregateHandler->isAllowAddress(frame->getReceiverAddress()))
                {
                    cMessage * aux = mpduAggregateHandler->getBlock(frame,64,-1,cat,minMpduASize);
                    if (aux)
                        msg = aux;
                }

            }
        }
        msg->setKind(cat);
        sendOut(msg);
    }
}

cMessage *Ieee80211MgmtBase::requestMpuA(const MACAddress &addr, const int &size, const int64_t &remanent, const int &cat)
{
    if (mpduAggregateHandler)
    {

        MpduAggregateHandler::ADDBAInfo *infoAdda;
        if (mpduAggregateHandler->isAllowAddress(addr, infoAdda))
        {
            int64_t maxByteSize = 65535;
            if (infoAdda)
                maxByteSize = exp2(13+infoAdda->exponent)-1;
            maxByteSize -= remanent;
            cMessage * msg = mpduAggregateHandler->getBlock(addr,size,maxByteSize,cat,-1);
            if (msg)
            {
                if (msg->getOwner() == this)
                    drop(msg);
                return msg;
            }
        }
    }
    return nullptr;
}

cMessage *Ieee80211MgmtBase::dequeue(const int & cat)
{
    cMessage *pk = nullptr;
    // management frames have priority
    if (!mgmtQueue.empty())
    {
        pk = (cMessage *)mgmtQueue.front();
        mgmtQueue.pop_front();
    }
    else if (!dataQueue[cat].empty())
    {
        pk = (cMessage *) dataQueue[cat].front();
        dataQueue[0].pop_front();
        // statistics
        int length = 0;
        for (int i = 0; i < numQueues; i++)
            length += dataQueue[i].size();
        if (mpduAggregateHandler)
        {
            Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(pk);
            if (frame)
                mpduAggregateHandler->decreaseSize(frame,cat);
        }


        emit(dataQueueLenSignal, length);
    }
    return pk;
}

void Ieee80211MgmtBase::sendOut(cMessage *msg)
{
    ASSERT(isOperational);
    // check if MsduA
    Ieee80211MpduAContainer * mpdu = dynamic_cast<Ieee80211MpduAContainer *>(msg);
    if (mpdu)
    {
        // search from Msdu and convert it
        for (unsigned int i = 0; i < mpdu->getNumEncap(); i++)
        {
            Ieee80211MsduAContainer *msduAContainer = dynamic_cast<Ieee80211MsduAContainer *>(mpdu->getPacket(i));
            if (msduAContainer)
                mpdu->replacePacket(i,fromMsduAToMsduAFrame(msduAContainer));
        }
    }
    else if (dynamic_cast<Ieee80211MsduAContainer *>(msg))
    {
        Ieee80211DataOrMgmtFrame *frameAux = fromMsduAToMsduAFrame((Ieee80211DataOrMgmtFrame *)msg);
        frameAux->setKind(msg->getKind());
        delete msg;
        msg = frameAux;
    }
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
    if (mpduAggregateHandler && frame != nullptr)
    {
        if (mpduAggregateHandler->handleFrames(frame))
            return;
    }
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

void Ieee80211MgmtBase::clear()
{
    cMessage *msg;
    for (unsigned int i = 0 ; i < dataQueue.size(); i++)
    {
        while (nullptr != (msg = dequeue(i)))
            delete msg;
        packetRequestedCat[i] = 0;
    }

    for (int i = 0 ; i < numQueues - 1; i++)
        packetRequestedCat[i] = 0;
    packetRequested = 0;
}

void Ieee80211MgmtBase::clear(const int & category)
{
    cMessage *msg;
    while (nullptr != (msg = dequeue(category)))
          delete msg;
    if (category == 0)
        packetRequested = 0;
    else
        packetRequestedCat[category -1] = 0;
}


void Ieee80211MgmtBase::stop()
{
    clear();
    dataQueue.clear();
    int length = 0;
    for (int i = 0; i < numQueues; i++)
    {
        dataQueue[i].clear();
        length += dataQueue[i].size();
    }
    emit(dataQueueLenSignal, length);
    mgmtQueue.clear();
    isOperational = false;
}

Ieee80211DataOrMgmtFrame * Ieee80211MgmtBase::getQueueElement(const int &cat, const int &pos) const
{
    if (cat > (int) dataQueue.size())
        return nullptr;
    if (pos < (int) mgmtQueue.size())
    {
        return mgmtQueue.at(pos);
    }
    if (pos >= (int) mgmtQueue.size() + (int) dataQueue[cat].size())
        throw cRuntimeError("queue position doesn't exist");
    return dataQueue[cat].at(pos-mgmtQueue.size());

}

unsigned int Ieee80211MgmtBase::getDataSize(const int &cat) const
{
    return dataQueue[cat].size();
}

unsigned int Ieee80211MgmtBase::getManagementSize() const
{
    return mgmtQueue.size();
}

cMessage * Ieee80211MgmtBase::pop(const int& cat) {
    cMessage *msg = dataQueue[cat].front();
    dataQueue[cat].pop_front();
    if (mpduAggregateHandler)
    {
        Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
        if (frame != nullptr)
            mpduAggregateHandler->decreaseSize(frame,cat);
    }
    return msg;
}

} // namespace ieee80211

} // namespace inet

