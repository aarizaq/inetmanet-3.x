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
 * @file BaseApp.cc
 * @author Ingmar Baumgart, Bernhard Heep, Stephan Krause
 */

#include <IPvXAddressResolver.h>
#include <NotificationBoard.h>
#include <cassert>

#include <CommonMessages_m.h>
#include <BaseRpc.h>
#include <OverlayAccess.h>
#include <GlobalNodeListAccess.h>
#include <GlobalStatisticsAccess.h>
#include <UnderlayConfiguratorAccess.h>
#include "UDPControlInfo_m.h"
#include "BaseApp.h"

using namespace std;

BaseApp::BaseApp()
{
    notificationBoard = NULL;

    overlay = NULL;
}

BaseApp::~BaseApp()
{
    finishRpcs();
}

int BaseApp::numInitStages() const
{
    return MAX_STAGE_APP + 1;
}

void BaseApp::initialize(int stage)
{
    CompType compType = getThisCompType();
    bool tier = (compType == TIER1_COMP ||
                 compType == TIER2_COMP ||
                 compType == TIER3_COMP);

    if (stage == REGISTER_STAGE) {
        OverlayAccess().get(this)->registerComp(getThisCompType(), this);
        return;
    }

    if ((tier && stage == MIN_STAGE_APP) ||
        (!tier && stage == MIN_STAGE_COMPONENTS)) {
        // fetch parameters
        debugOutput = par("debugOutput");

        globalNodeList = GlobalNodeListAccess().get();
        underlayConfigurator = UnderlayConfiguratorAccess().get();
        globalStatistics = GlobalStatisticsAccess().get();
        notificationBoard = NotificationBoardAccess().get();

        // subscribe to the notification board
        notificationBoard->subscribe(this, NF_OVERLAY_TRANSPORTADDRESS_CHANGED);
        notificationBoard->subscribe(this, NF_OVERLAY_NODE_LEAVE);
        notificationBoard->subscribe(this, NF_OVERLAY_NODE_GRACEFUL_LEAVE);

        // determine the terminal's transport address
        if (getParentModule()->getSubmodule("interfaceTable", 0) != NULL) {
            thisNode.setIp(IPvXAddressResolver()
                          .addressOf(getParentModule()));
        } else {
            thisNode.setIp(IPvXAddressResolver()
                          .addressOf(getParentModule()->getParentModule()));
        }

        thisNode.setPort(-1);

        WATCH(thisNode);

        // statistics
        numOverlaySent = 0;
        numOverlayReceived = 0;
        bytesOverlaySent = 0;
        bytesOverlayReceived = 0;
        numUdpSent = 0;
        numUdpReceived = 0;
        bytesUdpSent = 0;
        bytesUdpReceived = 0;

        creationTime = simTime();

        WATCH(numOverlaySent);
        WATCH(numOverlayReceived);
        WATCH(bytesOverlaySent);
        WATCH(bytesOverlayReceived);
        WATCH(numUdpSent);
        WATCH(numUdpReceived);
        WATCH(bytesUdpSent);
        WATCH(bytesUdpReceived);

        // init rpcs
        initRpcs();

        // set TCP output gate
        setTcpOut(gate("tcpOut"));
    }

    if ((stage >= MIN_STAGE_APP && stage <= MAX_STAGE_APP) ||
        (stage >= MIN_STAGE_COMPONENTS && stage <= MAX_STAGE_COMPONENTS)) //TODO
        initializeApp(stage);
}

void BaseApp::initializeApp(int stage)
{
    // ...
}

// Process messages passed up from the overlay.
void BaseApp::handleMessage(cMessage* msg)
{
    if (internalHandleMessage(msg)) {
        return;
    }

    if (msg->arrivedOn("from_lowerTier") ||
        msg->arrivedOn("direct_in")) {
        CompReadyMessage* readyMsg = dynamic_cast<CompReadyMessage*>(msg);
        if (readyMsg != NULL) {
            handleReadyMessage(readyMsg);
            return;
        }
        // common API
        CommonAPIMessage* commonAPIMsg = dynamic_cast<CommonAPIMessage*>(msg);
        if (commonAPIMsg != NULL) {
            handleCommonAPIMessage(commonAPIMsg);
        } else if (msg->arrivedOn("from_lowerTier")) {
            // TODO: What kind of messages to we want to measure here?
        	cPacket* packet = check_and_cast<cPacket*>(msg);
            RECORD_STATS(numOverlayReceived++;
                         bytesOverlayReceived += packet->getByteLength());
            handleLowerMessage(msg);
        }
        else delete msg;
    } else if (msg->arrivedOn("from_upperTier")) {
        handleUpperMessage(msg);
    } else if (msg->arrivedOn("udpIn")) {
    	cPacket* packet = check_and_cast<cPacket*>(msg);
        RECORD_STATS(numUdpReceived++; bytesUdpReceived += packet->getByteLength());
        // debug message
        if (debugOutput && !ev.isDisabled()) {
            UDPDataIndication* udpDataIndication =
                check_and_cast<UDPDataIndication*>(msg->getControlInfo());
            EV << "[BaseApp:handleMessage() @ " << thisNode.getIp()
            << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
            << "    Received " << *msg << " from "
            << udpDataIndication->getSrcAddr() << endl;
        }
        handleUDPMessage(msg);
    } else if(msg->arrivedOn("tcpIn")) {
        handleTCPMessage(msg);
    } else if (msg->arrivedOn("trace_in")) {
        handleTraceMessage(msg);
    }else {
        delete msg;
    }
}

CompType BaseApp::getThisCompType()
{
    std::string name(this->getName());

    if (name == std::string("tier1")) {
        return TIER1_COMP;
    } else if (name == std::string("tier2")) {
        return TIER2_COMP;
    } else if (name == std::string("tier3")) {
        return TIER3_COMP;
    }

    std::string parentName(this->getParentModule()->getName());

    if (parentName == std::string("tier1")) {
        return TIER1_COMP;
    } else if (parentName == std::string("tier2")) {
        return TIER2_COMP;
    } else if (parentName == std::string("tier3")) {
        return TIER3_COMP;
    } else {
        throw cRuntimeError("BaseApp::getThisCompType(): "
                             "Unknown module type!");
    }

    return INVALID_COMP;
}

void BaseApp::receiveChangeNotification(int category, const cPolymorphic * details)
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

void BaseApp::handleTransportAddressChangedNotification()
{
    // ...
}

void BaseApp::handleNodeLeaveNotification()
{
    // ...
}

void BaseApp::handleNodeGracefulLeaveNotification()
{
    // ...
}

void BaseApp::callRoute(const OverlayKey& key, cPacket* msg,
                        const std::vector<TransportAddress>& sourceRoute,
                        RoutingType routingType)
{
    // create route-message (common API)
    KBRroute* routeMsg = new KBRroute();
    routeMsg->setDestKey(key);

    if (!(sourceRoute.size() == 1 && sourceRoute[0].isUnspecified())) {
        routeMsg->setSourceRouteArraySize(sourceRoute.size());
        for (uint32_t i = 0; i < sourceRoute.size(); ++i) {
            routeMsg->setSourceRoute(i, sourceRoute[i]);
        }
    }
    routeMsg->encapsulate(msg);
    routeMsg->setSrcComp(thisCompType);
    routeMsg->setDestComp(thisCompType);
    routeMsg->setRoutingType(routingType);

    routeMsg->setType(KBR_ROUTE);

    sendDirect(routeMsg, overlay->getCompRpcGate(OVERLAY_COMP));

    // debug message
    if (debugOutput && !ev.isDisabled()) {
        EV << "[BaseApp::callRoute() @ " << thisNode.getIp()
        << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
        << "    Sending " << *msg
        << " to destination key " << key
        << " with source route ";

        for (uint32_t i = 0; i < sourceRoute.size(); ++i) {
            EV << sourceRoute[i] << " ";
        }

        EV << endl;
    }

    // count
    RECORD_STATS(numOverlaySent++; bytesOverlaySent += msg->getByteLength());
}

void BaseApp::deliver(OverlayKey& key, cMessage* msg)
{
    // deliver...

    delete msg;
}

void BaseApp::forward(OverlayKey* key, cPacket** msg, NodeHandle* nextHopNode)
{
    // usually do nothing
}

void BaseApp::forwardResponse(const OverlayKey& key, cPacket* msg,
                              const NodeHandle& nextHopNode)
{
    OverlayCtrlInfo* ctrlInfo =
        check_and_cast<OverlayCtrlInfo*>(msg->removeControlInfo());

    //create forwardResponse message (common API)
    KBRforward* forwardMsg = new KBRforward();
    forwardMsg->setDestKey(key);
    forwardMsg->setNextHopNode(nextHopNode);
    forwardMsg->setControlInfo(ctrlInfo);
    forwardMsg->encapsulate(msg);

    forwardMsg->setType(KBR_FORWARD_RESPONSE);

    if (getThisCompType() == TIER1_COMP) {
        send(forwardMsg, "to_lowerTier");
    } else {
        sendDirect(forwardMsg, overlay->getCompRpcGate(OVERLAY_COMP));
    }
}

void BaseApp::update(const NodeHandle& node, bool joined)
{
}

void BaseApp::handleCommonAPIMessage(CommonAPIMessage* commonAPIMsg)
{
    cPacket* tempMsg = commonAPIMsg->decapsulate();

    // process interface control information
    OverlayCtrlInfo* overlayCtrlInfo =
        dynamic_cast<OverlayCtrlInfo*>(commonAPIMsg->removeControlInfo());

    if (overlayCtrlInfo != NULL) {
        tempMsg->setControlInfo(overlayCtrlInfo);
    }

    switch (commonAPIMsg->getType()) {
        case KBR_DELIVER:
        {
            KBRdeliver* apiMsg = dynamic_cast<KBRdeliver*>(commonAPIMsg);
            OverlayKey key = apiMsg->getDestKey();
            NodeHandle nextHopNode = overlay->getThisNode();

            //first call forward, then deliver
            forward(&key, &tempMsg, &nextHopNode);

            if(tempMsg != NULL) {
                //if key or nextHopNode is changed send msg back to overlay
                if ((!key.isUnspecified() && key != apiMsg->getDestKey()) ||
                    (!nextHopNode.isUnspecified()
                            && nextHopNode != overlay->getThisNode())) {
                    forwardResponse(key, tempMsg, nextHopNode);
                }
                else {
                    RECORD_STATS(numOverlayReceived++;
                                 bytesOverlayReceived += tempMsg->getByteLength());

                    assert(overlayCtrlInfo->getTransportType()
                           == ROUTE_TRANSPORT);

                    // debug message
                    if (debugOutput) {
                        EV << "[BaseApp:handleCommonAPIMessage() @ "
                        << thisNode.getIp() << " ("
                        << overlay->getThisNode().getKey().toString(16) << ")]\n"
                        << "    Received " << *tempMsg << " from "
                        << overlayCtrlInfo->getSrcRoute() << endl;
                    }

                    //handle RPC first
                    BaseRpcMessage* rpcMessage
                        = dynamic_cast<BaseRpcMessage*>(tempMsg);
                    if (rpcMessage!=NULL) {
                        internalHandleRpcMessage(rpcMessage);
                    } else {
                        deliver(apiMsg->getDestKey(), tempMsg);
                    }
                }
            }
            break;
        }

        case KBR_FORWARD:
        {
            KBRforward* apiMsg = dynamic_cast<KBRforward*>(commonAPIMsg);
            OverlayKey key = apiMsg->getDestKey();
            NodeHandle nextHopNode = apiMsg->getNextHopNode();

            forward(&key, &tempMsg, &nextHopNode);

            //if message ist not deleted send it back
            if(tempMsg != NULL) {
                if(nextHopNode == apiMsg->getNextHopNode())
                    //do we need this?
                    nextHopNode = NodeHandle::UNSPECIFIED_NODE;
                forwardResponse(key, tempMsg, nextHopNode);
            }
            break;
        }

        case KBR_UPDATE:
        {
            KBRupdate* apiMsg = dynamic_cast<KBRupdate*>(commonAPIMsg);
            update(apiMsg->getNode(), apiMsg->getJoined());

            break;
        }

        default:
        {
            delete tempMsg;
        }
    }
    delete commonAPIMsg;
}

void BaseApp::handleUpperMessage(cMessage* msg)
{
    delete msg;
}

void BaseApp::handleLowerMessage(cMessage* msg)
{
    delete msg;
}

void BaseApp::handleUDPMessage(cMessage *msg)
{
    delete msg;
}

void BaseApp::handleReadyMessage(CompReadyMessage* msg)
{
    delete msg;
}

void BaseApp::handleTraceMessage(cMessage* msg)
{
    throw cRuntimeError("This application cannot handle trace data. "
                         "You have to overwrite handleTraceMessage() in your "
                         "application to make trace files work");
}

void BaseApp::sendMessageToLowerTier(cPacket* msg)
{
    RECORD_STATS(numOverlaySent++; bytesOverlaySent += msg->getByteLength());

    send(msg, "to_lowerTier");
}
void BaseApp::sendReadyMessage(bool ready)
{
    CompReadyMessage* msg = new CompReadyMessage();
    msg->setReady(ready);
    msg->setComp(getThisCompType());

    overlay->sendMessageToAllComp(msg, getThisCompType());
}

void BaseApp::finish()
{
    // record scalar data
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);

    string baseAppName = string("BaseApp (") += string(this->getName())
                         += string("): ");

    if (time >= GlobalStatistics::MIN_MEASURED) {
        globalStatistics->addStdDev(baseAppName + string("Sent Messages/s to "
                                                         "Overlay"),
                                    numOverlaySent / time);
        globalStatistics->addStdDev(baseAppName +
                                    string("Received Messages/s from Overlay"),
                                    numOverlayReceived / time);
        globalStatistics->addStdDev(baseAppName + string("Sent Bytes/s to "
                                                         "Overlay"),
                                    bytesOverlaySent / time);
        globalStatistics->addStdDev(baseAppName + string("Received Bytes/s "
                                                         "from Overlay"),
                                    bytesOverlayReceived / time);
        globalStatistics->addStdDev(baseAppName + string("Sent Messages/s to "
                                                         "UDP"),
                                    numUdpSent / time);
        globalStatistics->addStdDev(baseAppName +
                                    string("Received Messages/s from UDP"),
                                    numUdpReceived / time);
        globalStatistics->addStdDev(baseAppName + string("Sent Bytes/s to UDP"),
                                    bytesUdpSent / time);
        globalStatistics->addStdDev(baseAppName + string("Received Bytes/s "
                                                         "from UDP"),
                                    bytesUdpReceived / time);

    }

    finishApp();
}

void BaseApp::finishApp()
{
    // ...
}

void BaseApp::bindToPort(int port)
{
    EV << "[BaseApp::bindToPort() @ " << thisNode.getIp()
       << ":  Binding to UDP port " << port << endl;

    thisNode.setPort(port);
    socket.setOutputGate(gate("udpOut"));
    socket.bind(port);
}

void BaseApp::sendMessageToUDP(const TransportAddress& destAddr, cPacket *msg)
{
    // send message to UDP, with the appropriate control info attached
    msg->removeControlInfo();
    if (ev.isGUI()) {
        BaseRpcMessage* rpc = dynamic_cast<BaseRpcMessage*>(msg);
        if (rpc) rpc->setStatType(APP_DATA_STAT);
    }

    // debug message
    if (debugOutput) {
        EV << "[BaseApp::sendMessageToUDP() @ " << thisNode.getIp()
        << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
        << "    Sending " << *msg << " to " << destAddr.getIp()
        << endl;
    }
    RECORD_STATS(numUdpSent++; bytesUdpSent += msg->getByteLength());
    socket.sendTo(msg,destAddr.getIp(),destAddr.getPort());
}

//private
bool BaseApp::internalHandleRpcCall(BaseCallMessage* msg)
{
    // if RPC was handled return true, else tell the parent class to handle it
    return BaseRpc::internalHandleRpcCall(msg);
}

void BaseApp::internalHandleRpcResponse(BaseResponseMessage* msg,
                                        cPolymorphic* context,
                                        int rpcId, simtime_t rtt)
{
    // if RPC was handled return true, else tell the parent class to handle it
    BaseRpc::internalHandleRpcResponse(msg, context, rpcId, rtt);
}

void BaseApp::internalSendRouteRpc(BaseRpcMessage* message,
                                   const OverlayKey& destKey,
                                   const std::vector<TransportAddress>&
                                       sourceRoute,
                                   RoutingType routingType) {
    callRoute(destKey, message, sourceRoute, routingType);
}

void BaseApp::internalSendRpcResponse(BaseCallMessage* call,
                                      BaseResponseMessage* response)
{
    // default values for UDP transport
    TransportType transportType = UDP_TRANSPORT;
    CompType compType = INVALID_COMP;
    const TransportAddress* destNode = &TransportAddress::UNSPECIFIED_NODE;//&(call->getSrcNode());
    const OverlayKey* destKey = &OverlayKey::UNSPECIFIED_KEY;

    TransportAddress tempNode;

    OverlayCtrlInfo* overlayCtrlInfo =
        dynamic_cast<OverlayCtrlInfo*>(call->getControlInfo());

    if (overlayCtrlInfo &&
        overlayCtrlInfo->getTransportType() == ROUTE_TRANSPORT) {
        //destNode = &(overlayCtrlInfo->getSrcNode());
        if (overlayCtrlInfo->getSrcNode().isUnspecified())
            destNode = &(overlayCtrlInfo->getLastHop());
        else
            destNode = &(overlayCtrlInfo->getSrcNode());
        transportType = ROUTE_TRANSPORT;
        compType = static_cast<CompType>(overlayCtrlInfo->getSrcComp());
        if (static_cast<RoutingType>(overlayCtrlInfo->getRoutingType())
                == FULL_RECURSIVE_ROUTING) {
            destKey = &(overlayCtrlInfo->getSrcNode().getKey());//&(call->getSrcNode().getKey());
            destNode = &NodeHandle::UNSPECIFIED_NODE;
        }
    } else {
        UDPDataIndication* udpDataIndicationInfo =
            check_and_cast<UDPDataIndication*>(call->getControlInfo());

        tempNode = TransportAddress(udpDataIndicationInfo->getSrcAddr(), udpDataIndicationInfo->getSrcPort());
        destNode = &tempNode;

    }

    sendRpcResponse(transportType, compType,
                    *destNode, *destKey, call, response);
}
