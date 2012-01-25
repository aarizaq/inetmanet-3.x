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
 * @file RealworldConnector.h
 * @author Stephan Krause
 */

#ifndef _REALWORLDCONNECTOR_H_
#define _REALWORLDCONNECTOR_H_
#include "realtimescheduler.h"
#include "INETDefs.h"
#include "PacketParser.h"

/**
 * helper funcition needed for computing checksums
 *
 * \param buf The buffer containing the data to be checksummed
 * \param nbytes The length of the buffer in bytes
 * \return the Checksum
 */
inline u_short cksum(uint16_t *buf, int nbytes)
{
    register unsigned long  sum;
    u_short                 oddbyte;

    sum = 0;
    while (nbytes > 1) {
        sum += *buf++;
        nbytes -= 2;
    }

    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char *) &oddbyte) = *(u_char *) buf;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return ~sum;
}

/**
 * RealworldConnector is a pseudo interface that allows communcation with the real world
 * through the TunOutScheduler
 */
class INET_API RealworldConnector : public cSimpleModule
{
protected:

    int gateIndexNetwOut;
    unsigned int mtu;
#define BUFFERZITE mtu + 4 // 4 bytes for packet information

    // statistics
    long numSent;
    long numSendError;
    long numRcvdOK;
    long numRcvError;

    cMessage* packetNotification; // used by TunOutScheduler to notify about new packets
    RealtimeScheduler::PacketBuffer packetBuffer; // received packets are stored here
    RealtimeScheduler* scheduler;
    PacketParser* parser;

    /** Send a message to the (realworld) network
     *
     * \param msg A pointer to the message to be send
     */
    virtual void transmitToNetwork(cPacket *msg);
    virtual void updateDisplayString();

    /**
     * Converts an IP datagram to a data block for sending it to the (realworld) network
     *
     * \param msg A pointer to the message to be converted
     * \param length A pointer to an int that will hold the length of the converted data
     * \param addr If needed, the destination address
     * \param addrlen If needed, the length of the address
     * \return A pointer to the converted data
     */
    virtual char* encapsulate(cPacket *msg,
                              unsigned int* length,
                              sockaddr** addr,
                              socklen_t* addrlen) = 0;

    /**
     * Parses data received from the (realworld) network and converts it into a cMessage
     *
     * \param buf A pointer to the data to be parsed
     * \param length The lenght of the buffer in bytes
     * \param addr If needed, the destination address
     * \param addrlen If needed, the length of the address
     * \return The parsed message
     */
    virtual cPacket *decapsulate(char* buf,
                                  uint32_t length,
                                  sockaddr* addr,
                                  socklen_t addrlen) = 0;

    /**
     * If the Connector connects to an application, this method has to be overwritten to return "true"
     * \return false
     */
    virtual bool isApp() {return false;}

public:
    RealworldConnector();
    virtual ~RealworldConnector();

    virtual int numInitStages() const
    {
        return 4;
    }

    /** Initialization of the module.
     * Registers the device at the scheduler and searches for the appropriate payload-parser
     * Will be called automatically at startup
     */
    virtual void initialize(int stage);

    /**
     * The "main loop". Every message that is received or send is handled by this method
     */
    virtual void handleMessage(cMessage *msg);
};

#endif


