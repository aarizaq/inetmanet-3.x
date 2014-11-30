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

class INET_API MpduAggregateHandler : public cOwnedObject
{
    private:
        // Timeout structures
        enum TimerType
        {
            BLOCKTIMEOUT,
            ADDBAFALIURE,
            RESETBLOCK
        };
        struct Timer
        {
                MACAddress nodeId;
                TimerType type;

        };
        typedef std::multimap<simtime_t,Timer> TimerQueue;
        TimerQueue timerQueue;

        // timer methods
        void addTimer(const MACAddress &, const TimerType &, const double &);
        void checkTimer();
        bool checkPending(const MACAddress &, const TimerType &);
        void erasePending(const MACAddress &, const TimerType &);

        // timer out action methods
        void blockTimeOutAction(const MACAddress &);
        void addbaFaliureAction(const MACAddress &);
        void resetBlock(const MACAddress &);

    public:
        // Enum Types
        enum State {
            DEFAULT,
            WAITCONFIRMATION,
            WAITBLOCK,
            SENDBLOCK,
        };
        enum Ieee80211BlockAckCode
        {
            ADDBArequest = 0,
            ADDBAReponse = 1,
            DELBA = 2,
        };
        class ADDBAInfo
        {
            public:
                State state = DEFAULT;
                simtime_t startBlockAck;
                uint64_t bytesSend;
                unsigned char token;
                unsigned char tid;
                unsigned char BlockAckPolicy;
                unsigned char BufferSize;
                unsigned short BlockAckTimeout;
                unsigned short ADDBAFailureTimeout;
                unsigned short BlockAckStartingSequenceControl;
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
        std::map<MACAddress,ADDBAInfo *> listAllowAddress;
        MACAddress neighAddress;

        bool allAddress;
        bool resetAfterSend;
        int automaticMimimumAddress; // if the number of "free" address is bigger than this value the MpduAggregateHandler changes to automatically the handle Ack mode, 0 or lower disabled

        std::vector<CategotyInfo> categories;
        virtual void increaseSize(Ieee80211DataOrMgmtFrame* val, int cat);
        virtual void decreaseSize(Ieee80211DataOrMgmtFrame* val, int cat);


        virtual int findAddress(const MACAddress  &,int = -1);
        virtual int findAddressFree(const MACAddress &addr,int = -1);
        virtual void createBlocks(const MACAddress &, int = -1);
        virtual void removeBlock(const MACAddress &, int = -1);

        //
        virtual void setMacAcceptMpdu(const MACAddress &);
        virtual void setMacDiscardMpdu(const MACAddress &);

        // management of frame control
        virtual void prepareADDBA(const MACAddress &);
        virtual bool handleADDBA(Ieee80211DataOrMgmtFrame *);
        virtual void sendDELBA(const MACAddress &addr);

        // handle states
        virtual bool checkState(const MACAddress&);
             //  ADDBAInfo management

    public:
        MpduAggregateHandler();
        virtual ~MpduAggregateHandler();
        virtual void setNumQueues(int num){categories.resize(num);}

        virtual unsigned int getNumQueques() {return categories.size();}

        virtual bool checkState(const Ieee80211DataOrMgmtFrame *);
        virtual bool handleFrames(Ieee80211DataOrMgmtFrame *pkt);

        virtual void setAllAddress(const bool &p) {allAddress = p;}
        virtual void setResetAfterSend(const bool &p) {resetAfterSend = p;}
        virtual void setADDBAInfo(const MACAddress &addr, ADDBAInfo *);
        virtual bool isAllowAddress(const MACAddress &add, ADDBAInfo *&iaddai);

        //
        virtual void setAutomaticMimimumAddress(const int & p) {automaticMimimumAddress = p;} // if the number of "free" address is bigger than this value the MpduAggregateHandler changes to automatically the handle Ack mode, 0 or lower disabled
        virtual int setAutomaticMimimumAddress() {return automaticMimimumAddress;}

        virtual bool getAllAddress() const {return allAddress;}
        virtual bool getResetAfterSend() const {return resetAfterSend;}
        virtual bool isAllowAddress(const MACAddress &add);

};

}

}

#endif /* MULTIQUEUE_H_ */
