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


#ifndef __INET_UDPBasicP2P2C_H
#define __INET_UDPBasicP2P2C_H


#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <fstream>

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

#include "inet/networklayer/arp/IARP.h"
#include "inet/applications/udpapp/fis.h"
#include "inet/common/WirelessNumHops.h"
#include "inet/applications/udpapp/UDPBasicPacketP2P_m.h"

namespace inet {

/**
 * UDP application. See NED for more info.
 */
class INET_API UDPBasicP2P2C : public cSimpleModule
{
public:
    enum  modoP2P
    {
        CONCAVO = 1, ADITIVO, ADITIVO_PONDERADO,MIN_HOP,RANDOM
    };
private:
    modoP2P modo = ADITIVO;

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
           L3Address nodeId_var;
           L3Address destination_var;
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
               nodeId_var = L3Address();
               destination_var = L3Address();
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

           virtual L3Address getNodeId() const;
           virtual void setNodeId(L3Address nodeId);
           virtual L3Address getDestination() const;
           virtual void setDestination(L3Address destination);
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
    int MaxServices = 1;
    TimeQueue timeQueue;
    int getQueueSize();
    cMessage *queueTimer = nullptr;
    double serverTimer = 0;
    simtime_t lastServer;

    struct InfoClient
    {
            simtime_t lastMessage;
            unsigned int numRequest = 0;
            unsigned int numRequestRec = 0;
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
            uint64_t nodeId = 0;
            SegmentList list;
    };

    UDPSocket socket;
    IARP *arp;

    typedef std::map<uint64_t,SegmentList *> SegmentMap;
    typedef std::map<uint64_t,uint64_t> SequenceList;

    static SegmentMap segmentMap;
    SegmentMap networkSegmentMap;
    SegmentList mySegmentList;
    SequenceList sequenceList;
    bool useGlobal = false;

    std::deque<uint16_t> request;
    L3Address myAddress;
    IPv4Address myAddressIp4;
    cMessage *myTimer = nullptr;
    cMessage *retryTimer = nullptr;
    cMessage *informTimeOut = nullptr;
    FuzzYControl *fuzzy = nullptr;
    uint64_t mySeqNumber = 0;


    int numSent = 0;
    int numReceived = 0;
    int numDeleted = 0;
    int numDuplicated = 0;
    int numSegPresent = 0;
    simtime_t endReception = 0;
    simtime_t startReception = 0;


    int numSentB = 0;
    int numReceivedB = 0;
    int numDeletedB = 0;
    int numDuplicatedB = 0;

    struct ConnectInTransit
    {
            L3Address nodeId;
            cMessage timer;
            uint32_t segmentId;
            cSimpleModule *owner;
            std::set<uint16_t> segmentInTransit;
            ConnectInTransit(L3Address id, uint32_t s,uint64_t nseg, cSimpleModule *o)
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
    cMessage *pendingRequestTimer = nullptr;
    unsigned int numParallelRequest = 0;

    uint16_t requestPerPacket;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t outOfOrderPkSignal;
    static simsignal_t dropPkSignal;
    static simsignal_t queueLengthSignal;

    cMessage periodicTimer; // periodic timer

    uint32_t totalSegments = 0;
    uint32_t segmentSize = 0;
    uint32_t  maxPacketSize = 0;

    int destPort = 0;
    int localPort = 0;

    simtime_t lastPacket;
    double rateLimit = 0;

    bool writeData = false;
    unsigned int numRegData  = 0;
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
    virtual bool processPacket(cPacket *pk);
    virtual void processRequest(cPacket *pkt);
    virtual void actualizeList(UDPBasicPacketP2P *pkt);
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
    virtual void handleMessage(cMessage *msg);
    virtual uint64_t selectBest(const std::vector<uint64_t> &);
    virtual std::vector<UDPBasicP2P2C::InfoData> selectBestList(const std::vector<uint64_t> &address, const std::vector<SegmentList> &List);
    virtual std::vector<UDPBasicP2P2C::InfoData>  chooseDestAddrList();

    virtual void purgePendingRequest(uint64_t segment);


    UDPBasicPacketP2PNotification *getPacketWitMap();
    virtual void informChanges();
    virtual bool processMsgChanges(cPacket *msg);


   public:
    UDPBasicP2P2C();
    virtual ~UDPBasicP2P2C();
};

}

#endif

