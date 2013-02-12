//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//

#include "ControlPlaneAP.h"

/**
/ Die Methode makeDL_MAP erzeugt die DL-MAP Managment Nachricht.
/ Diese Nachricht enthält Informationen über die Basisstation
/
*/
void ControlPlaneAP::makeDL_MAP(BaseStationInfo BSInfo) //create DL_MAP frame
{
    Ieee80216DL_MAP *ManagementMessage = new Ieee80216DL_MAP("DL_MAP");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    ManagementMessage->setTYPE(Type);
    ManagementMessage->setBS_ID(BSInfo.BasestationID); //Überträgt die BSID der Basisstation in die Nachricht
    EV << "Sending DL-Map " << endl;
    sendLowerMessage(ManagementMessage);
}

/**
/ Die Methode makeDCD erzeugt die DCD(Downlink Channel Description) Managment Nachricht.
/ Diese Nachricht enthält Informationen über den Downlink Kanal
/
*/
void ControlPlaneAP::makeDCD(BaseStationInfo BSInfo)
{
    Ieee80216_DCD *frame = new Ieee80216_DCD("DCD");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    frame->setBS_ID(BSInfo.BasestationID); //copy Base Station ID into frame
    EV << "Sending DCD " << endl;
    sendLowerDelayMessage(0.5, frame); //Wird verzögert nach der DL-Map gesendet
}

/**
/ Die Methode makeUCD erzeugt die UCD(Uplink Channel Description) Management Nachricht.
/ Diese Nachricht enthaelt Informationen ueber den Uplink Kanal
/
*/
void ControlPlaneAP::makeUCD(BaseStationInfo BSInfo)
{
    Ieee80216_UCD *frame = new Ieee80216_UCD("UCD");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg definiert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    frame->setUploadChannel(BSInfo.UplinkChannel);
    EV << "Sending UCD " << endl;
    sendLowerDelayMessage(1, frame); //Wird verzögert nach der DCD Nachricht gesendet
}

/**
/ Die Methode makeRNG_RSP erzeugt die RNG-RSP Management Nachricht.
/ Mit dieser Nachricht anwortet die Basisstation auf die RNG-REQ Nachricht der Mobilstation
/
*/
void ControlPlaneAP::makeRNG_RSP(MACAddress recieveMobileStation)
{
    Ieee80216_RNG_RSP *frame = new Ieee80216_RNG_RSP("RNG_RSP");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    frame->setMSS_MAC_Address(recieveMobileStation);
    EV << "Sending RNG_RSP" << endl;
    sendLowerMessage(frame);
}

/**
/ Die Methode makeRNG_RSP erzeugt die RNG-RSP Managment Nachricht.
/ Mitdieser Nachricht anwortet die Basisstation auf die RNG-REQ Nachricht der Mobilstation
/
*/
void ControlPlaneAP::makeSBC_RSP()
{
    Ieee80216_SBC_RSP *frame = new Ieee80216_SBC_RSP("SBC_RSP");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    EV << "Sending SBC_RSP" << endl;
    sendLowerMessage(frame);
}

/**
/ Die Methode makeRNG_RSP erzeugt die RNG-RSP Managment Nachricht.
/ Mitdieser Nachricht anwortet die Basisstation auf die RNG-REQ Nachricht der Mobilstation
/
*/
void ControlPlaneAP::makeREG_RSP()
{
    Ieee80216_REG_RSP *frame = new Ieee80216_REG_RSP("REG_RSP");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    EV << "Sending REG_RSP" << endl;
    sendLowerMessage(frame);
}

void ControlPlaneAP::setRadio(BaseStationInfo BSInfo)
{
    setRadioDownlink(BSInfo);
    setRadioUplink(BSInfo);
}

void ControlPlaneAP::setRadioDownlink(BaseStationInfo BSInfo)
{

    EV << "Setze Downlink Kanal:" << BSInfo.DownlinkChannel << endl;
    PhyControlInfo *phyCtrl = new PhyControlInfo();
    phyCtrl->setChannelNumber(BSInfo.DownlinkChannel);
    cMessage *msg = new cMessage("changeChannel", PHY_C_CONFIGURERADIO); //VITA geaendert, davor war cMessage
    msg->setControlInfo(phyCtrl);
    sendLowerMessage(msg);

}

void ControlPlaneAP::setRadioUplink(BaseStationInfo BSInfo)
{
    EV << "Setze Uplink Kanal:" << BSInfo.UplinkChannel << endl;
    PhyControlInfo *phyCtrl = new PhyControlInfo();
    phyCtrl->setChannelNumber(BSInfo.UplinkChannel);
    cMessage *msg = new cMessage("changeChannel", PHY_C_CONFIGURERADIO); //VITA geaendert, davor war cMessage
    msg->setControlInfo(phyCtrl);
    sendRadioUpOut(msg);
}
