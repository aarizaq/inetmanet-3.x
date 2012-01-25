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
 * @file realtimescheduler.h
 * @author Stephan Krause, Ingmar Baumgart
 */

#ifndef __REALTIMESCHEDULER_H__
#define __REALTIMESCHEDULER_H__

#define WANT_WINSOCK2
#include <list>
#include <climits>
#include <platdep/sockets.h>
#include <omnetpp.h>

/** This class implements a event scheduler for OMNeT++
 *  It makes the simulation run in realtime (i.e. 1 simsec == 1 sec)
 *  It must be subclassed; its subclasses must handle network
 *  traffic from/to the simulation
 **/
class RealtimeScheduler : public cScheduler
{
public:
    class PacketBufferEntry {
	public:
	    char* data;
	    uint32_t length;
	    sockaddr* addr;
	    socklen_t addrlen;
            enum fdCommand {
                PACKET_DATA = 0,
                PACKET_FD_NEW = 1,
                PACKET_FD_CLOSE = 2,
                PACKET_APPTUN_DATA = 3
            } func;
            SOCKET fd;
	    PacketBufferEntry(char* buf, uint32_t len) :
		    data(buf), length(len), addr(0), addrlen(0), func(PACKET_DATA), fd(0) {};
	    PacketBufferEntry(char* buf, uint32_t len, sockaddr* ad, socklen_t al) :
		    data(buf), length(len), addr(ad), addrlen(al), func(PACKET_DATA), fd(0) {};
		PacketBufferEntry(char* buf, uint32_t len, fdCommand fc, int _fd) :
		    data(buf), length(len), addr(0), addrlen(0), func(fc), fd(_fd) {};
	    PacketBufferEntry(char* buf, uint32_t len, sockaddr* ad, socklen_t al,
	                      fdCommand fc, int _fd) :
		    data(buf), length(len), addr(ad), addrlen(al), func(fc), fd(_fd) {};
    };
    typedef std::list<PacketBufferEntry> PacketBuffer;

protected:
    // FD set with all file descriptors
    fd_set all_fds;
    SOCKET maxfd;

    // Buffer, module and FD for network communication
    SOCKET netw_fd;
    SOCKET apptun_fd;
    cModule *module;
    cMessage *notificationMsg;
    PacketBuffer *packetBuffer;
    size_t buffersize;

    // ... for realworldapp communication
    cModule *appModule;
    cMessage *appNotificationMsg;
    PacketBuffer *appPacketBuffer;
    size_t appBuffersize;
    int appConnectionLimit;

    SOCKET additional_fd; // Can be used by concrete implementations of this class

    // state
    timeval baseTime;

    /** Initialize the network
     **/
    virtual int initializeNetwork() = 0;

    /**
     * This function is called from main loop if data is accessible from "additional_fd".
     * This FD can be set in initializeNetwork by concrete implementations.
     **/
    virtual void additionalFD() {};

    /**
     * Waits for incoming data on the tun device
     *
     * \param usec Timeout after which to quit waiting (in µsec)
     * \return true if data was read, false otherwise
     **/
    virtual bool receiveWithTimeout(long usec);

    /**
     * Tries to read data until the given time is up
     *
     * \param targetTime stop waiting after this time is up
     * \return 1 if data is read, -1 if there is an error, 0 if no data is read
     **/
    virtual int receiveUntil(const timeval& targetTime);

public:
    /**
     * Constructor.
     */
    RealtimeScheduler();

    /**
     * Destructor.
     */
    virtual ~RealtimeScheduler();

    /**
     * Called at the beginning of a simulation run.
     */
    virtual void startRun();

    /**
     * Called at the end of a simulation run.
     */
    virtual void endRun();

    /**
     * Recalculates "base time" from current wall clock time.
     */
    virtual void executionResumed();

    /**
     * To be called from the module which wishes to receive data from the
     * tun device. The method must be called from the module's initialize()
     * function.
     *
     * \param module Pointer to the module that wants to receive the data
     * \param notificationMsg A pointer to a message that will be scheduled if there is data to read
     * \param buffer A pointer to the buffer the data will be written into
     * \param mtu Max allowed packet size
     * \param isApp set to "true" if called from a realworldApp
     */
    virtual void setInterfaceModule(cModule *module,
                                    cMessage *notificationMsg,
                                    PacketBuffer* buffer,
                                    int mtu,
                                    bool isApp = false);

    /**
     * Scheduler function -- it comes from cScheduler interface.
     */
    virtual cMessage *getNextEvent();

    /**
     * send notification msg to module
     *
     * \param msg The notification Message
     * \param mod The destination
     */
    void sendNotificationMsg(cMessage* msg, cModule* mod);

    /**
     * Send data to network
     *
     * \param buf A pointer to the data to be send
     * \param numBytes the length of the data
     * \param isApp set to "true" if called from a realworldApp
     * \param addr If needed, the destination address
     * \param addrlen The length of the address
     * \param fd If connected to more than one external app, set to the corresponding FD.
     *           If left to default and multiple apps are connected, the data will be send to
     *           one arbitrarily chosen app.
     * \return The number of bytes written, -1 on error
     */
    virtual ssize_t sendBytes(const char *buf,
                              size_t numBytes,
                              sockaddr* addr = 0,
                              socklen_t addrlen = 0,
                              bool isApp = false,
                              SOCKET fd = INVALID_SOCKET);

    /**
     * Close the application TCP socket
     */
    void closeAppSocket(SOCKET fd);

    /**
     * Returns the FD for the application TUN socket.
     *
     * @return the application TUN socket FD
     */
    virtual SOCKET getAppTunFd() { return apptun_fd; };

};

#endif

