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
 * @file CBR-DHT.h
 * @author Ingmar Baumgart, Fabian Hartmann, Bernhard Heep
 */

#ifndef __CBRDHT_H_
#define __CBRDHT_H_

#include <omnetpp.h>

#include <OverlayKey.h>
#include <SHA1.h>
#include <CommonMessages_m.h>

#include "CBR-DHTMessage_m.h"
#include "DHTMessage_m.h"
#include "DHTDataStorage.h"

#include "BaseApp.h"
#include <RpcMacros.h>

class CoordBasedRouting;
class NeighborCache;

/**
 * A Distributed Hash Table (DHT) for KBR protocols
 *
 * A Distributed Hash Table (DHT) for KBR protocols
 */
class CBRDHT : public BaseApp
{
public:
    CBRDHT();
    virtual ~CBRDHT();

protected:
    typedef std::vector<NodeHandle> ReplicaVector;

    struct GetMapEntry
    {
        ReplicaVector replica;
        std::map<BinaryValue, ReplicaVector> hashes;
        int numSent;
        int numAvailableReplica;
        int numResponses;
        int teamNumber;
        DHTgetCAPICall* callMsg;
        ReplicaVector* hashVector;
    };

    struct PutMapEntry
    {
        int numSent;
        int numFailed;
        int numResponses;
        DHTputCAPICall* callMsg;
    };

    void initializeApp(int stage);
    void finishApp();
    void handleTimerEvent(cMessage* msg);

    bool handleRpcCall(BaseCallMessage* msg);
    void handleRpcResponse(BaseResponseMessage* msg, cPolymorphic *context,
                           int rpcId, simtime_t rtt);
    void handleRpcTimeout(BaseCallMessage* msg, const TransportAddress& dest,
                          cPolymorphic* context, int rpcId,
                          const OverlayKey& destKey);
    void handleUpperMessage(cMessage* msg);
    void handlePutRequest(DHTPutCall* dhtMsg);
    void handleGetRequest(CBRDHTGetCall* dhtMsg);
    void handlePutResponse(DHTPutResponse* dhtMsg, int rpcId);
    void handleGetResponse(CBRDHTGetResponse* dhtMsg, int rpcId);
    void handlePutCAPIRequest(DHTputCAPICall* capiPutMsg);
    void handleGetCAPIRequest(DHTgetCAPICall* capiGetMsg, int teamnum = 0);

    void handleDumpDhtRequest(DHTdumpCall* call);
    void update(const NodeHandle& node, bool joined);
    void handleLookupResponse(LookupResponse* lookupMsg);

    int resultValuesBitLength(DHTGetResponse* msg);


    int numReplica;
    uint8_t numReplicaTeams;

    double maintenanceMessages;
    double normalMessages;
    double numBytesMaintenance;
    double numBytesNormal;
    double lastGetCall;
    std::map<unsigned int, BaseCallMessage*> rpcIdMap; /**< List of the Rpc Ids of the messages sent following the reception of an rpc request (the second member) */
    std::map<int, GetMapEntry> getMap;
    std::map<int, PutMapEntry> putMap;

    // module references
    DHTDataStorage* dataStorage; /**< pointer to the dht data storage */
    CoordBasedRouting* coordBasedRouting;
    NeighborCache* neighborCache;
};

#endif
