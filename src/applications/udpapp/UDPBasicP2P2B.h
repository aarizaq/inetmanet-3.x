//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2011 Zoltan Bojthe
// Copyright (C) 2011 Alfonso Ariza, Universidad de Malaga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __INET_UDPBASICP2P2B_H
#define __INET_UDPBASICP2P2B_H


#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <fstream>

#include "INETDefs.h"
#include "UDPSocket.h"
#include "ARP.h"
#include "fis.h"
#include "WirelessNumHops.h"
#include "UDPBasicPacketP2P_m.h"

/**
 * UDP application. See NED for more info.
 */
class INET_API UDPBasicP2P2B : public cSimpleModule, protected INotifiable
{
public:
    enum  modoP2P
    {
        CONCAVO = 1, ADITIVO, ADITIVO_PONDERADO,MIN_HOP,RANDOM
    };
private:
    modoP2P modo;

    typedef std::map<uint64_t,IPv4Address> InverseAddres;
    typedef std::map<IPv4Address, uint64_t> DirectAddres;

    static InverseAddres inverseAddress;
    static DirectAddres directAddress;

    int numRequestSent;
    int numRequestServed;
    int numRequestSegmentServed;

    static std::vector<int> initNodes;
    WirelessNumHops *routing;

    private:
    class DelayMessage
    {
           ManetAddress nodeId_var;
           ManetAddress destination_var;
           int type_var;
           int64_t segmentRequest_var;
           uint64_t totalSize_var;
           uint64_t segmentId_var;
           uint16_t subSegmentId_var;
           simtime_t lastSend_var;
           unsigned int index_var;
           unsigned int total_var;
           uint64_t remain_var;
           simtime_t nextTime;
        public:
           DelayMessage()
           {
               nodeId_var = ManetAddress::ZERO;
               destination_var = ManetAddress::ZERO;
               type_var = 0;
               segmentRequest_var = 0;
               index_var = 0;
               total_var = 0;
               remain_var = 0;

               totalSize_var = 0;
               segmentId_var = 0;
               subSegmentId_var = 0;
               subSegmentRequest.clear();
           }
           virtual ~DelayMessage()
           {
               subSegmentRequest.clear();
           }
           std::vector<uint16_t> subSegmentRequest;

           virtual void setRemain(const uint64_t & val) {remain_var = val;};
           virtual uint64_t getRemain() {return remain_var;}
        // field getter/setter methods

           virtual ManetAddress getNodeId() const;
           virtual void setNodeId(ManetAddress nodeId);
           virtual ManetAddress getDestination() const;
           virtual void setDestination(ManetAddress destination);
           virtual int getType() const;
           virtual void setType(int type);
           virtual uint64_t getTotalSize() const;
           virtual void setTotalSize(uint64_t totalSize);
           virtual uint64_t getSegmentId() const;
           virtual void setSegmentId(uint64_t segmentId);
           virtual uint16_t getSubSegmentId() const;
           virtual void setSubSegmentId(uint16_t subSegmentId);
           UDPBasicPacketP2P * getPkt(const uint32_t&);
           void setPkt(UDPBasicPacketP2P *pkt);

           virtual simtime_t getLastSend();
           virtual void setLastSend(const simtime_t&);

           virtual unsigned int getIndex();
           virtual unsigned int getTotal();
           virtual void setIndex(const unsigned int&);
           virtual void setTotal(const unsigned int&);
           virtual void setNextTime(const simtime_t &v) {nextTime =v;}
           virtual simtime_t getNextTime() {return nextTime;}
    };


    typedef std::deque<DelayMessage*> TimeQueue;
    int MaxServices;
    TimeQueue timeQueue;
    int getQueueSize();
    cMessage *queueTimer;
    double serverTimer;
    simtime_t lastServer;

    struct InfoClient
    {
            simtime_t lastMessage;
            unsigned int numRequest;
            unsigned int numRequestRec;
    };

    typedef std::map<uint64_t,InfoClient> ClientList;
    ClientList clientList;

    void WirelessNumNeig();
    int getNumNeighNodes(uint64_t,double);
    typedef std::map<uint64_t, IMobility*> VectorList;
    static VectorList vectorList;


  protected:
    typedef std::set<uint32_t> SegmentList;
    struct InfoData
    {
            uint64_t nodeId;
            SegmentList list;
    };

    UDPSocket socket;
    ARP *arp;

    typedef std::map<uint64_t,SegmentList *> SegmentMap;
    static SegmentMap segmentMap;
    SegmentList mySegmentList;
    std::deque<uint16_t> request;
    ManetAddress myAddress;
    IPv4Address myAddressIp4;
    cMessage *myTimer;
    cMessage *retryTimer;
    FuzzYControl *fuzzy;

    int numSent;
    int numReceived;
    int numDeleted;
    int numDuplicated;
    int numSegPresent;
    simtime_t endReception;
    simtime_t startReception;


    int numSentB;
    int numReceivedB;
    int numDeletedB;
    int numDuplicatedB;

    struct ConnectInTransit
    {
            ManetAddress nodeId;
            cMessage timer;
            uint32_t segmentId;
            cSimpleModule *owner;
            std::set<uint16_t> segmentInTransit;
            ConnectInTransit(ManetAddress id, uint32_t s,uint64_t nseg, cSimpleModule *o)
            {
                timer.setName("PARALLELTIMER");
                nodeId = id;
                segmentId = s;
                owner = o;

                for (unsigned int i = 1; i<= nseg; i++)
                    segmentInTransit.insert(i);


            }
            ~ConnectInTransit()
            {
                if (timer.isScheduled())
                    owner->cancelEvent(&timer);
            }
    };

    typedef std::multimap<simtime_t,UDPBasicPacketP2P*> PendingRequest;
    typedef std::deque<ConnectInTransit> ParallelConnection;
    ParallelConnection parallelConnection;
    PendingRequest pendingRequest;
    cMessage *pendingRequestTimer;
    unsigned int numParallelRequest;

    uint16_t requestPerPacket;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t outOfOrderPkSignal;
    static simsignal_t dropPkSignal;
    static simsignal_t queueLengthSignal;

    static simsignal_t sentPkSignalP2p;
    static simsignal_t rcvdPkSignalP2p;

    cMessage periodicTimer; // periodic timer

    uint32_t totalSegments;
    uint32_t segmentSize;
    uint32_t  maxPacketSize;

    int destPort;
    int localPort;

    simtime_t lastPacket;
    double rateLimit;

    bool writeData;
    unsigned int numRegData;
    std::ofstream *outfile;

  protected:

    virtual bool areDiff(const SegmentList &List1, const SegmentList &List2);
    virtual int numInitStages() const {return 7;}
    virtual void initialize(int stage);
    virtual void finish();

    virtual bool processMyTimer(cMessage *);
    // chooses random destination address
    virtual void generateRequestNew();
    virtual void generateRequestSub();
    virtual bool processPacket(cPacket *pk);
    virtual void processRequest(cPacket *pkt);
    virtual void answerRequest(UDPBasicPacketP2P *pkt);
    virtual void sendNow(UDPBasicPacketP2P *pkt);
    virtual void sendDelayed(UDPBasicPacketP2P *pkt,const simtime_t &delay);
    virtual void sendQueue();
    virtual uint64_t chooseDestAddr();
    virtual uint64_t chooseDestAddr(uint32_t segmentId);
    virtual void getList(std::vector<uint64_t> &);
    virtual void getList(std::vector<uint64_t> &address,uint32_t segmentId);
    virtual void getList(std::vector<uint64_t> &address, std::vector<SegmentList> &);

    virtual uint64_t searchBestSegment(const uint64_t &);
    virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void handleMessage(cMessage *msg);
    virtual uint64_t selectBest(const std::vector<uint64_t> &);
    virtual std::vector<UDPBasicP2P2B::InfoData> selectBestList(const std::vector<uint64_t> &address, const std::vector<SegmentList> &List);
    virtual std::vector<UDPBasicP2P2B::InfoData>  chooseDestAddrList();

    virtual void purgePendingRequest(uint64_t segment);


   public:
    UDPBasicP2P2B();
    virtual ~UDPBasicP2P2B();



  public:
     enum ChooseDestAddrMode
     {
         ONCE = 1, PER_BURST, PER_SEND
     };

   protected:
     enum SelfMsgKinds { START = 1, SEND, STOP };


     ChooseDestAddrMode chooseDestAddrMode;
     std::vector<IPvXAddress> destAddresses;
     IPvXAddress destAddr;
     int destAddrRNG;

     typedef std::map<int,int> SourceSequence;
     SourceSequence sourceSequence;
     simtime_t delayLimit;
     cMessage *timerNext;
     simtime_t startTime;
     simtime_t stopTime;
     simtime_t nextPkt;
     simtime_t nextBurst;
     simtime_t nextSleep;
     bool activeBurst;
     bool isSource;
     bool haveSleepDuration;
     int outputInterface;
     std::vector<int> outputInterfaceMulticastBroadcast;

     static int counter; // counter for generating a global number for each packet

     // volatile parameters:
     cPar *messageLengthPar;
     cPar *burstDurationPar;
     cPar *sleepDurationPar;
     cPar *sendIntervalPar;

     //statistics:
     // chooses random destination address
     virtual IPvXAddress chooseDestAddrBurst();
     virtual cPacket *createPacket();
     virtual void processPacketBurst(cPacket *msg);
     virtual void generateBurst();

     void initializeBurst(int stage);


   protected:
     virtual void handleMessageWhenUp(cMessage *msg);

     virtual void processStart();
     virtual void processSend();


};

#endif

