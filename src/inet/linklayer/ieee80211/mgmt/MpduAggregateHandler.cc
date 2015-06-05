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

#include "inet/linklayer/ieee80211/mgmt/MpduAggregateHandler.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211MpduA.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211MsduA.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"

namespace inet {

namespace ieee80211 {

#define MAXBLOCK 64
#define MINBLOCK 3
#define DEFAULT_BL_ACK 0.01
#define DEFAULT_FALIURE 0.01
#define DEFAULT_RESET_BLOCK 0.02


///
/// Timer methods

void MpduAggregateHandler::addTimer(const MACAddress & addr, const TimerType & type, const double &timeout)
{
    erasePending(addr, type);
    Timer t;
    t.nodeId = addr;
    t.type = type;
    simtime_t time = simTime() + timeout;
    timerQueue.insert(std::make_pair(time,t));
}

void MpduAggregateHandler::checkTimer()
{
    simtime_t now = simTime();
    while(!timerQueue.empty() && timerQueue.begin()->first <= now)
    {
        switch(timerQueue.begin()->second.type)
        {
            case BLOCKTIMEOUT:
                blockTimeOutAction(timerQueue.begin()->second.nodeId);
                break;
            case ADDBAFALIURE:
                addbaFaliureAction(timerQueue.begin()->second.nodeId);
                break;
            case RESETBLOCK:
                resetBlock(timerQueue.begin()->second.nodeId);
            default:
                throw cRuntimeError("MpduAggregateHandler timer type unknown");
        }
        timerQueue.erase(timerQueue.begin());
    }
}

bool MpduAggregateHandler::checkPending(const MACAddress & addr, const TimerType &type)
{
    for (auto &elem : timerQueue)
    {
        if (elem.second.nodeId == addr && elem.second.type == type)
            return true;
    }
    return false;
}

void MpduAggregateHandler::erasePending(const MACAddress &addr, const TimerType &type)
{
    for (auto it = timerQueue.begin(); it !=  timerQueue.end(); ++it)
    {
        if (it->second.nodeId == addr && it->second.type == type)
        {
            timerQueue.erase(it);
            return;
        }
    }
}

void MpduAggregateHandler::blockTimeOutAction(const MACAddress & addr)
{

    ADDBAInfo *addai;
    if (!isAllowAddress(addr, addai))
        return;

    if (addai->state != DEFAULT)
    {
        // reset state
        addai->state = DEFAULT;
        for (unsigned int i = 0; i < categories.size(); i++)
        {
            removeBlock(addr, i);
        }
        sendDELBA(addr);
    }
}

void MpduAggregateHandler::addbaFaliureAction(const MACAddress & addr)
{
    ADDBAInfo *addai;
    if (!isAllowAddress(addr, addai))
           return;

    if (addai->state != WAITCONFIRMATION)
    {
        // reset state
        addai->state = DEFAULT;
    }
}

void MpduAggregateHandler::resetBlock(const MACAddress & addr)
{
    setMacDiscardMpdu(addr);
}

/////////
MpduAggregateHandler::MpduAggregateHandler()
{
    categories.clear();
    listAllowAddress.clear();

    Ieee80211MgmtBase *mgmt = dynamic_cast<Ieee80211MgmtBase*>(this->getOwner());

    categories.resize(mgmt->dataQueue.size());
    queueManagement = &(mgmt->mgmtQueue);

    for (unsigned int i = 0; i < mgmt->dataQueue.size(); i++)
        categories[i].queue = &(mgmt->dataQueue[i]);

}


bool MpduAggregateHandler::isAllowAddress(const MACAddress &add)
{
    if (allAddress)
        return true;
    auto it = listAllowAddress.find(add);
    if (it == listAllowAddress.end())
        return false;
    return true;
}

bool MpduAggregateHandler::isAllowAddress(const MACAddress &add, ADDBAInfo *&addai)
{
    auto it = listAllowAddress.find(add);
    addai = nullptr;
    if (it == listAllowAddress.end())
    {
        if (!allAddress)
            return false;
        else
            return true;
    }
    addai = it->second;
    return true;
}

void MpduAggregateHandler::setADDBAInfo(const MACAddress &addr, ADDBAInfo *p)
{
    auto it = listAllowAddress.find(addr);
    if (it != listAllowAddress.end())
    {
        delete it->second;
        it->second = p;
    }
    else
    {
        listAllowAddress[addr] = p;
    }
}

bool MpduAggregateHandler::checkState(const Ieee80211DataOrMgmtFrame * pkt)
{
    checkTimer();
    return checkState(pkt->getReceiverAddress());
}

bool MpduAggregateHandler::checkState(const MACAddress &addr)
{
    ADDBAInfo *addai;
    if (!isAllowAddress(addr, addai) && !allAddress)
        return false;

    if (addai == nullptr)
    {
        // allAddress == true
        addai = new ADDBAInfo();
        if (!requestProcedure)
            addai->state = SENDBLOCK;
        setADDBAInfo(addr,addai);
    }

    if (automaticMimimumAddress > 0 && addai->state == DEFAULT)
    {
        // check number of free address
        for (unsigned int i = 0; i < categories.size(); i++)
        {
            auto it = categories[i].numFramesDestinationFree.find(addr);
            if (it != categories[i].numFramesDestinationFree.end() && it->second >= automaticMimimumAddress)
            {
                prepareADDBA(addr);
                break;
            }
        }
        return false;
    }

    if (addai->state != WAITBLOCK && addai->state != SENDBLOCK)
        return false;

    bool moreFrames = false;
    if (!perpetual)
    {
        for (unsigned int i = 0; i < categories.size(); i++)
        {
            auto it = categories[i].numFramesDestination.find(addr);
            if (it != categories[i].numFramesDestination.end())
            {
                moreFrames = true;
                break;
            }
        }
    }
    else
        moreFrames = true;

    if (addai->state == SENDBLOCK)
    {
        if (!moreFrames) // no more frames, send DELBA and reset state
        {
            // reset state
            sendDELBA (addr);
            erasePending(addr, BLOCKTIMEOUT);
            addai->state = DEFAULT;

        }
    }
    return true;
}


void MpduAggregateHandler::increaseSize(Ieee80211DataOrMgmtFrame* val, int cat)
{
    Ieee80211MpduA * block = dynamic_cast<Ieee80211MpduA *>(val);
    auto it = categories[cat].numFramesDestination.find(val->getReceiverAddress());
    if (it == categories[cat].numFramesDestination.end())
    {
        categories[cat].numFramesDestination[val->getReceiverAddress()] = 0;
        it = categories[cat].numFramesDestination.find(val->getReceiverAddress());
    }

    auto it2 = categories[cat].numFramesDestinationFree.find(val->getReceiverAddress());
    if (it2 == categories[cat].numFramesDestinationFree.end() && block == nullptr)
    {
        categories[cat].numFramesDestinationFree[val->getReceiverAddress()] = 0;
        it2 = categories[cat].numFramesDestinationFree.find(val->getReceiverAddress());
    }

    if (block) {
        categories[cat].queueSize += block->getNumEncap();
        it->second += block->getNumEncap();
    }
    else {
        categories[cat].queueSize++;
        it->second++;
        it2->second++;
    }
}

void MpduAggregateHandler::decreaseSize(Ieee80211DataOrMgmtFrame* val, int cat)
{
    Ieee80211MpduA * block = dynamic_cast<Ieee80211MpduA *>(val);
    auto it = categories[cat].numFramesDestination.find(val->getReceiverAddress());
    if (it == categories[cat].numFramesDestination.end())
        throw cRuntimeError("Multi queue error address not found");

    auto it2 = categories[cat].numFramesDestinationFree.find(val->getReceiverAddress());
    if (it2 == categories[cat].numFramesDestinationFree.end() && block == nullptr)
        throw cRuntimeError("Multi queue error address not found");

    if (block) {
        categories[cat].queueSize -= block->getNumEncap();
        it->second -= block->getNumEncap();
    }
    else {
        categories[cat].queueSize--;
        it->second--;
        it2->second--;
        if (it2->second < 0)
                throw cRuntimeError("Multi queue error address2 not size error");
        if (it2->second == 0)
            categories[cat].numFramesDestinationFree.erase(it2);
    }

    if (it->second < 0)
        throw cRuntimeError("Multi queue error address not size error");

    if (it->second == 0)
        categories[cat].numFramesDestination.erase(it);
}

MpduAggregateHandler::~MpduAggregateHandler()
{

}

int MpduAggregateHandler::findAddress(const MACAddress &addr ,int cat)
{
    if (addr.isMulticast() || addr.isBroadcast())
        return 0;
    if (cat == -1)
        cat = categories.size()-1;
    auto it = categories[cat].numFramesDestination.find(addr);
    if (it != categories[cat].numFramesDestination.end())
        return it->second;
    return 0;
}

int MpduAggregateHandler::findAddressFree(const MACAddress &addr ,int cat)
{
    if (addr.isMulticast() || addr.isBroadcast())
        return 0;
    if (cat == -1)
        cat = categories.size()-1;
    auto it = categories[cat].numFramesDestinationFree.find(addr);
    if (it != categories[cat].numFramesDestinationFree.end())
        return it->second;
    return 0;
}

Ieee80211MpduA* MpduAggregateHandler::getBlock(Ieee80211DataFrame *frame,int maxPkSize, int64_t maxByteSize, int cat, int minSize)
{

    if (maxByteSize == -1)
    {
        maxByteSize = 65535;
        ADDBAInfo *infoAdda;
        isAllowAddress(frame->getReceiverAddress(), infoAdda);
        if (infoAdda)
            maxByteSize = exp2(13+infoAdda->exponent)-1;
    }

    Ieee80211MpduA* block = getBlock(frame->getReceiverAddress(),maxPkSize-1, maxByteSize - frame->getByteLength(),cat , minSize);
    if (block)
    {
        block->pushFront(frame);
        return block;
    }
    return nullptr;
}

Ieee80211MpduA* MpduAggregateHandler::getBlock(const MACAddress &addr,int maxPkSize,int64_t maxByteSize, int cat, int minSize)
{
    if (cat >= (int) categories.size())
         throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    if (addr.isMulticast() || addr.isBroadcast())
       return nullptr;

    if (cat == -1)
        cat = categories.size()-1;

    if (categories[cat].queue->empty())
        return nullptr;

    ADDBAInfo *infoAdda;
    if (!isAllowAddress(addr, infoAdda))
        return nullptr;

    if (infoAdda == nullptr)
    {
        // allAddress == true
        infoAdda = new ADDBAInfo();
        if (!requestProcedure)
            infoAdda->state = SENDBLOCK;
        setADDBAInfo(addr,infoAdda);
    }

    if (infoAdda->state != SENDBLOCK)
        return nullptr;

    auto itDest2 = categories[cat].numFramesDestinationFree.find(addr);
    if (itDest2 == categories[cat].numFramesDestinationFree.end())
        return nullptr; // non free

    if (minSize >= 0 && itDest2->second < minSize)
        return nullptr; // not enough free

    Ieee80211MpduA *block = nullptr;

    for (auto it = categories[cat].queue->begin() ;it != categories[cat].queue->end();)
    {
        Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame*>(*it);
        if (frame == nullptr || frame->getReceiverAddress().compareTo(addr) != 0)
        {
            ++it;
            continue;
        }

        if (!block)
        {
            block = new Ieee80211MpduA();
            block->setReceiverAddress(addr);
            if (block->getOwner() == this)
                    drop(block);
        }

    // check sizes
        if (block->getByteLength() + frame->getByteLength() >= maxByteSize)
            break;

        if (block->getEncapSize()+1 > (unsigned int)maxPkSize)
            break;

        it = categories[cat].queue->erase(it);
        take(frame);
        drop(frame);
        block->pushBack(frame);
        itDest2->second--;
        if (itDest2->second <= 0)
        {
            categories[cat].numFramesDestinationFree.erase(itDest2);
            break;
        }
    }
    return block;
}


void MpduAggregateHandler::createBlocks(const MACAddress &addr, int cat)
{

    if (cat >= (int) categories.size())
         throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    if (addr.isMulticast() || addr.isBroadcast())
       return;

    if (cat == -1)
        cat = categories.size()-1;

    if (categories[cat].queue->empty())
        return;

    Ieee80211MpduA *block = nullptr;

    auto itDest2 = categories[cat].numFramesDestinationFree.find(addr);
    if (itDest2 == categories[cat].numFramesDestinationFree.end())
        return; // non free

    int64_t maxSize = 65535;
    auto itInfo = listAllowAddress.find(addr);
    if (itInfo != listAllowAddress.end())
        maxSize = exp2(13+itInfo->second->exponent)-1;


    auto itAux = categories[cat].queue->end();
    for (auto it = categories[cat].queue->begin() ;it != categories[cat].queue->end();)
    {
        Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame*>(*it);
        if (frame == nullptr || frame->getReceiverAddress().compareTo(addr) != 0)
        {
            ++it;
            continue;
        }


        if (!block)
        {
            block = new Ieee80211MpduA();
            block->setReceiverAddress(addr);
            if (itAux == categories[cat].queue->end())
                *it = block;
            else
            {
                ++itAux;
                if (block->getOwner() == this)
                    drop(block);

                if (block->getByteLength() + frame->getByteLength() >= maxSize)
                {
                    itAux = it;
                    block = nullptr;
                    if (itDest2->second < MINBLOCK)
                        return;
                }
                categories[cat].queue->insert(itAux,block);
                it = categories[cat].queue->erase(it);
            }
        }
        else
            it = categories[cat].queue->erase(it);
        block->pushBack(frame);
        itDest2->second--;
        if (itDest2->second <= 0)
        {
            categories[cat].numFramesDestinationFree.erase(itDest2);
            return;
        }

        if (block->getEncapSize() >= MAXBLOCK)
        {
            itAux = it;
            block = nullptr;
            if (itDest2->second < MINBLOCK)
                return;
        }
    }
}


void MpduAggregateHandler::removeBlock(const MACAddress &addr, int cat)
{

    if (cat >= (int) categories.size())
         throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    if (addr.isMulticast() || addr.isBroadcast())
       return;

    if (cat == -1)
        cat = categories.size()-1;

    if (categories[cat].queue->empty())
        return;

    auto itDest = categories[cat].numFramesDestination.find(addr);
    if (itDest == categories[cat].numFramesDestination.end())
        return;
    if (itDest->second <= 0)
    {
        categories[cat].numFramesDestination.erase(itDest);
        return; // no frames
    }
    // search in the queue
    for (auto it = categories[cat].queue->begin() ;it != categories[cat].queue->end();++it)
    {
        Ieee80211MpduA *frame = dynamic_cast<Ieee80211MpduA*>(*it);
        if (frame == nullptr || frame->getReceiverAddress().compareTo(addr) != 0)
        {
            ++it;
            continue;
        }
        auto itDest2 = categories[cat].numFramesDestinationFree.find(addr);
        if (itDest2 == categories[cat].numFramesDestinationFree.end())
        {
            categories[cat].numFramesDestinationFree[addr] = 0;
            itDest2 = categories[cat].numFramesDestinationFree.find(addr);
        }
        while (frame->getEncapSize() > 1)
        {
            Ieee80211DataOrMgmtFrame *pkt = frame->popFront();
            // check ownership
            if (pkt->getOwner() == this)
                drop(pkt);
            categories[cat].queue->insert(it,pkt);
            itDest2->second++;
        }
        *it = frame->popFront();
        if ((*it)->getOwner() == this)
            drop(*it);
        itDest2->second++;
        //if (frame->getOwner() != this)
        //    take(frame);
        delete frame;
    }
}

void MpduAggregateHandler::prepareADDBA(const MACAddress &addr)
{

    ADDBAInfo *infoAdda = nullptr;
    if (!allAddress && !isAllowAddress(addr,infoAdda))
        return;

    if (infoAdda != nullptr)
    {
        if (infoAdda->state != DEFAULT)
            return; // nothing to do
    }
    else
    {
        throw cRuntimeError("ADDAInfo not found");
    }

    if (findAddressFree(addr) >= MINBLOCK)
    {
        // create ADDBA frame
        Ieee80211ActionBlockAckADDBA * addbaFrame = new Ieee80211ActionBlockAckADDBA();
        Ieee80211ADDBAFrameBody body;
        body.setAction(ADDBArequest);
        body.setTimeOut(1000);
        Ieee80211DataOrMgmtFrame *frame = new Ieee80211DataOrMgmtFrame();
        frame->encapsulate(addbaFrame);
        frame->setReceiverAddress(addr);
        if (frame->getOwner() == this)
            drop(frame);
        queueManagement->push_back(frame);

        addTimer(addr, ADDBAFALIURE, DEFAULT_FALIURE);

        return;
    }
}



bool MpduAggregateHandler::handleFrames(Ieee80211DataOrMgmtFrame *pkt)
{
    checkState(pkt);
    bool takePkt = false;
    if (handleADDBA(pkt))
        takePkt = true;
    else if (dynamic_cast<Ieee80211ActionBlockAckDELBA *>(pkt->getEncapsulatedPacket()))
    {
        MACAddress addr = pkt->getTransmitterAddress();
        setMacDiscardMpdu(addr);
        //if (pkt->getOwner() != this) // check ownership
        //    this->take(pkt);
        delete pkt;
        takePkt = true;
    }
    return takePkt;
}


bool MpduAggregateHandler::handleADDBA(Ieee80211DataOrMgmtFrame *pkt)
{
    Ieee80211ActionBlockAckADDBA * addbaFrame = dynamic_cast<Ieee80211ActionBlockAckADDBA *>(pkt->getEncapsulatedPacket());
    if (addbaFrame == nullptr)
        return false;
    // extract data
    MACAddress addr = pkt->getTransmitterAddress();
    if (addbaFrame->getBody().getAction() == ADDBArequest)
    {
        pkt->setReceiverAddress(pkt->getTransmitterAddress());
        addbaFrame->getBody().setAction(ADDBAReponse);
        if (pkt->getOwner() == this)
            drop(pkt);
        queueManagement->push_back(pkt);
        erasePending(addr, RESETBLOCK);
        addTimer(addr, RESETBLOCK, DEFAULT_RESET_BLOCK);
        setMacAcceptMpdu(addr);
    }
    else if (addbaFrame->getBody().getAction() == ADDBAReponse)
    {
        ADDBAInfo *infoAdda;
        if (!isAllowAddress(addr, infoAdda))
            throw cRuntimeError("ADDBAInfo not found");

        infoAdda->state = SENDBLOCK;
        infoAdda->bytesSend = 0;
        infoAdda->startBlockAck = simTime();

        erasePending(addr, ADDBAFALIURE);

        if (!perpetual)
            addTimer(addr, BLOCKTIMEOUT, DEFAULT_BL_ACK);

        //if (pkt->getOwner() != this)
        //    take(pkt);
        delete pkt;
    }
    return true;
}

void MpduAggregateHandler::sendDELBA(const MACAddress &addr)
{
    Ieee80211ActionBlockAckDELBA * addbaFrame = new Ieee80211ActionBlockAckDELBA();
    Ieee80211DELBAFrameBody body;
    Ieee80211DataOrMgmtFrame *frame = new Ieee80211DataOrMgmtFrame();
    frame->encapsulate(addbaFrame);
    frame->setReceiverAddress(addr);
    if (frame->getOwner() == this)
        drop(frame);
    queueManagement->push_back(frame);
}


void MpduAggregateHandler::setMacAcceptMpdu(const MACAddress &addr)
{
    auto it = listAllowAddress.find(addr);
    if (it == listAllowAddress.end())
    {
        ADDBAInfo *infoAdda = new ADDBAInfo();
        if (!requestProcedure)
            infoAdda->state = SENDBLOCK;
        listAllowAddress[addr] = infoAdda;
    }
}

void MpduAggregateHandler::setMacDiscardMpdu(const MACAddress &addr)
{
    auto it = listAllowAddress.find(addr);
    if (it != listAllowAddress.end())
    {
        delete it->second;
        listAllowAddress.erase(it);
    }
}

bool MpduAggregateHandler::setMsduA(Ieee80211DataFrame * frame, const int &cat)
{

    return false;

    // check if other frames with the same destination are available.
    auto itDest = categories[cat].numFramesDestination.find(frame->getReceiverAddress());
    if (itDest == categories[cat].numFramesDestination.end())
        return false; // no frames

    if (itDest->second <= 0)
    {
        categories[cat].numFramesDestination.erase(itDest);
        return false; // no frames
    }
    MACAddress dest = frame->getReceiverAddress();
    MACAddress addr3 = frame->getAddress3();
    MACAddress addr4 = frame->getAddress4();


    // if frames search for frames of the same type,
    // search frames to the same destination and characteristics

    int64_t maxSize = 7000;

    // search for msda-a in the queues
    for (auto it = categories[cat].queue->begin() ;it != categories[cat].queue->end();)
    {
        Ieee80211MsduA *frameMsda = dynamic_cast<Ieee80211MsduA*>(*it); // check if msdu-a
        if (frameMsda == nullptr)
        {
            ++it;
            continue;
        }

        if (frameMsda->getByteLength() + frame->getByteLength() >= maxSize)
        {
            ++it;
            continue;
        }

        MACAddress destAux = frameMsda->getReceiverAddress();
        MACAddress addr3Aux = frameMsda->getAddress3();
        MACAddress addr4Aux = frameMsda->getAddress4();
        // Compare addresses
        if (destAux.compareTo(dest) != 0)
        {
            ++it;
            continue;
        }
        // add to the msdu-a
        // decapsulate and recapsulate
        // it is necessary to include padding bytes in the latest frame
        cPacket *pkt = frame->decapsulate();
        if (dynamic_cast<Ieee80211MeshFrame *>(frame))
        {
            Ieee80211MsduAMeshSubframe * header = new Ieee80211MsduAMeshSubframe();
            Ieee80211DataFrame *aux = header;
            *aux = (*frame);

        }
        else
        {
            Ieee80211MsduASubframe * header = new Ieee80211MsduASubframe();
            Ieee80211DataFrame *aux = header;
            *aux = (*frame);
        }


        return true;
    }

    // search for other frame and create an Msdu-a
    for (auto it = categories[cat].queue->begin() ;it != categories[cat].queue->end();)
    {
        Ieee80211MsduA *frameMsda= dynamic_cast<Ieee80211MsduA*>(*it); // check if msdu-a
        if (frameMsda != nullptr)
        {
            ++it;
            continue;
        }
        Ieee80211DataFrame * frameAux = (Ieee80211DataFrame *)(*it);
        MACAddress destAux = frameAux->getReceiverAddress();
        MACAddress addr3Aux = frameAux->getAddress3();
        MACAddress addr4Aux = frameAux->getAddress4();
        // Compare addresses
        if (destAux.compareTo(dest) != 0 || addr3Aux.compareTo(addr3) != 0 || addr4Aux.compareTo(addr4) != 0 )
        {
            ++it;
            continue;
        }

        frameMsda = new Ieee80211MsduA();
        frameMsda->setReceiverAddress(destAux);
        frameMsda->setAddress3(addr3Aux);
        frameMsda->setAddress4(addr4Aux);
        drop(frameMsda);
        return true;
    }


    return false;
}


} // namespace ieee80211

} // namespace inet
