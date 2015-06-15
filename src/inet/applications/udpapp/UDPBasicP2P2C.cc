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


#include "UDPBasicP2P2C.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/common/GlobalWirelessLinkInspector.h"
#include <algorithm>
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

Define_Module(UDPBasicP2P2C);

simsignal_t UDPBasicP2P2C::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPBasicP2P2C::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t UDPBasicP2P2C::outOfOrderPkSignal = registerSignal("outOfOrderPk");
simsignal_t UDPBasicP2P2C::dropPkSignal = registerSignal("dropPk");

simsignal_t UDPBasicP2P2C::queueLengthSignal = registerSignal("queueLength");


UDPBasicP2P2C::SegmentMap UDPBasicP2P2C::segmentMap;

UDPBasicP2P2C::VectorList UDPBasicP2P2C::vectorList;

UDPBasicP2P2C::InverseAddres UDPBasicP2P2C::inverseAddress;
UDPBasicP2P2C::DirectAddres UDPBasicP2P2C::directAddress;

std::vector<int> UDPBasicP2P2C::initNodes;

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("modoP2P");
    if (!e) enums.getInstance()->add(e = new cEnum("modoP2P"));
    e->insert(UDPBasicP2P2C::CONCAVO, "concavo");
    e->insert(UDPBasicP2P2C::ADITIVO, "aditivo");
    e->insert(UDPBasicP2P2C::ADITIVO_PONDERADO, "aditivo_ponderado");
    e->insert(UDPBasicP2P2C::MIN_HOP, "minhop");
    e->insert(UDPBasicP2P2C::RANDOM, "random");
);


UDPBasicP2P2C::UDPBasicP2P2C()
{
    myTimer = new cMessage("UDPBasicP2P-timer");
    queueTimer = new cMessage("UDPBasicP2P-queueTimer");
    informTimeOut = new cMessage("UDPBasicP2P-informTimeOut");
    parallelConnection.clear();
    vectorList.clear();
    inverseAddress.clear();
    directAddress.clear();
    initNodes.clear();
    pendingRequestTimer = new cMessage("pendingRequestTimer");
    sequenceList.clear();
}

UDPBasicP2P2C::~UDPBasicP2P2C()
{
    cancelAndDelete(myTimer);
    cancelAndDelete(queueTimer);
    cancelEvent(&periodicTimer);
    cancelAndDelete(informTimeOut);
    while(!timeQueue.empty())
    {
        delete timeQueue.back();
        timeQueue.pop_back();
    }
    auto it = segmentMap.find(myAddress.toMAC().getInt());
    if (it != segmentMap.end())
        segmentMap.erase(it);
    mySegmentList.clear();
    clientList.clear();
    for (auto it = parallelConnection.begin();it != parallelConnection.end();++it)
    {
        cancelEvent(&it->timer);
        it->segmentInTransit.clear();
    }

    parallelConnection.clear();
    if (outfile)
    {
        outfile->close();
        delete outfile;
    }
    if (routing)
        delete routing;

    while (!pendingRequest.empty())
    {
        delete pendingRequest.begin()->second;
        pendingRequest.erase(pendingRequest.begin());
    }
    cancelAndDelete(pendingRequestTimer);

    while (!networkSegmentMap.empty())
    {
        delete networkSegmentMap.begin()->second;
        networkSegmentMap.erase(networkSegmentMap.begin());
    }
}

void UDPBasicP2P2C::initialize(int stage)
{

    // because of AddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.

    if (stage == 5)
    {
        const char *addrModeStr = par("modoP2P").stringValue();

        int p2pMode = cEnum::get("modoP2P")->lookup(addrModeStr);
        if (p2pMode == -1)
            throw cRuntimeError("Invalid modoP2P: '%s'", addrModeStr);
        modo = (modoP2P) p2pMode;

        if (!GlobalWirelessLinkInspector::isActive())
            throw cRuntimeError("GlobalWirelessLinkInspector not found");

        if (par("fuzzy"))
        {
            fuzzy = new FuzzYControl(par("fuzzyFisFile").stringValue());
            fuzzy->setSizeParam(par("numVarFuzzy").longValue());
        }

        if (par("useGlobal"))
            useGlobal = true;

        totalSegments = par("totalSegments").longValue();
        segmentSize = par("segmentSize").longValue();

        uint64_t initseg = 0;
        L3Address myAddr = L3AddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
        myAddressIp4 = myAddr.toIPv4();
        IInterfaceTable * ift = L3AddressResolver().interfaceTableOf(this->getParentModule());

        myAddress = L3Address();
        for (int i = 0; i < ift->getNumInterfaces(); i++)
        {
            InterfaceEntry * e = ift->getInterface(i);
            if (e->ipv4Data())
            {
                if (e->ipv4Data()->getIPAddress() == myAddr.toIPv4())
                {
                    myAddress = L3Address(e->getMacAddress());
                    break;
                }
            }
        }

        if (myAddr.toIPv4() == IPv4Address::UNSPECIFIED_ADDRESS)
            throw cRuntimeError("addr invalid");

        if (myAddress == L3Address())
            throw cRuntimeError("MACAddress addr invalid");

        inverseAddress[myAddress.toMAC().getInt()] = myAddr.toIPv4();
        directAddress[myAddr.toIPv4()] = myAddress.toMAC().getInt();

        if (!par("initNodesRand").boolValue())
            initseg = par("numInitialSegments").longValue();
        else
        {
            // cModule * myNode = L3AddressResolver().findHostWithAddress(myAddr);
            cModule * myNode = this->getParentModule();
            if (!myNode->isVector())
                throw cRuntimeError("No es un vector");
            if (initNodes.empty())
            {
                int numNodes = myNode->getVectorSize();
                while ((int) initNodes.size() < par("numInitNodesRand").longValue())
                {
                    int val = intuniform(0, numNodes);
                    bool isInside = false;
                    for (int i = 0; i < (int) initNodes.size(); i++)
                    {
                        if (val == initNodes[i])
                        {
                            isInside = true;
                            break;
                        }
                    }
                    if (isInside)
                        continue;
                    initNodes.push_back(val);
                }
            }
            int myIndex = myNode->getIndex();
            for (int i = 0; i < (int) initNodes.size(); i++)
            {
                if (myIndex == initNodes[i])
                {
                    initseg = totalSegments;
                    break;
                }
            }
        }
        maxPacketSize = par("maxPacketSize");
        rateLimit = par("packetRate");
        requestPerPacket = par("requestPerPacket");
        lastPacket = 0;
        serverTimer = par("serverRate");

        if (totalSegments == initseg)
        {
            for (unsigned int i = 1; i <= totalSegments; i++)
                mySegmentList.insert(i);
        }
        else if (initseg == 0)
        {
            mySegmentList.clear();
        }
        else
        {
            // random initialization
            while (mySegmentList.size() < initseg)
            {
                mySegmentList.insert(intuniform(1, initseg+1));
            }
        }
        segmentMap[myAddress.toMAC().getInt()] = &mySegmentList;
        if (mySegmentList.size() < totalSegments)
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

        if (par("periodicTimer").doubleValue() > 0)
            scheduleAt(simTime() + par("periodicTimer"), &periodicTimer);

        localPort = par("localPort");
        destPort = par("destPort");
        request.resize(totalSegments, 0);

        socket.setOutputGate(gate("udpOut"));
        socket.bind(localPort);
        numSegPresent = (int) mySegmentList.size();

        numParallelRequest = par("numParallelRequest");

        if (numSegPresent>0)
        {
            scheduleAt(simTime()+uniform(0,0.01),informTimeOut);
        }

        clientList.clear();
        timeQueue.clear();
        WATCH(numSegPresent);
        // WATCH((unsigned int)timeQueue.size());

        const char *listNodes = par("listNodes");
        cStringTokenizer tokenizer(listNodes);
        const char *token;

        while ((token = tokenizer.nextToken()) != nullptr)
        {
            L3Address addr = L3AddressResolver().resolve(token);
            if (addr == myAddr)
            {
                writeData = true;
                std::string name(token);
                outfile = new std::ofstream(name.c_str(),
                        std::ios_base::out | std::ios_base::trunc | std::ios_base::app);
                *outfile << simulation.getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_SEEDSET)
                        << endl;
            }
        }
    }
    else if (stage == 6)
    {
        WirelessNumNeig();
        GlobalWirelessLinkInspector::initRoutingTables(this, myAddress, true);
        if (par("appRouting").boolValue())
        {
            routing = new WirelessNumHops();
            routing->setRoot(myAddress.toMAC());

            routing->fillRoutingTables(par("coverageArea").doubleValue());
            routing->run();
            routing->setIpRoutingTable();
            for (unsigned int i = 0; i < routing->getNumRoutes(); i++)
            {
                std::deque<MACAddress> pathNode;
                routing->getRoute(i, pathNode);
                if (pathNode.empty())
                    continue;
                GlobalWirelessLinkInspector::setRoute(this, myAddress, L3Address(pathNode.back()),
                        L3Address(pathNode[0]), false);

            }
            delete routing;
            routing = nullptr;
        }
    }
}


uint64_t UDPBasicP2P2C::chooseDestAddr()
{
    std::vector<uint64_t> address;
    getList(address);
    return selectBest(address);
}


uint64_t UDPBasicP2P2C::chooseDestAddr(uint32_t segmentId)
{
    std::vector<uint64_t> address;
    getList(address,segmentId);
    return selectBest(address);
}


std::vector<UDPBasicP2P2C::InfoData>  UDPBasicP2P2C::chooseDestAddrList()
{
    std::vector<uint64_t> address;
    std::vector<SegmentList> Segments;
    getList(address,Segments);
    return selectBestList(address, Segments);
}

bool UDPBasicP2P2C::areDiff(const SegmentList &List1, const SegmentList &List2)
{
    SegmentList result;
    std::set_difference(List1.begin(),List1.end(),List2.begin(),List2.end(),std::inserter(result, result.end()));
    if (!result.empty())
        return true;
    return
        false;
}

std::vector<UDPBasicP2P2C::InfoData> UDPBasicP2P2C::selectBestList(const std::vector<uint64_t> &address, const std::vector<SegmentList> &List)
{
    double costMax = 1e300;
    double maxfuzzyCost = 0;
    uint64_t addr = 0;

    std::vector<UDPBasicP2P2C::InfoData> value;

    L3Address myAdd = myAddress;
    std::vector<uint64_t> winners;
    int cont = numParallelRequest;

    for (unsigned int i =0 ;i<address.size(); i++)
    {
//        bool procNode = true;
//        for (unsigned int l = 0; l < winners.size(); l++)
//        {
//
//            if (!areDiff(List[i],List[winners[l]]))
//            {
//                procNode = false;
//                break;
//            }
//        }
//
//        if (!procNode)
//            continue;

        std::vector<L3Address> route;
        L3Address aux = L3Address(MACAddress(address[i]));
        GlobalWirelessLinkInspector::Link cost;
        bool validRoute = false;

        int neigh = 0;
        if (routing)
        {
            std::deque<MACAddress> pathNode;
            //if (routing->findRouteWithCost(par("coverageArea").doubleValue(), MACAddress(address[i]),pathNode,true,costAdd,costMax))
            if (routing->findRoute(par("coverageArea").doubleValue(), MACAddress(address[i]),pathNode))
            {
                auto it = inverseAddress.find(pathNode[0].getInt());
                if (it == inverseAddress.end())
                    throw cRuntimeError(" address not found %s", pathNode[0].str().c_str());
                IPv4Address desAdd = it->second;
                validRoute = true;
                route.push_back(myAdd);
                for (unsigned int i = 0; i < pathNode.size(); i++)
                    route.push_back(L3Address(pathNode[i]));
            }
        }
        else if (GlobalWirelessLinkInspector::getRouteWithLocator(myAdd, aux, route))
        {
            validRoute = true;
        }
        if(validRoute)
        {
            if (fuzzy)
            {
                int nodesN = -1;
                //nodesN = getNumNeighNodes(address[i],120.0);
                bool areNei;
                GlobalWirelessLinkInspector::areNeighbour(myAdd,aux,areNei);
                if (areNei)
                {
                    InfoData data;
                    winners.push_back(address[i]);
                    cont--;
                }
                // GlobalWirelessLinkInspector::getNumNodes(aux, neigh);
                if (modo == CONCAVO)
                    GlobalWirelessLinkInspector::getWorst(route, cost);
                else
                    GlobalWirelessLinkInspector::getCostPath(route, cost);
                if (modo == ADITIVO_PONDERADO)
                     cost.costEtx/=(route.size()-1);

                std::vector<double> inputData;
                inputData.push_back(route.size()-1);
                inputData.push_back(cost.costEtx);

                if (nodesN > neigh)
                    inputData.push_back(nodesN);
                else
                    inputData.push_back(neigh);

                double fuzzyCost;
                fuzzy->runFuzzy(inputData,fuzzyCost);
                if (writeData)
                {
                    *outfile << numRegData << " ";
                    for (unsigned int i = 0 ; i < inputData.size(); i++)
                        *outfile << inputData[i] << " ";
                    *outfile << fuzzyCost << endl;
                    numRegData++;
                }

                if (par("forceMinHop").boolValue())
                {
                    if (route.size() < costMax)
                    {
                        costMax = route.size();
                        addr = address[i];
                        maxfuzzyCost = fuzzyCost;
                        winners.clear();
                        winners.push_back(addr);
                    }
                    else if (fuzzyCost > maxfuzzyCost && route.size() == costMax)
                    {
                        costMax = route.size();
                        addr = address[i];
                        maxfuzzyCost = fuzzyCost;
                        winners.clear();
                        winners.push_back(addr);
                    }
                    else if (fuzzyCost == maxfuzzyCost && route.size() == costMax)
                    {
                        addr = address[i];
                        winners.push_back(addr);
                    }
                }
                else
                {
                    if (fuzzyCost > maxfuzzyCost)
                    {
                        costMax = route.size();
                        addr = address[i];
                        maxfuzzyCost = fuzzyCost;
                        winners.clear();
                        winners.push_back(addr);
                    }
                    else if (fuzzyCost == maxfuzzyCost)
                    {
                        addr = address[i];
                        winners.push_back(addr);
                    }
                }
            }
            else
            {

                // GlobalWirelessLinkInspector::getNumNodes(aux, neigh);
                double costPath;

                if (modo == MIN_HOP)
                    costPath = route.size();
                else
                {
                    if (modo == CONCAVO)
                        GlobalWirelessLinkInspector::getWorst(route, cost);
                    else
                        GlobalWirelessLinkInspector::getCostPath(route, cost);
                    if (modo == ADITIVO_PONDERADO)
                        cost.costEtx/=(route.size()-1);
                    costPath = cost.costEtx;
                }
                if (costPath < costMax)
                {
                    costMax = costPath;
                    addr = address[i];
                    winners.clear();
                    winners.push_back(addr);
                }
                else if (costPath == costMax)
                {
                    addr = address[i];
                    winners.push_back(addr);
                }
            }
        }
    }
    if (winners.empty())
        throw cRuntimeError("winners empty");
    else
    {
        int val = intuniform(0,winners.size()-1);
        addr = winners[val];
        for (unsigned int i = 0; i < winners.size(); i++)
        {
            InfoData data;
            data.nodeId = winners[val];
            std::set_difference(mySegmentList.begin(),mySegmentList.end(),List[winners[i]].begin(),List[winners[i]].end(),std::inserter(data.list,data.list.end()));
            value.push_back(data);
        }
    }
    return value;
}


uint64_t UDPBasicP2P2C::selectBest(const std::vector<uint64_t> &address)
{
    double costMax = 1e300;
    double maxfuzzyCost = 0;
    uint64_t addr = 0;
    L3Address myAdd = myAddress;
    std::vector<uint64_t> winners;
    if (modo == UDPBasicP2P2C::RANDOM)
    {
        unsigned int pos = intuniform(0,address.size()-1);
        return address[pos];
    }

    for (unsigned int i = 0 ;i < address.size(); i++)
    {
        std::vector<L3Address> route;
        L3Address aux = L3Address(MACAddress(address[i]));
        bool validRoute = false;
        GlobalWirelessLinkInspector::Link cost;
        int neigh = 0;
        if (routing)
        {

            std::deque<MACAddress> pathNode;
            //if (routing->findRouteWithCost(par("coverageArea").doubleValue(), aux.toMAC(), pathNode,true,costAdd,costMax))
            if (routing->findRoute(par("coverageArea").doubleValue(), MACAddress(address[i]),pathNode))
            {
                auto it = inverseAddress.find(pathNode[0].getInt());
                if (it == inverseAddress.end())
                    throw cRuntimeError(" address not found %s", pathNode[0].str().c_str());
                IPv4Address desAdd = it->second;
                validRoute = true;
                route.push_back(myAdd);
                for (unsigned int i = 0; i < pathNode.size(); i++)
                    route.push_back(L3Address(pathNode[i]));
            }
            else
                throw cRuntimeError("route not found");
        }
        else if (GlobalWirelessLinkInspector::getRouteWithLocator(myAdd, aux, route))
        {
            validRoute = true;
        }

        if (validRoute)
        {
            if (fuzzy)
            {
                int nodesN = -1;
                //nodesN = getNumNeighNodes(address[i],120.0);

#ifdef NEIGH
                bool areNei;
                GlobalWirelessLinkInspector::areNeighbour(myAdd,aux,areNei);
                if (areNei)
                    return address[i];
#endif
                GlobalWirelessLinkInspector::getNumNodes(aux, neigh);
                if (modo == CONCAVO)
                    GlobalWirelessLinkInspector::getWorst(route, cost);
                else
                    GlobalWirelessLinkInspector::getCostPath(route, cost);
                if (modo == ADITIVO_PONDERADO)
                   cost.costEtx/=(route.size()-1);

                std::vector<double> inputData;
                inputData.push_back(route.size()-1);
                inputData.push_back(cost.costEtx);
                if (nodesN > neigh)
                    inputData.push_back(nodesN);
                else
                    inputData.push_back(neigh);

                double fuzzyCost;
                fuzzy->runFuzzy(inputData,fuzzyCost);
                if (writeData)
                {
                    *outfile << numRegData << " ";
                    for (unsigned int i = 0 ; i < inputData.size(); i++)
                        *outfile << inputData[i] << " ";
                    *outfile << fuzzyCost << endl;
                    numRegData++;
                }

                if (par("forceMinHop").boolValue())
                {
                    if (route.size() < costMax)
                    {
                        costMax = route.size();
                        addr = address[i];
                        maxfuzzyCost = fuzzyCost;
                        winners.clear();
                        winners.push_back(addr);
                    }
                    else if (fuzzyCost > maxfuzzyCost && route.size() == costMax)
                    {
                        costMax = route.size();
                        addr = address[i];
                        maxfuzzyCost = fuzzyCost;
                        winners.clear();
                        winners.push_back(addr);
                    }
                    else if (fuzzyCost == maxfuzzyCost && route.size() == costMax)
                    {
                        addr = address[i];
                        winners.push_back(addr);
                    }
                }
                else
                {
                    if (fuzzyCost > maxfuzzyCost)
                    {
                        costMax = route.size();
                        addr = address[i];
                        maxfuzzyCost = fuzzyCost;
                        winners.clear();
                        winners.push_back(addr);
                    }
                    else if (fuzzyCost == maxfuzzyCost)
                    {
                        addr = address[i];
                        winners.push_back(addr);
                    }
                }
            }
            else
            {
                // GlobalWirelessLinkInspector::getNumNodes(aux, neigh);
                double costPath;
                if (modo == MIN_HOP)
                    costPath = route.size();
                else
                {
                    if (modo == CONCAVO)
                        GlobalWirelessLinkInspector::getWorst(route, cost);
                    else
                        GlobalWirelessLinkInspector::getCostPath(route, cost);
                    if (modo == ADITIVO_PONDERADO)
                        cost.costEtx/=(route.size()-1);
                    costPath = cost.costEtx;
                }
                if (costPath < costMax)
                {
                    costMax = costPath;
                    addr = address[i];
                    winners.clear();
                    winners.push_back(addr);
                }
                else if (costPath == costMax)
                {
                    addr = address[i];
                    winners.push_back(addr);
                }
            }
        }
    }
    if (winners.empty())
        throw cRuntimeError("winners empty");
    else
    {
        int val = intuniform(0,winners.size()-1);
        addr = winners[val];
    }
    return addr;
}

void UDPBasicP2P2C::handleMessage(cMessage *msg)
{
    if (myAddressIp4 == IPv4Address("192.168.1.10"))
        EV << "\n";
    if (myAddressIp4 == IPv4Address("192.168.1.35"))
            EV << "\n";
    if (msg->isSelfMessage())
    {
        if (informTimeOut)
        {
            informChanges();
            return;
        }
        if (msg == &periodicTimer)
        {
            emit(queueLengthSignal, timeQueue.size());
            scheduleAt(simTime()+par("periodicTimer"), &periodicTimer);
            return;
        }
        if (msg == pendingRequestTimer)
        {
            while(!pendingRequest.empty() && pendingRequest.begin()->first <= simTime())
            {
                sendNow(pendingRequest.begin()->second);
                pendingRequest.erase(pendingRequest.begin());
            }
            if (!pendingRequest.empty())
            {
                scheduleAt(pendingRequest.begin()->first, pendingRequestTimer);
            }
            return;
        }

        if (msg == queueTimer)
            sendQueue();
        else if (msg == myTimer || strcmp(msg->getName(),"PARALLELTIMER") == 0)
            processMyTimer(msg);
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        UDPBasicPacketP2PNotification *pktNot = dynamic_cast<UDPBasicPacketP2PNotification*>(msg);
        if (pktNot)
        {
            processMsgChanges(pktNot);
            return;
        }

        UDPBasicPacketP2P *pkt = check_and_cast<UDPBasicPacketP2P*>(msg);

        // I am not sure if the actualization of the list must be here
        actualizeList(pkt);
        if (pkt->getType() == GENERAL)
        {
            if (routing)
            {
                if (pkt->getDestination() != myAddress)
                {
                    std::deque<MACAddress> pathNode;

                    routing->findRoute(par("coverageArea").doubleValue(), pkt->getDestination().toMAC(),pathNode);
                    auto it = inverseAddress.find(pathNode[0].getInt());
                    if (it == inverseAddress.end())
                        throw cRuntimeError(" address not found %s", pathNode[0].str().c_str());
                    IPv4Address desAdd = it->second;
                    if (pkt->getControlInfo())
                        delete pkt->removeControlInfo();
                    socket.sendTo(pkt, desAdd, destPort);
                    return;
                }
            }
            delete pkt;
            return;
        }

        if (processPacket(pkt))
        {
            if (mySegmentList.size() == totalSegments && endReception != 0)
            {
                endReception = simTime();
            }
            bool endSim = true;
            for (auto it = segmentMap.begin(); it != segmentMap.end(); ++it)
            {
                if (it->second->size() != totalSegments)
                {
                    endSim = false;
                    break;
                }
            }
            if (endSim)
                endSimulation();
        }
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (!timeQueue.empty() && !queueTimer->isScheduled())
    {
        throw cRuntimeError("Queue not empty and timer not scheduled");
    }


    if (mySegmentList.size()<totalSegments && !myTimer->isScheduled() && (parallelConnection.size() < numParallelRequest))
        scheduleAt(simTime()+par("requestTimer"), myTimer);
    else
    {
        for (unsigned int i = 0; i < parallelConnection.size(); i++)
        {
            if (!parallelConnection[i].timer.isScheduled())
                scheduleAt(simTime()+par("requestTimer"), &parallelConnection[i].timer);
        }
    }
    numSegPresent = (int)mySegmentList.size();
}

void UDPBasicP2P2C::sendNow(UDPBasicPacketP2P *pkt)
{
    auto it = inverseAddress.find(pkt->getDestination().toMAC().getInt());
    if (it == inverseAddress.end())
        throw cRuntimeError(" address not found %s", pkt->getDestination().toMAC().str().c_str());
    IPv4Address desAdd = it->second;
    numSent++;
    if (routing)
    {
        std::deque<IPv4Address> pathNode;
        if (routing->findRoute(par("coverageArea").doubleValue(), desAdd,pathNode))
            desAdd = pathNode[0];
        else
            throw cRuntimeError(" route not found not found %s", desAdd.str().c_str());
    }
    if (pkt->getControlInfo())
        delete pkt->removeControlInfo();
    emit(sentPkSignal, pkt);
    socket.sendTo(pkt, desAdd, destPort);
}

void UDPBasicP2P2C::sendDelayed(UDPBasicPacketP2P *pkt,const simtime_t &delay)
{

    auto it = inverseAddress.find(pkt->getDestination().toMAC().getInt());
    if (it == inverseAddress.end())
        throw cRuntimeError(" address not found %s", pkt->getDestination().toMAC().str().c_str());
    IPv4Address desAdd = it->second;
    if (pkt->getControlInfo())
        delete pkt->removeControlInfo();

    pendingRequest.insert(std::make_pair(delay,pkt));
    if (pendingRequestTimer->isScheduled())
    {
        if (pendingRequestTimer->getArrivalTime() == pendingRequest.begin()->first)
            return;
        else
            cancelEvent(pendingRequestTimer);
    }
    scheduleAt(pendingRequest.begin()->first, pendingRequestTimer);
}

void UDPBasicP2P2C::purgePendingRequest(uint64_t segment)
{
    for (auto it = pendingRequest.begin(); it != pendingRequest.end();)
    {
        if(it->second->getSegmentId() == segment)
        {
            delete it->second;
            pendingRequest.erase(it++);
        }
        else
            ++it;
    }
}

void UDPBasicP2P2C::sendQueue()
{

    if (timeQueue.empty())
        return;

    DelayMessage *delayM = timeQueue.front();
    if (delayM->getNextTime() > simTime())
    {
        scheduleAt(delayM->getNextTime(),queueTimer);
        return;
    }

    timeQueue.pop_front();
    auto it = inverseAddress.find(delayM->getDestination().toMAC().getInt());
    if (it == inverseAddress.end())
        throw cRuntimeError(" address not found %s", delayM->getDestination().toMAC().str().c_str());
    UDPBasicPacketP2P *pkt = delayM->getPkt(maxPacketSize);
    lastPacket = simTime();
    lastServer = simTime();
    if (delayM->getRemain() > 0)
    {
        delayM->setNextTime(delayM->getNextTime()+par("maxFlowRate").doubleValue());
        timeQueue.push_back(delayM);


/*        if (timeQueue.empty() || (!timeQueue.empty() && timeQueue.back()->getNextTime() < simTime()))
            timeQueue.push_back(delayM);
        else
        {
            // search position
            for (auto it = timeQueue.begin();it != timeQueue.end(); ++it)
            {
                if ((*it)->getNextTime() > delayM->getNextTime())
                {
                    timeQueue.insert(it,delayM);
                    break;
                }
            }
        }*/
    }
    else
        delete delayM;


    if (!timeQueue.empty())
    {
        simtime_t min;
        min.setRaw(INT64_MAX);
        for (unsigned int i = 0 ; i < timeQueue.size();i++)
        {
            if (timeQueue[i]->getNextTime() < min)
            {
                min = timeQueue[i]->getNextTime();
            }
        }

        if ((timeQueue.front()->getNextTime() < simTime()+serverTimer) || min < simTime())
            scheduleAt(simTime()+serverTimer,queueTimer);
        else
            scheduleAt(timeQueue.front()->getNextTime(),queueTimer);

    }
    sendNow(pkt);
    GlobalWirelessLinkInspector::setQueueSize(myAddress, getQueueSize());
}

bool UDPBasicP2P2C::processMyTimer(cMessage *msg)
{

    if (msg == myTimer)
    {
        // lost request;
        generateRequestNew();
        return false;
    }

    ParallelConnection::iterator itPar;
    for (itPar = parallelConnection.begin(); itPar != parallelConnection.end();++itPar)
    {
        if (&(itPar->timer) == msg)
            break;
    }

    if (itPar == parallelConnection.end())
        return true;

    if (itPar->segmentInTransit.empty())
        parallelConnection.erase(itPar);

    numRequestSent++;
    if (parallelConnection.size() < numParallelRequest)
        generateRequestNew();
    else
        generateRequestSub();

    return false;
}

bool UDPBasicP2P2C::processPacket(UDPBasicPacketP2P *pkt)
{
    if (routing)
    {
        if (pkt->getDestination() != myAddress)
        {
            std::deque<MACAddress> pathNode;

            routing->findRoute(par("coverageArea").doubleValue(), pkt->getDestination().toMAC(),pathNode);
            auto it = inverseAddress.find(pathNode[0].getInt());
            if (it == inverseAddress.end())
                throw cRuntimeError(" address not found %s", pathNode[0].str().c_str());
            IPv4Address desAdd = it->second;
            if (pkt->getControlInfo())
                delete pkt->removeControlInfo();
            socket.sendTo(pkt, desAdd, destPort);
            return false;
        }
    }

    bool change = false;

    if (pkt->getType() == REQUEST)
    {
        numRequestServed++;
        processRequest(pkt);
    }
    else if (pkt->getType() == SEGMEN)
    {
        if (myTimer->isScheduled())
            cancelEvent(myTimer);
        numRequestSegmentServed++;
        auto it = mySegmentList.find(pkt->getSegmentId());

        // in transit, purge
        purgePendingRequest(pkt->getSegmentId());
        if (it == mySegmentList.end())
        {
            // check sub segments
            if (pkt->getSubSegmentId() == 0)
            {
                mySegmentList.insert(pkt->getSegmentId());
                informChanges();
            }
            else
            {
                auto itPar = parallelConnection.begin();
                for (; itPar != parallelConnection.end();++itPar)
                {
                    if (itPar->segmentId == pkt->getSegmentId())
                        break;
                }

                if (itPar == parallelConnection.end())
                {
                    uint64_t numSubSegments = ceil((double)pkt->getTotalSize()/(double)maxPacketSize);
                    if (pkt->getNodeId() == L3Address())
                        throw cRuntimeError("id invalid");
                    ConnectInTransit info(pkt->getNodeId(),pkt->getSegmentId(),numSubSegments,this);

                    parallelConnection.push_back(info);
                    itPar = (parallelConnection.end()-1);
                }
                        // erase the subsegment from transit list
                auto itaux = itPar->segmentInTransit.find(pkt->getSubSegmentId());
                if  (itaux != itPar->segmentInTransit.end())
                {
                    itPar->segmentInTransit.erase(itaux);
                }
                if (itPar->segmentInTransit.empty())
                {

                    mySegmentList.insert(pkt->getSegmentId());
                    informChanges();
                    if (itPar->timer.isScheduled())
                        cancelEvent(&(itPar->timer));
                    parallelConnection.erase(itPar);
                }
                else
                {
                    if (itPar->timer.isScheduled())
                        cancelEvent(&(itPar->timer));
                    scheduleAt(simTime()+par("requestTimer"),&(itPar->timer));
                }
            }
        }
        else
        {
            for (auto itPar = parallelConnection.begin(); itPar != parallelConnection.end();++itPar)
            {
                if (itPar->segmentId == pkt->getSegmentId())
                {
                    if (itPar->timer.isScheduled())
                        cancelEvent(&(itPar->timer));
                    parallelConnection.erase(itPar);
                    break;
                }
            }
            // check that the segment is not in the transit list
        }
        change = true;
        if (numParallelRequest > parallelConnection.size() && totalSegments != mySegmentList.size())
            generateRequestNew();
    }

    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pkt) << endl;
    emit(rcvdPkSignal, pkt);
    numReceived++;
    delete pkt;
    return change;
}

void UDPBasicP2P2C::generateRequestSub()
{
    for (auto itPar = parallelConnection.begin(); itPar != parallelConnection.end();)
    {
        if (itPar->segmentInTransit.empty())
        {
            itPar = parallelConnection.erase(itPar);
            continue;
        }

        if (itPar->timer.isScheduled())
        {
            ++itPar;
            continue;
        }

        uint32_t segmentId = itPar->segmentId;
        if (itPar->segmentInTransit.empty())
            throw cRuntimeError("");

        uint64_t node = itPar->nodeId.toMAC().getInt();

        if (par("changeSugSegment").boolValue())
            node = chooseDestAddr(segmentId);

        if (node)
            itPar->nodeId = L3Address(MACAddress(node));
        else
            node = chooseDestAddr(segmentId);

        auto it = clientList.find(itPar->nodeId.toMAC().getInt());

        if (it == clientList.end())
        {
            InfoClient infoC;
            infoC.numRequest = 1;
            infoC.numRequestRec = 0;
            clientList.insert(std::pair<uint64_t, InfoClient>(itPar->nodeId.toMAC().getInt(), infoC));
            it = clientList.find(itPar->nodeId.toMAC().getInt());
            if (it == clientList.end())
            {
                throw cRuntimeError("client list error");
            }

        }
        std::vector<uint16_t> vectorAux;
        vectorAux.clear();
        int numReq = 0;
        simtime_t now = simTime();
        simtime_t rateRequest = par("rateRequest").doubleValue();

        for (auto itset = itPar->segmentInTransit.begin();itset != itPar->segmentInTransit.end(); ++itset)
        {
            vectorAux.push_back(*itset);
            if (vectorAux.size() == requestPerPacket)
            {
                UDPBasicPacketP2P *pkt = new UDPBasicPacketP2P;
                pkt->setType(REQUEST);
                pkt->setSegmentId(segmentId);
                pkt->setNodeId(myAddress);
                pkt->setSubSegmentRequestArraySize(vectorAux.size());
                for (unsigned int j = 0; j < vectorAux.size(); j++)
                    pkt->setSubSegmentRequest(j, vectorAux[j]);
                pkt->setByteLength(10 + (vectorAux.size() * 2));
                pkt->setDestination(itPar->nodeId);
                actualizePacketMap(pkt);
                if (numReq == 0)
                    sendNow(pkt);
                else
                    sendDelayed(pkt, now);
                now += rateRequest;
                numReq++;
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
            for (unsigned int j = 0; j < vectorAux.size(); j++)
                pkt->setSubSegmentRequest(j, vectorAux[j]);
            pkt->setByteLength(10 + (vectorAux.size() * 2));
            pkt->setDestination(itPar->nodeId);
            actualizePacketMap(pkt);
            sendNow(pkt);
            it->second.numRequest++;
            vectorAux.clear();
        }
        scheduleAt(simTime() + par("requestTimer"), &(itPar->timer));
        ++itPar;
    }
}

// actualize the node actual segment list
void UDPBasicP2P2C::actualizePacketMap(UDPBasicPacketP2P *pkt)
{
    if (useGlobal) // no necessary
        return;

    int size = static_cast<int>(std::ceil(static_cast<double>(totalSegments)/8.0));
    pkt->setMapSegmentsArraySize(size);
    for (auto & elem : mySegmentList)
    {
        // compute the char position
        int pos = (elem-1)/8;
        char val = pkt->getMapSegments(pos);
        int bit = (elem-1)%8;
        char aux = 1<<bit;
        val |= aux;
        pkt->setMapSegments(pos,val);
    }
    // we suppose 64 bits of CRC
    pkt->setByteLength(pkt->getByteLength()+8+4); // map size + seq num // id can extracted from ip
}

void UDPBasicP2P2C::generateRequestNew()
{
    if (totalSegments > mySegmentList.size() + parallelConnection.size())
    {
        uint64_t best = chooseDestAddr();
        UDPBasicPacketP2P *pkt = new UDPBasicPacketP2P;
        pkt->setType(REQUEST);
        pkt->setNodeId(myAddress);
        pkt->setByteLength(10);
        pkt->setSegmentRequest(0);
        pkt->setSegmentId(0);

        auto it = clientList.find(best);
        if (it == clientList.end())
        {
            InfoClient infoC;
            infoC.numRequest = 1;
            infoC.numRequestRec = 0;
            clientList.insert(std::pair<uint64_t,InfoClient>(best,infoC));
        }
        else
            it->second.numRequest++;

        auto it2 = inverseAddress.find(best);
        if (it2 == inverseAddress.end())
            throw cRuntimeError("arp->getInverseAddressResolution address not found %s", MACAddress(best).str().c_str());
        IPv4Address desAdd = it2->second;

        pkt->setDestination(L3Address(MACAddress(best)));
        actualizePacketMap(pkt);
        sendNow(pkt);
        scheduleAt(simTime()+par("requestTimer"),myTimer);
    }
}



void UDPBasicP2P2C::getList(std::vector<uint64_t> &address)
{
    address.clear();
    SegmentList tempMySegmentList(mySegmentList);
    // include segment in transit
    for (auto itPar =  parallelConnection.begin(); itPar != parallelConnection.end();)
    {
        auto it = mySegmentList.find(itPar->segmentId);

        if (it != mySegmentList.end())
        {
            if (itPar->timer.isScheduled())
                cancelEvent(&(itPar->timer));
            itPar = parallelConnection.erase(itPar);
        }
        else
        {
            tempMySegmentList.insert(itPar->segmentId);
            ++itPar;
        }
    }

    if (useGlobal)
    {
        for (auto &elem : segmentMap)
        {
            if (elem.first == myAddress.toMAC().getInt())
                continue;
            SegmentList * nodeSegmentList = elem.second;
            SegmentList result;
            std::set_difference(nodeSegmentList->begin(), nodeSegmentList->end(), tempMySegmentList.begin(),
                    tempMySegmentList.end(), std::inserter(result, result.end()));
            if (!result.empty())
                address.push_back(elem.first);
        }
    }
    else
    {
        for (auto &elem : networkSegmentMap)
        {
            if (elem.first == myAddress.toMAC().getInt())
                continue;
            SegmentList * nodeSegmentList = elem.second;
            SegmentList result;
            std::set_difference(nodeSegmentList->begin(),nodeSegmentList->end(),tempMySegmentList.begin(),tempMySegmentList.end(),std::inserter(result, result.end()));
            if (!result.empty())
                address.push_back(elem.first);
        }
    }
}


void UDPBasicP2P2C::getList(std::vector<uint64_t> &address,uint32_t segmentId)
{
    address.clear();
    if (useGlobal)
    {
        for (auto &elem : segmentMap)
        {
            if (elem.first == myAddress.toMAC().getInt())
                continue;
            SegmentList * nodeSegmentList = elem.second;
            SegmentList result;
            auto it2 = nodeSegmentList->find(segmentId);
            if (it2 != nodeSegmentList->end())
                address.push_back(elem.first);
        }
    }
    else
    {
        for (auto &elem : networkSegmentMap)
        {
            if (elem.first == myAddress.toMAC().getInt())
                continue;
            SegmentList * nodeSegmentList = elem.second;
            SegmentList result;
            auto it2 = nodeSegmentList->find(segmentId);
            if (it2 != nodeSegmentList->end())
                address.push_back(elem.first);
        }
    }
}


void UDPBasicP2P2C::getList(std::vector<uint64_t> &address, std::vector<SegmentList> &segmentData)
{
    address.clear();
    segmentData.clear();
    if (useGlobal)
    {
        for (auto &elem : segmentMap)
        {
            if (elem.first == myAddress.toMAC().getInt())
                continue;
            SegmentList * nodeSegmentList = elem.second;
            SegmentList result;
            std::set_difference(nodeSegmentList->begin(), nodeSegmentList->end(), mySegmentList.begin(),
                    mySegmentList.end(), std::inserter(result, result.end()));
            if (!result.empty())
            {
                address.push_back(elem.first);
                segmentData.push_back(result);
            }
        }
    }
    else
    {
        for (auto &elem : networkSegmentMap)
        {
            if (elem.first == myAddress.toMAC().getInt())
                continue;
            SegmentList * nodeSegmentList = elem.second;
            SegmentList result;
            std::set_difference(nodeSegmentList->begin(),nodeSegmentList->end(),mySegmentList.begin(),mySegmentList.end(),std::inserter(result, result.end()));
            if (!result.empty())
            {
                address.push_back(elem.first);
                segmentData.push_back(result);
            }
        }
    }
}



uint64_t UDPBasicP2P2C::searchBestSegment(const uint64_t & address)
{

    auto it = segmentMap.find(address);
    if (it == segmentMap.end())
        throw cRuntimeError("Node not found in segmentMap");
    SegmentList * nodeSegmentList = it->second;
    SegmentList result;
    std::set_difference(mySegmentList.begin(),mySegmentList.end(),nodeSegmentList->begin(),nodeSegmentList->end(),std::inserter(result, result.end()));
    if (result.empty())
        throw cRuntimeError("request error");
    uint16_t min = 64000;
    uint64_t seg = UINT64_MAX;
    for (auto it2 = result.begin(); it2 != result.end(); ++it2)
    {
        auto it3 =  mySegmentList.find(*it2);
        if (it3 == mySegmentList.end())
        {
            throw cRuntimeError("request error segment not found in my list");
            continue;
        }
        if (request[*it2]<min)
        {
            min = request[*it2];
            seg = *it2;
        }
    }
    if (seg == UINT64_MAX)
        throw cRuntimeError("request error segment not found");

    request[seg]++;
    return seg;
}


void UDPBasicP2P2C::processRequest(cPacket *p)
{
    int services = 0;
    UDPBasicPacketP2P* pkt = check_and_cast<UDPBasicPacketP2P *>(p);
    for (unsigned int i = 0; i < timeQueue.size(); i++)
    {
        if (pkt->getNodeId() == timeQueue[i]->getDestination())
        {
            services++;
            if (timeQueue[i]->getSegmentId() == timeQueue[i]->getSegmentId())
                return; // this segment is now under service
        }
    }

    if (services >= MaxServices)
        return;

    // check if this node is under service
    auto it = clientList.find(pkt->getNodeId().toMAC().getInt());
    if (it == clientList.end())
    {

        InfoClient infoC;
        infoC.numRequestRec = 1;
        infoC.numRequest = 0;
        clientList.insert(std::pair<uint64_t,InfoClient>(pkt->getNodeId().toMAC().getInt(),infoC));
    }
    else
        it->second.numRequestRec++;
    answerRequest(pkt);
}

int UDPBasicP2P2C::getQueueSize()
{

    if (timeQueue.empty())
        return 0;
    // search position
    uint64_t remain = 0;
    for (auto it = timeQueue.begin();it != timeQueue.end(); ++it)
        remain += (*it)->getRemain();
    return ceil((double)remain/(double)maxPacketSize);
}


// actualize list

void UDPBasicP2P2C::actualizeList(UDPBasicPacketP2P *pkt)
{
    if (pkt->getMapSegmentsArraySize() == 0)
        return; // nothing to do

    auto itSegList = networkSegmentMap.find(pkt->getNodeId().toMAC().getInt());
    SegmentList * segList = nullptr;
    if (itSegList != networkSegmentMap.end())
        segList = itSegList->second;
    // actualize list

    if (segList == nullptr)
    {
        // new
        segList = new SegmentList();
        networkSegmentMap.insert(std::make_pair(pkt->getNodeId().toMAC().getInt(),segList));
    }
    for (unsigned int i = 0; i < pkt->getMapSegmentsArraySize(); i++)
    {
        char val = pkt->getMapSegments(i);
        for (int j = 0; j < 8; j++)
        {
            if (val & (1<<j))
            {
                unsigned int segId = (i * 8)+j+1;
                segList->insert(segId);
            }
        }
    }
}


void UDPBasicP2P2C::answerRequest(UDPBasicPacketP2P *pkt)
{

    if (pkt->getSegmentId() != 0) // specific segment check if the segment is present in the node
    {
        auto it =  mySegmentList.find(pkt->getSegmentId());
        if (it == mySegmentList.end())
            return;
    }

    // prepare a delayed
    DelayMessage *delayM = new DelayMessage();
    delayM->setNodeId(myAddress);
    delayM->setDestination(pkt->getNodeId());
    delayM->setType(SEGMEN);
    delayM->setTotalSize(segmentSize);
    delayM->setNextTime(simTime());

    unsigned int total = ceil((double)delayM->getTotalSize()/(double)maxPacketSize);
    delayM->setTotal(total);

    delayM->setLastSend(simTime());
    uint64_t desAdd = pkt->getNodeId().toMAC().getInt();

    if (pkt->getSubSegmentRequestArraySize() == 0 && pkt->getSegmentId() == 0)
    {
        delayM->setIndex(1);
        delayM->setSegmentId(searchBestSegment(desAdd));
        delayM->setRemain(delayM->getTotalSize());
        // insert in queue
    }
    else
    {

        for (int i = (int) pkt->getSubSegmentRequestArraySize() - 1; i>=0  ; i--)
            delayM->subSegmentRequest.push_back(pkt->getSubSegmentRequest(i));

        delayM->setSegmentId(pkt->getSegmentId());
        delayM->setRemain(delayM->subSegmentRequest.size()*maxPacketSize);
    }

    // El primero se envía inmediatamente para evitar que la otra parte se impaciente
    //
    UDPBasicPacketP2P *pktSend = delayM->getPkt(maxPacketSize);
    lastPacket = simTime();
    lastServer = simTime();
    if (delayM->getRemain() > 0)
    {
        delayM->setNextTime(delayM->getNextTime()+par("maxFlowRate").doubleValue());
        //timeQueue.push_back(delayM);

        if (timeQueue.empty() || (!timeQueue.empty() && timeQueue.back()->getNextTime() < simTime()))
            timeQueue.push_back(delayM);
        else
        {
            // search position
            for (auto it = timeQueue.begin();it != timeQueue.end(); ++it)
            {
                if ((*it)->getNextTime() > delayM->getNextTime())
                {
                    timeQueue.insert(it,delayM);
                    break;
                }
            }
        }
    }
    else
    {
        delete delayM;
        delayM = nullptr;
    }
    sendNow(pktSend);

    if (delayM)
    {
        if (timeQueue.empty() || (!timeQueue.empty() && timeQueue.back()->getNextTime() < delayM->getNextTime()))
            timeQueue.push_back(delayM);
        else
        {
            // search position
            for (auto it = timeQueue.begin();it != timeQueue.end(); ++it)
            {
                if ((*it)->getNextTime() > delayM->getNextTime())
                {
                    timeQueue.insert(it,delayM);
                    break;
                }
            }
        }
    }


    if (!queueTimer->isScheduled()) //
    {
        if (!timeQueue.empty())
            scheduleAt(simTime(), queueTimer);
    }
    GlobalWirelessLinkInspector::setQueueSize(myAddress, getQueueSize());
}


void UDPBasicP2P2C::finish()
{
    recordScalar("packets send", numSent);
    recordScalar("packets received", numReceived);
    recordScalar("segment present", numSegPresent);


    recordScalar("Request sent", numRequestSent);
    recordScalar("request received", numRequestServed);
    recordScalar("segment received", numRequestSegmentServed);

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


void UDPBasicP2P2C::WirelessNumNeig()
{
    // TODO Auto-generated constructor stub
    // fill in routing tables with static routes

    if (!vectorList.empty())
        return;

    cTopology topo("topo");
    topo.extractByProperty("networkNode");
    for (int i = 0; i < topo.getNumNodes(); i++)
    {
        cTopology::Node *destNode = topo.getNode(i);
        IMobility *mod;
        IInterfaceTable* itable = L3AddressResolver().findInterfaceTableOf(destNode->getModule());
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
            for(auto it = segmentMap.begin(); it != segmentMap.end();++it)
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

        cModule *host = getContainingNode(this);
        mod = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        if (mod == nullptr)
            throw cRuntimeError("node or mobility module not found");

        vectorList[add] = mod;
    }
}

int UDPBasicP2P2C::getNumNeighNodes(uint64_t add,double dist)
{
    auto it = vectorList.find(add);
    if (it == vectorList.end())
        throw cRuntimeError("Node not found");
    int cont = 0;
    Coord ci = it->second->getCurrentPosition();
    for (auto it2 = vectorList.begin();it2 != vectorList.end();++it2)
    {
        if (it2->first == add)
            continue;
        Coord cj = it2->second->getCurrentPosition();
        if (ci.distance(cj) <= dist)
            cont++;
    }
    return cont;
}

L3Address UDPBasicP2P2C::DelayMessage::getNodeId() const
{
    return nodeId_var;
}

void UDPBasicP2P2C::DelayMessage::setNodeId(L3Address nodeId)
{
    this->nodeId_var = nodeId;
}

L3Address UDPBasicP2P2C::DelayMessage::getDestination() const
{
    return destination_var;
}

void UDPBasicP2P2C::DelayMessage::setDestination(L3Address destination)
{
    this->destination_var = destination;
}

int UDPBasicP2P2C::DelayMessage::getType() const
{
    return type_var;
}

void UDPBasicP2P2C::DelayMessage::setType(int type)
{
    this->type_var = type;
}

uint64_t UDPBasicP2P2C::DelayMessage::getTotalSize() const
{
    return totalSize_var;
}

void UDPBasicP2P2C::DelayMessage::setTotalSize(uint64_t totalSize)
{
    this->totalSize_var = totalSize;
}

uint64_t UDPBasicP2P2C::DelayMessage::getSegmentId() const
{
    return segmentId_var;
}

void UDPBasicP2P2C::DelayMessage::setSegmentId(uint64_t segmentId)
{
    this->segmentId_var = segmentId;
}

uint16_t UDPBasicP2P2C::DelayMessage::getSubSegmentId() const
{
    return subSegmentId_var;
}

void UDPBasicP2P2C::DelayMessage::setSubSegmentId(uint16_t subSegmentId)
{
    this->subSegmentId_var = subSegmentId;
}


simtime_t UDPBasicP2P2C::DelayMessage::getLastSend()
{
    return lastSend_var;
}

void UDPBasicP2P2C::DelayMessage::setLastSend(const simtime_t &val)
{

    lastSend_var = val;
}

unsigned int UDPBasicP2P2C::DelayMessage::getIndex()
{
    return index_var;
}

unsigned int UDPBasicP2P2C::DelayMessage::getTotal()
{
    return total_var;
}

void UDPBasicP2P2C::DelayMessage::setIndex(const unsigned int &val) {index_var = val;}
void UDPBasicP2P2C::DelayMessage::setTotal(const unsigned int &val)  {total_var = val;}


UDPBasicPacketP2P * UDPBasicP2P2C::DelayMessage::getPkt(const uint32_t &pkSize)
{
    UDPBasicPacketP2P *pkt = new UDPBasicPacketP2P();
    pkt->setNodeId(getNodeId());
    pkt->setDestination(getDestination());
    pkt->setType(getType());
    pkt->setTotalSize(getTotalSize());
    pkt->setSegmentId(getSegmentId());

    pkt->setByteLength(pkSize);
    if (getRemain() > pkSize)
        setRemain(getRemain()-pkSize);
    else
        setRemain(0);
    if (!this->subSegmentRequest.empty())
    {
        pkt->setSubSegmentId(this->subSegmentRequest.back());
        this->subSegmentRequest.pop_back();
        if (this->subSegmentRequest.empty() && getRemain() != 0)
            setRemain(0);
    }
    else
    {
        pkt->setSubSegmentId(this->getIndex());
        this->setIndex(this->getIndex()+1);
        if (this->getIndex() >= this->getTotal() && getRemain() != 0)
            setRemain(0);
    }
    return pkt;
}


void UDPBasicP2P2C::DelayMessage::setPkt(UDPBasicPacketP2P *pkt)
{
    setNodeId(pkt->getNodeId());
    setDestination(pkt->getDestination());
    setType(pkt->getType());
    setTotalSize(pkt->getTotalSize());
    setSegmentId(pkt->getSegmentId());
    setSubSegmentId(pkt->getSubSegmentId());

    for (unsigned int i = 0; i < pkt->getSubSegmentRequestArraySize();i++)
        subSegmentRequest.push_back(pkt->getSubSegmentRequest(i));
    delete pkt;
}

UDPBasicPacketP2PNotification *UDPBasicP2P2C::getPacketWitMap()
{
    UDPBasicPacketP2PNotification *pkt = new UDPBasicPacketP2PNotification();

    pkt->setNodeId(myAddress);
    pkt->setSeqnum(mySeqNumber);
    mySeqNumber++;
    pkt->setMapSegmentsSize(totalSegments);
    int size = static_cast<int>(std::ceil(static_cast<double>(totalSegments)/8.0));
    pkt->setMapSegmentsArraySize(size);

    for (auto & elem : mySegmentList)
    {
        // compute the char position
        int pos = (elem-1)/8;
        char val = pkt->getMapSegments(pos);
        int bit = (elem-1)%8;
        char aux = 1<<bit;
        val |= aux;
        pkt->setMapSegments(pos,val);
    }
    pkt->setByteLength(8+4); // map size + seq num // id can extracted from ip
    return pkt;
}

void UDPBasicP2P2C::informChanges()
{
    if (useGlobal)
        return;
    UDPBasicPacketP2PNotification *pkt = getPacketWitMap();
    socket.sendTo(pkt, L3Address(IPv4Address::ALLONES_ADDRESS), destPort);
}

bool UDPBasicP2P2C::processMsgChanges(cPacket *msg)
{
    UDPBasicPacketP2PNotification *pkt = dynamic_cast<UDPBasicPacketP2PNotification *>(msg);
    if (pkt == nullptr)
        return false;
    if (pkt->getNodeId() == myAddress)
    {
        delete msg;
        return true;
    }

    auto it = sequenceList.find(pkt->getNodeId().toMAC().getInt());
    if (it != sequenceList.end())
    {
        if (it->second >= pkt->getSeqnum())
        {
        // old delete and return
            delete msg;
            return true;
        }
        else
        {
            // actualize
            it->second = pkt->getSeqnum();
        }
    }
    else
    {
        // insert
        sequenceList[pkt->getNodeId().toMAC().getInt()] = pkt->getSeqnum();
    }

    // process
    auto itSegList = networkSegmentMap.find(pkt->getNodeId().toMAC().getInt());
    SegmentList * segList = nullptr;
    if (itSegList != networkSegmentMap.end())
        segList = itSegList->second;

    // actualize list

    if (segList == nullptr)
    {
        // new
        segList = new SegmentList();
        networkSegmentMap.insert(std::make_pair(pkt->getNodeId().toMAC().getInt(),segList));
    }

    for (unsigned int i = 0; i < pkt->getMapSegmentsArraySize(); i++)
    {
        char val = pkt->getMapSegments(i);
        for (int j = 0; j < 8; j++)
        {
            if (val & (1<<j))
            {
                unsigned int segId = (i * 8)+j+1;
                segList->insert(segId);
            }
        }
    }

    socket.sendTo(msg, L3Address(IPv4Address::ALLONES_ADDRESS), destPort); // propagate
    return true;
}

}

