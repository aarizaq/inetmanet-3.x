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
 * @author Gregoire Menuel, Ingmar Baumgart
 */

#include <IPvXAddressResolver.h>

#include "DHT.h"

#include <RpcMacros.h>
#include <BaseRpc.h>
#include <GlobalStatistics.h>

Define_Module(DHT);

using namespace std;

DHT::DHT()
{
    dataStorage = NULL;
}

DHT::~DHT()
{
    PendingRpcs::iterator it;

    for (it = pendingRpcs.begin(); it != pendingRpcs.end(); it++) {
        delete(it->second.putCallMsg);
        delete(it->second.getCallMsg);
    }

    pendingRpcs.clear();

    if (dataStorage != NULL) {
        dataStorage->clear();
    }
}

void DHT::initializeApp(int stage)
{
    if (stage != MIN_STAGE_APP)
        return;

    dataStorage = check_and_cast<DHTDataStorage*>
                      (getParentModule()->getSubmodule("dhtDataStorage"));

    numReplica = par("numReplica");
    numGetRequests = par("numGetRequests");
    ratioIdentical = par("ratioIdentical");
    secureMaintenance = par("secureMaintenance");
    invalidDataAttack = par("invalidDataAttack");
    maintenanceAttack = par("maintenanceAttack");

    if ((int)numReplica > overlay->getMaxNumSiblings()) {
        opp_error("DHT::initialize(): numReplica bigger than what this "
                  "overlay can handle (%d)", overlay->getMaxNumSiblings());
    }

    maintenanceMessages = 0;
    normalMessages = 0;
    numBytesMaintenance = 0;
    numBytesNormal = 0;
    WATCH(maintenanceMessages);
    WATCH(normalMessages);
    WATCH(numBytesNormal);
    WATCH(numBytesMaintenance);
    WATCH_MAP(pendingRpcs);
}

void DHT::handleTimerEvent(cMessage* msg)
{
    DHTTtlTimer* msg_timer = dynamic_cast<DHTTtlTimer*> (msg);

    if (msg_timer) {
        EV << "[DHT::handleTimerEvent()]\n"
           << "    received timer ttl, key: "
           << msg_timer->getKey().toString(16)
           << "\n (overlay->getThisNode().getKey() = "
           << overlay->getThisNode().getKey().toString(16) << ")"
           << endl;

        dataStorage->removeData(msg_timer->getKey(), msg_timer->getKind(),
                                msg_timer->getId());
    }
}

bool DHT::handleRpcCall(BaseCallMessage* msg)
{
    RPC_SWITCH_START(msg)
        // RPCs between nodes
        RPC_DELEGATE(DHTPut, handlePutRequest);
        RPC_DELEGATE(DHTGet, handleGetRequest);
        // internal RPCs
        RPC_DELEGATE(DHTputCAPI, handlePutCAPIRequest);
        RPC_DELEGATE(DHTgetCAPI, handleGetCAPIRequest);
        RPC_DELEGATE(DHTdump, handleDumpDhtRequest);
    RPC_SWITCH_END( )

    return RPC_HANDLED;
}

void DHT::handleRpcResponse(BaseResponseMessage* msg, cPolymorphic* context,
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
    RPC_ON_RESPONSE(DHTGet) {
        handleGetResponse(_DHTGetResponse, rpcId);
        EV << "[DHT::handleRpcResponse()]\n"
           << "    DHT Get RPC Response received: id=" << rpcId
           << " msg=" << *_DHTGetResponse << " rtt=" << rtt
           << endl;
        break;
    }
    RPC_ON_RESPONSE(Lookup) {
        handleLookupResponse(_LookupResponse, rpcId);
        EV << "[DHT::handleRpcResponse()]\n"
           << "    Lookup RPC Response received: id=" << rpcId
           << " msg=" << *_LookupResponse << " rtt=" << rtt
           << endl;
        break;
    }
    RPC_SWITCH_END()
}

void DHT::handleRpcTimeout(BaseCallMessage* msg, const TransportAddress& dest,
                           cPolymorphic* context, int rpcId,
                           const OverlayKey& destKey)
{
    RPC_SWITCH_START(msg)
    RPC_ON_CALL(DHTPut){
        EV << "[DHT::handleRpcResponse()]\n"
           << "    DHTPut Timeout"
           << endl;

        PendingRpcs::iterator it = pendingRpcs.find(rpcId);

        if (it == pendingRpcs.end()) // unknown request
            return;

        it->second.numFailed++;

        if (it->second.numFailed / (double)it->second.numSent >= 0.5) {
            DHTputCAPIResponse* capiPutRespMsg = new DHTputCAPIResponse();
            capiPutRespMsg->setIsSuccess(false);
            sendRpcResponse(it->second.putCallMsg, capiPutRespMsg);
            //cout << "timeout 1" << endl;
            pendingRpcs.erase(rpcId);
        }

        break;
    }
    RPC_ON_CALL(DHTGet) {
        EV << "[DHT::handleRpcResponse()]\n"
           << "    DHTGet Timeout"
           << endl;

        PendingRpcs::iterator it = pendingRpcs.find(rpcId);

        if (it == pendingRpcs.end()) { // unknown request
            return;
        }

        if (it->second.state == GET_VALUE_SENT) {
            // we have sent a 'real' get request
            // ask anyone else, if possible
            if ((it->second.hashVector != NULL)
                && (it->second.hashVector->size() > 0)) {

                DHTGetCall* getCall = new DHTGetCall();
                getCall->setKey(_DHTGetCall->getKey());
                getCall->setKind(_DHTGetCall->getKind());
                getCall->setId(_DHTGetCall->getId());
                getCall->setIsHash(false);
                getCall->setBitLength(GETCALL_L(getCall));
                RECORD_STATS(normalMessages++;
                             numBytesNormal += getCall->getByteLength());

                sendRouteRpcCall(TIER1_COMP, it->second.hashVector->back(),
                                 getCall, NULL, DEFAULT_ROUTING, -1, 0, rpcId);
                it->second.hashVector->pop_back();
            } else {
                // no one else
                DHTgetCAPIResponse* capiGetRespMsg = new DHTgetCAPIResponse();
                capiGetRespMsg->setIsSuccess(false);
                sendRpcResponse(it->second.getCallMsg,
                                capiGetRespMsg);
                //cout << "DHT: GET failed: timeout (no one else)" << endl;
                pendingRpcs.erase(rpcId);
                return;
            }
        } else {
            // timeout while waiting for hashes
            // try to ask another one of the replica list for the hash
            if (it->second.replica.size() > 0) {
                DHTGetCall* getCall = new DHTGetCall();
                getCall->setKey(_DHTGetCall->getKey());
                getCall->setKind(_DHTGetCall->getKind());
                getCall->setId(_DHTGetCall->getId());
                getCall->setIsHash(true);
                getCall->setBitLength(GETCALL_L(getCall));

                RECORD_STATS(normalMessages++;
                             numBytesNormal += getCall->getByteLength());

                sendRouteRpcCall(TIER1_COMP, it->second.replica.back(),
                                 getCall, NULL, DEFAULT_ROUTING, -1, 0,
                                 rpcId);
                it->second.replica.pop_back();
            } else {
                // no one else to ask, see what we can do with what we have
                if (it->second.numResponses > 0) {
                    unsigned int maxCount = 0;
                    NodeVector* hashVector = NULL;
                    std::map<BinaryValue, NodeVector>::iterator itHashes;
                    for (itHashes = it->second.hashes.begin();
                         itHashes != it->second.hashes.end(); itHashes++) {

                        if (itHashes->second.size() > maxCount) {
                            maxCount = itHashes->second.size();
                            hashVector = &(itHashes->second);
                        }
                    }

                    // since it makes no difference for us, if we
                    // return a invalid result or return nothing,
                    // we simply return the value with the highest probability
                    it->second.hashVector = hashVector;
#if 0
                    if ((double)maxCount/(double)it->second.numResponses >=
                                                             ratioIdentical) {
                        it->second.hashVector = hashVector;
                    }
#endif
                }

                if ((it->second.hashVector != NULL)
                     && (it->second.hashVector->size() > 0)) {

                    DHTGetCall* getCall = new DHTGetCall();
                    getCall->setKey(_DHTGetCall->getKey());
                    getCall->setKind(_DHTGetCall->getKind());
                    getCall->setId(_DHTGetCall->getId());
                    getCall->setIsHash(false);
                    getCall->setBitLength(GETCALL_L(getCall));
                    RECORD_STATS(normalMessages++;
                                 numBytesNormal += getCall->getByteLength());
                    sendRouteRpcCall(TIER1_COMP, it->second.hashVector->back(),
                                     getCall, NULL, DEFAULT_ROUTING, -1,
                                     0, rpcId);
                    it->second.hashVector->pop_back();
                } else {
                    // no more nodes to ask -> get failed
                    DHTgetCAPIResponse* capiGetRespMsg = new DHTgetCAPIResponse();
                    capiGetRespMsg->setIsSuccess(false);
                    sendRpcResponse(it->second.getCallMsg, capiGetRespMsg);
                    //cout << "DHT: GET failed: timeout2 (no one else)" << endl;
                    pendingRpcs.erase(rpcId);
                }
            }
        }
        break;
    }
    RPC_SWITCH_END( )
}

void DHT::handlePutRequest(DHTPutCall* dhtMsg)
{
    std::string tempString = "PUT_REQUEST received: "
            + std::string(dhtMsg->getKey().toString(16));
    getParentModule()->getParentModule()->bubble(tempString.c_str());

    bool err;
    bool isSibling = overlay->isSiblingFor(overlay->getThisNode(),
                  dhtMsg->getKey(), secureMaintenance ? numReplica : 1, &err);
    if (err) {
        isSibling = true;
    }

    if (secureMaintenance && dhtMsg->getMaintenance()) {
        DhtDataEntry* entry = dataStorage->getDataEntry(dhtMsg->getKey(),
                                                        dhtMsg->getKind(),
                                                        dhtMsg->getId());
        if (entry == NULL) {
            // add ttl timer
            DHTTtlTimer *timerMsg = new DHTTtlTimer("ttl_timer");
            timerMsg->setKey(dhtMsg->getKey());
            timerMsg->setKind(dhtMsg->getKind());
            timerMsg->setId(dhtMsg->getId());
            scheduleAt(simTime() + dhtMsg->getTtl(), timerMsg);

            entry = dataStorage->addData(dhtMsg->getKey(), dhtMsg->getKind(),
                                 dhtMsg->getId(), dhtMsg->getValue(), timerMsg,
                                 dhtMsg->getIsModifiable(), dhtMsg->getSrcNode(),
                                 isSibling);
        } else if ((entry->siblingVote.size() == 0) && isSibling) {
            // we already have a verified entry with this key and are
            // still responsible => ignore maintenance calls
            delete dhtMsg;
            return;
        }

        SiblingVoteMap::iterator it = entry->siblingVote.find(dhtMsg->getValue());
        if (it == entry->siblingVote.end()) {
            // new hash
            NodeVector vect;
            vect.add(dhtMsg->getSrcNode());
            entry->siblingVote.insert(make_pair(dhtMsg->getValue(),
                                                vect));
        } else {
            it->second.add(dhtMsg->getSrcNode());
        }

        size_t maxCount = 0;
        SiblingVoteMap::iterator majorityIt;

        for (it = entry->siblingVote.begin(); it != entry->siblingVote.end(); it++) {
            if (it->second.size() > maxCount) {
                maxCount = it->second.size();
                majorityIt = it;
            }
        }

        entry->value = majorityIt->first;
        entry->responsible = true;

        if (maxCount > numReplica) {
            entry->siblingVote.clear();
        }

        // send back
        DHTPutResponse* responseMsg = new DHTPutResponse();
        responseMsg->setSuccess(true);
        responseMsg->setBitLength(PUTRESPONSE_L(responseMsg));
        RECORD_STATS(normalMessages++; numBytesNormal += responseMsg->getByteLength());

        sendRpcResponse(dhtMsg, responseMsg);

        return;
    }

#if 0
    if (!(dataStorage->isModifiable(dhtMsg->getKey(), dhtMsg->getKind(),
                                    dhtMsg->getId()))) {
        // check if the put request came from the right node
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
#endif

    // remove data item from local data storage
    dataStorage->removeData(dhtMsg->getKey(), dhtMsg->getKind(),
                            dhtMsg->getId());

    if (dhtMsg->getValue().size() > 0) {
        // add ttl timer
        DHTTtlTimer *timerMsg = new DHTTtlTimer("ttl_timer");
        timerMsg->setKey(dhtMsg->getKey());
        timerMsg->setKind(dhtMsg->getKind());
        timerMsg->setId(dhtMsg->getId());
        scheduleAt(simTime() + dhtMsg->getTtl(), timerMsg);
        // storage data item in local data storage
        dataStorage->addData(dhtMsg->getKey(), dhtMsg->getKind(),
        		             dhtMsg->getId(), dhtMsg->getValue(), timerMsg,
                             dhtMsg->getIsModifiable(), dhtMsg->getSrcNode(),
                             isSibling);
    }

    // send back
    DHTPutResponse* responseMsg = new DHTPutResponse();
    responseMsg->setSuccess(true);
    responseMsg->setBitLength(PUTRESPONSE_L(responseMsg));
    RECORD_STATS(normalMessages++; numBytesNormal += responseMsg->getByteLength());

    sendRpcResponse(dhtMsg, responseMsg);
}

void DHT::handleGetRequest(DHTGetCall* dhtMsg)
{
    std::string tempString = "GET_REQUEST received: "
            + std::string(dhtMsg->getKey().toString(16));

    getParentModule()->getParentModule()->bubble(tempString.c_str());

    if (dhtMsg->getKey().isUnspecified()) {
        throw cRuntimeError("DHT::handleGetRequest: Unspecified key!");
    }

    DhtDumpVector* dataVect = dataStorage->dumpDht(dhtMsg->getKey(),
                                                   dhtMsg->getKind(),
                                                   dhtMsg->getId());

    if (overlay->isMalicious() && invalidDataAttack) {
        dataVect->resize(1);
        dataVect->at(0).setKey(dhtMsg->getKey());
        dataVect->at(0).setKind(dhtMsg->getKind());
        dataVect->at(0).setId(dhtMsg->getId());
        dataVect->at(0).setValue("Modified Data");
        dataVect->at(0).setTtl(3600*24*365);
        dataVect->at(0).setOwnerNode(overlay->getThisNode());
        dataVect->at(0).setIs_modifiable(false);
        dataVect->at(0).setResponsible(true);
    }

    // send back
    DHTGetResponse* responseMsg = new DHTGetResponse();
    responseMsg->setKey(dhtMsg->getKey());
    responseMsg->setIsHash(dhtMsg->getIsHash());

    if (dataVect->size() == 0) {
        responseMsg->setHashValue(BinaryValue::UNSPECIFIED_VALUE);
        responseMsg->setResultArraySize(0);
    } else {
        if (dhtMsg->getIsHash()) {
            // TODO: verify this
            BinaryValue resultValues;
            for (uint32_t i = 0; i < dataVect->size(); i++) {
                resultValues += (*dataVect)[i].getValue();
            }

            CSHA1 sha1;
            BinaryValue hashValue(20);
            sha1.Reset();
            sha1.Update((uint8_t*) (&(*resultValues.begin())),
                        resultValues.size());
            sha1.Final();
            sha1.GetHash((unsigned char*)&hashValue[0]);

            responseMsg->setHashValue(hashValue);
        } else {
            responseMsg->setResultArraySize(dataVect->size());

            for (uint32_t i = 0; i < dataVect->size(); i++) {
                responseMsg->setResult(i, (*dataVect)[i]);
            }

        }
    }
    delete dataVect;

    responseMsg->setBitLength(GETRESPONSE_L(responseMsg));
    RECORD_STATS(normalMessages++;
                 numBytesNormal += responseMsg->getByteLength());
    sendRpcResponse(dhtMsg, responseMsg);
}

void DHT::handlePutCAPIRequest(DHTputCAPICall* capiPutMsg)
{
    // asks the replica list
    LookupCall* lookupCall = new LookupCall();
    lookupCall->setKey(capiPutMsg->getKey());
    lookupCall->setNumSiblings(numReplica);
    sendInternalRpcCall(OVERLAY_COMP, lookupCall, NULL, -1, 0,
                        capiPutMsg->getNonce());

    PendingRpcsEntry entry;
    entry.putCallMsg = capiPutMsg;
    entry.state = LOOKUP_STARTED;
    pendingRpcs.insert(make_pair(capiPutMsg->getNonce(), entry));
}

void DHT::handleGetCAPIRequest(DHTgetCAPICall* capiGetMsg)
{
    LookupCall* lookupCall = new LookupCall();
    lookupCall->setKey(capiGetMsg->getKey());
    lookupCall->setNumSiblings(numReplica);
    sendInternalRpcCall(OVERLAY_COMP, lookupCall, NULL, -1, 0,
                        capiGetMsg->getNonce());

    PendingRpcsEntry entry;
    entry.getCallMsg = capiGetMsg;
    entry.state = LOOKUP_STARTED;
    pendingRpcs.insert(make_pair(capiGetMsg->getNonce(), entry));
}

void DHT::handleDumpDhtRequest(DHTdumpCall* call)
{
    DHTdumpResponse* response = new DHTdumpResponse();
    DhtDumpVector* dumpVector = dataStorage->dumpDht();

    response->setRecordArraySize(dumpVector->size());

    for (uint32_t i = 0; i < dumpVector->size(); i++) {
        response->setRecord(i, (*dumpVector)[i]);
    }

    delete dumpVector;

    sendRpcResponse(call, response);
}

void DHT::handlePutResponse(DHTPutResponse* dhtMsg, int rpcId)
{
    PendingRpcs::iterator it = pendingRpcs.find(rpcId);

    if (it == pendingRpcs.end()) // unknown request
        return;

    if (dhtMsg->getSuccess()) {
        it->second.numResponses++;
    } else {
        it->second.numFailed++;
    }


//    if ((it->second.numFailed + it->second.numResponses) == it->second.numSent) {
    if (it->second.numResponses / (double)it->second.numSent > 0.5) {

        DHTputCAPIResponse* capiPutRespMsg = new DHTputCAPIResponse();
        capiPutRespMsg->setIsSuccess(true);
        sendRpcResponse(it->second.putCallMsg, capiPutRespMsg);
        pendingRpcs.erase(rpcId);
    }
}

void DHT::handleGetResponse(DHTGetResponse* dhtMsg, int rpcId)
{
    NodeVector* hashVector = NULL;
    PendingRpcs::iterator it = pendingRpcs.find(rpcId);

    if (it == pendingRpcs.end()) // unknown request
        return;

    if (it->second.state == GET_VALUE_SENT) {
        // we have sent a 'real' get request
        if (!dhtMsg->getIsHash()) {
            // TODO verify hash
            DHTgetCAPIResponse* capiGetRespMsg = new DHTgetCAPIResponse();
            capiGetRespMsg->setResultArraySize(dhtMsg->getResultArraySize());
            for (uint i = 0; i < dhtMsg->getResultArraySize(); i++) {
                capiGetRespMsg->setResult(i, dhtMsg->getResult(i));
            }
            capiGetRespMsg->setIsSuccess(true);
            sendRpcResponse(it->second.getCallMsg, capiGetRespMsg);
            pendingRpcs.erase(rpcId);
            return;
        }
    }

    if (dhtMsg->getIsHash()) {
        std::map<BinaryValue, NodeVector>::iterator itHashes =
            it->second.hashes.find(dhtMsg->getHashValue());

        if (itHashes == it->second.hashes.end()) {
            // new hash
            NodeVector vect;
            vect.push_back(dhtMsg->getSrcNode());
            it->second.hashes.insert(make_pair(dhtMsg->getHashValue(),
                                               vect));
        } else {
            itHashes->second.push_back(dhtMsg->getSrcNode());
        }

        it->second.numResponses++;

        if (it->second.state == GET_VALUE_SENT) {
            // we have already sent a real get request
            return;
        }

        // count the maximum number of equal hash values received so far
        unsigned int maxCount = 0;


        for (itHashes = it->second.hashes.begin();
        itHashes != it->second.hashes.end(); itHashes++) {

            if (itHashes->second.size() > maxCount) {
                maxCount = itHashes->second.size();
                hashVector = &(itHashes->second);
            }
        }

        if ((double) maxCount / (double) it->second.numAvailableReplica
                >= ratioIdentical) {
            it->second.hashVector = hashVector;
        } else if (it->second.numResponses >= numGetRequests) {
            // we'll try to ask some other nodes
            if (it->second.replica.size() > 0) {
                DHTGetCall* getCall = new DHTGetCall();
                getCall->setKey(dhtMsg->getKey());
                getCall->setKind(dhtMsg->getKind());
                getCall->setId(dhtMsg->getId());
                getCall->setIsHash(true);
                getCall->setBitLength(GETCALL_L(getCall));
                RECORD_STATS(normalMessages++;
                numBytesNormal += getCall->getByteLength());
                sendRouteRpcCall(TIER1_COMP,
                                 it->second.replica.back(), getCall,
                                 NULL, DEFAULT_ROUTING, -1, 0, rpcId);
                it->second.replica.pop_back();
                it->second.state = GET_HASH_SENT;
            } else if (hashVector == NULL) {
                // we don't have anyone else to ask and no hash
                DHTgetCAPIResponse* capiGetRespMsg =
                    new DHTgetCAPIResponse();
                DhtDumpEntry result;
                result.setKey(dhtMsg->getKey());
                result.setValue(BinaryValue::UNSPECIFIED_VALUE);
                capiGetRespMsg->setResultArraySize(1);
                capiGetRespMsg->setResult(0, result);
                capiGetRespMsg->setIsSuccess(false);
                sendRpcResponse(it->second.getCallMsg, capiGetRespMsg);
#if 0
                cout << "DHT: GET failed: hash (no one else)" << endl;
                cout << "numResponses: " << it->second.numResponses
                     << " numAvailableReplica: " << it->second.numAvailableReplica << endl;

                for (itHashes = it->second.hashes.begin();
                     itHashes != it->second.hashes.end(); itHashes++) {
                    cout << "   - " << itHashes->first << " ("
                         << itHashes->second.size() << ")" << endl;
                }
#endif

                pendingRpcs.erase(rpcId);
                return;
            } else {
                // we don't have anyone else to ask => take what we've got
                it->second.hashVector = hashVector;
            }
        }
    }

    if ((it->second.state != GET_VALUE_SENT) &&
            (it->second.hashVector != NULL)) {
        // we have already received all the response and chosen a hash
        if (it->second.hashVector->size() > 0) {
            DHTGetCall* getCall = new DHTGetCall();
            getCall->setKey(it->second.getCallMsg->getKey());
            getCall->setKind(it->second.getCallMsg->getKind());
            getCall->setId(it->second.getCallMsg->getId());
            getCall->setIsHash(false);
            getCall->setBitLength(GETCALL_L(getCall));
            RECORD_STATS(normalMessages++;
                         numBytesNormal += getCall->getByteLength());
            sendRouteRpcCall(TIER1_COMP, it->second.hashVector->back(),
                             getCall, NULL, DEFAULT_ROUTING, -1, 0, rpcId);
            it->second.hashVector->pop_back();
            it->second.state = GET_VALUE_SENT;
        } else { // we don't have anyone else to ask
            DHTgetCAPIResponse* capiGetRespMsg = new DHTgetCAPIResponse();
            DhtDumpEntry result;
            result.setKey(dhtMsg->getKey());
            result.setValue(BinaryValue::UNSPECIFIED_VALUE);
            capiGetRespMsg->setResultArraySize(1);
            capiGetRespMsg->setResult(0, result);
            capiGetRespMsg->setIsSuccess(false);
            sendRpcResponse(it->second.getCallMsg, capiGetRespMsg);
            //cout << "DHT: GET failed: hash2 (no one else)" << endl;
            pendingRpcs.erase(rpcId);
        }
    }
}

void DHT::update(const NodeHandle& node, bool joined)
{
    OverlayKey key;
    bool err = false;
    DhtDataEntry entry;
    std::map<OverlayKey, DhtDataEntry>::iterator it;

    EV << "[DHT::update() @ " << overlay->getThisNode().getIp()
       << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
       << "    Update called()"
       << endl;

    if (secureMaintenance) {
        for (it = dataStorage->begin(); it != dataStorage->end(); it++) {
            if (it->second.responsible) {
                NodeVector* siblings = overlay->local_lookup(it->first,
                                                             numReplica,
                                                             false);
                if (siblings->size() == 0) {
                    delete siblings;
                    continue;
                }

                if (joined) {
                    EV << "[DHT::update() @ " << overlay->getThisNode().getIp()
                       << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
                       << "    Potential new sibling for record " << it->first
                       << endl;

                    if (overlay->distance(node.getKey(), it->first) <=
                        overlay->distance(siblings->back().getKey(), it->first)) {

                        sendMaintenancePutCall(node, it->first, it->second);
                    }

                    if (overlay->distance(overlay->getThisNode().getKey(), it->first) >
                        overlay->distance(siblings->back().getKey(), it->first)) {

                        it->second.responsible = false;
                    }
                } else {
                    if (overlay->distance(node.getKey(), it->first) <
                        overlay->distance(siblings->back().getKey(), it->first)) {

                        sendMaintenancePutCall(siblings->back(), it->first,
                                               it->second);
                    }
                }

                delete siblings;
            }
        }

        return;
    }

    for (it = dataStorage->begin(); it != dataStorage->end(); it++) {
        key = it->first;
        entry = it->second;
        if (joined) {
            if (entry.responsible && (overlay->isSiblingFor(node, key,
                                                            numReplica, &err)
                    || err)) { // hack for Chord, if we've got a new predecessor

                if (err) {
                    EV << "[DHT::update()]\n"
                       << "    Unable to know if key: " << key
                       << " is in range of node: " << node
                       << endl;
                    // For Chord: we've got a new predecessor
                    // TODO: only send record, if we are not responsible any more
                    // TODO: check all protocols to change routing table first,
                    //       and than call update.

                    //if (overlay->isSiblingFor(overlay->getThisNode(), key, 1, &err)) {
                    //    continue;
                    //}
                }

                sendMaintenancePutCall(node, key, entry);
            }
        }
        //TODO: move this to the inner block above?
        entry.responsible = overlay->isSiblingFor(overlay->getThisNode(),
                                                  key, 1, &err);
    }
}

void DHT::sendMaintenancePutCall(const TransportAddress& node,
                                 const OverlayKey& key,
                                 const DhtDataEntry& entry) {

    DHTPutCall* dhtMsg = new DHTPutCall();

    dhtMsg->setKey(key);
    dhtMsg->setKind(entry.kind);
    dhtMsg->setId(entry.id);

    if (overlay->isMalicious() && maintenanceAttack) {
        dhtMsg->setValue("Modified Data");
    } else {
        dhtMsg->setValue(entry.value);
    }

    dhtMsg->setTtl((int)SIMTIME_DBL(entry.ttlMessage->getArrivalTime()
                                    - simTime()));
    dhtMsg->setIsModifiable(entry.is_modifiable);
    dhtMsg->setMaintenance(true);
    dhtMsg->setBitLength(PUTCALL_L(dhtMsg));
    RECORD_STATS(maintenanceMessages++;
                 numBytesMaintenance += dhtMsg->getByteLength());

    sendRouteRpcCall(TIER1_COMP, node, dhtMsg);
}

void DHT::handleLookupResponse(LookupResponse* lookupMsg, int rpcId)
{
    PendingRpcs::iterator it = pendingRpcs.find(rpcId);

    if (it == pendingRpcs.end()) {
        return;
    }

    if (it->second.putCallMsg != NULL) {

#if 0
        cout << "DHT::handleLookupResponse(): PUT "
             << lookupMsg->getKey() << " ("
             << overlay->getThisNode().getKey() << ")" << endl;

        for (unsigned int i = 0; i < lookupMsg->getSiblingsArraySize(); i++) {
            cout << i << ": " << lookupMsg->getSiblings(i) << endl;
        }
#endif

        if ((lookupMsg->getIsValid() == false)
                || (lookupMsg->getSiblingsArraySize() == 0)) {

            EV << "[DHT::handleLookupResponse()]\n"
               << "    Unable to get replica list : invalid lookup"
               << endl;
            DHTputCAPIResponse* capiPutRespMsg = new DHTputCAPIResponse();
            capiPutRespMsg->setIsSuccess(false);
            //cout << "DHT::lookup failed" << endl;
            sendRpcResponse(it->second.putCallMsg, capiPutRespMsg);
            pendingRpcs.erase(rpcId);
            return;
        }

        if ((it->second.putCallMsg->getId() == 0) &&
                (it->second.putCallMsg->getValue().size() > 0)) {
            // pick a random id before replication of the data item
            // id 0 is kept for delete requests (i.e. a put with empty value)
            it->second.putCallMsg->setId(intuniform(1, 2147483647));
        }

        for (unsigned int i = 0; i < lookupMsg->getSiblingsArraySize(); i++) {
            DHTPutCall* dhtMsg = new DHTPutCall();
            dhtMsg->setKey(it->second.putCallMsg->getKey());
            dhtMsg->setKind(it->second.putCallMsg->getKind());
            dhtMsg->setId(it->second.putCallMsg->getId());
            dhtMsg->setValue(it->second.putCallMsg->getValue());
            dhtMsg->setTtl(it->second.putCallMsg->getTtl());
            dhtMsg->setIsModifiable(it->second.putCallMsg->getIsModifiable());
            dhtMsg->setMaintenance(false);
            dhtMsg->setBitLength(PUTCALL_L(dhtMsg));
            RECORD_STATS(normalMessages++;
                         numBytesNormal += dhtMsg->getByteLength());
            sendRouteRpcCall(TIER1_COMP, lookupMsg->getSiblings(i),
                             dhtMsg, NULL, DEFAULT_ROUTING, -1,
                             0, rpcId);
        }

        it->second.state = PUT_SENT;
        it->second.numResponses = 0;
        it->second.numFailed = 0;
        it->second.numSent = lookupMsg->getSiblingsArraySize();
    }
    else if (it->second.getCallMsg != NULL) {

#if 0
        cout << "DHT::handleLookupResponse(): GET "
             << lookupMsg->getKey() << " ("
             << overlay->getThisNode().getKey() << ")" << endl;

        for (unsigned int i = 0; i < lookupMsg->getSiblingsArraySize(); i++) {
            cout << i << ": " << lookupMsg->getSiblings(i) << endl;
        }
#endif

        if ((lookupMsg->getIsValid() == false)
                || (lookupMsg->getSiblingsArraySize() == 0)) {

            EV << "[DHT::handleLookupResponse()]\n"
               << "    Unable to get replica list : invalid lookup"
               << endl;
            DHTgetCAPIResponse* capiGetRespMsg = new DHTgetCAPIResponse();
            DhtDumpEntry result;
            result.setKey(lookupMsg->getKey());
            result.setValue(BinaryValue::UNSPECIFIED_VALUE);
            capiGetRespMsg->setResultArraySize(1);
            capiGetRespMsg->setResult(0, result);
            capiGetRespMsg->setIsSuccess(false);
            //cout << "DHT: lookup failed 2" << endl;
            sendRpcResponse(it->second.getCallMsg, capiGetRespMsg);
            pendingRpcs.erase(rpcId);
            return;
        }

        it->second.numSent = 0;

        for (unsigned int i = 0; i < lookupMsg->getSiblingsArraySize(); i++) {
            if (i < (unsigned int)numGetRequests) {
                DHTGetCall* dhtMsg = new DHTGetCall();
                dhtMsg->setKey(it->second.getCallMsg->getKey());
                dhtMsg->setKind(it->second.getCallMsg->getKind());
                dhtMsg->setId(it->second.getCallMsg->getId());
                dhtMsg->setIsHash(true);
                dhtMsg->setBitLength(GETCALL_L(dhtMsg));
                RECORD_STATS(normalMessages++;
                             numBytesNormal += dhtMsg->getByteLength());
                sendRouteRpcCall(TIER1_COMP, lookupMsg->getSiblings(i), dhtMsg,
                                 NULL, DEFAULT_ROUTING, -1, 0, rpcId);
                it->second.numSent++;
            } else {
                // we don't send, we just store the remaining keys
                it->second.replica.push_back(lookupMsg->getSiblings(i));
            }
        }

        it->second.numAvailableReplica = lookupMsg->getSiblingsArraySize();
        it->second.numResponses = 0;
        it->second.hashVector = NULL;
        it->second.state = GET_HASH_SENT;
    }
}

void DHT::finishApp()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);

    if (time >= GlobalStatistics::MIN_MEASURED) {
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

int DHT::resultValuesBitLength(DHTGetResponse* msg) {
    int bitSize = 0;
    for (uint i = 0; i < msg->getResultArraySize(); i++) {
        bitSize += msg->getResult(i).getValue().size();

    }
    return bitSize;
}

std::ostream& operator<<(std::ostream& os, const DHT::PendingRpcsEntry& entry)
{
    if (entry.getCallMsg) {
        os << "GET";
    } else if (entry.putCallMsg) {
        os << "PUT";
    }

    os << " state: " << entry.state
       << " numSent: " << entry.numSent
       << " numResponses: " << entry.numResponses
       << " numFailed: " << entry.numFailed
       << " numAvailableReplica: " << entry.numAvailableReplica;

    if (entry.replica.size() > 0) {
        os << " replicaSize: " << entry.replica.size();
    }

    if (entry.hashVector != NULL) {
        os << " hashVectorSize: " << entry.hashVector->size();
    }

    if (entry.hashes.size() > 0) {
        os << " hashes:";
        std::map<BinaryValue, NodeVector>::const_iterator it;

        int i = 0;
        for (it = entry.hashes.begin(); it != entry.hashes.end(); it++, i++) {
            os << " hash" << i << ":" << it->second.size();
        }
    }

    return os;
}
