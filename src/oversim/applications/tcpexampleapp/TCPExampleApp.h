//
// Copyright (C) 2010 Karlsruhe Institute of Technology (KIT),
//                    Institute of Telematics
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
 * @file TCPExampleApp.h
 * @author Antonio Zea, Bernhard Mueller, Ingmar Baumgart
 */

#ifndef TCPEXAMPLEAPP_H
#define TCPEXAMPLEAPP_H

#include <omnetpp.h>
#include <BaseApp.h>

#include "TCPExampleMessage_m.h"

/**
 * A simple TCP example application
 *
 * This is a simple TCP example application. It periodically establishes TCP
 * connections to random nodes, sends PING and receives PONG messages.
 *
 * @author Antonio Zea
 * @author Bernhard Mueller
 * @author Ingmar Baumgart
 */
class TCPExampleApp : public BaseApp
{
public:
    TCPExampleApp();
    ~TCPExampleApp();

private:
    // application routines
    void initializeApp(int stage);
    void finishApp();
    void handleTimerEvent(cMessage* msg);
    void handleDataReceived(TransportAddress address, cPacket* msg, bool urgent);
    void handleConnectionEvent(EvCode code, TransportAddress address);

    // module parameters
    simtime_t sendPeriod;     // we'll store the "sendPeriod" parameter here

    // statistics
    int numSent;              //number of packets sent
    int numReceived;          //number of packets received

    // our timer
    cMessage *timerMsg;
};

#endif
