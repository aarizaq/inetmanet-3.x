//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
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

#include "inet/applications/udpapp/UDPCdmaMac.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/linklayer/dmamac/packets/DMAMACPkt_m.h"        // MAC Data/ACK packet


namespace inet {

Define_Module(UDPCdmaMac);

simsignal_t UDPCdmaMac::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPCdmaMac::rcvdPkSignal = registerSignal("rcvdPk");


UDPCdmaMac::~UDPCdmaMac()
{
    cancelAndDelete(selfMsg);
}

void UDPCdmaMac::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        localPort = par("localPort");
        destPort = par("destPort");
        selfMsg = new cMessage();
        selfMsg->setKind(START);
        scheduleAt(0,selfMsg);
    }
}

void UDPCdmaMac::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UDPCdmaMac::setSocketOptions()
{
    int timeToLive = -1;
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int typeOfService = -1;
    if (typeOfService != -1)
        socket.setTypeOfService(typeOfService);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0]) {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *ie = ift->getInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups) {
        MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
        socket.joinLocalMulticastGroups(mgl);
    }
}

L3Address UDPCdmaMac::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if (destAddresses[k].isLinkLocal()) {    // KLUDGE for IPv6
        const char *destAddrs = par("destAddresses");
        cStringTokenizer tokenizer(destAddrs);
        const char *token = nullptr;

        for (int i = 0; i <= k; ++i)
            token = tokenizer.nextToken();
        destAddresses[k] = L3AddressResolver().resolve(token);
    }
    return destAddresses[k];
}

void UDPCdmaMac::processStart()
{
    socket.setOutputGate(gate("udpOut"));


    L3Address result;
    L3AddressResolver().tryResolve( par("sinkAddress"), result);
    if (result.isUnspecified())
        EV_ERROR << "cannot resolve destination address: " << par("sinkAddress").stringValue() << endl;
    else
        sinkAddress = result;

    setSocketOptions();
}

void UDPCdmaMac::processStop()
{
    socket.close();
}


void UDPCdmaMac::processDmaMac(cPacket *msg)
{
    // send to the sink
    if (msg->getControlInfo())
        delete msg->removeControlInfo();
    socket.sendTo(msg, sinkAddress, destPort);
    numSent++;

}

void UDPCdmaMac::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else if (dynamic_cast<DMAMACPkt *>(msg)) {
        processDmaMac(PK(msg));

    }
    else if (msg->getKind() == UDP_I_DATA) {
        // process incoming packet
        processPacket(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR) {
        EV_WARN << "Ignoring UDP error report\n";
        delete msg;
    }
    else {
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void UDPCdmaMac::refreshDisplay() const
{
    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void UDPCdmaMac::processPacket(cPacket *pk)
{
    emit(rcvdPkSignal, pk);
    EV_INFO << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    DMAMACPkt *pkt = dynamic_cast<DMAMACPkt *>(pk);
    if (pkt) {
        delete pkt->removeControlInfo();
        send(pkt, "dmaGateOut");
    }
    else
        delete pk;
    numReceived++;
}

bool UDPCdmaMac::handleNodeStart(IDoneCallback *doneCallback)
{
    return true;
}

bool UDPCdmaMac::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (selfMsg)
        cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPCdmaMac::handleNodeCrash()
{
    if (selfMsg)
        cancelEvent(selfMsg);
}

} // namespace inet

