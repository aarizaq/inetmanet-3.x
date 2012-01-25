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
 * @file TCPExampleApp.cc
 * @author Antonio Zea, Bernhard Mueller, Ingmar Baumgart
 */

#include <string>

#include "UnderlayConfigurator.h"
#include "GlobalStatistics.h"
#include "GlobalNodeList.h"
#include "IPvXAddressResolver.h"

using namespace std;

#include "TCPExampleApp.h"

Define_Module(TCPExampleApp);

TCPExampleApp::TCPExampleApp()
{
    timerMsg = NULL;
}

TCPExampleApp::~TCPExampleApp()
{
    cancelAndDelete(timerMsg);
}

void TCPExampleApp::initializeApp(int stage)
{
    if (stage != MIN_STAGE_APP) {
        return;
    }

    // copy the module parameter values to our own variables
    sendPeriod = par("sendPeriod");

    // initialize our statistics variables
    numSent = 0;
    numReceived = 0;

    // tell the GUI to display our variables
    WATCH(numSent);
    WATCH(numReceived);

    // start our timer
    timerMsg = new cMessage("Periodic timer");

    // set up and listen on tcp
    bindAndListenTcp(24000);

    // first node which was created starts with PING-PONG messaging
    if (globalNodeList->getNumNodes() == 1) {
        scheduleAt(simTime() + SimTime::parse("20s"), timerMsg);
    }
}


void TCPExampleApp::finishApp()
{
    globalStatistics->addStdDev("TCPExampleApp: Sent packets", numSent);
    globalStatistics->addStdDev("TCPExampleApp: Received packets", numReceived);
}


void TCPExampleApp::handleTimerEvent(cMessage* msg)
{
    if (msg == timerMsg) {
        // if the simulator is still busy creating the network,
        // let's wait a bit longer
        if (underlayConfigurator->isInInitPhase()) {
            scheduleAt(simTime() + sendPeriod, timerMsg);
            return;
        }

        // do the same, if the node is the only one alive
        if (globalNodeList->getNumNodes() == 1){
            scheduleAt(simTime() + sendPeriod, timerMsg);
            return;
        }

        // get the address of one of the other nodes
        TransportAddress* addr = globalNodeList->getRandomAliveNode();
        while (thisNode.getIp().equals(addr->getIp())) {
            addr = globalNodeList->getRandomAliveNode();
        }

        // create a PING message
        TCPExampleMessage *TCPMsg = new TCPExampleMessage();
        TCPMsg->setType(TCPEXMSG_PING); // set the message type to PING
        TCPMsg->setSenderAddress(thisNode); // set the sender address to our own
        TCPMsg->setByteLength(100); // set the message length to 100 bytes

        RECORD_STATS(numSent++); // update statistics

        // connect and send message
        TransportAddress remoteAddress = TransportAddress(addr->getIp(), 24000);
        establishTcpConnection(remoteAddress);
        sendTcpData(TCPMsg, remoteAddress);

        // user output
        EV << thisNode.getIp() << ": Connecting to "<< addr->getIp()
           << " and sending PING."<< std::endl;
    }
}

void TCPExampleApp::handleDataReceived(TransportAddress address, cPacket* msg, bool urgent)
{
        // *redefine* to perform or schedule next sending
        TCPExampleMessage *TCPMsg = dynamic_cast<TCPExampleMessage*>(msg);

        RECORD_STATS(numReceived++);

        if (TCPMsg && TCPMsg->getType() == TCPEXMSG_PING){
            // create PONG message
            TCPExampleMessage *respMsg = new TCPExampleMessage();
            respMsg->setType(TCPEXMSG_PONG); // set the message type to PONG
            respMsg->setSenderAddress(thisNode); // set the sender address to our own
            respMsg->setByteLength(100);

            sendTcpData(respMsg, address);

            RECORD_STATS(numSent++);

            // user output
            EV << thisNode.getIp() << ": Got PING from "
               << TCPMsg->getSenderAddress().getIp() << ", sending PONG!"
               << std::endl;

        } else if (TCPMsg && TCPMsg->getType() == TCPEXMSG_PONG){
            // user output
            EV << thisNode.getIp() << ": Got PONG reply! Closing connection." << std::endl;

            // dialog successfully transmitted, close connection
            closeTcpConnection(address);
        }

        delete msg;
}

void TCPExampleApp::handleConnectionEvent(EvCode code, TransportAddress address)
{
    if (code == PEER_CLOSED) {
        scheduleAt(simTime() + sendPeriod, timerMsg);
    } else {
        BaseTcpSupport::handleConnectionEvent(code, address);
    }
}
