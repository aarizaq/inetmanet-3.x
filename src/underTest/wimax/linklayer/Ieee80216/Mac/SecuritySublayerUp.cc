
#include "SecuritySublayerUp.h"
#include "PhyControlInfo_m.h"

Define_Module(SecuritySublayerUp);

SecuritySublayerUp::SecuritySublayerUp()
{
    endTransmissionEvent = NULL;
}

SecuritySublayerUp::~SecuritySublayerUp()
{
    cancelAndDelete(endTransmissionEvent);
}

void SecuritySublayerUp::initialize()
{
}

void SecuritySublayerUp::handleMessage(cMessage *msg)
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
