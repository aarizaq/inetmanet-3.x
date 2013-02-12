
#include "ConvergenceSublayerReceiver.h"
#include "PhyControlInfo_m.h"

Define_Module(ConvergenceSublayerReceiver);

ConvergenceSublayerReceiver::ConvergenceSublayerReceiver()
{
    endTransmissionEvent = NULL;
}

ConvergenceSublayerReceiver::~ConvergenceSublayerReceiver()
{
    cancelAndDelete(endTransmissionEvent);
}

void ConvergenceSublayerReceiver::initialize()
{
    commonPartGateIn = findGate("commonPartGateIn");
    higherLayerGateOut = findGate("higherLayerGateOut");
}

void ConvergenceSublayerReceiver::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == commonPartGateIn)
    {
        EV << "\n\nMessage arrived on ConvergenceSublayerUp! ==> " << msg;

        EV << "\nSuddenly, a black hole forwarded the message to another universe.";
        send(msg, higherLayerGateOut);
    }
    else
    {
        EV << "nothing" << endl;
    }
}
