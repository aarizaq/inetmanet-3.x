
#include "SecuritySublayerReceiver.h"
#include "PhyControlInfo_m.h"

Define_Module(SecuritySublayerReceiver);

SecuritySublayerReceiver::SecuritySublayerReceiver()
{
}

SecuritySublayerReceiver::~SecuritySublayerReceiver()
{
}

void SecuritySublayerReceiver::initialize()
{
    receiverRadioGateIn = findGate("receiverRadioGateIn");
    receiverRadioGateOut = findGate("receiverRadioGateOut");
    commonPartGateIn = findGate("commonPartGateIn");
    commonPartGateOut = findGate("commonPartGateOut");
}

void SecuritySublayerReceiver::handleMessage(cMessage *msg)
{
    EV << "(in handleMessage) message " << msg->
        getName() << " eingetroffen an SecuritySublayerReceiver.\n";

    if (msg->getArrivalGateId() == receiverRadioGateIn)
    {
        EV << "von receiverRadioGateIn: " << msg << ". An commonPartGateOut gesendet.\n";
        send(msg, commonPartGateOut);
    }
    else if (msg->getArrivalGateId() == commonPartGateIn)
    {
        EV << "von commonPartGateIn: " << msg << ". An receiverRadioGateOut gesendet.\n";
        send(msg, receiverRadioGateOut);
    }
    else
    {
        EV << "nothing to do in function SecuritySublayerReceiver::handleMessage" << endl;
    }
}
