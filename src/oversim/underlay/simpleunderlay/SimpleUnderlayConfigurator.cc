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
 * @file SimpleUnderlayConfigurator.cc
 * @author Stephan Krause
 * @author Bernhard Heep (migrateRandomNode)
 */

#include <omnetpp.h>
#include <malloc.h>
#include <vector>
#include <map>

#include <fstream>

#include <NodeHandle.h>
#include "IInterfaceTable.h"
#include "InterfaceEntry.h"
#include "IPv4InterfaceData.h"
#include "IPv6InterfaceData.h"
#include "TransportAddress.h"
#include "IPvXAddressResolver.h"
#include <cenvir.h>
#include <cxmlelement.h>
#include "ChurnGenerator.h"
#include "GlobalNodeList.h"
#include <StringConvert.h>

#include "SimpleUDP.h"
#include "SimpleTCP.h"

#include "SimpleUnderlayConfigurator.h"

Define_Module(SimpleUnderlayConfigurator);

using namespace std;

SimpleUnderlayConfigurator::~SimpleUnderlayConfigurator()
{
    for (uint32_t i = 0; i < nodeRecordPool.size(); ++i) {
        //std::cout << (nodeRecordPool[i].second ? "+" : "_");
        delete nodeRecordPool[i].first;
    }
    nodeRecordPool.clear();
}

void SimpleUnderlayConfigurator::initializeUnderlay(int stage)
{
    if (stage != MAX_STAGE_UNDERLAY)
        return;

    // fetch some parameters
    fixedNodePositions = par("fixedNodePositions");
    useIPv6 = par("useIPv6Addresses");

    // set maximum coordinates and send queue length
    fieldSize = par("fieldSize");
    sendQueueLength = par("sendQueueLength");

    // get parameter of sourcefile's name
    nodeCoordinateSource = par("nodeCoordinateSource");

    if (std::string(nodeCoordinateSource) != "") {
        // check if exists and process xml-file containing coordinates
        std::ifstream check_for_xml_file(nodeCoordinateSource);
        if (check_for_xml_file) {
            useXmlCoords = 1;

            EV<< "[SimpleNetConfigurator::initializeUnderlay()]\n"
            << "    Using '" << nodeCoordinateSource
            << "' as coordinate source file" << endl;

            maxCoordinate = parseCoordFile(nodeCoordinateSource);
        } else {
            throw cRuntimeError("Coordinate source file not found!");
        }
        check_for_xml_file.close();
    } else {
        useXmlCoords = 0;
        dimensions = 2; //TODO do we need this variable?
        NodeRecord::setDim(dimensions);
        EV << "[SimpleNetConfigurator::initializeUnderlay()]\n"
        << "    Using conventional (random) coordinates for placing nodes!\n"
        << "    (no XML coordinate source file was specified)" << endl;
    }

    // FIXME get address from parameter
    nextFreeAddress = 0x1000001;

    // count the overlay clients
    overlayTerminalCount = 0;

    numCreated = 0;
    numKilled = 0;
}

TransportAddress* SimpleUnderlayConfigurator::createNode(NodeType type,
                                                         bool initialize)
{
    Enter_Method_Silent();
    // derive overlay node from ned
    cModuleType* moduleType = cModuleType::get(type.terminalType.c_str());

    std::string nameStr = "overlayTerminal";
    if (churnGenerator.size() > 1) {
        nameStr += "-" + convertToString<int32_t>(type.typeID);
    }
    cModule* node = moduleType->create(nameStr.c_str(), getParentModule(),
                                       numCreated + 1, numCreated);

    std::string displayString;

    if ((type.typeID > 0) && (type.typeID <= NUM_COLORS)) {
        ((displayString += "i=device/wifilaptop_l,")
                += colorNames[type.typeID - 1]) += ",40;i2=block/circle_s";
    } else {
        displayString = "i=device/wifilaptop_l;i2=block/circle_s";
    }

    node->finalizeParameters();
    node->setDisplayString(displayString.c_str());
    node->buildInside();
    node->scheduleStart(simTime());

    for (int i = 0; i < MAX_STAGE_UNDERLAY + 1; i++) {
        node->callInitialize(i);
    }

    IPvXAddress addr;
    if (useIPv6) {
        addr = IPv6Address(0, nextFreeAddress++, 0, 0);
    } else {
        addr = IPv4Address(nextFreeAddress++);
    }

    int chanIndex = intuniform(0, type.channelTypesRx.size() - 1);
    cChannelType* rxChan = cChannelType::find(type.channelTypesRx[chanIndex].c_str());
    cChannelType* txChan = cChannelType::find(type.channelTypesTx[chanIndex].c_str());

    if (!txChan || !rxChan)
         opp_error("Could not find Channel Type. Most likely parameter "
            "channelTypesRx or channelTypes does not match the channels defined in "
             "channels.ned");

    SimpleNodeEntry* entry;

    if (!useXmlCoords) {
        entry = new SimpleNodeEntry(node, rxChan, txChan, sendQueueLength, fieldSize);
    } else {
        // get random unused node
        uint32_t volunteer = intuniform(0, nodeRecordPool.size() - 1);
        uint32_t temp = volunteer;
        while (nodeRecordPool[volunteer].second == false) {
            ++volunteer;
            if (volunteer >= nodeRecordPool.size())
                volunteer = 0;
            // stop with errormessage if no more unused nodes available
            if (temp == volunteer)
                throw cRuntimeError("No unused coordinates left -> "
                    "cannot create any more nodes. "
                    "Provide %s-file with more nodes!\n", nodeCoordinateSource);
        }

         entry = new SimpleNodeEntry(node, rxChan, txChan,
                 sendQueueLength, nodeRecordPool[volunteer].first, volunteer);

        // insert IP-address into noderecord used
        // nodeRecordPool[volunteer].first->ip = addr;
        nodeRecordPool[volunteer].second = false;
    }

    SimpleUDP* simpleUdp = check_and_cast<SimpleUDP*> (node->getSubmodule("udp"));
    simpleUdp->setNodeEntry(entry);
    SimpleTCP* simpleTcp = dynamic_cast<SimpleTCP*> (node->getSubmodule("tcp", 0));
    if (simpleTcp) {
        simpleTcp->setNodeEntry(entry);
    }

    // Add pseudo-Interface to node's interfaceTable
    if (useIPv6) {
        IPv6InterfaceData* ifdata = new IPv6InterfaceData;
        ifdata->assignAddress(addr.get6(),false, 0, 0);
        IPv6InterfaceData::AdvPrefix prefix;
        prefix.prefix = addr.get6();
        prefix.prefixLength = 64;
        ifdata->addAdvPrefix(prefix);
        InterfaceEntry* e = new InterfaceEntry;
        e->setName("dummy interface");
        e->setIPv6Data(ifdata);
        IPvXAddressResolver().interfaceTableOf(node)->addInterface(e, NULL);
    }
    else {
        IPv4InterfaceData* ifdata = new IPv4InterfaceData;
        ifdata->setIPAddress(addr.get4());
        ifdata->setNetmask(IPv4Address("255.255.255.255"));
        InterfaceEntry* e = new InterfaceEntry;
        e->setName("dummy interface");
        e->setIPv4Data(ifdata);

        IPvXAddressResolver().interfaceTableOf(node)->addInterface(e, NULL);
    }

    // create meta information
    SimpleInfo* info = new SimpleInfo(type.typeID, node->getId(), type.context);
    info->setEntry(entry);

    //add node to bootstrap oracle
    globalNodeList->addPeer(addr, info);

    // if the node was not created during startup we have to
    // finish the initialization process manually
    if (!initialize) {
        for (int i = MAX_STAGE_UNDERLAY + 1; i < NUM_STAGES_ALL; i++) {
            node->callInitialize(i);
        }
    }

    //show ip...
    //TODO: migrate
    if (fixedNodePositions && ev.isGUI()) {
        node->getDisplayString().insertTag("p");
        node->getDisplayString().setTagArg("p", 0, (long int)(entry->getX() * 5));
        node->getDisplayString().setTagArg("p", 1, (long int)(entry->getY() * 5));
        node->getDisplayString().insertTag("t", 0);
        node->getDisplayString().setTagArg("t", 0, addr.str().c_str());
        node->getDisplayString().setTagArg("t", 1, "l");
    }

    overlayTerminalCount++;
    numCreated++;

    churnGenerator[type.typeID]->terminalCount++;

    TransportAddress *address = new TransportAddress(addr);

    // update display
    setDisplayString();

    return address;
}

uint32_t SimpleUnderlayConfigurator::parseCoordFile(const char* nodeCoordinateSource)
{
    cXMLElement* rootElement = ev.getXMLDocument(nodeCoordinateSource);

    // get number of dimensions from attribute of xml rootelement
    dimensions = atoi(rootElement->getAttribute("dimensions"));
    NodeRecord::setDim(dimensions);
    EV << "[SimpleNetConfigurator::parseCoordFile()]\n"
       << "    using " << dimensions << " dimensions: ";

    double max_coord = 0;

    for (cXMLElement *tmpElement = rootElement->getFirstChild(); tmpElement;
         tmpElement = tmpElement->getNextSibling()) {

        // get "ip" and "isRoot" from Attributes (not needed yet)
      /*
       const char* str_ip = tmpElement->getAttribute("ip");
       int tmpIP = 0;
       if (str_ip) sscanf(str_ip, "%x", &tmpIP);
       bool tmpIsRoot = atoi(tmpElement->getAttribute("isroot"));
       */

        // create tmpNode to be added to vector
        NodeRecord* tmpNode = new NodeRecord;

        // get coords from childEntries and fill tmpNodes Array
        int i = 0;
        for (cXMLElement *coord = tmpElement->getFirstChild(); coord;
             coord = coord->getNextSibling()) {

            tmpNode->coords[i] = atof(coord->getNodeValue());

            double newMax = fabs(tmpNode->coords[i]);
            if (newMax > max_coord) {
               max_coord = newMax;
            }
            i++;
        }

        // add to vector
        nodeRecordPool.push_back(make_pair(tmpNode, true));

        //if (nodeRecordPool.size() >= maxSize) break; //TODO use other xml lib
    }

    EV << nodeRecordPool.size()
       << " nodes added to vector \"nodeRecordPool\"." << endl;

#if OMNETPP_VERSION>=0x0401
    // free memory used for xml document
    ev.forgetXMLDocument(nodeCoordinateSource);
#if !defined(__APPLE__) &&  !defined(_WIN32)
    malloc_trim(0);
#endif    
#endif
    
    return (uint32_t)ceil(max_coord);
}

void SimpleUnderlayConfigurator::preKillNode(NodeType type, TransportAddress* addr)
{
    Enter_Method_Silent();

    SimpleNodeEntry* entry = NULL;
    SimpleInfo* info;

    if (addr == NULL) {
        addr = globalNodeList->getRandomAliveNode(type.typeID);

        if (addr == NULL) {
            // all nodes are already prekilled
            std::cout << "all nodes are already prekilled" << std::endl;
            return;
        }
    }

    info = dynamic_cast<SimpleInfo*> (globalNodeList->getPeerInfo(*addr));

    if (info != NULL) {
        entry = info->getEntry();
        globalNodeList->setPreKilled(*addr);
    } else {
        opp_error("SimpleNetConfigurator: Trying to pre kill node "
                  "with nonexistant TransportAddress!");
    }

    uint32_t effectiveType = info->getTypeID();
    cGate* gate = entry->getUdpIPv4Gate();

    cModule* node = gate->getOwnerModule()->getParentModule();

    if (scheduledID.count(node->getId())) {
        std::cout << "SchedID" << std::endl;
        return;
    }

    // remove node from bootstrap oracle
    globalNodeList->removePeer(IPvXAddressResolver().addressOf(node));

    // put node into the kill list and schedule a message for final removal
    // of the node
    killList.push_front(IPvXAddressResolver().addressOf(node));
    scheduledID.insert(node->getId());

    overlayTerminalCount--;
    numKilled++;

    churnGenerator[effectiveType]->terminalCount--;

    // update display
    setDisplayString();

    // inform the notification board about the removal
    NotificationBoard* nb = check_and_cast<NotificationBoard*> (
                             node-> getSubmodule("notificationBoard"));
    nb->fireChangeNotification(NF_OVERLAY_NODE_LEAVE);

    double random = uniform(0, 1);
    if (random < gracefulLeaveProbability) {
        nb->fireChangeNotification(NF_OVERLAY_NODE_GRACEFUL_LEAVE);
    }

    cMessage* msg = new cMessage();
    scheduleAt(simTime() + gracefulLeaveDelay, msg);
}

void SimpleUnderlayConfigurator::migrateNode(NodeType type, TransportAddress* addr)
{
    Enter_Method_Silent();

    SimpleNodeEntry* entry = NULL;

    if (addr != NULL) {
        SimpleInfo* info =
              dynamic_cast<SimpleInfo*> (globalNodeList->getPeerInfo(*addr));
        if (info != NULL) {
            entry = info->getEntry();
        } else {
            opp_error("SimpleNetConfigurator: Trying to migrate node with "
                      "nonexistant TransportAddress!");
        }
    } else {
        SimpleInfo* info = dynamic_cast<SimpleInfo*> (
                globalNodeList-> getRandomPeerInfo(type.typeID));
        entry = info->getEntry();
    }

    cGate* gate = entry->getUdpIPv4Gate();
    cModule* node = gate->getOwnerModule()->getParentModule();

    // do not migrate node that is already scheduled
    if (scheduledID.count(node->getId()))
        return;

    //std::cout << "migration @ " << tmp_ip << " --> " << address << std::endl;

    // FIXME use only IPv4?
    IPvXAddress address = IPv4Address(nextFreeAddress++);

    IPvXAddress tmp_ip = IPvXAddressResolver().addressOf(node);
    SimpleNodeEntry* newentry;

    int chanIndex = intuniform(0, type.channelTypesRx.size() - 1);
    cChannelType* rxChan = cChannelType::find(type.channelTypesRx[chanIndex].c_str());
    cChannelType* txChan = cChannelType::find(type.channelTypesTx[chanIndex].c_str());

    if (!txChan || !rxChan)
         opp_error("Could not find Channel Type. Most likely parameter "
            "channelTypesRx or channelTypes does not match the channels defined in "
             "channels.ned");

    if (useXmlCoords) {
       newentry = new SimpleNodeEntry(node,
                                      rxChan,
                                      txChan,
                                      sendQueueLength,
                                      entry->getNodeRecord(), entry->getRecordIndex());
        //newentry->getNodeRecord()->ip = address;
    } else {
        newentry = new SimpleNodeEntry(node, rxChan, txChan, fieldSize, sendQueueLength);
    }

    node->bubble("I am migrating!");

    //remove node from bootstrap oracle
    globalNodeList->killPeer(tmp_ip);

    SimpleUDP* simple = check_and_cast<SimpleUDP*> (gate->getOwnerModule());
    simple->setNodeEntry(newentry);

    InterfaceEntry* ie = IPvXAddressResolver().interfaceTableOf(node)->
                                      getInterfaceByName("dummy interface");
    delete ie->ipv4Data();

    // Add pseudo-Interface to node's interfaceTable
    IPv4InterfaceData* ifdata = new IPv4InterfaceData;
    ifdata->setIPAddress(address.get4());
    ifdata->setNetmask(IPv4Address("255.255.255.255"));
    ie->setIPv4Data(ifdata);

    // create meta information
    SimpleInfo* newinfo = new SimpleInfo(type.typeID, node->getId(),
                                         type.context);
    newinfo->setEntry(newentry);

    //add node to bootstrap oracle
    globalNodeList->addPeer(address, newinfo);

    // inform the notification board about the migration
    NotificationBoard* nb = check_and_cast<NotificationBoard*> (
                                      node->getSubmodule("notificationBoard"));
    nb->fireChangeNotification(NF_OVERLAY_TRANSPORTADDRESS_CHANGED);
}

void SimpleUnderlayConfigurator::handleTimerEvent(cMessage* msg)
{
    Enter_Method_Silent();

    // get next scheduled node and remove it from the kill list
    IPvXAddress addr = killList.back();
    killList.pop_back();

    SimpleNodeEntry* entry = NULL;

    SimpleInfo* info =
            dynamic_cast<SimpleInfo*> (globalNodeList->getPeerInfo(addr));

    if (info != NULL) {
        entry = info->getEntry();
    } else {
        throw cRuntimeError("SimpleUnderlayConfigurator: Trying to kill "
                            "node with unknown TransportAddress!");
    }

    cGate* gate = entry->getUdpIPv4Gate();
    cModule* node = gate->getOwnerModule()->getParentModule();

    if (useXmlCoords) {
        nodeRecordPool[entry->getRecordIndex()].second = true;
    }

    scheduledID.erase(node->getId());
    globalNodeList->killPeer(addr);

    InterfaceEntry* ie = IPvXAddressResolver().interfaceTableOf(node)->
                                         getInterfaceByName("dummy interface");
    delete ie->ipv4Data();

    node->callFinish();
    node->deleteModule();

    delete msg;
}

void SimpleUnderlayConfigurator::setDisplayString()
{
    // Updates the statistics display string.
    char buf[80];
    sprintf(buf, "%i overlay terminals", overlayTerminalCount);
    getDisplayString().setTagArg("t", 0, buf);
}

void SimpleUnderlayConfigurator::finishUnderlay()
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
