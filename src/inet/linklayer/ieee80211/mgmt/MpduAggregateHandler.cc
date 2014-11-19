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
#include "inet/linklayer/ieee80211/mac/FrameBlock.h"
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
    std::map<MACAddress,ADDBAInfo>::iterator it = listAllowAddress.find(add);
    if (it == listAllowAddress.end())
        return false;
    return true;
}

bool MpduAggregateHandler::isAllowAddress(const MACAddress &add, ADDBAInfo &iaddai)
{
    std::map<MACAddress,ADDBAInfo>::iterator it = listAllowAddress.find(add);
    if (it == listAllowAddress.end())
        return false;
    iaddai = it->second;
    return true;
}

bool MpduAggregateHandler::checkState(const MACAddress &add)
{

    if (state != WAITBLOCK && state != SENDBLOCK)
        return false;

    double blockAckTimeout = DEFAULT_BL_ACK;
    double aDDBAFailureTimeout = DEFAULT_FALIURE;
    std::map<MACAddress,ADDBAInfo>::iterator it = listAllowAddress.find(add);
    unsigned short BlockAckTimeout;
    unsigned short ADDBAFailureTimeout;
    if (it == listAllowAddress.end())
        return false;

    if (state == WAITBLOCK && simTime() - blockState > blockAckTimeout)
    {


    }
    if (state == WAITBLOCK || state == SENDBLOCK)
    {

    }
    return false;
}


void MpduAggregateHandler::increaseSize(Ieee80211TwoAddressFrame* val, int cat)
{
    Ieee80211MpduA * block = dynamic_cast<Ieee80211MpduA *>(val);
    NumFramesDestination::iterator it = categories[cat].numFramesDestination.find(val->getReceiverAddress());
    if (it == categories[cat].numFramesDestination.end())
    {
        categories[cat].numFramesDestination[val->getReceiverAddress()] = 0;
        it = categories[cat].numFramesDestination.find(val->getReceiverAddress());
    }

    NumFramesDestination::iterator it2 = categories[cat].numFramesDestinationFree.find(val->getReceiverAddress());
    if (it2 == categories[cat].numFramesDestinationFree.end() && block == NULL)
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

void MpduAggregateHandler::decreaseSize(Ieee80211TwoAddressFrame* val, int cat)
{
    Ieee80211MpduA * block = dynamic_cast<Ieee80211MpduA *>(val);
    NumFramesDestination::iterator it = categories[cat].numFramesDestination.find(val->getReceiverAddress());
    if (it == categories[cat].numFramesDestination.end())
        throw cRuntimeError("Multi queue error address not found");

    NumFramesDestination::iterator it2 = categories[cat].numFramesDestinationFree.find(val->getReceiverAddress());
    if (it2 == categories[cat].numFramesDestinationFree.end() && block == NULL)
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

    cQueue::Iterator it(*(categories[cat].queue));

    if (it.end())
        return;
    Ieee80211MpduA *block = NULL;

    NumFramesDestination::iterator itDest2 = categories[cat].numFramesDestinationFree.find(addr);
    if (itDest2 != categories[cat].numFramesDestinationFree.end())
        return; // non free


    while (!it.end())
    {
        Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame*>(it());
        if (frame == NULL || !frame->getReceiverAddress().compareTo(addr))
        {
            it++;
            continue;
        }
        it++;

        if (!block)
        {
            block = new Ieee80211MpduA();
            categories[cat].queue->insertBefore(frame,block);
        }
        frame = check_and_cast<Ieee80211DataFrame *>(categories[cat].queue->remove(frame));
        block->pushBack(frame);
        itDest2->second--;
        if (itDest2->second <= 0)
        {
            categories[cat].numFramesDestinationFree.erase(itDest2);
            return;
        }
        if (it.end())
            return;
        if (block->getEncapSize() >= MAXBLOCK)
        {
            block = NULL;
            if (itDest2->second < MINBLOCK)
                return;
        }
    }
}

void MpduAggregateHandler::prepareADDBA(const int &cat)
{

    if (!allAddress && listAllowAddress.empty())
        return;

    MACAddress addr;
    Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame*>(categories[cat].queue->front());
    addr = frame->getReceiverAddress();
    if (!allAddress && !isAllowAddress(addr))
        return;
    if (findAddressFree(addr) >= MINBLOCK)
    {
        // create ADDBA frame
        Ieee80211ActionBlockAckADDBA * addbaFrame = new Ieee80211ActionBlockAckADDBA();
        Ieee80211TwoAddressFrame *frame = new Ieee80211TwoAddressFrame();
        frame->encapsulate(addbaFrame);
        frame->setReceiverAddress(addr);
        queueManagement->insert(frame);
        return;
    }
}


}

}
