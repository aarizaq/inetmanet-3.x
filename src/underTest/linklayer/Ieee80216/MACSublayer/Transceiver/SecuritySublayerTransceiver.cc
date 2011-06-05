
#include "SecuritySublayerTransceiver.h"
#include "PhyControlInfo_m.h"

Define_Module(SecuritySublayerTransceiver);

SecuritySublayerTransceiver::SecuritySublayerTransceiver()
{
}

SecuritySublayerTransceiver::~SecuritySublayerTransceiver()
{
}

void SecuritySublayerTransceiver::initialize()
{
    commonPartGateIn = findGate("commonPartGateIn");
    commonPartGateOut = findGate("commonPartGateOut");
    transceiverRadioGateIn = findGate("transceiverRadioGateIn");
    transceiverRadioGateOut = findGate("transceiverRadioGateOut");
}

void SecuritySublayerTransceiver::handleMessage(cMessage *msg)
{
    EV << "(in handleMessage) message " << msg->getName() <<
        " eingetroffen an SecuritySublayerTransceiver.\n";
    if (msg->getArrivalGateId() == commonPartGateIn)
    {
        EV << "von commonPartGateIn: " << msg << ". An transceiverRadioGateOut gesendet.\n";
        send(msg, transceiverRadioGateOut);
    }
    else if (msg->getArrivalGateId() == transceiverRadioGateIn)
    {
        EV << "von transceiverRadioGateIn: " << msg << ". An commonPartGateOut gesendet.\n";
        send(msg, commonPartGateOut);
    }
    else
    {
        EV << "nothing to do in function SecuritySublayerTransceiver::handleMessage" << endl;
    }
}
