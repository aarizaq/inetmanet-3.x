//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//

#include "ControlPlaneBasestation.h"
#include "ModulationsConsts.h"

Define_Module(ControlPlaneBasestation);

ControlPlaneBasestation::ControlPlaneBasestation()
{
    systemStartTimer = NULL;
    preambleTimer = NULL;
    broadcastTimer = NULL;
    endTransmissionEvent = NULL;
    startTransmissionEvent = NULL;
    setMStoScanmodusTimer = NULL;
    scantimer = 0;
}

ControlPlaneBasestation::~ControlPlaneBasestation()
{
    cancelAndDelete(systemStartTimer);
    cancelAndDelete(preambleTimer);
    cancelAndDelete(broadcastTimer);
    cancelAndDelete(endTransmissionEvent);
    cancelAndDelete(startTransmissionEvent);
    cancelAndDelete(setMStoScanmodusTimer);
}

//void ControlPlaneBasestation::startTransmitting(cMessage *msg)
//{
//    EV << "Starting transmission of " << msg << endl;
//}

void ControlPlaneBasestation::finish()
{
    recordScalar("Simulation Time", simTime());
    recordScalar("Downlink per second", localBasestationInfo.downlink_per_second);
    recordScalar("Downlink to Uplink ratio", localBasestationInfo.downlink_to_uplink);
    recordScalar("Uplink-Grant scheduling for more than one frame",
                 localBasestationInfo.useULGrantWaitingQueue);
    recordScalar("Uplink-Grant compensation factor", localBasestationInfo.grant_compensation);
}

void ControlPlaneBasestation::initialize(int stage)
{
    ControlPlaneBase::initialize(stage);

    if (stage == 0)
    {
        /**
         * Basestation ID zuweisen
         */
        const char *BasestationIDString = par("basestationid");
        if (!strcmp(BasestationIDString, "auto")) //wenn "auto" als Parameter wird eine Adresse generiert
        {
            localBasestationInfo.BasestationID = MACAddress::generateAutoAddress();
            par("basestationid").setStringValue(localBasestationInfo.BasestationID.str().c_str());
        }
        else
        {
            localBasestationInfo.BasestationID.setAddress(BasestationIDString); // Es wird der Parameter aus der INI-Datei uebernommen
        }

        /**
         * Uebernahme von Paramtern fuer die Basisstation
         *
         */
        //DlMapInterval = &cMsgPar("DLMapInterval");
        //localBasestationInfo.DLMAP_interval = DlMapInterval->doubleValue();// festlegen der DL-MAP Intervall und länge des Donlink Rahmen

        //DlMapInterval = par("DLMapInterval");

        localBasestationInfo.DLMAP_interval = par("DLMapInterval");
        localBasestationInfo.DownlinkChannel = par("DownlinkChannel");
        localBasestationInfo.UplinkChannel = par("UplinkChannel");

        localBasestationInfo.strartFrame = par("startFrame"); //Anzahl der Frames nach dem die MS mit dem scanning begiien soll

        // korrekten Wert nochmal im Standard nachschlagen!
        localBasestationInfo.ul_dl_ttg = par("UplinkDownlinkTTG");

        localBasestationInfo.enablePacking = par("enablePacking").boolValue();

        localBasestationInfo.radioDatenrate = QPSK1_DATENRATE; // Consts.h Datenrate bei QPSK 1/2

        localBasestationInfo.downlink_per_second = par("downlink_per_second").doubleValue();
        localBasestationInfo.downlink_to_uplink = par("downlink_to_uplink").doubleValue();
        if (localBasestationInfo.downlink_per_second == 0)
        {
            localBasestationInfo.downlink_per_second =
                localBasestationInfo.radioDatenrate * par("downlink_to_uplink").doubleValue();
        }

        localBasestationInfo.useULGrantWaitingQueue = par("useULGrantWaitingQueue").boolValue();
        localBasestationInfo.grant_compensation = par("grant_compensation").doubleValue();

        /**
         * Inizilisieren von Hilfsvariablen
         *
         */

        systemStartTimer = new cMessage("SystemStartTimer");
        preambleTimer = new cMessage("PreambleTimer");
        broadcastTimer = new cMessage("BroadcastTimer");
        endTransmissionEvent = new cMessage("endTxEvent");
        startTransmissionEvent = new cMessage("startTransmissionEvent");
        //setMStoScanmodusTimer = new cMessage("setMStoScanmodusTimer");

//        scheduleTimer = new cMessage("scheduleTimer");
//        scheduleAt(simTime() + DlMapInterval->doubleValue() / 2, scheduleTimer);

          /**
         * Inizilisieren von Hilfsvariablen
         *
         */
        clearMSSList();         //clear Mobile Subscriber Station list

        scheduleAt(simTime(), systemStartTimer); //Startet mit dem Senden
        //scheduleAt(simTime(), preambleTimer);//Setzt die Radiokanaele
        //scheduleAt(simTime()+2, broadcastTimer);//Starte das aussenden der DL-MAP,DCD und UCD (isSelfMessage)

        // state variables
        fsm.setName("BS Control Plane State Machine");
        //EV << "Inizilisierung von ControlPlaneBS abgeschlossen.\n";
    }

    // get the pointer to the scheduling module
    cModule *module2 =
        getParentModule()->getParentModule()->getSubmodule("bsTransceiver")->
        getSubmodule("cpsTransceiver")->getSubmodule("scheduling");
    cps_scheduling = check_and_cast<CommonPartSublayerScheduling *>(module2);

    // get the pointer to the cpsServiceFlows module
    cModule *module = getParentModule()->getSubmodule("cp_serviceflows");
    cpsSF_BS = check_and_cast<CommonPartSublayerServiceFlows_BS *>(module);
    cpsSF_BS->setMSSList(&localeMobilestationList);
    cpsSF_BS->setBSInfo(&localBasestationInfo);

    map_connections = cpsSF_BS->getConnectionMap();
    map_serviceFlows = cpsSF_BS->getServiceFlowMap();

    cModule *module_cps_receiver =
        getParentModule()->getParentModule()->getSubmodule("bsReceiver")->
        getSubmodule("cpsReceiver")->getSubmodule("cps_receiver");

    cps_Receiver_BS = check_and_cast<CommonPartSublayerReceiver *>(module_cps_receiver);
	cps_Receiver_BS->setConnectionMap(map_connections);
	cps_Receiver_BS->setSubscriberList(&localeMobilestationList);

    //Verbindung zu den Radio-Modulen
    cModule *module_radio_Transceiver =
        getParentModule()->getParentModule()->getSubmodule("bsTransceiver")->getSubmodule("radioTransceiver");
    EV << "Achtung RS: Name des Radio Moduls: " << module_radio_Transceiver->getName() << "\n";
    transceiverRadio = check_and_cast<Ieee80216Radio *>(module_radio_Transceiver);

    cModule *module_radio_Receiver =
        getParentModule()->getParentModule()->getSubmodule("bsReceiver")->getSubmodule("radioReceiver");
    EV << "Achtung RS: Name des Radio Moduls: " << module_radio_Receiver->getName() << "\n";
    receiverRadio = check_and_cast<Ieee80216Radio *>(module_radio_Receiver);

    /**
     * the initial ulmap-size:
     * ranging opportunities + request opportunities
     */
    cur_ulmap_size = localBasestationInfo.Ranging_request_opportunity_size * 48
        + localBasestationInfo.Bandwidth_request_opportunity_size * 48;

    // initially set the size of dlmap_ie messages for later reference
    Ieee80216DL_MAP_IE *tmp = new Ieee80216DL_MAP_IE();
    localBasestationInfo.dlmap_ie_size = tmp->getBitlength();
    delete tmp;

    uplink_subframe_starttime_offset =
        (localBasestationInfo.radioDatenrate -
         localBasestationInfo.downlink_per_second) * localBasestationInfo.DLMAP_interval /
        localBasestationInfo.radioDatenrate;
    EV << "VITA: uplink_subframe_starttime_offset: " << uplink_subframe_starttime_offset << endl;

// module = parentModule()->submodule("cp_authorization");
// cps_auth = check_and_cast<CommonPartSublayerAuthorizationModule*>(module);
    cpsSF_BS->setAvailableDownlinkDatarate(localBasestationInfo.downlink_per_second);
    cpsSF_BS->setAvailableUplinkDatarate(localBasestationInfo.radioDatenrate -
                                         localBasestationInfo.downlink_per_second);

    cvec_grant_capacity[MANAGEMENT].setName("Management grant capacity");
    cvec_grant_capacity[UGS].setName("UGS grant capacity");
    cvec_grant_capacity[RTPS].setName("rtPS grant capacity");
    cvec_grant_capacity[ERTPS].setName("ertPS grant capacity");
    cvec_grant_capacity[NRTPS].setName("nrtPS grant capacity");
    cvec_grant_capacity[BE].setName("BE grant capacity");

    registerInterface();
}

/**
 * @brief Hauptfunktionen
 ***************************************************************************************/

/**
 * Handles classification commands from the traffic classification module for BS initiated traffic.
 * If the primary management connection is not active for the SS, the BS activates it and grants
 * the SS uplink opportunities in the next UL-MAP.
 * TODO classification BS
 */
void ControlPlaneBasestation::handleClassificationCommand(Ieee80216ClassificationCommand *command)
{
    EV << "(in ControlPlaneBasestation::handleClassificationCommand) received command: " <<
        command->getName() << ".\n";
    MobileSubscriberStationList::iterator it;
    for (it = localeMobilestationList.begin(); it != localeMobilestationList.end(); it++)
    {
        ServiceFlow *requested_sf = new ServiceFlow();
        requested_sf->state = (sf_state) command->getRequested_sf_state();
        requested_sf->provisioned_parameters = &(command->getRequested_qos_params());

        cpsSF_BS->createAndSendNewDSA_REQ(it->Primary_Management_CID,
                                          requested_sf,
                                          (ip_traffic_types) command->getTraffic_type());

        getServiceFlowForCID(it->Primary_Management_CID)->state = SF_ACTIVE;
    }
    delete command;
}

/**
* Erweiterung handleMessage() aus ControlPlaneBase.h
*
*/
void ControlPlaneBasestation::handleMessage(cMessage *msg)
{
    EV << "(in ControlPlaneBasestation::handleMessage) received message: " <<
        msg->getName() << ".\n";
    ControlPlaneBase::handleMessage(msg);

    if (msg->getArrivalGateId() == serviceFlowsGateIn)
    {
        //every message coming via serviceFlowGateIn is MacManagementMessage
        //ist die Nachricht vom Type Generic MAC-Header == MAC-PDU
        if (dynamic_cast<Ieee80216GenericMacHeader *>(msg))
        {
            Ieee80216GenericMacHeader *genericMacHeader =
                check_and_cast<Ieee80216GenericMacHeader *>(msg);
            handleManagementFrame(genericMacHeader);
        }
        else
        {
            error("Handle Service Flow kein Generic Mac Header Frame!");
        }
    }
}

/**
* Bearbeitung von Nachrichten h�herer Schichten
*/
void ControlPlaneBasestation::handleHigherLayerMsg(cMessage *msg)
{
    EV << "(in ControlPlaneBasestation::handleHigherLayerMsg) received message: " <<
        msg->getName() << ".\n";
}

/**
* Bearbeitung von Nachrichten unterer Schichten
*/
void ControlPlaneBasestation::handleLowerLayerMsg(cMessage *msg)
{
    EV << "(in ControlPlaneBasestation::handleLowerLayerMsg) received message: " <<
        msg->getName() << ".\n";
    if (dynamic_cast<Ieee80216GenericMacHeader *>(msg))
    {
        Ieee80216GenericMacHeader *genericMacHeader =
            check_and_cast<Ieee80216GenericMacHeader *>(msg);

        // TODO die Abfrage muss genauer werden. Problem, wenn die SS zwischen 2 BS steht
        // RS die Abfrage ob die MAC PDU an diese MS gerichtet ist, wird im CPS Module abgefangen
        SubType Type;
        Type = genericMacHeader->getTYPE();

        if (Type.Subheader == 1)
        {
            handleManagementFrame(genericMacHeader);
        }
        // if (msg->getOwner() == this)   // FIXME
            delete msg;
    }
    else if (dynamic_cast<Ieee80216BandwidthMacHeader *>(msg))
    {
        handleBandwidthRequest(check_and_cast<Ieee80216BandwidthMacHeader *>(msg));
        // if (msg->getOwner() == this)   // FIXME
            delete msg;
    }
    else
    {
        error("Handle Lower Message kein Generic Mac Header Frame!");
    }
}

/**
* Identifikation von Management-Nachrichten
*/
void ControlPlaneBasestation::handleManagementFrame(Ieee80216GenericMacHeader *genericFrame)
{
    EV << "(in ControlPlaneBasestation::handleManagementFrame) " << genericFrame << endl;
    if (!(dynamic_cast<Ieee80216ManagementFrame *>(genericFrame)))
    {
        error("Handle Management Frame keine Kontrolnachricht!");
    }

    Ieee80216ManagementFrame *mgmtFrame =
        check_and_cast<Ieee80216ManagementFrame *>(genericFrame);

    switch (mgmtFrame->getManagement_Message_Type())
    {
    case ST_RNG_REQ:
        handle_RNG_REQ_Frame(check_and_cast<Ieee80216_RNG_REQ *>(mgmtFrame));
        break;

    case ST_SBC_REQ:
        handle_SBC_REQ_Frame(check_and_cast<Ieee80216_SBC_REQ *>(mgmtFrame));
        break;

    case ST_REG_REQ:
        handle_REG_REQ_Frame(check_and_cast<Ieee80216_REG_REQ *>(mgmtFrame));
        break;

    case ST_MOB_SCN_REQ:
        handle_MOB_SCN_REQ_Frame(check_and_cast<Ieee80216_MOB_SCN_REQ *>(mgmtFrame));
        break;

    case ST_MSHO_REQ:
        handle_MOB_MSHO_REQ_Frame(check_and_cast<Ieee80216_MSHO_REQ *>(mgmtFrame));
        break;

    case ST_MOB_HO_IND:
        handle_MOB_HO_IND_Frame(check_and_cast<Ieee80216_MOB_HO_IND *>(mgmtFrame));
        break;

    case ST_DSA_REQ:
    case ST_DSA_RSP:
    case ST_DSA_ACK:
    case ST_DSX_RVD:
        // The request arrives via receiver gate
        if (mgmtFrame->getArrivalGateId() == receiverCommonGateIn)
        {
            send(mgmtFrame, serviceFlowsGateOut);
        }
        // The request arrives via ServiceFlowManagement gate
        else
        {
            send(mgmtFrame, transceiverCommonGateOut);
        }
        break;

    default:
        error("error: Management Message Type not in switch-statement");
        delete mgmtFrame;
    }
}

/**
 * Funktion zum Abarbeiten von Self messages.
 *
 *
 ****************************************************************************/
void ControlPlaneBasestation::handleSelfMsg(cMessage *msg)
{
    EV << "(in  ControlPlaneBasestation::handleSelfMsg) " << endl;
    EV << "Self message:" << msg->getName() << " im Modul (" << msg <<
        "|" << setMStoScanmodusTimer << " | " << scantimer << "\n";
    string scanmodusTimer = "setMStoScanmodusTimer";

    if (msg == broadcastTimer)
    {
        handleWithFSM(msg);
    }
    else if (msg == systemStartTimer)
    {
        setRadio();             //Radiokanaele werden gesetzt
        scheduleAt(simTime() + 0.002, preambleTimer);
    }
    else if (msg == preambleTimer)
    {
        handleWithFSM(msg);
    }
    else if (msg == startTransmissionEvent)
    {
        handleWithFSM(msg);
    }
    else if (msg == endTransmissionEvent)
    {
        stopControlBurst();
    }
    else if (scanmodusTimer.compare(msg->getName()) == 0)
    {
        scantimer++;
        setMobilestationScanmodus(msg);
    }
    else if (strcmp(msg->getName(), "sendControlPDU") == 0)
    {
        Ieee80216Prim_sendControlRequest *send_req = new Ieee80216Prim_sendControlRequest();
        int index = msg->findPar("CID");
        int cid = msg->par(index).longValue();

        send_req->setPduCID(cid);
        sendRequest(send_req);

        delete msg;
    }
    else if (strcmp(msg->getName(), "stopControlPDU") == 0)
    {
        Ieee80216Prim_stopControlRequest *stop_req = new Ieee80216Prim_stopControlRequest();
        int index = msg->findPar("CID");
        int cid = msg->par(index).longValue();

        stop_req->setPduCID(cid);
        sendRequest(stop_req);

        delete msg;
    }
    else
    {
        error("internal error: unrecognized timer '%s'", msg->getName());
    }
}

/**
* Basestation State-Machine
*/
void ControlPlaneBasestation::handleWithFSM(cMessage *msg)
{
    FSMA_Switch(fsm)
    {
        FSMA_State(PREAMBLE)
        {
            FSMA_Event_Transition(Transmit - Preamble,
                                  msg == preambleTimer,
                                  DLMAP,
                                  updateDisplay();
                                  cps_scheduling->sendPacketsDown(localBasestationInfo.downlink_per_second *
                                                                  localBasestationInfo.DLMAP_interval);
                                  scheduleAt(simTime(), preambleTimer);
                                  );
        }

        FSMA_State(DLMAP)
        {
            /** @brief "Kapitel 5.1.1" Broadcast-Control-Feld */
            FSMA_Event_Transition(Transmit - DL - Map,
                                  msg == preambleTimer,
                                  ULMAP,
                                  buildDL_MAP(); //erzeugt die DL-MAP Nachricht
                                  scheduleAt(simTime() + localBasestationInfo.DLMAP_interval,
                                             preambleTimer);
                                  scheduleAt(simTime(), broadcastTimer);
                                  );
        }

        FSMA_State(ULMAP)
        {
            /** @brief "Kapitel 5.1.1" Broadcast-Control-Feld */
            FSMA_Event_Transition(Transmit - UL - Map,
                                  msg == broadcastTimer,
                                  DCD,
                                  buildUL_MAP(); //erzeugt die UL-MAP Nachricht
                                  scheduleAt(simTime(), broadcastTimer);
                                  );
        }

        FSMA_State(DCD)
        {
            /** @brief "Kapitel 5.1.1" Broadcast-Control-Feld */
            FSMA_Event_Transition(Transmit - DCD,
                                  msg == broadcastTimer,
                                  UCD,
                                  buildDCD(); //erzeugt die DCD Nachricht
                                  scheduleAt(simTime(), broadcastTimer);
                                  );
        }

        FSMA_State(UCD)
        {
            /** @brief "Kapitel 5.1.1" Broadcast-Control-Feld */
            FSMA_Event_Transition(Transmit - UCD,
                                  msg == broadcastTimer,
                                  STARTBURST,
                                  buildUCD(); //erzeugt die UCD Nachricht
                                  scheduleAt(simTime(), broadcastTimer);
                                  );
        }
        FSMA_State(STARTBURST)
        {
            FSMA_Event_Transition(Transmit - Burst,
                                  msg == broadcastTimer,
                                  STOPBURST,
                                  startControlBurst();
                                  scheduleAt(simTime(), startTransmissionEvent);
                                  );
        }
        FSMA_State(STOPBURST)
        {
            FSMA_Event_Transition(Transmit - Burst,
                                  msg == startTransmissionEvent,
                                  PREAMBLE,
                                  scheduleAt(broadcast_stop_time, endTransmissionEvent);
                                  );
        }
    }
}

void ControlPlaneBasestation::handleServiceFlowMessage(cMessage *msg)
{
    EV << "(in  ControlPlaneBasestation::handleServiceFlowMessage) " << endl;
    send(msg, transceiverCommonGateOut);
}

/**
* Funktion zur bearbeitung des Bandwith Request MAC Header
*
**/
void ControlPlaneBasestation::handleBandwidthRequest(Ieee80216BandwidthMacHeader *bw_req)
{
    EV << "(in  ControlPlaneBasestation::handleBandwidthRequest) " << endl;
    bool isAuthorized;

    // Management Connections
    // are immediately activated
    if (lookupLocaleMobilestationListCID(bw_req->getCID()) != NULL)
    {
        EV << "BW-REQ for Management-Connection arrived, CID: " << bw_req->getCID() << "\n";
        isAuthorized = true;
    }
    // Transport Connections
    // have to be authorized before activation
    else
    {
        EV << "BW-REQ for Transport-Connection arrived, CID: " << bw_req->getCID() << "\n";
        isAuthorized = false;

        // TODO: Authorization holen!!!
    }

    if (isAuthorized)
    {
        // activate the corresponding ServiceFlow
        getServiceFlowForCID(bw_req->getCID())->state = SF_ACTIVE;
        EV << "ServiceFlow " << getServiceFlowForCID(bw_req->getCID())->SFID <<
            " set ACTIVE for CID: " << bw_req->getCID() << " (type: " <<
            getServiceFlowForCID(bw_req->getCID())->link_type << ")\n";
    }
}

/*
void ControlPlaneBasestation::handleScheduleTimer()
{
    EV << "\n ENTER HANDLESCHEDULETIMER \n";

    if (map_serviceFlows->size() > 0)
    {
        ServiceFlowMap::iterator sf_it;
        for (sf_it=map_serviceFlows->begin(); sf_it!=map_serviceFlows->end(); sf_it++)
        {
            ServiceFlow *cur_sf = &(sf_it->second);
            if (cur_sf->state == SF_ACTIVE)
            {
                // dummy zum testen
                char *cid_str;
                sprintf(cid_str, "%d", cur_sf->CID);
                cMessage *sf_timer = new cMessage(cid_str);

                if (sf_timer->isScheduled() )
                {
                    EV << "\n\n\n KLAPPT!!! \n\n\n";
                }
                else
                {
                    EV << "Message with CID="<< cur_sf->CID
                        <<" has not been scheduled, yet. \n\n\n";
                    scheduleAt(simTime() + (DlMapInterval->doubleValue() / 2),
                        sf_timer);
                }
            }
        }
    }
}
*/

void ControlPlaneBasestation::buildPreamble()
{
    Ieee80216_Preambel *Preamble = new Ieee80216_Preambel("Preamble");
    sendtoLowerLayer(Preamble);
    EV << "Laenge der Praemble: " << Preamble->getByteLength() << endl;
}

/**
* @brief Funktionen zum erzeugen von Management-Nachrichten
*
******************************************************************************************/

/**
 * Constructs the DL-MAP.
 * When the map is built, DL_MAP_IEs are built and appended to the map for elements
 * in the dataMsgQueue. These can be read by the transceiver module in the DL-MAPs fly-by
 * to trigger send-/stopControlRequests for the data queue.
 */
/** @brief "Kapitel 5.2.4" MAP-Nachrichten */
void ControlPlaneBasestation::buildDL_MAP() //Erzeugt die DL-Map Managementnachricht
{
    EV << "(in ControlPlaneBasestation::buildDL_MAP) " << endl;
    long tx_data_in_next_dlmap = 0;

    /** General MAC-Header Informationen
    */
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg definiert. Er kennzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;

    /** Erstelle DL-Map
    */
    Ieee80216DL_MAP *ManagementMessage = new Ieee80216DL_MAP("DL_MAP"); // Erzeugt eine Nachricht vom Type DL-Map. Management Message Type = 2 ist gesetzt (8 bit)
    ManagementMessage->setTYPE(Type);

    /** Nachrichtenspecifische Informationen
    */
    ManagementMessage->setBS_ID(localBasestationInfo.BasestationID); //Uebertraegt die BSID der Basisstation in die Nachricht

    // TODO assume UCD and DCD are 48bit... should be corrected when UCD/DCD are fully used
    int ucd_length = 48;
    int dcd_length = 48;

    /**
    * Keeps track of the offsets for start time calculations of controlSendRequests
    */
    double accumulated_offset_duration = 0;

    /**
    * Build DL-MAP_IE for each packet in the data queue and
    *  send send-/stopControlRequests to the transceiver.
    *
    * Einschränkung: datenpakete sollen an alle stationen mit geeignetem traffic_type im aktiven
    * ServiceFlow gehen --> testen mit mehreren stationen!!
    */
    if (!tx_dataMsgQueue->empty())
    {
        int tx_elements = tx_dataMsgQueue->size();
        EV << "Found " << tx_elements << " in the data queue. Creating DL_MAP-IEs...\n";

        ManagementMessage->setDlmap_ie_ListArraySize(tx_elements);

        accumulated_offset_duration = (tx_elements
                                       * localBasestationInfo.dlmap_ie_size + cur_ulmap_size
                                       + ucd_length + dcd_length)
            / localBasestationInfo.radioDatenrate;

        EV << "dlmap_ie_size	  : " << localBasestationInfo.dlmap_ie_size << "\n";
        EV << "cur_ulmap_size	  : " << cur_ulmap_size << "\n";
        EV << "acc_offset_duration: " << accumulated_offset_duration << "\n";
        EV << "radioDatenrate	  : " << localBasestationInfo.radioDatenrate << "\n";

        // send all elements currently waiting in the data queue
        Ieee80216MacHeaderFrameList::iterator it = tx_dataMsgQueue->begin();
        int index = 0;
        while (index < tx_elements && it != tx_dataMsgQueue->end())
        {
            //for (it=tx_dataMsgQueue->begin(); index < tx_elements; it++) {

            Ieee80216DL_MAP_IE *dl_map_ie = new Ieee80216DL_MAP_IE("DL-MAP_IE");
            Ieee80216GenericMacHeader *gm = check_and_cast<Ieee80216GenericMacHeader *>(*it);

            tx_data_in_next_dlmap += gm->getByteLength();

            double tx_element_duration =
                (double) gm->getByteLength() / localBasestationInfo.radioDatenrate;
            //EV << "gm->length()        = " << gm->length() <<"\n";
            //EV << "tx_element_duration = " << tx_element_duration <<"\n";

            dl_map_ie->setStartTime_Offset(accumulated_offset_duration);

            accumulated_offset_duration = accumulated_offset_duration + tx_element_duration;

            EV << "DL-Burst Starttime Offset: " << dl_map_ie->getStartTime_Offset() << "\n";

            ManagementMessage->setDlmap_ie_List(index++, *dl_map_ie);

            cMessage *start_msg = new cMessage("sendControlPDU");
            cMsgPar *par = new cMsgPar();
            par->setName("CID");
            par->setLongValue(1025);
            start_msg->addPar(par);

            scheduleAt(simTime() + dl_map_ie->getStartTime_Offset(), start_msg);

            // when all sendRequests for downlink are scheduled, schedule the stopRequest
            cMessage *stop_msg = new cMessage("stopControlPDU");
            cMsgPar *par2 = new cMsgPar();
            par2->setName("CID");
            par2->setLongValue(1025);
            stop_msg->addPar(par2);

            scheduleAt(simTime() + accumulated_offset_duration, stop_msg);
            //scheduleAt(simTime() + dl_map_ie->getStartTime_Offset()+ 0.0001 , stop_msg);

            it++;

            delete dl_map_ie;
        }
    }
    else
    {
        accumulated_offset_duration = (cur_ulmap_size
                                       + ucd_length + dcd_length)
            / localBasestationInfo.radioDatenrate;
    }

    broadcast_stop_time = simTime().dbl() + accumulated_offset_duration;
    EV << "BROADCAST DOWNLINK STOP TIME: " << broadcast_stop_time << "\n";

    /** Sende Nachricht zum Transciever Module
    */
    //uplink_subframe_starttime_offset = simTime() + localBasestationInfo.DLMAP_interval;

    uplink_subframe_starttime = simTime().dbl() + uplink_subframe_starttime_offset;
    EV << "\nDL subframe start: " << simTime() << "\n";
    EV << "UL subframe start: " << uplink_subframe_starttime << "\n";
    double duration = broadcast_stop_time - simTime().dbl();
    EV << "DL subframe contains " << tx_data_in_next_dlmap <<
        " bit downlink data (needed transmission time: " << duration << ")\n\n";

    EV << "Sende " << ManagementMessage->getName() <<
        " Nachricht. Die Simulationszeit:" << simTime() << " .\n";

    next_frame_start = simTime().dbl();
    sendtoLowerLayer(ManagementMessage);
}

/**
* Die Funktion erzeugt die UL-MAP Management Nachricht.
* Die Nachricht informiert die Mobilstation ueber ihre
* Senderechte
**/
/** @brief "Kapitel 5.2.4" MAP-Nachrichten */

void ControlPlaneBasestation::buildUL_MAP() //create DL_MAP frame
{
    EV << "(in ControlPlaneBasestation::buildUL_MAP)" << endl;
    buildRangingUL_MAP_IE();    //Funktion zum erzeugen der Ranging Sende-Intervalle
    buildBandwithRequestUL_MAP_IE(); //Funktion zum erzeugen der Bandwith Request Intervalle

    listActiveServiceFlows();   //debug output

    buildScheduledIEs();

    /** General MAC-Header Informationen
    */
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;

    /** Erstelle UL-Map
    */
    Ieee80216UL_MAP *ManagementMessage = new Ieee80216UL_MAP("UL_MAP"); // Erzeugt eine Nachricht vom Type DL-Map. Management Message Type = 2 ist gesetzt (8 bit)
    ManagementMessage->setTYPE(Type);

    //Zeitpunkt an dem der uplink Unterrahmen begint
    // anmerkung mk: bzw. der erste granted uplink burst der MS
    //ManagementMessage->setAllocation_start_time(simTime()+localBasestationInfo.DLMAP_interval);
    //ManagementMessage->setAllocation_start_time( simTime() + localBasestationInfo.DLMAP_interval
    //                + uplink_grant_starttime + localBasestationInfo.ul_dl_ttg );
    //ManagementMessage->setBS_ID(BSInfo.BasestationID);//Uebertraegt die BSID der Basisstation in die Nachricht
    ManagementMessage->setAllocation_start_time(uplink_subframe_starttime);

    /** Fuege Informationselemente in die UL-Map ein
    */
    EV << "BS: #UL-MAP_IE in UL-MAP: " << ul_map_ie_List.size() << "\n";

    // insert all IEs currently in the list
    ManagementMessage->setUlmap_ie_ListArraySize(ul_map_ie_List.size());

    int i = 0;
    for (UL_MAP_InformationselementList::iterator it = ul_map_ie_List.begin(); it != ul_map_ie_List.end(); ++it)
    {
        ManagementMessage->setUlmap_ie_List(i, *it);
        ++i;
    }

    ManagementMessage->setByteLength(ManagementMessage->getBitlength_without_ielist() + ul_map_ie_List.size() * 32); // (mk)

    /** Sende Nachricht zum Transciever Module
    */
    EV << "Sende " << ManagementMessage->getName() << "("
        << ManagementMessage->getByteLength()
        << "bit) Nachricht. Die Simulationszeit:" << simTime() << " .\n";
    sendtoLowerLayer(ManagementMessage);

    if (ul_map_ie_List.size() > 0) //Alte Informations Elemente aus der Liste Löschen
        ul_map_ie_List.clear();
}

/**
* Funktion zum erzeugen der Ranging Intervalle
*
**/
/** @brief "Kapitel 5.3.3" Initial Ranging */
void ControlPlaneBasestation::buildRangingUL_MAP_IE()
{
    EV << "(in ControlPlaneBasestation::buildRangingUL_MAP_IE)" << endl;
    Ieee80216UL_MAP_IE *frame = new Ieee80216UL_MAP_IE("RNG_UL_MAP_IE");
    frame->setCID(0x0000);      //Ranging CID
    frame->setStartTime_Offset(0);
    frame->setStopTime_Offset(localBasestationInfo.Ranging_request_opportunity_size * 48 / 4E6);
    ul_map_ie_List.push_back(*frame);

    delete frame;
}

/**
* Funktion zum erzeugen der Bandwith Request Intervalle
*
**/
void ControlPlaneBasestation::buildBandwithRequestUL_MAP_IE()
{
    EV << "(in ControlPlaneBasestation::buildBandwithRequestUL_MAP_IE)" << endl;
    Ieee80216UL_MAP_IE *frame = new Ieee80216UL_MAP_IE("BW_UL_MAP_IE");
    frame->setCID(0xFFFF);      //Broadcast CID
    frame->setStartTime_Offset(localBasestationInfo.Ranging_request_opportunity_size * 48 / 4E6);
    frame->setStopTime_Offset((localBasestationInfo.Ranging_request_opportunity_size
                               + localBasestationInfo.Bandwidth_request_opportunity_size) * 48 / 4E6);
    ul_map_ie_List.push_back(*frame);

    delete frame;
}

/**
 * Creates an UL_MAP_IE by a given CID and appends it to the list of pending IEs.
 * The CID has to be the corresponding stations Basic-CID (->802.16-Standard).
 * The IEs contain information for the transceiver on when to schedule a send-/stopControlRequest.
 * These requests trigger the sending of the next management-queue element.
 */
/** @brief "Kapitel 5.2.4" Informationselemente */
void ControlPlaneBasestation::buildUL_MAP_IE(ServiceFlow *sf, double *accumulated_offset,
                                             long *granted_data_in_next_ul_subframe,
                                             double compensation)
{
    // security check
    if (sf->state == SF_ACTIVE)
    {                           //|| cid == 0x0000 | cid == 0xFFFF ) {  mk: müsste von den externen methoden abgefangen werden, da die hier nur die nicht-ranging und nicht-request IEs baut
        int cid = sf->CID;      //this is the actual CID of the ServiceFlow

        Ieee80216UL_MAP_IE *ul_map_ie = new Ieee80216UL_MAP_IE("UL_MAP_IE");

        structMobilestationInfo *struct_msinfo = lookupLocaleMobilestationListCID(cid);
        if (!struct_msinfo)
        {
            error("No corresponding Basic-CID found for CID: " + cid);
        }

        int basic_cid_for_active_sf = struct_msinfo->Basic_CID;
        ul_map_ie->setCID(basic_cid_for_active_sf);

        ul_map_ie->setRealCID(cid);

        // Management Connection (UIUC = 2 = REQ region full ,standard)
        // TODO anpassen: der UIUC 2 ist für management connections, andere codes
        // für die normalen burst intervalle (ich hatte die lookup-fkt. erweitert, deswegen)
        if (struct_msinfo != NULL)
        {
            ul_map_ie->setUiuc(2);
        }
        else
        {
            // TODO UIUC auf BurstProfile setzen
        }

        /**
         * Calculates 48bit for management messages and an equally over a second
         * distributed amount of bits according to the given needed bitrate
         */
        int bits_per_ulmap = 0;

        if (cid == struct_msinfo->Basic_CID || cid
            == struct_msinfo->Primary_Management_CID || cid
            == struct_msinfo->Secondary_Management_CID)
        {
            bits_per_ulmap = 48;
        }
        else
        {
            // calculate the bits per ul-burst for granted traffic rate
            sf_QoSParamSet *active_params = sf->active_parameters;
            bits_per_ulmap =
                active_params->granted_traffic_rate * compensation *
                localBasestationInfo.DLMAP_interval;

            // add space for GenericMacHeaders for the number of packets arriving for each
            // ul-burst according to the calculated bitrate
            bits_per_ulmap +=
                48 * (localBasestationInfo.DLMAP_interval / active_params->packetInterval);

            if (localBasestationInfo.enablePacking)
                bits_per_ulmap += 16;

            EV << "bits per ul-map interval, CID=" << cid << ": " << bits_per_ulmap << " (="
                << active_params->min_reserved_traffic_rate << "*"
                << localBasestationInfo.DLMAP_interval << "+48)\n";
        }

        *granted_data_in_next_ul_subframe += bits_per_ulmap;

        /**
         * These two offsets are also used to calculate the number of bits reserved for
         * this uplink burst.
         */
        ul_map_ie->setStartTime_Offset((localBasestationInfo.Ranging_request_opportunity_size +
                                        localBasestationInfo.Bandwidth_request_opportunity_size) *
                                        48 / 4E6 + *accumulated_offset);

        *accumulated_offset += bits_per_ulmap / 4E6;
        ul_map_ie->setStopTime_Offset((localBasestationInfo.Ranging_request_opportunity_size +
                                       localBasestationInfo.Bandwidth_request_opportunity_size) *
                                       48 / 4E6 + *accumulated_offset);

        /**
         * The uplink subframe cannot take longer than the whole frame.
         * Building of the current uplink grant is cancelled and the next
         * one is tried out. Maybe it contains less granted data...
         *
         * TODO: implementation of fairness in combination with the scheduler
         */
        EV << "next_frame_start: " << next_frame_start << "   frame_stop: " << next_frame_start +
            localBasestationInfo.DLMAP_interval << "   offset: " << ul_map_ie->getStopTime_Offset();

        if (ul_map_ie->getStopTime_Offset() >
            localBasestationInfo.DLMAP_interval * localBasestationInfo.downlink_to_uplink)
        {
            //breakpoint("grants exceed uplink subframe");
            EV << "(!!!)\n";
            *granted_data_in_next_ul_subframe -= bits_per_ulmap;

            // use waiting queue only if activated
            // if disabled, grants are created by strict priority of the ServiceFlows
            if (localBasestationInfo.useULGrantWaitingQueue)
                map_waitingGrants[ul_map_ie->getRealCID()] = ul_map_ie->getCID();

            cvec_grant_capacity[sf->traffic_type].record(0);

            delete ul_map_ie;
        }

        /**
         * If the grant fits in the next uplink frame, put it there!
         */
        else
        {
            EV << "\n";
            /**
             * The duration is set according to standard and can later be used on the MS side
             * to schedule the stopControlRequest instead of saving extra information in the
             * offset variables
             */
            ul_map_ie->setDuration(ul_map_ie->getStopTime_Offset() -
                                   ul_map_ie->getStartTime_Offset());

            // number of bits for this uplink burst
            ul_map_ie->setBits_for_burst(bits_per_ulmap);

            ul_map_ie->setBitlength(ul_map_ie->getBitlength());

            EV << "Pushing back new UL_MAP-IE in list (CID: " << cid << ")\n";
            ul_map_ie_List.push_back(*ul_map_ie);

            map_servedGrants[ul_map_ie->getRealCID()] = ul_map_ie->getCID();

            cvec_grant_capacity[sf->traffic_type].record(bits_per_ulmap);

            delete ul_map_ie;
        }
    }
    else
    {
        EV << "Cannot allocate resources for CID=" << sf->CID << ", ServiceFlow not active\n";
    }
}

/**
 * This method implements the uplink grant scheduler for the basestation.
 * It can be configured as follows:
 *              1) If scheduling is set to be done for one frame only ("useULGrantWaitingQueue" parameter set to "false"),
 *              uplink is granted by descending priority of the active ServiceFlows. First, as many UGS flows are served
 *              as possible, then rtPS ... down to BE.
 *
 *              2) If "useULGrantWaitingQueue" parameter is set to "true", grants that do not fit in the current
 *              uplink subframe are marked to be served first during the next uplink subframe. This way a certain fairness
 *              between differently prioritized flows is achieved, but in order to guarantee throughput for all
 *              high priority flows, further implementation needs to be done.
 */
void ControlPlaneBasestation::buildScheduledIEs()
{
    EV << "(in  ControlPlaneBasestation::buildScheduledIEs)" << endl;
    if (map_serviceFlows->size() > 0)
    {
        double accumulated_offset = 0;
        long granted_data_in_next_ul_subframe = 0;

        EV << "\n#ServiceFlows in Map: " << map_serviceFlows->size() << "\n";

        map_servedGrants.clear();

        sortServiceFlowsByPriority();

        /**
         * Handle the waiting grants from the former frame, first.
         * This is done only if "useULGrantWaitingQueue" is set to true in qos_parameters.ini
         * because only then the queue is filled with potentially dismissed ServiceFlows.
         *
         * A switch can turn on/off double datarate for compensation of lost grants.
         */
        if (map_waitingGrants.size() > 0)
        {
            map<int, int>::iterator waiting_it;
            for (waiting_it = map_waitingGrants.begin(); waiting_it != map_waitingGrants.end();
                 waiting_it++)
            {
                int sfid = map_connections->find(waiting_it->first)->second;
                ServiceFlow *waiting_sf = &(map_serviceFlows->find(sfid)->second);

                if (waiting_sf->state == SF_ACTIVE &&
                    (waiting_sf->link_type == ldUPLINK || waiting_sf->link_type == ldMANAGEMENT))
                {
                    EV << "buildScheduledIEs(): Building UL_MAP-IE for waiting CID: "
                        << waiting_sf->CID << "(" <<
                        lookupLocaleMobilestationListCID(waiting_sf->CID)->Primary_Management_CID << ")\n";

                    buildUL_MAP_IE(waiting_sf, &accumulated_offset,
                                   &granted_data_in_next_ul_subframe,
                                   localBasestationInfo.grant_compensation);

                    map_waitingGrants.erase(waiting_it);
                }
            }
        }

        /**
         * Then try to serve the other ServiceFlows
         */
        for (short prio = MANAGEMENT; prio <= BE; prio++)
        {
            while (serviceFlowsSortedByPriority[prio].size() > 0)
            {
                ServiceFlow *cur_sf = serviceFlowsSortedByPriority[prio].front();
                if (cur_sf->state == SF_ACTIVE &&
                    map_servedGrants.find(cur_sf->CID) == map_servedGrants.end() &&
                    (cur_sf->link_type == ldUPLINK || cur_sf->link_type == ldMANAGEMENT))
                {
                    EV << "buildScheduledIEs(): Building UL_MAP-IE for CID: " << cur_sf->CID << "(" <<
                        lookupLocaleMobilestationListCID(cur_sf->CID)->Primary_Management_CID << ")\n";

                    buildUL_MAP_IE(cur_sf, &accumulated_offset, &granted_data_in_next_ul_subframe, 1);
                }

                serviceFlowsSortedByPriority[prio].pop_front();
            }
        }

//      ServiceFlowMap::iterator sf_it;
//      for (sf_it=map_serviceFlows->begin(); sf_it!=map_serviceFlows->end(); sf_it++)
//      {
//          ServiceFlow *cur_sf = &(sf_it->second);
//
//          if ( cur_sf->state == SF_ACTIVE &&
//              map_servedGrants.find( cur_sf->CID ) == map_servedGrants.end() &&
//              (cur_sf->link_type == ldUPLINK || cur_sf->link_type == ldMANAGEMENT ))
//          {
//              EV << "buildScheduledIEs(): Building UL_MAP-IE for CID: "
//                  << cur_sf->CID << "("<< lookupLocaleMobilestationListCID(cur_sf->CID)->Primary_Management_CID <<")\n";
//
//              buildUL_MAP_IE(cur_sf, &accumulated_offset, &granted_data_in_next_ul_subframe );
//          }
//      }
        EV << "Granted data in next UL subframe: " << granted_data_in_next_ul_subframe <<
            "  Duration: " << accumulated_offset << "\n";
    }
}

/**
* Die Funktion erzeugt die DCD(Downlink Channel Description) Management Nachricht.
* Diese Nachricht enthaelt Informationen ueber den Downlink Kanal
*
**/
/** @brief "Kapitel 5.2.4" Kanal Deskriptor Nachrichten */
void ControlPlaneBasestation::buildDCD()
{
    /** General MAC-Header Informationen
     */
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;

    /** Erstelle Downlink Channel Descriotion Nachricht
     */
    Ieee80216_DCD *ManagementMessage = new Ieee80216_DCD("DCD");
    ManagementMessage->setTYPE(Type);

    /** Nachrichtenspecifische Informationen
     */
    ManagementMessage->setBS_ID(localBasestationInfo.BasestationID); //Setzt die BS_ID (MAC-Adresse)

    /** Sende Nachricht zum Transciever Module
     */
    EV << "Sende " << ManagementMessage->getName() << " Nachricht. Die Simulationszeit:" << simTime() << " .\n";

    //burst.byteLength = burst.byteLength +6;
    sendtoLowerLayer(ManagementMessage);
}

/**
* Die funktion erstellt die UCD(Uplink Channel Description) Management Nachricht.
* Diese Nachricht enthaelt Informationen ueber den Uplink Kanal
*
**/
/** @brief "Kapitel 5.2.4" Kanal Deskriptor Nachrichten */
void ControlPlaneBasestation::buildUCD()
{
    /** General MAC-Header Informationen
     */
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;

    /** Erstelle Uplink Channel Descriotion Nachricht
     */
    Ieee80216_UCD *ManagementMessage = new Ieee80216_UCD("UCD");
    ManagementMessage->setTYPE(Type);

    /** Nachrichtenspecifische Informationen
     */
    ManagementMessage->setBS_ID(localBasestationInfo.BasestationID); //Setzt die BS_ID (MAC-Adresse)
    ManagementMessage->setUploadChannel(localBasestationInfo.UplinkChannel); //Infomiert die Mobilstaton welcher Uplinkkanal verwendet wird
    /**Informiert die Mobilstation über die Anzahl der Ranging Slots **/
    ManagementMessage->setRanging_request_opportunity_size(localBasestationInfo.
                                                           Ranging_request_opportunity_size);
    /**Informiert die Mobilstation über die Anzahl der Bandwith Request Slots **/
    ManagementMessage->setBandwidth_request_opportunity_size(localBasestationInfo.
                                                             Bandwidth_request_opportunity_size);
    //ManagementMessage->setLength(48);
    //burst.byteLength = burst.byteLength +6;

    /** Sende Nachricht zum Transciever Module **/
    EV << "Sende " << ManagementMessage->getName() << " Nachricht. Die Simulationszeit:" << simTime() << " .\n";
    sendtoLowerLayer(ManagementMessage);
}

/**
* Die DIe Funktion erzeugt die RNG-RSP Management Nachricht.
* Mit dieser Nachricht anwortet die Basisstation auf die RNG-REQ Nachricht der Mobilstation
*
**/
/** @brief "Kapitel 5.2.4" Ranging Request */
void ControlPlaneBasestation::buildRNG_RSP(MACAddress receiveMobileStation)
{
    /** General MAC-Header Informationen
     */
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;

    /** Erstelle Ranging Response Nachricht
     */
    Ieee80216_RNG_RSP *ManagementMessage = new Ieee80216_RNG_RSP("RNG_RSP");
    ManagementMessage->setTYPE(Type);
    ManagementMessage->setByteLength(48);

    /** Nachrichtenspecifische Informationen
     */
    ManagementMessage->setMSS_MAC_Address(receiveMobileStation); //Setzt die MAC-Adresse der anfragenden Mobilstation
    // (mk)
    structMobilestationInfo *ss = lookupLocaleMobilestationListMAC(receiveMobileStation);
    ManagementMessage->setBasic_CID(ss->Basic_CID); //Uebertraegt die Basic CID
    ManagementMessage->setPrimary_Management_CID(ss->Primary_Management_CID); // Uebertraegt die Primayry CID

    //activate the basic cid for further requests
    getServiceFlowForCID(ss->Basic_CID)->state = SF_ACTIVE;

    /** Sende Nachricht zum Transciever Module */
    EV << "Sende " << ManagementMessage->getName() << " Nachricht. Die Simulationszeit:" << simTime() << " .\n";
    sendtoLowerLayer(ManagementMessage);
}

/**
 / Die Funktion erzeugt die RNG-RSP Management Nachricht.
 / Mit dieser Nachricht anwortet die Basisstation auf die RNG-REQ Nachricht der Mobilstation
 /
 */
/** @brief "Kapitel 5.2.4" SS Basic Capability Response */
void ControlPlaneBasestation::buildSBC_RSP(int basic_cid)
{
    Ieee80216_SBC_RSP *frame = new Ieee80216_SBC_RSP("SBC_RSP");

    /** General MAC-Header Informationen */
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    //frame->setLength(48);

    // (mk)
    // send the response to the same CID that came with the request
    // --> CID<->SubscriberStation is unique
    frame->setCID(basic_cid);   //Übertraegt die Nahricht mit der Basic CID

    getServiceFlowForCID(lookupLocaleMobilestationListCID(basic_cid)->Primary_Management_CID)->state = SF_ACTIVE;

    EV << "Sending SBC_RSP" << endl;
    sendtoLowerLayer(frame);
}

/**
* Die Funktion erzeugt die RNG-RSP Management Nachricht.
* Mit dieser Nachricht anwortet die Basisstation auf die RNG-REQ Nachricht der Mobilstation
*
**/
/** @brief "Kapitel 5.2.4" Registration Response */
void ControlPlaneBasestation::buildREG_RSP(int primary_cid)
{
    Ieee80216_REG_RSP *frame = new Ieee80216_REG_RSP("REG_RSP");

    /** General MAC-Header Informationen
     */
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    //frame->setLength(48);//TODO Wichtig dynamische Länge aus MAC Header erstelelen
    //    vector<ServiceFlow> *provisionedSFs = parentModule()->submodule("cp_serviceflows").vec_serviceFlows;
    //    frame->setProvisionedServiceFlows( provisionedSFs );

    // (mk)
    frame->setCID(primary_cid);
    cpsSF_BS->createManagementConnection(lookupLocaleMobilestationListCID(primary_cid), SECONDARY);
    frame->setSecondary_Management_CID(lookupLocaleMobilestationListCID(primary_cid)->
                                       Secondary_Management_CID);

    // de-activate the Basic connection used during Registration
    getServiceFlowForCID(lookupLocaleMobilestationListCID(primary_cid)->Basic_CID)->state =
        SF_MANAGEMENT;
    //getServiceFlowForCID( lookupLocaleMobilestationListCID(primary_cid)->Basic_CID )->link_type = ldMANAGEMENT;

    // cps_scheduling->setNumberOfConnectedStations( localeMobilestationList.size() );

    EV << "Sending REG_RSP" << endl;
    sendtoLowerLayer(frame);

    // new: generate DSA-REQs for all currently active downlink ServiceFlows for the
    //   newly registered station
    //provideStationWithServiceFlows( primary_cid );
}

/**
* Die Funktion erzeugt die RNG-RSP Management Nachricht.
* Mit dieser Nachricht anwortet die Basisstation auf die RNG-REQ Nachricht der Mobilstation
*
*/
/** @brief "Kapitel 5.2.4"  */
void ControlPlaneBasestation::build_MOB_SCN_RSP(int cid)
{
    Ieee80216_MOB_SCN_RSP *frame = new Ieee80216_MOB_SCN_RSP("MOB_SCN-RSP");

    /** General MAC-Header Informationen
    */
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);

    structMobilestationInfo *scanMobilestation;
    scanMobilestation = lookupLocaleMobilestationListCID(cid);
    //scanMobilestation->scanModus = true;
    scanMobilestation->strartFrame = localBasestationInfo.strartFrame;
    scanMobilestation->interleavingInterval = localBasestationInfo.interleavingInterval;
    scanMobilestation->scanDuration = localBasestationInfo.scanDuration;
    scanMobilestation->scanIteration = localBasestationInfo.scanIteration;

    /** Nachrichtenspecifische Informationen
    */
    //localBasestationInfo.strartFrame = 2; // FIXME: in omnetpp.ini übernehmen
    frame->setStrartFrame(localBasestationInfo.strartFrame); // Mobilstation beginnt nach X Frame mit dem Abtasten
    frame->setScanDuration(localBasestationInfo.scanDuration); // Mobilstation tastet fuer X Frame den Kanal ab
    frame->setInterleavingInterval(localBasestationInfo.interleavingInterval); // Mobilstation kommt fuer X Frame zur active BS zurueck
    frame->setScanIteration(localBasestationInfo.scanIteration); // Wiederholt X mal das Abtasten
    // (mk)
    frame->setCID(cid);

    Ieee80216Prim_ScanMS *MStoScanmodusTimer = new Ieee80216Prim_ScanMS("setMStoScanmodusTimer");
    MStoScanmodusTimer->setScanMobilestationAddress(scanMobilestation->MobileMacAddress);

    scheduleAt(simTime() + localBasestationInfo.strartFrame * localBasestationInfo.DLMAP_interval,
               MStoScanmodusTimer);

    EV << "Sending MOB_SCN-RSP" << endl;
    EV << "Simulationtime" << simTime() << endl;
    EV << "Start Time" << simTime() +
        localBasestationInfo.strartFrame * localBasestationInfo.DLMAP_interval << endl;

    //breakpoint("Send SCN-RSP");

    sendtoLowerLayer(frame);
}

/**
* Die Funktion erzeugt die MOB_BSHO-RSP Management Nachricht.
* Mit dieser Nachricht anwortet die Basisstation auf die MOB_BSHO-REQ Nachricht der Mobilstation
*
*/
void ControlPlaneBasestation::build_MOB_BSHO_RSP(int CID)
{
    Ieee80216_BSHO_RSP *frame = new Ieee80216_BSHO_RSP("MOB_BSHO-RSP");

    /** General MAC-Header Informationen
    */
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);

    /** Nachrichtenspecifische Informationen */
    // (mk)
    frame->setCID(CID);
    EV << "Sending MOB_BSHO-RSP" << endl;
    sendtoLowerLayer(frame);
}

/**
* @brief Funktionen mit denen empfangene Management-Nachrichten bearbeitet werden
*/

/**
* Die Funktion bearbeitet die RNG-RSP Management Nachricht einer MS.
*
*/
void ControlPlaneBasestation::handle_RNG_REQ_Frame(Ieee80216_RNG_REQ *frame)
{
    EV << "(in ControlPlaneBasestation::handle_RNG_REQ_Frame) RNG_REQ arrived.\n";

    // if the station is not yet in the connected stations list, add it
    if (!lookupLocaleMobilestationListMAC(frame->getMSS_MAC_Address()))
    {
        storeMSSInfo(frame->getMSS_MAC_Address()); //(mk)info: saves the MAC address for the SS
    }
    // else the station tries to register
    else
    {
        EV << "STATION ALREADY REGISTERED" << "\n";

        structMobilestationInfo *station = NULL;
        MobileSubscriberStationList::iterator hit_it;

        MobileSubscriberStationList::iterator it;
        for (it = localeMobilestationList.begin(); it != localeMobilestationList.end(); ++it)
        {

            if (it->MobileMacAddress == frame->getMSS_MAC_Address())
            {
                EV << frame->getMSS_MAC_Address() << " found in SubscriberList\n";
                station = &(*it);
                hit_it = it;
            }
        }

        if (station != NULL)
        {
            ConnectionMap::iterator cid_it;
            for (cid_it = station->map_own_connections.begin();
                 cid_it != station->map_own_connections.end(); cid_it++)
            {
                int cid = cid_it->first;

                ConnectionMap::iterator c_it = map_connections->find(cid);
                if (c_it != map_connections->end())
                {
                    int sfid = c_it->second;

                    ServiceFlowMap::iterator sf_it = map_serviceFlows->find(sfid);
                    if (sf_it != map_serviceFlows->end())
                    {
                        map_serviceFlows->erase(sf_it);
                    }
                    map_connections->erase(c_it);
                }
                station->map_own_connections.erase(cid_it);
            }
        }
        //localeMobilestationList.erase( hit_it );
    }

    /**
     * (mk)
     * A new SS is connecting to this BS.
     * Next, we create the Basic and Primary MAC connections and their ServiceFlows.
     * The IDs and the ServiceFlow are directly put into "map_connections" and "map_serviceFlow".
     */
    EV << "Creating Basic and Primary Management connections to SS\n";
    structMobilestationInfo *cur_station =
        lookupLocaleMobilestationListMAC(frame->getMSS_MAC_Address());

    cpsSF_BS->createManagementConnection(cur_station, BASIC); //Type Management Nachrichten A (Netzeintritt, Handover, Scanning)
    cpsSF_BS->createManagementConnection(cur_station, PRIMARY); //Type Management Nachrichten B (z.B.: QoS Nachrichten)

// wird schon bei createManagementConnection gemacht...
// getServiceFlowForCID( cur_station->Basic_CID )->link_type = ldUPLINK;
// getServiceFlowForCID( cur_station->Primary_Management_CID )->link_type = ldUPLINK;

    // (mk) TODO: Aktivierung muss noch richtig gemacht werden da:
    //     Zuweisung in der UL-MAP = Ressourcen reserviert = Verbindung aktiv
    //getServiceFlowForCID( cur_station->Basic_CID )->state = SF_ACTIVE;
    //getServiceFlowForCID( cur_station->Primary_Management_CID )->state = SF_ACTIVE;

    // wird zentral beim bauen der UL-MAP gemacht
    //buildUL_MAP_IE( cur_station->Basic_CID, 48 );
    //buildUL_MAP_IE( cur_station->Primary_Management_CID, 48 );

    buildRNG_RSP(frame->getMSS_MAC_Address());
}

void ControlPlaneBasestation::handle_SBC_REQ_Frame(Ieee80216_SBC_REQ *frame)
{
    EV << "(in  ControlPlaneBasestation::handle_SBC_REQ_Frame) SBC_REQ arrived.\n";
    buildSBC_RSP(frame->getCID());
}

void ControlPlaneBasestation::handle_REG_REQ_Frame(Ieee80216_REG_REQ *frame)
{
    EV << "(in  ControlPlaneBasestation::handle_REG_REQ_Frame) REG_REQ arrived.\n";
    buildREG_RSP(frame->getCID());
}

void ControlPlaneBasestation::handle_MOB_SCN_REQ_Frame(Ieee80216_MOB_SCN_REQ *frame)
{
    EV << "(in  ControlPlaneBasestation::handle_MOB_SCN_REQ_Frame) MOB_SCN-REQ arrived.\n";
    //get scan parameter from mobilestation
    localBasestationInfo.scanDuration = frame->getScanDuration();
    localBasestationInfo.interleavingInterval = frame->getInterleavingInterval();
    localBasestationInfo.scanIteration = frame->getScanIteration();
    EV << "scanDuration" << localBasestationInfo.scanDuration << endl;
    //breakpoint("BS recieve MOB_SCN_REQ");
    if (localBasestationInfo.scanDuration >= 1)
    {
        build_MOB_SCN_RSP(frame->getCID());
    }
}

void ControlPlaneBasestation::handle_MOB_MSHO_REQ_Frame(Ieee80216_MSHO_REQ *frame)
{
    EV << "(in  ControlPlaneBasestation::handle_MOB_MSHO_REQ_Frame) MOB_MSHO-REQ arrived.\n";
    build_MOB_BSHO_RSP(frame->getCID());
    //breakpoint("BS-MSHO-REQ");
}

void ControlPlaneBasestation::handle_MOB_HO_IND_Frame(Ieee80216_MOB_HO_IND *frame)
{
    EV <<
        "(in  ControlPlaneBasestation::handle_MOB_HO_IND_Frame) MOB_HO-IND arrived, removing MS from system.\n";

    int lost_station_cid = frame->getCID();

    structMobilestationInfo *station = NULL;
    MobileSubscriberStationList::iterator hit_it;

    MobileSubscriberStationList::iterator it;
    for (it = localeMobilestationList.begin(); it != localeMobilestationList.end(); it++)
    {
        if (it->Basic_CID == lost_station_cid || it->Primary_Management_CID == lost_station_cid
            || it->Secondary_Management_CID == lost_station_cid
            || it->map_own_connections.find(lost_station_cid) != it->map_own_connections.end())
        {
            station = &(*it);
            hit_it = it;
        }
    }

    if (station != NULL)
    {
        ConnectionMap::iterator cid_it;
        for (cid_it = station->map_own_connections.begin();
             cid_it != station->map_own_connections.end(); cid_it++)
        {
            int cid = cid_it->first;

            ConnectionMap::iterator c_it = map_connections->find(cid);
            if (c_it != map_connections->end())
            {
                int sfid = c_it->second;

                ServiceFlowMap::iterator sf_it = map_serviceFlows->find(sfid);
                if (sf_it != map_serviceFlows->end())
                {
                    map_serviceFlows->erase(sf_it);
                }
                map_connections->erase(c_it);
            }
            station->map_own_connections.erase(cid_it);
        }

    }
    localeMobilestationList.erase(hit_it);
}

/**
* @brief Funktionen fuer die Bearbeitung der localeMobilestationList
*****************************************************************************************/

void ControlPlaneBasestation::clearMSSList()
{
    EV << "(in  ControlPlaneBasestation::clearMSSList) " << endl;
    MobileSubscriberStationList::iterator it;

    for (it = localeMobilestationList.begin(); it != localeMobilestationList.end(); ++it)
        if (it->authTimeoutMsg)
            delete cancelEvent(it->authTimeoutMsg);
    localeMobilestationList.clear();
}

void ControlPlaneBasestation::storeMSSInfo(const MACAddress &Address) //create new Base Station ID in BSInfo list
{
    EV << "(in  ControlPlaneBasestation::storeMSSInfo) " << endl;
    structMobilestationInfo *mss = lookupLocaleMobilestationListMAC(Address);
    if (mss != NULL)
    {
        EV << "MAC Address=" << Address << " already in our MSS list.\n";
    }
    else
    {
        EV << "Inserting MAC Address " << Address << " into our MSS list\n";
        structMobilestationInfo *ms = new structMobilestationInfo();
        ms->MobileMacAddress = Address;
        ms->multiplicator = localeMobilestationList.size() + 1;
        ms->time_offset = ms->multiplicator * 0.001;
        localeMobilestationList.push_back(*ms);
    }
}

// TODO effizienter: auf Map umstellen... (mk)
structMobilestationInfo *ControlPlaneBasestation::lookupLocaleMobilestationListMAC(
    const MACAddress &Address)
{
    //EV << "mssList enthält " << localeMobilestationList.size()
    //  << " Mobilstationen\n";

    MobileSubscriberStationList::iterator it;
    for (it = localeMobilestationList.begin(); it != localeMobilestationList.end(); ++it)
    {

        if (it->MobileMacAddress == Address)
        {
            EV << Address << " found in SubscriberList\n";
            return &(*it);
        }
    }
    return NULL;
}

structMobilestationInfo *ControlPlaneBasestation::lookupLocaleMobilestationListCID(int cid)
{
    EV << "mssList enthält " << localeMobilestationList.size() << " Mobilstationen\n";

    MobileSubscriberStationList::iterator it;
    for (it = localeMobilestationList.begin(); it != localeMobilestationList.end(); it++)
    {
        if (it->Basic_CID == cid || it->Primary_Management_CID == cid
            || it->Secondary_Management_CID == cid
            || it->map_own_connections.find(cid) != it->map_own_connections.end())
        {
            return &(*it);
        }
    }
    EV << "MobileStation not found!\n";
    return NULL;
}

/**
 *****************************************************************************************/

/**
 * @brief Funktionen fuer die Parameterbergabe (Kanal,Bitrate) an die Radiomodule
 *****************************************************************************************/
void ControlPlaneBasestation::setRadio()
{
    ev << "MK:  CHECK(setRadio)\n";
    setRadioDownlink();
    setRadioUplink();
}

void ControlPlaneBasestation::setRadioDownlink()
{
    EV << "Setze Downlink Kanal:" << localBasestationInfo.DownlinkChannel << endl;
    PhyControlInfo *phyCtrl = new PhyControlInfo();
    phyCtrl->setChannelNumber(localBasestationInfo.DownlinkChannel);
    phyCtrl->setBitrate(localBasestationInfo.radioDatenrate);
    cMessage *msg = new cMessage("changeChannel", PHY_C_CONFIGURERADIO);
    msg->setControlInfo(phyCtrl);
    sendtoLowerLayer(msg);
}

void ControlPlaneBasestation::setRadioUplink()
{
    EV << "Setze Uplink Kanal:" << localBasestationInfo.UplinkChannel << endl;
    PhyControlInfo *phyCtrl = new PhyControlInfo();
    phyCtrl->setChannelNumber(localBasestationInfo.UplinkChannel);
    phyCtrl->setBitrate(localBasestationInfo.radioDatenrate);
    cMessage *msg = new cMessage("changeChannel", PHY_C_CONFIGURERADIO);
    msg->setControlInfo(phyCtrl);
    sendtoHigherLayer(msg);
}

/**
 *****************************************************************************************/

void ControlPlaneBasestation::startControlBurst()
{
    EV << "(in  ControlPlaneBasestation::startControlBurst) " << endl;
    Ieee80216Prim_sendControlRequest *sendControlPDU = new Ieee80216Prim_sendControlRequest();
    sendControlPDU->setPduCID(0xFFFF);
    sendControlPDU->setRealCID(0xFFFF);
    sendRequest(sendControlPDU);
}

void ControlPlaneBasestation::stopControlBurst()
{
    EV << "(in  ControlPlaneBasestation::stopControlBurst) " << endl;
    Ieee80216Prim_stopControlRequest *stopControlPDU = new Ieee80216Prim_stopControlRequest();
    stopControlPDU->setPduCID(0xFFFF);
    sendRequest(stopControlPDU);
}

/**
 * Computes the duration of the transmission of a frame over the
 * physical channel. 'bits' should be the total length of the MAC
 * packet in bits.
 */
double ControlPlaneBasestation::computePacketDuration(int bits)
{
    EV << "(in  ControlPlaneBasestation::computePacketDuration) " << endl;
    // FIXME: bitrate vom Radio-Modul holen!!!
    bitrate = 4E+6;
    return bits / bitrate;
}

bool ControlPlaneBasestation::cidIsInConnectionMap(int cid)
{
    map<int,int>::iterator connection_it = map_connections->find(cid);

    return (connection_it != map_connections->end());
}

// (mk)
// copy/paste from 802.11Mac
void ControlPlaneBasestation::registerInterface()
{
    EV << "(in  ControlPlaneBasestation::registerInterface) " << endl;
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (!ift)
        return;

    char *interfaceName = new char[strlen(getParentModule()->getFullName()) + 1];

    // interface name: NetworkInterface module's name without special characters ([])
    char *d = interfaceName;
    for (const char *s = getParentModule()->getFullName(); *s; s++)
    {
        if (isalnum(*s))
            *d++ = *s;
    }
    *d = '\0';

    // FIXME necessary to check this?
    // Check if the interface is yet register
    if (ift->getInterfaceByName(interfaceName) != NULL)
    {
        delete [] interfaceName;
        return;
    }

    InterfaceEntry *e = new InterfaceEntry(this);

    e->setName(interfaceName);
    delete [] interfaceName;

    // address
    MACAddress mac_add = localBasestationInfo.BasestationID;
    e->setMACAddress(mac_add);
    e->setInterfaceToken(mac_add.formInterfaceIdentifier());

    // FIXME: MTU on 802.11 = ?
    e->setMtu(1500);

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    // add
    ift->addInterface(e);
}

void ControlPlaneBasestation::setMobilestationScanmodus(cMessage *msg)
{
    EV << "(in  ControlPlaneBasestation::setMobilestationScanmodus) " << endl;
    Ieee80216Prim_ScanMS *scanMS = check_and_cast<Ieee80216Prim_ScanMS *>(msg);

    structMobilestationInfo *scanMobilestation;
    scanMobilestation = lookupLocaleMobilestationListMAC(scanMS->getScanMobilestationAddress());
    scanMobilestation->scanModus = true;

    ev << "Mobilestation MAC Adresse:" << scanMS->getScanMobilestationAddress() << "\n";
    ev << "Mobilestation MAC Adresse:" << scanMobilestation->MobileMacAddress << "\n";

    delete scanMS;

    //breakpoint("SetScanmodus");
}

station_type ControlPlaneBasestation::getStationType()
{
    Enter_Method_Silent();
    return BASESTATION;
}

void ControlPlaneBasestation::setDataMsgQueue(Ieee80216MacHeaderFrameList *data_queue)
{
    Enter_Method_Silent();

    tx_dataMsgQueue = data_queue;
}

void ControlPlaneBasestation::sortServiceFlowsByPriority()
{
    serviceFlowsSortedByPriority[MANAGEMENT].clear();
    serviceFlowsSortedByPriority[UGS].clear();
    serviceFlowsSortedByPriority[RTPS].clear();
    serviceFlowsSortedByPriority[ERTPS].clear();
    serviceFlowsSortedByPriority[NRTPS].clear();
    serviceFlowsSortedByPriority[BE].clear();

    if (map_serviceFlows->size() > 0)
    {
        ServiceFlowMap::iterator it;
        for (it = map_serviceFlows->begin(); it != map_serviceFlows->end(); it++)
        {
            ServiceFlow *sf = &(it->second);
            serviceFlowsSortedByPriority[sf->traffic_type].push_back(sf);
        }
    }
}

void ControlPlaneBasestation::provideStationWithServiceFlows(int primary_cid)
{
    ServiceFlowMap::iterator it;
    it = map_serviceFlows->begin();

    std::map<ip_traffic_types, ip_traffic_types>needed_traffic_types;
    for (int i = UGS; i <= BE; i++)
    {
        needed_traffic_types[(ip_traffic_types) i] = (ip_traffic_types) i;
    }

    while (it != map_serviceFlows->end() || needed_traffic_types.size() > 0)
    {
        if (it->second.link_type == ldDOWNLINK &&
            needed_traffic_types.find(it->second.traffic_type) != needed_traffic_types.end())
        {

            cpsSF_BS->createAndSendNewDSA_REQ(primary_cid, new ServiceFlow(it->second),
                                              it->second.traffic_type);
            needed_traffic_types.erase(it->second.traffic_type);
        }
        it++;
    }
}

management_type ControlPlaneBasestation::getManagementType(int x)
{
	error("called ControlPlaneBasestation::getManagementType(%d)", x);
	return BASIC;	// TODO //FIXME it's was an empty function
}

// used only in mobilestation, but needed for casting purposes in transceiver...
int ControlPlaneBasestation::getBasicCID()
{
    return -1;
};

int ControlPlaneBasestation::getPrimaryCID()
{
    return -1;
};

/**
* Funktion um Werte auf der Simulationumgebung auszugeben.
*
+++++++++++++*********************************************/
void ControlPlaneBasestation::updateDisplay()
{
    char buf[90];
    sprintf(buf, "Available Downlink/s: %d \nAvailable Uplink/s: %d",
            (int) (localBasestationInfo.downlink_per_second),
            (int) (localBasestationInfo.radioDatenrate - localBasestationInfo.downlink_per_second));
    //(int)( (localBasestationInfo.radioDatenrate - localBasestationInfo.downlink_per_second ) * localBasestationInfo.DLMAP_interval) );

    getDisplayString().setTagArg("t", 0, buf);
    getDisplayString().setTagArg("t", 1, "t");
}
