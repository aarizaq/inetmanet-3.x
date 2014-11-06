//
// Copyright (C) 2011 Alfonso Ariza, Universidad de Malaga
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

// Default Max Size is 100 packets and the default number of queues are 1
#ifndef MULTIQUEUE_H_
#define MULTIQUEUE_H_
#include <vector>
#include <list>
#include <map>
#include <utility>
#include "inet/linklayer/ieee80211/mac/IQoSClassifier.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"


namespace inet {

namespace ieee80211 {

class INET_API MultiQueue : public cObject
{
    protected:
        typedef std::pair<simtime_t, Ieee80211TwoAddressFrame *> Data;
        typedef std::list<Data> Queue;
        typedef std::map<MACAddress,int> NumFramesDestination;
        std::pair<int, Ieee80211TwoAddressFrame *> firstPk;
        class CategotyInfo
        {
            public:
            Queue  queue;
            unsigned int queueSize;
            NumFramesDestination numFramesDestination;
            NumFramesDestination numFramesDestinationFree;
        };
        Queue::iterator position;
        unsigned int exploreQueue;
        bool allQueues;


        std::vector<CategotyInfo> categories;

        unsigned int maxSize;
        unsigned int numStrictQueuePriorities;
        int getCategory(Ieee80211TwoAddressFrame* val);
        void makeSpace();
        void increaseSize(Ieee80211TwoAddressFrame* val, int cat);
        void decreaseSize(Ieee80211TwoAddressFrame* val, int cat);
    public:
        MultiQueue();
        virtual ~MultiQueue();
        virtual void setNumQueues(int num);

        unsigned int getNumQueques()
        {
            return categories.size();
        }
        virtual void setMaxSize(unsigned int i)
        {
            maxSize = i;
        }
        virtual unsigned int getMaxSize()
        {
            return maxSize;
        }
        virtual unsigned int size(int i = -1);
        virtual bool empty(int i = -1);
        virtual Ieee80211TwoAddressFrame* front(int i = -1);
        virtual Ieee80211TwoAddressFrame* back(int i);
        virtual void push_front(Ieee80211TwoAddressFrame* val, int i = -1);
        virtual void pop_front(int i = -1);
        virtual void push_back(Ieee80211TwoAddressFrame* val, int i = -1);
        virtual void pop_back(int i);

        virtual int findAddress(const MACAddress  &,int = -1);
        virtual int findAddressFree(const MACAddress &addr,int = -1);
        virtual void createBlocks(const MACAddress &, int = -1);

        virtual Ieee80211TwoAddressFrame * initIterator(int i = -1);
        virtual Ieee80211TwoAddressFrame * next();
        virtual bool isEnd();
};

}

}

#endif /* MULTIQUEUE_H_ */
