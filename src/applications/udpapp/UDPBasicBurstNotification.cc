//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2007 Universidad de Malaga
// Copyright (C) 2011 Zoltan Bojthe
// Copyright (C) 2011 Alfonso Ariza, Universidad de Malaga
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


#include "UDPBasicBurstNotification.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"


Define_Module(UDPBasicBurstNotification);

UDPBasicBurstNotification::UDPBasicBurstNotification()
{
    addressModule = NULL;
}

UDPBasicBurstNotification::~UDPBasicBurstNotification()
{
    if (addressModule)
        delete addressModule;
}

void UDPBasicBurstNotification::initialize(int stage)
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

    delayLimit = par("delayLimit");
    simtime_t startTime = par("startTime");
    if (startTime < simTime())
        startTime = simTime();
    stopTime = par("stopTime");

    messageLengthPar = &par("messageLength");
    burstDurationPar = &par("burstDuration");
    sleepDurationPar = &par("sleepDuration");
    sendIntervalPar = &par("sendInterval");
    nextSleep = startTime;
    nextBurst = startTime;
    nextPkt = startTime;


    destAddrRNG = par("destAddrRNG");
    const char *addrModeStr = par("chooseDestAddrMode").stringValue();
    int addrMode = cEnum::get("ChooseDestAddrMode")->lookup(addrModeStr);
    if (addrMode == -1)
        throw cRuntimeError(this, "Invalid chooseDestAddrMode: '%s'", addrModeStr);
    chooseDestAddrMode = (ChooseDestAddrMode)addrMode;

    WATCH(numSent);
    WATCH(numReceived);
    WATCH(numDeleted);
    WATCH(numDuplicated);

    localPort = par("localPort");
    destPort = par("destPort");

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    if (par("setBroadcast").boolValue())
        socket.setBroadcast(true);

    if (strcmp(par("outputInterface").stringValue(),"") != 0)
    {
        IInterfaceTable* ift = InterfaceTableAccess().get();
        InterfaceEntry *ie = ift->getInterfaceByName(par("outputInterface").stringValue());
        if (ie == NULL)
            throw cRuntimeError(this, "Invalid output interface name : %s",par("outputInterface").stringValue());
        outputInterface = ie->getInterfaceId();
    }

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


    addressModule = new AddressModule();
    //addressModule->initModule(par("chooseNewIfDeleted").boolValue());
    addressModule->initModule(true);


    std::string destAddresses = par("destAddresses").stdstringValue();
    if (strcmp(destAddresses.c_str(),"") != 0)
        isSource = true;

    if (isSource)
    {
        if (chooseDestAddrMode == ONCE)
            destAddr = chooseDestAddr();

        activeBurst = true;

        timerNext = new cMessage("UDPBasicBurstTimer");
        scheduleAt(startTime, timerNext);
    }

    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");
    outOfOrderPkSignal = registerSignal("outOfOrderPk");
    dropPkSignal = registerSignal("dropPk");

    NotificationBoard *nb = NotificationBoardAccess().get();
    nb->subscribe(this,NF_INTERFACE_IPv4CONFIG_CHANGED);
    nb->subscribe(this,NF_INTERFACE_IPv6CONFIG_CHANGED);
}

IPvXAddress UDPBasicBurstNotification::chooseDestAddr()
{
    if (addressModule->isInit())
        return addressModule->choseNewAddress();
    else
    {
        addressModule->initModule(true);
        if (addressModule->isInit())
            return addressModule->choseNewAddress();
        return IPv4Address::UNSPECIFIED_ADDRESS;
    }
}


void UDPBasicBurstNotification::generateBurst()
{
    simtime_t now = simTime();

    if (nextPkt < now)
        nextPkt = now;

    double sendInterval = sendIntervalPar->doubleValue();
    if (sendInterval <= 0.0)
        throw cRuntimeError(this, "The sendInterval parameter must be bigger than 0");
    nextPkt += sendInterval;

    if (activeBurst && nextBurst <= now) // new burst
    {
        double burstDuration = burstDurationPar->doubleValue();
        if (burstDuration < 0.0)
            throw cRuntimeError(this, "The burstDuration parameter mustn't be smaller than 0");
        double sleepDuration = sleepDurationPar->doubleValue();

        if (burstDuration == 0.0)
            activeBurst = false;
        else
        {
            if (sleepDuration < 0.0)
                throw cRuntimeError(this, "The sleepDuration parameter mustn't be smaller than 0");
            nextSleep = now + burstDuration;
            nextBurst = nextSleep + sleepDuration;
        }

        if (chooseDestAddrMode == PER_BURST)
            chooseDestAddr();
    }

    if (chooseDestAddrMode == PER_SEND)
        chooseDestAddr();

    destAddr = addressModule->getAddress();
    if (!destAddr.isUnspecified())
    {
        cPacket *payload = createPacket();
        payload->setTimestamp();
        emit(sentPkSignal, payload);
        // Check address type
        // Check address type
        if (!sendBroadcast(destAddr, payload))
            socket.sendTo(payload, destAddr, destPort,outputInterface);
        numSent++;
    }
    // Next timer
    if (activeBurst && nextPkt >= nextSleep)
        nextPkt = nextBurst;

    scheduleAt(nextPkt, timerNext);
}

void UDPBasicBurstNotification::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
    if (category == NF_INTERFACE_IPv4CONFIG_CHANGED || category == NF_INTERFACE_IPv6CONFIG_CHANGED)
        addressModule->rebuildAddressList();

}

