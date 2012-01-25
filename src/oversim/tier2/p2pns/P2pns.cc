//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file P2pns.cc
 * @author Ingmar Baumgart
 */

#include <XmlRpcInterface.h>
#include <P2pnsMessage_m.h>

#include "P2pns.h"

Define_Module(P2pns);

using namespace std;

P2pns::P2pns()
{
    p2pnsCache = NULL;
}

P2pns::~P2pns()
{
}

void P2pns::initializeApp(int stage)
{
    if (stage != MIN_STAGE_APP)
        return;

    twoStageResolution = par("twoStageResolution");
    keepaliveInterval = par("keepaliveInterval");
    idCacheLifetime = par("idCacheLifetime");

    p2pnsCache = check_and_cast<P2pnsCache*> (getParentModule()->
            getSubmodule("p2pnsCache"));

    xmlRpcInterface = dynamic_cast<XmlRpcInterface*>(overlay->
            getCompModule(TIER3_COMP));
}

void P2pns::handleReadyMessage(CompReadyMessage* msg)
{
    if ((msg->getReady() == false) || (msg->getComp() != OVERLAY_COMP)) {
        delete msg;
        return;
    }

    thisId = (overlay->getThisNode().getKey() >> (OverlayKey::getLength() - 100))
             << (OverlayKey::getLength() - 100);
    delete msg;
}

void P2pns::tunnel(const OverlayKey& destKey, const BinaryValue& payload)
{
    Enter_Method_Silent();

    P2pnsIdCacheEntry* entry = p2pnsCache->getIdCacheEntry(destKey);

    if (entry == NULL) {
        // lookup destKey and create new entry

        EV << "[P2pns::tunnel()]\n"
           << "    Establishing new cache entry for key: " << destKey
           << endl;

        LookupCall* lookupCall = new LookupCall();
        lookupCall->setKey(destKey);
        lookupCall->setNumSiblings(1);
        sendInternalRpcCall(OVERLAY_COMP, lookupCall, NULL, -1, 0,
                            TUNNEL_LOOKUP);
        p2pnsCache->addIdCacheEntry(destKey, &payload);
    } else if (entry->state == CONNECTION_PENDING) {
        // lookup not finished yet => append packet to queue
        EV << "[P2pns::tunnel()]\n"
           << "    Queuing packet since lookup is still pending for key: "
           << destKey << endl;

        entry->lastUsage = simTime();
        entry->payloadQueue.push_back(payload);
    } else {
        entry->lastUsage = simTime();
        sendTunnelMessage(entry->addr, payload);
    }
}

void P2pns::handleTunnelLookupResponse(LookupResponse* lookupResponse)
{
    P2pnsIdCacheEntry* entry =
        p2pnsCache->getIdCacheEntry(lookupResponse->getKey());

    if ((entry == NULL) || (entry->state == CONNECTION_ACTIVE)) {
        // no matching entry in idCache or connection is already active
        // => lookup result not needed anymore
        return;
    }

    if (lookupResponse->getIsValid()) {
        // verify if nodeId of lookup's closest node matches requested id
        if (lookupResponse->getKey().sharedPrefixLength(lookupResponse->
                      getSiblings(0).getKey()) < (OverlayKey::getLength() - 100)) {
            EV << "[P2pns::handleTunnelLookupResponse()]\n"
                   << "    Lookup response " << lookupResponse->getSiblings(0)
                   << " doesn't match requested id " << lookupResponse->getKey()
                   << endl;
            // lookup failed => drop cache entry and all queued packets
            p2pnsCache->removeIdCacheEntry(lookupResponse->getKey());

            return;
        }

        // add transport address to cache entry
        entry->addr = lookupResponse->getSiblings(0);
        entry->state = CONNECTION_ACTIVE;

        // start periodic ping timer
        P2pnsKeepaliveTimer* msg =
            new P2pnsKeepaliveTimer("P2pnsKeepaliveTimer");
        msg->setKey(lookupResponse->getKey());
        scheduleAt(simTime() + keepaliveInterval, msg);

        // send all pending tunnel messages
        while (!entry->payloadQueue.empty()) {
            sendTunnelMessage(entry->addr, entry->payloadQueue.front());
            entry->payloadQueue.pop_front();
        }
    } else {
        // lookup failed => drop cache entry and all queued packets
        p2pnsCache->removeIdCacheEntry(lookupResponse->getKey());
    }
}

void P2pns::sendTunnelMessage(const TransportAddress& addr,
                              const BinaryValue& payload)
{
    EV << "[P2pns::sendTunnelMessage()]\n"
           << "    Sending TUNNEL message to " << addr << endl;

    P2pnsTunnelMessage* msg = new P2pnsTunnelMessage("P2pnsTunnelMsg");
    msg->setPayload(payload);
    msg->setSrcId(thisId);
    msg->setBitLength(P2PNSTUNNELMESSAGE_L(msg));

    callRoute(OverlayKey::UNSPECIFIED_KEY, msg, addr);
}

void P2pns::registerId(const std::string& addr)
{
    Enter_Method_Silent();

    std::string name = par("registerName").stdstringValue();
    DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();

    EV << "[P2pns::p2pnsRegisterRpc()]\n"
       << "    registerId(): name: " << name << " addr: " << addr
       << endl;

    dhtPutMsg->setKey(OverlayKey::sha1(BinaryValue(name)));

    dhtPutMsg->setValue(BinaryValue(addr));
    dhtPutMsg->setKind(28);
    dhtPutMsg->setId(1);
    dhtPutMsg->setTtl(60*60*24*7);
    dhtPutMsg->setIsModifiable(true);

    sendInternalRpcCall(TIER1_COMP, dhtPutMsg);
}

void P2pns::deliver(OverlayKey& key, cMessage* msg)
{
    P2pnsTunnelMessage* tunnelMsg = check_and_cast<P2pnsTunnelMessage*>(msg);

    if (xmlRpcInterface) {
        xmlRpcInterface->deliverTunneledMessage(tunnelMsg->getPayload());
    }

    updateIdCacheWithNewTransport(msg);

    delete msg;
}

void P2pns::pingRpcResponse(PingResponse* response, cPolymorphic* context,
                            int rpcId, simtime_t rtt)
{
    delete context;
}

void P2pns::pingTimeout(PingCall* call, const TransportAddress& dest,
                        cPolymorphic* context, int rpcId)
{
    OverlayKeyObject* key = dynamic_cast<OverlayKeyObject*>(context);
    P2pnsIdCacheEntry* entry = NULL;

    // lookup entry in id cache
    if ((key != NULL) &&
            (entry = p2pnsCache->getIdCacheEntry(*key))) {

        // remove entry if TransportAddress hasn't been updated in the meantime
        if (!entry->addr.isUnspecified() && (entry->addr != dest)) {
            EV << "[P2pns::pingTimeout()]\n"
               << "    Removing id " << key << " from idCache (ping timeout)"
               << endl;
            p2pnsCache->removeIdCacheEntry(*key);
        }
    }

    delete context;
}

void P2pns::handleTimerEvent(cMessage* msg)
{
    P2pnsKeepaliveTimer* timer = dynamic_cast<P2pnsKeepaliveTimer*>(msg);

    if (timer) {
        P2pnsIdCacheEntry* entry = p2pnsCache->getIdCacheEntry(timer->getKey());

        if (entry == NULL) {
            // no valid cache entry found
            delete msg;
            return;
        }

        if ((entry->lastUsage + idCacheLifetime) < simTime()) {
            // remove idle connections
            EV << "[P2pns::handleTimerEvent()]\n"
               << "    Removing id " << timer->getKey()
               << " from idCache (connection idle)"
               << endl;
            p2pnsCache->removeIdCacheEntry(timer->getKey());
            delete msg;
            return;
        }

        if (!entry->addr.isUnspecified()) {
            // ping 3 times with default timeout
            pingNode(entry->addr, -1, 2, new OverlayKeyObject(timer->getKey()));
        }

        // reschedule periodic keepalive timer
        scheduleAt(simTime() + keepaliveInterval, msg);

        // exhaustive-iterative lookup to refresh siblings, if our ip has changed
        LookupCall* lookupCall = new LookupCall();
        lookupCall->setKey(overlay->getThisNode().getKey());
        lookupCall->setNumSiblings(1);
        lookupCall->setRoutingType(EXHAUSTIVE_ITERATIVE_ROUTING);
        sendInternalRpcCall(OVERLAY_COMP, lookupCall, NULL, -1, 0,
                            TUNNEL_LOOKUP);
    }
}

void P2pns::updateIdCacheWithNewTransport(cMessage* msg)
{
    // update idCache with new TransportAddress of the ping originator
    OverlayCtrlInfo* ctrlInfo =
        dynamic_cast<OverlayCtrlInfo*>(msg->getControlInfo());

    OverlayKey srcId;

    if (!ctrlInfo) {
        // can't update cache without knowing the originator id
        EV << "[P2pns::updateCacheWithNewTransport()]\n"
           << "    Can't update cache without knowing the originator id"
           << endl;
        return;
    }

    if (ctrlInfo->getSrcRoute().isUnspecified()) {
        P2pnsTunnelMessage* tunnelMsg = dynamic_cast<P2pnsTunnelMessage*>(msg);
        if (tunnelMsg) {
            srcId = tunnelMsg->getSrcId();
        } else {
            // can't update cache without knowing the originator id
            EV << "[P2pns::updateCacheWithNewTransport()]\n"
               << "    Can't update cache without knowing the originator id"
               << endl;
            return;
        }
    } else {
        srcId = (ctrlInfo->getSrcRoute().getKey() >> (OverlayKey::getLength() - 100))
                << (OverlayKey::getLength() - 100);
    }

    P2pnsIdCacheEntry* entry = p2pnsCache->getIdCacheEntry(srcId);

    if (entry == NULL) {
        EV << "[P2pns::updateCacheWithNewTransport()]\n"
           << "    Adding new cache entry for id " << srcId
           << " with addr " << (const TransportAddress&)ctrlInfo->getSrcRoute()
           << endl;
        entry = p2pnsCache->addIdCacheEntry(srcId);
        entry->addr = ctrlInfo->getSrcRoute();

        // start periodic ping timer
        P2pnsKeepaliveTimer* msg =
            new P2pnsKeepaliveTimer("P2pnsKeepaliveTimer");
        msg->setKey(srcId);
        scheduleAt(simTime() + keepaliveInterval, msg);
    }

    // update transport address in idCache (node may have a new
    // TransportAddress due to mobility)
    if (entry->addr.isUnspecified() ||
            (entry->addr != ctrlInfo->getSrcRoute())) {
        EV << "[P2pns::handleRpcCall()]\n"
           << "    Ping with new transport address received: "
           << "    Changing from " << entry->addr << " to "
           << static_cast<TransportAddress>(ctrlInfo->getSrcRoute())
           << " for id " << srcId << endl;
        entry->addr = ctrlInfo->getSrcRoute();
    }

    entry->state = CONNECTION_ACTIVE;
}

bool P2pns::handleRpcCall(BaseCallMessage* msg)
{
    // delegate messages
    RPC_SWITCH_START(msg)
    RPC_ON_CALL(Ping) {
        updateIdCacheWithNewTransport(msg);
        return false;
    }
    RPC_DELEGATE( P2pnsRegister, p2pnsRegisterRpc );
    RPC_DELEGATE( P2pnsResolve, p2pnsResolveRpc );
    RPC_SWITCH_END()

    return RPC_HANDLED;
}


void P2pns::handleRpcResponse(BaseResponseMessage* msg,
                              cPolymorphic* context, int rpcId, simtime_t rtt)
{
    RPC_SWITCH_START(msg)
    RPC_ON_RESPONSE( DHTputCAPI ) {
        EV << "[P2pns::handleRpcResponse()]\n"
           << "    DHTputCAPI RPC Response received: id=" << rpcId
           << " msg=" << *_DHTputCAPIResponse << " rtt=" << rtt
           << endl;
        if (dynamic_cast<P2pnsRegisterCall*>(context)) {
            handleDHTputCAPIResponse(_DHTputCAPIResponse,
                                 check_and_cast<P2pnsRegisterCall*>(context));
        }
        break;
    }
    RPC_ON_RESPONSE( DHTgetCAPI ) {
        EV << "[P2pns::handleRpcResponse()]\n"
           << "    DHTgetCAPI RPC Response received: id=" << rpcId
           << " msg=" << *_DHTgetCAPIResponse << " rtt=" << rtt
           << endl;
        handleDHTgetCAPIResponse(_DHTgetCAPIResponse,
                                 check_and_cast<P2pnsResolveCall*>(context));
        break;
    }
    RPC_ON_RESPONSE( Lookup ) {
        EV << "[P2pns::handleRpcResponse()]\n"
           << "    Lookup RPC Response received: id=" << rpcId
           << " msg=" << *_LookupResponse << " rtt=" << rtt
           << endl;
        handleLookupResponse(_LookupResponse, context, rpcId);
        break;
    }
    RPC_SWITCH_END()
}

void P2pns::p2pnsRegisterRpc(P2pnsRegisterCall* registerCall)
{
    p2pnsCache->addData(registerCall->getP2pName(),
                        registerCall->getAddress());

    DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();

    EV << "[P2pns::p2pnsRegisterRpc()]\n"
       << "    RegisterRpc: name: " << registerCall->getP2pName()
       << " addr: " << registerCall->getAddress()
       << endl;

    dhtPutMsg->setKey(OverlayKey::sha1(registerCall->getP2pName()));
    if (twoStageResolution) {
        dhtPutMsg->setValue(overlay->getThisNode().getKey().toString());
    } else {
        dhtPutMsg->setValue(registerCall->getAddress());
    }

    dhtPutMsg->setKind(registerCall->getKind());
    dhtPutMsg->setId(registerCall->getId());
    dhtPutMsg->setTtl(registerCall->getTtl());
    dhtPutMsg->setIsModifiable(true);

    sendInternalRpcCall(TIER1_COMP, dhtPutMsg, registerCall);
}

void P2pns::p2pnsResolveRpc(P2pnsResolveCall* resolveCall)
{
    DHTgetCAPICall* dhtGetMsg = new DHTgetCAPICall();

    EV << "[P2pns::p2pnsResolveRpc()]\n"
       << "   ResolveRpc: name: " << resolveCall->getP2pName()
       << endl;

    dhtGetMsg->setKey(OverlayKey::sha1(resolveCall->getP2pName()));
    dhtGetMsg->setKind(resolveCall->getKind());
    dhtGetMsg->setId(resolveCall->getId());

    sendInternalRpcCall(TIER1_COMP, dhtGetMsg, resolveCall);
}

void P2pns::handleDHTputCAPIResponse(DHTputCAPIResponse* putResponse,
                                     P2pnsRegisterCall* registerCall)
{
    P2pnsRegisterResponse* registerResponse = new P2pnsRegisterResponse();
    registerResponse->setP2pName(registerCall->getP2pName());
    registerResponse->setAddress(registerCall->getAddress());
    registerResponse->setIsSuccess(putResponse->getIsSuccess());
    sendRpcResponse(registerCall, registerResponse);
}

void P2pns::handleDHTgetCAPIResponse(DHTgetCAPIResponse* getResponse,
                                     P2pnsResolveCall* resolveCall)
{
    if ((!getResponse->getIsSuccess())
            || (getResponse->getResultArraySize() == 0)) { // || (valueStream.str().size() == 0)) {
        P2pnsResolveResponse* resolveResponse = new P2pnsResolveResponse();
        resolveResponse->setAddressArraySize(1);
        resolveResponse->setKindArraySize(1);
        resolveResponse->setIdArraySize(1);
        resolveResponse->setP2pName(resolveCall->getP2pName());
        resolveResponse->setAddress(0, BinaryValue(""));
        resolveResponse->setKind(0, 0);
        resolveResponse->setId(0, 0);
        resolveResponse->setIsSuccess(false);
        sendRpcResponse(resolveCall, resolveResponse);
        return;
    }

//    TODO: fix cache to support kind and id of data records
//    p2pnsCache->addData(resolveCall->getP2pName(),
//                        getResponse->getValue());

    if (twoStageResolution) {
        if (getResponse->getResultArraySize() != 1) {
            throw cRuntimeError("P2pns::handleDHTgetCAPIResponse: "
                                "Two-stage name resolution currently only "
                                "works with unique keys!");
        }
        std::stringstream valueStream;
        valueStream << getResponse->getResult(0).getValue();
        OverlayKey key(valueStream.str(), 16);

        LookupCall* lookupCall = new LookupCall();

        lookupCall->setKey(key);
        lookupCall->setNumSiblings(1);

        sendInternalRpcCall(OVERLAY_COMP, lookupCall, resolveCall, -1, 0,
                            RESOLVE_LOOKUP);

        return;
    }

    EV << "[P2pns::handleDHTgetCAPIResponse()]\n"
       << "   ResolveRpcResponse: name: " << resolveCall->getP2pName();

    P2pnsResolveResponse* resolveResponse = new P2pnsResolveResponse();
    resolveResponse->setP2pName(resolveCall->getP2pName());
    resolveResponse->setIsSuccess(getResponse->getIsSuccess());
    resolveResponse->setAddressArraySize(getResponse->getResultArraySize());
    resolveResponse->setKindArraySize(getResponse->getResultArraySize());
    resolveResponse->setIdArraySize(getResponse->getResultArraySize());

    for (uint i = 0; i < getResponse->getResultArraySize(); i++) {
        EV << " addr: " << getResponse->getResult(i).getValue();
        resolveResponse->setAddress(i, getResponse->getResult(i).getValue());
        resolveResponse->setKind(i, getResponse->getResult(i).getKind());
        resolveResponse->setId(i, getResponse->getResult(i).getId());
    }

    EV << endl;
    sendRpcResponse(resolveCall, resolveResponse);
}

void P2pns::handleLookupResponse(LookupResponse* lookupResponse,
                                 cObject* context,
                                 int rpcId)
{
    switch (rpcId) {
    case RESOLVE_LOOKUP: {
        P2pnsResolveCall* resolveCall =
            check_and_cast<P2pnsResolveCall*>(context);

        stringstream sstream;
        sstream << lookupResponse->getSiblings(0);

        P2pnsResolveResponse* resolveResponse = new P2pnsResolveResponse();
        resolveResponse->setP2pName(resolveCall->getP2pName());
        resolveResponse->setAddressArraySize(1);
        resolveResponse->setKindArraySize(1);
        resolveResponse->setIdArraySize(1);

        resolveResponse->setAddress(0, sstream.str());
        resolveResponse->setKind(0, 0);
        resolveResponse->setId(0, 0);
        resolveResponse->setIsSuccess(lookupResponse->getIsValid());
        sendRpcResponse(resolveCall, resolveResponse);
        break;
    }
    case TUNNEL_LOOKUP:
        handleTunnelLookupResponse(lookupResponse);
        break;
    case REFRESH_LOOKUP:
        break;
    default:
        throw cRuntimeError("P2pns::handleLookupResponse(): invalid rpcId!");
    }
}


void P2pns::finishApp()
{
}

