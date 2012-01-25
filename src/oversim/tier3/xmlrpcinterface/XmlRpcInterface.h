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
 * @file XmlRpcInterface.h
 * @author Ingmar Baumgart
 */

#ifndef _XMLRPCINTERFACE_H__
#define _XMLRPCINTERFACE_H__
#include <realtimescheduler.h>
#include <omnetpp.h>
#include <CommonMessages_m.h>
#include <DHTMessage_m.h>
#include <BaseOverlay.h>
#include <BaseApp.h>
#include <tunoutscheduler.h>
#include <XmlRpc.h>

class P2pns;

const int XMLRPC_TIMEOUT=30;


/**
 * Applicaton that communicates with a realword application via a socket
 */
class XmlRpcInterface : public BaseApp, public XmlRpc::XmlRpcServer
{
protected:
    unsigned int mtu;

    cMessage* packetNotification; // used by TunOutScheduler to notify about new packets
    RealtimeScheduler::PacketBuffer packetBuffer; // received packets are stored here
    RealtimeScheduler* scheduler;

    //! Reads the http header
    bool readHeader(char *buf, uint32_t length);

    //! Reads the request (based on the content-length header value)
    bool readRequest(char *buf, uint32_t length);

    //! Executes the request and writes the resulting response
    bool writeResponse();

    //! Possible IO states for the connection
    enum ServerConnectionState { READ_HEADER, READ_REQUEST, EXECUTE_REQUEST,
			     WRITE_RESPONSE };

    struct XmlRpcConnectionState {
        //! Current IO state for the connection
        ServerConnectionState _connectionState;

        //! Request headers
        std::string _header;

        //! Number of bytes expected in the request body (parsed from header)
        int _contentLength;

        //! Request body
        std::string _request;

        //! Response
        std::string _response;

        //! Number of bytes of the response written so far
        int _bytesWritten;

        //! Whether to keep the current client connection open for further requests
        bool _keepAlive;

        //! the fd for the current app socket
        SOCKET appFd;

        //! true, if the connection is from localhost
        bool localhost;

        //! the nonce of the pending internal RPC this connection is waiting for
        uint32_t pendingRpc;
    };

    std::map<int, XmlRpcConnectionState> state;
    SOCKET curAppFd;
    bool limitAccess;

    XmlRpc::XmlRpcServerMethod* _localLookup;
    XmlRpc::XmlRpcServerMethod* _lookup;
    XmlRpc::XmlRpcServerMethod* _register;
    XmlRpc::XmlRpcServerMethod* _resolve;
    XmlRpc::XmlRpcServerMethod* _put;
    XmlRpc::XmlRpcServerMethod* _get;
    XmlRpc::XmlRpcServerMethod* _dumpDht;
    XmlRpc::XmlRpcServerMethod* _joinOverlay;

    /**
     * Check if the connected application is allowed to call privileged
     * methods. Currently this is true for all applications connecting to
     * localhost. If **.limitAccess = false there are no access restrictions.
     *
     * @return true, if the application is allowed to call privileged methods
     */
    bool isPrivileged();

    void handleAppTunPacket(char *buf, uint32_t len);
    void handleRealworldPacket(char *buf, uint32_t len);
    void handleCommonAPIPacket(cMessage *msg);
    void handleRpcResponse(BaseResponseMessage* msg,
                           cPolymorphic* context,
                           int rpcId,
                           simtime_t rtt);
    /**
     * Reset the internal connection state. This is called after a RPC
     * has finished and the socket was closed.
     */
    void resetConnectionState();

    void closeConnection();
    void sendInternalRpcWithTimeout(CompType destComp, BaseCallMessage *call);
    virtual void handleReadyMessage(CompReadyMessage* msg);

    SOCKET appTunFd; /**< FD of the application TUN socket used for tunneling */
    P2pns* p2pns; /**< Pointer to the P2PNS module */

public:
    XmlRpcInterface();
    ~XmlRpcInterface();

    virtual void initializeApp(int stage);

    /**
     * The "main loop". Every message that is received or send is handled
     * by this method
     */
    virtual void handleMessage(cMessage *msg);

    // see BaseRpc.cc
    void handleRpcTimeout(BaseCallMessage* msg,
                          const TransportAddress& dest,
                          cPolymorphic* context, int rpcId,
                          const OverlayKey&);

    void deliverTunneledMessage(const BinaryValue& payload);

    void localLookup(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
    void lookup(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
    void p2pnsRegister(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
    void p2pnsResolve(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
    void put(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
    void get(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
    void dumpDht(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
    void joinOverlay(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);
};

#endif
