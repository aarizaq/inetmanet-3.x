
#include "ConvergenceSublayerDown.h"
#include "PhyControlInfo_m.h"

Define_Module(ConvergenceSublayerDown);

ConvergenceSublayerDown::ConvergenceSublayerDown()
{
    endTransmissionEvent = NULL;
}

ConvergenceSublayerDown::~ConvergenceSublayerDown()
{
    cancelAndDelete(endTransmissionEvent);
}

void ConvergenceSublayerDown::initialize()
{
}

void ConvergenceSublayerDown::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("upperLayerIn"))
    {
        send(msg, "lowerLayerOut");
    }
    else
    {
        ev << "nothing" << endl;
    }
}
