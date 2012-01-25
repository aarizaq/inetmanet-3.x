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
 * @file XmlRpcInterface.cc
 * @author Ingmar Baumgart
 */

#include <platdep/sockets.h>
#if not defined _WIN32
#include <arpa/inet.h>
#endif

#if not defined _WIN32 && not defined __APPLE__
#include <netinet/ip6.h>
#endif

#include <NodeVector.h>
#include <P2pns.h>
#include <sstream>
#include "XmlRpcInterface.h"

using namespace XmlRpc;

Define_Module(XmlRpcInterface);

// Register a name with P2PNS
class P2pnsRegister : public XmlRpcServerMethod
{
public:
    P2pnsRegister(XmlRpcServer* s) :
        XmlRpcServerMethod("register", s)
    {
    }

    void execute(XmlRpcValue& params, XmlRpcValue& result)
    {
        (dynamic_cast<XmlRpcInterface*>(_server))->p2pnsRegister(params, result);
    }

    std::string help()
    {
        return std::string("Register a name with P2PNS");
    }
};

// Register a name with P2PNS
class P2pnsResolve : public XmlRpcServerMethod
{
public:
    P2pnsResolve(XmlRpcServer* s) :
        XmlRpcServerMethod("resolve", s)
    {
    }

    void execute(XmlRpcValue& params, XmlRpcValue& result)
    {
        (dynamic_cast<XmlRpcInterface*>(_server))->p2pnsResolve(params, result);
    }

    std::string help()
    {
        return std::string("Resolve a name with P2PNS");
    }
};

// Common API: local_lookup()
class LocalLookup : public XmlRpcServerMethod
{
public:
    LocalLookup(XmlRpcServer* s) :
        XmlRpcServerMethod("local_lookup", s)
    {
    }

    void execute(XmlRpcValue& params, XmlRpcValue& result)
    {
        (dynamic_cast<XmlRpcInterface*>(_server))->localLookup(params, result);
    }

    std::string help()
    {
        return std::string("Lookup key in local "
            "routing table");
    }
};

// Common API: lookup()
class Lookup : public XmlRpcServerMethod
{
public:
    Lookup(XmlRpcServer* s) :
        XmlRpcServerMethod("lookup", s)
    {
    }

    void execute(XmlRpcValue& params, XmlRpcValue& result)
    {
        (dynamic_cast<XmlRpcInterface*>(_server))->lookup(params, result);
    }

    std::string help()
    {
        return std::string("Lookup key with KBR layer");
    }
};

// Store a value in the DHT
class Put : public XmlRpcServerMethod
{
public:
    Put(XmlRpcServer* s) :
        XmlRpcServerMethod("put", s)
    {
    }

    void execute(XmlRpcValue& params, XmlRpcValue& result)
    {
        (dynamic_cast<XmlRpcInterface*>(_server))->put(params, result);
    }

    std::string help()
    {
        return std::string("Store a value in the DHT");
    }
};

// Get a value from the DHT
class Get : public XmlRpcServerMethod
{
public:
    Get(XmlRpcServer* s) :
        XmlRpcServerMethod("get", s)
    {
    }

    void execute(XmlRpcValue& params, XmlRpcValue& result)
    {
        (dynamic_cast<XmlRpcInterface*>(_server))->get(params, result);
    }

    std::string help()
    {
        return std::string("Get a value from the DHT");
    }
};

// Dump all local records from the DHT
class DumpDht : public XmlRpcServerMethod
{
public:
    DumpDht(XmlRpcServer* s) :
        XmlRpcServerMethod("dump_dht", s)
    {
    }

    void execute(XmlRpcValue& params, XmlRpcValue& result)
    {
        (dynamic_cast<XmlRpcInterface*>(_server))->dumpDht(params, result);
    }

    std::string help()
    {
        return std::string("Dump all local records from the DHT");
    }
};

// Join the overlay with a specific nodeID
class JoinOverlay : public XmlRpcServerMethod
{
public:
    JoinOverlay(XmlRpcServer* s) :
        XmlRpcServerMethod("join", s)
    {
    }

    void execute(XmlRpcValue& params, XmlRpcValue& result)
    {
        (dynamic_cast<XmlRpcInterface*>(_server))->joinOverlay(params, result);
    }

    std::string help()
    {
        return std::string("Join the overlay with a specific nodeID");
    }
};

void XmlRpcInterface::p2pnsRegister(XmlRpcValue& params, XmlRpcValue& result)
{
    if ((params.size() != 5) ||
            (params[0].getType() != XmlRpcValue::TypeBase64) ||
            (params[1].getType() != XmlRpcValue::TypeInt) ||
            (params[2].getType() != XmlRpcValue::TypeInt) ||
            (params[3].getType() != XmlRpcValue::TypeBase64) ||
            (params[4].getType() != XmlRpcValue::TypeInt))
        throw XmlRpcException("register(base64 name, int kind, int id, base64 address, int ttl): "
                              "Invalid argument type");

    if (overlay->getCompModule(TIER2_COMP) == NULL)
        throw XmlRpcException("register(base64 name, int kind, int id, base64 address, int ttl): "
                              "No P2PNS service");

    if (!isPrivileged()) {
        throw XmlRpcException("register(base64 name, int kind, base64 address, "
                              "int ttl): Not allowed");
    }

    state[curAppFd]._connectionState = EXECUTE_REQUEST;

    P2pnsRegisterCall* registerCall = new P2pnsRegisterCall();
    registerCall->setP2pName(((const XmlRpcValue::BinaryData&)params[0]));
    registerCall->setKind((int)params[1]);
    registerCall->setId((int)params[2]);
    registerCall->setAddress(((const XmlRpcValue::BinaryData&)params[3]));
    registerCall->setTtl(params[4]);

    sendInternalRpcWithTimeout(TIER2_COMP, registerCall);
}

void XmlRpcInterface::p2pnsResolve(XmlRpcValue& params, XmlRpcValue& result)
{
    if ((params.size() != 2) ||
            (params[0].getType() != XmlRpcValue::TypeBase64) ||
            (params[1].getType() != XmlRpcValue::TypeInt))
        throw XmlRpcException("resolve(base64 name, int kind): Invalid argument type");

    if (overlay->getCompModule(TIER2_COMP) == NULL)
        throw XmlRpcException("resolve(base64 name, int kind): No P2PNS service");

    state[curAppFd]._connectionState = EXECUTE_REQUEST;

    P2pnsResolveCall* resolveCall = new P2pnsResolveCall();

    resolveCall->setP2pName(((const XmlRpcValue::BinaryData&)params[0]));
    resolveCall->setKind((int)params[1]);
    resolveCall->setId(0);

    sendInternalRpcWithTimeout(TIER2_COMP, resolveCall);
}

void XmlRpcInterface::localLookup(XmlRpcValue& params, XmlRpcValue& result)
{
    if ((params.size() != 3)
            || (params[0].getType() != XmlRpcValue::TypeBase64)
            || (params[1].getType() != XmlRpcValue::TypeInt)
            || (params[2].getType() != XmlRpcValue::TypeBoolean))
        throw XmlRpcException("local_lookup(base64 key, int num, "
                "boolean safe): Invalid argument type");

    BinaryValue keyString = (const XmlRpcValue::BinaryData&)params[0];

    NodeVector* nextHops = NULL;

    if (keyString.size() > 0) {
        nextHops = overlay->local_lookup(OverlayKey::sha1(keyString),
                                                     params[1], params[2]);
    } else {
        nextHops = overlay->local_lookup(overlay->getThisNode().getKey(),
                                                     params[1], params[2]);
    }

    for (uint32_t i=0; i < nextHops->size(); i++) {
        result[i][0] = (*nextHops)[i].getIp().str();
        result[i][1] = (*nextHops)[i].getPort();
        result[i][2] = (*nextHops)[i].getKey().toString(16);
    }

    delete nextHops;
}

void XmlRpcInterface::lookup(XmlRpcValue& params, XmlRpcValue& result)
{
    if (((params.size() != 2)
            || (params[0].getType() != XmlRpcValue::TypeBase64)
            || (params[1].getType() != XmlRpcValue::TypeInt))
        && ((params.size() != 3)
            || (params[0].getType() != XmlRpcValue::TypeBase64)
            || (params[1].getType() != XmlRpcValue::TypeInt)
            || (params[2].getType() != XmlRpcValue::TypeInt)))
        throw XmlRpcException("lookup(base64 key, int numSiblings(, "
                              "int RoutingType)): Invalid argument type");

    if ((int)params[1] > overlay->getMaxNumSiblings())
        throw XmlRpcException("lookup(base64 key, int numSiblings(, "
                              "int RoutingType)): numSibling to big");

    if (params.size() == 3) {
        if (((int)params[2] != DEFAULT_ROUTING) &&
            ((int)params[2] != ITERATIVE_ROUTING) &&
            ((int)params[2] != EXHAUSTIVE_ITERATIVE_ROUTING) &&
            ((int)params[2] != SEMI_RECURSIVE_ROUTING) &&
            ((int)params[2] != FULL_RECURSIVE_ROUTING) &&
            ((int)params[2] != RECURSIVE_SOURCE_ROUTING)) {

            throw XmlRpcException("lookup(base64 key, int numSiblings(, "
                                  "int RoutingType)): invalid routingType");
        }
    }

    state[curAppFd]._connectionState = EXECUTE_REQUEST;

    LookupCall* lookupCall = new LookupCall();

    BinaryValue keyString = (const XmlRpcValue::BinaryData&)params[0];
    lookupCall->setKey(OverlayKey::sha1(keyString));
    lookupCall->setNumSiblings(params[1]);

    if (params.size() == 3) {
        lookupCall->setRoutingType(params[2]);
    }

    sendInternalRpcWithTimeout(OVERLAY_COMP, lookupCall);
}

void XmlRpcInterface::joinOverlay(XmlRpcValue& params, XmlRpcValue& result)
{
    if ((params.size() != 1)
            || (params[0].getType() != XmlRpcValue::TypeBase64))
        throw XmlRpcException("join(base64 nodeID): Invalid argument type");

    if (!isPrivileged()) {
        throw XmlRpcException("join(base64 nodeID): Not allowed");
    }

    BinaryValue nodeID = (const XmlRpcValue::BinaryData&)params[0];

    overlay->join(OverlayKey::sha1(nodeID));

    result[0] = 0;
}

void XmlRpcInterface::put(XmlRpcValue& params, XmlRpcValue& result)
{
    if ((params.size() != 4)
            || (params[0].getType() != XmlRpcValue::TypeBase64)
            || (params[1].getType() != XmlRpcValue::TypeBase64)
            || (params[2].getType() != XmlRpcValue::TypeInt)
            || (params[3].getType() != XmlRpcValue::TypeString))
        throw XmlRpcException("put(base64 key, base64 value, int ttl "
                ", string application): Invalid argument type");

    if (!isPrivileged()) {
        throw XmlRpcException("put(base64 key, base64 value, int ttl "
                ", string application): Not allowed");
    }

    if (overlay->getCompModule(TIER1_COMP) == NULL)
        throw XmlRpcException("put(base64 key, base64 value, int ttl "
                ", string application): No DHT service");

    state[curAppFd]._connectionState = EXECUTE_REQUEST;

    DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();

    BinaryValue keyString = (const XmlRpcValue::BinaryData&)params[0];

    dhtPutMsg->setKey(OverlayKey::sha1(keyString));
    dhtPutMsg->setValue(((const XmlRpcValue::BinaryData&)params[1]));
    dhtPutMsg->setTtl(params[2]);
    dhtPutMsg->setIsModifiable(true);

    sendInternalRpcWithTimeout(TIER1_COMP, dhtPutMsg);
}

void XmlRpcInterface::get(XmlRpcValue& params, XmlRpcValue& result)
{
    if ((params.size() != 4)
            || (params[0].getType() != XmlRpcValue::TypeBase64)
            || (params[1].getType() != XmlRpcValue::TypeInt)
            || (params[2].getType() != XmlRpcValue::TypeBase64)
            || (params[3].getType() != XmlRpcValue::TypeString))
        throw XmlRpcException("get(base64 key, int num, base64 placemark "
                ", string application): Invalid argument type");

    if (overlay->getCompModule(TIER1_COMP) == NULL)
        throw XmlRpcException("get(base64 key, int num, base64 placemark "
                ", string application): No DHT service");

    state[curAppFd]._connectionState = EXECUTE_REQUEST;

    DHTgetCAPICall* dhtGetMsg = new DHTgetCAPICall();

    BinaryValue keyString = (const XmlRpcValue::BinaryData&)params[0];
    dhtGetMsg->setKey(OverlayKey::sha1(keyString));

    sendInternalRpcWithTimeout(TIER1_COMP, dhtGetMsg);
}

void XmlRpcInterface::dumpDht(XmlRpcValue& params, XmlRpcValue& result)
{
    if (params.size() != 1) {
        throw XmlRpcException("dump_dht(int dummy): Invalid argument type");
    }

    if (!isPrivileged()) {
         throw XmlRpcException("dump_dht(int dummy): Not allowed");
     }

    if (overlay->getCompModule(TIER1_COMP) == NULL)
        throw XmlRpcException("dump_dht(): No DHT service");

    state[curAppFd]._connectionState = EXECUTE_REQUEST;

    DHTdumpCall* call = new DHTdumpCall();

    sendInternalRpcWithTimeout(TIER1_COMP, call);
}

bool XmlRpcInterface::isPrivileged()
{
    if (limitAccess) {
        return state[curAppFd].localhost;
    } else {
        return true;
    }
}

void XmlRpcInterface::initializeApp(int stage)
{
    // all initialization is done in the first stage
    if (stage != MAX_STAGE_APP)
        return;

    packetNotification = new cMessage("packetNotification");
    mtu = par("mtu");
    limitAccess = par("limitAccess");

    scheduler = check_and_cast<RealtimeScheduler *>(simulation.getScheduler());
    scheduler->setInterfaceModule(this, packetNotification, &packetBuffer, mtu,
                                  true);

    appTunFd = scheduler->getAppTunFd();

    p2pns = dynamic_cast<P2pns*>(overlay->getCompModule(TIER2_COMP));

    XmlRpc::setVerbosity(1);

    _localLookup = new LocalLookup(this);
    _lookup = new Lookup(this);
    _register = new P2pnsRegister(this);
    _resolve = new P2pnsResolve(this);
    _put = new Put(this);
    _get = new Get(this);
    _dumpDht = new DumpDht(this);
    _joinOverlay = new JoinOverlay(this);

    enableIntrospection(true);

    curAppFd = INVALID_SOCKET;
}

XmlRpcInterface::XmlRpcInterface()
{
    p2pns = NULL;

    _localLookup = NULL;
    _lookup = NULL;
    _register = NULL;
    _resolve = NULL;
    _put = NULL;
    _get = NULL;
    _dumpDht = NULL;
    _joinOverlay = NULL;

    packetNotification = NULL;
}

XmlRpcInterface::~XmlRpcInterface()
{
    delete _localLookup;
    delete _lookup;
    delete _register;
    delete _resolve;
    delete _put;
    delete _get;
    delete _dumpDht;
    delete _joinOverlay;

    cancelAndDelete(packetNotification);
}

void XmlRpcInterface::resetConnectionState()
{
    if (state.count(curAppFd) && state[curAppFd].pendingRpc) {
        cancelRpcMessage(state[curAppFd].pendingRpc);
    }

    state[curAppFd].appFd = INVALID_SOCKET;
    state[curAppFd].localhost = false;
    state[curAppFd]._header = "";
    state[curAppFd]._request = "";
    state[curAppFd]._response = "";
    state[curAppFd]._connectionState = READ_HEADER;
    state[curAppFd]._keepAlive = true;
    state[curAppFd].pendingRpc = 0;
}

void XmlRpcInterface::closeConnection()
{
    scheduler->closeAppSocket(curAppFd);
    resetConnectionState();
}

void XmlRpcInterface::sendInternalRpcWithTimeout(CompType destComp,
                                                 BaseCallMessage *call)
{
    state[curAppFd].pendingRpc = sendInternalRpcCall(destComp, call, NULL,
                                                     XMLRPC_TIMEOUT, 0,
                                                     curAppFd);
}

void XmlRpcInterface::handleMessage(cMessage *msg)
{
    // Packet from the application...
    if (msg==packetNotification) {
        EV << "[XmlRpcInterface::handleMessage() @ " << overlay->getThisNode().getIp()
        << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
        << "    Message from application. Queue length = " << packetBuffer.size()
        << endl;
        while (packetBuffer.size() > 0) {
            // get packet from buffer and parse it
            RealtimeScheduler::PacketBufferEntry packet =
                    *(packetBuffer.begin());
            packetBuffer.pop_front();
            curAppFd = packet.fd;

            switch (packet.func) {
            case RealtimeScheduler::PacketBufferEntry::PACKET_APPTUN_DATA:
                handleAppTunPacket(packet.data, packet.length);
                break;

            case RealtimeScheduler::PacketBufferEntry::PACKET_DATA:
                if (state.count(curAppFd) == 0) {
                    throw cRuntimeError("XmlRpcInterface::handleMessage(): "
                                            "Received packet "
                                            "from unknown socket!");
                }

                handleRealworldPacket(packet.data, packet.length);
                break;

            case RealtimeScheduler::PacketBufferEntry::PACKET_FD_NEW:
                if (state.count(curAppFd)) {
                    throw cRuntimeError("XmlRpcInterface::handleMessage(): "
                                            "Connection state table corrupt!");
                }

                resetConnectionState();

                if (packet.addr != NULL) {
                    if (((sockaddr_in*)packet.addr)->sin_addr.s_addr
                            == inet_addr("127.0.0.1")) {
                        state[curAppFd].localhost = true;
                    }
                    delete packet.addr;
                    packet.addr = NULL;
                }
                break;

            case RealtimeScheduler::PacketBufferEntry::PACKET_FD_CLOSE:
                if (state.count(curAppFd) == 0) {
                    throw cRuntimeError("XmlRpcInterface::handleMessage(): "
                                            "Trying to close unknown "
                                            "connection!");
                }

                resetConnectionState();
                state.erase(curAppFd);
            }

            if (packet.data) {
                delete[] packet.data;
            }
        }
    } else if (msg->isSelfMessage()) {
        // process rpc self-messages
        BaseRpcMessage* rpcMessage = dynamic_cast<BaseRpcMessage*>(msg);
        if (rpcMessage!=NULL) {
            internalHandleRpcMessage(rpcMessage);
            return;
        }

        delete msg;
    } else {
        // RPCs
        BaseRpcMessage* rpcMessage = dynamic_cast<BaseRpcMessage*>(msg);
        if (rpcMessage!=NULL) {
            internalHandleRpcMessage(rpcMessage);
            return;
        }
        // common API
        CommonAPIMessage* commonAPIMsg = dynamic_cast<CommonAPIMessage*>(msg);
        if (commonAPIMsg != NULL)
            handleCommonAPIPacket(commonAPIMsg);

        CompReadyMessage* readyMsg = dynamic_cast<CompReadyMessage*>(msg);
        if (readyMsg != NULL)
            handleReadyMessage(readyMsg);

        delete msg;
    }
}

void XmlRpcInterface::handleRpcTimeout(BaseCallMessage* msg,
                                       const TransportAddress& dest,
                                       cPolymorphic* context, int rpcId,
                                       const OverlayKey&)
{
    curAppFd = rpcId;

    if (state.count(curAppFd) == 0)
        return;

    std::cout << "XmlRpcInterface(): XML-RPC failed!" << endl;
    state[curAppFd]._response = generateFaultResponse("XML-RPC timeout", 22);
    state[curAppFd]._connectionState = WRITE_RESPONSE;
    if (!writeResponse() ) {
        closeConnection();
    }
}

void XmlRpcInterface::handleReadyMessage(CompReadyMessage* msg)
{
    if ((msg->getReady() == false) || (msg->getComp() != OVERLAY_COMP)) {
        return;
    }

    if (appTunFd != INVALID_SOCKET) {
        // set TUN interface address using the current NodeId
        // TODO: this is ugly
        const OverlayKey& key = overlay->getThisNode().getKey();

        if (OverlayKey::getLength() < 100) {
            throw cRuntimeError("XmlRpcInterface::handleReadyMessage(): "
                    "P2PNS needs at least 100 bit nodeIds!");
        }

        std::stringstream addr;
        addr << "2001:001";
        for (int i = 0; i < 100/4; i++) {
            if (((i + 3) % 4) == 0) {
                addr << ":";
            }
            addr << std::hex << key.getBitRange(OverlayKey::getLength() -
                                                4 * (i + 1), 4);
        }

        std::string cmd = "/sbin/ip addr add " + addr.str() + "/28 dev tun0";

        EV << "XmlRpcInterface::handleOverlayReady(): "
              "Setting TUN interface address " << addr.str() << endl;

        if (system(cmd.c_str()) != 0) {
            EV << "XmlRpcInterface::handleOverlayReady(): "
                  "Failed to set TUN interface address!" << endl;
        }

        if (system("/sbin/ip link set tun0 up") != 0) {
            EV << "XmlRpcInterface::handleOverlayReady(): "
                  "Failed to set TUN interface up!" << endl;
        }

        p2pns->registerId(addr.str());
    }
}

void XmlRpcInterface::handleAppTunPacket(char *buf, uint32_t length)
{
#if not defined _WIN32 && not defined __APPLE__
    EV << "XmlRpcInterface::handleAppTunPacket(): packet of "
       << "length " << length << endl;

    if (!p2pns) {
        throw cRuntimeError("XmlRpcInterface::handleAppTunPacket(): "
                "P2PNS module missing on tier2!");
    }

    if (OverlayKey::getLength() < 100) {
        throw cRuntimeError("XmlRpcInterface::handleAppTunPacket(): "
                "P2PNS needs at least 100 bit nodeIds!");
    }

    if (length < 40) {
        EV << "XmlRpcInterface::handleAppTunPacket(): packet too "
           << "short - discarding packet!" << endl;
        return;
    }

    ip6_hdr* ip_buf = (ip6_hdr*) buf;
    if (((ip_buf->ip6_vfc & 0xf0) >> 4) != 6) {
        EV << "XmlRpcInterface::handleAppTunPacket(): received packet "
              "is no IPv6 - discarding packet!" << endl;
        return;
    }

    OverlayKey destKey = OverlayKey(ntohl(ip_buf->ip6_dst.s6_addr32[0]));

    for (int i = 1; i < 4; i++) {
        destKey = (destKey << 32) + OverlayKey(ntohl(ip_buf->ip6_dst.s6_addr32[i]));
    }
    destKey = destKey << (OverlayKey::getLength() - 100);

    p2pns->tunnel(destKey, BinaryValue(buf, buf + length));
#endif
}

void XmlRpcInterface::deliverTunneledMessage(const BinaryValue& payload)
{
#if not defined _WIN32 && not defined __APPLE__
    Enter_Method_Silent();

    if (payload.size() == 0) {
        return;
    }

    int curBytesWritten = scheduler->sendBytes(&payload[0],
                                               payload.size(),
                                               0, 0, true, appTunFd);

    if (curBytesWritten <= 0) {
        throw cRuntimeError("XmlRpcServerConnection::deliverTunneledMessage(): "
                            "Error writing to application TUN device.");
    }
#endif
}

void XmlRpcInterface::handleRealworldPacket(char *buf, uint32_t length)
{
    if (state[curAppFd]._connectionState == READ_HEADER) {
        if (!readHeader(buf, length)) {
            // discard data, if the header is invalid
            state[curAppFd]._header = "";
            state[curAppFd]._request = "";
            state[curAppFd]._response = "";
            state[curAppFd]._connectionState = READ_HEADER;
            return;
        }
    }

    if (state[curAppFd]._connectionState == READ_REQUEST)
        if (!readRequest(buf, length))
            return;

    if (state[curAppFd]._connectionState == WRITE_RESPONSE)
        if (!writeResponse() ) {
            closeConnection();
            return;
        }

    return;
}

void XmlRpcInterface::handleRpcResponse(BaseResponseMessage* msg,
                                        cPolymorphic* context,
                                        int rpcId,
                                        simtime_t rtt)
{
    curAppFd = rpcId;

    if (state.count(curAppFd) == 0) {
        return;
    }

    RPC_SWITCH_START(msg)
    RPC_ON_RESPONSE(Lookup) {
        if (state[curAppFd]._connectionState != EXECUTE_REQUEST) break;

        XmlRpcValue resultValue;
        resultValue.setSize(_LookupResponse->getSiblingsArraySize());

        if (_LookupResponse->getIsValid() == true) {
            for (uint32_t i=0; i < _LookupResponse->getSiblingsArraySize();
                    i++) {
                resultValue[i].setSize(3);
                resultValue[i][0] =
                    _LookupResponse->getSiblings(i).getIp().str();
                resultValue[i][1] =
                    _LookupResponse->getSiblings(i).getPort();
                resultValue[i][2] =
                    _LookupResponse->getSiblings(i).getKey().toString(16);
            }
            state[curAppFd]._response = generateResponse(resultValue.toXml());
        } else {
            std::cout << "XmlRpcInterface(): lookup() failed!" << endl;
            state[curAppFd]._response = generateFaultResponse("lookup() failed", 22);
        }

        state[curAppFd]._connectionState = WRITE_RESPONSE;
        if (!writeResponse()) {
            closeConnection();
        }
        break;
    }
    RPC_ON_RESPONSE(P2pnsRegister) {
        if (state[curAppFd]._connectionState != EXECUTE_REQUEST)
            break;

        XmlRpcValue resultValue;

        if (_P2pnsRegisterResponse->getIsSuccess() == true) {
            resultValue = 0;
            state[curAppFd]._response = generateResponse(resultValue.toXml());
        } else {
            std::cout << "XmlRpcInterface(): register() failed!" << endl;
            state[curAppFd]._response = generateFaultResponse("register() failed", 22);
        }

        state[curAppFd]._connectionState = WRITE_RESPONSE;
        if (!writeResponse() ) {
            closeConnection();
        }
        break;
    }
    RPC_ON_RESPONSE(P2pnsResolve) {
        if (state[curAppFd]._connectionState != EXECUTE_REQUEST)
            break;

        XmlRpcValue resultValue;
        resultValue.setSize(_P2pnsResolveResponse->getAddressArraySize());

        if (_P2pnsResolveResponse->getIsSuccess() == true) {
            for (uint i=0; i < _P2pnsResolveResponse->getAddressArraySize(); i++) {
                resultValue[i].setSize(3);
                BinaryValue& addr = _P2pnsResolveResponse->getAddress(i);
                resultValue[i][0] = XmlRpcValue(&addr[0], addr.size());
                resultValue[i][1] = (int)_P2pnsResolveResponse->getKind(i);
                resultValue[i][2] = (int)_P2pnsResolveResponse->getId(i);
            }
            state[curAppFd]._response = generateResponse(resultValue.toXml());
        } else {
            std::cout << "XmlRpcInterface(): resolve() failed!" << endl;
            state[curAppFd]._response = generateFaultResponse("resolve() failed: Name not found", 9);
        }

        state[curAppFd]._connectionState = WRITE_RESPONSE;
        if (!writeResponse() ) {
            closeConnection();
        }
        break;
    }
    RPC_ON_RESPONSE(DHTputCAPI) {
        if (state[curAppFd]._connectionState != EXECUTE_REQUEST)
            break;

        XmlRpcValue resultValue;

        if (_DHTputCAPIResponse->getIsSuccess() == true) {
            resultValue = 0;
            state[curAppFd]._response = generateResponse(resultValue.toXml());
        } else {
            std::cout << "XmlRpcInterface(): put() failed!" << endl;
            state[curAppFd]._response = generateFaultResponse("put() failed", 22);
        }

        state[curAppFd]._connectionState = WRITE_RESPONSE;
        if (!writeResponse() ) {
            closeConnection();
        }
        break;
    }
    RPC_ON_RESPONSE(DHTgetCAPI) {
        if (state[curAppFd]._connectionState != EXECUTE_REQUEST)
            break;

        XmlRpcValue resultValue;
        resultValue.setSize(2);
        resultValue[0].setSize(_DHTgetCAPIResponse->getResultArraySize());

        if (_DHTgetCAPIResponse->getIsSuccess() == true) {
            for (uint i=0; i < _DHTgetCAPIResponse->getResultArraySize(); i++) {
                resultValue[i].setSize(2);
                DhtDumpEntry& entry = _DHTgetCAPIResponse->getResult(i);
                resultValue[0][i] = XmlRpcValue(&(*(entry.getValue().begin())),
                                                entry.getValue().size());
            }
            resultValue[1] = std::string();

//            resultValue[0][0] = XmlRpcValue(
//                      &(*(_DHTgetCAPIResponse->getValue().begin())),
//                      _DHTgetCAPIResponse->getValue().size());
            state[curAppFd]._response = generateResponse(resultValue.toXml());
        } else {
            std::cout << "XmlRpcInterface(): get() failed!" << endl;
            state[curAppFd]._response = generateFaultResponse("get() failed", 22);
        }

        state[curAppFd]._connectionState = WRITE_RESPONSE;
        if (!writeResponse() ) {
            closeConnection();
        }
        break;
    }
    RPC_ON_RESPONSE(DHTdump) {
        if (state[curAppFd]._connectionState != EXECUTE_REQUEST)
            break;

        XmlRpcValue resultValue;
        resultValue.setSize(_DHTdumpResponse->getRecordArraySize());

        for (uint32_t i=0; i < _DHTdumpResponse->getRecordArraySize();
             i++) {
            resultValue[i].setSize(3);
            resultValue[i][0] =
                _DHTdumpResponse->getRecord(i).getKey().toString(16);
            resultValue[i][1] = XmlRpcValue(
                 &(*(_DHTdumpResponse->getRecord(i).getValue().begin())),
                 _DHTdumpResponse->getRecord(i).getValue().size());
            resultValue[i][2] =
                _DHTdumpResponse->getRecord(i).getTtl();
        }

        state[curAppFd]._response = generateResponse(resultValue.toXml());

        state[curAppFd]._connectionState = WRITE_RESPONSE;
         if (!writeResponse()) {
             closeConnection();
         }
         break;

    }
    RPC_SWITCH_END( )
}

void XmlRpcInterface::handleCommonAPIPacket(cMessage *msg)
{
    error("DHTXMLRealworldApp::handleCommonAPIPacket(): Unknown Packet!");
}

bool XmlRpcInterface::readHeader(char* buf, uint32_t length)
{
    // Read available data
    bool eof = false;

    state[curAppFd]._header.append(std::string(buf, length));

    if (length <= 0) {
        // Its only an error if we already have read some data
        if (state[curAppFd]._header.length() > 0)
            XmlRpcUtil::error("XmlRpcServerConnection::readHeader: error "
                "while reading header.");
        return false;
    }

    XmlRpcUtil::log(4, "XmlRpcServerConnection::readHeader: read %d bytes.",
                    state[curAppFd]._header.length());
    char *hp = (char*)state[curAppFd]._header.c_str(); // Start of header
    char *ep = hp + state[curAppFd]._header.length(); // End of string
    char *bp = 0; // Start of body
    char *lp = 0; // Start of content-length value
    char *kp = 0; // Start of connection value

    for (char *cp = hp; (bp == 0) && (cp < ep); ++cp) {
        if ((ep - cp > 16) && (strncasecmp(cp, "Content-length: ", 16) == 0))
            lp = cp + 16;
        else if ((ep - cp > 12) && (strncasecmp(cp, "Connection: ", 12) == 0))
            kp = cp + 12;
        else if ((ep - cp > 4) && (strncmp(cp, "\r\n\r\n", 4) == 0))
            bp = cp + 4;
        else if ((ep - cp > 2) && (strncmp(cp, "\n\n", 2) == 0))
            bp = cp + 2;
    }

    // If we haven't gotten the entire header yet, return (keep reading)
    if (bp == 0) {
        // EOF in the middle of a request is an error, otherwise its ok
        if (eof) {
            XmlRpcUtil::log(4, "XmlRpcServerConnection::readHeader: EOF");
            if (state[curAppFd]._header.length() > 0)
                XmlRpcUtil::error("XmlRpcServerConnection::readHeader: EOF while reading header");
            return false; // Either way we close the connection
        }

        return true; // Keep reading
    }

    // Decode content length
    if (lp == 0) {
        XmlRpcUtil::error("XmlRpcServerConnection::readHeader: No Content-length specified");
        return false; // We could try to figure it out by parsing as we read, but for now...
    }

    state[curAppFd]._contentLength = atoi(lp);
    if (state[curAppFd]._contentLength <= 0) {
        XmlRpcUtil::error(
                          "XmlRpcServerConnection::readHeader: Invalid Content-length specified (%d).",
                          state[curAppFd]._contentLength);
        return false;
    }

    XmlRpcUtil::log(
                     3,
                    "XmlRpcServerConnection::readHeader: specified content length is %d.",
                    state[curAppFd]._contentLength);

    // Otherwise copy non-header data to request buffer and set state to read request.
    state[curAppFd]._request = bp;

    // Parse out any interesting bits from the header (HTTP version, connection)
    state[curAppFd]._keepAlive = true;
    if (state[curAppFd]._header.find("HTTP/1.0") != std::string::npos) {
        if (kp == 0 || strncasecmp(kp, "keep-alive", 10) != 0)
            state[curAppFd]._keepAlive = false; // Default for HTTP 1.0 is to close the connection
    } else {
        if (kp != 0 && strncasecmp(kp, "close", 5) == 0)
            state[curAppFd]._keepAlive = false;
    }
    XmlRpcUtil::log(3, "KeepAlive: %d", state[curAppFd]._keepAlive);

    state[curAppFd]._header = "";
    state[curAppFd]._connectionState = READ_REQUEST;
    return true; // Continue monitoring this source
}

bool XmlRpcInterface::readRequest(char *buf, uint32_t length)
{
    // If we dont have the entire request yet, read available data
    if (int(state[curAppFd]._request.length()) < state[curAppFd]._contentLength) {
        bool eof = false;

        state[curAppFd]._request.append(std::string(buf, length));

        if (length <= 0) {
            XmlRpcUtil::error("XmlRpcServerConnection::readRequest: read error.");
            return false;
        }

        // If we haven't gotten the entire request yet, return (keep reading)
        if (int(state[curAppFd]._request.length()) < state[curAppFd]._contentLength) {
            if (eof) {
                XmlRpcUtil::error("XmlRpcServerConnection::readRequest: EOF while reading request");
                return false; // Either way we close the connection
            }
            return true;
        }
    }

    // Otherwise, parse and dispatch the request
    XmlRpcUtil::log(3, "XmlRpcServerConnection::readRequest read %d bytes.",
                    state[curAppFd]._request.length());
    //XmlRpcUtil::log(5, "XmlRpcServerConnection::readRequest:\n%s\n", state[curAppFd]._request.c_str());

    state[curAppFd]._connectionState = WRITE_RESPONSE;

    return true; // Continue monitoring this source
}

bool XmlRpcInterface::writeResponse()
{
    if (state[curAppFd]._response.length() == 0) {
        state[curAppFd]._response = executeRequest(state[curAppFd]._request);
        state[curAppFd]._bytesWritten = 0;

        if (state[curAppFd]._connectionState == EXECUTE_REQUEST)
            return true;

        if (state[curAppFd]._response.length() == 0) {
            XmlRpcUtil::error("XmlRpcServerConnection::writeResponse: empty response.");
            return false;
        }
    }

    // Try to write the response
    int curBytesWritten = scheduler->sendBytes(state[curAppFd]._response.c_str(),
                                               state[curAppFd]._response.length(),
                                               0, 0, true, curAppFd);

    if (curBytesWritten <= 0) {
        XmlRpcUtil::error("XmlRpcServerConnection::writeResponse: write error.");
        return false;
    } else {
        state[curAppFd]._bytesWritten += curBytesWritten;
    }

    XmlRpcUtil::log(3,
                "XmlRpcServerConnection::writeResponse: wrote %d of %d bytes.",
                state[curAppFd]._bytesWritten, state[curAppFd]._response.length());

    // Prepare to read the next request
    if (state[curAppFd]._bytesWritten == int(state[curAppFd]._response.length())) {
        state[curAppFd]._header = "";
        state[curAppFd]._request = "";
        state[curAppFd]._response = "";
        state[curAppFd]._connectionState = READ_HEADER;
    }

    return state[curAppFd]._keepAlive; // Continue monitoring this source if true
}
