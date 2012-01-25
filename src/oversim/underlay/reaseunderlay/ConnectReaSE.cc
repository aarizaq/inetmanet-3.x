//
// Copyright (C) 2010 Institut fuer Telematik, Karlsruher Institut fuer Technologie (KIT)
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

/**
 * @file ConnectReaSE.cc
 * @author Markus Mauch, Bernhard Mueller
 */


#include <vector>
#include <iostream>

#include <omnetpp.h>

#include <IRoutingTable.h>
#include <IInterfaceTable.h>
#include <IPvXAddressResolver.h>
#include <IPv4InterfaceData.h>

#include "ConnectReaSE.h"

using namespace ::std;

Define_Module(ConnectReaSE);

std::ostream& operator<<(std::ostream& os, terminalInfo& n)
{
    os << n.module;
    return os;
}

void ConnectReaSE::initialize(int stage)
{
    //EV << "Connector Stage" << stage;
    if (stage != MAX_STAGE_UNDERLAY)
        return;

    //get parameters
    channelTypesTx = cStringTokenizer(par("channelTypes"), " ").asVector();
    channelTypesRx = cStringTokenizer(par("channelTypesRx"), " ").asVector();
    channelDiversity = par("channelDiversity");

    if (channelTypesTx.size() < channelTypesRx.size()) {
        channelTypesTx = channelTypesRx;
    }
    else if (channelTypesTx.size() > channelTypesRx.size()) {
        channelTypesRx = channelTypesTx;
    }

    // make sure that delay cannot be zero and diversity cannot be below zero
    if (channelDiversity>=100)
        channelDiversity = 99.99f;
    else if (channelDiversity<0)
        channelDiversity = 0;

    // statistics
    lifetimeVector.setName("Terminal Lifetime");

    // set up network

    cModule* tempModule;
    cTopology tempTopology("tempTopo");
    edgePool tempEdgePool;


    tempTopology.extractByProperty("AS");
    totalCountOfAS = tempTopology.getNumNodes();

    nextPow = 0;
    while (((uint32_t) 1 << nextPow) < totalCountOfAS + 1) {
        nextPow++;
    }
    ASShift = 32 - nextPow;

    if (tempTopology.getNumNodes() == 0) {

        //no AS topology

        tempTopology.extractByProperty("RL");

        if (tempTopology.getNumNodes() == 0)

            //no router topology
            opp_error("ConnectReaSE: Neither an AS topology nor a router topology was detected.");

        setUpAS(getParentModule());
    }
    else {

        // set up all autonomous systems (AS)
        int index = 0;
        std::vector<cModule*> Modules;
        for (int i=0; i<tempTopology.getNumNodes(); i++) {

            tempModule = tempTopology.getNode(i)->getModule();
            Modules.push_back(tempModule);
            index++;
        }
        for (uint32 i=0; i< Modules.size(); i++) {
            setUpAS(Modules[i]);
        }
    }

    // put all edge router in one data structure to get a equal probability of selection
    for (uint32 i=0; i<AS_Pool.size(); i++) {
        for (uint32 j=0; j< AS_Pool[i].edgeRouter.size(); j++) {
            tempEdgePool.edge = &AS_Pool[i].edgeRouter[j];
            tempEdgePool.indexAS = i;
            globalEdgePool.push_back(tempEdgePool);
        }
    }
    //cout << "Number of Edges in Network: " << globalEdgePool.size() << endl;

    for (uint32 i=0; i<globalEdgePool.size(); i++) {

        //select channel
        globalEdgePool[i].edge->channelTypeTxStr = channelTypesTx[intuniform(0,channelTypesTx.size()-1)];
        globalEdgePool[i].edge->channelTypeRxStr = channelTypesRx[intuniform(0,channelTypesRx.size()-1)];
    }
}

AccessInfo ConnectReaSE::getAccessNode()
{
    Enter_Method("getAccessNode()");

    bool candidateOK = false;
    int numTries = 10;
    uint32 test_IP = 0;
    AccessInfo node;
    edgeRoutes* connectionCandidate;
    uint32 tempIndex, tempASindex;

    while ((numTries > 0)&&(!candidateOK)) {
        numTries--;
        tempIndex = intuniform(0, globalEdgePool.size()-1);
        connectionCandidate = globalEdgePool[tempIndex].edge;
        tempASindex = globalEdgePool[tempIndex].indexAS;

        //limit terminals per edge router
        if (connectionCandidate->IPAddresses.size() >= ((uint32_t) 1 << AS_Pool[tempASindex].edgeShift)) // maximum reached?
            continue;

//        test_IP = connectionCandidate->IPAddress + 1; // begin with first IP after the edge router
//        for (int i = 0; i < (1 << AS_Pool[tempASindex].edgeShift); i++ ) {
//            test_IP += i;
//            candidateOK = true;
//            for (uint32 j = 0; j < connectionCandidate->IPAddresses.size(); j++) {
//                if (connectionCandidate->IPAddresses[j] == test_IP) {
//                    candidateOK =false;
//                    break;
//                }
//            }
//            if (candidateOK) {
//                connectionCandidate->IPAddresses.push_back(test_IP);
//                break;
//            }
//        }

        // FIXME: check overlays for side effects of reused IP addresses

        test_IP = ++connectionCandidate->lastIP;
        connectionCandidate->IPAddresses.push_back(test_IP);
        break;

    }

    // no free IP address after 10 tries
    if (numTries == 0) {
        opp_error("Error creating node: No available IP found after four tries!");
    }
    EV << "Found available IP: " << test_IP;

    node.ASindex = tempASindex;
    node.IPAddress = test_IP;
    node.edge = connectionCandidate;
    return node;
}

int ConnectReaSE::addOverlayNode(AccessInfo* overlayNode, bool migrate)
{
        Enter_Method("addOverlayNode()");

        cModule* node = overlayNode->terminal;
        terminalInfo terminal;

        terminal.module = node;
        terminal.interfaceTable = IPvXAddressResolver().interfaceTableOf(node);
        terminal.remoteInterfaceTable = overlayNode->edge->interfaceTable;
        terminal.routingTable = IPvXAddressResolver().routingTableOf(node);
        terminal.PPPInterface = node->getSubmodule("ppp", 0);
        terminal.createdAt = simTime();
        terminal.IPAddress = overlayNode->IPAddress;
        terminal.edgeRouter = overlayNode->edge;
        terminal.ASindex = overlayNode->ASindex;

        // update display
        if (ev.isGUI()) {
                const char* ip_disp = const_cast<char*>
                (IPv4Address(terminal.IPAddress).str().c_str());
                terminal.module->getDisplayString().insertTag("t", 0);
                terminal.module->getDisplayString().setTagArg("t", 0, ip_disp);
                terminal.module->getDisplayString().setTagArg("t", 1, "l");
        }

        //find first unused interface
        int k = 1;
        while ( overlayNode->edge->Router->findSubmodule("ppp", k) != -1 )
            k++;


        cModuleType* pppInterfaceModuleType = cModuleType::get("inet.linklayer.ppp.PPPInterface");
        terminal.remotePPPInterface = pppInterfaceModuleType->
        create("ppp", overlayNode->edge->Router, 0, k);

        overlayNode->edge->countPPPInterfaces++;


        // connect terminal to access router and vice versa
        cGate* routerInGate = firstUnusedGate(overlayNode->edge->Router, "pppg", cGate::INPUT);
        cGate* routerOutGate = firstUnusedGate(overlayNode->edge->Router, "pppg", cGate::OUTPUT);

        cChannelType* channelTypeRx = cChannelType::find( overlayNode->edge->channelTypeRxStr.c_str() );
        cChannelType* channelTypeTx = cChannelType::find( overlayNode->edge->channelTypeTxStr.c_str() );
        if (!channelTypeRx || !channelTypeTx)
            opp_error("Could not find Channel or ChannelRx Type. Most likely "
                "parameter channelTypes does not match the channels defined "
                "in channels.ned");

        //create channels
        cDatarateChannel* channelRx = check_and_cast<cDatarateChannel*>(channelTypeRx->create(overlayNode->edge->channelTypeRxStr.c_str()));
        cDatarateChannel* channelTx = check_and_cast<cDatarateChannel*>(channelTypeTx->create(overlayNode->edge->channelTypeTxStr.c_str()));

        //connect terminal
        terminal.module->gate("pppg$o", 0)->connectTo(routerInGate,channelRx);
        routerOutGate->connectTo(terminal.module->gate("pppg$i", 0),channelTx);

        // connect ppp interface module to router module and vice versa
        routerInGate->connectTo(terminal.remotePPPInterface->gate("phys$i"));
        terminal.remotePPPInterface->gate("phys$o")->connectTo(routerOutGate);

        // connect ppp interface module to network layer module and vice versa
        cModule* netwModule = overlayNode->edge->Router->getSubmodule("networkLayer");

        cGate* netwInGate = firstUnusedGate(netwModule, "ifIn");
        cGate* netwOutGate = firstUnusedGate(netwModule, "ifOut");

        netwOutGate->connectTo(terminal.remotePPPInterface->gate("netwIn"));
        terminal.remotePPPInterface->gate("netwOut")->connectTo(netwInGate);

        // connect network layer module to ip and arp modules
        cModule* ipModule = overlayNode->edge->Router->getSubmodule("networkLayer")->
        getSubmodule("ip");

        cGate* ipIn = firstUnusedGate(ipModule, "queueIn");
        netwInGate->connectTo(ipIn);

        //
        // Start ppp interface modules
        //
        terminal.remotePPPInterface->finalizeParameters();
        terminal.remotePPPInterface->setDisplayString("i=block/ifcard");
        terminal.remotePPPInterface->buildInside();
        terminal.remotePPPInterface->scheduleStart(simTime());
        terminal.remotePPPInterface->callInitialize();

        if ( !migrate) {
            // we are already in stage 4 and need to call initialize
        // for all previous stages manually
            for (int i=0; i < MAX_STAGE_UNDERLAY + 1; i++) {
                terminal.module->callInitialize(i);
            }
        }

        //calculate diversity factor for both channels (+/- diversity)
        double diversityFactor = (uniform(0 , 2*channelDiversity) + (100-channelDiversity))/100;

        //customize channel delays
        channelRx->setDelay(SIMTIME_DBL(channelRx->getDelay()*diversityFactor));
        channelTx->setDelay(SIMTIME_DBL(channelTx->getDelay()*diversityFactor));

        terminal.remoteInterfaceEntry = overlayNode->edge->interfaceTable->getInterface(
        overlayNode->edge->interfaceTable->getNumInterfaces() - 1);
        terminal.interfaceEntry = terminal.interfaceTable->getInterfaceByName("ppp0");


        //
        // Fill in interface table.
        //

        // router
        IPv4InterfaceData* interfaceData = new IPv4InterfaceData;
        interfaceData->setIPAddress(overlayNode->edge->IPAddress);
        interfaceData->setNetmask(IPv4Address::ALLONES_ADDRESS);
        terminal.remoteInterfaceEntry->setIPv4Data(interfaceData);

        // terminal
        terminal.interfaceEntry->ipv4Data()->setIPAddress(IPv4Address(terminal.IPAddress));
        terminal.interfaceEntry->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS);

        //
        // Fill in routing table.
        //


        // add edge routing entry

        IPv4Route* re = new IPv4Route();
        re->setDestination(IPv4Address(terminal.IPAddress));
        re->setNetmask(IPv4Address::ALLONES_ADDRESS);
        re->setInterface(terminal.remoteInterfaceEntry);
        re->setType(IPv4Route::DIRECT);
        re->setSource(IPv4Route::MANUAL);
        overlayNode->edge->routingTable->addRoute(re);
        terminal.remoteRoutingEntry = re;


        //  add terminal routing entry
        IPv4Route* te = new IPv4Route();
        te->setDestination(IPv4Address::UNSPECIFIED_ADDRESS);
        te->setNetmask(IPv4Address::UNSPECIFIED_ADDRESS);
        te->setGateway(overlayNode->edge->IPAddress);
        te->setInterface(terminal.interfaceEntry);
        te->setType(IPv4Route::REMOTE);
        te->setSource(IPv4Route::MANUAL);
        terminal.routingTable->addRoute(te);
        terminal.routingEntry = te;

        // append module to overlay terminal vector
        overlayTerminal.push_back(terminal);
        updateDisplayString();
        int ID = node->getId();
        return ID;
}

cModule* ConnectReaSE::removeOverlayNode(int ID)
{
    Enter_Method("removeOverlayNode()");

    cModule* node = NULL;
    terminalInfo terminal;
    int index = 0;
    uint32 releasedIP;

    // find module
    for (unsigned int i=0; i<overlayTerminal.size(); i++) {
        if (overlayTerminal[i].module->getId() == ID) {
            terminal = overlayTerminal[i];
            node = terminal.module;
            index = i;
            break;
        }
    }

    if (node == NULL) return NULL;


    releasedIP = IPvXAddressResolver().addressOf(terminal.module).get4().getInt();;

    // free IP address
    for (unsigned int i=0; i < terminal.edgeRouter->IPAddresses.size(); i++) {
            if (terminal.edgeRouter->IPAddresses[i] == releasedIP) {
                terminal.edgeRouter->IPAddresses.erase(terminal.edgeRouter->IPAddresses.begin() + i);
                break;
            }
        }


    cModule* ppp = terminal.remotePPPInterface;


    // disconnect terminal
    node->gate("pppg$o", 0)->disconnect();
    node->gate("pppg$i", 0)->getPreviousGate()->disconnect();

    // disconnect ip and arp modules
    ppp->gate("netwIn")->getPathStartGate()->disconnect();
    ppp->gate("netwOut")->getNextGate()->disconnect();

    // remove associated ppp interface module
    ppp->callFinish();
    ppp->deleteModule();

    terminal.edgeRouter->countPPPInterfaces--;

    // remove associated interface table entry
    terminal.edgeRouter->interfaceTable->deleteInterface(terminal.remoteInterfaceEntry);

    // remove routing entries

    terminal.routingTable->deleteRoute(terminal.routingEntry);
    terminal.edgeRouter->routingTable ->deleteRoute(terminal.remoteRoutingEntry);

    //TODO: implement life vector statistics
    lifetimeVector.record(simTime() - overlayTerminal[index].createdAt);

    // remove terminal from overlay terminal vector
    overlayTerminal.erase(overlayTerminal.begin() + index);

     updateDisplayString();

    return node;
}

cModule* ConnectReaSE::getOverlayNode(int ID)
{
    Enter_Method("getOverlayNode()");

    cModule* node = NULL;

    for (unsigned int i=0; i<overlayTerminal.size(); i++) {
        if (overlayTerminal[i].module->getId() == ID) {
            node = overlayTerminal[i].module;
            return node;
        }
    }
    opp_error("Node was not found in global list of overlay terminals");
    return node;
}

AccessInfo ConnectReaSE::migrateNode(int ID)
{

    Enter_Method("migrateNode()");

    AccessInfo newEdgeRouter;
    terminalInfo terminal;

    for (unsigned int i=0; i<overlayTerminal.size(); i++) {
        if (overlayTerminal[i].module->getId() == ID) {
            terminal = overlayTerminal[i];
            break;
        }
    }

    if (terminal.module == NULL) opp_error("ConnectReaSE: Cannot find migrating node");

    do {
        newEdgeRouter = getAccessNode();
    }
    while ((newEdgeRouter.edge->Router->getId() == terminal.edgeRouter->Router->getId()) && (AS_Pool[terminal.ASindex].edgeRouter.size()!= 1));

    return newEdgeRouter;
}

void ConnectReaSE::handleMessage(cMessage* msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

void ConnectReaSE::updateDisplayString()
{
    if (ev.isGUI()) {
        char buf[80];
        if ( overlayTerminal.size() == 1 ) {
            sprintf(buf, "1 terminal connected");
        } else {
            sprintf(buf, "%ui terminals connected", overlayTerminal.size());
        }
        getDisplayString().setTagArg("t", 0, buf);
        getDisplayString().setTagArg("t", 2, "blue");

        if ((overlayTerminal.size() % 100) == 0) {
            cerr << "ConnectReaSE: " << overlayTerminal.size() << " Terminals connected in network." <<endl;
        }
    }
}

cGate* ConnectReaSE::firstUnusedGate(cModule* owner, const char* name, cGate::Type type)
{
    int index;
    for (index = 0; index < owner->gateSize(name); index++) {
        cGate *gate = type == cGate::NONE ? owner->gate(name, index) : owner->gateHalf(name, type, index);
        if (!gate->isConnectedOutside()) {
            return gate;
        }
    }

    owner->setGateSize(name, index + 2);
    return type == cGate::NONE ? owner->gate(name, index + 1) : owner->gateHalf(name, type, index + 1);
}

bool ConnectReaSE::extractFromParentModule(cModule* currModule, void* properties)
{
    topologyProperty* currProp = (topologyProperty*)properties;

    if (currModule->getParentModule() == currProp->pModule) {
        if (currModule->getProperties()->get(currProp->property)) {
            return true;
        }
    }
    return false;
}

void ConnectReaSE::setUpAS(cModule* currAS)
{

    cTopology edgeTopo("Edges");
    cTopology Topo("All nodes");
    topologyProperty* tempProp = new topologyProperty;
    cModule* tempNode;
    uint32 tempIP;
    edgeRoutes tempEdge;
    autoSystem tempAS;
    int k;

    tempProp->pModule = currAS;


    tempProp->property = "EdgeRouter";
    edgeTopo.extractFromNetwork(extractFromParentModule, (void*) tempProp);

    tempProp->property = "RL";
    Topo.extractFromNetwork(extractFromParentModule, (void*) tempProp);

    //get net shift for IP address

    nextPow = 1;
    while ((1 << nextPow) < edgeTopo.getNumNodes() + 2) {
        nextPow++;
    }
    tempAS.edgeShift = ASShift - nextPow;

    // add information about edge router

    for (int i=0; i< edgeTopo.getNumNodes(); i++) {

        tempEdge.Router = edgeTopo.getNode(i)->getModule();
        tempEdge.IPAddress = IPvXAddressResolver().addressOf(tempEdge.Router).get4().getInt();
        tempEdge.IPAddresses.push_back(IPvXAddressResolver().addressOf(tempEdge.Router).get4().getInt());
        tempEdge.interfaceTable = IPvXAddressResolver().interfaceTableOf(tempEdge.Router);
        tempEdge.routingTable = IPvXAddressResolver().routingTableOf(tempEdge.Router);

        k = 0;
        while ( tempEdge.Router->findSubmodule("ppp", k) != -1 )
            k++;
        tempEdge.countPPPInterfaces = k;

        // the last allocated IP is the router address plus the (k - 1) connected ReaSE hosts
        tempEdge.lastIP = tempEdge.IPAddress + k - 1; // FIXME: check overlays for side effects of reused IP addresses

        // find hosts

        Topo.calculateUnweightedSingleShortestPathsTo(Topo.getNodeFor(edgeTopo.getNode(i)->getModule()));

        for (int j=0; j< Topo.getNumNodes(); j++) {
            tempNode = Topo.getNode(j)->getModule();
            if (tempNode->getProperties()->get("CoreRouter"))
                continue;
            if (tempNode->getProperties()->get("GatewayRouter"))
                continue;
            if (tempNode->getProperties()->get("EdgeRouter"))
                continue;
            if (Topo.getNode(j)->getPath(0)->getRemoteNode()->getModule() != edgeTopo.getNode(i)->getModule())
                continue;

            //fill IP table
            tempIP = IPvXAddressResolver().addressOf(Topo.getNode(j)->getModule()).get4().getInt();
            tempEdge.IPAddresses.push_back(tempIP);
        }

        tempAS.edgeRouter.push_back(tempEdge);
    }

    delete tempProp;
    AS_Pool.push_back(tempAS);
}
