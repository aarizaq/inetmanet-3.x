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
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h"

namespace inet {

namespace ieee80211 {

#define MAXBLOCK 16
#define MINBLOCK 3
#define DEFAULT_BL_ACK 0.001
#define DEFAULT_FALIURE 0.001

MpduAggregateHandler::MpduAggregateHandler()
{
    categories.clear();
    categories.resize(2);
    listAllowAddress.clear();
    allAddress = false;
    resetAfterSend = true;
}


bool MpduAggregateHandler::isAllowAddress(const MACAddress &add)
{
    std::map<MACAddress,ADDBAInfo *>::iterator it = listAllowAddress.find(add);
    if (it == listAllowAddress.end())
        return false;
    return true;
}

bool MpduAggregateHandler::isAllowAddress(const MACAddress &add, ADDBAInfo *&addai)
{
    std::map<MACAddress,ADDBAInfo *>::iterator it = listAllowAddress.find(add);
    addai = nullptr;
    if (it == listAllowAddress.end())
        return false;
    addai = it->second;
    return true;
}

void MpduAggregateHandler::setADDBAInfo(const MACAddress &addr, ADDBAInfo *p)
{
    std::map<MACAddress,ADDBAInfo *>::iterator it = listAllowAddress.find(addr);
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
    return checkState(pkt->getReceiverAddress());
}

bool MpduAggregateHandler::checkState(const MACAddress &add)
{

    ADDBAInfo *addai;
    if (!isAllowAddress(add, addai))
        return false;

    if (addai->state != WAITBLOCK && addai->state != SENDBLOCK)
        return false;

    double blockAckTimeout = DEFAULT_BL_ACK;
    double aDDBAFailureTimeout = DEFAULT_FALIURE;

    std::map<MACAddress,ADDBAInfo *>::iterator it = listAllowAddress.find(add);
    unsigned short BlockAckTimeout;
    unsigned short ADDBAFailureTimeout;
    if (it == listAllowAddress.end())
        return false;

    if (addai->state == WAITBLOCK && simTime() - addai->startBlockAck > blockAckTimeout)
    {
        // reset state

    }
    else if (addai->state == SENDBLOCK && - addai->startBlockAck > blockAckTimeout )
    {
        // reset state

    }
    else if (addai->state == SENDBLOCK)
    {
        // check if more block are possible
    }

    return false;
}


void MpduAggregateHandler::increaseSize(Ieee80211DataOrMgmtFrame* val, int cat)
{
    Ieee80211MpduA * block = dynamic_cast<Ieee80211MpduA *>(val);
    NumFramesDestination::iterator it = categories[cat].numFramesDestination.find(val->getReceiverAddress());
    if (it == categories[cat].numFramesDestination.end())
    {
        categories[cat].numFramesDestination[val->getReceiverAddress()] = 0;
        it = categories[cat].numFramesDestination.find(val->getReceiverAddress());
    }

    NumFramesDestination::iterator it2 = categories[cat].numFramesDestinationFree.find(val->getReceiverAddress());
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
    NumFramesDestination::iterator it = categories[cat].numFramesDestination.find(val->getReceiverAddress());
    if (it == categories[cat].numFramesDestination.end())
        throw cRuntimeError("Multi queue error address not found");

    NumFramesDestination::iterator it2 = categories[cat].numFramesDestinationFree.find(val->getReceiverAddress());
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
    NumFramesDestination::iterator it = categories[cat].numFramesDestination.find(addr);
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
    NumFramesDestination::iterator it = categories[cat].numFramesDestinationFree.find(addr);
    if (it != categories[cat].numFramesDestinationFree.end())
        return it->second;
    return 0;
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

    NumFramesDestination::iterator itDest2 = categories[cat].numFramesDestinationFree.find(addr);
    if (itDest2 != categories[cat].numFramesDestinationFree.end())
        return; // non free


    DataQueue::iterator itAux = categories[cat].queue->end();
    for (DataQueue::iterator it = categories[cat].queue->begin() ;it != categories[cat].queue->end();)
    {
        Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame*>(*it);
        if (frame == nullptr || !frame->getReceiverAddress().compareTo(addr))
        {
            ++it;
            continue;
        }


        if (!block)
        {
            block = new Ieee80211MpduA();
            if (itAux == categories[cat].queue->end())
                *it = block;
            else
            {
                ++itAux;
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


void MpduAggregateHandler::prepareADDBA(const int &cat)
{

    MACAddress addr;
    Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame*>(categories[cat].queue->front());
    addr = frame->getReceiverAddress();
    prepareADDBA(cat, addr);
}

void MpduAggregateHandler::prepareADDBA(const int &cat, const MACAddress &addr)
{

    if (!allAddress && !isAllowAddress(addr))
        return;

    ADDBAInfo *info;
    if (isAllowAddress(addr, info))
    {
        if (info->state != DEFAULT)
            return; // nothing to do
    }
    else
    {
        info = new ADDBAInfo();
        setADDBAInfo(addr,info);
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
        queueManagement->push_back(frame);

        if (!isAllowAddress(addr, info))
        {
            info = new ADDBAInfo();
            setADDBAInfo(addr,info);
        }

        return;
    }
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
        queueManagement->push_back(pkt);
    }
    else if (addbaFrame->getBody().getAction() == ADDBAReponse)
    {
        ADDBAInfo *info;
        if (!isAllowAddress(addr, info))
            throw cRuntimeError("ADDBAInfo not found");

        info->state = SENDBLOCK;
        info->bytesSend = 0;
        info->startBlockAck = simTime();

        createBlocks(addr);
        delete pkt;
    }
    return true;
}


}

}
