#include "CommonPartSublayerDown.h"
#include "PhyControlInfo_m.h"

Define_Module(CommonPartSublayerDown);

CommonPartSublayerDown::CommonPartSublayerDown()
{
    endTransmissionEvent = NULL;
}

CommonPartSublayerDown::~CommonPartSublayerDown()
{
    cancelAndDelete(endTransmissionEvent);
}

void CommonPartSublayerDown::initialize()
{
    frame_CID = 0x0000;

    queue.setName("queue");
    endTransmissionEvent = new cMessage("endTxEvent");

    gateToWatch = gate("lowergateOut");
}

void CommonPartSublayerDown::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("controlplaneIn"))
    {
        handleControlPlaneMsg(msg);
    }
    else if (msg->arrivedOn("uppergateIn"))
    {
        //handleUpperMsg(msg);
    }
    else
    {
        ev << "nothing" << endl;
    }
}

void CommonPartSublayerDown::handleCommand(int msgkind, cPolymorphic *ctrl)
{
}

void CommonPartSublayerDown::handleControlPlaneMsg(cMessage *msg)
{
    send(msg, "lowergateOut");
}

void CommonPartSublayerDown::handleUpperMsg(cMessage *msg)
{
    send(msg, "lowergateOut");
}
