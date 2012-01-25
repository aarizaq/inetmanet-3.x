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
 * @file RealworldConnector.cc
 * @author Stephan Krause
 */

#include <string.h>
#include "RealworldConnector.h"

RealworldConnector::RealworldConnector()
{
    packetNotification = NULL;
}

RealworldConnector::~RealworldConnector()
{
    cancelAndDelete(packetNotification);
}

void RealworldConnector::initialize(int stage)
{
    if (stage==3) {
        // update display string when addresses have been autoconfigured etc.
        updateDisplayString();
        return;
    }

    // all initialization is done in the first stage
    if (stage!=0)
        return;

    packetNotification = new cMessage("packetNotification");
    mtu = par("mtu");

    scheduler = check_and_cast<RealtimeScheduler *>(simulation.getScheduler());
    scheduler->setInterfaceModule(this, packetNotification, &packetBuffer,
                                  mtu, isApp());

    if (!isApp() ) {
        parser = check_and_cast<PacketParser*>(
                      getParentModule()->getSubmodule("packetParser"));
    } else {
        parser = check_and_cast<PacketParser*>(
                      getParentModule()->getSubmodule("applicationParser"));
    }

    numSent = numRcvdOK = numRcvError = numSendError = 0;
    WATCH(numSent);
    WATCH(numRcvdOK);
    WATCH(numRcvError);
    WATCH(numSendError);

    if (!isApp()) {
        gateIndexNetwOut = gate("netwOut")->getId();
    } else {
        gateIndexNetwOut = gate("to_lowerTier")->getId();
    }

}


void RealworldConnector::handleMessage(cMessage *msg)
{
    // Packet from the real world...
    if (msg==packetNotification) {
        EV << "[RealworldConnector::handleMessage()]\n"
           << "    Message from outside. Queue length = "
           << packetBuffer.size() << endl;

        while( packetBuffer.size() > 0 ) {
            // get packet from buffer and parse it

            RealtimeScheduler::PacketBufferEntry packet = *(packetBuffer.begin());
            packetBuffer.pop_front();
            char* buf = packet.data;
            uint32_t len = packet.length;
            sockaddr* addr = packet.addr;
            socklen_t addrlen = packet.addrlen;
            cMessage *parsedPacket = decapsulate(buf, len, addr, addrlen);
            if (parsedPacket) {
                numRcvdOK++;
                send(parsedPacket, gateIndexNetwOut);
            } else {
                numRcvError++;
            }

        }
    } else // arrived on gate "netwIn"
    {
        // Packet from inside, send to real word
        EV << "[RealworldConnector::handleMessage()]\n"
           << "    Received " << msg << " for transmission"
           << endl;

        transmitToNetwork(check_and_cast<cPacket*>(msg));
    }

    if (ev.isGUI())
        updateDisplayString();

}

void RealworldConnector::transmitToNetwork(cPacket *msg)
{
    unsigned int length;
    sockaddr* addr = 0;
    socklen_t addrlen = 0;
    char* buf = encapsulate(msg, &length, &addr, &addrlen);
    if (buf) {
        numSent++;
        int nByte = scheduler->sendBytes(buf, length, addr, addrlen, isApp());

        if (nByte < 0) {
            EV << "[RealworldConnector::transmitToNetwork()]\n"
               << "    Error sending Packet, sendBytes returned " << nByte
               << endl;

            numSendError++;
        } else {
            EV << "[RealworldConnector::transmitToNetwork()]\n"
               << "    Packet (size = " << nByte << ") sent"
               << endl;
        }
    } else {
        numSendError++;
    }

    delete[] buf;
    delete addr;
}

void RealworldConnector::updateDisplayString()
{
    char buf[80];

    if (ev.isDisabled()) {
        // speed up things
        getDisplayString().setTagArg("t",0,"");
    }

    sprintf(buf, "rcv:%ld snt:%ld", numRcvdOK, numSent);

    if (numRcvError>0)
        sprintf(buf+strlen(buf), "\nerrin:%ld errout:%ld", numRcvError,
                numSendError);

    getDisplayString().setTagArg("t",0,buf);
}

