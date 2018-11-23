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


#include "UDPBasicP2P2Multi.h"
#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "GlobalWirelessLinkInspector.h"
#include <algorithm>
#include "MobilityAccess.h"
#include "IPv4InterfaceData.h"
#include "RoutingTableAccess.h"
#include <cmath>

Define_Module(UDPBasicP2P2Multi);

simsignal_t UDPBasicP2P2Multi::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPBasicP2P2Multi::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t UDPBasicP2P2Multi::outOfOrderPkSignal = registerSignal("outOfOrderPk");
simsignal_t UDPBasicP2P2Multi::dropPkSignal = registerSignal("dropPk");

simsignal_t UDPBasicP2P2Multi::queueLengthSignal = registerSignal("queueLength");
simsignal_t UDPBasicP2P2Multi::queueLengthInstSignal = registerSignal("queueLengthInst");

UDPBasicP2P2Multi::SegmentMap UDPBasicP2P2Multi::segmentMap;

UDPBasicP2P2Multi::VectorList UDPBasicP2P2Multi::vectorList;

UDPBasicP2P2Multi::InverseAddres UDPBasicP2P2Multi::inverseAddress;
UDPBasicP2P2Multi::DirectAddres UDPBasicP2P2Multi::directAddress;

std::map<int,uint32_t> UDPBasicP2P2Multi::totalSegments;
std::map<int,uint32_t> UDPBasicP2P2Multi::segmentSize;

std::vector<int> UDPBasicP2P2Multi::initNodes;

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("modoP2P");
    if (!e) enums.getInstance()->add(e = new cEnum("modoP2P"));
    e->insert(UDPBasicP2P2Multi::CONCAVO, "concavo");
    e->insert(UDPBasicP2P2Multi::ADITIVO, "aditivo");
    e->insert(UDPBasicP2P2Multi::ADITIVO_PONDERADO, "aditivo_ponderado");
    e->insert(UDPBasicP2P2Multi::MIN_HOP, "minhop");
    e->insert(UDPBasicP2P2Multi::RANDOM, "random");
);

static std::ostream& operator<<(std::ostream& out, const UDPBasicP2P2Multi::SegmentList& e)
{
    out << "Present " << e.size();
    return out;
}

UDPBasicP2P2Multi::UDPBasicP2P2Multi()
{
    myTimer = new cMessage("UDPBasicP2P-timer");
    queueTimer = new cMessage("UDPBasicP2P-queueTimer");
    parallelConnection.clear();
    vectorList.clear();
    numRegData = 0;
    inverseAddress.clear();
    directAddress.clear();
    initNodes.clear();
    initialList.clear();
    pendingRequestTimer = new cMessage("pendingRequestTimer");
}

UDPBasicP2P2Multi::~UDPBasicP2P2Multi()
{
    cancelAndDelete(myTimer);
    cancelAndDelete(queueTimer);
    cancelEvent(&periodicTimer);
    initialList.clear();
    if (periodicMeasureTimer)
        cancelAndDelete(periodicMeasureTimer);
    while(!timeQueue.empty())
    {
        delete timeQueue.back();
        timeQueue.pop_back();
    }
    SegmentMap::iterator it = segmentMap.find(myAddress.getMAC().getInt());
    if (it != segmentMap.end())
        segmentMap.erase(it);
    mySegmentList.clear();
    clientList.clear();
    for (ParallelConnection::iterator it = parallelConnection.begin();it != parallelConnection.end();++it)
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

void UDPBasicP2P2Multi::initialize(int stage)
{

    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.

    if (stage == 0)
    {
        mySegmentList.clear();

        const char *totalSegmentsStr = par("totalSegmentsStr").stringValue();
        const char *totalSegmentsIdStr = par("totalSegmentsIdStr").stringValue();
        const char *segmentSizeStr = par("segmentSizeStr").stringValue();
        cStringTokenizer tokenizer1(totalSegmentsStr);
        cStringTokenizer tokenizer2(totalSegmentsIdStr);
        cStringTokenizer tokenizer3(segmentSizeStr);
        std::vector<int> totS = tokenizer1.asIntVector();
        std::vector<int> totId = tokenizer2.asIntVector();
        std::vector<int> segSize = tokenizer3.asIntVector();
        if (totS.size() < totId.size() || totId.size() > segSize.size())
        {
            unsigned int max = std::min({totS.size(), totId.size(), segSize.size()});
            totS.resize(max);
            totId.resize(max);
            segSize.resize(max);
            EV << "Resize the total number of objects to " << totId.size() <<endl;
        }
        else
            EV << "Total number of objects to " << totId.size() <<endl;

        for (unsigned int i = 0; i < totId.size(); i++)
        {
            totalSegments[totId[i]] = totS[i];
            segmentSize[totId[i]] = segSize[i];
        }
    }
    else if (stage == 5)
    {

        changeDestination = par("changeDestination");

        if (par("useGlobal"))
            useGlobal = true;

        if (!useGlobal)
            informTimeOut = new cMessage("UDPBasicP2P-infortimer");;

        const char *addrModeStr = par("modoP2P").stringValue();

        int p2pMode = cEnum::get("modoP2P")->lookup(addrModeStr);
        if (p2pMode == -1)
            throw cRuntimeError("Invalid modoP2P: '%s'", addrModeStr);
        modo = (modoP2P) p2pMode;

        if (!GlobalWirelessLinkInspector::isActive())
            opp_error("GlobalWirelessLinkInspector not found");

        if (par("fuzzy"))
        {
            fuzzy = new FuzzYControl(par("fuzzyFisFile").stringValue());
            fuzzy->setSizeParam(par("numVarFuzzy").longValue());
            sourceRouting = par("sourceRouting");
        }


        uint64_t initseg = 0;
        IPvXAddress myAddr = IPvXAddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
        myAddressIp4 = myAddr.get4();
        IInterfaceTable * ift = IPvXAddressResolver().interfaceTableOf(this->getParentModule());

        myAddress = ManetAddress::ZERO;
        for (int i = 0; i < ift->getNumInterfaces(); i++)
        {
            InterfaceEntry * e = ift->getInterface(i);
            if (e->ipv4Data())
            {
                if (e->ipv4Data()->getIPAddress() == myAddr.get4())
                {
                    myAddress = ManetAddress(e->getMacAddress());
                    break;
                }
            }
        }
        if (myAddr.get4() == IPv4Address::UNSPECIFIED_ADDRESS)
            opp_error("addr invalid");
        if (myAddress == ManetAddress::ZERO)
            opp_error("MACAddress addr invalid");
        inverseAddress[myAddress.getMAC().getInt()] = myAddr.get4();
        directAddress[myAddr.get4()] = myAddress.getMAC().getInt();

        if (!par("initNodesRand").boolValue())
        {
            if (strcmp(par("numInitialSegmentsStr").stringValue(),"") != 0)
            {
                const char *totalSegmentsStr = par("numInitialSegmentsStr").stringValue();
                const char *totalSegmentsIdStr = par("objectsIdStr").stringValue();

                cStringTokenizer tokenizer1(totalSegmentsStr);
                cStringTokenizer tokenizer2(totalSegmentsIdStr);
                std::vector<int> totS = tokenizer1.asIntVector();
                std::vector<int> totId = tokenizer2.asIntVector();
                if (totS.size() < totId.size())
                {
                    // resize to the smaller
                    unsigned int max = std::min(totS.size(), totId.size());
                    totS.resize(max);
                    totId.resize(max);
                }

                for (unsigned int i = 0 ; i<  totId.size(); i++)
                {
                    auto it = totalSegments.find(totId[i]);
                    if (it == totalSegments.end())
                        throw cRuntimeError("configuration error, Object ID not found %i",totId[i]);
                    // random initialization
                    while ((int)mySegmentList[totId[i]].size() < totS[i])
                    {
                        mySegmentList[totId[i]].insert(intuniform(1, initseg+1));
                    }
                }
            }
            if (!onlyInitialNodes)
                recordVector = true;
            else
            {
                for (const auto it : mySegmentList)
                {
                    if(!it.second.empty())
                        recordVector = true;
                }
            }
        }
        else
        {
            // cModule * myNode = IPvXAddressResolver().findHostWithAddress(myAddr);
            cModule * myNode = this->getParentModule();
            if (!myNode->isVector())
                error("No es un vector");
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
                    for (const auto &elem : totalSegments)
                    {
                        for (unsigned int i = 1; i <= elem.second; i++)
                            mySegmentList[elem.first].insert(i);

                    }
                    break;
                }
            }
            // include desired objects
        }

        if (!onlyInitialNodes)
            recordVector = true;
        else
        {
            for (const auto it : mySegmentList)
            {
                if(!it.second.empty())
                    recordVector = true;
            }
        }

        maxPacketSize = par("maxPacketSize");
        rateLimit = par("packetRate");
        requestPerPacket = par("requestPerPacket");
        lastPacket = 0;
        serverTimer = par("serverRate");

        segmentMap[myAddress.getMAC().getInt()] = &mySegmentList;

        int numObjects = par("numObjects");

        if (par("useZipfLaw").boolValue())
        {
        // Ahora usa la ley zipf para determinar los objetos que debe pedir cada nodo que no tiene nada relleno
            if (mySegmentList.empty())
            {
                std::vector<double> probVec;
                double den = 0;
                for (unsigned int i = 1; i <= totalSegments.size();i++)
                    den += (1.0/(double)i);

                den = 1/den;

                double val = 0;
                for (unsigned int i = 1; i <= totalSegments.size();i++)
                {
                    double aux = den/i;
                    val += aux;
                    probVec.push_back(val);
                }
                const char *totalSegmentsIdStr = par("totalSegmentsIdStr").stringValue();
                cStringTokenizer tokenizer(totalSegmentsIdStr);
                std::vector<int> totId = tokenizer.asIntVector();

                probVec.back() = 1.0; // force 1.0, it is possible due to the rounds that this value could be a bi lower than 1, force 1
                while ((int)mySegmentList.size()<numObjects)
                {
                    val = dblrand();
                    for (unsigned int i = 0; i < probVec.size(); i++)
                    {
                        if (val < probVec[i])
                        {
                            // i element. search if exist
                            auto it = mySegmentList.find(totId[i]);
                            if (it == mySegmentList.end())
                            {
                                // insert
                                mySegmentList[totId[i]].empty();
                            }
                            break;
                        }
                    }
                }
            }
        }
// initialize timers
        for (const auto &elem:mySegmentList)
        {
            auto it = totalSegments.find(elem.first);
            if (it == totalSegments.end())
                throw cRuntimeError("Object Id not found in totalSegments");
            if (elem.second.size() < it->second)
            {
                startReception[elem.first] = par("startTime");
                endReception[elem.first] = SimTime::getMaxTime();
                if (!myTimer->isScheduled())
                    scheduleAt(par("startTime"), myTimer);
            }
            else
            {
                startReception[elem.first] = 0;
                endReception[elem.first] = 0;
            }
        }

        if (par("periodicTimer").doubleValue() > 0)
            scheduleAt(simTime() + par("periodicTimer"), &periodicTimer);

        localPort = par("localPort");
        destPort = par("destPort");

        socket.setOutputGate(gate("udpOut"));
        socket.bind(localPort);
        numObjectPresent = (int) mySegmentList.size();

        numParallelRequest = par("numParallelRequest");

        bool inform = false;
        for (auto &elem : mySegmentList)
        {
            if (elem.second.size()>0)
            {
                inform = true;
                break;
            }
        }

        if (inform && !useGlobal)
        {
            scheduleAt(simTime()+uniform(0,0.01),informTimeOut);
        }

        clientList.clear();
        timeQueue.clear();
        WATCH_MAP(mySegmentList);

        // WATCH((unsigned int)timeQueue.size());

        const char *listNodes = par("listNodes");
        cStringTokenizer tokenizer(listNodes);
        const char *token;

        while ((token = tokenizer.nextToken()) != NULL)
        {
            IPvXAddress addr = IPvXAddressResolver().resolve(token);
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

        if (par("periodicMeasure").boolValue())
        {
            periodicMeasureTimer = new cMessage("periodicMeasureTimer");
            scheduleAt(simTime()+par("periodicMeasureTimer").doubleValue(),periodicMeasureTimer);
        }

    }
    else if (stage == 6)
    {
        /*
        if (par("SelectServerListInInit"))
        {
            for (const auto &elem : segmentMap)
            {
                if (elem.firstfirst != myAddress.getMAC().getInt())
                {
                    // check if the node has the
                    initialList.push_back(it->first);
                }
            }
        }
        */
        GlobalWirelessLinkInspector::initRoutingTables(this, myAddress, true);
        routing = new WirelessNumHops();
        if (sourceRouting)
            routing->activateKshortest();
        routing->setRoot(myAddress.getMAC());
        routing->fillRoutingTables(par("coverageArea").doubleValue());
        routing->run();
        routing->setIpRoutingTable();
        for (unsigned int i = 0; i < routing->getNumRoutes(); i++)
        {
            std::deque<MACAddress> pathNode;
            routing->getRoute(i, pathNode);
            if (pathNode.empty())
                continue;
            GlobalWirelessLinkInspector::setRoute(this, myAddress, ManetAddress(pathNode.back()), ManetAddress(pathNode[0]), false);

        }
        if (!sourceRouting)
        {
            delete routing;
            routing = NULL;
        }
        if (recordVector)
        {
            periodDataRequestPackets.setName("RequestPkInterval");
            periodDataSendPackets.setName("SendPkInterval");
            periodDataSendBytes.setName("BytesSendInterval");
            periodDataRequestPackets.enable();
            periodDataSendPackets.enable();
            periodDataSendBytes.enable();
        }
    }
}


uint64_t UDPBasicP2P2Multi::chooseDestAddr(PathAddress &route)
{
    std::vector<uint64_t> address;
    getList(address);

    if (changeDestination)
        return selectBest(address, route);

    bool inList = false;
    if (destination != 0)
    {
        for (int  i = 0 ; i < (int)address.size(); i++)
            if (address[i] == destination)
                inList = true;
    }
    if (destination == inList)
    {
        address.clear();
        address.push_back(destination);
    }
    destination = selectBest(address, route);
    return destination;
}


uint64_t UDPBasicP2P2Multi::chooseDestAddr(int obj, uint32_t segmentId, PathAddress &route)
{
    std::vector<uint64_t> address;
    getList(address, obj,segmentId);

    if (changeDestination)
        return selectBest(address, route);

    bool inList = false;
    if (destination != 0)
    {
        for (int  i = 0 ; i < (int)address.size(); i++)
            if (address[i] == destination)
                inList = true;
    }
    if (destination == inList)
    {
        address.clear();
        address.push_back(destination);
    }
    destination = selectBest(address, route);
    return destination;
}


bool UDPBasicP2P2Multi::areDiff(const SegmentList &List1, const SegmentList &List2)
{
    SegmentList result;
    std::set_difference(List1.begin(),List1.end(),List2.begin(),List2.end(),std::inserter(result, result.end()));
    if (!result.empty())
        return true;
    return false;
}


uint64_t UDPBasicP2P2Multi::selectBest(const ListAddress &address, PathAddress &routeSelected)
{
    double costMax = 1e300;
    double maxfuzzyCost = 0;
    uint64_t addr = 0;
    ManetAddress myAdd = myAddress;
    std::vector<uint64_t> winners;
    routeSelected.clear();
    std::vector<PathAddress> winnersPath;

    routeSelected.clear();
    if (address.empty())
    {
        if (useGlobal)
            throw cRuntimeError("No available address");
        else
            return 0;
    }


    if (modo == UDPBasicP2P2Multi::RANDOM)
    {
        unsigned int pos = intuniform(0,address.size()-1);
        return address[pos];
    }

    for (unsigned int i = 0 ;i < address.size(); i++)
    {
        PathAddress route;
        std::vector<PathAddress> kRoutes;
        ManetAddress aux = ManetAddress(MACAddress(address[i]));
        bool validRoute = false;
        GlobalWirelessLinkInspector::Link cost;
        int neigh = 0;
        if (routing)
        {
            if (sourceRouting && fuzzy)
            {
                KroutesMac routes;
                routing->getKshortest(aux.getMAC(),routes);
                for (unsigned int i = 0 ; i < routes.size(); i++)
                {
                    if (routes[i].empty())
                        continue;
                    PathAddress routeAux;
                    routeAux.push_back(myAddress);
                    for (unsigned int j = 0; j < routes[i].size();j++)
                    {
                        routeAux.push_back(ManetAddress(routes[i][j]));
                    }
                    kRoutes.push_back(routeAux);
                }
                if (!kRoutes.empty())
                    validRoute = true;
            }
            else
            {
                std::deque<MACAddress> pathNode;
                //if (routing->findRouteWithCost(par("coverageArea").doubleValue(), aux.getMAC(), pathNode,true,costAdd,costMax))
                if (routing->findRoute(par("coverageArea").doubleValue(), MACAddress(address[i]), pathNode))
                {
                    InverseAddres::iterator it = inverseAddress.find(pathNode[0].getInt());
                    if (it == inverseAddress.end())
                        throw cRuntimeError(" address not found %s", pathNode[0].str().c_str());
                    IPv4Address desAdd = it->second;
                    validRoute = true;
                    route.push_back(myAdd);
                    for (unsigned int i = 0; i < pathNode.size(); i++)
                        route.push_back(ManetAddress(pathNode[i]));
                }
                else
                    throw cRuntimeError("route not found");
            }
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
                if (kRoutes.empty()) // in this case the valid route is in route vector
                    kRoutes.push_back(route);
                for (unsigned int k = 0 ; k < kRoutes.size(); k++)
                {
                    route = kRoutes[k];
                    GlobalWirelessLinkInspector::getNumNodes(aux, neigh);
                    if (modo == CONCAVO)
                        GlobalWirelessLinkInspector::getWorst(route, cost);
                    else
                        GlobalWirelessLinkInspector::getCostPath(route, cost);
                    if (modo == ADITIVO_PONDERADO)
                        cost.costEtx /= (route.size() - 1);

                    std::vector<double> inputData;
                    inputData.push_back(route.size() - 1);
                    inputData.push_back(cost.costEtx);
                    if (nodesN > neigh)
                        inputData.push_back(nodesN);
                    else
                        inputData.push_back(neigh);

                    double fuzzyCost;
                    fuzzy->runFuzzy(inputData, fuzzyCost);
                    if (writeData)
                    {
                        *outfile << numRegData << " ";
                        for (unsigned int i = 0; i < inputData.size(); i++)
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
                            winnersPath.clear();
                            winners.push_back(addr);
                            winnersPath.push_back(route);
                        }
                        else if (fuzzyCost > maxfuzzyCost && route.size() == costMax)
                        {
                            costMax = route.size();
                            addr = address[i];
                            maxfuzzyCost = fuzzyCost;
                            winners.clear();
                            winnersPath.clear();
                            winners.push_back(addr);
                            winnersPath.push_back(route);
                        }
                        else if (fuzzyCost == maxfuzzyCost && route.size() == costMax)
                        {
                            addr = address[i];
                            winners.push_back(addr);
                            winnersPath.push_back(route);
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
                            winnersPath.clear();
                            winners.push_back(addr);
                            winnersPath.push_back(route);
                        }
                        else if (fuzzyCost == maxfuzzyCost)
                        {
                            addr = address[i];
                            winners.push_back(addr);
                            winnersPath.push_back(route);
                        }
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
                    winnersPath.clear();
                    winners.push_back(addr);
                    winnersPath.push_back(route);
                }
                else if (costPath == costMax)
                {
                    addr = address[i];
                    winners.push_back(addr);
                    winnersPath.push_back(route);
                }
            }
        }
    }
    if (winners.empty())
        error("winners empty");
    else
    {
        int val = intuniform(0,winners.size()-1);
        addr = winners[val];
        routeSelected = winnersPath[val];
    }
    return addr;
}

void UDPBasicP2P2Multi::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (periodicMeasureTimer == msg)
        {
            scheduleAt(simTime()+par("periodicMeasureTimer").doubleValue(),periodicMeasureTimer);
            if (recordVector)
            {
                periodDataRequestPackets.record(totalRequestInPeriod);
                periodDataSendPackets.record(totalSendInPeriod);
                periodDataSendBytes.record(totalBytesInPeriod);
            }
            totalRequestInPeriod = 0;
            totalSendInPeriod = 0;
            totalBytesInPeriod = 0;
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

        if (informTimeOut == msg)
            informChanges();

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
        if (pkt->getDestination() != myAddress && pkt->getSourceRouteArraySize() > 0) // source routing.
        {
            int position = -1;
            for (int i = 0; i < (int)pkt->getSourceRouteArraySize(); i++)
            {
                // find position in the array
                if (pkt->getSourceRoute(i) == myAddress)
                {
                    position = i;
                    break;
                }
            }
            if (position == -1)
                throw cRuntimeError("Invalid route");
            MACAddress nextAddress;
            if (position == (int)pkt->getSourceRouteArraySize()-1)
                nextAddress = pkt->getDestination().getMAC();// send to destination
            else
                nextAddress = pkt->getSourceRoute(position+1).getMAC();
            InverseAddres::iterator it = inverseAddress.find(nextAddress.getInt());
            if (it == inverseAddress.end())
                throw cRuntimeError(" address not found %s", nextAddress.str().c_str());

            if (pkt->getControlInfo())
                delete pkt->removeControlInfo();
            IPv4Address desAdd = it->second;
            socket.sendTo(pkt, desAdd, destPort);
            return;
        }

        // I am not sure if the actualization of the list must be here
        actualizeList(pkt);
        if (pkt->getType() == GENERAL)
        {
            if (routing && !sourceRouting)
            {
                if (pkt->getDestination() != myAddress)
                {
                    std::deque<MACAddress> pathNode;

                    routing->findRoute(par("coverageArea").doubleValue(), pkt->getDestination().getMAC(),pathNode);
                    InverseAddres::iterator it = inverseAddress.find(pathNode[0].getInt());
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

        int objId = -1;
        if (processPacket(pkt, objId))
        {
            // check if complete
            if (objId < 0)
                throw cRuntimeError("object id not not identify %i",objId);
            auto itAux = mySegmentList.find(objId);
            if (itAux != mySegmentList.end())
            {
                auto itSize = totalSegments.find(itAux->first);
                if (itSize == totalSegments.end())
                    throw cRuntimeError("object id not found in the list size");
                if (itAux->second.size() == itSize->second)
                {
                    endReception[itAux->first] = simTime();
                }
                else if (itAux->second.size() > itSize->second)
                    throw cRuntimeError("object id size erroneous");
            }
            bool endSim = true;
            for (auto it = segmentMap.begin(); it != segmentMap.end() && endSim == true; ++it)
            {
                for (const auto &elem : *(it->second))
                {
                    auto itSize = totalSegments.find(elem.first);
                    if (itSize == totalSegments.end())
                        throw cRuntimeError("object id not found in the list size");
                    if (elem.second.size() < itSize->second)
                    {
                        endSim = false;
                        break;
                    }
                    else if (elem.second.size() > itSize->second)
                        throw cRuntimeError("object id size erroneous");
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
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (!timeQueue.empty() && !queueTimer->isScheduled())
    {
        opp_error("Queue not empty and timer not scheduled");
    }

    bool complete = true;
    for (const auto &elem : mySegmentList)
    {
        auto itSize = totalSegments.find(elem.first);
        if (itSize == totalSegments.end())
            throw cRuntimeError("object id not found in the list size");
        if (elem.second.size() < itSize->second)
        {
            complete = false;
            break;
        }
        else if (elem.second.size() != itSize->second)
            throw cRuntimeError("object id size erroneous");
    }

    if (!complete && !myTimer->isScheduled() && (parallelConnection.size() < numParallelRequest))
        scheduleAt(simTime()+par("requestTimer"), myTimer);
    else
    {
        for (unsigned int i = 0; i < parallelConnection.size(); i++)
        {
            if (!parallelConnection[i].timer.isScheduled())
                scheduleAt(simTime()+par("requestTimer"), &parallelConnection[i].timer);
        }
    }
    numObjectPresent = (int)mySegmentList.size();
}

void UDPBasicP2P2Multi::sendNow(UDPBasicPacketP2P *pkt)
{
    numSent++;

    if (pkt->getSourceRouteArraySize() > 0)
    {
        int position = -1;
        for (int i = 0; i < (int)pkt->getSourceRouteArraySize(); i++)
        {
            // find position in the array
            if (pkt->getSourceRoute(i) == myAddress)
            {
                position = i;
                break;
            }
        }
        MACAddress nextAddress;
        if (pkt->getNodeId() != myAddress && position == -1)
            throw cRuntimeError("Invalid route");
        if (pkt->getNodeId() == myAddress)
            nextAddress = pkt->getSourceRoute(0).getMAC();
        else if (position == (int)pkt->getSourceRouteArraySize()-1)
            nextAddress = pkt->getDestination().getMAC();// send to destination
        else
            nextAddress = pkt->getSourceRoute(position+1).getMAC();
        InverseAddres::iterator it = inverseAddress.find(nextAddress.getInt());
        if (it == inverseAddress.end())
            throw cRuntimeError(" address not found %s", nextAddress.str().c_str());
        if (pkt->getControlInfo())
            delete pkt->removeControlInfo();
        IPv4Address desAdd = it->second;

        totalBytesInPeriod += pkt->getByteLength();
        totalSendInPeriod++;

        socket.sendTo(pkt, desAdd, destPort);
        return;
    }

    InverseAddres::iterator it = inverseAddress.find(pkt->getDestination().getMAC().getInt());
    if (it == inverseAddress.end())
        throw cRuntimeError(" address not found %s", pkt->getDestination().getMAC().str().c_str());
    IPv4Address desAdd = it->second;
    if (routing && !sourceRouting)
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
    totalBytesInPeriod += pkt->getByteLength();
    totalSendInPeriod++;
    socket.sendTo(pkt, desAdd, destPort);
}

void UDPBasicP2P2Multi::sendDelayed(UDPBasicPacketP2P *pkt,const simtime_t &delay)
{
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

void UDPBasicP2P2Multi::purgePendingRequest(int objectid, uint64_t segment)
{
    for (PendingRequest::iterator it = pendingRequest.begin(); it != pendingRequest.end();)
    {
        if(it->second->getSegmentId() == segment && it->second->getObjectId() == objectid)
        {
            delete it->second;
            pendingRequest.erase(it++);
        }
        else
            ++it;
    }
}

void UDPBasicP2P2Multi::sendQueue()
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
    InverseAddres::iterator it = inverseAddress.find(delayM->getDestination().getMAC().getInt());
    if (it == inverseAddress.end())
        throw cRuntimeError(" address not found %s", delayM->getDestination().getMAC().str().c_str());
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
            for (TimeQueue::iterator it = timeQueue.begin();it != timeQueue.end(); ++it)
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
    emit(queueLengthInstSignal,getQueueSize());
    GlobalWirelessLinkInspector::setQueueSize(myAddress, getQueueSize());
}

bool UDPBasicP2P2Multi::processMyTimer(cMessage *msg)
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

bool UDPBasicP2P2Multi::processPacket(UDPBasicPacketP2P *pkt, int &objId)
{

    if (routing && !sourceRouting)
    {
        if (pkt->getDestination() != myAddress)
        {
            std::deque<MACAddress> pathNode;

            routing->findRoute(par("coverageArea").doubleValue(), pkt->getDestination().getMAC(),pathNode);
            InverseAddres::iterator it = inverseAddress.find(pathNode[0].getInt());
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
        totalRequestInPeriod++;
        processRequest(pkt);
    }
    else if (pkt->getType() == SEGMEN)
    {
        if (myTimer->isScheduled())
            cancelEvent(myTimer);
        numRequestSegmentServed++;
        uint64_t segment = pkt->getSegmentId();
        objId = pkt->getObjectId();

        if (objId < 0)
            throw cRuntimeError("Bad object id : %i",objId);

        auto itObject = mySegmentList.find(objId);
        if (itObject == mySegmentList.end())
            throw cRuntimeError("Object Id not found");

        auto it = itObject->second.find(segment);

        // in transit, purge
        purgePendingRequest(objId,segment);
        if (it == itObject->second.end())
        {
            // check sub segments
            if (pkt->getSubSegmentId() == 0)
            {
                itObject->second.insert(pkt->getSegmentId());
                informChanges();
            }
            else
            {
                ParallelConnection::iterator itPar;
                for (itPar = parallelConnection.begin(); itPar != parallelConnection.end();++itPar)
                {
                    if (itPar->transitSegmentId == segment && itPar->transitObjectId == objId)
                        break;
                }

                if (itPar == parallelConnection.end())
                {
                    uint64_t numSubSegments = ceil((double)pkt->getTotalSize()/(double)maxPacketSize);
                    if (pkt->getNodeId() == ManetAddress::ZERO)
                        opp_error("id invalid");
                    ConnectInTransit info(pkt->getNodeId(),objId, segment, numSubSegments,this);

                    parallelConnection.push_back(info);
                    itPar = (parallelConnection.end()-1);
                }
                        // erase the subsegment from transit list
                std::set<uint16_t>::iterator itaux = itPar->segmentInTransit.find(pkt->getSubSegmentId());
                if  (itaux != itPar->segmentInTransit.end())
                {
                    itPar->segmentInTransit.erase(itaux);
                }
                if (itPar->segmentInTransit.empty())
                {

                    itObject->second.insert(segment);
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
            for (ParallelConnection::iterator itPar = parallelConnection.begin(); itPar != parallelConnection.end();++itPar)
            {
                if (itPar->transitSegmentId == pkt->getSegmentId() && itPar->transitObjectId == pkt->getObjectId())
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
        if (numParallelRequest > parallelConnection.size())
            generateRequestNew(); // the method check there is a pending objects/segments
    }

    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pkt) << endl;
    emit(rcvdPkSignal, pkt);
    numReceived++;
    delete pkt;
    return change;
}



// actualize the node actual segment list
void UDPBasicP2P2Multi::actualizePacketMap(UDPBasicPacketP2P *pkt)
{
    if (useGlobal) // no necessary
        return;

    int obj = pkt->getObjectId();

    auto itSize = totalSegments.find(obj);

    int size = static_cast<int>(std::ceil(static_cast<double>(itSize->second)/8.0));
    pkt->setMapSegmentsArraySize(size);
    auto objectIt = mySegmentList.find(obj);
    if (objectIt == mySegmentList.end())
        throw cRuntimeError("Object Id not found in the list");

    for (SegmentList::iterator it = objectIt->second.begin();it != objectIt->second.end(); ++it)
    {
        // compute the char position
        int pos = ((*it)-1)/8;
        char val = pkt->getMapSegments(pos);
        int bit = ((*it)-1)%8;
        char aux = 1<<bit;
        val |= aux;
        pkt->setMapSegments(pos,val);
    }
    // we suppose 64 bits of CRC
    pkt->setByteLength(pkt->getByteLength()+8+4); // map size + seq num // id can extracted from ip
}

void UDPBasicP2P2Multi::generateRequestSub()
{
    for (ParallelConnection::iterator itPar = parallelConnection.begin(); itPar != parallelConnection.end();)
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

        uint32_t segmentId = itPar->transitSegmentId;
        int objectId = itPar->transitObjectId;
        if (itPar->segmentInTransit.empty())
            throw cRuntimeError("");

        uint64_t node = itPar->nodeId.getMAC().getInt();
        PathAddress route;

        if (par("changeSugSegment").boolValue())
            node = chooseDestAddr(itPar->transitObjectId, segmentId, route);
        else
        {
            if (sourceRouting && routing)
            {
                // search
                ListAddress address;
                address.push_back(node);
                selectBest(address, route);
            }
        }

        if (node)
            itPar->nodeId = ManetAddress(MACAddress(node));
        else
            node = chooseDestAddr(itPar->transitObjectId, segmentId, route);


        ClientList::iterator it = clientList.find(itPar->nodeId.getMAC().getInt());

        if (it == clientList.end())
        {
            InfoClient infoC;
            infoC.numRequest = 1;
            infoC.numRequestRec = 0;
            clientList.insert(std::pair<uint64_t, InfoClient>(itPar->nodeId.getMAC().getInt(), infoC));
            it = clientList.find(itPar->nodeId.getMAC().getInt());
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

        for (std::set<uint16_t>::iterator itset = itPar->segmentInTransit.begin();itset != itPar->segmentInTransit.end(); ++itset)
        {
            vectorAux.push_back(*itset);
            if (vectorAux.size() == requestPerPacket)
            {
                UDPBasicPacketP2P *pkt = new UDPBasicPacketP2P;
                pkt->setType(REQUEST);
                pkt->setSegmentId(segmentId);
                pkt->setObjectId(objectId);
                pkt->setNodeId(myAddress);
                pkt->setSubSegmentRequestArraySize(vectorAux.size());
                for (unsigned int j = 0; j < vectorAux.size(); j++)
                    pkt->setSubSegmentRequest(j, vectorAux[j]);
                pkt->setByteLength(10 + (vectorAux.size() * 2));
                pkt->setDestination(itPar->nodeId);
                actualizePacketMap(pkt);
                if (sourceRouting && !route.empty())
                {
                    int size = route.size() - 2; // camino - origen y destino (ruta contiene todo incluidos origen y destino
                    if (size < 0)
                        throw cRuntimeError("ruta para source routing no tiene direcciones validas");

                    if (size > 0)
                    {
                        pkt->setSourceRouteArraySize(size);
                        for (int i = 0; i < size; i++)
                        {
                            pkt->setSourceRoute(i,route[i+1]);
                        }
                    }
                }
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
            pkt->setObjectId(objectId);
            pkt->setNodeId(myAddress);
            pkt->setSubSegmentRequestArraySize(vectorAux.size());
            for (unsigned int j = 0; j < vectorAux.size(); j++)
                pkt->setSubSegmentRequest(j, vectorAux[j]);
            pkt->setByteLength(10 + (vectorAux.size() * 2));
            pkt->setDestination(itPar->nodeId);
            actualizePacketMap(pkt);
            if (sourceRouting && !route.empty())
            {
                int size = route.size() - 2;
                if (size < 0)
                    throw cRuntimeError("ruta para source routing no tiene direcciones validas");

                if (size > 0)
                {
                    pkt->setSourceRouteArraySize(size);
                    for (int i = 0; i < size; i++)
                    {
                        pkt->setSourceRoute(i,route[i+1]);
                    }
                }
            }
            sendNow(pkt);
            it->second.numRequest++;
            vectorAux.clear();
        }
        scheduleAt(simTime() + par("requestTimer"), &(itPar->timer));
        ++itPar;
    }
}

void UDPBasicP2P2Multi::generateRequestNew()
{
    // check if complete
    bool notComplete = false;
    std::vector<int> objDesired;
    for (const auto &elem:mySegmentList)
    {
        int objid = elem.first;
        auto itSize = totalSegments.find(objid);

        int inTransit = 0;
        for (const auto &elemAux : parallelConnection)
        {
            if (elemAux.transitObjectId == objid)
                inTransit++;
        }
        if (itSize->second > elem.second.size()+inTransit)
        {
            notComplete = true;
            objDesired.push_back(elem.first);
        }
    }

    if (notComplete)
    {
        PathAddress route;
        uint64_t best = chooseDestAddr(route);
        UDPBasicPacketP2P *pkt = new UDPBasicPacketP2P;
        pkt->setType(REQUEST);
        pkt->setNodeId(myAddress);
        pkt->setByteLength(10);
        pkt->setSegmentRequest(0);
        pkt->setSegmentId(0);


        pkt->setObjectsDesiredArraySize(objDesired.size());
        for (unsigned int i = 0; i < objDesired.size(); i++)
            pkt->setObjectsDesired(i,objDesired[i]);

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

        InverseAddres::iterator it2 = inverseAddress.find(best);
        if (it2 == inverseAddress.end())
            throw cRuntimeError("arp->getInverseAddressResolution address not found %s", MACAddress(best).str().c_str());
        IPv4Address desAdd = it2->second;

        pkt->setDestination(ManetAddress(MACAddress(best)));
        actualizePacketMap(pkt);
        if (sourceRouting && !route.empty())
        {
            int size = route.size() - 2;
            if (size < 0)
                throw cRuntimeError("ruta para source routing no tiene direcciones validas");

            if (size > 0)
            {
                pkt->setSourceRouteArraySize(size);
                for (int i = 0; i < size; i++)
                {
                    pkt->setSourceRoute(i,route[i+1]);
                }
            }
        }
        sendNow(pkt);
        scheduleAt(simTime()+par("requestTimer"),myTimer);
    }
}


void UDPBasicP2P2Multi::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
}

void UDPBasicP2P2Multi::getList(ListAddress &address)
{
    address.clear();
    if (!initialList.empty())
    {
        address = initialList;
        return;
    }
    for (const auto &elem : mySegmentList)
    {
        SegmentList tempMySegmentList(elem.second);
        // include segment in transit
        for (ParallelConnection::iterator itPar =  parallelConnection.begin(); itPar != parallelConnection.end();)
        {

            auto itAux = mySegmentList.find(itPar->transitObjectId);
            if (itAux == mySegmentList.end())
                throw cRuntimeError("Object id : %i in pending list is not in the node",itPar->transitObjectId);

            if (itPar->transitObjectId != elem.first)
            {
                ++itPar;
                continue;
            }
            SegmentList::iterator it = tempMySegmentList.find(itPar->transitSegmentId);
            if (it != tempMySegmentList.end())
            {
                if (itPar->timer.isScheduled())
                    cancelEvent(&(itPar->timer));
                itPar = parallelConnection.erase(itPar);
            }
            else
            {
                tempMySegmentList.insert(itPar->transitSegmentId);
                ++itPar;
            }
        }
        if (useGlobal)
        {
            for (SegmentMap::iterator it = segmentMap.begin();it != segmentMap.end();++it)
            {
                if (it->first == myAddress.getMAC().getInt())
                    continue;
                for (auto &elemAux : *(it->second))
                {
                    if (elem.first != elemAux.first)
                        continue;
                    SegmentList * nodeSegmentList = &(elemAux.second);
                    SegmentList result;
                    std::set_difference(nodeSegmentList->begin(), nodeSegmentList->end(), tempMySegmentList.begin(),
                            tempMySegmentList.end(), std::inserter(result, result.end()));
                    if (!result.empty())
                        address.push_back(it->first);
                }
            }
        }
        else
        {
            for (SegmentMap::iterator it = networkSegmentMap.begin();it != networkSegmentMap.end();++it)
            {
                if (it->first == myAddress.getMAC().getInt())
                    continue;

                for (auto &elemAux : *(it->second))
                {
                    if (elem.first != elemAux.first)
                        continue;
                    SegmentList * nodeSegmentList = &(elemAux.second);
                    SegmentList result;
                    std::set_difference(nodeSegmentList->begin(),nodeSegmentList->end(),tempMySegmentList.begin(),tempMySegmentList.end(),std::inserter(result, result.end()));
                    if (!result.empty())
                        address.push_back(it->first);
                }
            }
        }
    }
}


void UDPBasicP2P2Multi::getList(ListAddress &address,int objectId, uint32_t segmentId)
{
    address.clear();
    auto itAux = mySegmentList.find(objectId);
    if (itAux == mySegmentList.end())
        throw cRuntimeError("Requesting object that is not in the request list");

    if (useGlobal)
    {
        for (SegmentMap::iterator it = segmentMap.begin();it != segmentMap.end();++it)
        {
            if (it->first == myAddress.getMAC().getInt())
                continue;
            auto itSegment = it->second->find(objectId);
            if (itSegment == it->second->end())
                continue;
            SegmentList * nodeSegmentList = &(itSegment->second);
            SegmentList result;
            SegmentList::iterator it2 = nodeSegmentList->find(segmentId);
            if (it2 != nodeSegmentList->end())
                address.push_back(it->first);
        }
    }
    else
    {
        for (SegmentMap::iterator it = networkSegmentMap.begin();it != networkSegmentMap.end();++it)
        {
            if (it->first == myAddress.getMAC().getInt())
                continue;
            auto itSegment = it->second->find(objectId);
            if (itSegment == it->second->end())
                continue;
            SegmentList * nodeSegmentList = &(itSegment->second);
            SegmentList result;
            SegmentList::iterator it2 = nodeSegmentList->find(segmentId);
            if (it2 != nodeSegmentList->end())
                address.push_back(it->first);
        }
    }
}


void UDPBasicP2P2Multi::getList(ListAddress  &address,int objectId ,std::vector<SegmentList> &segmentData)
{
    address.clear();
    segmentData.clear();

    auto itAux = mySegmentList.find(objectId);
    if (itAux == mySegmentList.end())
        throw cRuntimeError("Requesting object that is not in the request list");
    SegmentList * myList = &(itAux->second);

    if (useGlobal)
    {
        for (SegmentMap::iterator it = segmentMap.begin();it != segmentMap.end();++it)
        {
            if (it->first == myAddress.getMAC().getInt())
                continue;
            auto itSegment = it->second->find(objectId);
            if (itSegment == it->second->end())
                continue;

            SegmentList * nodeSegmentList = &(itSegment->second);
            SegmentList result;
            std::set_difference(nodeSegmentList->begin(), nodeSegmentList->end(), myList->begin(),
                    myList->end(), std::inserter(result, result.end()));
            if (!result.empty())
            {
                address.push_back(it->first);
                segmentData.push_back(result);
            }
        }
    }
    else
    {
        for (SegmentMap::iterator it = networkSegmentMap.begin();it != networkSegmentMap.end();++it)
        {
            if (it->first == myAddress.getMAC().getInt())
                continue;
            auto itSegment = it->second->find(objectId);
            if (itSegment == it->second->end())
                continue;
            SegmentList * nodeSegmentList = &(itSegment->second);
            SegmentList result;
            std::set_difference(nodeSegmentList->begin(),nodeSegmentList->end(),myList->begin(),myList->end(),std::inserter(result, result.end()));
            if (!result.empty())
            {
                address.push_back(it->first);
                segmentData.push_back(result);
            }
        }
    }
}



uint64_t UDPBasicP2P2Multi::searchBestSegment(const int &object, const uint64_t & address)
{

    SegmentList * nodeSegmentList = nullptr;
    SegmentList * myList = nullptr;

    SegmentMap::iterator it = segmentMap.find(address);
    if (it == segmentMap.end())
        opp_error("Node not found in segmentMap");
    auto objectSegmentMapIt = it->second->find(object);

    if (objectSegmentMapIt !=  it->second->end())
        nodeSegmentList = &objectSegmentMapIt->second;

    objectSegmentMapIt = mySegmentList.find(object);
    if (objectSegmentMapIt !=  mySegmentList.end())
        myList = &objectSegmentMapIt->second;
    else
        throw cRuntimeError("Object requested and not found");

    SegmentList result;
    if (nodeSegmentList == nullptr)
        result = *myList;
    else
        std::set_difference(myList->begin(),myList->end(),nodeSegmentList->begin(),nodeSegmentList->end(),std::inserter(result, result.end()));
    if (result.empty())
        throw cRuntimeError("request error");
    uint16_t min = 64000;
    uint64_t seg = UINT64_MAX;

    Requested * rqPtr = nullptr;
    auto itAux = request.find(object);
    if (itAux != request.end())
    {
        rqPtr = & (itAux->second);
    }
    else
    {
        Requested rq;
        auto itSize = segmentSize.find(object);
        rq.resize(itSize->second, 0);
        request[object] = rq;
        itAux = request.find(object);
        rqPtr = & (itAux->second);
    }

    for (SegmentList::iterator it2 = result.begin(); it2 != result.end(); ++it2)
    {
        SegmentList::iterator it3 =  myList->find(*it2);
        if (it3 == myList->end())
        {
            throw cRuntimeError("request error segment not found in my list");
            continue;
        }
        if ((*rqPtr)[*it2]<min)
        {
            min = (*rqPtr)[*it2];
            seg = *it2;
        }
    }
    if (seg == UINT64_MAX)
        throw cRuntimeError("request error segment not found");

    (*rqPtr)[seg]++;
    return seg;
}


void UDPBasicP2P2Multi::processRequest(cPacket *p)
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
    ClientList::iterator it = clientList.find(pkt->getNodeId().getMAC().getInt());
    if (it == clientList.end())
    {

        InfoClient infoC;
        infoC.numRequestRec = 1;
        infoC.numRequest = 0;
        clientList.insert(std::pair<uint64_t,InfoClient>(pkt->getNodeId().getMAC().getInt(),infoC));
    }
    else
        it->second.numRequestRec++;
    answerRequest(pkt);
}

long unsigned UDPBasicP2P2Multi::getQueueSize()
{

    if (timeQueue.empty())
        return 0;
    // search position
    uint64_t remain = 0;
    for (TimeQueue::iterator it = timeQueue.begin();it != timeQueue.end(); ++it)
    {
        remain += ceil((double)(*it)->getRemain()/(double)maxPacketSize);
    }
    return remain;
}

// actualize list

void UDPBasicP2P2Multi::actualizeList(UDPBasicPacketP2P *pkt)
{
    if (pkt->getMapSegmentsArraySize() == 0)
        return; // nothing to do


    int obj = pkt->getObjectId();
    if (obj == -1)
        return;

    SegmentMap::iterator itSegList = networkSegmentMap.find(pkt->getNodeId().getMAC().getInt());
    if (itSegList == networkSegmentMap.end())
    {
        networkSegmentMap[pkt->getNodeId().getMAC().getInt()] = new ObjectSegmentList();
        itSegList = networkSegmentMap.find(pkt->getNodeId().getMAC().getInt());
    }


    SegmentList * segList = NULL;
    auto itAux = itSegList->second->find(obj);
    if (itAux != itSegList->second->end())
        segList = &itAux->second;
    // actualize list

    if (segList == NULL)
    {
        // new
        itSegList->second->insert(std::make_pair(obj,SegmentList()));
        itAux = itSegList->second->find(obj);
        segList = &itAux->second;
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


void UDPBasicP2P2Multi::answerRequest(UDPBasicPacketP2P *pkt)
{
    PathAddress route;
    if (pkt->getSourceRouteArraySize() > 0) // source routing.
    {
        for (int i = pkt->getSourceRouteArraySize()-1; i >=0 ; i--)
        {
            route.push_back(pkt->getSourceRoute(i));
        }
    }

    int obj =  pkt->getObjectId();

    if (pkt->getSegmentId() != 0 && obj != -1 ) // specific segment check if the segment is present in the node
    {
        auto objIt =  mySegmentList.find(pkt->getObjectId());
        if (objIt ==  mySegmentList.end())
            throw cRuntimeError(" Request objects doesn't exist in the node ");
        SegmentList::iterator it =  objIt->second.find(pkt->getSegmentId());
        if (it == objIt->second.end())
            return;
    }

    // check if request objects exist in the node
    if (obj == -1 && pkt->getObjectsDesiredArraySize() == 0)
        throw cRuntimeError(" Object id and desired list empty ");

    if (obj == -1)
    {
        //check list
        bool found = false;
        for (int i = 0; i < (int)pkt->getObjectsDesiredArraySize(); i++)
        {
            auto objIt =  mySegmentList.find(pkt->getObjectsDesired(i));
            if (objIt !=  mySegmentList.end())
            {
                found = true;
                break;
            }
        }
        if (!found)
            throw cRuntimeError(" Request objects doesn't exist in the node ");
    }

    if (obj == -1)
    {
        // first at all, select a object
        std::vector<int> presentObj;
        for (int i = 0; i < (int)pkt->getObjectsDesiredArraySize(); i++)
        {
            auto objIt =  mySegmentList.find(pkt->getObjectsDesired(i));
            if (objIt !=  mySegmentList.end())
                presentObj.push_back(pkt->getObjectsDesired(i));
        }
        if (presentObj.empty())
            throw cRuntimeError(" Object requested not in list ");
        // search objects th
        int index = intrand(presentObj.size());
        obj = presentObj[index];
    }
    // prepare a delayed
    auto itSize = segmentSize.find(obj);
    DelayMessage *delayM = new DelayMessage();
    delayM->setNodeId(myAddress);
    delayM->setDestination(pkt->getNodeId());
    delayM->setType(SEGMEN);
    delayM->setTotalSize(itSize->second);
    delayM->setNextTime(simTime());
    delayM->setObjectId(obj);
    if (!route.empty())
        delayM->route = route;
    unsigned int total = ceil((double)delayM->getTotalSize()/(double)maxPacketSize);
    delayM->setTotal(total);
    delayM->setLastSend(simTime());
    uint64_t desAdd = pkt->getNodeId().getMAC().getInt();
    if (pkt->getSubSegmentRequestArraySize() == 0 && pkt->getSegmentId() == 0)
    {
        delayM->setIndex(1);
        delayM->setSegmentId(searchBestSegment(obj,desAdd));
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


    // El primero se env�a inmediatamente para evitar que la otra parte se impaciente
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
            for (TimeQueue::iterator it = timeQueue.begin();it != timeQueue.end(); ++it)
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
        delayM = NULL;
    }
    sendNow(pktSend);


    if (delayM)
    {
        if (timeQueue.empty() || (!timeQueue.empty() && timeQueue.back()->getNextTime() < delayM->getNextTime()))
            timeQueue.push_back(delayM);
        else
        {
            // search position
            for (TimeQueue::iterator it = timeQueue.begin();it != timeQueue.end(); ++it)
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
    emit(queueLengthInstSignal,getQueueSize());
    GlobalWirelessLinkInspector::setQueueSize(myAddress, getQueueSize());
}


void UDPBasicP2P2Multi::finish()
{
    recordScalar("packets send", numSent);
    recordScalar("packets received", numReceived);
    recordScalar("segment present", numObjectPresent);


    recordScalar("Request sent", numRequestSent);
    recordScalar("request received", numRequestServed);
    recordScalar("segment received", numRequestSegmentServed);

    for (auto elem : mySegmentList)
    {
        auto itAux = totalSegments.find(elem.first);
        if (itAux == totalSegments.end())
            throw cRuntimeError("Object id not found");
        if (elem.second.size() < itAux->second)
        {
            recordScalar("time need", SimTime::getMaxTime());
        }
        else
        {

            auto itStart = startReception.find(elem.first);
            if (itStart == startReception.end())
                throw cRuntimeError("Object id not found");
            auto itEnd = endReception.find(elem.first);
            if (itEnd == endReception.end())
                throw cRuntimeError("Object id not found");
            double timeNeed = itEnd->second.dbl() - itStart->second.dbl();
            char str[50];
            sprintf(str,"Object id : %d time need ",elem.first);
            recordScalar(str, timeNeed);
        }
    }
}


void UDPBasicP2P2Multi::WirelessNumNeig()
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

int UDPBasicP2P2Multi::getNumNeighNodes(uint64_t add,double dist)
{
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
}

ManetAddress UDPBasicP2P2Multi::DelayMessage::getNodeId() const
{
    return nodeId_var;
}

void UDPBasicP2P2Multi::DelayMessage::setNodeId(ManetAddress nodeId)
{
    this->nodeId_var = nodeId;
}

ManetAddress UDPBasicP2P2Multi::DelayMessage::getDestination() const
{
    return destination_var;
}

void UDPBasicP2P2Multi::DelayMessage::setDestination(ManetAddress destination)
{
    this->destination_var = destination;
}

int UDPBasicP2P2Multi::DelayMessage::getType() const
{
    return type_var;
}

void UDPBasicP2P2Multi::DelayMessage::setType(int type)
{
    this->type_var = type;
}

uint64_t UDPBasicP2P2Multi::DelayMessage::getTotalSize() const
{
    return totalSize_var;
}

void UDPBasicP2P2Multi::DelayMessage::setTotalSize(uint64_t totalSize)
{
    this->totalSize_var = totalSize;
}

uint64_t UDPBasicP2P2Multi::DelayMessage::getSegmentId() const
{
    return segmentId_var;
}

void UDPBasicP2P2Multi::DelayMessage::setSegmentId(uint64_t segmentId)
{
    this->segmentId_var = segmentId;
}

uint16_t UDPBasicP2P2Multi::DelayMessage::getSubSegmentId() const
{
    return subSegmentId_var;
}

void UDPBasicP2P2Multi::DelayMessage::setSubSegmentId(uint16_t subSegmentId)
{
    this->subSegmentId_var = subSegmentId;
}


simtime_t UDPBasicP2P2Multi::DelayMessage::getLastSend()
{
    return lastSend_var;
}

void UDPBasicP2P2Multi::DelayMessage::setLastSend(const simtime_t &val)
{

    lastSend_var = val;
}

unsigned int UDPBasicP2P2Multi::DelayMessage::getIndex()
{
    return index_var;
}

unsigned int UDPBasicP2P2Multi::DelayMessage::getTotal()
{
    return total_var;
}

void UDPBasicP2P2Multi::DelayMessage::setIndex(const unsigned int &val) {index_var = val;}
void UDPBasicP2P2Multi::DelayMessage::setTotal(const unsigned int &val)  {total_var = val;}


UDPBasicPacketP2P * UDPBasicP2P2Multi::DelayMessage::getPkt(const uint32_t &pkSize)
{
    UDPBasicPacketP2P *pkt = new UDPBasicPacketP2P();
    pkt->setNodeId(getNodeId());
    pkt->setDestination(getDestination());
    pkt->setType(getType());
    pkt->setTotalSize(getTotalSize());
    pkt->setSegmentId(getSegmentId());
    pkt->setObjectId(getObjectId());

    if (!route.empty())
    {
        pkt->setSourceRouteArraySize(route.size());
        for (int i = 0; i < (int)route.size();i++)
        {
            pkt->setSourceRoute(i,route[i]);
        }
    }

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


void UDPBasicP2P2Multi::DelayMessage::setPkt(UDPBasicPacketP2P *pkt)
{
    setNodeId(pkt->getNodeId());
    setDestination(pkt->getDestination());
    setType(pkt->getType());
    setTotalSize(pkt->getTotalSize());
    setSegmentId(pkt->getSegmentId());
    setSubSegmentId(pkt->getSubSegmentId());
    setObjectId(pkt->getObjectId());

    for (unsigned int i = 0; i < pkt->getSubSegmentRequestArraySize();i++)
        subSegmentRequest.push_back(pkt->getSubSegmentRequest(i));
    delete pkt;
}

UDPBasicPacketP2PNotification *UDPBasicP2P2Multi::getPacketWitMap(int obj)
{
    UDPBasicPacketP2PNotification *pkt = new UDPBasicPacketP2PNotification();

    pkt->setNodeId(myAddress);
    pkt->setSeqnum(mySeqNumber);
    mySeqNumber++;

    auto totalSegIt = totalSegments.find(obj);
    if (totalSegIt == totalSegments.end())
        throw cRuntimeError("Object id not found");

    pkt->setMapSegmentsSize(totalSegIt->second);
    pkt->setObjectId(obj);
    int size = static_cast<int>(std::ceil(static_cast<double>(totalSegIt->second)/8.0));
    pkt->setMapSegmentsArraySize(size);

    auto itObjectList = mySegmentList.find(obj);
    if (itObjectList != mySegmentList.end())
    {
        for (const auto &it : itObjectList->second)
        {
            // compute the char position
            int pos = (it-1)/8;
            char val = pkt->getMapSegments(pos);
            int bit = (it-1)%8;
            char aux = 1<<bit;
            val |= aux;
            pkt->setMapSegments(pos,val);
        }
    }
    pkt->setByteLength(8+4); // map size + seq num // id can extracted from ip
    return pkt;
}

void UDPBasicP2P2Multi::informChanges()
{
    if (useGlobal)
        return;
    for (auto it : mySegmentList)
    {
        UDPBasicPacketP2PNotification *pkt = getPacketWitMap(it.first);
        socket.sendTo(pkt, IPvXAddress(IPv4Address::ALLONES_ADDRESS), destPort);
    }
}

bool UDPBasicP2P2Multi::processMsgChanges(cPacket *msg)
{
    UDPBasicPacketP2PNotification *pkt = dynamic_cast<UDPBasicPacketP2PNotification *>(msg);
    if (pkt == NULL)
        return false;
    if (pkt->getNodeId() == myAddress)
    {
        delete msg;
        return true;
    }

    SequenceList::iterator it = sequenceList.find(pkt->getNodeId().getMAC().getInt());
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
        sequenceList[pkt->getNodeId().getMAC().getInt()] = pkt->getSeqnum();
    }

    int object = pkt->getObjectId();

    // process
    SegmentMap::iterator itSegList = networkSegmentMap.find(pkt->getNodeId().getMAC().getInt());
    ObjectSegmentList * segList = NULL;
    if (itSegList != networkSegmentMap.end())
        segList = itSegList->second;

    // actualize list

    if (segList == NULL)
    {
        // new
        segList = new ObjectSegmentList();
        networkSegmentMap.insert(std::make_pair(pkt->getNodeId().getMAC().getInt(),segList));
    }


    auto itAux = segList->find(object);
    if (itAux == segList->end())
    {
        SegmentList l;
        segList->insert(std::make_pair(object,l));
        itAux = segList->find(object);
    }

    for (unsigned int i = 0; i < pkt->getMapSegmentsArraySize(); i++)
    {
        char val = pkt->getMapSegments(i);
        for (int j = 0; j < 8; j++)
        {
            if (val & (1<<j))
            {
                unsigned int segId = (i * 8)+j+1;
                itAux->second.insert(segId);
            }
        }
    }

    socket.sendTo(msg, IPvXAddress(IPv4Address::ALLONES_ADDRESS), destPort); // propagate
    return true;
}



