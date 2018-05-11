//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_TCPSRVHOSTAPP_H
#define __INET_TCPSRVHOSTAPP_H

#include "inet/common/INETDefs.h"

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

//forward declaration:
class TcpServerThreadBase;

/**
 * Hosts a server application, to be subclassed from TCPServerProcess (which
 * is a sSimpleModule). Creates one instance (using dynamic module creation)
 * for each incoming connection. More info in the corresponding NED file.
 */
class INET_API TcpServerHostApp : public cSimpleModule, public ILifecycle
{
  protected:
    TcpSocket serverSocket;
    SocketMap socketMap;
    typedef std::set<TcpServerThreadBase *> ThreadSet;
    ThreadSet threadSet;
    NodeStatus *nodeStatus = nullptr;

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    bool isNodeUp() { return !nodeStatus || nodeStatus->getState() == NodeStatus::UP; }
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void start();
    virtual void stop();
    virtual void crash();

  public:
    virtual ~TcpServerHostApp() { socketMap.deleteSockets(); }
    virtual void removeThread(TcpServerThreadBase *thread);
};

/**
 * Abstract base class for server processes to be used with TcpServerHostApp.
 * Subclasses need to be registered using the Register_Class() macro.
 *
 * @see TcpServerHostApp
 */
class INET_API TcpServerThreadBase : public cSimpleModule, public TcpSocket::ICallback
{
  protected:
    TcpServerHostApp *hostmod;
    TcpSocket *sock;    // ptr into socketMap managed by TcpServerHostApp

    // internal: TcpSocket::ICallback methods
    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override { dataArrived(msg, urgent); }
    virtual void socketEstablished(TcpSocket *socket) override { established(); }
    virtual void socketPeerClosed(TcpSocket *socket) override { peerClosed(); }
    virtual void socketClosed(TcpSocket *socket) override { closed(); }
    virtual void socketFailure(TcpSocket *socket, int code) override { failure(code); }
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override { statusArrived(status); }

    virtual void refreshDisplay() const override;

  public:

    TcpServerThreadBase() { sock = nullptr; hostmod = nullptr; }
    virtual ~TcpServerThreadBase() {}

    // internal: called by TcpServerHostApp after creating this module
    virtual void init(TcpServerHostApp *hostmodule, TcpSocket *socket) { hostmod = hostmodule; sock = socket; }

    /*
     * Returns the socket object
     */
    virtual TcpSocket *getSocket() { return sock; }

    /*
     * Returns pointer to the host module
     */
    virtual TcpServerHostApp *getHostModule() { return hostmod; }

    /**
     * Called when connection is established. To be redefined.
     */
    virtual void established() = 0;

    /*
     * Called when a data packet arrives. To be redefined.
     */
    virtual void dataArrived(Packet *msg, bool urgent) = 0;

    /*
     * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
     */
    virtual void timerExpired(cMessage *timer) = 0;

    /*
     * Called when the client closes the connection. By default it closes
     * our side too, but it can be redefined to do something different.
     */
    virtual void peerClosed() { getSocket()->close(); }

    /*
     * Called when the connection closes (successful TCP teardown). By default
     * it deletes this thread, but it can be redefined to do something different.
     */
    virtual void closed() { hostmod->removeThread(this); }

    /*
     * Called when the connection breaks (TCP error). By default it deletes
     * this thread, but it can be redefined to do something different.
     */
    virtual void failure(int code) { hostmod->removeThread(this); }

    /*
     * Called when a status arrives in response to getSocket()->getStatus().
     * By default it deletes the status object, redefine it to add code
     * to examine the status.
     */
    virtual void statusArrived(TcpStatusInfo *status) { delete status; }
};

} // namespace inet

#endif // ifndef __INET_TCPSRVHOSTAPP_H
