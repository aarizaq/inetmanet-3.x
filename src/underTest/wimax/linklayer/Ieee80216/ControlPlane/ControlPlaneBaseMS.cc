//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//

#include"ControlPlaneBaseMS.h"

Define_Module(ControlPlaneBaseMS);

#define MK_STARTUP  1

static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

ControlPlaneBaseMS::ControlPlaneBaseMS()
{
    endTransmissionEvent = NULL;
}

void ControlPlaneBaseMS::displayStatus(bool isBusy)
{
    this->getDisplayString().setTagArg("t", 0, isBusy ? "transmitting" : "idle");
    this->getDisplayString().setTagArg("i", 1, isBusy ? (queue.length() >= 3 ? "red" : "yellow") : "");
}

ControlPlaneBaseMS::~ControlPlaneBaseMS()
{
    cancelAndDelete(endTransmissionEvent);
}

void ControlPlaneBaseMS::initialize()
{
    const char *addressString = par("address"); //MAC Addresse automatisch oder mit vorgegebene Addresse zuweisen
    if (!strcmp(addressString, "auto"))
    {
        MSInfo.MobileMacAddress = MACAddress::generateAutoAddress(); // MAC Addresse automatisch zuweisen
        par("address").setStringValue(MSInfo.MobileMacAddress.str().c_str()); // module parameter von "auto" auf konkrete Addresse umstellen
    }
    else
    {
        MSInfo.MobileMacAddress.setAddress(addressString); // Vorgegebene MAC Addresse übernehmen
    }
    nb = NotificationBoardAccess().get();
    MSInfo.ScanTimeInterval = par("scanintervall");

    clearBSList();              //Liste mit gescannten Basisstationen löschen

    isScanning = false;
    isAssociated = false;
    //assocTimeoutMsg = NULL;
    scanning.recieve_DL_MAP = false;
    scanning.recieve_DCD = false;
    scanning.recieve_UCD = false;
    scanning.scanChannel = 0;
    queue.setName("queue");
    endTransmissionEvent = new cMessage("endTxEvent");

    //gateToWatch = gate("");
    // subscribe for the information of the carrier sense
    nb->subscribe(this, NF_RADIOSTATE_CHANGED);
    EV << "Inizilisierung von ControlPlaneMS abgeschlossen.\n";
    // start up: send scan request
    scheduleAt(simTime(), new cMessage("startUp", MK_STARTUP));
}

void ControlPlaneBaseMS::startTransmitting(cMessage *msg)
{
    if (ev.isGUI())
        displayStatus(true);

    EV << "Starting transmission of " << msg << endl;
}

/** Ankommende Nachrichten werden an die Funktionen handleTimer(),
 handleCommand(), handleMangementFrame(), handleUpperMessage() or processFrame(). */
void ControlPlaneBaseMS::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())   // Abarbeiten von Nachrichten die vom diesem Module gesendet werden
    {
        // process timers
        EV << "Received " << msg << " from it self" << endl;
        handleTimer(msg);
    }
    else if (msg->arrivedOn("cpsUpIn")) // Abarbeiten von Nachrichten die vom MAC Module gesendet werden
    {

        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        Ieee80216GenericMacHeader *frame = check_and_cast<Ieee80216GenericMacHeader *>(msg);
        EV << "Arrived Generic Frame\n";
        SubType Type;           //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
        Type = frame->getTYPE();

        if (Type.Subheader == 1)
        {
            EV << "Subheader\n";
            handleManagmentFrame(frame);
        }
        else
        {
        	error("unhandled message: %s (type: %s)\n", msg->getName(), msg->getClassName());
        	delete msg;
        }
    }
    else                        // Abarbeiten von Nachrichten die vom überliegenden Schichten gesendet werden
    {
        EV << "Received " << msg << " from Quelle" << endl;
        EV << " Mgmt Module ID:" << getId() << endl;
        send(msg, "cpsDownOut");
    }
}

void ControlPlaneBaseMS::handleManagmentFrame(Ieee80216GenericMacHeader *GenericFrame)
{
    Ieee80216ManagementFrame *Mgmtframe = check_and_cast<Ieee80216ManagementFrame *>(GenericFrame);

    switch (Mgmtframe->getManagement_Message_Type())
    {
    case ST_DL_MAP:
        handleDL_MAPFrame(check_and_cast<Ieee80216DL_MAP *>(Mgmtframe));
        break;

    case ST_DCD:
        handleDCDFrame(check_and_cast<Ieee80216_DCD *>(Mgmtframe));
        break;

    case ST_UCD:
        handleUCDFrame(check_and_cast<Ieee80216_UCD *>(Mgmtframe));
        break;

    case ST_RNG_RSP:
        handle_RNG_RSP_Frame(check_and_cast<Ieee80216_RNG_RSP *>(Mgmtframe));
        break;

    case ST_SBC_RSP:
        handle_SBC_RSP_Frame(check_and_cast<Ieee80216_SBC_RSP *>(Mgmtframe));
        break;

    case ST_REG_RSP:
        handle_REG_RSP_Frame(check_and_cast<Ieee80216_REG_RSP *>(Mgmtframe));
        break;

    default:
        error("Invalid management message type: %i",Mgmtframe->getManagement_Message_Type());
        break;
    }
}

void ControlPlaneBaseMS::handleCommand(int msgkind, cPolymorphic *ctrl)
{
    if (dynamic_cast<Ieee80216Prim_ScanRequest *>(ctrl))
        processScanCommand((Ieee80216Prim_ScanRequest *) ctrl);
    else
    	error("unhandled message: %s (type: %s)\n", ctrl->getName(), ctrl->getClassName());

//    else if (ctrl);
    delete ctrl;
}

void ControlPlaneBaseMS::handleTimer(cMessage *msg)
{
    if (msg->getKind() == MK_Scan_Channel_Timer)
    {
        scanNextChannel();
    }
    else if (msg->getKind() == MK_DL_MAP_Timeout)
    {
        scanNextChannel();
    }
    else if (msg->getKind() == MK_STARTUP)
    {
        // process command from agent
        int msgkind = msg->getKind();
        cPolymorphic *ctrl = msg->removeControlInfo();
        delete msg;

        handleCommand(msgkind, ctrl);
    }
    else
    {
        error("internal error: unrecognized timer '%s'", msg->getName());
    }
}

void ControlPlaneBaseMS::handleUpperMessage(cMessage *msg)
{
	error("unhandled message: %s (type: %s)\n", msg->getName(), msg->getClassName());
}

void ControlPlaneBaseMS::sendLowerMessage(cMessage *msg)
{
    send(msg, "cpsDownOut");
}

/**
* Basisstation in die Liste eintragen
*
*/
void ControlPlaneBaseMS::storeBSInfo(const MACAddress & BasestationID) //create new Base Station ID in BSInfo list
{
    BSInfo *bs = lookupBS(BasestationID);
    if (bs)
    {
        EV << "Basestation ID=" << BasestationID << " already in our BS list.\n";
    }
    else
    {
        EV << "Inserting Basestation ID " << BasestationID << " into our BS list\n";
        bsList.push_back(BSInfo());
        bs = &bsList.back();
    }
}

/**
* Suchfunktion mit der anhand der BasestationID eine Basisstation
* in der Liste gefunden wird
*/
ControlPlaneBaseMS::BSInfo* ControlPlaneBaseMS::lookupBS(const MACAddress & BasestationID)
{
    for (BasestationList::iterator it = bsList.begin(); it != bsList.end(); ++it)
    {
        if (it->BasestationID == BasestationID)
        {
            EV << BasestationID;
			return &(*it);
        }
    }
    return NULL;
}

void ControlPlaneBaseMS::clearBSList()
{
    for (BasestationList::iterator it = bsList.begin(); it != bsList.end(); ++it)
    {
        if (it->authTimeoutMsg)
        {
            delete cancelEvent(it->authTimeoutMsg);
        }
    }
    bsList.clear();
}

void ControlPlaneBaseMS::processScanCommand(Ieee80216Prim_ScanRequest * ctrl)
{
    EV << "Received Scan Request from agent.\n";

    if (isScanning)
        error("processScanCommand: scanning already in progress");

    isScanning = true;
    scanNextChannel();
}

bool ControlPlaneBaseMS::scanNextChannel()
{
    if (scanning.scanChannel == 6) // Es sindnur fünf Kanäle, wenn der Zähler bei sechs ist von neuem Anfangen
        scanning.scanChannel = 1;
    changeDownlinkChannel(scanning.scanChannel);
    scanning.scanChannel = ++(scanning.scanChannel);
    ScanChannelTimer = new cMessage("ScanChannelTimer", MK_Scan_Channel_Timer);
    scheduleAt(simTime() + MSInfo.ScanTimeInterval, ScanChannelTimer);
    EV << "Scan next channel " << scanning.scanChannel << "\n";
    return false;
}

/**
/Die Methode changDownlinkChannel ändert den Kanal der Donlink Radio Moduls
/Mit der eigenen msg-Nachricht WiMAXPhyControlInfo, mit ihr ist es möglich
/zwischen dem Uplink und Downlink Module zu wählen
*/
void ControlPlaneBaseMS::changeDownlinkChannel(int channelNum) //Einstellen des Kanal
{
    PhyControlInfo *phyCtrl = new PhyControlInfo();
    phyCtrl->setChannelNumber(channelNum);
    cMessage *msg = new cMessage("changeChannel", PHY_C_CONFIGURERADIO);
    msg->setControlInfo(phyCtrl);
    send(msg, "radioUpOut");
}

/**
/Die Methode changUplinkChannel ändert den Kanal der Uplink Radio Moduls
/Mit der eigenen msg-Nachricht WiMAXPhyControlInfo, mit ihr ist es möglich
/zwischen dem Uplink und Downlink Module zu wählen
*/
void ControlPlaneBaseMS::changeUplinkChannel(int channelNum)
{
    PhyControlInfo *phyCtrl = new PhyControlInfo();
    phyCtrl->setChannelNumber(channelNum);
    cMessage *msg = new cMessage("changeChannel", PHY_C_CONFIGURERADIO);
    msg->setControlInfo(phyCtrl);
    send(msg, "cpsDownOut");
}

/**
* In die Liste den UlLink Kanal in die zugehörige Basisstation eintragen
*
*/
void ControlPlaneBaseMS::UplinkChannelBS(const MACAddress & BasestationID, const int UpChannel)
{
    BSInfo *bs = lookupBS(BasestationID);
    if (bs)
    {
        bs->UplinkChannel = UpChannel;
    }
}

void ControlPlaneBaseMS::handleDL_MAPFrame(Ieee80216DL_MAP * frame)
{
    EV << "DL_MAP arrived from BasestationID:" << frame->getBS_ID() << "\n";

    storeBSInfo(frame->getBS_ID());
    if (ScanChannelTimer != NULL) //Wenn eine gültige DL_Map empfangen wurde, wird nicht weiter nacheinem Download Kanal gesucht
    {
        cancelEvent(ScanChannelTimer);
        EV << "Cancel ScanChannelTimer event\n";
        scheduleAt(simTime() + 30, ScanChannelTimer);
    }
    scanning.recieve_DL_MAP = true;
}

void ControlPlaneBaseMS::handleDCDFrame(Ieee80216_DCD * frame)
{
    if (scanning.recieve_DL_MAP)
    {
        EV << "DCD arrived.\n";
        scanning.recieve_DCD = true;
    }
}

void ControlPlaneBaseMS::handleUCDFrame(Ieee80216_UCD * frame)
{
    if (scanning.recieve_DL_MAP && scanning.recieve_DCD)
    {
        EV << "UCD arrived.\n";
        scanning.recieve_UCD = true;
        UplinkChannelBS(frame->getBS_ID(), frame->getUploadChannel());
        changeUplinkChannel(frame->getUploadChannel());
        makeRNG_REQ(MSInfo);
    }
}

void ControlPlaneBaseMS::handle_RNG_RSP_Frame(Ieee80216_RNG_RSP * frame)
{
    if (scanning.recieve_DL_MAP && scanning.recieve_DCD && scanning.recieve_UCD)
    {
        EV << "RNG_RSP arrived.\n";
        if (frame->getMSS_MAC_Address() == MSInfo.MobileMacAddress)
        {
            makeSBC_REQ(MSInfo);
        }
        else
        {
            EV << "RNG_RSP not for this mobile station.\n";
        }
    }
}

void ControlPlaneBaseMS::handle_SBC_RSP_Frame(Ieee80216_SBC_RSP * frame)
{
    if (scanning.recieve_DL_MAP && scanning.recieve_DCD && scanning.recieve_UCD)
    {
        EV << "SBC_RSP arrived.\n";
        makeREG_REQ(MSInfo);
    }
}

void ControlPlaneBaseMS::handle_REG_RSP_Frame(Ieee80216_REG_RSP * frame)
{
    if (scanning.recieve_DL_MAP && scanning.recieve_DCD && scanning.recieve_UCD)
        EV << "REG_RSP arrived.\n";
}

void ControlPlaneBaseMS::receiveChangeNotification(int category, const cPolymorphic * details)
{
    Enter_Method_Silent();
    //printNotificationBanner(category, details);

    if (category == NF_RADIOSTATE_CHANGED)
    {
        //RadioState::State newRadioState = check_and_cast<RadioState *>(details)->getState();
        RadioState *newRadioState = check_and_cast<RadioState *>(details);
        //EV << "** Radio state update in " << className() << ":info " << details->info() << ":state " << newRadioState.getState() << ":R "<< newRadioState.getRadioId()<< "\n";
        EV << "** Radio state update in " << getClassName() << ":state " <<
            newRadioState->getChannelNumber() << ":Name " << newRadioState->getRadioId() << "\n";
        // FIXME: double recording, because there's no sample hold in the gui
        //radioStateVector.record(radioState);
        //radioStateVector.record(newRadioState);

        //radioState = newRadioState;
    }
}
