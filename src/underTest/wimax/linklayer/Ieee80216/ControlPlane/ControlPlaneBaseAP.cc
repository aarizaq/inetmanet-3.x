//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//

#include "ControlPlaneBaseAP.h"

Define_Module(ControlPlaneBaseAP);

ControlPlaneBaseAP::ControlPlaneBaseAP()
{
    endTransmissionEvent = NULL;
}

ControlPlaneBaseAP::~ControlPlaneBaseAP()
{
    cancelAndDelete(endTransmissionEvent);
}

void ControlPlaneBaseAP::startTransmitting(cMessage *msg)
{
    ev << "Starting transmission of " << msg << endl;
}

void ControlPlaneBaseAP::initialize()
{
    /**
    *Basestation ID zuweisen
    *
    */
    const char *BasestationIDString = par("basestationid");
    if (!strcmp(BasestationIDString, "auto")) //wenn "auto" als Parameter wird eine Adresse generiert
    {
        BSInfo.BasestationID = MACAddress::generateAutoAddress();
        par("basestationid").setStringValue(BSInfo.BasestationID.str().c_str());
    }
    else
        BSInfo.BasestationID.setAddress(BasestationIDString); // Es wird der Parameter aus der INI-Datei übernommen
    /**
    * Übernahme von Paramtern für dieBasisstation
    *
    */
    BSInfo.DLMAP_interval = par("DLMapInterval"); // festlegen der DL-MAP Intervall
    BSInfo.DownlinkChannel = par("DownlinkChannel");
    BSInfo.UplinkChannel = par("UplinkChannel");

    /*****************************************************************************/
    /**
    * Inizilisieren von Hilfsvariablen
    *
    */
    BroadcastTimer = new cMessage("BroadcastTimer");
    StartsetRadio = new cMessage("StartsetRadio");

    /*****************************************************************************/

    clearMSSList();             //clear Mobile Subscriber Station list
    queue.setName("queue");
    endTransmissionEvent = new cMessage("endTxEvent");
    //gateToWatch = gate("MacOut");

    scheduleAt(1, StartsetRadio); //Setzt die Radiokanäle
    scheduleAt(2, BroadcastTimer); //Starte das aussenden der DL-MAP,DCD und UCD (isSelfMessage)

    ev << "Inizilisierung von ControlPlaneBS abgeschlossen.\n";
}

/**
* @name handleMessage
* Funktion zum abarbeiten ankommender Nachrichten
*/

void ControlPlaneBaseAP::handleMessage(cMessage *msg)
{
    // Nachrichten die von diesem Module an sich selber gesendet wurden
    if (msg->isSelfMessage())
    {
        ev << "Timer expired: " << msg << "\n";
        handleTimer(msg);
    }
    // Nachrichten die vom MAC Module gesendet wurden
    else if (msg->arrivedOn("cpsUpIn")) // arrived message on gate "MacIn"
    {
        ev << "Frame arrived from MAC: " << msg << "\n";
        Ieee80216GenericMacHeader *frame = check_and_cast<Ieee80216GenericMacHeader *>(msg);
        ev << "Arrived Generic Frame\n";
        SubType Type;
        Type = frame->getTYPE();
        if (Type.Subheader == 1)
        {
            ev << "Subheader\n";
            handleManagmentFrame(frame);
        }
        else
        {
        	error("unhandled message: %s (type: %s)\n", msg->getName(), msg->getClassName());
        	delete msg;
        }
    }
    else
    {
    	error("unhandled message: %s (type: %s)\n", msg->getName(), msg->getClassName());
    	delete msg;
    }
}

void ControlPlaneBaseAP::handleTimer(cMessage *msg)
{
    if (msg == BroadcastTimer)
    {
        sendBroadcast();
    }
    else if (msg == StartsetRadio)
    {
        setRadio(BSInfo);       //Radiokanäle werden gesetzt
    }
    else
    {
        error("internal error: unrecognized timer '%s'", msg->getName());
    	delete msg;
    }
}

void ControlPlaneBaseAP::handleManagmentFrame(Ieee80216GenericMacHeader *GenericFrame)
{
    Ieee80216ManagementFrame *Mgmtframe = check_and_cast<Ieee80216ManagementFrame *>(GenericFrame);

    switch (Mgmtframe->getManagement_Message_Type())
    {
    case ST_RNG_REQ:
        handle_RNG_REQ_Frame(check_and_cast<Ieee80216_RNG_REQ *>(Mgmtframe));
        break;

    case ST_SBC_REQ:
        handle_SBC_REQ_Frame(check_and_cast<Ieee80216_SBC_REQ *>(Mgmtframe));
        break;

    case ST_REG_REQ:
        handle_REG_REQ_Frame(check_and_cast<Ieee80216_REG_REQ *>(Mgmtframe));
        break;

    default:
    	error("unhandled message in ControlPlaneBaseAP::handleManagmentFrame: %s (type: %s)\n",
    			GenericFrame->getName(), GenericFrame->getClassName());
    	delete GenericFrame;
    }
}

void ControlPlaneBaseAP::handleUpperMessage(cMessage *msg)
{
	error("unhandled message in ControlPlaneBaseAP::handleUpperMessage: %s (type: %s)\n",
			msg->getName(), msg->getClassName());
	delete msg;
}

void ControlPlaneBaseAP::sendLowerMessage(cMessage *msg)
{
    send(msg, "cpsDownOut");
}

void ControlPlaneBaseAP::sendRadioUpOut(cMessage *msg)
{
    send(msg, "radioUpOut");
}

void ControlPlaneBaseAP::sendLowerDelayMessage(double delayTime, cMessage *msg)
{
    sendDelayed(msg, delayTime, "cpsDownOut");
}

void ControlPlaneBaseAP::sendBroadcast()
{
    makeDL_MAP(BSInfo);
    makeDCD(BSInfo);
    makeUCD(BSInfo);
    scheduleAt(simTime() + BSInfo.DLMAP_interval, BroadcastTimer);
}

void ControlPlaneBaseAP::handle_RNG_REQ_Frame(Ieee80216_RNG_REQ *frame)
{
    ev << "RNG_REQ arrived.\n";
    storeMSSInfo(frame->getMSS_MAC_Address());
    makeRNG_RSP(frame->getMSS_MAC_Address());
}

void ControlPlaneBaseAP::handle_SBC_REQ_Frame(Ieee80216_SBC_REQ *frame)
{
    ev << "SBC_REQ arrived.\n";
    makeSBC_RSP();
}

void ControlPlaneBaseAP::handle_REG_REQ_Frame(Ieee80216_REG_REQ *frame)
{
    ev << "REG_REQ arrived.\n";
    makeREG_RSP();
}

void ControlPlaneBaseAP::clearMSSList()
{
    for (MobileSubscriberStationList::iterator it = mssList.begin(); it != mssList.end(); ++it)
        if (it->authTimeoutMsg)
            delete cancelEvent(it->authTimeoutMsg);
    mssList.clear();
}

void ControlPlaneBaseAP::storeMSSInfo(const MACAddress& Address) //create new Base Station ID in BSInfo list
{
    MSSInfo *mss = lookupMSS(Address);
    if (mss)
    {
        EV << "MAC Address=" << Address << " already in our MSS list.\n";
    }
    else
    {
        EV << "Inserting MAC Address " << Address << " into our MSS list\n";
        mssList.push_back(MSSInfo());
        mss = &mssList.back();
    }
}

ControlPlaneBaseAP::MSSInfo* ControlPlaneBaseAP::lookupMSS(const MACAddress& Address)
{
    for (MobileSubscriberStationList::iterator it = mssList.begin(); it != mssList.end(); ++it)
    {
        if (it->MSS_Address == Address)
        {
            ev << Address;
            return &(*it);
        }
    }
    return NULL;
}
