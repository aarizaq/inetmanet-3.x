//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


#include "TCPGenericCliAppBasePromis.h"

#include "GenericAppMsg_m.h"
#include "IPvXAddressResolver.h"
#include "NotifierConsts.h"
#include "NotificationBoard.h"
#include "Ieee80211Frame_m.h"
#include "IPv4Datagram.h"
#include "ARP.h"

simsignal_t TCPGenericCliAppBasePromis::connectSignal = SIMSIGNAL_NULL;
simsignal_t TCPGenericCliAppBasePromis::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t TCPGenericCliAppBasePromis::sentPkSignal = SIMSIGNAL_NULL;

void TCPGenericCliAppBasePromis::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage != 3)
        return;

    numSessions = numBroken = packetsSent = packetsRcvd = bytesSent = bytesRcvd = 0;

    //statistics
    connectSignal = registerSignal("connect");
    rcvdPkSignal = registerSignal("rcvdPk");
    sentPkSignal = registerSignal("sentPk");

    WATCH(numSessions);
    WATCH(numBroken);
    WATCH(packetsSent);
    WATCH(packetsRcvd);
    WATCH(bytesSent);
    WATCH(bytesRcvd);

    // parameters
    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    socket.readDataTransferModePar(*this);
    socket.bind(*localAddress ? IPvXAddressResolver().resolve(localAddress) : IPvXAddress(), localPort);

    socket.setCallbackObject(this);
    socket.setOutputGate(gate("tcpOut"));

    setStatusString("waiting");

    NotificationBoard * nb = NotificationBoardAccess().get();
    nb->subscribe(this,NF_LINK_FULL_PROMISCUOUS);
}

void TCPGenericCliAppBasePromis::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else
        socket.processMessage(msg);
}

void TCPGenericCliAppBasePromis::connect()
{
    // we need a new connId if this is not the first connection
    socket.renewSocket();

    // connect
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");

    EV << "issuing OPEN command\n";
    setStatusString("connecting");

    socket.connect(IPvXAddressResolver().resolve(connectAddress), connectPort);

    numSessions++;
    emit(connectSignal, 1L);
}

void TCPGenericCliAppBasePromis::close()
{
    setStatusString("closing");
    EV << "issuing CLOSE command\n";
    socket.close();
    emit(connectSignal, -1L);
}

void TCPGenericCliAppBasePromis::sendPacket(int numBytes, int expectedReplyBytes, bool serverClose)
{
    EV << "sending " << numBytes << " bytes, expecting " << expectedReplyBytes
       << (serverClose ? ", and server should close afterwards\n" : "\n");

    GenericAppMsg *msg = new GenericAppMsg("data");
    msg->setByteLength(numBytes);
    msg->setExpectedReplyLength(expectedReplyBytes);
    msg->setServerClose(serverClose);

    emit(sentPkSignal, msg);
    socket.send(msg);

    packetsSent++;
    bytesSent += numBytes;
}

void TCPGenericCliAppBasePromis::setStatusString(const char *s)
{
    if (ev.isGUI())
        getDisplayString().setTagArg("t", 0, s);
}

void TCPGenericCliAppBasePromis::socketEstablished(int, void *)
{
    // *redefine* to perform or schedule first sending
    EV << "connected\n";
    setStatusString("connected");
}

void TCPGenericCliAppBasePromis::socketDataArrived(int, void *, cPacket *msg, bool)
{
    // *redefine* to perform or schedule next sending
    packetsRcvd++;
    bytesRcvd += msg->getByteLength();
    emit(rcvdPkSignal, msg);
    delete msg;
}

void TCPGenericCliAppBasePromis::socketPeerClosed(int, void *)
{
    // close the connection (if not already closed)
    if (socket.getState() == TCPSocket::PEER_CLOSED)
    {
        EV << "remote TCP closed, closing here as well\n";
        close();
    }
}

void TCPGenericCliAppBasePromis::socketClosed(int, void *)
{
    // *redefine* to start another session etc.
    EV << "connection closed\n";
    setStatusString("closed");
}

void TCPGenericCliAppBasePromis::socketFailure(int, void *, int code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV << "connection broken\n";
    setStatusString("broken");

    numBroken++;
}

void TCPGenericCliAppBasePromis::finish()
{
    std::string modulePath = getFullPath();

    EV << modulePath << ": opened " << numSessions << " sessions\n";
    EV << modulePath << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";
    EV << modulePath << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
}

void TCPGenericCliAppBasePromis::receiveChangeNotification(int category, const cObject *details)
{
     if (category == NF_LINK_FULL_PROMISCUOUS)
     {

         Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(const_cast<cObject*>(details));
         if (frame != NULL)
         {
             //frame->getReceiverAddress();
             MACAddress senderMac = frame->getTransmitterAddress();
             ARP *arp = ArpAccess().get();
             IPv4Address sender = arp->getIPv4AddressFor(senderMac);
             IPv4Datagram *dgram = dynamic_cast<IPv4Datagram*> (frame->getEncapsulatedPacket());
             if (dgram)
             {
                 IPv4Address src = dgram->getSrcAddress();
                 IPv4Address dest = dgram->getDestAddress();
                 /***
                  * ............
                  *
                  */
             }
         }
     }
}
