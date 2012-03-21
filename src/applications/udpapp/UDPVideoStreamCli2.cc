//
// Copyright (C) 2005 Andras Varga
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


//
// based on the video streaming app of the similar name by Johnny Lai
//

#include "UDPVideoStreamCli2.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"


Define_Module(UDPVideoStreamCli2);

simsignal_t UDPVideoStreamCli2::rcvdPkSignal = SIMSIGNAL_NULL;

UDPVideoStreamCli2::UDPVideoStreamCli2()
{
    reintentTimer = NULL;
    timeOutMsg = NULL;
    socketOpened = false;
}

UDPVideoStreamCli2::~UDPVideoStreamCli2()
{
    cancelAndDelete(reintentTimer);
}

void UDPVideoStreamCli2::initialize()
{
    // statistics
    rcvdPkSignal = registerSignal("rcvdPk");

    simtime_t startTime = par("startTime");
    reintentTimer = new cMessage();
    timeOutMsg = new cMessage();

    timeOut  = par("timeOut");

    if (startTime >= 0)
        scheduleAt(startTime, new cMessage("UDPVideoStreamStart"));
}

void UDPVideoStreamCli2::finish()
{
    recordScalar("Total received", numRecPackets);
}

void UDPVideoStreamCli2::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        if (reintentTimer == msg)
            requestStream();
        else if (timeOutMsg == msg)
        {
            timeOutData();
        }
        else
        {
            delete msg;
            requestStream();
        }
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        // process incoming packet
        receiveStream(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void UDPVideoStreamCli2::requestStream()
{
    int svrPort = par("serverPort");
    int localPort = par("localPort");
    const char *address = par("serverAddress");
    IPvXAddress svrAddr = IPvXAddressResolver().resolve(address);

    if (svrAddr.isUnspecified())
    {
        EV << "Server address is unspecified, skip sending video stream request\n";
        return;
    }

    EV << "Requesting video stream from " << svrAddr << ":" << svrPort << "\n";

    if (!socketOpened)
    {
        socket.setOutputGate(gate("udpOut"));
        socket.bind(localPort);
        socketOpened = true;
    }

    cPacket *pk = new cPacket("VideoStrmReq");
    socket.sendTo(pk, svrAddr, svrPort);
    scheduleAt(simTime()+par("reintent").longValue(),reintentTimer);
}

void UDPVideoStreamCli2::receiveStream(cPacket *pk)
{
    EV << "Video stream packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);
    delete pk;
    if (reintentTimer->isScheduled())
        cancelEvent(reintentTimer);
    if (timeOutMsg->isScheduled())
        cancelEvent(timeOutMsg);
    numRecPackets++;
    scheduleAt(simTime()+timeOut,timeOutMsg);
}

void UDPVideoStreamCli2::timeOutData()
{
    scheduleAt(simTime()+par("reintent").longValue(),reintentTimer);
}

