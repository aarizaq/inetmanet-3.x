#include "CommonPartSublayerControlModule.h"
#include "PhyControlInfo_m.h"
#include <stdio.h>

#include <omnetpp.h>

Define_Module(CommonPartSublayerControlModule);

CommonPartSublayerControlModule::CommonPartSublayerControlModule()
{
}

CommonPartSublayerControlModule::~CommonPartSublayerControlModule()
{
}

void CommonPartSublayerControlModule::initialize()
{
    authIn = findGate("authIn");
    authOut = findGate("authOut");
    controlIn = findGate("controlIn");
    controlOut = findGate("controlOut");
    serviceflowIn = findGate("serviceflowIn");
    serviceflowOut = findGate("serviceflowOut");
}

void CommonPartSublayerControlModule::handleMessage(cMessage *msg)
{
}
