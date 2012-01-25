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
 * @file ReaSEUnderlayConfigurator.cc
 * @author Markus Mauch, Stephan Krause, Bernhard Heep, Bernhard Mueller
 */

#include "ReaSEUnderlayConfigurator.h"

#include <GlobalNodeList.h>
#include <TransportAddress.h>

#include <StringConvert.h>

#include <ConnectReaSE.h>
#include <IRoutingTable.h>
#include <IInterfaceTable.h>
#include <IPvXAddressResolver.h>
#include <IPv4InterfaceData.h>
#include <NotificationBoard.h>


#include <ReaSEInfo.h>

Define_Module(ReaSEUnderlayConfigurator);

void ReaSEUnderlayConfigurator::initializeUnderlay(int stage)
{
    //backbone configuration
    if (stage == MIN_STAGE_UNDERLAY) {

    }
    //access net configuration
    else if (stage == MAX_STAGE_UNDERLAY) {
        // fetch some parameters


        // count the overlay clients
        overlayTerminalCount = 0;

        numCreated = 0;
        numKilled = 0;

        // add access node modules to access node vector
        TerminalConnector = (ConnectReaSE*)getParentModule()->getSubmodule("TerminalConnector");
    }
}

TransportAddress* ReaSEUnderlayConfigurator::createNode(NodeType type, bool initialize)
{
    Enter_Method_Silent();
    // derive overlay node from ned
    std::string nameStr = "overlayTerminal";
    if (churnGenerator.size() > 1) {
        nameStr += "-" + convertToString<int32_t>(type.typeID);
    }
    AccessInfo accessNet= TerminalConnector->getAccessNode();
    cModuleType* moduleType = cModuleType::get(type.terminalType.c_str());
    cModule* node = moduleType->create(nameStr.c_str(), accessNet.edge->Router->getParentModule(), //TODO: insert node in submodule
                                       numCreated + 1, numCreated);

    if (type.channelTypesTx.size() > 0) {
        throw cRuntimeError("ReaSEUnderlayConfigurator::createNode(): Setting "
                    "channel types via the churn generator is not allowed "
                    "with the ReaSEUnderlay. Use **.accessNet.channelTypes instead!");
    }

    node->setGateSize("pppg", 1);

    std::string displayString;

    if ((type.typeID > 0) && (type.typeID <= NUM_COLORS)) {
        ((displayString += "i=device/wifilaptop_l,")
                        += colorNames[type.typeID - 1])
                        += ",40;i2=block/circle_s";
    } else {
        displayString = "i=device/wifilaptop_l;i2=block/circle_s";
    }

    node->finalizeParameters();
    node->setDisplayString(displayString.c_str());

    node->buildInside();
    node->scheduleStart(simTime());

    // create meta information
    ReaSEInfo* info = new ReaSEInfo(type.typeID, node->getId(), type.context);

    accessNet.terminal = node;
    // add node to a randomly chosen access net
    info->setNodeID(TerminalConnector->addOverlayNode(&accessNet));


    // add node to bootstrap oracle
    globalNodeList->addPeer(IPvXAddressResolver().addressOf(node), info);

    // if the node was not created during startup we have to
    // finish the initialization process manually
    if (!initialize) {
        for (int i = MAX_STAGE_UNDERLAY + 1; i < NUM_STAGES_ALL; i++) {
            node->callInitialize(i);
        }
    }

    overlayTerminalCount++;
    numCreated++;

    churnGenerator[type.typeID]->terminalCount++;

    TransportAddress *address = new TransportAddress(
                                       IPvXAddressResolver().addressOf(node));

    // update display
    setDisplayString();

    return address;
}

//TODO: getRandomNode()
void ReaSEUnderlayConfigurator::preKillNode(NodeType type, TransportAddress* addr)
{
    Enter_Method_Silent();

    // AccessNet* accessNetModule = NULL;
    int nodeID;
    ReaSEInfo* info;

    // If no address given, get random node
    if (addr == NULL) {
        addr = globalNodeList->getRandomAliveNode(type.typeID);

        if (addr == NULL) {
            // all nodes are already prekilled
            std::cout << "all nodes are already prekilled" << std::endl;
            return;
        }
    }

    // get node information
    info = dynamic_cast<ReaSEInfo*>(globalNodeList->getPeerInfo(*addr));

    if (info != NULL) {
        //accessNetModule = info->getAccessNetModule();
        nodeID = info->getNodeID();
    } else {
        opp_error("IPv4UnderlayConfigurator: Trying to pre kill node "
                  "with nonexistant TransportAddress!");
    }

    uint32_t effectiveType = info->getTypeID();

    // do not kill node that is already scheduled
    if (scheduledID.count(nodeID))
        return;
    //TODO: get overlay node
    cModule* node = TerminalConnector->getOverlayNode(nodeID);
    globalNodeList->removePeer(IPvXAddressResolver().addressOf(node));

    //put node into the kill list and schedule a message for final removal of the node
    killList.push_front(IPvXAddressResolver().addressOf(node));
    scheduledID.insert(nodeID);

    overlayTerminalCount--;
    numKilled++;

    churnGenerator[effectiveType]->terminalCount--;

    // update display
    setDisplayString();

    // inform the notification board about the removal
    NotificationBoard* nb = check_and_cast<NotificationBoard*>(
            node->getSubmodule("notificationBoard"));
    nb->fireChangeNotification(NF_OVERLAY_NODE_LEAVE);

    double random = uniform(0, 1);

    if (random < gracefulLeaveProbability) {
        nb->fireChangeNotification(NF_OVERLAY_NODE_GRACEFUL_LEAVE);
    }

    cMessage* msg = new cMessage();
    scheduleAt(simTime() + gracefulLeaveDelay, msg);

}

void ReaSEUnderlayConfigurator::migrateNode(NodeType type, TransportAddress* addr)
{
    Enter_Method_Silent();

    // AccessNet* accessNetModule = NULL;
    int nodeID = -1;
    ReaSEInfo* info;

    // If no address given, get random node
    if (addr == NULL) {
        info = dynamic_cast<ReaSEInfo*>(globalNodeList->getRandomPeerInfo(type.typeID));
    } else {
        // get node information
        info = dynamic_cast<ReaSEInfo*>(globalNodeList->getPeerInfo(*addr));
    }

    if (info != NULL) {
        //accessNetModule = info->getAccessNetModule();
        nodeID = info->getNodeID();
    } else {
        opp_error("ReaSEUnderlayConfigurator: Trying to pre kill node with nonexistant TransportAddress!");
    }

    // do not migrate node that is already scheduled
    if (scheduledID.count(nodeID))
        return;

    cModule* node = TerminalConnector->removeOverlayNode(nodeID);//intuniform(0, accessNetModule->size() - 1));

    if (node == NULL)
        opp_error("ReaSEUnderlayConfigurator: Trying to remove node which is not an overlay node in network!");

    //remove node from bootstrap oracle
    globalNodeList->killPeer(IPvXAddressResolver().addressOf(node));

    node->bubble("I am migrating!");
    // connect the node to another access net
    AccessInfo newAccessModule;

    // create meta information
    ReaSEInfo* newinfo = new ReaSEInfo(type.typeID, node->getId(), type.context);
        newAccessModule = TerminalConnector->migrateNode(node->getId());
        newAccessModule.terminal = node;
        // add node to a randomly chosen access net
        info->setNodeID(TerminalConnector->addOverlayNode(&newAccessModule));

    //add node to bootstrap oracle
    globalNodeList->addPeer(IPvXAddressResolver().addressOf(node), newinfo);

    // inform the notification board about the migration
    NotificationBoard* nb = check_and_cast<NotificationBoard*>(node->getSubmodule("notificationBoard"));
    nb->fireChangeNotification(NF_OVERLAY_TRANSPORTADDRESS_CHANGED);
}

void ReaSEUnderlayConfigurator::handleTimerEvent(cMessage* msg)
{
    Enter_Method_Silent();

    // get next scheduled node from the kill list
    IPvXAddress addr = killList.back();
    killList.pop_back();

    //AccessNet* accessNetModule = NULL;
    int nodeID = -1;

    ReaSEInfo* info = dynamic_cast<ReaSEInfo*>(globalNodeList->getPeerInfo(addr));
    if (info != NULL) {
        //accessNetModule = info->getAccessNetModule();
        nodeID = info->getNodeID();
    } else {
        opp_error("IPv4UnderlayConfigurator: Trying to kill node with nonexistant TransportAddress!");
    }

    scheduledID.erase(nodeID);
    globalNodeList->killPeer(addr);

    cModule* node = TerminalConnector->removeOverlayNode(nodeID);

    if (node == NULL)
        opp_error("IPv4UnderlayConfigurator: Trying to remove node which is nonexistant in AccessNet!");

    node->callFinish();
    node->deleteModule();

    delete msg;
}

void ReaSEUnderlayConfigurator::setDisplayString()
{
    char buf[80];
    sprintf(buf, "%i overlay terminals",
            overlayTerminalCount);
    getDisplayString().setTagArg("t", 0, buf);
}

void ReaSEUnderlayConfigurator::finishUnderlay()
{
    // statistics
    recordScalar("Terminals added", numCreated);
    recordScalar("Terminals removed", numKilled);

    if (!isInInitPhase()) {
        struct timeval now, diff;
        gettimeofday(&now, NULL);
        timersub(&now, &initFinishedTime, &diff);
        printf("Simulation time: %li.%06li\n", diff.tv_sec, diff.tv_usec);
    }
}


