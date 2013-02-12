
#include "ConvergenceSublayerUp.h"
#include "PhyControlInfo_m.h"

Define_Module(ConvergenceSublayerUp);

ConvergenceSublayerUp::ConvergenceSublayerUp()
{
    endTransmissionEvent = NULL;
}

ConvergenceSublayerUp::~ConvergenceSublayerUp()
{
    cancelAndDelete(endTransmissionEvent);
}

void ConvergenceSublayerUp::initialize()
{
}

void ConvergenceSublayerUp::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("lowerLayerIn"))
    {
        send(msg, "upperLayerOut");
    }
    else
    {
        ev << "nothing" << endl;
    }
}
