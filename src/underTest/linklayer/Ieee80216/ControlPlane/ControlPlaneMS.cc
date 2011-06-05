
#include "ControlPlaneMS.h"

/**
* Erzeugt Ranging Restpons Nachricht
*
*
*************************************************/
void ControlPlaneMS::makeRNG_REQ(MobileSubStationInfo MSInfo)
{
    Ieee80216_RNG_REQ *frame = new Ieee80216_RNG_REQ("RNG_REQ");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    frame->setMSS_MAC_Address(MSInfo.MobileMacAddress); //copy Mac Adrress of MSS into frame
    //sendDelayed(frame,10,"MacOut");
    sendLowerMessage(frame);
}

/**
* Erzeugt Ranging Restpons Nachricht
*
*
*************************************************/
void ControlPlaneMS::makeSBC_REQ(MobileSubStationInfo MSInfo)
{
    Ieee80216_SBC_REQ *frame = new Ieee80216_SBC_REQ("SBC_REQ");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    EV << "Sending SBC_REQ " << endl;
    sendLowerMessage(frame);
}

/**
* Erzeugt Ranging Restpons Nachricht
*
*
*************************************************/
void ControlPlaneMS::makeREG_REQ(MobileSubStationInfo MSInfo)
{
    Ieee80216_REG_REQ *frame = new Ieee80216_REG_REQ("REG_REQ");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    EV << "Sending REG_REQ" << endl;
    sendLowerMessage(frame);
}
