#include "ConvergenceSublayerHeaderCompression.h"
#include "PhyControlInfo_m.h"
#include <stdio.h>
#include <omnetpp.h>

Define_Module(ConvergenceSublayerHeaderCompression);

ConvergenceSublayerHeaderCompression::ConvergenceSublayerHeaderCompression()
{
}

ConvergenceSublayerHeaderCompression::~ConvergenceSublayerHeaderCompression()
{
}

void ConvergenceSublayerHeaderCompression::initialize()
{
    trafficClassificationGateIn = findGate("trafficClassificationGateIn");
    trafficClassificationGateOut = findGate("trafficClassificationGateOut");
    commonPartGateIn = findGate("commonPartGateIn");
    commonPartGateOut = findGate("commonPartGateOut");
}

void ConvergenceSublayerHeaderCompression::handleMessage(cMessage *msg)
{
    //packet arrives from classification
    if (msg->getArrivalGateId() == trafficClassificationGateIn)
    {
        send(msg, commonPartGateOut);
    }
    //packet arrives from commonPartSublayer
    else if (msg->getArrivalGateId() == commonPartGateIn)
    {
        send(msg, trafficClassificationGateOut);
    }
}
