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
#include "UDPDmaMacSink.h"


namespace inet {

Define_Module(UDPDmaMacSink);

simsignal_t UDPDmaMacSink::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPDmaMacSink::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t UDPDmaMacSink::rcvdPkSignalDma = registerSignal("rcvdPkDma");


UDPDmaMacSink::~UDPDmaMacSink()
{
    cancelAndDelete(selfMsg);
}

void UDPDmaMacSink::initialize(int stage)
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
    else if (stage == INITSTAGE_APPLICATION_LAYER)
        parseXMLConfigFile();
}

void UDPDmaMacSink::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);

    recordScalar("packets received DmaMac", numReceivedDmaMac);
    recordScalar("packets received DmaMac Not dup", totalRec);

    ApplicationBase::finish();
}

void UDPDmaMacSink::setSocketOptions()
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

L3Address UDPDmaMacSink::chooseDestAddr()
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

void UDPDmaMacSink::processStart()
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
    for (unsigned int i = 0; i < actuators.size(); i++) {
        void *ptr = (void *) &(actuators[i]);
        actuators[i].timer = new cMessage("Actuator Time");
        actuators[i].timer->setContextPointer(ptr);

        DISTRUBUTIONTYPE dist = actuators[i].disType;
        double a = actuators[i].a;
        double b = actuators[i].b;
        double c = actuators[i].c;
        simtime_t start = actuators[i].startTime;
        switch(dist) {
        case uniformDist:
            scheduleAt(start + simTime() + uniform(a,b),actuators[i].timer);
            break;
        case exponentialDist:
            scheduleAt(start + simTime() + exponential(a),actuators[i].timer);
            break;
        case constantDist:
            scheduleAt(start + simTime() + a, actuators[i].timer);
            break;
        case pareto_shiftedDist:
            scheduleAt(start + simTime() + pareto_shifted(a,b,c),actuators[i].timer);
            break;
        case normalDist:
            scheduleAt(start + simTime() + normal(a,b), actuators[i].timer);
            break;
        }
    }

}

void UDPDmaMacSink::processStop()
{
    socket.close();
}


void UDPDmaMacSink::parseXMLConfigFile()
{
    // configure interfaces from XML config file
    cXMLElement *config = par("configuration");
    // relay configuration
    for (cXMLElement *child = config->getFirstChild(); child; child = child->getNextSibling()) {
        // Search for the relay entries.
        if (opp_strcmp(child->getTagName(), "relay") != 0)
            continue;
        const char * relayName = child->getAttribute("node");
        if (relayName == nullptr)
            throw cRuntimeError("Relay node attribute not found");
        L3Address addr = L3AddressResolver().resolve(relayName);
        if (addr.isUnspecified())
            throw cRuntimeError("Relay %s address not found",relayName);

        const char * networkId = child->getAttribute("networkId");
        if (networkId == nullptr)
            throw cRuntimeError("Relay networkId attribute not found");
        int32_t netId = atoi(networkId);

        auto it = relayId.find(netId);
        if (it != relayId.end())
            throw cRuntimeError("network Id %i already defined",netId);
        relayId.insert(std::make_pair(netId,addr));
    }

    for (cXMLElement *child = config->getFirstChild(); child; child = child->getNextSibling()) {
        // Search for the relay entries.
        if (opp_strcmp(child->getTagName(), "actuator") != 0)
            continue;

        ActuatorInfo actuatorInfo;
        const char * addrStr = child->getAttribute("address");
        if (addrStr == nullptr)
            throw cRuntimeError("Actuator address attribute not found");

        int32_t addr = atoi(addrStr);
        actuatorInfo.address = addr;

        const char * networkId = child->getAttribute("networkId");
        if (networkId == nullptr)
            throw cRuntimeError("Actuator networkId attribute not found");
        int32_t netId = atoi(networkId);
        actuatorInfo.networkId  = netId;

        auto it = relayId.find(netId);
        if (it == relayId.end())
            throw cRuntimeError("actuator network Id %i not found",netId);

        const char * distribution = child->getAttribute("distribution");
        if (distribution == nullptr)
            throw cRuntimeError("actuator distribution attribute not found");
        if (opp_strcmp(distribution,"uniform") == 0) {
            actuatorInfo.disType = uniformDist;
            const char * a = child->getAttribute("a");
            if (a == nullptr)
                throw cRuntimeError("uniform distribution parameter a not found");
            const char * b = child->getAttribute("b");
            if (b == nullptr)
                throw cRuntimeError("uniform distribution parameter b not found");
            actuatorInfo.a = atof(a);
            actuatorInfo.b = atof(b);

        }
        else if (opp_strcmp(distribution,"normal") == 0) {
            actuatorInfo.disType = normalDist;
            const char * a = child->getAttribute("a");
            if (a == nullptr)
                throw cRuntimeError("normal distribution parameter a not found");
            const char * b = child->getAttribute("b");
            if (b == nullptr)
                throw cRuntimeError("normal distribution parameter b not found");
            actuatorInfo.a = atof(a);
            actuatorInfo.b = atof(b);
        }
        else if (opp_strcmp(distribution,"exponential") == 0) {
            actuatorInfo.disType = exponentialDist;
            const char * a = child->getAttribute("a");
            if (a == nullptr)
                throw cRuntimeError("exponential distribution parameter a not found");
            actuatorInfo.a = atof(a);
        }
        else if (opp_strcmp(distribution,"constant") == 0) {
            actuatorInfo.disType = constantDist;
            const char * a = child->getAttribute("a");
            if (a == nullptr)
                throw cRuntimeError("constant distribution parameter a not found");
            actuatorInfo.a = atof(a);

        }
        else if (opp_strcmp(distribution,"pareto_shifted") == 0) {
            actuatorInfo.disType = pareto_shiftedDist;
            const char * a = child->getAttribute("a");
            if (a == nullptr)
                throw cRuntimeError("pareto_shifted distribution parameter a not found");
            const char * b = child->getAttribute("b");
            if (b == nullptr)
                throw cRuntimeError("pareto_shifted distribution parameter b not found");
            const char * c = child->getAttribute("c");
            if (c == nullptr)
                throw cRuntimeError("pareto_shifted distribution parameter c not found");
            actuatorInfo.a = atof(a);
            actuatorInfo.b = atof(b);
            actuatorInfo.c = atof(c);
        }
        else {
            throw cRuntimeError("actuator distribution type not found %s",distribution);
        }
        const char * start = child->getAttribute("start");
        if (start != nullptr)
            actuatorInfo.startTime = atof(child->getAttribute("start"));
        actuators.push_back(actuatorInfo);
    }
}

void UDPDmaMacSink::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == selfMsg) {
            switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;
            case STOP:
                processStop();
                break;
            default:
                throw cRuntimeError("Invalid kind %d in self message", (int) selfMsg->getKind());
            }
            return;
        }
        if (msg->getContextPointer()) {
            ActuatorInfo * actuatorInfo = static_cast<ActuatorInfo*> (msg->getContextPointer());
            ASSERT(actuatorInfo->timer == msg);
            double a = actuatorInfo->a;
            double b = actuatorInfo->b;
            double c = actuatorInfo->c;
            switch(actuatorInfo->disType) {
            case uniformDist:
                scheduleAt(simTime() + uniform(a, b),msg);
                break;
            case exponentialDist:
                scheduleAt(simTime() + exponential(a),msg);
                break;
            case constantDist:
                scheduleAt(simTime() + a,msg);
                break;
            case pareto_shiftedDist:
                scheduleAt(simTime() + pareto_shifted(a,b,c),msg);
                break;
            case normalDist:
                scheduleAt(simTime() + normal(a,b),msg);
                break;
            }
            DMAMACPkt* actuatorData = new DMAMACPkt();
            actuatorData->setDestAddr(MACAddress(actuatorInfo->address));
            actuatorData->setNetworkId(actuatorInfo->networkId);
            actuatorData->setBitLength(0);
            actuatorData->setKind(1); // DMAMAC_ACTUATOR_DATA
            auto it = relayId.find(actuatorInfo->networkId);
            if (it == relayId.end())
                throw cRuntimeError("Network id: %i relay not found",actuatorInfo->networkId);
            numSent++;
            socket.sendTo(actuatorData, it->second, destPort);
        }
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

void UDPDmaMacSink::refreshDisplay() const
{
    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void UDPDmaMacSink::processPacket(cPacket *pk)
{
    emit(rcvdPkSignal, pk);
    EV_INFO << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk)
                   << endl;
    DMAMACPkt *pkt = dynamic_cast<DMAMACPkt *>(pk);
    if (pkt) {
        numReceivedDmaMac++;
        NodeId nId;
        nId.addr = pkt->getSourceAddress();
        nId.networkId = pkt->getNetworkId();
        auto it = sequences.find(nId);
        if (it == sequences.end() || (it != sequences.end() && it->second.sequence < pkt->getSequence())) {
            if (it == sequences.end()) {
                DataNode dataNode;

                sequences.insert(std::make_pair(nId, dataNode));
                it = sequences.find(nId);
            }
            it->second.sequence = pkt->getSequence();
            it->second.totalNoDup++;
            totalRec++;
        }
        else {
            auto it = dupli.find(nId);
            if (it == dupli.end()) {

                dupli.insert(std::make_pair(nId, 0));
                it = dupli.find(nId);
            }
            it->second++;

        }
        it->second.totalRec++;
    }
    delete pk;
    numReceived++;
}

bool UDPDmaMacSink::handleNodeStart(IDoneCallback *doneCallback)
{
    return true;
}

bool UDPDmaMacSink::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (selfMsg)
        cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPDmaMacSink::handleNodeCrash()
{
    if (selfMsg)
        cancelEvent(selfMsg);
}

} // namespace inet

