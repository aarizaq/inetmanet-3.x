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
#include <deque>
#include <map>
#include <utility>
#include "inet/linklayer/ieee80211/mac/IQoSClassifier.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"


namespace inet {

namespace ieee80211 {

class INET_API MpduAggregateHandler : public cObject
{
    public:
           class ADDBAInfo
           {
               public:
                   unsigned char token;
                   unsigned char tid;
                   unsigned char BlockAckPolicy;
                   unsigned char BufferSize;
                   unsigned short BlockAckTimeout;
                   unsigned short ADDBAFailureTimeout;
                   unsigned short BlockAckStartingSequenceControl;
           };
           enum State {
               DEFAULT,
               WAITCONFIRMATION,
               WAITBLOCK,
               SENDBLOCK,
           };
    protected:
        typedef std::map<MACAddress,int> NumFramesDestination;
        typedef std::deque<Ieee80211DataOrMgmtFrame *> DataQueue;
        class CategotyInfo
        {
            public:
                DataQueue*  queue;
                int queueSize;
                NumFramesDestination numFramesDestination;
                NumFramesDestination numFramesDestinationFree;
        };
        DataQueue*  queueManagement;

        State state;
        simtime_t blockState;
        std::map<MACAddress,ADDBAInfo> listAllowAddress;
        MACAddress neighAddress;
        bool checkState(const MACAddress&);
        bool allAddress;
        bool resetAfterSend;

        std::vector<CategotyInfo> categories;
        void increaseSize(Ieee80211TwoAddressFrame* val, int cat);
        void decreaseSize(Ieee80211TwoAddressFrame* val, int cat);
        // structures to store allow block destination address
    public:
        MpduAggregateHandler();
        virtual ~MpduAggregateHandler();
        virtual void setNumQueues(int num){categories.resize(num);}

        unsigned int getNumQueques()
        {
            return categories.size();
        }
        virtual void prepareADDBA(const int &);
        virtual int findAddress(const MACAddress  &,int = -1);
        virtual int findAddressFree(const MACAddress &addr,int = -1);
        virtual void createBlocks(const MACAddress &, int = -1);


        // ADDBAInfo management
        virtual void setAllAddress(const bool &p) {allAddress = p;}
        virtual void setResetAfterSend(const bool &p) {resetAfterSend = p;}
        virtual void setAllowAddress(const MACAddress &addr, ADDBAInfo *){}

        virtual bool getAllAddress() const {return allAddress;}
        virtual bool getResetAfterSend() const {return resetAfterSend;}
        virtual bool isAllowAddress(const MACAddress &add);
        virtual bool isAllowAddress(const MACAddress &add, ADDBAInfo &iaddai);
};

}

}

#endif /* MULTIQUEUE_H_ */
