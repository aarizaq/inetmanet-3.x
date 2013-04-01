//
// Copyright (C) 2012 Alfonso Ariza, Universidad de Malaga
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


#include "UDPBasicP2P.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "UDPBasicPacketP2P_m.h"
#include "GlobalWirelessLinkInspector.h"
#include <algorithm>
#include "MobilityAccess.h"

Define_Module(UDPBasicP2P);

simsignal_t UDPBasicP2P::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicP2P::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicP2P::outOfOrderPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicP2P::dropPkSignal = SIMSIGNAL_NULL;

UDPBasicP2P::SegmentMap UDPBasicP2P::segmentMap;

UDPBasicP2P::VectorList UDPBasicP2P::vectorList;

UDPBasicP2P::UDPBasicP2P()
{
    myTimer = new cMessage("UDPBasicP2P-timer");
    queueTimer = new cMessage("UDPBasicP2P-queueTimer");
    fuzzy = NULL;
    numSent = 0;
    numReceived = 0;
    numDeleted = 0;
    numDuplicated = 0;
    numSegPresent = 0;
    segmentInTransit.clear();
    vectorList.clear();
}

UDPBasicP2P::~UDPBasicP2P()
{
    cancelAndDelete(myTimer);
    cancelAndDelete(queueTimer);
    while(!timeQueue.empty())
    {
        delete timeQueue.begin()->second->getPkt();
        delete timeQueue.begin()->second;
        timeQueue.erase(timeQueue.begin());
    }
    clientList.clear();
    segmentInTransit.clear();
}

void UDPBasicP2P::initialize(int stage)
{

    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage == 6)
       WirelessNumNeig();

    if (stage != 5)
        return;

    if (!GlobalWirelessLinkInspector::isActive())
        opp_error("GlobalWirelessLinkInspector not found");

    arp = ArpAccess().get();

    if (par("fuzzy"))
    {
        fuzzy = new FuzzYControl(par("fuzzyFisFile").stringValue());
    }

    totalSegments = par("totalSegments").longValue();
    segmentSize = par("segmentSize").longValue();
    uint64_t initseg = par("numInitialSegments").longValue();
    IPvXAddress myAddr = IPvXAddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
    myAddress = arp->getDirectAddressResolution(myAddr.get4()).getInt();


     maxPacketSize = par("maxPacketSize");
     rateLimit = par("packetRate");
     requestPerPacket = par("requestPerPacket");
     lastPacket = 0;

    if (totalSegments == initseg)
    {
        for (unsigned int i = 0; i < totalSegments; i++)
            mySegmentList.insert(i);
    }
    else if (initseg == 0)
    {
        mySegmentList.clear();
    }
    else
    {
        // random initialization
        while (mySegmentList.size()<initseg)
        {
            mySegmentList.insert(intuniform(0,initseg));
        }
    }

    segmentMap[myAddress] = &mySegmentList;

    if (mySegmentList.size()<totalSegments)
    {
        startReception = par("startTime");
        endReception = SimTime::getMaxTime();
        scheduleAt(startReception, myTimer);
    }
    else
    {
        startReception = 0;
        endReception = 0;
    }

    localPort = par("localPort");
    destPort = par("destPort");
    request.resize(totalSegments,0);

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");
    outOfOrderPkSignal = registerSignal("outOfOrderPk");
    dropPkSignal = registerSignal("dropPk");
    numSegPresent = (int)mySegmentList.size();

    clientList.clear();
    timeQueue.clear();
    WATCH(numSegPresent);
}


uint64_t UDPBasicP2P::chooseDestAddr()
{
    std::vector<uint64_t> address;
    getList(address);
    return selectBest(address);
}


uint64_t UDPBasicP2P::chooseDestAddr(uint32_t segmentId)
{
    std::vector<uint64_t> address;
    getList(address,segmentId);
    return selectBest(address);
}


uint64_t UDPBasicP2P::selectBest(const std::vector<uint64_t> &address)
{
    unsigned int hops = 1000;
    double maxfuzzyCost = 0;
    uint64_t addr = 0;
    ManetAddress myAdd = ManetAddress(MACAddress(myAddress));
    for (unsigned int i =0 ;i<address.size(); i++)
    {
        std::vector<ManetAddress> route;
        ManetAddress aux = ManetAddress(MACAddress(address[i]));
        if (GlobalWirelessLinkInspector::getRouteWithLocator(myAdd, aux, route))
        {
            bool areNei;
            GlobalWirelessLinkInspector::areNeighbour(myAdd,aux,areNei);
            if (areNei)
            {
                return address[i];
            }
            int neigh;
            GlobalWirelessLinkInspector::Link cost;
            GlobalWirelessLinkInspector::getNumNodes(aux, neigh);
            GlobalWirelessLinkInspector::getCostPath(route, cost);
            if (fuzzy)
            {
                int nodesN = -1;
                nodesN = getNumNeighNodes(address[i],120.0);
                std::vector<double> inputData;
                inputData.push_back(route.size()-1);
                inputData.push_back(cost.costEtx);
                //inputData.push_back(1);
                if (nodesN > neigh)
                    inputData.push_back(nodesN);
                else
                    inputData.push_back(neigh);
                double fuzzyCost;
                fuzzy->runFuzzy(inputData,fuzzyCost);
                if (par("forceMinHop").boolValue())
                {
                    if (route.size() < hops)
                    {
                        hops = route.size();
                        addr = address[i];
                        maxfuzzyCost = fuzzyCost;
                    }
                    else if (fuzzyCost > maxfuzzyCost && route.size() == hops)
                    {
                        hops = route.size();
                        addr = address[i];
                        maxfuzzyCost = fuzzyCost;
                    }
                }
                else
                {
                    if (fuzzyCost > maxfuzzyCost)
                    {
                        hops = route.size();
                        addr = address[i];
                        maxfuzzyCost = fuzzyCost;
                    }
                }
            }
            else
            {
                if (route.size() < hops)
                {
                    hops = route.size();
                    addr = address[i];
                }
            }
        }
    }
    return addr;
}

void UDPBasicP2P::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (msg == queueTimer)
            sendQueue();
        else if (msg == myTimer)
            processMyTimer();
        else
            opp_error("timer message unknown");
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        // process incoming packet
        if (myTimer->isScheduled())
            cancelEvent(myTimer);
        processPacket(PK(msg));
        if (mySegmentList.size() == totalSegments && endReception != 0)
            endReception = simTime();
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (!timeQueue.empty() && !queueTimer->isScheduled())
        scheduleAt(timeQueue.begin()->first,queueTimer);

    if (mySegmentList.size()<totalSegments && !myTimer->isScheduled())
        scheduleAt(simTime()+par("requestTimer"), myTimer);
    numSegPresent = (int)mySegmentList.size();
}

void UDPBasicP2P::sendQueue()
{
    while(!timeQueue.empty() && timeQueue.begin()->first <= simTime())
    {
        DelayMessage *delayM = timeQueue.begin()->second;
        timeQueue.erase(timeQueue.begin());
        IPv4Address desAdd = arp->getInverseAddressResolution(MACAddress(delayM->getAddr()));
        numSent++;
        socket.sendTo(delayM->getPkt(), desAdd, destPort);
        delete delayM;
    }
}

void UDPBasicP2P::processMyTimer()
{
    if (!segmentInTransit.empty() && segmentInTransit.begin()->second.subSegmentPending.empty())
    {
        while(1)
        {
            if (segmentInTransit.begin()->second.subSegmentPending.empty())
                segmentInTransit.erase(segmentInTransit.begin());
            else
                break;
        }
    }
    if (segmentInTransit.empty())
        generateRequestNew();
    else
        generateRequestSub();
}

void UDPBasicP2P::processPacket(cPacket *pk)
{
    if (pk->getKind() == UDP_I_ERROR)
    {
        EV << "UDP error received\n";
        delete pk;
        return;
    }

    UDPBasicPacketP2P *pkt = check_and_cast<UDPBasicPacketP2P*>(pk);
    if (pkt->getType() == REQUEST)
    {
        processReques(pk);
    }
    else if (pkt->getType() == SEGMEN)
    {
        SegmentList::iterator it = mySegmentList.find(pkt->getSegmentId());
        if (it == mySegmentList.end())
        {
            // check sub segments
            if (pkt->getSubSegmentId() == 0)
                mySegmentList.insert(pkt->getSegmentId());
            else
            {
                std::map<uint32_t,SegmentInTransitInfo>::iterator it = segmentInTransit.find(pkt->getSegmentId());
                if (it == segmentInTransit.end())
                {
                    SegmentInTransitInfo info;
                    uint64_t numSubSegments = ceil((double)pkt->getTotalSize()/(double)maxPacketSize);
                    for (unsigned int i = 1; i<= numSubSegments; i++)
                        info.subSegmentPending.insert(i);
                    info.destination = pkt->getNodeId();

                    segmentInTransit.insert (std::pair<uint32_t,SegmentInTransitInfo>(pkt->getSegmentId(),info));
                    it = segmentInTransit.find(pkt->getSegmentId());
                }
                        // erase the subsegment from transit list
                std::set<uint16_t>::iterator itaux = it->second.subSegmentPending.find(pkt->getSubSegmentId());
                if  (itaux != it->second.subSegmentPending.end())
                {
                        it->second.subSegmentPending.erase(itaux);
                }
                if (it->second.subSegmentPending.empty())
                {
                    mySegmentList.insert(pkt->getSegmentId());
                    segmentInTransit.erase(it);
                }
            }
        }
        else
        {
            // check that the segment is not in the transit list
            std::map<uint32_t,SegmentInTransitInfo>::iterator itAux = segmentInTransit.find(pkt->getSegmentId());
            if (itAux != segmentInTransit.end())
                segmentInTransit.erase(itAux);
        }
    }
    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);
    numReceived++;
    delete pk;
}

void UDPBasicP2P::generateRequestSub()
{
    if (totalSegments > mySegmentList.size())
    {
        std::map<uint32_t,SegmentInTransitInfo>::iterator segmentProc = segmentInTransit.begin();

        uint32_t segmentId = segmentProc->first;
        if (segmentProc->second.subSegmentPending.empty())
            opp_error("");

        if (par("changeSugSegment").boolValue())
            segmentProc->second.destination = chooseDestAddr(segmentId);

        ClientList::iterator it = clientList.find(segmentProc->second.destination);

        if (it == clientList.end())
        {
            InfoClient infoC;
            infoC.numRequest = 1;
            infoC.numRequestRec = 0;
            clientList.insert(std::pair<uint64_t,InfoClient>(segmentProc->second.destination,infoC));
        }

        std::vector<uint16_t> vectorAux;
        vectorAux.clear();

        for (std::set<uint16_t>::iterator itset = segmentProc->second.subSegmentPending.begin(); itset != segmentProc->second.subSegmentPending.end(); ++itset)
        {
            vectorAux.push_back(*itset);
            if (vectorAux.size() == requestPerPacket)
            {
                UDPBasicPacketP2P *pkt = new UDPBasicPacketP2P;
                pkt->setType(REQUEST);
                pkt->setSegmentId(segmentId);
                pkt->setNodeId(myAddress);
                pkt->setSubSegmentRequestArraySize(vectorAux.size());
                for (unsigned int j = 0; j<vectorAux.size(); j++)
                    pkt->setSubSegmentRequest(j,vectorAux[j]);
                pkt->setByteLength(10+(vectorAux.size()*2));
                sendRateLimit(pkt, segmentProc->second.destination, rateLimit);
                it->second.numRequest++;
                vectorAux.clear();
            }
        }
        if (!vectorAux.empty())
        {
            UDPBasicPacketP2P *pkt = new UDPBasicPacketP2P;
            pkt->setType(REQUEST);
            pkt->setSegmentId(segmentId);
            pkt->setNodeId(myAddress);
            pkt->setSubSegmentRequestArraySize(vectorAux.size());
            for (unsigned int j = 0; j<vectorAux.size(); j++)
                pkt->setSubSegmentRequest(j,vectorAux[j]);
            pkt->setByteLength(10+(vectorAux.size()*2));
            sendRateLimit(pkt, segmentProc->second.destination, rateLimit);
            it->second.numRequest++;
            vectorAux.clear();
        }
    }
    else
        segmentInTransit.clear();
}


void UDPBasicP2P::generateRequestNew()
{
    if (totalSegments > mySegmentList.size())
    {
        uint64_t best = chooseDestAddr();
        UDPBasicPacketP2P *pkt = new UDPBasicPacketP2P;
        pkt->setType(REQUEST);
        pkt->setNodeId(myAddress);
        pkt->setByteLength(10);

        ClientList::iterator it = clientList.find(best);
        if (it == clientList.end())
        {
            InfoClient infoC;
            infoC.numRequest = 1;
            infoC.numRequestRec = 0;
            clientList.insert(std::pair<uint64_t,InfoClient>(best,infoC));
        }
        else
            it->second.numRequest++;

        IPv4Address desAdd = arp->getInverseAddressResolution(MACAddress(best));
        if (desAdd.isUnspecified())
            opp_error("address unknown");
        numSent++;
        socket.sendTo(pkt, desAdd, destPort);
    }
}


void UDPBasicP2P::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
}

void UDPBasicP2P::getList(std::vector<uint64_t> &address)
{
    address.clear();
    for (SegmentMap::iterator it = segmentMap.begin(); it != segmentMap.end(); ++it)
    {
        if (it->first == myAddress)
            continue;
        SegmentList * nodeSegmentList = it->second;

        if (nodeSegmentList->size() > mySegmentList.size())
            address.push_back(it->first);
        else
        {
            /*
            std::set_difference(nodeSegmentList->begin(),nodeSegmentList->end(),mySegmentList.begin(),mySegmentList.end(),std::inserter(result, result.end()));
            std::set_difference(nodeSegmentList->begin(),nodeSegmentList->end(),mySegmentList.begin(),mySegmentList.end(),std::inserter(result, result.end()));
            if (!result.empty())
                address.push_back(it->first);
                */
            for (SegmentList::iterator it2 = nodeSegmentList->begin(); it2 != nodeSegmentList->end();++it2)
            {
                SegmentList::iterator it3 = mySegmentList.find(*it2);
                if (it3 == mySegmentList.end())
                {
                    address.push_back(it->first);
                    break;
                }
            }
        }
    }
}


void UDPBasicP2P::getList(std::vector<uint64_t> &address,uint32_t segmentId)
{
    address.clear();
    for (SegmentMap::iterator it = segmentMap.begin(); it != segmentMap.end(); ++it)
    {
        if (it->first == myAddress)
            continue;
        SegmentList * nodeSegmentList = it->second;
        SegmentList result;
        SegmentList::iterator it2 = nodeSegmentList->find(segmentId);
        if (it2 != nodeSegmentList->end())
            address.push_back(it->first);
    }
}

uint64_t UDPBasicP2P::searchBestSegment(const uint64_t & address)
{

    SegmentMap::iterator it = segmentMap.find(address);
    if (it == segmentMap.end())
        opp_error("Node not found in segmentMap");
    SegmentList * nodeSegmentList = it->second;
    SegmentList result;
    std::set_difference(mySegmentList.begin(),mySegmentList.end(),nodeSegmentList->begin(),nodeSegmentList->end(),std::inserter(result, result.end()));
    if (result.empty())
        opp_error("request error");
    uint16_t min = 64000;
    uint64_t seg = UINT64_MAX;
    for (SegmentList::iterator it2 = result.begin(); it2 != result.end(); ++it2)
    {
        SegmentList::iterator it3 =  mySegmentList.find(*it2);
        if (it3 == mySegmentList.end())
        {
            opp_error("request error segment not found in my list");
            continue;
        }
        if (request[*it2]<min)
        {
            min = request[*it2];
            seg = *it2;
        }
    }
    if (seg == UINT64_MAX)
        opp_error("request error segment not found");

    request[seg]++;
    return seg;
}


void UDPBasicP2P::processReques(cPacket *p)
{
    UDPBasicPacketP2P *pkt = dynamic_cast<UDPBasicPacketP2P*>(p);
    ClientList::iterator it = clientList.find(pkt->getNodeId());
    if (it == clientList.end())
    {

        InfoClient infoC;
        infoC.numRequestRec = 1;
        infoC.numRequest = 0;
        clientList.insert(std::pair<uint64_t,InfoClient>(pkt->getNodeId(),infoC));
    }
    else
        it->second.numRequestRec++;

    if (pkt->getSubSegmentRequestArraySize() == 0)
    {
        // first request
        UDPBasicPacketP2P *pkt2 = new UDPBasicPacketP2P;
        pkt2->setType(SEGMEN);
        pkt2->setNodeId(myAddress);
        pkt2->setByteLength(maxPacketSize);
        uint64_t desAdd = pkt->getNodeId();
        pkt2->setSegmentId(searchBestSegment(desAdd));
        uint64_t remain = segmentSize;
        pkt2->setTotalSize(remain);
        uint16_t subSegId = 1;

        if (remain <= maxPacketSize)
        {
            pkt2->setSubSegmentId(0);
            pkt2->setByteLength(remain);
            sendRateLimit(pkt2, desAdd, rateLimit);
            remain = 0;
        }

        while (remain > 0)
        {
            pkt2->setSubSegmentId(subSegId);
            subSegId++;
            if (remain > maxPacketSize)
            {
                sendRateLimit(pkt2->dup(), desAdd, rateLimit);
                remain -= maxPacketSize;
            }
            else
            {
                pkt2->setByteLength(remain);
                sendRateLimit(pkt2, desAdd, rateLimit);
                remain = 0;
            }
        }
    }
    else
    {
        // first request
          UDPBasicPacketP2P *pkt2 = new UDPBasicPacketP2P;
          pkt2->setType(SEGMEN);
          pkt2->setNodeId(myAddress);
          pkt2->setByteLength(maxPacketSize);
          uint64_t desAdd = pkt->getNodeId();
          pkt2->setSegmentId(pkt->getSegmentId());
          uint64_t remain = (uint64_t)totalSegments * (uint64_t)segmentSize;

          pkt2->setTotalSize(remain);
          for (unsigned int i = 0; i< pkt->getSubSegmentRequestArraySize()-1; i++)
          {
              pkt2->setSubSegmentId(pkt->getSubSegmentRequest(i));
              sendRateLimit(pkt2->dup(), desAdd, rateLimit);
          }
          pkt2->setSubSegmentId(pkt->getSubSegmentRequest(pkt->getSubSegmentRequestArraySize()-1));
          sendRateLimit(pkt2, desAdd, rateLimit);
    }
}

void UDPBasicP2P::sendRateLimit(cPacket *pkt, const uint64_t &addr, double rate)
{

    ClientList::iterator it = clientList.find(addr);

    IPvXAddress auxAddress = IPvXAddress(arp->getInverseAddressResolution(MACAddress(addr)));
    if (simTime() - it->second.lastMessage > rate)
    {
        numSent++;
        socket.sendTo(pkt, auxAddress, destPort);
        lastPacket = simTime();
        it->second.lastMessage = simTime();
    }
    else
    {
        // prepare a delayed
        DelayMessage *delayM = new DelayMessage();
        delayM->setPkt(pkt);
        delayM->setAddr(addr);
        //
        it->second.lastMessage += rate;
        lastPacket += rate;
        timeQueue.insert (std::pair<simtime_t,DelayMessage*>(it->second.lastMessage,delayM));
        if (queueTimer->isScheduled())
        {
            if (queueTimer->getArrivalTime() > timeQueue.begin()->first)
            {
                cancelEvent(queueTimer);
                scheduleAt(timeQueue.begin()->first,queueTimer);
            }
        }
        else
        {
            scheduleAt(timeQueue.begin()->first,queueTimer);
        }
        GlobalWirelessLinkInspector::setQueueSize(ManetAddress(MACAddress(myAddress)), timeQueue.size());
    }
}


void UDPBasicP2P::finish()
{
    recordScalar("packets send", numSent);
    recordScalar("packets received", numReceived);
    recordScalar("segment present", numSegPresent);
    if (mySegmentList.size() < totalSegments)
    {
        recordScalar("time need", SimTime::getMaxTime());
    }
    else
    {
        double timeNeed = endReception.dbl()-startReception.dbl();
        recordScalar("time need", timeNeed);
    }
}


void UDPBasicP2P::WirelessNumNeig()
{
    // TODO Auto-generated constructor stub
    // fill in routing tables with static routes

    if (!vectorList.empty())
        return;

    cTopology topo("topo");
    topo.extractByProperty("node");
    for (int i = 0; i < topo.getNumNodes(); i++)
    {
        cTopology::Node *destNode = topo.getNode(i);
        IMobility *mod;
        IInterfaceTable* itable = IPvXAddressResolver().findInterfaceTableOf(destNode->getModule());
        bool notfound = true;
        uint64_t add;

        for (int j = 0 ; j < itable->getNumInterfaces(); j++)
        {
            InterfaceEntry *e = itable->getInterface(j);
            if (e->getMacAddress().isUnspecified())
                continue;
            if (e->isLoopback())
                continue;
            if (!notfound)
                break;
            for(SegmentMap::iterator it = segmentMap.begin(); it != segmentMap.end();++it)
            {
                if (it->first == e->getMacAddress().getInt())
                {
                    notfound = false;
                    add = e->getMacAddress().getInt();
                    break;
                }
            }
        }

        if (notfound)
            continue;

        mod = MobilityAccess().get(destNode->getModule());
        if (mod == NULL)
            opp_error("node or mobility module not found");

        vectorList[add] = mod;
    }
}

int UDPBasicP2P::getNumNeighNodes(uint64_t add,double dist)
{
    return 0;
    /*
    VectorList::iterator it = vectorList.find(add);
    if (it == vectorList.end())
        opp_error("Node not found");
    int cont = 0;
    Coord ci = it->second->getCurrentPosition();
    for (VectorList::iterator it2 = vectorList.begin();it2 != vectorList.end();++it2)
    {
        if (it2->first == add)
            continue;
        Coord cj = it2->second->getCurrentPosition();
        if (ci.distance(cj) <= dist)
            cont++;
    }
    return cont;
    */
}
