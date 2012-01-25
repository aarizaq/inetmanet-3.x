//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file realtimescheduler.cc
 * @author Stephan Krause, Ingmar Baumgart
 */

#include "realtimescheduler.h"

Register_PerRunConfigOption(CFGID_EXTERNALAPP_CONNECTION_LIMIT, "externalapp-connection-limit", CFG_INT, NULL, "TODO some documentation");
Register_PerRunConfigOption(CFGID_EXTERNALAPP_APP_PORT, "externalapp-app-port", CFG_INT, NULL, "TODO some documentation");

inline std::ostream& operator<<(std::ostream& os, const timeval& tv)
{
    return os << (unsigned long)tv.tv_sec << "s" << tv.tv_usec << "us";
}

RealtimeScheduler::RealtimeScheduler() : cScheduler()
{
    FD_ZERO(&all_fds);
    maxfd = 0;
    netw_fd = INVALID_SOCKET;
    additional_fd = INVALID_SOCKET;
    apptun_fd = INVALID_SOCKET;
}

RealtimeScheduler::~RealtimeScheduler()
{ }

void RealtimeScheduler::startRun()
{
    if (initsocketlibonce()!=0)
        throw cRuntimeError("RealtimeScheduler: Cannot initialize socket library");

    gettimeofday(&baseTime, NULL);

    appModule = NULL;
    appNotificationMsg = NULL;
    module = NULL;
    notificationMsg = NULL;

    appConnectionLimit = ev.getConfig()->getAsInt(CFGID_EXTERNALAPP_CONNECTION_LIMIT, 0);

    if (initializeNetwork()) {
        opp_error("realtimeScheduler error: initializeNetwork failed\n");
    }
}

void RealtimeScheduler::endRun()
{}

void RealtimeScheduler::executionResumed()
{
    gettimeofday(&baseTime, NULL);
    baseTime = timeval_substract(baseTime, SIMTIME_DBL(simTime()));
}

void RealtimeScheduler::setInterfaceModule(cModule *mod, cMessage *notifMsg,
                                           PacketBuffer* buffer, int mtu,
                                           bool isApp)
{
    if (!mod || !notifMsg || !buffer) {
        throw cRuntimeError("RealtimeScheduler: setInterfaceModule(): "
                                "arguments must be non-NULL");
    }

    if (!isApp) {
        if (module) {
            throw cRuntimeError("RealtimeScheduler: setInterfaceModule() "
                                    "already called");
        }
        module = mod;
        notificationMsg = notifMsg;
        packetBuffer = buffer;
        buffersize = mtu;
    } else {
        if (appModule) {
            throw cRuntimeError("RealtimeScheduler: setInterfaceModule() "
                                    "already called");
        }
        appModule = mod;
        appNotificationMsg = notifMsg;
        appPacketBuffer = buffer;
        appBuffersize = mtu;
    }
}

bool RealtimeScheduler::receiveWithTimeout(long usec)
{
    bool newEvent = false;
    // prepare sets for select()
    fd_set readFD;
    readFD = all_fds;

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = usec;

    if (select(FD_SETSIZE, &readFD, NULL, NULL, &timeout) > 0) {
        // Read on all sockets with data
        for (SOCKET fd = 0; fd <= maxfd; fd++) {
            if (FD_ISSET(fd, &readFD)) {
                // Incoming data on netw_fd
                if (fd == netw_fd) {
                    char* buf = new char[buffersize];
                    int nBytes;

                    // FIXME: Ugly. But we want to support IPv4 and IPv6 here, so we
                    // reserve enough space for the "bigger" address.
                    sockaddr* from = (sockaddr*) new sockaddr_in;
                    socklen_t addrlen = sizeof(sockaddr_in);

                    // FIXME: Ugly...
                    getsockname(netw_fd, from, &addrlen);
                    if ( from->sa_family != SOCK_DGRAM ) {
                        delete from;
                        from = 0;
                        addrlen = 0;
                        // use read() for TUN device
                        nBytes = read(netw_fd, buf, buffersize);
                    } else {
                        addrlen = sizeof(sockaddr_in);
                        nBytes = recvfrom(netw_fd, buf, buffersize, 0, from, &addrlen);
                    }

                    if (nBytes < 0) {
                        ev << "[RealtimeScheduler::receiveWithTimeout()]\n"
                            << "    Error reading from network: " << strerror(sock_errno())
                            << endl;
                        delete[] buf;
                        buf = NULL;
                        opp_error("Read from network device returned an error");
                    } else if (nBytes == 0) {
                        ev << "[RealtimeScheduler::receiveWithTimeout()]\n"
                           << "    Received 0 byte long UDP packet!" << endl;
                        delete[] buf;
                        buf = NULL;
                    } else {
                        // write data to buffer
                        ev << "[RealtimeScheduler::receiveWithTimeout()]\n"
                            << "    Received " << nBytes << " bytes"
                            << endl;
                        packetBuffer->push_back(PacketBufferEntry(buf, nBytes, from, addrlen));
                        // schedule notificationMsg for the interface module
                        sendNotificationMsg(notificationMsg, module);
                        newEvent = true;
                    }
                } else if ( fd == apptun_fd ) {
                    // Data on application TUN FD
                    char* buf = new char[appBuffersize];
                    // use read() for TUN device
                    int nBytes = read(fd, buf, appBuffersize);

                    if (nBytes < 0) {
                        ev << "[RealtimeScheduler::receiveWithTimeout()]\n"
                            << "    Error reading from application TUN socket: "
                            << strerror(sock_errno())
                            << endl;
                        delete[] buf;
                        buf = NULL;
                        opp_error("Read from application TUN socket returned "
                                  "an error");
                    } else if (nBytes == 0) {
                        ev << "[RealtimeScheduler::receiveWithTimeout()]\n"
                           << "    Received 0 byte long UDP packet!" << endl;
                        delete[] buf;
                        buf = NULL;
                    } else {
                        // write data to buffer
                        ev << "[RealtimeScheduler::receiveWithTimeout()]\n"
                            << "    Received " << nBytes << " bytes"
                            << endl;

                        appPacketBuffer->push_back(PacketBufferEntry(buf,
                            nBytes, PacketBufferEntry::PACKET_APPTUN_DATA, fd));

                        // schedule notificationMsg for the interface module
                        sendNotificationMsg(appNotificationMsg, appModule);
                        newEvent = true;
                    }
                } else if ( fd == additional_fd ) {
                    // Data on additional FD
                    additionalFD();
                    newEvent = true;
                } else {
                    // Data on app FD
                    char* buf = new char[appBuffersize];
                    int nBytes = recv(fd, buf, appBuffersize, 0);
                    if (nBytes < 0) {
                        delete[] buf;
                        buf = NULL;
                        ev << "[RealtimeScheduler::receiveWithTimeout()]\n"
                            << "    Read error from application socket: "
                            << strerror(sock_errno()) << endl;
                        opp_error("Read from network device returned an error (App)");
                    } else if (nBytes == 0) {
                        // Application closed Socket
                        ev << "[RealtimeScheduler::receiveWithTimeout()]\n"
                            << "    Application closed socket"
                            << endl;
                        delete[] buf;
                        buf = NULL;
                        closeAppSocket(fd);
                        newEvent = true;
                    } else {
                        // write data to buffer
                        ev << "[RealtimeScheduler::receiveWithTimeout()]\n"
                            << "    Received " << nBytes << " bytes"
                            << endl;
                        appPacketBuffer->push_back(PacketBufferEntry(buf, nBytes, PacketBufferEntry::PACKET_DATA, fd));
                        // schedule notificationMsg for the interface module
                        sendNotificationMsg(appNotificationMsg, appModule);
                        newEvent = true;
                    }
                }
            }
        }
    }
    return newEvent;
}

int RealtimeScheduler::receiveUntil(const timeval& targetTime)
{
    // if there's more than 200ms to wait, wait in 100ms chunks
    // in order to keep UI responsiveness by invoking ev.idle()
    timeval curTime;
    gettimeofday(&curTime, NULL);
    while (targetTime.tv_sec-curTime.tv_sec >=2 ||
            timeval_diff_usec(targetTime, curTime) >= 200000) {
        if (receiveWithTimeout(100000)) { // 100ms
            if (ev.idle()) return -1;
            return 1;
        }
        if (ev.idle()) return -1;
        gettimeofday(&curTime, NULL);
    }

    // difference is now at most 100ms, do it at once
    long usec = timeval_diff_usec(targetTime, curTime);
    if (usec>0)
        if (receiveWithTimeout(usec)) {
            if (ev.idle()) return -1;
            return 1;
        }
    if (ev.idle()) return -1;
    return 0;
}

cMessage *RealtimeScheduler::getNextEvent()
{
    // assert that we've been configured
    if (!module)
        throw cRuntimeError("RealtimeScheduler: setInterfaceModule() not called: it must be called from a module's initialize() function");
    // FIXME: reimplement sanity check
//    if (app_fd >= 0 && !appModule)
//        throw cRuntimeError("RealtimeScheduler: setInterfaceModule() not called from application: it must be called from a module's initialize() function");

    // calculate target time
    timeval targetTime;
    cMessage *msg = sim->msgQueue.peekFirst();
    if (!msg) {
        // if there are no events, wait until something comes from outside
        // TBD: obey simtimelimit, cpu-time-limit
        targetTime.tv_sec = LONG_MAX;
        targetTime.tv_usec = 0;
    } else {
        // use time of next event
        simtime_t eventSimtime = msg->getArrivalTime();
        targetTime = timeval_add(baseTime, SIMTIME_DBL(eventSimtime));
    }

    // if needed, wait until that time arrives
    timeval curTime;
    gettimeofday(&curTime, NULL);
    if (timeval_greater(targetTime, curTime)) {
        int status = receiveUntil(targetTime);
        if (status == -1) {
            printf("WARNING: receiveUntil returned -1 (user interrupt)\n");
            return NULL; // interrupted by user
        } else if (status == 1) {
            msg = sim->msgQueue.peekFirst(); // received something
        }
    } else {
        //    printf("WARNING: Lagging behind realtime!\n");
        // we're behind -- customized versions of this class may
        // alert if we're too much behind, whatever that means
    }
    // ok, return the message
    return msg;
}

void RealtimeScheduler::closeAppSocket(SOCKET fd)
{
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
    FD_CLR(fd, &all_fds);

    appPacketBuffer->push_back(PacketBufferEntry(0, 0, PacketBufferEntry::PACKET_FD_CLOSE, fd));
    sendNotificationMsg(appNotificationMsg, appModule);
}

void RealtimeScheduler::sendNotificationMsg(cMessage* msg, cModule* mod)
{
    if (msg->isScheduled()) return; // Notification already scheduled
    timeval curTime;
    gettimeofday(&curTime, NULL);
    curTime = timeval_substract(curTime, baseTime);
    simtime_t t = curTime.tv_sec + curTime.tv_usec*1e-6;

    // if t < simTime, clock would go backwards. this would be bad...
    // (this could happen as timeval has a lower number of digits that simtime_t)
    if (t < simTime()) t = simTime();

    msg->setSentFrom(mod, -1, simTime());
    msg->setArrival(mod,-1,t);
    simulation.msgQueue.insert(msg);
}

ssize_t RealtimeScheduler::sendBytes(const char *buf,
                                     size_t numBytes,
                                     sockaddr* addr,
                                     socklen_t addrlen,
                                     bool isApp,
                                     SOCKET fd)
{
    if (!buf) {
        ev << "[RealtimeScheduler::sendBytes()]\n"
        << "    Error sending packet: buf = NULL"
        << endl;
        return -1;
    }
    if (!isApp) {
        if( numBytes > buffersize ) {
            ev << "[RealtimeScheduler::sendBytes()]\n"
            << "    Trying to send oversized packet: size " << numBytes << " mtu " << buffersize
            << endl;
            opp_error("Can't send packet: too large"); //FIXME: Throw exception instead
        }

        if ( netw_fd == INVALID_SOCKET ) {
            ev << "[RealtimeScheduler::sendBytes()]\n"
            << "    Can't send packet to network: no tun/udp socket"
            << endl;
            return 0;
        }
        int nBytes;
        if (addr) {
            nBytes =  sendto(netw_fd, buf, numBytes, 0, addr, addrlen);
        } else {
            // TUN
            nBytes =  write(netw_fd, buf, numBytes);
        }
        if (nBytes < 0) {
            ev << "[RealtimeScheduler::sendBytes()]\n"
            << "    Error sending data to network: " << strerror(sock_errno()) << "\n"
            << "    FD = " << netw_fd << ", numBytes = " << numBytes <<  ", addrlen = " << addrlen
            << endl;
        }
        return nBytes;

    } else {
        if (numBytes > appBuffersize) {
            ev << "[RealtimeScheduler::sendBytes()]\n"
            << "    Trying to send oversized packet: size " << numBytes << "\n"
            << "    mtu " << appBuffersize
            << endl;
            opp_error("Can't send packet: too large"); //FIXME: Throw exception instead
        }
        // If no fd is given, select a "random" one
        if (fd == INVALID_SOCKET) {
            for (fd = 0; fd <= maxfd; fd++) {
                if (fd == netw_fd) continue;
                if (fd == additional_fd) continue;
                if (FD_ISSET(fd, &all_fds)) break;
            }
            if (fd > maxfd) {
                throw cRuntimeError("Can't send packet to Application: no socket");
            }
        }
        if (fd == apptun_fd) {
            // Application TUN FD
            return write(fd, buf, numBytes);
        } else {
            return send(fd, buf, numBytes, 0);
        }
    }
    // TBD check for errors
}

