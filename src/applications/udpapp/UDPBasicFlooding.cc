//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2007 Universidad de Malaga
// Copyright (C) 2011 Zoltan Bojthe
// Copyright (C) 2013 Universidad de Malaga
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


#include "UDPBasicFlooding.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"

Define_Module(UDPBasicFlooding);

int UDPBasicFlooding::counter;

simsignal_t UDPBasicFlooding::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicFlooding::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicFlooding::outOfOrderPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicFlooding::dropPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicFlooding::floodPkSignal = SIMSIGNAL_NULL;

UDPBasicFlooding::UDPBasicFlooding()
{
    messageLengthPar = NULL;
    burstDurationPar = NULL;
    sleepDurationPar = NULL;
    sendIntervalPar = NULL;
    timerNext = NULL;
    addressModule = NULL;
    outputInterfaceMulticastBroadcast.clear();
}

UDPBasicFlooding::~UDPBasicFlooding()
{
    cancelAndDelete(timerNext);
}

void UDPBasicFlooding::initialize(int stage)
{
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;

    counter = 0;
    numSent = 0;
    numReceived = 0;
    numDeleted = 0;
    numDuplicated = 0;
    numFlood = 0;

    delayLimit = par("delayLimit");
    simtime_t startTime = par("startTime");
    stopTime = par("stopTime");

    messageLengthPar = &par("messageLength");
    burstDurationPar = &par("burstDuration");
    sleepDurationPar = &par("sleepDuration");
    sendIntervalPar = &par("sendInterval");
    nextSleep = startTime;
    nextBurst = startTime;
    nextPkt = startTime;


    WATCH(numSent);
    WATCH(numReceived);
    WATCH(numDeleted);
    WATCH(numDuplicated);
    WATCH(numFlood);


    localPort = par("localPort");
    destPort = par("destPort");

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    socket.setBroadcast(true);

    outputInterfaceMulticastBroadcast.clear();
    if (strcmp(par("outputInterfaceMulticastBroadcast").stringValue(),"") != 0)
    {
        IInterfaceTable* ift = InterfaceTableAccess().get();
        const char *ports = par("outputInterfaceMulticastBroadcast");
        cStringTokenizer tokenizer(ports);
        const char *token;
        while ((token = tokenizer.nextToken()) != NULL)
        {
            if (strstr(token, "ALL") != NULL)
            {
                for (int i = 0; i < ift->getNumInterfaces(); i++)
                {
                    InterfaceEntry *ie = ift->getInterface(i);
                    if (ie->isLoopback())
                        continue;
                    if (ie == NULL)
                        throw cRuntimeError(this, "Invalid output interface name : %s", token);
                    outputInterfaceMulticastBroadcast.push_back(ie->getInterfaceId());
                }
            }
            else
            {
                InterfaceEntry *ie = ift->getInterfaceByName(token);
                if (ie == NULL)
                    throw cRuntimeError(this, "Invalid output interface name : %s", token);
                outputInterfaceMulticastBroadcast.push_back(ie->getInterfaceId());
            }
        }
    }

    IPvXAddress myAddr = IPvXAddressResolver().resolve(this->getParentModule()->getFullPath().c_str());

    isSource = par("isSource");

    if (isSource)
    {
        activeBurst = true;
        timerNext = new cMessage("UDPBasicFloodingTimer");
        scheduleAt(startTime, timerNext);
    }

    if (strcmp(par("destAddresses").stringValue(),"") != 0)
    {
        addressModule = new AddressModule();
        //addressModule->initModule(par("chooseNewIfDeleted").boolValue());
        addressModule->initModule(true);

    }

    myId = this->getParentModule()->getId();


    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");
    outOfOrderPkSignal = registerSignal("outOfOrderPk");
    dropPkSignal = registerSignal("dropPk");
    floodPkSignal = registerSignal("floodPk");
}


cPacket *UDPBasicFlooding::createPacket()
{
    char msgName[32];
    sprintf(msgName, "UDPBasicAppData-%d", counter++);
    long msgByteLength = messageLengthPar->longValue();
    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(msgByteLength);
    payload->addPar("sourceId") = getId();
    payload->addPar("msgId") = numSent;
    if (addressModule)
        payload->addPar("destAddr") = addressModule->choseNewModule();

    return payload;
}

void UDPBasicFlooding::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (dynamic_cast<cPacket*>(msg))
        {
            IPvXAddress destAddr(IPv4Address::ALLONES_ADDRESS);
            sendBroadcast(destAddr, PK(msg));
        }
        else
        {
            if (stopTime <= 0 || simTime() < stopTime)
            {
                // send and reschedule next sending
                if (isSource) // if the node is a sink, don't generate messages
                    generateBurst();
            }
        }
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        // process incoming packet
        processPacket(PK(msg));
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

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void UDPBasicFlooding::processPacket(cPacket *pk)
{
    if (pk->getKind() == UDP_I_ERROR)
    {
        EV << "UDP error received\n";
        delete pk;
        return;
    }

    if (pk->hasPar("sourceId") && pk->hasPar("msgId"))
    {
        // duplicate control
        int moduleId = (int)pk->par("sourceId");
        int msgId = (int)pk->par("msgId");
        // check if this message has like origin this node
        if (moduleId == getId())
        {
            delete pk;
            return;
        }
        SourceSequence::iterator it = sourceSequence.find(moduleId);
        if (it != sourceSequence.end())
        {
            if (it->second >= msgId)
            {
                EV << "Out of order packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
                emit(outOfOrderPkSignal, pk);
                delete pk;
                numDuplicated++;
                return;
            }
            else
                it->second = msgId;
        }
        else
            sourceSequence[moduleId] = msgId;
    }

    if (delayLimit > 0)
    {
        if (simTime() - pk->getTimestamp() > delayLimit)
        {
            EV << "Old packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
            emit(dropPkSignal, pk);
            delete pk;
            numDeleted++;
            return;
        }
    }

    if (pk->hasPar("destAddr"))
    {
        int moduleId = (int)pk->par("destAddr");
        if (moduleId == myId)
        {
            EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
            emit(rcvdPkSignal, pk);
            numReceived++;
            delete pk;
            return;
        }
    }
    else
    {
        EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
        emit(rcvdPkSignal, pk);
        numReceived++;
    }

    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->removeControlInfo());
    if (ctrl->getDestAddr().get4() == IPv4Address::ALLONES_ADDRESS && par("flooding").boolValue())
    {
        numFlood++;
        emit(floodPkSignal, pk);
        scheduleAt(simTime()+par("delay").doubleValue(),pk);
    }
    else
        delete pk;
    delete ctrl;

}

void UDPBasicFlooding::generateBurst()
{
    simtime_t now = simTime();

    if (nextPkt < now)
        nextPkt = now;

    double sendInterval = sendIntervalPar->doubleValue();
    if (sendInterval <= 0.0)
        throw cRuntimeError("The sendInterval parameter must be bigger than 0");
    nextPkt += sendInterval;

    if (activeBurst && nextBurst <= now) // new burst
    {
        double burstDuration = burstDurationPar->doubleValue();
        if (burstDuration < 0.0)
            throw cRuntimeError("The burstDuration parameter mustn't be smaller than 0");
        double sleepDuration = sleepDurationPar->doubleValue();

        if (burstDuration == 0.0)
            activeBurst = false;
        else
        {
            if (sleepDuration < 0.0)
                throw cRuntimeError("The sleepDuration parameter mustn't be smaller than 0");
            nextSleep = now + burstDuration;
            nextBurst = nextSleep + sleepDuration;
        }
    }

    IPvXAddress destAddr(IPv4Address::ALLONES_ADDRESS);

    cPacket *payload = createPacket();
    payload->setTimestamp();
    emit(sentPkSignal, payload);

    // Check address type
    sendBroadcast(destAddr, payload);

    numSent++;

    // Next timer
    if (activeBurst && nextPkt >= nextSleep)
        nextPkt = nextBurst;

    scheduleAt(nextPkt, timerNext);
}

void UDPBasicFlooding::finish()
{
    recordScalar("Total sent", numSent);
    recordScalar("Total received", numReceived);
    recordScalar("Total deleted", numDeleted);
}

bool UDPBasicFlooding::sendBroadcast(const IPvXAddress &dest, cPacket *pkt)
{
    if (!outputInterfaceMulticastBroadcast.empty() && (dest.isMulticast() || (!dest.isIPv6() && dest.get4() == IPv4Address::ALLONES_ADDRESS)))
    {
        for (unsigned int i = 0; i < outputInterfaceMulticastBroadcast.size(); i++)
        {
            if (outputInterfaceMulticastBroadcast.size() - i > 1)
                socket.sendTo(pkt->dup(), dest, destPort, outputInterfaceMulticastBroadcast[i]);
            else
                socket.sendTo(pkt, dest, destPort, outputInterfaceMulticastBroadcast[i]);
        }
        return true;
    }
    return false;
}
