//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "MACAddress.h"
#include "PhyControlInfo_m.h"
#include "Ieee80216ManagementMessages_m.h"
#include "Ieee80216Primitives_m.h"
#include "WiMAXRadioControl_m.h"

class ControlPlaneAP
{
  public:
    struct BaseStationInfo
    {
        MACAddress BasestationID; //Basestation ID
        simtime_t DLMAP_interval; // Intervall der DL-MAP Nachricht
        int DownlinkChannel;
        int UplinkChannel;
    };

    virtual void sendLowerMessage(cMessage *msg) = 0;
    virtual void sendLowerDelayMessage(double delayTime, cMessage *msg) = 0;
    virtual void sendRadioUpOut(cMessage *msg) = 0;

    void makeDL_MAP(BaseStationInfo BSInfo);
    void makeDCD(BaseStationInfo BSInfo);
    void makeUCD(BaseStationInfo BSInfo);
    void makeRNG_RSP(MACAddress recieveMobileStation);
    void makeSBC_RSP();
    void makeREG_RSP();

    void setRadio(BaseStationInfo BSInfo);
    void setRadioDownlink(BaseStationInfo BSInfo);
    void setRadioUplink(BaseStationInfo BSInfo);
};
