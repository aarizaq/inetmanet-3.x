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

#include "inet/linklayer/ieee80211/mac/MultiQueue.h"
#include "inet/linklayer/ieee80211/mac/FrameBlock.h"

namespace inet {

namespace ieee80211 {

#define MAXBLOCK 16
#define MINBLOCK 3

MultiQueue::MultiQueue()
{
    categories.clear();
    categories.resize(2);
    categories[0].queueSize = 0;
    categories[1].queueSize = 0;
    maxSize = 1000000;
    numStrictQueuePriorities = 0;
    exploreQueue = 1;
    firstPk.second = NULL;
}

int MultiQueue::getCategory(Ieee80211TwoAddressFrame* val)
{
    int cat = 0;
 // check type
   Ieee80211DataFrame * frameAux = dynamic_cast<Ieee80211DataFrame *>(val);
   FrameBlock * block = dynamic_cast<FrameBlock *>(val);
   if (frameAux == NULL && block == NULL)
   {
       // management frame first queue
       cat = 0;
   }
   else
   {
       if (categories.size() == 2 || val->getReceiverAddress().isMulticast())
           cat = 1;
       else
           cat = 2;
   }
    return cat;
}

void MultiQueue::makeSpace()
{
    if (size() < getMaxSize())
        return;

    // make space
    simtime_t max;
    int nQueue = -1;
    for (unsigned int j = 0; j < categories.size(); j++) {
        // delete the oldest, the fist can't be deleted because can be in process.
        if ((int)j == firstPk.first) {
            if (categories[j].queue.front().first > max) {
                max = categories[j].queue.front().first;
                nQueue = j;
            }
        }
        else {
            // check the second
                if (categories[j].queue.size() > 1) {
                    Queue::iterator it = categories[j].queue.begin();
                    ++it;
                    if (it->first > max) {
                        max = it->first ;
                        nQueue = j;
                    }
                }
        }
    }

    if (nQueue < 0)
        return;


    Queue::iterator it = categories[nQueue].queue.begin();
    if (nQueue != firstPk.first)
        ++it;

    FrameBlock * block = dynamic_cast<FrameBlock *>(it->second);
    MACAddress dest = it->second->getReceiverAddress();

    categories[nQueue].queueSize--;
    if (block) {
        // delete one
        FrameBlock * block = dynamic_cast<FrameBlock *>(categories[nQueue].queue.front().second);
        if (block->getEncapSize() == 1) {
            delete it->second;
            categories[nQueue].queue.erase(it);
        }
        else {
            delete block->decapsulatePacket(0);
        }
    }
    else {
        delete it->second;
        categories[nQueue].queue.erase(it);
    }

    NumFramesDestination::iterator itDest = categories[nQueue].numFramesDestination.find(dest);
    if (itDest == categories[nQueue].numFramesDestination.end())
        throw cRuntimeError("Multi queue error address not found");

    itDest->second--;

    if (!block)
    {
        NumFramesDestination::iterator itDest2 = categories[nQueue].numFramesDestinationFree.find(dest);
        if (itDest2 != categories[nQueue].numFramesDestinationFree.end())
        {
            itDest2->second--;
            if (itDest2->second <=0)
                categories[nQueue].numFramesDestinationFree.erase(itDest2);
        }
    }

    if (itDest->second <=0)
        categories[nQueue].numFramesDestination.erase(itDest);
}


void MultiQueue::increaseSize(Ieee80211TwoAddressFrame* val, int cat)
{
    FrameBlock * block = dynamic_cast<FrameBlock *>(val);
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

void MultiQueue::decreaseSize(Ieee80211TwoAddressFrame* val, int cat)
{
    FrameBlock * block = dynamic_cast<FrameBlock *>(val);
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

MultiQueue::~MultiQueue()
{
    // TODO Auto-generated destructor stub
    if (firstPk.second)
        delete firstPk.second;
    while (categories.empty())
    {
        while (!categories.back().queue.empty())
        {
            delete categories.back().queue.back().second;
            categories.back().queue.pop_back();
        }
        categories.pop_back();
    }
}

void MultiQueue::setNumQueues(int num)
{
    exploreQueue = num;  // set iterator to null

    if (num > (int) categories.size())
    {
        categories.resize(num);

        for (int i = (int) categories.size() - 1; i <= num; i++)
        {
            categories[i].queueSize = 0;
        }
    }
    else if (num < (int) categories.size())
    {
        while ((int) categories.size() > num)
        {
            while (!categories.back().queue.empty())
            {
                delete categories.back().queue.back().second;
                categories.back().queue.pop_back();
            }
            categories.pop_back();
        }
    }
}

unsigned int MultiQueue::size(int i)
{
    if (i >= (int) categories.size())
         throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    if (firstPk.second == NULL)
        return 0;

    unsigned int total = 0;
    if (i != -1 && i == firstPk.first)
        total++;

    if (i != -1)
        total  += categories[i].queueSize;
    else
    {
        total = 1;
        for (unsigned int j = 0; j < categories.size(); j++)
            total += categories[i].queueSize;
    }
    return total;
}

bool MultiQueue::empty(int i)
{
    if (i >= (int) categories.size())
        opp_error("MultiQueue::size Queue doesn't exist");
    if (firstPk.second == NULL)
        return true;
    if (i == -1 && firstPk.second != NULL)
        return false;
    if (i != -1 && i == firstPk.first)
        return true;
    return categories[i].queue.empty();
}

Ieee80211TwoAddressFrame* MultiQueue::front(int i)
{
    if (i >= (int) categories.size())
        throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    if (i == -1 || ((i != -1 && i == firstPk.first)))
        return firstPk.second;

    if (!categories[i].queue.empty())
        return categories[i].queue.front().second;
    else
        return NULL;
 }

Ieee80211TwoAddressFrame* MultiQueue::back(int i)
{
    if (i >= (int) categories.size())
        throw cRuntimeError("MultiQueue::size Queue doesn't exist");
    return categories[i].queue.back().second;
}

void MultiQueue::push_front(Ieee80211TwoAddressFrame* val, int i)
{
    std::pair<simtime_t, Ieee80211TwoAddressFrame*> value;


    if (i >= (int) categories.size())
        throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    int cat = 0;
    if (i == -1)
        cat = getCategory(val);
    else
        cat = i;

    makeSpace();
    increaseSize(val,cat);

    if (firstPk.second == NULL) {
        firstPk.second = val;
        firstPk.first = cat;
        return;
    }

    value = std::make_pair(simTime(), val);
    categories[cat].queue.push_front(value);

}

void MultiQueue::pop_front(int i)
{
    std::pair<simtime_t, Ieee80211TwoAddressFrame*> value;
    if (i >= (int) categories.size())
        throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    if (firstPk.second == NULL)
        return;

    int cat;
    Ieee80211TwoAddressFrame *frame = NULL;
    if (i == -1)
    {
        cat = firstPk.first;
        frame = firstPk.second;
    }
    else
    {
        if (categories[i].queue.empty())
            return;
        cat = i;
        if (cat == firstPk.first)
            frame = firstPk.second;
        else
            frame = categories[cat].queue.front().second;
    }

    decreaseSize(frame, cat);
    if (frame == firstPk.second)
    {
        firstPk.second = NULL;
        for (unsigned int j = 0; j < categories.size(); j++)
        {
            if (!categories[j].queue.empty())
            {
                firstPk.second = categories[j].queue.front().second;
                categories[j].queue.pop_front();
                firstPk.first = j;
                break;
            }
        }
    }
    else
        categories[cat].queue.pop_front();
    return;
}

void MultiQueue::push_back(Ieee80211TwoAddressFrame* val, int i)
{
    if (i >= (int) categories.size())
        throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    int cat = 0;
    if (i == -1)
        cat = getCategory(val);
    else
        cat = i;

// insert in position
    makeSpace();
    increaseSize(val,cat);

    if (firstPk.second == NULL) {
        firstPk.second = val;
        firstPk.first = cat;
        return;
    }

    std::pair<simtime_t, Ieee80211TwoAddressFrame*> value;
    value = std::make_pair(simTime(), val);
    categories[cat].queue.push_back(value);
}

void MultiQueue::pop_back(int i)
{
    if (i >= (int) categories.size())
        throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    if (firstPk.second == NULL)
        return;

    if (categories[i].queue.empty() && i != firstPk.first)
        return;



    decreaseSize(categories[i].queue.back().second,i);
    if (categories[i].queue.empty() && i == firstPk.first)
    {
        firstPk.second = NULL;
        for (unsigned int j = 0; j < categories.size(); j++)
        {
            if (!categories[j].queue.empty())
            {
                firstPk.second = categories[j].queue.front().second;
                categories[j].queue.pop_front();
                firstPk.first = j;
                break;
             }
         }
    }
    else
        categories[i].queue.pop_back();
}

Ieee80211TwoAddressFrame* MultiQueue::initIterator(int i)
{
    if (i >= (int) categories.size())
         throw cRuntimeError("MultiQueue::size Queue doesn't exist");
    if (firstPk.second == NULL)
        return NULL; // empty

    if (i == -1)
    {
        allQueues = true;
        exploreQueue = 0;
        position = categories[0].queue.begin();
        return firstPk.second;
    }

    allQueues = false;
    exploreQueue = i;

    position = categories[exploreQueue].queue.begin();
    Ieee80211TwoAddressFrame *aux = position->second;
    ++position;
    return aux;
}

Ieee80211TwoAddressFrame* MultiQueue::next()
{
    if (allQueues)
    {
        while (position == categories[exploreQueue].queue.end() && exploreQueue < categories.size())
        {
            exploreQueue++;
            position = categories[exploreQueue].queue.begin();
        }

        if (exploreQueue >= categories.size())
            return NULL;
        Ieee80211TwoAddressFrame *aux = position->second;
        ++position;
        return aux;
    }
    if (position == categories[exploreQueue].queue.end())
        return NULL;
    else
    {
        Ieee80211TwoAddressFrame *aux = position->second;
        ++position;
        return aux;
    }
}


bool  MultiQueue::isEnd()
{

    if (exploreQueue >= categories.size())
        return true;
    if (allQueues)
    {
        bool otherQueuesHave = false;
        for (unsigned int j = exploreQueue+1; j < categories.size(); j++)
        {
            if (!categories[exploreQueue].queue.empty())
            {
                otherQueuesHave = true;
                break;
            }
        }

        if  (position == categories[exploreQueue].queue.end() && !otherQueuesHave)
        {
            exploreQueue = categories.size();
            return true;
        }
        else
            return false;
    }

    if (position == categories[exploreQueue].queue.end())
        return true;
    else
        return false;
}

int MultiQueue::findAddress(const MACAddress &addr ,int cat)
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

int MultiQueue::findAddressFree(const MACAddress &addr ,int cat)
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


void MultiQueue::createBlocks(const MACAddress &addr, int cat)
{

    if (cat >= (int) categories.size())
         throw cRuntimeError("MultiQueue::size Queue doesn't exist");

    if (addr.isMulticast() || addr.isBroadcast())
       return;

    if (cat == -1)
        cat = categories.size()-1;

    Queue::iterator it = categories[cat].queue.begin();
    if (it == categories[cat].queue.end())
        return;

    if (firstPk.first != cat)
        ++it;
    FrameBlock *block = NULL;

    NumFramesDestination::iterator itDest2 = categories[cat].numFramesDestinationFree.find(addr);
    if (itDest2 != categories[cat].numFramesDestinationFree.end())
        return; // non free


    while (it == categories[cat].queue.end())
    {
        if (it->second->getReceiverAddress().compareTo(addr))
        {
            // check if block
            if (dynamic_cast<Ieee80211DataFrame *>(it->second))
            {
                if (!block)
                {
                    block = new FrameBlock(it->second);
                    it->second = block;
                    itDest2->second--;
                    if (itDest2->second <=0)
                    {
                        categories[cat].numFramesDestinationFree.erase(itDest2);
                        return;
                    }
                    ++it;

                }
                else
                {
                    block->pushBack(it->second);
                    it = categories[cat].queue.erase(it);
                    itDest2->second--;
                    if (itDest2->second <=0)
                    {
                        categories[cat].numFramesDestinationFree.erase(itDest2);
                        return;
                    }
                    if (block->getEncapSize() >= MAXBLOCK)
                    {
                        block = NULL;
                        if (itDest2->second < MINBLOCK)
                            return;
                    }
                }
            }
        }
        else
            ++it;
    }
}

}

}
