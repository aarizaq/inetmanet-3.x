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


#ifndef __INET_UDPBasicP2P2D_H
#define __INET_UDPBasicP2P2D_H


#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <stdint.h>

#include "INETDefs.h"
#include "UDPSocket.h"
#include "ARP.h"
#include "fis.h"
#include "WirelessNumHops.h"
#include "UDPBasicPacketP2P_m.h"

/**
 * UDP application. See NED for more info.
 */
class INET_API UDPBasicP2P2Multi : public cSimpleModule, protected INotifiable
{
public:
    enum  modoP2P
    {
        CONCAVO = 1, ADITIVO, ADITIVO_PONDERADO,MIN_HOP,RANDOM
    };

    typedef std::vector<uint64_t> ListAddress;
    typedef std::vector<ManetAddress> PathAddress;

private:

    bool changeDestination = true;
    uint64_t destination = 0;

    modoP2P modo = modoP2P::ADITIVO;

    typedef std::map<uint64_t,IPv4Address> InverseAddres;
    typedef std::map<IPv4Address, uint64_t> DirectAddres;


    static InverseAddres inverseAddress;
    static DirectAddres directAddress;

    int numRequestSent = 0;
    int numRequestServed = 0;
    int numRequestSegmentServed = 0;

    static std::vector<int> initNodes;
    WirelessNumHops *routing = nullptr;

    private:

    class DelayMessage
    {
           ManetAddress nodeId_var = ManetAddress::ZERO;
           ManetAddress destination_var = ManetAddress::ZERO;
           int type_var = 0;
           int objectId = 0;
           int64_t segmentRequest_var = 0;
           uint64_t totalSize_var = 0;
           uint64_t segmentId_var = 0;
           uint16_t subSegmentId_var = 0;
           simtime_t lastSend_var = 0;
           unsigned int index_var = 0;
           unsigned int total_var = 0;
           uint64_t remain_var = 0;
           simtime_t nextTime = 0;
        public:
           PathAddress route;
           DelayMessage()
           {
               nodeId_var = ManetAddress::ZERO;
               destination_var = ManetAddress::ZERO;
               subSegmentRequest.clear();
               route.clear();
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

           virtual int getObjectId() {return objectId;}
           virtual void setObjectId(const int &obj) {objectId = obj;}

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
    int MaxServices = 1;
    TimeQueue timeQueue;
    long unsigned getQueueSize();
    cMessage *queueTimer = nullptr;
    double serverTimer = 0;
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

  public:
    typedef std::set<uint32_t> SegmentList;
  protected:

    bool sourceRouting = false;

    typedef std::map<int,SegmentList> ObjectSegmentList;
    typedef std::vector<int> ObjectiveObject; //lista de objetos que tiene como objectivo el nodo

    struct InfoData
    {
            uint64_t nodeId = 0;
            SegmentList list;
    };

    UDPSocket socket;
    ARP *arp = nullptr;

    typedef std::map<uint64_t,ObjectSegmentList *> SegmentMap;
    typedef std::map<uint64_t,uint64_t> SequenceList;

    static SegmentMap segmentMap;
    SegmentMap networkSegmentMap;
    ObjectSegmentList mySegmentList;
    SequenceList sequenceList;
    bool useGlobal = false;
    ListAddress initialList;

    typedef std::deque<uint16_t> Requested;
    std::map<int, Requested> request;

    ManetAddress myAddress;
    IPv4Address myAddressIp4;
    // timers
    cMessage *myTimer = nullptr;
    cMessage *retryTimer = nullptr;
    cMessage *informTimeOut = nullptr;
    cMessage *periodicMeasureTimer = nullptr;

    FuzzYControl *fuzzy = nullptr;
    uint64_t mySeqNumber = 0;

    // periodic statistics
    uint64_t totalBytesInPeriod = 0;
    int64_t totalRequestInPeriod = 0;
    int64_t totalSendInPeriod = 0;
    bool onlyInitialNodes = false;
    bool recordVector = false;
    cOutVector periodDataRequestPackets;
    cOutVector periodDataSendPackets;
    cOutVector periodDataSendBytes;

    int numSent = 0;
    int numReceived = 0;
    int numDeleted = 0;
    int numDuplicated = 0;
    int numObjectPresent = 0;

    typedef std::map<int,simtime_t> TimeMark;
    TimeMark endReception;
    TimeMark startReception;


    int numSentB = 0;
    int numReceivedB = 0;
    int numDeletedB = 0;
    int numDuplicatedB = 0;

    struct ConnectInTransit
    {
            ManetAddress nodeId;
            cMessage timer;
            uint32_t transitSegmentId = 0;
            int transitObjectId = 0;
            cSimpleModule *owner = nullptr;
            std::set<uint16_t> segmentInTransit;
            PathAddress route;
            ConnectInTransit(ManetAddress id, int obj, uint32_t s,uint64_t nseg, cSimpleModule *o)
            {
                timer.setName("PARALLELTIMER");
                nodeId = id;
                transitSegmentId = s;
                transitObjectId = obj;
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
    cMessage *pendingRequestTimer = nullptr;
    unsigned int numParallelRequest = 1;

    uint16_t requestPerPacket = 1;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t outOfOrderPkSignal;
    static simsignal_t dropPkSignal;
    static simsignal_t queueLengthSignal;
    static simsignal_t queueLengthInstSignal;

    cMessage periodicTimer; // periodic timer

    static std::map<int,uint32_t> totalSegments;
    static std::map<int,uint32_t> segmentSize;
    uint32_t  maxPacketSize;

    int destPort = 0;
    int localPort = 0;

    simtime_t lastPacket;
    double rateLimit = 0;

    bool writeData = false;
    unsigned int numRegData = 0;
    std::ofstream *outfile = nullptr;

  protected:

    virtual bool areDiff(const SegmentList &List1, const SegmentList &List2);
    virtual int numInitStages() const {return 7;}
    virtual void initialize(int stage);
    virtual void finish();

    virtual bool processMyTimer(cMessage *);
    // chooses random destination address
    virtual void generateRequestNew();
    virtual void generateRequestSub();
    virtual void actualizePacketMap(UDPBasicPacketP2P *pkt);
    virtual bool processPacket(UDPBasicPacketP2P *pk, int &);
    virtual void processRequest(cPacket *pkt);
    virtual void actualizeList(UDPBasicPacketP2P *pkt);
    virtual void answerRequest(UDPBasicPacketP2P *pkt);
    virtual void sendNow(UDPBasicPacketP2P *pkt);
    virtual void sendDelayed(UDPBasicPacketP2P *pkt,const simtime_t &delay);
    virtual void sendQueue();
    virtual uint64_t chooseDestAddr(PathAddress &);
    virtual uint64_t chooseDestAddr(int, uint32_t segmentId, PathAddress&);
    virtual void getList(ListAddress &);
    virtual void getList(ListAddress &address,int , uint32_t segmentId);
    virtual void getList(ListAddress &address,int , std::vector<SegmentList> &);

    virtual uint64_t searchBestSegment(const int &, const uint64_t &);
    virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void handleMessage(cMessage *msg);
    virtual uint64_t selectBest(const ListAddress &, PathAddress &);

    virtual void purgePendingRequest(int objectid, uint64_t segment);


    // create a packet with the map of the node
    UDPBasicPacketP2PNotification *getPacketWitMap(int);
    // Send a broadcast packet with the information
    virtual void informChanges();
    // process the packet with the information of status
    virtual bool processMsgChanges(cPacket *msg);

   public:
    UDPBasicP2P2Multi();
    virtual ~UDPBasicP2P2Multi();
};

#endif

