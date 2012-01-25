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
 * @file BaseTcpSupport.cc
 * @author Bernhard Mueller
 */

#include "BaseTcpSupport.h"
#include <GlobalStatisticsAccess.h>
#include <UDPSocket.h>
#include <TCPCommand_m.h>
#include <omnetpp.h>

void BaseTcpSupport::handleTCPMessage(cMessage* msg)
{
    TCPSocket* socket = sockets.findSocketFor(msg);

    if (socket == NULL) {
        socket = new TCPSocket(msg);
        socket->setCallbackObject(this);
        socket->setOutputGate(getTcpOut());
        sockets.addSocket(socket);
        TransportAddress newAddress =
                TransportAddress(socket->getRemoteAddress(),
                                 socket->getRemotePort());

        socket->processMessage(msg);
        handleIncomingConnection(newAddress);
    } else {
        socket->processMessage(msg);
    }
}

void BaseTcpSupport::bindAndListenTcp(int port)
{
    if (sockets.size() != 0) {
        return;
    }

    TCPSocket* newSocket = new TCPSocket();
    newSocket->bind(port);
    newSocket->setOutputGate(getTcpOut());
    newSocket->setCallbackObject(this);
    newSocket->listen();
    sockets.addSocket(newSocket);
}

bool BaseTcpSupport::isAlreadyConnected(TransportAddress address)
{
    TCPSocket* newSocket = sockets.findSocketFor(address.getIp(),
                                                 address.getPort());
    if (newSocket == NULL) {
        return false;
    } else if (newSocket->getState() >= TCPSocket::PEER_CLOSED) {
        return false;
    } else return true;
}

void BaseTcpSupport::establishTcpConnection(TransportAddress address)
{
    if (isAlreadyConnected(address)) {
        return;
    }

    TCPSocket* newSocket = new TCPSocket();
    newSocket->setOutputGate(getTcpOut());
    newSocket->setCallbackObject(this);
    newSocket->connect(address.getIp(), address.getPort());

    sockets.addSocket(newSocket);
}

void BaseTcpSupport::sendTcpData(cPacket *msg, TransportAddress address)
{
    if (!isAlreadyConnected(address)) {
        handleConnectionEvent(NO_EST_CONNECTION, address);
        return;
    }

    TCPSocket* socket = sockets.findSocketFor(address.getIp(),
                                              address.getPort());

    if (socket->getState() == TCPSocket::CONNECTED) {
        socket->send(msg);
    }

    transQueue::iterator tx = queuedTx.find(address);

    if (tx != queuedTx.end()) {
        tx->second->push_back(msg);
    } else {
        msgQueue* newQueue = new msgQueue();
        newQueue->push_back(msg);
        queuedTx[address] = newQueue;
    }
}

void BaseTcpSupport::handleConnectionEvent(EvCode code, TransportAddress address)
{
    if (code == NO_EST_CONNECTION) {
        establishTcpConnection(address);
    }
}

void BaseTcpSupport::handleDataReceived(TransportAddress address, cPacket* msg,
                                        bool urgent)
{
}

void BaseTcpSupport::handleIncomingConnection(TransportAddress address)
{
}

void BaseTcpSupport::socketDataArrived(int connId, void *yourPtr, cPacket *msg,
                                       bool urgent)
{
    TCPSocket* socket = sockets.findSocketFor(connId);
    TransportAddress remoteAddress(socket->getRemoteAddress(),
                                   socket->getRemotePort());

    handleDataReceived(remoteAddress, msg, urgent);
}

void BaseTcpSupport::socketEstablished(int connId, void *yourPtr)
{
    TCPSocket* socket = sockets.findSocketFor(connId);

    if (socket == NULL) {
        return;
    }

    TransportAddress remoteAddress(socket->getRemoteAddress(),
                                   socket->getRemotePort());

    transQueue::iterator tx = queuedTx.find(remoteAddress);

    if (tx != queuedTx.end()) {
        for (uint32 i = 0 ; i < tx->second->size(); i++) {
            socket->send(tx->second->at(i));
        }

        tx->second->clear();
        delete tx->second;
        queuedTx.erase(remoteAddress);
    }
}

void BaseTcpSupport::socketPeerClosed(int connId, void *yourPtr)
{
    TCPSocket* socket = sockets.findSocketFor(connId);
    TransportAddress remoteAddress(socket->getRemoteAddress(),
                                   socket->getRemotePort());

    if (socket->getState() == TCPSocket::PEER_CLOSED) {
        socket->close();
        handleConnectionEvent(PEER_CLOSED, remoteAddress);
    }  else if (socket->getState() == TCPSocket::CLOSED) {
        sockets.removeSocket(socket);
        handleConnectionEvent(CONNECTION_SUCC_ClOSED, remoteAddress);
    }
}

void BaseTcpSupport::socketFailure(int connId, void *yourPtr, int code)
{
    TCPSocket* socket = sockets.findSocketFor(connId);
    TransportAddress remoteAddress(socket->getRemoteAddress(),
                                   socket->getRemotePort());

    if (code == TCP_I_CONNECTION_REFUSED) {
        handleConnectionEvent(PEER_REFUSED, remoteAddress);
    } else if (code == TCP_I_TIMED_OUT) {
        handleConnectionEvent(PEER_TIMEDOUT, remoteAddress);
    } else if (code == TCP_I_CONNECTION_RESET) {
        handleConnectionEvent(CONNECTION_RESET, remoteAddress);
    } else {
        throw new cRuntimeError("Invalid error code on socketFailure.");
    }
}

void BaseTcpSupport::closeTcpConnection(TransportAddress address)
{
    if (!isAlreadyConnected(address)) {
        return;
    }

    TCPSocket* oldSocket = sockets.findSocketFor(address.getIp(),
                                                 address.getPort());

    oldSocket->close();
}
