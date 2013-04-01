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


#ifndef __INET_UDPBASICP2P_H
#define __INET_UDPBASICP2P_H


#include <deque>
#include <map>
#include <set>

#include "INETDefs.h"
#include "UDPSocket.h"
#include "ARP.h"
#include "fis.h"
#include "WirelessNumHops.h"

/**
 * UDP application. See NED for more info.
 */
class INET_API UDPBasicP2P : public cSimpleModule, protected INotifiable
{
    private:
    class DelayMessage : public cMessage
    {
           uint64_t destination;
           cPacket *pkt;
        public:
           void setPkt(cPacket *p){pkt = p;}
           cPacket *getPkt(){return pkt;}

           void setAddr(uint64_t p){destination = p;}
           uint64_t getAddr(){return destination;}
    };

    struct SegmentInTransitInfo
    {
           uint64_t destination;
           std::set<uint16_t> subSegmentPending;
           SegmentInTransitInfo()
           {
               subSegmentPending.clear();
           }
           ~SegmentInTransitInfo()
           {
               subSegmentPending.clear();
           }
    };

    typedef std::multimap<simtime_t,DelayMessage*> TimeQueue;
    TimeQueue timeQueue;
    cMessage *queueTimer;

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
    UDPSocket socket;
    ARP *arp;
    typedef std::set<uint32_t> SegmentList;
    typedef std::map<uint64_t,SegmentList *> SegmentMap;
    static SegmentMap segmentMap;
    SegmentList mySegmentList;
    std::deque<uint16_t> request;
    uint64_t myAddress;
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


    std::map<uint32_t,SegmentInTransitInfo> segmentInTransit;


    uint16_t requestPerPacket;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t outOfOrderPkSignal;
    static simsignal_t dropPkSignal;

    uint32_t totalSegments;
    uint32_t segmentSize;
    uint32_t  maxPacketSize;

    int destPort;
    int localPort;

    simtime_t lastPacket;
    double rateLimit;


  protected:
    virtual int numInitStages() const {return 7;}
    virtual void initialize(int stage);
    virtual void finish();

    void processMyTimer();
    // chooses random destination address
    void generateRequestNew();
    void generateRequestSub();
    void processPacket(cPacket *pk);
    void processReques(cPacket *pkt);
    void sendRateLimit(cPacket *pkt, const uint64_t &,double rate);
    void sendQueue();
    virtual uint64_t chooseDestAddr();
    virtual uint64_t chooseDestAddr(uint32_t segmentId);
    virtual void getList(std::vector<uint64_t> &);
    virtual void getList(std::vector<uint64_t> &address,uint32_t segmentId);
    uint64_t searchBestSegment(const uint64_t &);
    virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void handleMessage(cMessage *msg);
    virtual uint64_t selectBest(const std::vector<uint64_t> &);
  public:
    UDPBasicP2P();
    virtual ~UDPBasicP2P();
};

#endif

