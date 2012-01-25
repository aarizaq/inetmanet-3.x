//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file BaseOverlay.cc
 * @author Ingmar Baumgart
 * @author Bernhard Heep
 * @author Stephan Krause
 * @author Sebastian Mies
 */


#include <cassert>

#include <RpcMacros.h>

#include <IPvXAddressResolver.h>
#include <NotificationBoard.h>

#include <GlobalNodeListAccess.h>
#include <UnderlayConfiguratorAccess.h>
#include <GlobalStatisticsAccess.h>
#include <GlobalParametersAccess.h>

#include <LookupListener.h>
#include <RecursiveLookup.h>
#include <IterativeLookup.h>

#include <BootstrapList.h>

#include "BaseOverlay.h"
#include "UDPControlInfo_m.h"

using namespace std;


//------------------------------------------------------------------------
//--- Initialization & finishing -----------------------------------------
//------------------------------------------------------------------------

BaseOverlay::BaseOverlay()
{
    globalNodeList = NULL;
    underlayConfigurator = NULL;
    notificationBoard = NULL;
    globalParameters = NULL;
    bootstrapList = NULL;
}

BaseOverlay::~BaseOverlay()
{
    finishLookups();
    finishRpcs();
}

int BaseOverlay::numInitStages() const
{
    return NUM_STAGES_ALL;
}

void BaseOverlay::initialize(int stage)
{
    if (stage == 0) {
        OverlayKey::setKeyLength(par("keyLength").longValue());
    }

    if (stage == REGISTER_STAGE) {
        registerComp(getThisCompType(), this);
        return;
    }

    if (stage == MIN_STAGE_OVERLAY) {
        // find friend modules
        globalNodeList = GlobalNodeListAccess().get();
        underlayConfigurator = UnderlayConfiguratorAccess().get();
        notificationBoard = NotificationBoardAccess().get();
        globalParameters = GlobalParametersAccess().get();
        bootstrapList = check_and_cast<BootstrapList*>(getParentModule()->
                getParentModule()->getSubmodule("bootstrapList", 0));

        udpGate = gate("udpIn");
        appGate = gate("appIn");

        // fetch some parameters
        debugOutput = par("debugOutput");
        collectPerHopDelay = par("collectPerHopDelay");
        localPort = par("localPort");
        hopCountMax = par("hopCountMax");
        drawOverlayTopology = par("drawOverlayTopology");
        rejoinOnFailure = par("rejoinOnFailure");
        sendRpcResponseToLastHop = par("sendRpcResponseToLastHop");
        dropFindNodeAttack = par("dropFindNodeAttack");
        isSiblingAttack = par("isSiblingAttack");
        invalidNodesAttack = par("invalidNodesAttack");
        dropRouteMessageAttack = par("dropRouteMessageAttack");
        measureAuthBlock = par("measureAuthBlock");
        restoreContext = par("restoreContext");

        // we assume most overlays don't provide KBR services
        kbr = false;

        // set routing type
        std::string temp = par("routingType").stdstringValue();
        if (temp == "iterative")
            defaultRoutingType = ITERATIVE_ROUTING;
        else if (temp == "exhaustive-iterative")
            defaultRoutingType = EXHAUSTIVE_ITERATIVE_ROUTING;
        else if (temp == "semi-recursive")
            defaultRoutingType = SEMI_RECURSIVE_ROUTING;
        else if (temp == "full-recursive")
            defaultRoutingType = FULL_RECURSIVE_ROUTING;
        else if (temp == "source-routing-recursive")
            defaultRoutingType = RECURSIVE_SOURCE_ROUTING;
        else throw cRuntimeError((std::string("Wrong routing type: ")
                                      + temp).c_str());

        useCommonAPIforward = par("useCommonAPIforward");
        routeMsgAcks = par("routeMsgAcks");
        recNumRedundantNodes = par("recNumRedundantNodes");
        recordRoute = par("recordRoute");

        // set base lookup parameters
        iterativeLookupConfig.redundantNodes = par("lookupRedundantNodes");
        iterativeLookupConfig.parallelPaths = par("lookupParallelPaths");
        iterativeLookupConfig.parallelRpcs = par("lookupParallelRpcs");
        iterativeLookupConfig.verifySiblings = par("lookupVerifySiblings");
        iterativeLookupConfig.majoritySiblings = par("lookupMajoritySiblings");
        iterativeLookupConfig.merge = par("lookupMerge");
        iterativeLookupConfig.failedNodeRpcs = par("lookupFailedNodeRpcs");
        iterativeLookupConfig.strictParallelRpcs =
            par("lookupStrictParallelRpcs");
        iterativeLookupConfig.useAllParallelResponses =
            par("lookupUseAllParallelResponses");
        iterativeLookupConfig.newRpcOnEveryTimeout =
            par("lookupNewRpcOnEveryTimeout");
        iterativeLookupConfig.newRpcOnEveryResponse =
            par("lookupNewRpcOnEveryResponse");
        iterativeLookupConfig.finishOnFirstUnchanged =
            par("lookupFinishOnFirstUnchanged");
        iterativeLookupConfig.visitOnlyOnce =
            par("lookupVisitOnlyOnce");
        iterativeLookupConfig.acceptLateSiblings =
            par("lookupAcceptLateSiblings");

        recursiveLookupConfig.redundantNodes = par("lookupRedundantNodes");
        recursiveLookupConfig.numRetries = 0; //TODO

        // statistics
        numAppDataSent = 0;
        bytesAppDataSent = 0;
        numAppLookupSent = 0;
        bytesAppLookupSent = 0;
        numMaintenanceSent = 0;
        bytesMaintenanceSent = 0;
        numAppDataReceived = 0;
        bytesAppDataReceived = 0;
        numAppLookupReceived = 0;
        bytesAppLookupReceived = 0;
        numMaintenanceReceived = 0;
        bytesMaintenanceReceived = 0;
        numAppDataForwarded = 0;
        bytesAppDataForwarded = 0;
        numAppLookupForwarded = 0;
        bytesAppLookupForwarded = 0;
        numMaintenanceForwarded = 0;
        bytesMaintenanceForwarded = 0;
        bytesAuthBlockSent = 0;

        numDropped = 0;
        bytesDropped = 0;
        numFindNodeSent = 0;
        bytesFindNodeSent = 0;
        numFindNodeResponseSent = 0;
        bytesFindNodeResponseSent = 0;
        numFailedNodeSent = 0;
        bytesFailedNodeSent = 0;
        numFailedNodeResponseSent = 0;
        bytesFailedNodeResponseSent = 0;

        joinRetries = 0;

        numInternalSent = 0;
        bytesInternalSent = 0;
        numInternalReceived = 0;
        bytesInternalReceived = 0;

        WATCH(numAppDataSent);
        WATCH(bytesAppDataSent);
        WATCH(numAppLookupSent);
        WATCH(bytesAppLookupSent);
        WATCH(numMaintenanceSent);
        WATCH(bytesMaintenanceSent);
        WATCH(numAppDataReceived);
        WATCH(bytesAppDataReceived);
        WATCH(numAppLookupReceived);
        WATCH(bytesAppLookupReceived);
        WATCH(numMaintenanceReceived);
        WATCH(bytesMaintenanceReceived);
        WATCH(numAppDataForwarded);
        WATCH(bytesAppDataForwarded);
        WATCH(numAppLookupForwarded);
        WATCH(bytesAppLookupForwarded);
        WATCH(numMaintenanceForwarded);
        WATCH(bytesMaintenanceForwarded);

        WATCH(numDropped);
        WATCH(bytesDropped);
        WATCH(numFindNodeSent);
        WATCH(bytesFindNodeSent);
        WATCH(numFindNodeResponseSent);
        WATCH(bytesFindNodeResponseSent);
        WATCH(numFailedNodeSent);
        WATCH(bytesFailedNodeSent);
        WATCH(numFailedNodeResponseSent);
        WATCH(bytesFailedNodeResponseSent);

        WATCH(joinRetries);

        if (isInSimpleMultiOverlayHost()) {
            WATCH(numInternalSent);
            WATCH(bytesInternalSent);
            WATCH(numInternalReceived);
            WATCH(bytesInternalReceived);
        }

        // set up local nodehandle
        thisNode.setIp(IPvXAddressResolver().
                      addressOf(getParentModule()->getParentModule()));
        thisNode.setKey(OverlayKey::UNSPECIFIED_KEY);

        state = INIT;
        internalReadyState = false;

        getDisplayString().setTagArg("i", 1, "red");
        globalNodeList->setOverlayReadyIcon(getThisNode(), false);

        // set up UDP
        bindToPort(localPort);

        // subscribe to the notification board
        notificationBoard->subscribe(this, NF_OVERLAY_TRANSPORTADDRESS_CHANGED);
        notificationBoard->subscribe(this, NF_OVERLAY_NODE_LEAVE);
        notificationBoard->subscribe(this, NF_OVERLAY_NODE_GRACEFUL_LEAVE);

        // init visualization with terminal ptr
        if (drawOverlayTopology)
            initVis(getParentModule()->getParentModule());

        // init rpcs
        initRpcs();
        initLookups();

        // set TCP output gate
        setTcpOut(gate("tcpOut"));

        // statistics
        creationTime = simTime();
        WATCH(creationTime);
    }

    if (stage >= MIN_STAGE_OVERLAY && stage <= MAX_STAGE_OVERLAY)
        initializeOverlay(stage);

    if (stage == MAX_STAGE_TIER_1) {
        // bootstrapList registered its gate to the overlay 0
        // if this is not overlay 0, we may not have the gate, so retrieve it
        // this assumes that the overlay is in a container module!
        if (!compModuleList.count(BOOTSTRAPLIST_COMP)) {
            BaseOverlay *firstOverlay = dynamic_cast<BaseOverlay*>
                    (getParentModule()->getParentModule()
                    ->getSubmodule("overlay", 0)->gate("appIn")
                    ->getNextGate()->getOwnerModule());
            if (!firstOverlay) {
                throw cRuntimeError("BaseOverlay.cc: "
                                    "Couldn't obtain bootstrap gate");
            }
            registerComp(BOOTSTRAPLIST_COMP,
                         firstOverlay->getCompModule(BOOTSTRAPLIST_COMP));
        }
    }
}


void BaseOverlay::initializeOverlay(int stage)
{
}

void BaseOverlay::finish()
{
    finishOverlay();

    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);

    if (time >= GlobalStatistics::MIN_MEASURED) {

        if (collectPerHopDelay) {
            std::ostringstream singleHopName;
            HopDelayRecord* hdrl = NULL;
            HopDelayRecord* hdr = NULL;
            for (size_t i = 0; i < singleHopDelays.size();) {
                hdrl = singleHopDelays[i++];
                hdr = hdrl;
                for (size_t j = 1; j <= i; ++j) {
                    if (hdr->count == 0) continue;
                    singleHopName.str("");
                    singleHopName << "BaseOverlay: Average Delay in Hop "
                                  << j << " of " << i;
                    globalStatistics->addStdDev(singleHopName.str(),
                                          SIMTIME_DBL(hdr->val / hdr->count));
                    ++hdr;
                }
                delete[] hdrl;
            }
            singleHopDelays.clear();
        }

        globalStatistics->addStdDev("BaseOverlay: Join Retries", joinRetries);

        globalStatistics->addStdDev("BaseOverlay: Sent App Data Messages/s",
                                    numAppDataSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent App Data Bytes/s",
                                    bytesAppDataSent / time);
        if (isInSimpleMultiOverlayHost()) {
            globalStatistics->addStdDev("BaseOverlay: Internal Sent Messages/s",
                                        numInternalReceived / time);
            globalStatistics->addStdDev("BaseOverlay: Internal Sent Bytes/s",
                                        bytesInternalReceived / time);
        }
        globalStatistics->addStdDev("BaseOverlay: Sent App Lookup Messages/s",
                                    numAppLookupSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent App Lookup Bytes/s",
                                    bytesAppLookupSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent Maintenance Messages/s",
                                    numMaintenanceSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent Maintenance Bytes/s",
                                    bytesMaintenanceSent / time);

        globalStatistics->addStdDev("BaseOverlay: Sent Total Messages/s",
                                    (numAppDataSent + numAppLookupSent +
                                        numMaintenanceSent) / time);
        globalStatistics->addStdDev("BaseOverlay: Sent Total Bytes/s",
                                    (bytesAppDataSent + bytesAppLookupSent +
                                            bytesMaintenanceSent) / time);
        globalStatistics->addStdDev("BaseOverlay: Sent FindNode Messages/s",
                                    numFindNodeSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent FindNode Bytes/s",
                                    bytesFindNodeSent / time);

        globalStatistics->addStdDev("BaseOverlay: Sent FindNodeResponse Messages/s",
                                    numFindNodeResponseSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent FindNodeResponse Bytes/s",
                                    bytesFindNodeResponseSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent FailedNode Messages/s",
                                    numFailedNodeSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent FailedNode Bytes/s",
                                    bytesFailedNodeSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent FailedNodeResponse Messages/s",
                                    numFailedNodeResponseSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent FailedNodeResponse Bytes/s",
                                    bytesFailedNodeResponseSent / time);
        globalStatistics->addStdDev("BaseOverlay: Received App Data Messages/s",
                                    numAppDataReceived / time);
        globalStatistics->addStdDev("BaseOverlay: Received App Data Bytes/s",
                                    bytesAppDataReceived / time);
        if (isInSimpleMultiOverlayHost()) {
            globalStatistics->addStdDev("BaseOverlay: Internal Received Messages/s",
                                        numInternalReceived / time);
            globalStatistics->addStdDev("BaseOverlay: Internal Received Bytes/s",
                                        bytesInternalReceived / time);
        }
        globalStatistics->addStdDev("BaseOverlay: Received App Lookup Messages/s",
                                    numAppLookupReceived / time);
        globalStatistics->addStdDev("BaseOverlay: Received App Lookup Bytes/s",
                                    bytesAppLookupReceived / time);
        globalStatistics->addStdDev("BaseOverlay: Received Maintenance Messages/s",
                                    numMaintenanceReceived / time);
        globalStatistics->addStdDev("BaseOverlay: Received Maintenance Bytes/s",
                                    bytesMaintenanceReceived / time);

        globalStatistics->addStdDev("BaseOverlay: Received Total Messages/s",
                                    (numAppDataReceived + numAppLookupReceived +
                                            numMaintenanceReceived)/time);
        globalStatistics->addStdDev("BaseOverlay: Received Total Bytes/s",
                                    (bytesAppDataReceived + bytesAppLookupReceived +
                                            bytesMaintenanceReceived)/time);
        globalStatistics->addStdDev("BaseOverlay: Forwarded App Data Messages/s",
                                    numAppDataForwarded / time);
        globalStatistics->addStdDev("BaseOverlay: Forwarded App Data Bytes/s",
                                    bytesAppDataForwarded / time);
        globalStatistics->addStdDev("BaseOverlay: Forwarded App Lookup Messages/s",
                                    numAppLookupForwarded / time);
        globalStatistics->addStdDev("BaseOverlay: Forwarded App Lookup Bytes/s",
                                    bytesAppLookupForwarded / time);
        globalStatistics->addStdDev("BaseOverlay: Forwarded Maintenance Messages/s",
                                    numMaintenanceForwarded / time);
        globalStatistics->addStdDev("BaseOverlay: Forwarded Maintenance Bytes/s",
                                    bytesMaintenanceForwarded / time);
        globalStatistics->addStdDev("BaseOverlay: Forwarded Total Messages/s",
                                    (numAppDataForwarded + numAppLookupForwarded +
                                            numMaintenanceForwarded) / time);
        globalStatistics->addStdDev("BaseOverlay: Forwarded Total Bytes/s",
                                    (bytesAppDataForwarded + bytesAppLookupForwarded +
                                            bytesMaintenanceForwarded) / time);

        globalStatistics->addStdDev("BaseOverlay: Dropped Messages/s",
                                    numDropped / time);
        globalStatistics->addStdDev("BaseOverlay: Dropped Bytes/s",
                                    bytesDropped / time);

        globalStatistics->addStdDev("BaseOverlay: Measured Session Time",
                                    SIMTIME_DBL(simTime() - creationTime));

        globalStatistics->addStdDev("BaseOverlay: Sent Ping Messages/s",
                                    numPingSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent Ping Bytes/s",
                                    bytesPingSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent Ping Response Messages/s",
                                    numPingResponseSent / time);
        globalStatistics->addStdDev("BaseOverlay: Sent Ping Response Bytes/s",
                                    bytesPingResponseSent / time);

        if (getMeasureAuthBlock()) {
            globalStatistics->addStdDev("BaseOverlay: Sent AuthBlock Bytes/s",
                                        bytesAuthBlockSent / time);
        }
    }
}

void BaseOverlay::finishOverlay()
{
}

//------------------------------------------------------------------------
//--- General Overlay Parameters (getter and setters) --------------------
//------------------------------------------------------------------------
bool BaseOverlay::isMalicious()
{
    return globalNodeList->isMalicious(getThisNode());
}

CompType BaseOverlay::getThisCompType()
{
    return OVERLAY_COMP;
}

//------------------------------------------------------------------------
//--- UDP functions copied from the INET framework .----------------------
//------------------------------------------------------------------------
void BaseOverlay::bindToPort(int port)
{
    EV << "[BaseOverlay::bindToPort() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]\n"
       << "    Binding to UDP port " << port
       << endl;

    thisNode.setPort(port);

    // TODO UDPAppBase should be ported to use UDPSocket sometime, but for now
    // we just manage the UDP socket by hand...
    socket.setOutputGate(gate("udpOut"));
    socket.bind(port);
}


//------------------------------------------------------------------------
//--- Overlay Common API: Key-based Routing ------------------------------
//------------------------------------------------------------------------

void BaseOverlay::callDeliver(BaseOverlayMessage* msg,
                              const OverlayKey& destKey)
{
    KBRdeliver* deliverMsg = new KBRdeliver();

    OverlayCtrlInfo* overlayCtrlInfo =
        check_and_cast<OverlayCtrlInfo*>(msg->removeControlInfo());

    BaseAppDataMessage* appDataMsg = dynamic_cast<BaseAppDataMessage*>(msg);

    // TODO GIA
    if (appDataMsg != NULL) {
        overlayCtrlInfo->setSrcComp(appDataMsg->getSrcComp());
        overlayCtrlInfo->setDestComp(appDataMsg->getDestComp());
    }

    deliverMsg->setControlInfo(overlayCtrlInfo);
    deliverMsg->setDestKey(destKey);
    deliverMsg->encapsulate(msg->decapsulate());
    deliverMsg->setType(KBR_DELIVER);

    cGate* destGate = getCompRpcGate(static_cast<CompType>(
            overlayCtrlInfo->getDestComp()));

    if (destGate == NULL) {
        throw cRuntimeError("BaseOverlay::callDeliver(): Unknown destComp!");
    }

    sendDirect(deliverMsg, destGate);

    delete msg;
}

void BaseOverlay::callForward(const OverlayKey& key, BaseRouteMessage* msg,
                              const NodeHandle& nextHopNode)
{
    KBRforward* forwardMsg = new KBRforward();

    forwardMsg->setDestKey(msg->getDestKey());
    forwardMsg->setNextHopNode(nextHopNode);
    forwardMsg->encapsulate(msg->getEncapsulatedPacket()->decapsulate());

    OverlayCtrlInfo* overlayCtrlInfo =
        new OverlayCtrlInfo();
    overlayCtrlInfo->setTransportType(ROUTE_TRANSPORT);
    overlayCtrlInfo->setRoutingType(msg->getRoutingType());
    overlayCtrlInfo->setHopCount(msg->getHopCount());
    overlayCtrlInfo->setSrcNode(msg->getSrcNode());
    overlayCtrlInfo->setSrcComp(check_and_cast<BaseAppDataMessage*>
        (msg->getEncapsulatedPacket())->getSrcComp());
    overlayCtrlInfo->setDestComp(check_and_cast<BaseAppDataMessage*>
        (msg->getEncapsulatedPacket())->getDestComp());

    if (msg->getControlInfo() != NULL) {
        OverlayCtrlInfo* ctrlInfo =
            check_and_cast<OverlayCtrlInfo*>(msg->removeControlInfo());

        overlayCtrlInfo->setLastHop(ctrlInfo->getLastHop());

        delete ctrlInfo;
    }

    forwardMsg->setControlInfo(overlayCtrlInfo);

    forwardMsg->setType(KBR_FORWARD);

    send(forwardMsg, "appOut");

    delete msg;
}

NodeVector* BaseOverlay::local_lookup(const OverlayKey& key,
                                      int num, bool safe)
{
    Enter_Method("local_lookup()");

    if (safe == true) {
        throw cRuntimeError("BaseOverlay::local_lookup(): "
                            "safe flag is not implemented!");
    }

    if (num < 0) num = INT_MAX;
    NodeVector* nodeVector = findNode(key, min(num, getMaxNumRedundantNodes()),
                                      min(num,getMaxNumSiblings()));

    if (((int)nodeVector->size()) > num)
        nodeVector->resize(num);

    return nodeVector;
}

void BaseOverlay::join(const OverlayKey& nodeID)
{
    Enter_Method("join()");

    joinRetries++;

    if (((state == READY) || (state == FAILED)) && !rejoinOnFailure) {
        state = FAILED;
        return;
    }

    if (state != READY) {
        // set nodeID and IP
        thisNode.setIp(
                IPvXAddressResolver().addressOf(getParentModule()->getParentModule()));

        if (!nodeID.isUnspecified())  {
            thisNode.setKey(nodeID);
        } else if (thisNode.getKey().isUnspecified()) {
            std::string nodeIdStr = par("nodeId").stdstringValue();

            if (nodeIdStr.size()) {
                // manual configuration of nodeId in ini file
                thisNode.setKey(OverlayKey(nodeIdStr));
            } else {
                setOwnNodeID();
            }
        }
    }

    cObject** context = globalNodeList->getContext(getThisNode());
    if (restoreContext && context) {
        if (*context == NULL) {
            *context = new BaseOverlayContext(getThisNode().getKey(),
                                              isMalicious());
        }
    }

    joinOverlay();
}

void BaseOverlay::joinForeignPartition(const NodeHandle& node)
{
    throw cRuntimeError("BaseOverlay::joinForeignPartition(): "
                        "This overlay doesn't support merging!");
}

void BaseOverlay::setOwnNodeID()
{
    thisNode.setKey(OverlayKey::random());
}

NodeVector* BaseOverlay::neighborSet(int num)
{
    Enter_Method("neighborSet()");

    return local_lookup(thisNode.getKey(), num, false);
}

void BaseOverlay::callUpdate(const NodeHandle& node, bool joined)
{
    if ((!node.isUnspecified()) && (node != thisNode)) {
        if (joined) {
            EV << "[BaseOverlay::callUpdate() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    (" << node << ", " << joined << ") joined"
               << endl;
        } else {
            EV << "[BaseOverlay::callUpdate() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    (" << node << ", " << joined << ") left"
               << endl;
        }
    }

    KBRupdate* updateMsg = new KBRupdate("UPDATE");

    updateMsg->setNode(node);
    updateMsg->setJoined(joined);

    updateMsg->setType(KBR_UPDATE);

    send(updateMsg, "appOut");
}

bool BaseOverlay::isSiblingFor(const NodeHandle& node, const OverlayKey& key,
                               int numSiblings, bool* err)
{
    Enter_Method("isSiblingFor()");

    throw cRuntimeError("isSiblingFor: Not implemented!");

    return false;
}

int BaseOverlay::getMaxNumSiblings()
{
    Enter_Method("getMaxNumSiblings()");

    throw cRuntimeError("getMaxNumSiblings: Not implemented!");

    return false;
}

int BaseOverlay::getMaxNumRedundantNodes()
{
    Enter_Method("getMaxNumRedundantNodes()");

    throw cRuntimeError("getMaxNumRedundantNodes: Not implemented!");

    return false;
}


//------------------------------------------------------------------------
//--- Message Handlers ---------------------------------------------------
//------------------------------------------------------------------------

//private
void BaseOverlay::handleMessage(cMessage* msg)
{
    if (msg->getArrivalGate() == udpGate) {
        UDPDataIndication* udpControlInfo =
            check_and_cast<UDPDataIndication*>(msg->removeControlInfo());
        OverlayCtrlInfo* overlayCtrlInfo = new OverlayCtrlInfo;
        overlayCtrlInfo->setLastHop(TransportAddress(
                                        udpControlInfo->getSrcAddr(),
                                        udpControlInfo->getSrcPort()));
        overlayCtrlInfo->setSrcRoute(overlayCtrlInfo->getLastHop());
        overlayCtrlInfo->setTransportType(UDP_TRANSPORT);

        msg->setControlInfo(overlayCtrlInfo);
        delete udpControlInfo;

        // debug message
        if (debugOutput) {
            EV << "[BaseOverlay:handleMessage() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received " << *msg << " from "
            << overlayCtrlInfo->getLastHop().getIp() << endl;
        }

        BaseOverlayMessage* baseOverlayMsg =
            dynamic_cast<BaseOverlayMessage*>(msg);

        if (baseOverlayMsg == NULL) {
            cPacket* packet = check_and_cast<cPacket*>(msg);
            RECORD_STATS(numDropped++; bytesDropped += packet->getByteLength());
            delete msg;
            return;
        }

        // records stats if message is not a UDP "self message"
        if (overlayCtrlInfo->getLastHop() != thisNode) {
            // is this from anywhere else?
            if (baseOverlayMsg->getStatType() == APP_DATA_STAT)
                RECORD_STATS(numAppDataReceived++; bytesAppDataReceived +=
                             baseOverlayMsg->getByteLength());
            else if (baseOverlayMsg->getStatType() == APP_LOOKUP_STAT)
                RECORD_STATS(numAppLookupReceived++;bytesAppLookupReceived +=
                             baseOverlayMsg->getByteLength());
            else // MAINTENANCE_STAT
                RECORD_STATS(numMaintenanceReceived++;
                             bytesMaintenanceReceived +=
                                 baseOverlayMsg->getByteLength());
        }
        if (overlayCtrlInfo->getLastHop().getIp() == thisNode.getIp()) {
            // is this from the same node?
            RECORD_STATS(numInternalReceived++; bytesInternalReceived +=
                             baseOverlayMsg->getByteLength());
        } else overlayCtrlInfo->setHopCount(1);

        // process rpc calls/responses or BaseOverlayMessages
        if (!internalHandleMessage(msg)) {
            handleBaseOverlayMessage(baseOverlayMsg);
        }
    }

    // process timer events and rpc timeouts
    else if (internalHandleMessage(msg)) return;

    // process CommonAPIMessages from App
    else if (dynamic_cast<CommonAPIMessage*>(msg) != NULL) {
        if (dynamic_cast<KBRroute*>(msg) != NULL) {
            KBRroute* apiMsg = static_cast<KBRroute*>(msg);

            std::vector<TransportAddress> sourceRoute;
            for (uint32_t i = 0; i < apiMsg->getSourceRouteArraySize(); ++i)
                sourceRoute.push_back(apiMsg->getSourceRoute(i));

            route(apiMsg->getDestKey(), static_cast<CompType>(apiMsg->getDestComp()),
                  static_cast<CompType>(apiMsg->getSrcComp()), apiMsg->decapsulate(),
                          sourceRoute);
        } else if (dynamic_cast<KBRforward*>(msg) != NULL) {
            KBRforward* apiMsg = static_cast<KBRforward*>(msg);
            OverlayCtrlInfo* overlayCtrlInfo =
                check_and_cast<OverlayCtrlInfo*>
                (msg->removeControlInfo());

            BaseAppDataMessage* dataMsg =
                new BaseAppDataMessage();
            dataMsg->setType(APPDATA);
            dataMsg->setBitLength(BASEAPPDATA_L(dataMsg));
            dataMsg->setName(apiMsg->getEncapsulatedPacket()->getName());
            dataMsg->encapsulate(apiMsg->decapsulate());
            dataMsg->setSrcComp(overlayCtrlInfo->getSrcComp());
            dataMsg->setDestComp(overlayCtrlInfo->getDestComp());
            dataMsg->setStatType(APP_DATA_STAT);

            BaseRouteMessage* routeMsg = new BaseRouteMessage(dataMsg->getName());
            routeMsg->setType(OVERLAYROUTE);
            routeMsg->setBitLength(BASEROUTE_L(routeMsg));
            routeMsg->encapsulate(dataMsg);

            routeMsg->setStatType(APP_DATA_STAT);
            routeMsg->setRoutingType(overlayCtrlInfo->getRoutingType());
            routeMsg->setDestKey(apiMsg->getDestKey());
            routeMsg->setSrcNode(overlayCtrlInfo->getSrcNode());
            routeMsg->setHopCount(overlayCtrlInfo->getHopCount());
            routeMsg->setControlInfo(overlayCtrlInfo);

            // message marked with this-pointer as already forwarded to tier1
            routeMsg->setContextPointer(this);

            std::vector<TransportAddress> sourceRoute;
            sourceRoute.push_back(apiMsg->getNextHopNode());
            sendToKey(apiMsg->getDestKey(), routeMsg, 1, sourceRoute);
        }

        delete msg;
    }

    // process other messages from App
    else if (msg->getArrivalGate() == appGate) {
        handleAppMessage(msg);
    } else if(msg->arrivedOn("tcpIn")) {
        handleTCPMessage(msg);
    } else if (dynamic_cast<CompReadyMessage*>(msg)) {
        CompReadyMessage* readyMsg = static_cast<CompReadyMessage*>(msg);
        if (((bool)par("joinOnApplicationRequest") == false) &&
            readyMsg->getReady() &&
            readyMsg->getComp() == NEIGHBORCACHE_COMP) {
            cObject** context = globalNodeList->getContext(getThisNode());
            if (restoreContext && context && *context) {
                BaseOverlayContext* overlayContext = static_cast<BaseOverlayContext*>(*context);
                globalNodeList->setMalicious(getThisNode(),
                                             overlayContext->malicious);
                join(overlayContext->key);
            } else {
                join();
            }
        }
        delete msg;
    } else {
        throw cRuntimeError("BaseOverlay::handleMessage(): Received msg with "
                            "unknown type!");
        delete msg;
    }
}

void BaseOverlay::handleBaseOverlayMessage(BaseOverlayMessage* msg,
                                           const OverlayKey& destKey)
{
    switch (msg->getType()) {
    case OVERLAYSIGNALING:
        handleUDPMessage(msg);
        return;

    case RPC: {
        // process rpc-messages
        BaseRpcMessage* rpcMsg = check_and_cast<BaseRpcMessage*>(msg);

        internalHandleRpcMessage(rpcMsg);
        return;
    }

    case APPDATA: {
        //TODO use route messages? here: set transport type to ROUTE for "naked"
        // app messages
        OverlayCtrlInfo* overlayCtrlInfo = check_and_cast<OverlayCtrlInfo*>(msg->getControlInfo());
        overlayCtrlInfo->setTransportType(ROUTE_TRANSPORT);

        BaseAppDataMessage* baseAppDataMsg =
            check_and_cast<BaseAppDataMessage*>(msg);
        callDeliver(baseAppDataMsg, destKey);
        return;
    }

    case OVERLAYROUTE: {
        BaseRouteMessage* baseRouteMsg =
            check_and_cast<BaseRouteMessage*>(msg);

        // collect delay-value of completed hop
        if (collectPerHopDelay) {
            baseRouteMsg->setHopDelayArraySize(baseRouteMsg->
                                               getHopDelayArraySize() + 1);
            baseRouteMsg->setHopDelay(baseRouteMsg->getHopDelayArraySize() - 1,
                                      simTime() - baseRouteMsg->getHopStamp());
        }

        OverlayCtrlInfo* overlayCtrlInfo
            = check_and_cast<OverlayCtrlInfo*>(baseRouteMsg
                                               ->removeControlInfo());
        // set transport type
        overlayCtrlInfo->setTransportType(ROUTE_TRANSPORT);

        // source routing: save visited nodes, copy next hops
        std::vector<TransportAddress> sourceRoute;
        if ((baseRouteMsg->getNextHopsArraySize() > 0) ||
             (baseRouteMsg->getRoutingType() == RECURSIVE_SOURCE_ROUTING) ||
             recordRoute) {
            // store the TransportAddress of the sender in the visited list
            baseRouteMsg->setVisitedHopsArraySize(baseRouteMsg
                                          ->getVisitedHopsArraySize() + 1);
            baseRouteMsg->setVisitedHops(baseRouteMsg
                                          ->getVisitedHopsArraySize() - 1,
                                        overlayCtrlInfo->getLastHop());

            // remove nodes from next hops and copy them to sourceRoute
            if (baseRouteMsg->getNextHopsArraySize() > 0) {
                sourceRoute.resize(baseRouteMsg->getNextHopsArraySize()- 1);
                for (uint32_t i = 1; i < baseRouteMsg->getNextHopsArraySize();
                     ++i) {
                    sourceRoute[i - 1] = baseRouteMsg->getNextHops(i);
                }
                baseRouteMsg->setNextHopsArraySize(0);
            }
        }

        overlayCtrlInfo->setSrcNode(baseRouteMsg->getSrcNode());

        // decapsulate msg if node is sibling for destKey
        // or message is at its destination node
        bool err;
        if ((sourceRoute.size() == 0) &&
            (baseRouteMsg->getDestKey().isUnspecified() ||
             isSiblingFor(thisNode, baseRouteMsg->getDestKey(), 1, &err)
             /*&& !err*/)) {
            overlayCtrlInfo->setHopCount(baseRouteMsg->getHopCount());
            overlayCtrlInfo->setRoutingType(baseRouteMsg->getRoutingType());

            if (baseRouteMsg->getVisitedHopsArraySize() > 0) {
                // recorded route available => add to srcNode
                NodeHandle srcRoute(baseRouteMsg->getSrcNode().getKey(),
                                   baseRouteMsg->getVisitedHops(0));

                for (uint32_t i = 0; i < baseRouteMsg->getVisitedHopsArraySize(); ++i) {
                    srcRoute.appendSourceRoute(baseRouteMsg->getVisitedHops(i));
                }

                overlayCtrlInfo->setSrcRoute(srcRoute);
            } else if (baseRouteMsg->getDestKey().isUnspecified()) {
                // directly received (neither key routed nor source routed)
                // TODO: does this happen for a BaseRouteMessage?
                overlayCtrlInfo->setSrcRoute(
                        NodeHandle(baseRouteMsg->getSrcNode().getKey(),
                                   overlayCtrlInfo->getLastHop()));
            } else {
                // route to key and no recorded route available
                overlayCtrlInfo->setSrcRoute(baseRouteMsg->getSrcNode());
            }

            // copy visited nodes to control info
            overlayCtrlInfo->setVisitedHopsArraySize(
                    baseRouteMsg->getVisitedHopsArraySize());

            for (uint32_t i = 0; i < baseRouteMsg->getVisitedHopsArraySize();
                 ++i) {
                overlayCtrlInfo->setVisitedHops(i,
                        baseRouteMsg->getVisitedHops(i));
            }

            BaseOverlayMessage* tmpMsg
                = check_and_cast<BaseOverlayMessage*>(baseRouteMsg
                                                      ->decapsulate());
            tmpMsg->setControlInfo(overlayCtrlInfo);

            // delay between hops
            if (collectPerHopDelay) {
                RECORD_STATS(
                    size_t i;
                    for (i = singleHopDelays.size();
                             i < baseRouteMsg->getHopDelayArraySize();) {
                        singleHopDelays.push_back(new HopDelayRecord[++i]);
                    }

                    i = baseRouteMsg->getHopDelayArraySize() - 1;
                    HopDelayRecord* hdr = singleHopDelays[i];

                    for (size_t j = 0; j <= i; ++j) {
                        hdr[j].count++;
                        hdr[j].val += baseRouteMsg->getHopDelay(j);
                    }
                );
            }

            // handle encapsulated message at destination node
            if (((baseRouteMsg->getRoutingType() == ITERATIVE_ROUTING)
                    || (baseRouteMsg->getRoutingType() == EXHAUSTIVE_ITERATIVE_ROUTING)
            )
                    || recursiveRoutingHook(thisNode, baseRouteMsg)) {
                handleBaseOverlayMessage(tmpMsg, baseRouteMsg->getDestKey());
                delete baseRouteMsg;
            }
            return;
        } else {
            // forward msg if this node is not responsible for the key
            baseRouteMsg->setControlInfo(overlayCtrlInfo);

            // if this node is malicious drop the message
            if (isMalicious() && dropRouteMessageAttack) {
                EV << "[BaseOverlay::handleBaseOverlayMessage() @ " << thisNode.getIp()
                << " (" << thisNode.getKey().toString(16) << ")]\n"
                << "    BaseRouteMessage gets dropped because this node is malicious"
                << endl;
                //std::cout << "malicious!" << std::endl;
                RECORD_STATS(numDropped++;
                             bytesDropped += baseRouteMsg->getByteLength());
                delete baseRouteMsg;
                return;
            }

            sendToKey(baseRouteMsg->getDestKey(), baseRouteMsg, 1, sourceRoute);
            return;
        }
        break;
    }

    default:
        EV << "[BaseOverlay::handleBaseOverlayMessage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Received unknown message from UDP of type " << msg->getName()
           << endl;
        break;
    }
}

void BaseOverlay::receiveChangeNotification(int category, const cPolymorphic * details)
{
    Enter_Method_Silent();
    if (category == NF_OVERLAY_TRANSPORTADDRESS_CHANGED) {
        handleTransportAddressChangedNotification();
    } else if (category == NF_OVERLAY_NODE_LEAVE) {
        handleNodeLeaveNotification();
    } else if (category == NF_OVERLAY_NODE_GRACEFUL_LEAVE) {
        handleNodeGracefulLeaveNotification();
    }
}

void BaseOverlay::handleTransportAddressChangedNotification()
{
    // get new ip address
    thisNode.setIp(IPvXAddressResolver().addressOf(
                      getParentModule()->getParentModule()));

    joinOverlay();
}

void BaseOverlay::handleNodeLeaveNotification()
{
    // ...
}

void BaseOverlay::handleNodeGracefulLeaveNotification()
{
    // ...
}

//virtual protected
void BaseOverlay::handleAppMessage(cMessage* msg)
{
    delete msg;
}

void BaseOverlay::handleUDPMessage(BaseOverlayMessage* msg)
{
    delete msg;
}

//virtual protected
void BaseOverlay::recordOverlaySentStats(BaseOverlayMessage* msg)
{
    // collect statistics ...
}

void BaseOverlay::setOverlayReady(bool ready)
{
    //TODO new setOverlayState(State state) function
    if ((ready && internalReadyState) || (!ready && !internalReadyState)) {
            return;
    }

    internalReadyState = ready;

    getDisplayString().setTagArg("i", 1, ready ? "" : "red");
    if (isMalicious()) {
        getDisplayString().setTagArg("i", 1, ready ? "green" : "yellow");
    }

    globalNodeList->setOverlayReadyIcon(getThisNode(), ready);

    if (ready) {
        bootstrapList->registerBootstrapNode(thisNode);
    } else {
        bootstrapList->removeBootstrapNode(thisNode);
    }

    if (globalParameters->getPrintStateToStdOut()) {
        std::cout << "OVERLAY STATE: " << (ready ? "READY (" : "OFFLINE (")
                  << thisNode << ")" << std::endl;
    }

    CompReadyMessage* msg = new CompReadyMessage;
    msg->setReady(ready);
    msg->setComp(OVERLAY_COMP);

    // notify all registered components about new overlay state
    sendMessageToAllComp(msg, OVERLAY_COMP);
}



//------------------------------------------------------------------------
//--- Messages -----------------------------------------------------------
//------------------------------------------------------------------------

void BaseOverlay::sendRouteMessage(const TransportAddress& dest,
                                   BaseRouteMessage* msg,
                                   bool ack)
{
    OverlayCtrlInfo* ctrlInfo =
        dynamic_cast<OverlayCtrlInfo*>(msg->removeControlInfo());

    // records statistics, if we forward this message
    if (ctrlInfo && ctrlInfo->getLastHop().getIp() != thisNode.getIp()) {
        if (msg->getStatType() == APP_DATA_STAT) {
            RECORD_STATS(numAppDataForwarded++;
                         bytesAppDataForwarded += msg->getByteLength());
        } else if (msg->getStatType() == APP_LOOKUP_STAT){
            RECORD_STATS(numAppLookupForwarded++;
                         bytesAppLookupForwarded += msg->getByteLength());
        } else {
            RECORD_STATS(numMaintenanceForwarded++;
                         bytesMaintenanceForwarded += msg->getByteLength());
        }
    }

    delete ctrlInfo;

    if (msg && (dest != thisNode)) {
        msg->setHopCount(msg->getHopCount() + 1);
    }
    if (!ack)
        sendMessageToUDP(dest, msg);
    else {
        NextHopCall* nextHopCall = new NextHopCall(msg->getName());
        nextHopCall->setBitLength(NEXTHOPCALL_L(nextHopCall));
        nextHopCall->encapsulate(msg);
        nextHopCall->setStatType(msg->getStatType());

        // TODO parameter, in recursive mode routeRetries should be 0,
        // in iterative mode routeRetries could be more than 0
        uint8_t routeRetries = 0;
        sendUdpRpcCall(dest, nextHopCall, NULL, -1, routeRetries);
    }
}
void BaseOverlay::sendMessageToUDP(const TransportAddress& dest,
                                   cPacket* msg)
{
    // if there's still a control info attached to the message, remove it
    cPolymorphic* ctrlInfo = msg->removeControlInfo();
    if (ctrlInfo != NULL)
        delete ctrlInfo;

    // debug message
    if (debugOutput) {
        EV << "[BaseOverlay::sendMessageToUDP() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Sending " << *msg << " to " << dest.getIp()
        << endl;
    }

    if (dest != thisNode) {
        BaseOverlayMessage* baseOverlayMsg
            = check_and_cast<BaseOverlayMessage*>(msg);
        // record statistics, if message is not local
        if (baseOverlayMsg->getStatType() == APP_DATA_STAT) {
            RECORD_STATS(numAppDataSent++;
                         bytesAppDataSent += msg->getByteLength());
        } else if (baseOverlayMsg->getStatType() == APP_LOOKUP_STAT){
            RECORD_STATS(numAppLookupSent++; bytesAppLookupSent +=
                         msg->getByteLength());
        } else { // MAINTENANCE_STAT
            RECORD_STATS(numMaintenanceSent++; bytesMaintenanceSent +=
                         msg->getByteLength());
        }
        recordOverlaySentStats(baseOverlayMsg);

        if (dynamic_cast<BaseResponseMessage*>(msg) && getMeasureAuthBlock()) {
            // TODO: Also consider signed DhtPutMessages
            RECORD_STATS(bytesAuthBlockSent += ceil(AUTHBLOCK_L/8.0));
        }
    } else {
        if (dest.getIp() == thisNode.getIp()) { // to same node, but different port
            RECORD_STATS(numInternalSent++; bytesInternalSent += msg->getByteLength());
        }
    }
    socket.sendTo(msg,dest.getIp(),dest.getPort());
    send(msg, "udpOut");
}

//------------------------------------------------------------------------
//--- Basic Routing ------------------------------------------------------
//------------------------------------------------------------------------

static int pendingLookups = 0;

void BaseOverlay::initLookups()
{
    lookups = LookupSet();
}

void BaseOverlay::finishLookups()
{
    while (lookups.size() > 0) {
        (*lookups.begin())->abortLookup();
    }
    lookups.clear();
}

class SendToKeyListener : public LookupListener
{
private:
    BaseOverlay* overlay;
    BaseOverlayMessage* msg;
    GlobalStatistics* globalStatistics;
public:
    SendToKeyListener( BaseOverlay* overlay, BaseOverlayMessage* msg ) {
        this->overlay = overlay;
        this->msg = msg;
        globalStatistics = overlay->globalStatistics;
        pendingLookups++;
    }

    ~SendToKeyListener() {
        pendingLookups--;
        overlay = NULL;
        if (msg != NULL) {
            delete msg;
            msg = NULL;
        }
    }

    virtual void lookupFinished(AbstractLookup *lookup) {
        if (dynamic_cast<BaseRouteMessage*>(msg)) {
            BaseRouteMessage* routeMsg = static_cast<BaseRouteMessage*>(msg);
            if (lookup->isValid()) {
                if (lookup->getResult().size()==0) {
                    EV << "[SendToKeyListener::lookupFinished()]\n"
                          "    [ERROR] SendToKeyListener: Valid result, "
                          "but empty array." << endl;
                } else {
                    routeMsg->setHopCount(routeMsg->getHopCount()
                                          + lookup->getAccumulatedHops());

                    for (uint32_t i=0; i<lookup->getResult().size(); i++) {
                        overlay->sendRouteMessage(lookup->getResult()[i],
                                                  static_cast<BaseRouteMessage*>
                                                   (routeMsg->dup()),
                                                  overlay->routeMsgAcks);
                    }
                }
            } else {
                EV << "[SendToKeyListener::lookupFinished()]\n"
                   << "    Lookup failed - dropping message"
                   << endl;
                //std::cout << simTime() << " "
                //          << routeMsg->getSrcNode()
                //          << " [SendToKeyListener::lookupFinished()]\n"
                //          << "    Lookup failed - dropping message"
                //          << std::endl;
                RECORD_STATS(overlay->numDropped++;
                             overlay->bytesDropped += routeMsg->getByteLength());
            }
        } else if (dynamic_cast<LookupCall*>(msg)) {
            LookupCall* call = static_cast<LookupCall*>(msg);
            LookupResponse* response = new LookupResponse();
            response->setKey(call->getKey());
            response->setHopCount(lookup->getAccumulatedHops());
            if (lookup->isValid()) {
                response->setIsValid(true);
                response->setSiblingsArraySize(lookup->getResult().size());
                for (uint32_t i=0; i<lookup->getResult().size(); i++) {
                    response->setSiblings(i, lookup->getResult()[i]);
                }
                if (lookup->getResult().size() == 0) {
                    EV << "[SendToKeyListener::lookupFinished() @ "
                       << overlay->thisNode.getIp()
                       << " (" << overlay->thisNode.getKey().toString(16) << ")]\n"
                       << "    LookupCall "
                       << call->getNonce()
                       << " failed! (size=0)" << endl;
                }
            } else {
                response->setIsValid(false);
                EV << "[SendToKeyListener::lookupFinished() @ "
                   << overlay->thisNode.getIp()
                   << " (" << overlay->thisNode.getKey().toString(16) << ")]\n"
                   << "    LookupCall "
                   << call->getNonce()
                   << " failed!" << endl;
            }
            overlay->sendRpcResponse(call, response);
            msg = NULL;
        } else {
            throw cRuntimeError("SendToKeyListener::lookupFinished(): "
                                    "Unknown message type!");
        }
        delete this;
    }
};

void BaseOverlay::route(const OverlayKey& key, CompType destComp,
                        CompType srcComp, cPacket* msg,
                        const std::vector<TransportAddress>& sourceRoute,
                        RoutingType routingType)
{
    if (key.isUnspecified() &&
        (!sourceRoute.size() || sourceRoute[0].isUnspecified()))
        throw cRuntimeError("route(): Key and hint unspecified!");

    // encapsulate in a app data message for multiplexing
    // to destination component
    BaseAppDataMessage* baseAppDataMsg =
        new BaseAppDataMessage("BaseAppDataMessage");
    baseAppDataMsg->setType(APPDATA);
    baseAppDataMsg->setDestComp(destComp);
    baseAppDataMsg->setSrcComp(srcComp);
    baseAppDataMsg->setBitLength(BASEAPPDATA_L(baseAppDataMsg));
    baseAppDataMsg->setName(msg->getName());

    baseAppDataMsg->setStatType(APP_DATA_STAT);
    baseAppDataMsg->encapsulate(msg);

    // debug output
    if (debugOutput) {
        EV << "[BaseOverlay::route() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Received message from application"
        << endl;
    }

    if (key.isUnspecified() && sourceRoute.size() <= 1) {
        sendMessageToUDP(sourceRoute[0], baseAppDataMsg);
    } else {
        if (internalReadyState == false) {
            // overlay not ready => sendToKey doesn't work yet
            EV << "[BaseOverlay::route() @ "
               << getThisNode().getIp()
               << " (" << getThisNode().getKey().toString(16) << ")]\n"
               << "    Couldn't route application message to key "
               << key.toString(16)
               << " because the overlay module is not ready!" << endl;
            RECORD_STATS(numDropped++;
                         bytesDropped += baseAppDataMsg->getByteLength());
            delete baseAppDataMsg;
            return;
        }

        sendToKey(key, baseAppDataMsg, 1, sourceRoute, routingType);
    }
}

bool BaseOverlay::recursiveRoutingHook(const TransportAddress& dest,
                                       BaseRouteMessage* msg)
{
    return true;
}

void BaseOverlay::sendToKey(const OverlayKey& key, BaseOverlayMessage* msg,
                            int numSiblings,
                            const std::vector<TransportAddress>& sourceRoute,
                            RoutingType routingType)
{
    BaseRouteMessage* routeMsg = NULL;

    if (routingType == DEFAULT_ROUTING) routingType = defaultRoutingType;

    if (debugOutput) {
        EV << "[BaseOverlay::sendToKey() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Sending " << msg <<  " to " << key
        << endl;
    }

    if (key.isUnspecified() &&
        !(sourceRoute.size() && !sourceRoute[0].isUnspecified()))
        throw cRuntimeError("BaseOverlay::sendToKey(): "
                            "unspecified destination address and key!");

    if (msg->getType() != OVERLAYROUTE) {
        assert(!msg->getControlInfo());
        routeMsg = new BaseRouteMessage("BaseRouteMessage");
        routeMsg->setType(OVERLAYROUTE);
        routeMsg->setRoutingType(routingType);
        routeMsg->setDestKey(key);
        routeMsg->setSrcNode(thisNode);
        routeMsg->setStatType(msg->getStatType());
        // copy the name of the inner message
        routeMsg->setName(msg->getName());
        routeMsg->setBitLength(BASEROUTE_L(routeMsg));
        routeMsg->encapsulate(msg);

        OverlayCtrlInfo* routeCtrlInfo = new OverlayCtrlInfo;
        routeCtrlInfo->setLastHop(thisNode);
        routeCtrlInfo->setTransportType(ROUTE_TRANSPORT);
        routeCtrlInfo->setRoutingType(routingType);
        routeMsg->setControlInfo(routeCtrlInfo);

        //message marked as not already forwarded to tier1
        routeMsg->setContextPointer(NULL);
    } else {
        routeMsg = check_and_cast<BaseRouteMessage*>(msg);
        routingType = static_cast<RoutingType>(routeMsg->getRoutingType());
    }

    // set timestamp for next hop
    if (collectPerHopDelay) {
        routeMsg->setHopStamp(simTime());
    }

    if (sourceRoute.size() && !sourceRoute[0].isUnspecified()) {
        // send msg to nextHop if specified (used for e.g. join rpcs)
        OverlayCtrlInfo* ctrlInfo = check_and_cast<OverlayCtrlInfo*>
            (routeMsg->getControlInfo());
        ctrlInfo->setTransportType(UDP_TRANSPORT);
        assert(routeMsg->getNextHopsArraySize() == 0);
        routeMsg->setNextHopsArraySize(sourceRoute.size());
        for (uint32_t i = 0; i < sourceRoute.size(); ++i)
            routeMsg->setNextHops(i, sourceRoute[i]);
        if (recursiveRoutingHook(sourceRoute[0], routeMsg)) { //test
            sendRouteMessage(sourceRoute[0], routeMsg, routeMsgAcks);
        }
        return;
    }

    if ((routingType == ITERATIVE_ROUTING)
        || (routingType == EXHAUSTIVE_ITERATIVE_ROUTING)
        ) {

        // create lookup and sent to key
        AbstractLookup* lookup = createLookup(routingType, routeMsg, NULL,
                                    (routeMsg->getStatType() == APP_DATA_STAT));
        lookup->lookup(routeMsg->getDestKey(), numSiblings, hopCountMax,
                       0, new SendToKeyListener(this, routeMsg));
    } else  {
        // recursive routing
        NodeVector* nextHops = findNode(routeMsg->getDestKey(),
                                        recNumRedundantNodes,
                                        numSiblings, routeMsg);

        if (nextHops->size() == 0) {
            EV << "[BaseOverlay::sendToKey() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    FindNode() returned NULL - dropping message"
               << endl;
            //std::cout << simTime() << " " << thisNode.getIp() << " " << state
            //          << " FindNode() returned NULL - dropping message "
            //          << routeMsg->getName() << " from "
            //          << routeMsg->getSrcNode() << std::endl;

            // statistics
            RECORD_STATS(numDropped++; bytesDropped += routeMsg->getByteLength());
            delete routeMsg;
        } else {
            // delete message if the hop count maximum is exceeded
            if (routeMsg->getHopCount() >= hopCountMax) {

                EV << "[BaseOverlay::sendToKey() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Discards " << routeMsg->getName() << " from "
                   << routeMsg->getSrcNode().getIp() << "\n"
                   << "    The hop count maximum has been exceeded ("
                   << routeMsg->getHopCount() << ">="
                   << hopCountMax << ")"
                   << endl;
                //std::cout << "[BaseOverlay::sendToKey() @ " << thisNode.getIp()
                //          << " (" << thisNode.getKey().toString(16) << ")]\n"
                //          << "    Discards " << routeMsg->getName() << " from "
                //          << routeMsg->getSrcNode().getIp() << "\n"
                //          << "    The hop count maximum has been exceeded ("
                //          << routeMsg->getHopCount() << ">="
                //          << hopCountMax << ")"
                //          << std::endl;

                // statistics
                RECORD_STATS(numDropped++;
                             bytesDropped += routeMsg->getByteLength());
                delete routeMsg;
                delete nextHops;
                return;
            }

            OverlayCtrlInfo* overlayCtrlInfo =
                dynamic_cast<OverlayCtrlInfo*>(routeMsg->getControlInfo());
            assert(overlayCtrlInfo);

            // check and choose nextHop candidate
            NodeHandle* nextHop = NULL;
            bool err, isSibling;
            isSibling = isSiblingFor(thisNode, routeMsg->getDestKey(),
                                     numSiblings, &err);

            // if route is recorded we can do a real loop detection
            std::set<TransportAddress> visitedHops;
            for (uint32_t i = 0; i < routeMsg->getVisitedHopsArraySize(); ++i) {
                visitedHops.insert(routeMsg->getVisitedHops(i));
            }

            for (uint32_t index = 0; nextHop == NULL && nextHops->size() > index;
                 ++index) {
                nextHop = &((*nextHops)[index]);
                // loop detection
                if (((overlayCtrlInfo->getLastHop() == *nextHop) &&
                     (*nextHop != thisNode)) ||
                     (visitedHops.find(*nextHop) != visitedHops.end()) ||
                     // do not forward msg to source node
                    ((*nextHop == routeMsg->getSrcNode()) &&
                     (thisNode != routeMsg->getSrcNode())) ||
                     // nextHop is thisNode, but isSiblingFor() is false
                    ((*nextHop == thisNode) && (!isSibling))) {
                    nextHop = NULL;
                }
            }

            if (nextHop == NULL) {
                if (!checkFindNode(routeMsg)) {
                    EV << "[BaseOverlay::sendToKey() @ " << thisNode.getIp()
                       << " (" << thisNode.getKey().toString(16) << ")]\n"
                       << "    Discards " << routeMsg->getName() << " from "
                       << routeMsg->getSrcNode().getIp() << "\n"
                       << "    No useful nextHop found!"
                       << endl;
                    //std::cout << thisNode.getIp() << " packet from "
                    //          << routeMsg->getSrcNode().getIp()
                    //          << " dropped: " << routeMsg
                    //          << " " << state << std::endl;
                    RECORD_STATS(numDropped++;
                                 bytesDropped += routeMsg->getByteLength());
                }
                delete routeMsg;
                delete nextHops;
                return;
            }

            assert(!nextHop->isUnspecified());

            // callForward to app
            if (useCommonAPIforward &&
                dynamic_cast<BaseAppDataMessage*>(
                        routeMsg->getEncapsulatedPacket()) &&
                routeMsg->getContextPointer() == NULL) {
                callForward(routeMsg->getDestKey(), routeMsg, *nextHop);
                delete nextHops;
                return;
            }
            //message marked as not already forwarded
            routeMsg->setContextPointer(NULL);

            // is this node responsible?
            if (*nextHop == thisNode) {
                if (isSibling && !err) {
                    //EV << "[BaseOverlay::sendToKey() @ " << thisNode.getIp()
                    //   << " (" << thisNode.getKey().toString(16) << ")]\n"
                    //   << "    Forwards msg for key " << routeMsg->getDestKey() "\n"
                    //   << "    to node " << (*nextHops)[0]
                    //   << endl;
                    delete nextHops;
                    assert(routeMsg->getControlInfo());
                    handleBaseOverlayMessage(routeMsg, key);
                    return;
                } else {
                    throw cRuntimeError("isSiblingsFor() is true with an "
                                        "error: Erroneous method "
                                        "isSiblingFor()!");
                }
            }
            // else forward msg if this node is not responsible for the key
            overlayCtrlInfo->setHopCount(routeMsg->getHopCount());
            if (recursiveRoutingHook(*nextHop, routeMsg)) {
                sendRouteMessage(*nextHop, routeMsg, routeMsgAcks);
            }
        }
        delete nextHops;
    }
}

bool BaseOverlay::checkFindNode(BaseRouteMessage* routeMsg)
{
    if (dynamic_cast<FindNodeCall*>(routeMsg->getEncapsulatedPacket())) {
        FindNodeCall* findNodeCall =
            static_cast<FindNodeCall*>(routeMsg->decapsulate());
        findNodeCall
            ->setControlInfo(check_and_cast<OverlayCtrlInfo*>
            (routeMsg->removeControlInfo()));
        findNodeRpc(findNodeCall);
        return true;
    }
    return false;
}

//protected: create a lookup class
AbstractLookup* BaseOverlay::createLookup(RoutingType routingType,
                                          const BaseOverlayMessage* msg,
                                          const cPacket* findNodeExt,
                                          bool appLookup)
{
    AbstractLookup* newLookup;

    if (routingType == DEFAULT_ROUTING) {
        routingType = defaultRoutingType;
    }

    switch (routingType) {
        case ITERATIVE_ROUTING:
        case EXHAUSTIVE_ITERATIVE_ROUTING:
            newLookup = new IterativeLookup(this, routingType,
                                            iterativeLookupConfig, findNodeExt,
                                            appLookup);
            break;
        case RECURSIVE_SOURCE_ROUTING:
        case SEMI_RECURSIVE_ROUTING:
        case FULL_RECURSIVE_ROUTING:
            newLookup = new RecursiveLookup(this, routingType,
                                            recursiveLookupConfig,
                                            appLookup);
            break;
        default:
            throw cRuntimeError("BaseOverlay::createLookup():"
                                    " Unknown routingType!");
            break;
    }

    lookups.insert(newLookup);
    return newLookup;
}

void BaseOverlay::removeLookup(AbstractLookup* lookup)
{
    lookups.erase(lookup);
}

//virtual public
OverlayKey BaseOverlay::distance(const OverlayKey& x,
                                 const OverlayKey& y,
                                 bool useAlternative) const
{
    throw cRuntimeError("BaseOverlay::distance(): Not implemented!");
    return OverlayKey::UNSPECIFIED_KEY;
}

//protected: find closest nodes
NodeVector* BaseOverlay::findNode(const OverlayKey& key,
                                  int numRedundantNodes,
                                  int numSiblings,
                                  BaseOverlayMessage* msg)
{
    throw cRuntimeError("findNode: Not implemented!");
    return NULL;
}

//protected: join the overlay with a given nodeID
void BaseOverlay::joinOverlay()
{
//  std::cout << "BaseOverlay::joinOverlay(): Not implemented!" << endl;
    return;
}

bool BaseOverlay::handleFailedNode(const TransportAddress& failed)
{
    return true;
}

//------------------------------------------------------------------------
//--- RPCs ---------------------------------------------------------------
//------------------------------------------------------------------------

//private
bool BaseOverlay::internalHandleRpcCall(BaseCallMessage* msg)
{
    // call rpc stubs
    RPC_SWITCH_START( msg );
    RPC_DELEGATE( FindNode, findNodeRpc );
    RPC_DELEGATE( FailedNode, failedNodeRpc );
    RPC_DELEGATE( Lookup, lookupRpc );
    RPC_DELEGATE( NextHop, nextHopRpc );
    RPC_SWITCH_END( );

    // if RPC was handled return true, else tell the parent class to handle it
    return RPC_HANDLED || BaseRpc::internalHandleRpcCall(msg);
}

void BaseOverlay::internalHandleRpcResponse(BaseResponseMessage* msg,
                                            cPolymorphic* context,
                                            int rpcId, simtime_t rtt)
{
    BaseRpc::internalHandleRpcResponse(msg, context, rpcId, rtt);
}

void BaseOverlay::internalHandleRpcTimeout(BaseCallMessage* msg,
                                           const TransportAddress& dest,
                                           cPolymorphic* context, int rpcId,
                                           const OverlayKey& destKey)
{
    RPC_SWITCH_START( msg )
        RPC_ON_CALL( NextHop )
        {
            BaseRouteMessage* tempMsg
                = check_and_cast<BaseRouteMessage*>(msg->decapsulate());

            assert(!tempMsg->getControlInfo());
            if (!tempMsg->getControlInfo()) {
                OverlayCtrlInfo* overlayCtrlInfo = new OverlayCtrlInfo;
                overlayCtrlInfo->setLastHop(thisNode);
                overlayCtrlInfo->setHopCount(tempMsg->getHopCount());
                overlayCtrlInfo->setSrcNode(tempMsg->getSrcNode());
                overlayCtrlInfo->setRoutingType(tempMsg->getRoutingType());
                overlayCtrlInfo->setTransportType(UDP_TRANSPORT);
                tempMsg->setControlInfo(overlayCtrlInfo);
            }
            // remove node from local routing tables
            // + route message again if possible
            assert(!dest.isUnspecified() && destKey.isUnspecified());
            if (handleFailedNode(dest)) {
                if (!tempMsg->getDestKey().isUnspecified()) {
                    // TODO: msg is resent only in recursive mode
                    EV << "[BaseOverlay::internalHandleRpcTimeout() @ "
                       << thisNode.getIp()
                       << " (" << thisNode.getKey().toString(16) << ")]\n"
                       << "    Resend msg for key " << destKey
                       << endl;
                    handleBaseOverlayMessage(tempMsg, destKey);
                } else if(tempMsg->getNextHopsArraySize() > 1) {
                    for (uint8_t i = 0; i < tempMsg->getNextHopsArraySize() - 1; ++i) {
                        tempMsg->setNextHops(i, tempMsg->getNextHops(i + 1));
                    }
                    tempMsg->setNextHopsArraySize(tempMsg->getNextHopsArraySize() - 1);
                    EV << "[BaseOverlay::internalHandleRpcTimeout() @ "
                       << thisNode.getIp()
                       << " (" << thisNode.getKey().toString(16) << ")]\n"
                       << "    Resend msg to next available node in nextHops[]: "
                       << tempMsg->getNextHops(0).getIp()
                       << std::endl;
                    handleBaseOverlayMessage(tempMsg);
                } else {
                    EV << "[BaseOverlay::internalHandleRpcTimeout() @ "
                       << thisNode.getIp()
                       << " (" << thisNode.getKey().toString(16) << ")]\n"
                       << "    dropping msg for " << dest
                       << endl;
                    RECORD_STATS(numDropped++;
                                 bytesDropped += tempMsg->getByteLength());
                    delete tempMsg;
                }
            } else {
                RECORD_STATS(numDropped++;
                             bytesDropped += tempMsg->getByteLength());
                delete tempMsg;
                join();
            }
            break;
        }
    RPC_SWITCH_END( )

    BaseRpc::internalHandleRpcTimeout(msg, dest, context, rpcId, destKey);
}

void BaseOverlay::internalSendRouteRpc(BaseRpcMessage* message,
                                       const OverlayKey& destKey,
                                       const std::vector<TransportAddress>&
                                       sourceRoute,
                                       RoutingType routingType) {
    FindNodeCall* findNodeCall;
    uint32_t numSiblings = 1;
    if ((findNodeCall = dynamic_cast<FindNodeCall*>(message)))
        numSiblings = findNodeCall->getNumSiblings();

    sendToKey(destKey, message, numSiblings, sourceRoute, routingType);
}

void BaseOverlay::internalSendRpcResponse(BaseCallMessage* call,
                                          BaseResponseMessage* response)
{
    OverlayCtrlInfo* overlayCtrlInfo =
        check_and_cast<OverlayCtrlInfo*>(call->getControlInfo());

    TransportType transportType = ROUTE_TRANSPORT;
    const TransportAddress* destNode;
    if (overlayCtrlInfo->getSrcNode().isUnspecified()) {
        if (sendRpcResponseToLastHop) {
            // used for KBR protocols to deal with NATs
            // (srcNode in call message may contain private IP address)
            destNode = &(overlayCtrlInfo->getLastHop());
        } else {
            // used for non-KBR protocols which have to route RPC calls
            // but can't use BaseRouteMessage (this doesn't work with NATs)
            destNode = &(call->getSrcNode());
        }
    } else {
        destNode = &(overlayCtrlInfo->getSrcNode());
    }
    const OverlayKey* destKey = &OverlayKey::UNSPECIFIED_KEY;

    RoutingType routingType
        = static_cast<RoutingType>(overlayCtrlInfo->getRoutingType());

    assert(overlayCtrlInfo->getTransportType() != INTERNAL_TRANSPORT);

    if ((overlayCtrlInfo->getTransportType() == UDP_TRANSPORT) ||
        (routingType == SEMI_RECURSIVE_ROUTING) ||
        (routingType == ITERATIVE_ROUTING) ||
        (routingType == EXHAUSTIVE_ITERATIVE_ROUTING)
        ) {
        // received by UDP or direct response (IR, EIR or SRR routing)
        transportType = UDP_TRANSPORT;
        overlayCtrlInfo->setVisitedHopsArraySize(0); //???
    } else if ((static_cast<RoutingType> (overlayCtrlInfo->getRoutingType())
            == FULL_RECURSIVE_ROUTING)) {
        // full recursive routing
        destKey = &(overlayCtrlInfo->getSrcNode().getKey());
        destNode = &NodeHandle::UNSPECIFIED_NODE;
    }
    // else: source routing -> route back over visited hops

    sendRpcResponse(transportType,
                    static_cast<CompType>(overlayCtrlInfo->getSrcComp()),
                    *destNode, *destKey, call, response);
}

//protected: statistic helpers for IterativeLookup
void BaseOverlay::countFindNodeCall( const FindNodeCall* call )
{
    RECORD_STATS(numFindNodeSent++;
                 bytesFindNodeSent += call->getByteLength());
}

void BaseOverlay::countFailedNodeCall( const FailedNodeCall* call )
{
    RECORD_STATS(numFailedNodeSent++;
                 bytesFailedNodeSent += call->getByteLength());
}

//private: rpc stub
void BaseOverlay::findNodeRpc( FindNodeCall* call )
{
    // if this node is malicious don't answer a findNodeCall
    if (isMalicious() && dropFindNodeAttack) {
        EV << "[BaseOverlay::findNodeRpc() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Node ignores findNodeCall because this node is malicious"
           << endl;
        delete call;
        return;
    }

    FindNodeResponse* findNodeResponse =
        new FindNodeResponse("FindNodeResponse");

    findNodeResponse->setBitLength(FINDNODERESPONSE_L(findNodeResponse));
    NodeVector* nextHops = findNode(call->getLookupKey(),
                                    call->getNumRedundantNodes(),
                                    call->getExhaustiveIterative() ? -1 : call->getNumSiblings(), call);

    findNodeResponse->setClosestNodesArraySize(nextHops->size());
    for (uint32_t i=0; i < nextHops->size(); i++) {
        findNodeResponse->setClosestNodes(i, (*nextHops)[i]);
    }

    bool err;
    if (!call->getExhaustiveIterative() &&
            isSiblingFor(thisNode, call->getLookupKey(), call->getNumSiblings(),
                         &err)) {
        findNodeResponse->setSiblings(true);
    }

    if (isMalicious() && invalidNodesAttack) {
        if (isSiblingAttack) {
            findNodeResponse->setSiblings(true);
        } else {
            findNodeResponse->setSiblings(false);
        }

        int resultSize = isSiblingAttack ? call->getNumSiblings() :
                                           call->getNumRedundantNodes();

        findNodeResponse->setClosestNodesArraySize(resultSize);
        for (int i = 0; i < resultSize; i++) {
            findNodeResponse->setClosestNodes(i,
                    NodeHandle(call->getLookupKey() + i, IPvXAddress(IPv4Address(
                    isSiblingAttack ? (424242+i) : intuniform(42,123123))), 42));
#if 0
            // was not used for evaluation
            if ((i == 0) && isSiblingAttack) {
                findNodeResponse->setClosestNodes(0, thisNode);
            }
#endif
        }
    } else if (isMalicious() && isSiblingAttack) {
        findNodeResponse->setSiblings(true);
        findNodeResponse->setClosestNodesArraySize(1);
        findNodeResponse->setClosestNodes(0, thisNode);
    }

    findNodeResponse->setBitLength(FINDNODERESPONSE_L(findNodeResponse));

    if (call->hasObject("findNodeExt")) {
        cPacket* findNodeExt = check_and_cast<cPacket*>(call->removeObject("findNodeExt"));
        findNodeResponse->addObject(findNodeExt);
        findNodeResponse->addBitLength(findNodeExt->getBitLength());
    }

    RECORD_STATS(numFindNodeResponseSent++; bytesFindNodeResponseSent +=
        findNodeResponse->getByteLength());

    delete nextHops;

    sendRpcResponse(call, findNodeResponse);
}


void BaseOverlay::failedNodeRpc( FailedNodeCall* call )
{
    FailedNodeResponse* failedNodeResponse =
        new FailedNodeResponse("FailedNodeResponse");
    failedNodeResponse->setTryAgain(handleFailedNode(call->getFailedNode()));
    failedNodeResponse->setBitLength(FAILEDNODERESPONSE_L(failedNodeResponse));

    if (call->hasObject("findNodeExt")) {
        cPacket* findNodeExt = check_and_cast<cPacket*>(
                                    call->removeObject("findNodeExt"));
        failedNodeResponse->addObject(findNodeExt);
        failedNodeResponse->addBitLength(findNodeExt->getBitLength());
    }

    RECORD_STATS(numFailedNodeResponseSent++; bytesFailedNodeResponseSent +=
                     failedNodeResponse->getByteLength());

    sendRpcResponse(call, failedNodeResponse);
}

void BaseOverlay::lookupRpc(LookupCall* call)
{
    int numSiblings = call->getNumSiblings();

    if (numSiblings < 0) {
        numSiblings = getMaxNumSiblings();
    }

    if (internalReadyState == false) {
        // overlay not ready => lookup failed
        EV << "[BaseOverlay::lookupRpc() @ "
           << getThisNode().getIp()
           << " (" << getThisNode().getKey().toString(16) << ")]\n"
           << "    LookupCall "
           << call->getNonce()
           << " failed, because overlay module is not ready!" << endl;

        LookupResponse* response = new LookupResponse();
        response->setKey(call->getKey());
        response->setIsValid(false);

        sendRpcResponse(call, response);

        return;
    }

    // create lookup and sent to key
    AbstractLookup* lookup = createLookup(static_cast<RoutingType>(
            call->getRoutingType()), call, NULL, true);
    lookup->lookup(call->getKey(), numSiblings, hopCountMax,
                   1, new SendToKeyListener( this, call ));
}

void BaseOverlay::nextHopRpc(NextHopCall* call)
{
    if (state != READY) {
        //TODO EV...
        delete call;
        return;
    }

    BaseRouteMessage* routeMsg
        = check_and_cast<BaseRouteMessage*>(call->decapsulate());

    OverlayCtrlInfo* overlayCtrlInfo =
        check_and_cast<OverlayCtrlInfo*>(call->getControlInfo()->dup());
    overlayCtrlInfo->setHopCount(routeMsg->getHopCount());
    overlayCtrlInfo->setSrcNode(routeMsg->getSrcNode());
    overlayCtrlInfo->setRoutingType(routeMsg->getRoutingType());

    routeMsg->setControlInfo(overlayCtrlInfo);
    assert(routeMsg->getControlInfo());

    std::string temp("ACK: [");
    (temp += routeMsg->getName()) += "]";

    NextHopResponse* response
        = new NextHopResponse(temp.c_str());
    response->setBitLength(NEXTHOPRESPONSE_L(response));
    sendRpcResponse(call, response);

    handleBaseOverlayMessage(routeMsg, routeMsg->getDestKey());
}

void BaseOverlay::registerComp(CompType compType, cModule *module)
{
    cGate *gate = NULL;

    if (module != NULL) {
        gate = module->gate("direct_in");
        if (gate == NULL) {
            throw cRuntimeError("BaseOverlay::registerComp(): The module "
                                "which tried to register has "
                                "no direct_in gate!");
        }
    }

    compModuleList[compType] = make_pair<cModule*, cGate*>(module, gate);
}

cModule* BaseOverlay::getCompModule(CompType compType)
{
    CompModuleList::iterator it = compModuleList.find(compType);

    if (it != compModuleList.end())
        return it->second.first;
    else
        return NULL;
}

cGate* BaseOverlay::getCompRpcGate(CompType compType)
{
    CompModuleList::iterator it = compModuleList.find(compType);

    if (it != compModuleList.end())
        return it->second.second;
    else
        return NULL;
}

void BaseOverlay::sendMessageToAllComp(cMessage* msg, CompType srcComp)
{
    Enter_Method_Silent();
    take(msg);

    for (CompModuleList::iterator it = compModuleList.begin();
         it != compModuleList.end(); it++) {

        // don't send message to the origination component
        if (it->first != srcComp)
            sendDirect((cMessage*)msg->dup(), it->second.second);
    }

    delete msg;
}

bool BaseOverlay::isInSimpleMultiOverlayHost()
{
    return isVector() || getParentModule()->isVector();
}

