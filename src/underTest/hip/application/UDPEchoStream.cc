//*********************************************************************************
// File:           UDPEchoStream.cc
//
// Authors:        Laszlo Tamas Zeke, Levente Mihalyi, Laszlo Bokor
//
// Copyright: (C) 2008-2009 BME-HT (Department of Telecommunications,
// Budapest University of Technology and Economics), Budapest, Hungary
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
//**********************************************************************************
// Part of: HIPSim++ Host Identity Protocol Simulation Framework developed by BME-HT
//**********************************************************************************




#include "UDPEchoStream.h"
#include "UDPControlInfo_m.h"
#include "UDPEchoAppMsg_m.h"
#include "IPvXAddressResolver.h"


Define_Module(UDPEchoStream);

UDPEchoStream::UDPEchoStream()
{
}

UDPEchoStream::~UDPEchoStream()
{
}

// Initializing parameters
void UDPEchoStream::initialize()
{
    waitInterval = &par("waitInterval");
    packetLen = &par("packetLength");
    port = par("port");
	double startTime = par("startTime");
	destPort = par("destPort");
    destAddr.set(par("destAddress"));

	rttVector.setName("streamRTT");
	jitterVector.setName("streamJitter");
	rttStat.setName("rttStat");
	jitterStat.setName("jitterStat");

	first = true;
	numPkRec = 0;
    numPkSent = 0;
	lastrtt = 0;

	if (startTime>=0)
        scheduleAt(startTime, new cMessage("UDPEchoStreamStart"));
	socket.setOutputGate(gate("udpOut"));
	socket.bind(port);
}

// Collects statistics
void UDPEchoStream::finish()
{
    //recordScalar("streams served", numStreams);
    recordScalar("packets sent", numPkSent);
	recordScalar("packets recieved", numPkRec);
	recordScalar("packets lost", numPkSent - numPkRec);
	//recordScalar("rtt mean", rttStat.mean());
	//recordScalar("jitter mean", jitterStat.mean());
}

// Handles incoming messages
void UDPEchoStream::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
		if(first) {
			sendRequest();
			delete msg;
		}
		else
        // timer for stream expired, send packet
			sendStreamData(msg);
    }
    else
    {
		if(first) {
			first = false;
			double interval = (*waitInterval);
			scheduleAt(simTime()+interval, new cMessage("Stream_timer"));
			delete msg;
		}
		else
			processStreamData(msg);
    }
}
// Sends a packet to partner
void UDPEchoStream::sendRequest() {

    EV << "Requesting stream from " << destAddr << ":" << destPort << "\n";


    UDPEchoAppMsg *msg = new UDPEchoAppMsg("StrmReq");
	msg->setIsRequest(true);
    socket.sendTo(msg, destAddr, destPort);
}

// Get the time statistics from incoming packet
void UDPEchoStream::processStreamData(cMessage *msg)
{
	EV << "Stream packet:\n";
    //printPacket(msg);
	numPkRec++;
	simtime_t rtt = simTime() - msg->getCreationTime();
    rttVector.record(rtt * 1000.0);
	rttStat.collect(rtt * 1000.0);
	simtime_t jitter = (rtt - lastrtt) * 1000.0;
	jitterVector.record(jitter);
	if(jitter > 0.0)
		jitterStat.collect(jitter);
	else
		jitterStat.collect((lastrtt - rtt) * 1000.0);
	lastrtt = rtt;
    delete msg;

}

// Send a stream packet
void UDPEchoStream::sendStreamData(cMessage *timer)
{
	EV << "Sending stream to " << destAddr << ":" << destPort << "\n";
    // generate and send a packet
    UDPEchoAppMsg *pkt = new UDPEchoAppMsg("StrmPk");
	pkt->setIsRequest(true);
    int pktLen = (*packetLen);
    pkt->setByteLength(pktLen);
    socket.sendTo(pkt, destAddr, destPort);

    numPkSent++;

    // reschedule timer if there's bytes left to send

    double interval = (*waitInterval);
    scheduleAt(simTime()+interval, timer);

}
