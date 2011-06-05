#include "CommonPartSublayerFragmentation.h"
#include "PhyControlInfo_m.h"
#include <stdio.h>
#include <omnetpp.h>

Define_Module(CommonPartSublayerFragmentation);

CommonPartSublayerFragmentation::CommonPartSublayerFragmentation()
{
}

CommonPartSublayerFragmentation::~CommonPartSublayerFragmentation()
{
}

/**
 * WICHTIG:
 * Nachrichten auf Basic, Broadcast und Initial Ranging Connections duerfen NIE gepackt oder fragmentiert werden!!
 * (siehe Standard --> Management-Messages
 *
 */

void CommonPartSublayerFragmentation::initialize()
{
    commonPartGateOut = findGate("commonPartGateOut");
    commonPartGateIn = findGate("commonPartGateIn");
    securityGateIn = findGate("securityGateIn");
    securityGateOut = findGate("securityGateOut");
}

void CommonPartSublayerFragmentation::handleMessage(cMessage *msg)
{
    EV << "(in handleMessage) message " << msg->getName() << " eingetroffen an fragmentation.\n";
    //higher layer message in transceiver
    if (msg->getArrivalGateId() == commonPartGateIn)
    {
        EV << "von commonPartGateIn " << msg << " an securityGateOut gesendet.\n";
        send(msg, securityGateOut);
    }
    //lower layer message in receiver
    else if (msg->getArrivalGateId() == securityGateIn)
    {
        EV << "von securityGateIn " << msg << " an commonPartGateOut gesendet.\n";
        send(msg, commonPartGateOut);
    }
    //higher layer message in receiver
    else if (msg->getArrivalGateId() == commonPartGateIn)
    {
        EV << "von commonPartGateIn " << msg << " an securityGateOut gesendet.\n";
        send(msg, securityGateOut);
    }
}
