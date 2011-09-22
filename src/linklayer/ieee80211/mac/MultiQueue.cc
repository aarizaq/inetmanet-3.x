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

#include "MultiQueue.h"

MultiQueue::MultiQueue()
{
    classifier=NULL;
    firstPk.second=NULL;
    lastPk = NULL;
    queues.resize(1);
    basePriority.resize(1);
    priority.resize(1);
    basePriority[0]=1;
    priority[0]=1;
    maxSize=100;
}

MultiQueue::~MultiQueue()
{
    // TODO Auto-generated destructor stub
    while (queues.empty())
    {
           while (!queues.back().empty())
           {
               delete queues.back().back().second;
               queues.back().pop_back();
           }
           queues.pop_back();
    }
    if (firstPk.second)
       delete firstPk.second;
}

void MultiQueue::setNumQueues(int num)
{
    basePriority.resize(num);
    priority.resize(num);
    if (num>(int)queues.size())
    {

        for (int i=(int)queues.size()-1;i<=num;i++)
        {
             basePriority[0]=1;
             priority[0]=1;
        }
        queues.resize(num);
    }
    else if (num<(int)queues.size())
    {
         while ((int)queues.size()>num)
         {
             while (!queues.back().empty())
             {
                  delete queues.back().back().second;
                  queues.back().pop_back();
             }
             queues.pop_back();
         }
    }
}

unsigned int MultiQueue::size(int i)
{
    if (i>=(int)queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");

    if (i != -1)
    {
        if (firstPk.first ==i && firstPk.second)
            return queues[i].size()+1;
        return queues[i].size();
    }

    unsigned int total=0;
    for (unsigned int j=0;j<queues.size();j++)
        total+=queues[j].size();
    if (firstPk.second)
        total++;
    return total;
}


bool MultiQueue::empty(int i)
{
    if (i>=(int)queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");
    if (i != -1)
        return queues[i].empty();
    for (unsigned int j=0;j<queues.size();j++)
        if (!queues[j].empty())
            return false;
    return true;
}

cMessage* MultiQueue::front(int i)
{
    if (i>=(int)queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");
    if (!firstPk.second)
        return NULL; // empty queue
    if (i != -1)
    {
        if (firstPk.first==i)
            return firstPk.second;
        if (queues[i].empty())
            return queues[i].front().second;
    }
    return firstPk.second;
}

cMessage* MultiQueue::back(int i)
{
    if (i>=(int)queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");
    if (!firstPk.second)
        return NULL;
    if (i != -1 && i!=firstPk.first)
        return queues[i].back().second;
    if (lastPk)
        return lastPk;
    return firstPk.second;
}


void MultiQueue::push_front(cMessage* val,int i)
{
    std::pair <simtime_t,cMessage*> value;

    opp_error("this method doesn't be used");

    if (i>=(int)queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");

    if (size()>getMaxSize())
    {
        // first make space
        // delete the oldest, the fist can't be deleted because can be in process.
        simtime_t max;
        int nQueue=-1;
        for (unsigned int j=0;j<queues.size();j++)
        {
            simtime_t max;
            int nQueue=-1;
            if (queues[j].front().first>max)
            {
                max=queues[j].front().first;
                nQueue=j;
            }
            if (nQueue>=0)
            {
                delete queues[nQueue].front().second;
                queues[nQueue].pop_front();
            }
        }
    }

    if (i!=-1)
    {
        if (!firstPk.second)
        {
            firstPk.second=val;
            firstPk.first=i;
            return;
        }
        value = std::make_pair (simTime(),val);
        queues[i].push_front(value);

    }
    else if (!classifier)
    {
        if (!firstPk.second)
        {
            firstPk.second=val;
            firstPk.first=classifier->classifyPacket(val);
            return;
        }
        value = std::make_pair (simTime(),val);
        queues[classifier->classifyPacket(val)].push_front(value);
    }
    else
    {
        if (!firstPk.second)
        {
            firstPk.second=val;
            firstPk.first=0;
            return;
        }
        queues[0].push_front(value);
    }
}

void MultiQueue::pop_front(int i)
{
    std::pair <simtime_t,cMessage*> value;
    if (i>=(int)queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");
    if (!firstPk.second)
        return;
    if (i != -1 && i!=firstPk.first)
    {
        queues[i].pop_front();
        priority[i]=basePriority[i];
        return;
    }
    else
    {
        firstPk.second=NULL;
        // decrease all priorities
        for (unsigned int j=0;j<queues.size();j++)
        {
            priority[i]--;
            if (priority[j]<0 && queues.empty())
                priority[j]=0;
        }
        priority[firstPk.first]=basePriority[firstPk.first];
        // select the next packet
        int min = 10000000;
        int nQueue=-1;

        for (int j=0;j<(int)queues.size();j++)
        {
            if (queues[j].empty())
                continue;
            if (priority[j]<min)
            {
                min = priority[j];
                nQueue=j;
            }
        }
        if (nQueue>=0)
        {
            firstPk.second=queues[nQueue].front().second;
            queues[nQueue].pop_front();
            firstPk.first=nQueue;
        }
    }
}

void MultiQueue::push_back(cMessage* val,int i)
{
    std::pair <simtime_t,cMessage*> value;
    if (i>=(int)queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");

    if (size()>getMaxSize())
    {
        // first make space
        // delete the oldest, the fist can't be deleted because can be in process.
        simtime_t max;
        int nQueue=-1;
        for (unsigned int j=0;j<queues.size();j++)
        {
            simtime_t max;
            if (queues[j].front().first>max)
            {
                max=queues[j].front().first;
                nQueue=j;
            }
        }
        if (nQueue>=0)
        {
            delete queues[nQueue].front().second;
            queues[nQueue].pop_front();
        }
    }
    value = std::make_pair (simTime(),val);
    if (i != -1)
    {
        if (!firstPk.second)
        {
            firstPk.second=val;
            firstPk.first=i;
            return;
        }
        queues[i].push_back(value);
    }
    else if (!classifier)
    {
        if (!firstPk.second)
        {
            firstPk.second=val;
            firstPk.first=i;
            return;
        }
        queues[classifier->classifyPacket(val)].push_back(value);
    }
    else
    {
        if (!firstPk.second)
        {
            firstPk.second=val;
            firstPk.first=i;
            return;
        }
        queues[0].push_back(value);
    }
    lastPk=val;
}

void MultiQueue::pop_back(int i)
{
    std::pair <simtime_t,cMessage*> value;
    if (i>=(int)queues.size())
        opp_error("MultiQueue::size Queue doesn't exist");

    if (!firstPk.second)
        return;

    if (i != -1)
    {
        if (lastPk==queues[i].back().second)
        {
            lastPk=NULL;
        }
        queues[i].pop_back();
        if (lastPk!=NULL)
            return; // nothing to do
        if (this->empty())
            return;

    }
    else
    {
        for (unsigned int j=0;j<queues.size();j++)
        {
            if (queues[j].empty())
                continue;
            if (lastPk==queues[j].back().second)
                lastPk=NULL;
        }

    }
    if (this->empty())
        return;
    simtime_t max;
    int nQueue=-1;
    for (unsigned int j=0;j<queues.size();j++)
    {
        if (queues[j].back().first>max)
        {
            max=queues[j].back().first;
            nQueue=j;
        }
    }
    if (nQueue>=0)
    {
        lastPk=queues[nQueue].back().second;
    }
}




