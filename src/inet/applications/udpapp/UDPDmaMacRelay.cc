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

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/linklayer/dmamac/packets/DMAMACPkt_m.h"        // MAC Data/ACK packet
#include "UDPDmaMacRelay.h"


namespace inet {

Define_Module(UDPDmaMacRelay);

simsignal_t UDPDmaMacRelay::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPDmaMacRelay::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t UDPDmaMacRelay::rcvdPkSignalDma = registerSignal("rcvdPkDma");


UDPDmaMacRelay::~UDPDmaMacRelay()
{
    cancelAndDelete(selfMsg);
}

void UDPDmaMacRelay::initialize(int stage)
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

void UDPDmaMacRelay::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UDPDmaMacRelay::setSocketOptions()
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

L3Address UDPDmaMacRelay::chooseDestAddr()
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

void UDPDmaMacRelay::processStart()
{
    socket.setOutputGate(gate("udpOut"));

    socket.bind(localPort);
    L3Address result;
    L3AddressResolver().tryResolve( par("sinkAddress"), result);
    if (result.isUnspecified())
        EV_ERROR << "cannot resolve destination address: " << par("sinkAddress").stringValue() << endl;
    else
        sinkAddress = result;

    setSocketOptions();
}

void UDPDmaMacRelay::processStop()
{
    socket.close();
}


void UDPDmaMacRelay::processDmaMac(cPacket *msg)
{

    emit(rcvdPkSignalDma,msg);
    // send to the sink
    if (msg->getControlInfo())
        delete msg->removeControlInfo();
    socket.sendTo(msg, sinkAddress, destPort);
    numSent++;

}

void UDPDmaMacRelay::handleMessageWhenUp(cMessage *msg)
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
    else if (opp_strcmp("dmaGateIn",msg->getArrivalGate()->getBaseName()) == 0) {
        if (!dynamic_cast<DMAMACPkt *>(msg))
            throw cRuntimeError("Packet is not of the time DMAMACPkt");
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

void UDPDmaMacRelay::refreshDisplay() const
{
    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void UDPDmaMacRelay::processPacket(cPacket *pk)
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

bool UDPDmaMacRelay::handleNodeStart(IDoneCallback *doneCallback)
{
    return true;
}

bool UDPDmaMacRelay::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (selfMsg)
        cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPDmaMacRelay::handleNodeCrash()
{
    if (selfMsg)
        cancelEvent(selfMsg);
}

} // namespace inet

