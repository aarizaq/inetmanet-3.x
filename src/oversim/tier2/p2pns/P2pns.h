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
 * @file P2pns.h
 * @author Ingmar Baumgart
 */

#ifndef __P2PNS_H_
#define __P2PNS_H_

#include <omnetpp.h>

#include <OverlayKey.h>
#include <SHA1.h>
#include <CommonMessages_m.h>

#include <BaseApp.h>
#include <RpcMacros.h>

class XmlRpcInterface;

#include "P2pnsCache.h"

/**
 * Implementation of "P2PNS: A distributed name service for P2PSIP"
 *
 * Implementation of "P2PNS: A distributed name service for P2PSIP"
 */
class P2pns : public BaseApp
{
public:
    P2pns();
    virtual ~P2pns();

    void tunnel(const OverlayKey& destKey, const BinaryValue& payload);
    void registerId(const std::string& addr);

    void handleReadyMessage(CompReadyMessage* msg);

private:
    enum LookupRpcId {
        RESOLVE_LOOKUP = 0,
        TUNNEL_LOOKUP = 1,
        REFRESH_LOOKUP = 2
    };

    class OverlayKeyObject : public OverlayKey, public cObject {
    public:
        OverlayKeyObject(const OverlayKey& key) : OverlayKey(key) {};
    };

    void initializeApp(int stage);
    void finishApp();
    void handleTimerEvent(cMessage* msg);
    void deliver(OverlayKey& key, cMessage* msg);

    void sendTunnelMessage(const TransportAddress& addr,
                           const BinaryValue& payload);

    void updateIdCacheWithNewTransport(cMessage* msg);

    void handleTunnelLookupResponse(LookupResponse* lookupResponse);

    bool handleRpcCall(BaseCallMessage* msg);
    void handleRpcResponse(BaseResponseMessage* msg,
                           cPolymorphic* context, int rpcId, simtime_t rtt);

    void pingRpcResponse(PingResponse* response, cPolymorphic* context,
                         int rpcId, simtime_t rtt);
    void pingTimeout(PingCall* call, const TransportAddress& dest,
                     cPolymorphic* context, int rpcId);

    void p2pnsRegisterRpc(P2pnsRegisterCall* registerCall);
    void p2pnsResolveRpc(P2pnsResolveCall* registerCall);

    void handleDHTputCAPIResponse(DHTputCAPIResponse* putResponse,
                                  P2pnsRegisterCall* registerCall);
    void handleDHTgetCAPIResponse(DHTgetCAPIResponse* gettResponse,
                                  P2pnsResolveCall* resolveCall);
    void handleLookupResponse(LookupResponse* lookupResponse,
                              cObject* context, int rpcId);

    P2pnsCache *p2pnsCache; /**< pointer to the name cache module */
    XmlRpcInterface *xmlRpcInterface; /**< pointer to the XmlRpcInterface module */
    bool twoStageResolution; /**< Use the two stage name resolution (KBR/DHt) */
    simtime_t keepaliveInterval; /**< interval between two keeaplive pings for active connections */
    simtime_t idCacheLifetime; /**< idle connections in the idCache get deleted after this time */
    OverlayKey thisId; /**< the 100 most significant bit of this node's nodeId */
};

#endif
