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
 * @file DHT.cc
 * @author Ingmar Baumgart
 */

#include <IPvXAddressResolver.h>

#include "CBR-DHT.h"

#include <RpcMacros.h>
#include <BaseRpc.h>
#include <GlobalStatistics.h>
#include <CoordBasedRoutingAccess.h>
#include <NeighborCache.h>

Define_Module(CBRDHT);

using namespace std;

CBRDHT::CBRDHT()
{
    dataStorage = NULL;
}

CBRDHT::~CBRDHT()
{
    std::map<unsigned int, BaseCallMessage*>::iterator it;

    for (it = rpcIdMap.begin(); it != rpcIdMap.end(); it++) {
        delete it->second;
        it->second = NULL;
    }

    std::map<int, GetMapEntry>::iterator it2;
    for (it2 = getMap.begin(); it2 != getMap.end(); it2++) {
        //cancelAndDelete(it2->second.callMsg);
        delete it2->second.callMsg;
        it2->second.callMsg = NULL;
    }

    std::map<int, PutMapEntry>::iterator it3;

    for (it3 = putMap.begin(); it3 != putMap.end(); it3++) {
        //if (it3->second.callMsg != NULL) {
        //    cancelAndDelete(it3->second.callMsg);
        //}
        delete it3->second.callMsg;
        it3->second.callMsg = NULL;
    }

    rpcIdMap.clear();
    getMap.clear();
    putMap.clear();

    if (dataStorage != NULL) {
        dataStorage->clear();
    }
}

void CBRDHT::initializeApp(int stage)
{
	if (stage != MIN_STAGE_APP)
        return;

    dataStorage = check_and_cast<DHTDataStorage*>
                      (getParentModule()->getSubmodule("dhtDataStorage"));

    coordBasedRouting = CoordBasedRoutingAccess().get();
    neighborCache = (NeighborCache*)getParentModule()
        ->getParentModule()->getSubmodule("neighborCache");

    numReplica = par("numReplica");
    numReplicaTeams = par("numReplicaTeams");

    if (numReplica > numReplicaTeams * overlay->getMaxNumSiblings()) {
        opp_error("DHT::initialize(): numReplica bigger than what this "
                  "overlay can handle (%d)", numReplicaTeams*overlay->getMaxNumSiblings());
    }

    maintenanceMessages = 0;
    normalMessages = 0;
    numBytesMaintenance = 0;
    numBytesNormal = 0;
    WATCH(maintenanceMessages);
    WATCH(normalMessages);
    WATCH(numBytesNormal);
    WATCH(numBytesMaintenance);
    WATCH_MAP(rpcIdMap);
}

void CBRDHT::handleTimerEvent(cMessage* msg)
{
    DHTTtlTimer* msg_timer = dynamic_cast<DHTTtlTimer*> (msg);

    if (msg_timer) {
        EV << "[DHT::handleTimerEvent()]\n"
           << "    received timer ttl, key: "
           << msg_timer->getKey().toString(16)
           << "\n (overlay->getThisNode().key = "
           << overlay->getThisNode().getKey().toString(16) << ")"
           << endl;

        dataStorage->removeData(msg_timer->getKey(), msg_timer->getKind(),
                                msg_timer->getId());
        //delete msg_timer;
    }
    /*DHTTtlTimer* msg_timer;

    if (msg->isName("ttl_timer")) {
        msg_timer = check_and_cast<DHTTtlTimer*> (msg);

        EV << "[DHT::handleTimerEvent()]\n"
           << "    received timer ttl, key: "
           << msg_timer->getKey().toString(16)
           << "\n (overlay->getThisNode().key = "
           << overlay->getThisNode().getKey().toString(16) << ")"
           << endl;

        dataStorage->removeData(msg_timer->getKey(), msg_timer->getKind(),
                                msg_timer->getId());
        delete msg_timer;
    }*/
}

bool CBRDHT::handleRpcCall(BaseCallMessage* msg)
{
    // delegate messages
    RPC_SWITCH_START( msg )
        // RPC_DELEGATE( <messageName>[Call|Response], <methodToCall> )
        RPC_DELEGATE( DHTPut, handlePutRequest );
        RPC_DELEGATE( CBRDHTGet, handleGetRequest );
        RPC_DELEGATE( DHTputCAPI, handlePutCAPIRequest ); //requests coming from an upper tier
        RPC_DELEGATE( DHTgetCAPI, handleGetCAPIRequest );
        RPC_DELEGATE( DHTdump, handleDumpDhtRequest );
    RPC_SWITCH_END( )

    return RPC_HANDLED;
}

void CBRDHT::handleRpcResponse(BaseResponseMessage* msg, cPolymorphic* context,
                            int rpcId, simtime_t rtt)
{
    RPC_SWITCH_START(msg)
        RPC_ON_RESPONSE(DHTPut){
        handlePutResponse(_DHTPutResponse, rpcId);
        EV << "[DHT::handleRpcResponse()]\n"
           << "    DHT Put RPC Response received: id=" << rpcId
           << " msg=" << *_DHTPutResponse << " rtt=" << rtt
           << endl;
        break;
    }
    RPC_ON_RESPONSE(CBRDHTGet) {
        handleGetResponse(_CBRDHTGetResponse, rpcId);
        EV << "[DHT::handleRpcResponse()]\n"
           << "    DHT Get RPC Response received: id=" << rpcId
           << " msg=" << *_CBRDHTGetResponse << " rtt=" << rtt
           << endl;
        break;
    }
    RPC_ON_RESPONSE(Lookup) {
        handleLookupResponse(_LookupResponse);
        EV << "[DHT::handleRpcResponse()]\n"
           << "    Replica Set RPC Response received: id=" << rpcId
           << " msg=" << *_LookupResponse << " rtt=" << rtt
           << endl;
        break;
    }
    RPC_SWITCH_END()
}

void CBRDHT::handleRpcTimeout(BaseCallMessage* msg, const TransportAddress& dest,
                           cPolymorphic* context, int rpcId,
                           const OverlayKey& destKey)
{
    RPC_SWITCH_START(msg)
    RPC_ON_CALL(DHTPut){
        EV << "[DHT::handleRpcResponse()]\n"
           << "    DHTPut Timeout"
           << endl;

        std::map<int, PutMapEntry>::iterator it2 =
                putMap.find(rpcId);

        if (it2 == putMap.end()) //unknown request
            return;

        it2->second.numFailed++;

        if (it2->second.numFailed / (double)it2->second.numSent >= 0.5) {
            DHTputCAPIResponse* capiPutRespMsg = new DHTputCAPIResponse();
            capiPutRespMsg->setIsSuccess(false);
            sendRpcResponse(it2->second.callMsg, capiPutRespMsg);
            it2->second.callMsg = NULL;
            putMap.erase(rpcId);
        }
        break;
    }
    RPC_ON_CALL(CBRDHTGet) {
        EV << "[DHT::handleRpcResponse()]\n"
           << "    DHTGet Timeout"
           << endl;

        std::map<int, GetMapEntry>::iterator it2 =
            getMap.find(rpcId);

        if (it2 == getMap.end()) //unknown request
            return;

        if (it2->second.replica.size() > 0) {
            // Received empty value, try fallback replica
            NodeHandle fallbackReplica = it2->second.replica.back();
            CBRDHTGetCall* dhtRecall = new CBRDHTGetCall();
            dhtRecall->setOriginalKey(_CBRDHTGetCall->getOriginalKey());
            dhtRecall->setKey(_CBRDHTGetCall->getKey());
            dhtRecall->setIsHash(false);
            dhtRecall->setBitLength(GETCALL_L(dhtRecall));
            RECORD_STATS(normalMessages++;
            numBytesNormal += dhtRecall->getByteLength());
            sendRouteRpcCall(TIER1_COMP, fallbackReplica, dhtRecall,
                             NULL, DEFAULT_ROUTING, -1, 0,
                             it2->second.callMsg->getNonce());
            it2->second.numSent++;
            it2->second.replica.pop_back();
            return;
        } else if (it2->second.teamNumber < (numReplicaTeams - 1)) {
            // No more fallback replica in this team, try next one
            it2->second.teamNumber++;
            handleGetCAPIRequest(it2->second.callMsg, it2->second.teamNumber);
            return;
        } else {
            // No more replica, no more teams, send success == false to Tier 2 :(
            DHTgetCAPIResponse* capiGetRespMsg = new DHTgetCAPIResponse();
            //capiGetRespMsg->setKey(_CBRDHTGetCall->getOriginalKey());
            //capiGetRespMsg->setValue(BinaryValue::UNSPECIFIED_VALUE);
            DhtDumpEntry result;
            result.setKey(_CBRDHTGetCall->getKey());
            result.setValue(BinaryValue::UNSPECIFIED_VALUE);
            capiGetRespMsg->setResultArraySize(1);
            capiGetRespMsg->setResult(0, result);
            capiGetRespMsg->setIsSuccess(false);
            sendRpcResponse(it2->second.callMsg, capiGetRespMsg);
            getMap.erase(rpcId);
        }
        break;
    }
    RPC_SWITCH_END( )
}

void CBRDHT::handleUpperMessage(cMessage* msg)
{
    error("DHT::handleUpperMessage(): Received message with unknown type!");

    delete msg;
}

void CBRDHT::handlePutRequest(DHTPutCall* dhtMsg)
{
    std::string tempString = "PUT_REQUEST received: "
            + std::string(dhtMsg->getKey().toString(16));
    getParentModule()->getParentModule()->bubble(tempString.c_str());

    if (!(dataStorage->isModifiable(dhtMsg->getKey(), dhtMsg->getKind(),
                                    dhtMsg->getId()))) {
        //check if the put request came from the right node
        NodeHandle sourceNode = dataStorage->getSourceNode(dhtMsg->getKey(),
                                                           dhtMsg->getKind(), dhtMsg->getId());
        if (((!sourceNode.isUnspecified())
                && (!dhtMsg->getSrcNode().isUnspecified()) && (sourceNode
                != dhtMsg->getSrcNode())) || ((dhtMsg->getMaintenance())
                && (dhtMsg->getOwnerNode() == sourceNode))) {
            // TODO: set owner
            DHTPutResponse* responseMsg = new DHTPutResponse();
            responseMsg->setSuccess(false);
            responseMsg->setBitLength(PUTRESPONSE_L(responseMsg));
            RECORD_STATS(normalMessages++;
                         numBytesNormal += responseMsg->getByteLength());
            sendRpcResponse(dhtMsg, responseMsg);
            return;
        }

    }

    // remove data item from local data storage
    //cancelAndDelete(dataStorage->getTtlMessage(dhtMsg->getKey()));
    //dataStorage->removeData(dhtMsg->getKey());
    dataStorage->removeData(dhtMsg->getKey(), dhtMsg->getKind(),
                                dhtMsg->getId());
    if (dhtMsg->getValue().size() > 0) {
        // add ttl timer
        DHTTtlTimer *timerMsg = new DHTTtlTimer("ttl_timer");
        timerMsg->setKey(dhtMsg->getKey());
        scheduleAt(simTime() + dhtMsg->getTtl(), timerMsg);
        // storage data item in local data storage
        bool err;
        dataStorage->addData(dhtMsg->getKey(), dhtMsg->getKind(),
                             dhtMsg->getId(), dhtMsg->getValue(), timerMsg,
                             dhtMsg->getIsModifiable(), dhtMsg->getSrcNode(),
                             overlay->isSiblingFor(overlay->getThisNode(),
                                                   dhtMsg->getKey(),
                                                   1, &err));
    }

    // send back
    DHTPutResponse* responseMsg = new DHTPutResponse();

    responseMsg->setSuccess(true);
    responseMsg->setBitLength(PUTRESPONSE_L(responseMsg));
    RECORD_STATS(normalMessages++; numBytesNormal += responseMsg->getByteLength());

    sendRpcResponse(dhtMsg, responseMsg);
}

void CBRDHT::handleGetRequest(CBRDHTGetCall* dhtMsg)
{
    std::string tempString = "GET_REQUEST received: "
            + std::string(dhtMsg->getKey().toString(16));

    getParentModule()->getParentModule()->bubble(tempString.c_str());

    BinaryValue storedValue;
    DhtDataEntry* dataEntry = dataStorage->getDataEntry(dhtMsg->getKey(), 1, 1);
    if (dataEntry) {
        storedValue = dataStorage->getDataEntry(dhtMsg->getKey(), 1, 1)->value;
    } else {
        storedValue = BinaryValue::UNSPECIFIED_VALUE;
    }

    // send back
    CBRDHTGetResponse* responseMsg = new CBRDHTGetResponse();

    responseMsg->setKey(dhtMsg->getKey());
    responseMsg->setOriginalKey(dhtMsg->getOriginalKey());
    responseMsg->setIsHash(false);
    if (storedValue.isUnspecified()) {
        //responseMsg->setValue(BinaryValue::UNSPECIFIED_VALUE);
        DhtDumpEntry result;
        result.setKey(dhtMsg->getKey());
        result.setValue(BinaryValue::UNSPECIFIED_VALUE);
        responseMsg->setResultArraySize(1);
        responseMsg->setResult(0, result);
    } else {
        //responseMsg->setValue(storedValue);
        DhtDumpEntry result;
        result.setKey(dhtMsg->getKey());
        result.setValue(storedValue);
        responseMsg->setResultArraySize(1);
        responseMsg->setResult(0, result);
    }
    rpcIdMap.insert(make_pair(dhtMsg->getNonce(), (BaseCallMessage*)NULL));

    responseMsg->setBitLength(GETRESPONSE_L(responseMsg));
    RECORD_STATS(normalMessages++;
                 numBytesNormal += responseMsg->getByteLength());
    sendRpcResponse(dhtMsg, responseMsg);
}

void CBRDHT::handlePutCAPIRequest(DHTputCAPICall* capiPutMsg)
{
    // provide copies of this message for other teams
    for (int i = 1; i < numReplicaTeams; i++) {
        DHTPutCall* teamCopyPutMsg = new DHTPutCall; //TODO memleak

        // transfer attributes of original DHTputCAPICall to DHTPutCall for teams
        teamCopyPutMsg->setValue(capiPutMsg->getValue());
        teamCopyPutMsg->setTtl(capiPutMsg->getTtl());
        teamCopyPutMsg->setIsModifiable(capiPutMsg->getIsModifiable());
        teamCopyPutMsg->setKind(capiPutMsg->getKind());
        teamCopyPutMsg->setId(capiPutMsg->getId());

        // control info needs to be copied by value
        OverlayCtrlInfo controlInfo = *(check_and_cast<OverlayCtrlInfo*>(capiPutMsg->getControlInfo()));
        OverlayCtrlInfo* controlInfoCopy = new OverlayCtrlInfo;
        *controlInfoCopy = controlInfo;
        teamCopyPutMsg->setControlInfo(controlInfoCopy);

        // multiple SHA1 hashing of original key
        OverlayKey destKey = capiPutMsg->getKey();
        for (int j = 0; j < i; j++) {
            destKey = OverlayKey::sha1(BinaryValue(destKey.toString(16).c_str()));
        }
        teamCopyPutMsg->setKey(destKey);

        // rest is analog to handlePutCAPIRequest, but for DHTPutCall
        LookupCall* replicaMsg = new LookupCall();
        replicaMsg->setKey(teamCopyPutMsg->getKey());
        replicaMsg->setNumSiblings(floor(numReplica / numReplicaTeams));
        int nonce = sendInternalRpcCall(OVERLAY_COMP, replicaMsg);
        rpcIdMap.insert(make_pair(nonce, teamCopyPutMsg));
    }

    //asks the replica list
    LookupCall* replicaMsg = new LookupCall();
    replicaMsg->setKey(capiPutMsg->getKey());
    replicaMsg->setNumSiblings(floor(numReplica / numReplicaTeams));
    int nonce = sendInternalRpcCall(OVERLAY_COMP, replicaMsg);
    rpcIdMap.insert(make_pair(nonce, capiPutMsg));
}

void CBRDHT::handleGetCAPIRequest(DHTgetCAPICall* capiGetMsg, int teamnum) {
    // Extended multi team version, default: teamnum = 0
	if (teamnum >= numReplicaTeams)
		return;

	OverlayKey originalKey = capiGetMsg->getKey();
	std::vector<OverlayKey> possibleKeys;

	assert(!originalKey.isUnspecified());
	possibleKeys.push_back(originalKey);

	for (int i = 1; i < numReplicaTeams; i++) {
		// multiple SHA1 hashing of original key
		OverlayKey keyHash = originalKey;
		for (int j = 0; j < i; j++) {
			keyHash = OverlayKey::sha1(BinaryValue(keyHash.toString(16).c_str()));
		}
		assert(!keyHash.isUnspecified());
		possibleKeys.push_back(keyHash);
	}

    // Order possible keys by euclidian distance to this node
    std::vector<OverlayKey> orderedKeys;
    OverlayKey compareKey = overlay->getThisNode().getKey();

    while (possibleKeys.size() > 0) {
        OverlayKey bestKey = possibleKeys[0];
        int bestpos = 0;

        // TODO: i = 1?
        for (uint i = 0; i < possibleKeys.size(); i++) {
            //std::cout << neighborCache->getOwnEuclidianDistanceToKey(possibleKeys[i]) << std::endl;
            if (coordBasedRouting
                    ->getEuclidianDistanceByKeyAndCoords(possibleKeys[i],
                                                         ((const Nps&)neighborCache->getNcsAccess()).getOwnCoordinates(), //TODO
                                                         overlay->getBitsPerDigit()) <
                coordBasedRouting
                    ->getEuclidianDistanceByKeyAndCoords(bestKey,
                                                         ((const Nps&)neighborCache->getNcsAccess()).getOwnCoordinates(), //TODO
                                                         overlay->getBitsPerDigit())) {
                bestKey = possibleKeys[i];
                bestpos = i;
            }
        }
        //std::cout << neighborCache->getOwnEuclidianDistanceToKey(bestKey) << "\n" << std::endl;
        orderedKeys.push_back(bestKey);
        possibleKeys.erase(possibleKeys.begin()+bestpos);
    }

    /*
    std::cout << "NodeID: " << overlay->getThisNode().getKey().toString(16) << std::endl;
    std::cout << "Original Key: " << originalKey.toString(16) << std::endl;
    for (int i = 0; i < orderedKeys.size(); i++) {
        std::cout << "Sorted Key " << i << ": " << orderedKeys[i].toString(16) << " (" << overlay->getOwnEuclidianDistanceToKey(orderedKeys[i]) << ")" << std::endl;
    }
    */

    OverlayKey searchKey = orderedKeys[teamnum];

#define DIRECT_ROUTE_GET
#ifndef DIRECT_ROUTE_GET

    LookupCall* replicaMsg = new LookupCall();
    replicaMsg->setKey(searchKey);
    replicaMsg->setNumSiblings(floor(numReplica / numReplicaTeams));
    int nonce = sendInternalRpcCall(OVERLAY_COMP, replicaMsg);
    rpcIdMap.insert(make_pair(nonce, capiGetMsg));
    lastGetCall = SIMTIME_DBL(simTime());

#else

    GetMapEntry mapEntry;
    mapEntry.numSent = 0;

    // Multi team version: Already mapEntry from earlier team?
    std::map<int, GetMapEntry>::iterator it2 =
        getMap.find(capiGetMsg->getNonce());

    if (it2 != getMap.end()) {
        mapEntry = it2->second;
    } else {
        mapEntry.teamNumber = 0;
    }
    mapEntry.numAvailableReplica = 1;//lookupMsg->getSiblingsArraySize();
    mapEntry.numResponses = 0;
    mapEntry.callMsg = capiGetMsg;
    mapEntry.hashVector = NULL;
    mapEntry.replica.clear();
    for (unsigned int i = 0; i < 1/*lookupMsg->getSiblingsArraySize()*/; i++) {
        // Simplified GET Request: Just one real request, rest is for fallback
        if (i == 0) {
            CBRDHTGetCall* dhtMsg = new CBRDHTGetCall();

            dhtMsg->setOriginalKey(capiGetMsg->getKey());
            dhtMsg->setKey(searchKey);//lookupMsg->getKey());

            dhtMsg->setIsHash(false);
            dhtMsg->setKind(capiGetMsg->getKind());
            dhtMsg->setId(capiGetMsg->getId());
            dhtMsg->setBitLength(GETCALL_L(dhtMsg));
            RECORD_STATS(normalMessages++;
            numBytesNormal += dhtMsg->getByteLength());

            /*int nonce = */sendRouteRpcCall(TIER1_COMP, searchKey, dhtMsg,
                                         NULL, DEFAULT_ROUTING, -1, 0,
                                         capiGetMsg->getNonce());

            //rpcIdMap.insert(make_pair(nonce, capiGetMsg));
            //sendRouteRpcCall(TIER1_COMP, lookupMsg->getSiblings(i), dhtMsg,
            //                 NULL, DEFAULT_ROUTING, -1, 0,
            //                 capiGetMsg->getNonce());
            mapEntry.numSent++;
        } else {
            //We don't send, we just store the remaining keys as fallback
            //mapEntry.replica.push_back(lookupMsg->getSiblings(i));
        }
    }
    /*
                std::cout << "New replica: " <<  std::endl;
                for (int i = 0; i < mapEntry.replica.size(); i++) {
                    std::cout << mapEntry.replica[i] << std::endl;
                }
                std::cout << "*************************" << std::endl;
     */
    if (it2 != getMap.end())
        getMap.erase(it2);
    getMap.insert(make_pair(capiGetMsg->getNonce(), mapEntry));
#endif
}

void CBRDHT::handleDumpDhtRequest(DHTdumpCall* call)
{
    DHTdumpResponse* response = new DHTdumpResponse();
    DhtDumpVector* dumpVector = dataStorage->dumpDht();

    response->setRecordArraySize(dumpVector->size());

    for (uint i = 0; i < dumpVector->size(); i++) {
        response->setRecord(i, (*dumpVector)[i]);
    }

    delete dumpVector;

    sendRpcResponse(call, response);
}

void CBRDHT::handlePutResponse(DHTPutResponse* dhtMsg, int rpcId)
{
    std::map<int, PutMapEntry>::iterator it2 =
            putMap.find(rpcId);

    if (it2 == putMap.end()) //unknown request
        return;

    if (dhtMsg->getSuccess()) {
        it2->second.numResponses++;
    } else {
        it2->second.numFailed++;
    }

    if (it2->second.numResponses / (double)it2->second.numSent > 0.5) {
        DHTputCAPIResponse* capiPutRespMsg = new DHTputCAPIResponse();
        capiPutRespMsg->setIsSuccess(true);
        sendRpcResponse(it2->second.callMsg, capiPutRespMsg);
        it2->second.callMsg = NULL;
        putMap.erase(rpcId);
    }
}

void CBRDHT::handleGetResponse(CBRDHTGetResponse* dhtMsg, int rpcId)
{
	std::map<unsigned int, BaseCallMessage*>::iterator it =
            rpcIdMap.find(dhtMsg->getNonce());
    std::map<int, GetMapEntry>::iterator it2 =
            getMap.find(rpcId);

    //unknown request
    if (it2 == getMap.end()) {
        std::cout << "- 1 -" << std::endl;
        return;
    }

    if (!dhtMsg->getIsHash()) {
        //std::cout << "[" << overlay->getThisNode().getIp() << "] " << "Received an answer! Sending up key " << dhtMsg->getKey().toString(16) << "(orig: " << dhtMsg->getOriginalKey().toString(16) << ") -- value " << dhtMsg->getHashValue() << std::endl;
        //std::cout << "Replica left: " << it2->second.replica.size() << std::endl;

        if (dhtMsg->getHashValue().size() > 0 || dhtMsg->getResultArraySize() > 0) {
            // Successful Lookup, received a value
            DHTgetCAPIResponse* capiGetRespMsg = new DHTgetCAPIResponse();
            //capiGetRespMsg->setKey(dhtMsg->getOriginalKey());
            //capiGetRespMsg->setValue(dhtMsg->getHashValue());
            DhtDumpEntry result;
            result.setKey(dhtMsg->getKey());
            result.setValue(dhtMsg->getResult(0).getValue());//getHashValue());
            capiGetRespMsg->setResultArraySize(1);
            capiGetRespMsg->setResult(0, result);

            //std::cout << "[" << overlay->getThisNode().getIp() << "] " << "SUCCESSFUL LOOKUP! Sending up key " << dhtMsg->getKey().toString(16) << "(orig: " << dhtMsg->getOriginalKey().toString(16) << ") -- value " << dhtMsg->getHashValue() << std::endl;

            capiGetRespMsg->setIsSuccess(true);
            sendRpcResponse(it2->second.callMsg, capiGetRespMsg);
            getMap.erase(rpcId);
            return;
        } else if (it2->second.replica.size() > 0) {
            // Received empty value, try fallback replica
            NodeHandle fallbackReplica = it2->second.replica.back();

            std::cout << "[" << overlay->getThisNode().getIp() << "] " << "Empty value received. Asking replica now ("<< it2->second.replica.size()<<" left)!" << std::endl;

            CBRDHTGetCall* dhtRecall = new CBRDHTGetCall();
            dhtRecall->setOriginalKey(dhtMsg->getOriginalKey());
            dhtRecall->setKey(dhtMsg->getKey());
            dhtRecall->setIsHash(false);
            dhtRecall->setBitLength(GETCALL_L(dhtRecall));
            RECORD_STATS(normalMessages++;
            numBytesNormal += dhtRecall->getByteLength());
            sendRouteRpcCall(TIER1_COMP, fallbackReplica, dhtRecall,
                             NULL, DEFAULT_ROUTING, -1, 0,
                             it2->second.callMsg->getNonce());
            it2->second.numSent++;
            it2->second.replica.pop_back();
            return;
        } else if (it2->second.teamNumber < (numReplicaTeams - 1)) {
            // No more fallback replica in this team, try next one

            std::cout << "it2->second.teamNumber (" << it2->second.teamNumber << ") < (numReplicaTeams - 1) (" << (numReplicaTeams - 1) << ")" << std::endl;
            std::cout << "[" << overlay->getThisNode().getIp() << "] " << "No more fallback replica in this team "<< it2->second.teamNumber<<". Trying next one ("<< it2->second.teamNumber+1 <<  ")..." << std::endl;

            it2->second.teamNumber++;
            handleGetCAPIRequest(it2->second.callMsg, it2->second.teamNumber);
            return;
        } else {
            // No more replica, no more teams, send success == false to Tier 2 :(

            std::cout << "[" << overlay->getThisNode().getIp() << "] " << "No more fallback replica. Lookup failed. :(" << std::endl;

            DHTgetCAPIResponse* capiGetRespMsg = new DHTgetCAPIResponse();
            //capiGetRespMsg->setKey(dhtMsg->getOriginalKey());
            //capiGetRespMsg->setValue(BinaryValue::UNSPECIFIED_VALUE);
            DhtDumpEntry result;
            result.setKey(dhtMsg->getKey());
            result.setValue(BinaryValue::UNSPECIFIED_VALUE);
            capiGetRespMsg->setResultArraySize(1);
            capiGetRespMsg->setResult(0, result);
            capiGetRespMsg->setIsSuccess(false);
            sendRpcResponse(it2->second.callMsg, capiGetRespMsg);
            getMap.erase(rpcId);
        }
    }
}

void CBRDHT::update(const NodeHandle& node, bool joined)
{
    OverlayKey key;
    DHTPutCall* dhtMsg;
    bool err = false;
    //DHTData entry;
    DhtDataEntry entry;
    //std::map<OverlayKey, DHTData>::iterator it = dataStorage->begin();
    DhtDataMap::iterator it = dataStorage->begin();
    for (unsigned int i = 0; i < dataStorage->getSize(); i++) {
        key = it->first;
        entry = it->second;
        if (joined) {
            if (entry.responsible && (overlay->isSiblingFor(node, key,
                                                            numReplica, &err)
                    || err)) { // hack for Chord, if we've got a new predecessor

                dhtMsg = new DHTPutCall();
                dhtMsg->setKey(key);
                dhtMsg->setValue(entry.value);
                dhtMsg->setKind(entry.kind);
                dhtMsg->setId(entry.id);

                //dhtMsg->setTtl((int) (entry.ttlMessage->arrivalTime()
                //        - simTime()));
                dhtMsg->setTtl((int)SIMTIME_DBL(entry.ttlMessage->getArrivalTime()
                                                - simTime()));
                dhtMsg->setIsModifiable(entry.is_modifiable);
                dhtMsg->setMaintenance(true);
                dhtMsg->setBitLength(PUTCALL_L(dhtMsg));
                RECORD_STATS(maintenanceMessages++;
                        numBytesMaintenance += dhtMsg->getByteLength());
                sendRouteRpcCall(TIER1_COMP, node, dhtMsg);
            }

            if (err) {
                EV << "[DHT::update()]\n"
                   << "    Unable to know if key: " << key
                   << " is in range of node: " << node
                   << endl;
            }
        } else {
#if 0
            //the update concerns a node who has left
            //replicate
            LookupCall* replicaMsg = new LookupCall();
            replicaMsg->setKey(key);
            replicaMsg->setNumSiblings(numReplica);
            int nonce = sendInternalRpcCall(OVERLAY_COMP,
                                            replicaMsg);
            dhtMsg = new DHTPutCall();
            dhtMsg->setKey(key);
            dhtMsg->setValue(entry.value);
            dhtMsg->setTtl((int)(entry.ttlMessage->arrivalTime()
                    - simulation.simTime()));
            dhtMsg->setIsModifiable(entry.is_modifiable);
            dhtMsg->setMaintenance(true);
            dhtMsg->setLength(PUTCALL_L(dhtMsg));

            rpcIdMap.insert(make_pair(nonce, dhtMsg));
#endif
        }

        entry.responsible = overlay->isSiblingFor(overlay->getThisNode(),
                                                  key, 1, &err);
        it++;
    }
}

void CBRDHT::handleLookupResponse(LookupResponse* lookupMsg)
{
	std::map<unsigned int, BaseCallMessage*>::iterator it =
            rpcIdMap.find(lookupMsg->getNonce());

    if (it == rpcIdMap.end() || it->second == NULL)
        return;

    if (dynamic_cast<DHTputCAPICall*> (it->second)) {

        #if 0
        cout << "DHT::handleLookupResponse(): PUT "
             << lookupMsg->getKey() << " ("
             << overlay->getThisNode().getKey() << ")" << endl;

        for (unsigned int i = 0; i < lookupMsg->getSiblingsArraySize(); i++) {
            cout << i << ": " << lookupMsg->getSiblings(i) << endl;
        }
#endif

        DHTputCAPICall* capiPutMsg = dynamic_cast<DHTputCAPICall*> (it->second);
        rpcIdMap.erase(lookupMsg->getNonce());


        if ((lookupMsg->getIsValid() == false)
                || (lookupMsg->getSiblingsArraySize() == 0)) {

            EV << "[DHT::handleLookupResponse()]\n"
               << "    Unable to get replica list : invalid lookup"
               << endl;
            DHTputCAPIResponse* capiPutRespMsg = new DHTputCAPIResponse();
            capiPutRespMsg->setIsSuccess(false);
            sendRpcResponse(capiPutMsg, capiPutRespMsg);
            return;
        }

        for (unsigned int i = 0; i < lookupMsg->getSiblingsArraySize(); i++) {
            DHTPutCall* dhtMsg = new DHTPutCall();
            dhtMsg->setKey(capiPutMsg->getKey());
            dhtMsg->setValue(capiPutMsg->getValue());
            dhtMsg->setKind(capiPutMsg->getKind());
            dhtMsg->setId(capiPutMsg->getId());
            dhtMsg->setTtl(capiPutMsg->getTtl());
            dhtMsg->setIsModifiable(capiPutMsg->getIsModifiable());
            dhtMsg->setMaintenance(false);
            dhtMsg->setBitLength(PUTCALL_L(dhtMsg));
            RECORD_STATS(normalMessages++;
                         numBytesNormal += dhtMsg->getByteLength());
            sendRouteRpcCall(TIER1_COMP, lookupMsg->getSiblings(i),
                             dhtMsg, NULL, DEFAULT_ROUTING, -1,
                             0, capiPutMsg->getNonce());
        }

        PutMapEntry mapEntry;
        mapEntry.callMsg = capiPutMsg;
        mapEntry.numResponses = 0;
        mapEntry.numFailed = 0;
        mapEntry.numSent = lookupMsg->getSiblingsArraySize();

        putMap.insert(make_pair(capiPutMsg->getNonce(), mapEntry));
    }
    else if (dynamic_cast<DHTgetCAPICall*>(it->second)) {

#if 0
        cout << "DHT::handleLookupResponse(): GET "
             << lookupMsg->getKey() << " ("
             << overlay->getThisNode().getKey() << ")" << endl;

        for (unsigned int i = 0; i < lookupMsg->getSiblingsArraySize(); i++) {
            cout << i << ": " << lookupMsg->getSiblings(i) << endl;
        }
#endif

        DHTgetCAPICall* capiGetMsg = dynamic_cast<DHTgetCAPICall*>(it->second);
        rpcIdMap.erase(lookupMsg->getNonce());

        // Invalid lookup
        if ((lookupMsg->getIsValid() == false)
                || (lookupMsg->getSiblingsArraySize() == 0)) {

            EV << "[DHT::handleLookupResponse()]\n"
               << "    Unable to get replica list : invalid lookup"
               << endl;
            DHTgetCAPIResponse* capiGetRespMsg = new DHTgetCAPIResponse();
            //capiGetRespMsg->setKey(lookupMsg->getKey());
            //capiGetRespMsg->setValue(BinaryValue::UNSPECIFIED_VALUE);
            DhtDumpEntry result;
            result.setKey(lookupMsg->getKey());
            result.setValue(BinaryValue::UNSPECIFIED_VALUE);
            capiGetRespMsg->setResultArraySize(1);
            capiGetRespMsg->setResult(0, result);
            capiGetRespMsg->setIsSuccess(false);
            sendRpcResponse(capiGetMsg, capiGetRespMsg);
            return;
        }

        // Valid lookup
        GetMapEntry mapEntry;
        mapEntry.numSent = 0;

        // Multi team version: Already mapEntry from earlier team?

        std::map<int, GetMapEntry>::iterator it2 =
            getMap.find(capiGetMsg->getNonce());

        if (it2 != getMap.end()) {
            mapEntry = it2->second;
        } else {
            mapEntry.teamNumber = 0;
        }
        mapEntry.numAvailableReplica = lookupMsg->getSiblingsArraySize();
        mapEntry.numResponses = 0;
        mapEntry.callMsg = capiGetMsg;
        mapEntry.hashVector = NULL;
        mapEntry.replica.clear();
        for (unsigned int i = 0; i < lookupMsg->getSiblingsArraySize(); i++) {
            // Simplified GET Request: Just one real request, rest is for fallback
            if (i == 0) {
                CBRDHTGetCall* dhtMsg = new CBRDHTGetCall();

                dhtMsg->setOriginalKey(capiGetMsg->getKey());
                dhtMsg->setKey(lookupMsg->getKey());

                dhtMsg->setIsHash(false);
                dhtMsg->setKind(capiGetMsg->getKind());
                dhtMsg->setId(capiGetMsg->getId());
                dhtMsg->setBitLength(GETCALL_L(dhtMsg));
                RECORD_STATS(normalMessages++;
                numBytesNormal += dhtMsg->getByteLength());
                sendRouteRpcCall(TIER1_COMP, lookupMsg->getSiblings(i), dhtMsg,
                                 NULL, DEFAULT_ROUTING, -1, 0,
                                 capiGetMsg->getNonce());
                mapEntry.numSent++;
            } else {
                //We don't send, we just store the remaining keys as fallback
                mapEntry.replica.push_back(lookupMsg->getSiblings(i));
            }
        }
        /*
            std::cout << "New replica: " <<  std::endl;
            for (int i = 0; i < mapEntry.replica.size(); i++) {
                std::cout << mapEntry.replica[i] << std::endl;
            }
            std::cout << "*************************" << std::endl;
         */
        if (it2 != getMap.end())
            getMap.erase(it2);
        getMap.insert(make_pair(capiGetMsg->getNonce(), mapEntry));
    } else if (dynamic_cast<DHTPutCall*>(it->second)) {
    	DHTPutCall* putMsg = dynamic_cast<DHTPutCall*>(it->second);
        rpcIdMap.erase(lookupMsg->getNonce());

        if ((lookupMsg->getIsValid() == false)
            || (lookupMsg->getSiblingsArraySize() == 0)) {

            EV << "[DHT::handleLookupResponse()]\n"
               << "    Unable to get replica list : invalid lookup"
               << endl;
            delete putMsg;
            return;
        }

        for( unsigned int i = 0; i < lookupMsg->getSiblingsArraySize(); i++ ) {
            RECORD_STATS(maintenanceMessages++;
                         numBytesMaintenance += putMsg->getByteLength());

            sendRouteRpcCall(TIER1_COMP, lookupMsg->getSiblings(i),
                             new DHTPutCall(*putMsg));
        }

        delete putMsg;
    }
}

void CBRDHT::finishApp()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);

    if (time != 0) {
        // std::cout << dataStorage->getSize() << " " << overlay->getThisNode().getKey().toString(16) << std::endl;
        globalStatistics->addStdDev("DHT: Sent Maintenance Messages/s",
                                    maintenanceMessages / time);
        globalStatistics->addStdDev("DHT: Sent Normal Messages/s",
                                    normalMessages / time);
        globalStatistics->addStdDev("DHT: Sent Maintenance Bytes/s",
                                    numBytesMaintenance / time);
        globalStatistics->addStdDev("DHT: Sent Normal Bytes/s",
                                    numBytesNormal / time);
    }
}

int CBRDHT::resultValuesBitLength(DHTGetResponse* msg) {
    int bitSize = 0;
    for (uint i = 0; i < msg->getResultArraySize(); i++) {
        bitSize += msg->getResult(i).getValue().size();

    }
    return bitSize;
}

