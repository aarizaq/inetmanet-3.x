//
// Copyright (C) 2010 Institut fuer Telematik, Karlsruher Institut fuer Technologie (KIT)
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
 * @file BaseTcpSupport.h
 * @author Bernhard Mueller
 */

#ifndef BASETCPSUPPORT_H_
#define BASETCPSUPPORT_H_

#include <omnetpp.h>
#include <map>
#include <TransportAddress.h>
#include <TCPSocket.h>
#include <ExtTCPSocketMap.h>

class BaseTcpSupport : public TCPSocket::CallbackInterface
{
public:

    // Event codes
    enum EvCode {NO_EST_CONNECTION, PEER_CLOSED, PEER_TIMEDOUT, PEER_REFUSED, CONNECTION_RESET, CONNECTION_SUCC_ClOSED};

    // Utility methods of the CallbackInterface
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketPeerClosed(int connId, void *yourPtr);
//    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}

protected:

    /**
     * Member function to handle incoming TCP messages
     */
    void handleTCPMessage(cMessage* msg);

    /**
     * Member function to bind service to the specified port and listen afterwards
     *
     * @param port local portnumber to bind on
     */
    void bindAndListenTcp(int port);

    /**
     * Member function to check if the service is already connected
     *
     * @param address transport address of the remote host
     */
    bool isAlreadyConnected(TransportAddress address);

    /**
     * Member function to establish a connection to the specified node
     *
     * @param address transport address of the remote host
     */
    void establishTcpConnection(TransportAddress address);

    /**
     * Member function to send TCP data to the specified node
     *
     * @param msg data message
     * @param address transport address of the remote host
     */
    void sendTcpData(cPacket* msg, TransportAddress address);

    /**
     * Member function to handle passive connection events
     *
     * @param code event code of the current event
     * @param address transport address of the remote host
     */
    virtual void handleConnectionEvent(EvCode code, TransportAddress address);

    /**
     * Member function to handle incoming data
     *
     * @param address transport address of the remote host
     * @param msg incoming data message
     * @param urgent message urgency
     */
    virtual void handleDataReceived(TransportAddress address, cPacket* msg, bool urgent);

    /**
     * Member function to handle newly opened connections
     *
     * @param address transport address of the remote host
     */
    virtual void handleIncomingConnection(TransportAddress address);

    /**
     * Member function to close an established connection
     *
     * @param address transport address of the remote host
     */
    void closeTcpConnection(TransportAddress address);

    /**
     * Member function to set local gate towards the TCP module during init phase
     *
     * @param gate local gate towards the TCP module
     */
    void setTcpOut(cGate* gate) {tcpOut = gate;}

    /**
     * Member function to get local gate towards the TCP module
     *
     * @returns The local gate towards the TCP module
     */
    cGate* getTcpOut() {return tcpOut;}

private:

    ExtTCPSocketMap sockets;  ///< Socket map with extended functionality to find sockets
    typedef std::vector<cMessage*> msgQueue;
    typedef std::map<TransportAddress, msgQueue*> transQueue;
    transQueue queuedTx; ///< msg queue partitioned by destination

    cGate* tcpOut; ///< local gate towards the TCP module
};

#endif /* BASETCPSUPPORT_H_ */
