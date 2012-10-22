//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//

#include"ControlPlaneMobilestation.h"

#include "SnrList.h"
#include "ModulationsConsts.h"
#include "Ieee80216ManagementMessages_m.h"

#include <math.h>

Define_Module(ControlPlaneMobilestation);

//static std::ostream& operator<< (std::ostream& out, cMessage *msg)
//{
//    out << "(" << msg->className() << ")" << msg->fullName();
//    return out;
//}

ControlPlaneMobilestation::ControlPlaneMobilestation()
{
//Netentry timer
    ScanChannelTimer = NULL;
    DLMAPTimer = NULL;
    RegRspTimeOut = NULL;
    RngRspTimeOut = NULL;
    RngRegFail = NULL;
    ScnRegFail = NULL;
    SBCRSPTimeOut = NULL;
    startRangingRequest = NULL;
    startScanBasestation = NULL;

//Transmit Timer
    sendPDU = NULL;
    stopPDU = NULL;
    sendControlPDU = NULL;
    stopControlPDU = NULL;
    sendSecondControlPDU = NULL;
    stopSecondControlPDU = NULL;

//Handover Timer
    scanDurationTimer = NULL;
    interleavingIntervalTimer = NULL;
    scanIterationTimer = NULL;
    startFrameTimer = NULL;
    startScanTimer = NULL;
    startHandoverScan = NULL;
    startHandover = NULL;
    MSHO_REQ_Fail = NULL;

//Ranging Variable
    isRangingIntervall = true;
    counterRangingUL_MAP_IE = 0;
    Initial_Ranging_Interval_Slot = 0;
    HandoverCounter = 0;
    ULMap_Counter = 0;
    Use_ULMap = 0;
    rangingVersuche = 0;
    ScanCounter = 0;
    ScanDlMapCounter = 0;
    //rangingVersuche = 0;
    //granted_bandwidth = 0;
    //msgPuffer=NULL;

    localScanIteration = 0;
    ScanIterationBool = false;
    makeHandoverBool = false;
    scnChannel = 0;
    makeHandover_cdTrafficBool = false;

    FrameAnzahl = 1;

    isPrimaryConnectionActive = false;
    last_floor = 0;             // helper for updating the display string for the uplink rate
    grants_per_second = 0;

    scan_request_sent = false;

    startHO_bool = false;
}

ControlPlaneMobilestation::~ControlPlaneMobilestation()
{
    cancelAndDelete(ScanChannelTimer); //Timer startet den Scanvorgang nach einer g�ltigen DL-Map
    cancelAndDelete(DLMAPTimer);
    cancelAndDelete(RegRspTimeOut);
    cancelAndDelete(RngRspTimeOut);
    cancelAndDelete(RngRegFail);
    cancelAndDelete(ScnRegFail);
    cancelAndDelete(SBCRSPTimeOut);
    cancelAndDelete(sendPDU);
    cancelAndDelete(stopPDU);
    cancelAndDelete(sendControlPDU);
    cancelAndDelete(stopControlPDU);
    cancelAndDelete(sendSecondControlPDU);
    cancelAndDelete(stopSecondControlPDU);
    cancelAndDelete(startRangingRequest);
    cancelAndDelete(startScanBasestation);
    cancelAndDelete(scanDurationTimer);
    cancelAndDelete(interleavingIntervalTimer);
    cancelAndDelete(scanIterationTimer);
    cancelAndDelete(startFrameTimer);
    cancelAndDelete(startScanTimer);
    cancelAndDelete(startHandoverScan);
    cancelAndDelete(startHandover);
    cancelAndDelete(MSHO_REQ_Fail);
}

void ControlPlaneMobilestation::initialize(int stage)
{
    ControlPlaneBase::initialize(stage);

    if (stage == 0)
    {
        /**
         * Uebernahmen von Parametern aus der INI-Datei
         *
         ************************************************************************************************************/
        const char *addressString = par("address"); //MAC Addresse automatisch oder mit vorgegebene Addresse zuweisen

        if (!strcmp(addressString, "auto"))
        {
            localMobilestationInfo.MobileMacAddress = MACAddress::generateAutoAddress(); // MAC Addresse automatisch zuweisen
            par("address").setStringValue(localMobilestationInfo.MobileMacAddress.str().c_str()); // module parameter von "auto" auf konkrete Addresse umstellen
        }
        else
        {
            localMobilestationInfo.MobileMacAddress.setAddress(addressString); // Vorgegebene MAC Addresse übernehmen
        }

        //  cPar *DlMapScanIntervall;
        //  DlMapScanIntervall = &par("scanintervall");//Intervall nach dem nach einer DL-Map gesucht wird
        //  localMobilestationInfo.ScanTimeInterval = DlMapScanIntervall->doubleValue();
        localMobilestationInfo.ScanTimeInterval = par("scanintervall");

        //  cPar *DlMapInterval;
        //  DlMapInterval = &par("DLMapInterval");
        //  localMobilestationInfo.DLMAP_interval = DlMapInterval->doubleValue();// festlegen der DL-MAP Intervall und länge des Donlink Rahmen
        //  //(mk)
        localMobilestationInfo.DLMAP_interval = par("DLMapInterval");

        numChannels = par("numChannels"); //Anzahl der verwendeten Kanaele
        /**
         * Initial-Ranging parameter
         **************************************************************/
        //  cPar *InitialRangingTimeOut;
        //  InitialRangingTimeOut = &par("InitialRangingTimeOut");
        //  localMobilestationInfo.RngRspTimeOut_B = InitialRangingTimeOut->doubleValue();
        localMobilestationInfo.RngRspTimeOut_B = par("InitialRangingTimeOut");

        //  cPar *InitialRangingFail;
        //  InitialRangingFail = &par("InitialRangingFail");
        //  localMobilestationInfo.RngRspFail = InitialRangingFail->doubleValue();
        localMobilestationInfo.RngRspFail = par("InitialRangingFail");

        //  cPar *rngIntervall;
        //  rngIntervall = &par("rangingintervall");
        //  localMobilestationInfo.RngRspTimeOut = rngIntervall->doubleValue();
        localMobilestationInfo.RngRspTimeOut = par("rangingintervall");
        localMobilestationInfo.RegRspTimeOut = par("registrationResponseTimeout");

        /**
         * Scanning parameter
         **************************************************************/
        //  cPar *scanDuration;
        //  scanDuration = &par("scanDuration");
        //  localMobilestationInfo.scanDuration = scanDuration->doubleValue();
        localMobilestationInfo.scanDuration = par("scanDuration");

        //  cPar *interleavingInterval;
        //  interleavingInterval = &par("interleavingInterval");
        //  localMobilestationInfo.interleavingInterval = interleavingInterval->doubleValue();
        localMobilestationInfo.interleavingInterval = par("interleavingInterval");

        //  cPar *scanIteration;
        //  scanIteration = &par("scanIteration");
        //  localMobilestationInfo.scanIteration = scanIteration->doubleValue();
        localMobilestationInfo.scanIteration = par("scanIteration");

        /**
         * Handover parameter
         **************************************************************/
        //  cPar *maxMargin;
        //  maxMargin = &par("maxMargin");
        //  localMobilestationInfo.maxMargin = maxMargin->doubleValue();
        localMobilestationInfo.maxMargin = par("maxMargin");

        //  cPar *minMargin;
        //  minMargin = &par("minMargin");
        //  localMobilestationInfo.minMargin = minMargin->doubleValue();
        localMobilestationInfo.minMargin = par("minMargin");

        //  cPar *Anzahl;
        //  Anzahl = &par("FrameAnzahl");
        //  FrameAnzahl = Anzahl->doubleValue();
        FrameAnzahl = par("FrameAnzahl");

        /**
         * Timer for Events
         *
         ************************************************************************************************************/
        ScanChannelTimer = new cMessage("ScanChannelTimer");
        DLMAPTimer = new cMessage("DLMAPTimer");
        startRangingRequest = new cMessage("startRangingRequest");
        ScnRegFail = new cMessage("ScnRegFail");
        startScanBasestation = new cMessage("startScanBasestation");

        /** Timer for Initial Ranging*/
        RngRspTimeOut = new cMessage("RngRspTimeOut");
        RegRspTimeOut = new cMessage("RegRspTimeOut");
        RngRegFail = new cMessage("RngRegFail");
        SBCRSPTimeOut = new cMessage("SBCRSPTimeOut");

        /** Timer for start and stop send*/
        sendPDU = new cMessage("sendPDU");
        stopPDU = new cMessage("stopPDU");
        sendControlPDU = new cMessage("sendControlPDU");
        stopControlPDU = new cMessage("stopControlPDU");
        sendSecondControlPDU = new cMessage("sendSecondControlPDU");
        stopSecondControlPDU = new cMessage("stopSecondControlPDU");

        /** Timer for scanmodus */
        scanDurationTimer = new cMessage("scanDurationTimer");
        interleavingIntervalTimer = new cMessage("interleavingIntervalTimer");
        scanIterationTimer = new cMessage("scanIterationTimer");
        startFrameTimer = new cMessage("startFrameTimer");
        startScanTimer = new cMessage("startFrameTimer");
        startHandoverScan = new cMessage("startHandoverScan");
        startHandover = new cMessage("startHandover");
        MSHO_REQ_Fail = new cMessage("MSHO-REQ-Fail");

        /**
         * Zugriff auf Funktionen weiterer Module aufbauen
         *
         ************************************************************************************************************/
        // get the pointer to the cpsServiceFlows module
        cModule *module = getParentModule()->getSubmodule("cp_serviceflows");
        cpsSF_MS = dynamic_cast<CommonPartSublayerServiceFlows_MS *>(module);

        // get the pointer to the scheduling module
        module =
            getParentModule()->getParentModule()->getSubmodule("msTransceiver")->
            getSubmodule("cpsTransceiver")->getSubmodule("scheduling");
        cps_scheduling = check_and_cast<CommonPartSublayerScheduling *>(module);

        // initialize the local connection and ServiceFlow maps
        map_connections = cpsSF_MS->getConnectionMap();
        EV << "connectionmap_size: " << map_connections->
            size() << " -> " << map_connections << "\n";
        map_serviceFlows = cpsSF_MS->getServiceFlowMap();

        //get the pointer to the common part sublayer
        cModule *module_cps_receiver =
            getParentModule()->getParentModule()->getSubmodule("msReceiver")->
            getSubmodule("cpsReceiver")->getSubmodule("cps_receiver");
        cps_Receiver_MS = dynamic_cast<CommonPartSublayerReceiver *>(module_cps_receiver);
        EV << "Verbindung zum Module " << module_cps_receiver->getName() << " wurde aufgebaut!\n";

        // initialize the local connection to the connection map
        if (cps_Receiver_MS)
        {
            cps_Receiver_MS->setConnectionMap(map_connections);
            EV << "Setze MAP in cps_Receiver_MS mit dem Wert: " << map_connections << ".\n";
        }

        clearBSList();          //Die List der gescanten Basisstationen
        isAssociated = false;

        // state variables
        fsm.setName("MS Control Plane State Machine");

        EV << "Initialisierung von ControlPlaneMS abgeschlossen, beginne mit dem Kanal-Scan.\n";
        scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval, ScanChannelTimer);

        // Timer for initial ServiceFlow creation after successfull connect
        //setupInitialMACConnection = new cMessage ("setupInitialMACConnection");
        //scheduleAt(simTime()+0.005, setupInitialMACConnection);

        receiverSnrVector.setName("receiverSnrVector");
        scanSnrVector.setName("scanSnrVector");
        gemitelterSnrVector.setName("gemitelterSnrVector");
        gemitelterScanSnrVector.setName("gemitelterScanSnrVector");
        abstandVector.setName("abstandVector");
        x_posVector.setName("x_posVector");
        y_posVector.setName("y_posVector");
        startTimerVec.setName("startTimer");
        stopTimerVec.setName("stopTimer");

        BS1SNR.setName("BS1SNR");
        BS2SNR.setName("BS2SNR");
        BS3SNR.setName("BS3SNR");
        BS4SNR.setName("BS4SNR");
        BS5SNR.setName("BS5SNR");
        BS6SNR.setName("BS6SNR");
        BS7SNR.setName("BS7SNR");

        HandoverCounterVec.setName("HandoverCounter");

        RangingTimeStats.setName("Ranging Start Time");
        HandoverTime.setName("Handover Time");
        HandoverDone.setName("Handover Done");

        rcvdPowerVector.setName("rcvdPower");
        therNoiseVector.setName("thermNoise");
        snrVector.setName("snr");

        WATCH(localMobilestationInfo.Basic_CID);
        WATCH(localMobilestationInfo.Primary_Management_CID);
        WATCH(localMobilestationInfo.scanModus);
    }
    {
        getParentModule()->getParentModule()->getParentModule()->
            getDisplayString().setTagArg("i2", 0, "x_off");
    }
// registerInterface();
}

void ControlPlaneMobilestation::finish()
{
    recordScalar("Received DL-MAPs", localMobilestationInfo.dlmapCounter);
    RangingTimeStats.record();
}

/**
* @brief Hauptfunktionen
***************************************************************************************/

/**
 * If incoming packets cannot be mapped directly to an active ServiceFlow,
 * a new ServiceFlow needs to be activated or created.
 * This is done via the Primary Management Connection of the MobileStation.
 * So a DSx request can only be sent, if the Primary Management Connection is active.
 * Otherwise, it has to be activated, first.
 */
void ControlPlaneMobilestation::handleClassificationCommand(Ieee80216ClassificationCommand *
                                                            command)
{
    ev << "(in ControlPlaneMobilestation::handleClassificationCommand) " << command << endl;
    ServiceFlow *requested_sf = new ServiceFlow();
    requested_sf->state = (sf_state) command->getRequested_sf_state();
    requested_sf->provisioned_parameters = &(command->getRequested_qos_params());

    if (fsm.getState() == CONNECT)
    {
        EV << "State= CONNECT" << endl;
        if (hasUplinkGrants())
        {
            // directly send new DSX-REQ
            EV << "Primary Management Connection is active. Sending DSx REQ...\n";
            cpsSF_MS->createAndSendNewDSA_REQ(localMobilestationInfo.Primary_Management_CID,
                                              requested_sf,
                                              (ip_traffic_types) command->getTraffic_type());
        }
        else
        {
            EV << "keine UplinkGrants vorhanden. Erzeuge Ieee80216BandwidthMacHeader und sende es an transceiverCommonGateOut"
                << endl;
            // BW-Request for Primary Management Connection
            Ieee80216BandwidthMacHeader *bw_req = new Ieee80216BandwidthMacHeader("BW-REQ");
            bw_req->setCID(localMobilestationInfo.Primary_Management_CID);
            bw_req->setTYPE_BD(1); // 1=aggregiert

            // ist die gefordete datenmenge von 48bit korrekt für alle DSx requests??? bestimmt nicht..
            bw_req->setBR(48);
            bw_req->setByteLength(48);

            send(bw_req, transceiverCommonGateOut);
            //sendtoLowerLayer( bw_req );
        }
    }
    else
    {
        EV << "State=  not CONNECTED" << endl;
    }
    delete command;
}

/**
 * Append the MAC address of this MobileStation to the DSA-ACK message (SS-initiated DSA)
 * or to the DSA-RSP message (BS-initiated DSA).
 * This way the station can later be identified within the BaseStation by one of its CIDs.
 * --> see ServiceFlows_BS source, structMobilestationInfo->map_own_connections
 */
void ControlPlaneMobilestation::handleServiceFlowMessage(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handleServiceFlowMessage) " << msg << endl;
    Ieee80216_DSA_ACK* dsa_ack = dynamic_cast<Ieee80216_DSA_ACK *>(msg);
    if (dsa_ack)
    {
        dsa_ack->setMac_address(localMobilestationInfo.MobileMacAddress);
    }
    else
    {
        Ieee80216_DSA_RSP* dsa_rsp = dynamic_cast<Ieee80216_DSA_RSP *>(msg);
        if (dsa_rsp)
        {
            dsa_rsp->setMac_address(localMobilestationInfo.MobileMacAddress);
        }
    }

    send(msg, transceiverCommonGateOut);
}

void ControlPlaneMobilestation::handleHigherLayerMsg(cMessage *msg)
{
    delete msg;
}

void ControlPlaneMobilestation::handleLowerLayerMsg(cMessage *msg)
{
    // process incoming frame
    EV << "Module " << getName() << " der Mobilestation erhaelt die Nachricht " <<
        msg->getName() << ".\n";
    Ieee80216GenericMacHeader* frame = check_and_cast<Ieee80216GenericMacHeader *>(msg);
    //receiverSnrVector.record(frame->getMinSNR());

    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type = frame->getTYPE();

    if (Type.Subheader == 1)
    {
        handleManagementFrame(frame);
    }

    // if (msg->getOwner() == this)  // FIXME
        delete msg;
}

void ControlPlaneMobilestation::handleManagementFrame(Ieee80216GenericMacHeader *genericFrame)
{
    if (!(dynamic_cast<Ieee80216ManagementFrame *>(genericFrame)))
    {
        error("Handle Management Frame keine Kontrolnachricht!");
    }

    if (dynamic_cast<Ieee80216ServiceFlowFrame *>(genericFrame))
    {
        Ieee80216ServiceFlowFrame* mframe =
            check_and_cast<Ieee80216ServiceFlowFrame *>(genericFrame);
        send(mframe, serviceFlowsGateOut);
    }
    else
    {

        Ieee80216ManagementFrame* mgmtFrame =
            check_and_cast<Ieee80216ManagementFrame *>(genericFrame);

        switch (mgmtFrame->getManagement_Message_Type())
        {
        case ST_DL_MAP:
            handleWithFSM(mgmtFrame);
            break;

        case ST_UL_MAP:
            handleWithFSM(mgmtFrame);
            break;

        case ST_DCD:
            handleWithFSM(mgmtFrame);
            break;

        case ST_UCD:
            handleWithFSM(mgmtFrame);
            break;

        case ST_RNG_RSP:
            handleWithFSM(mgmtFrame);
            break;

        case ST_SBC_RSP:
            handleWithFSM(mgmtFrame);
            break;

        case ST_REG_RSP:
            handleWithFSM(mgmtFrame);
            break;

        case ST_MOB_SCN_RSP:
            handleWithFSM(mgmtFrame);
            //handle_MOB_SCN_RSP_Frame(check_and_cast<Ieee80216_MOB_SCN_RSP *>(mgmtFrame));
            break;

        case ST_BSHO_RSP:
            handleWithFSM(mgmtFrame);
            //handle_MOB_BSHO_RSP_Frame(check_and_cast<Ieee80216_BSHO_RSP *>(mgmtFrame));
            break;

        case ST_RNG_REQ:
        case ST_SBC_REQ:
        case ST_REG_REQ:
        case ST_MOB_SCN_REQ:
        case ST_MSHO_REQ:
            break;

        default:
            error("error: Management Message Type not in switch-statement");
            delete mgmtFrame;
            break;
        }
    }
}

void ControlPlaneMobilestation::handleSelfMsg(cMessage *msg)
{
    EV << "Module " << getName() << " erhaelt die SelfMsg: " << msg->getName() << ".\n";
    if (msg == ScanChannelTimer)
    {
        handleWithFSM(msg);
    }
    else if (msg == DLMAPTimer)
    {
        handleWithFSM(msg);
    }
    else if (msg == RngRspTimeOut || msg == RegRspTimeOut)
    {
        handleWithFSM(msg);
    }
    else if (msg == startRangingRequest)
    {
        handleWithFSM(msg);
    }
    else if (msg == RngRegFail)
    {
        handleWithFSM(msg);
    }
    else if (msg == SBCRSPTimeOut)
    {
        handleWithFSM(msg);
    }
    else if (msg == ScnRegFail)
    {
        localMobilestationInfo.scanModus = false;
        scan_request_sent = false;
    }
    else if (msg == startFrameTimer)
    {
        startScanmodus();
    }
    else if (msg == scanIterationTimer)
    {
        ScanIterationBool = true;
        startScanmodus();
    }
    else if (msg == scanDurationTimer)
    {
        handleWithFSM(msg);
    }
    else if (msg == startHandoverScan)
    {
        handleWithFSM(msg);
    }
    else if (msg == startHandover)
    {
        handleWithFSM(msg);
    }
    else if (msg == sendPDU)
    {
        sendData();
    }
    else if (msg == stopPDU)
    {
        stopData();
    }
    else if (strcmp(msg->getName(), "sendControlPDU") == 0 ||
             strcmp(msg->getName(), "sendSecondControlPDU") == 0)
    {
        Ieee80216Prim_sendControlRequest* send_req = new Ieee80216Prim_sendControlRequest();
        int index = msg->findPar("CID");
        int cid = msg->par(index).longValue();
        send_req->setPduCID(cid);

        index = msg->findPar("bits_for_burst");
        if (index == -1)
        {
            send_req->setBits_for_burst(0);
        }
        else
        {
            send_req->setBits_for_burst(msg->par(index).longValue());
        }

        index = msg->findPar("realCID");
        send_req->setRealCID(msg->par(index).longValue());

//      index = msg->findPar("connectionType");
//      send_req->setRealCID( msg->par(index).longValue() );

        sendRequest(send_req);

        delete msg;
    }
    else if (strcmp(msg->getName(), "stopControlPDU") == 0 ||
             strcmp(msg->getName(), "stopSecondControlPDU") == 0)
    {
        Ieee80216Prim_stopControlRequest* stop_req = new Ieee80216Prim_stopControlRequest();
        int index = msg->findPar("CID");
        int cid = msg->par(index).longValue();

        stop_req->setPduCID(cid);
        sendRequest(stop_req);
        //stopControl(  send_req );
        delete msg;
    }
    else if (msg == MSHO_REQ_Fail)
    {
        build_MOB_MSHO_REQ();
    }
    else
    {
        error("Error im Module", getName(), "Self Msg nicht enthalten: ", msg->getName());
    }
}

/**
* Mobilestaion State-Machine
*/
void ControlPlaneMobilestation::handleWithFSM(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handleWithFSM) " << msg << endl;
    EV << "Module " << getName() << " der Mobilestation berarbeitet die Nachricht " << msg->
        getName() << " in der FSM. State: " << fsm.getStateName() << "\n";
    // WATCH(localMobilestationInfo.scanModus);

    FSMA_Switch(fsm)
    {
        FSMA_State(WAITDLMAP)
        {
  /** @brief "Kapitel 5.3 " Suchen nach einem Downlink-Kanal */
            FSMA_Event_Transition(Find - DL - MAP,
                                  localMobilestationInfo.scanModus == false && isDlMapMsg(msg),
                                  WAITULMAP, handleDL_MAPFrame(msg););

            //Wenn die MS nicht im ScanModus ist und nach der Zeit "ScanChannelTimer" keine DL-Map einer BS gefunden wurde wird der Kanal gewechselt
            FSMA_Event_Transition(Scan - next - Channel,
                                  localMobilestationInfo.scanModus == false
                                  && msg == ScanChannelTimer, WAITDLMAP,
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer); scanNextChannel(););

            //Wenn die MS im ScanModus ist und die empfangende Nachricht eine DL-Map ist wird die Funktion handleScanDL_MAPFrame durchgefuehrt
            FSMA_Event_Transition(Scan - DL - MAP,
                                  localMobilestationInfo.scanModus == true && isDlMapMsg(msg),
                                  WAITULMAP, handleScanDL_MAPFrame(msg););

            //Der ScanModus ist abgelaufen und wird beendet
            FSMA_Event_Transition(Stop - Scannig,
                                  msg == scanDurationTimer, CONNECT, stopScanmodus(););
        }

        FSMA_State(WAITULMAP)
        {
  /** @brief "Kapitel 5.3 " Bestimmen der Uplink Sendeparameter */
            //Wenn die MS nicht im ScanModus ist und die empfangende Nachricht eine UL-Map ist wird die Funktion handleUL_MAPFrame durchgefuehrt
            FSMA_Event_Transition(Find - UL - MAP,
                                  isUlMapMsg(msg),
                                  WAITDCD,
                                  handleUL_MAPFrame(msg); //Bestimmen der Sendeparamet
                                  );

            FSMA_Event_Transition(Scan - next - Channel,
                                  localMobilestationInfo.scanModus == false
                                      && msg == ScanChannelTimer,
                                  WAITDLMAP,
                                  scheduleAt(simTime(), ScanChannelTimer);
                                  );

            FSMA_Event_Transition(Stop - Scannig,
                                  localMobilestationInfo.scanModus == true
                                      && msg == scanDurationTimer,
                                  CONNECT,
                                  stopScanmodus();
                                  );
        }

        FSMA_State(WAITDCD)
        {
  /** @brief "Kapitel 5.3 " Bestimmen der Uplink Sendeparameter */
            FSMA_Event_Transition(Find - DCD, isDCDMsg(msg), WAITUCD, handleDCDFrame(msg););

            FSMA_Event_Transition(Scan - next - Channel,
                                  localMobilestationInfo.scanModus == false
                                      && msg == ScanChannelTimer,
                                  WAITDLMAP,
                                  scheduleAt(simTime(), ScanChannelTimer);
                                  );

            FSMA_Event_Transition(Stop - Scannig,
                                  localMobilestationInfo.scanModus == true
                                      && msg == scanDurationTimer,
                                  CONNECT,
                                  stopScanmodus();
                                  );
        }

        FSMA_State(WAITUCD)
        {
  /** @brief "Kapitel 5.3 " Bestimmen der Uplink Sendeparameter */
            FSMA_Event_Transition(Find - UCD,
                                  localMobilestationInfo.scanModus == false && isUCDMsg(msg),
                                  STARTRANGING,
                                  handleUCDFrame(msg);
                                  scheduleAt(simTime(), startRangingRequest);
                                  );
            FSMA_Event_Transition(Find - UCD,
                                  localMobilestationInfo.scanModus == true && isUCDMsg(msg),
                                  WAITDLMAP,
                                  handleUCDFrame(msg);
                                  );
            FSMA_Event_Transition(Scan - next - Channel,
                                  localMobilestationInfo.scanModus == false
                                      && msg == ScanChannelTimer,
                                  WAITDLMAP,
                                  scheduleAt(simTime(), ScanChannelTimer);
                                  );
            FSMA_Event_Transition(Stop - Scannig,
                                  localMobilestationInfo.scanModus == true
                                      && msg == scanDurationTimer,
                                  CONNECT,
                                  stopScanmodus();
                                  );
        }

        FSMA_State(STARTRANGING)
        {
  /** @brief "Kapitel 5.3 " Initial Ranging */
            FSMA_Event_Transition(Send - Ranging - Request,
                                  localMobilestationInfo.scanModus == false
                                      && msg == startRangingRequest,
                                  WAITRNGRSP,
                                  buildRNG_REQ();
                                  //scheduleAt(simTime()+0.2, RngRspTimeOut);//RNG-RSP Timeout == T3 (Table342) T3 Max 200ms
                                  //scheduleAt(simTime()+0.004, RngRegFail);//RNG-REG Timeout
                                  scheduleAt(simTime() + localMobilestationInfo.RngRspTimeOut, RngRspTimeOut); //RNG-RSP Timeout == T3 (Table342) T3 Max 200ms
                                  scheduleAt(simTime() + localMobilestationInfo.RngRspTimeOut + 0.05, RngRegFail); //RNG-REG Timeout
                                  );
            FSMA_Event_Transition(Send - Ranging - Request - again,
                                  localMobilestationInfo.scanModus == false && msg == RngRegFail,
                                  WAITRNGRSP,
                                  buildRNG_REQ();
                                  scheduleAt(simTime() + 0.006, RngRegFail); //RNG-RSP Timeout == T3 (Table342) T3 Max 200ms
                                  );
            FSMA_Event_Transition(Scan - next - Channel,
                                  msg == ScanChannelTimer,
                                  WAITDLMAP,
                                  isRangingIntervall = true;
                                  scheduleAt(simTime(), ScanChannelTimer);
                                  );
            FSMA_Event_Transition(DL - MAP,
                                  localMobilestationInfo.scanModus == false && isCorrectDLMAP(msg),
                                  STARTRANGING,
                                  cancelEvent(ScanChannelTimer);
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                      ScanChannelTimer);
                                  );

            FSMA_Event_Transition(UL - MAP,
                                  localMobilestationInfo.scanModus == false && isUlMapMsg(msg),
                                  WAITRNGRSP,
                                  handleUL_MAPFrame(msg);
                                  );
        }

        FSMA_State(WAITRNGRSP)
        {
  /** @brief "Kapitel 5.3.3 " Initial Ranging */
            FSMA_Event_Transition(Find - RNG - RSP,
                                  localMobilestationInfo.scanModus == false && isRngRspMsg(msg),
                                  WAITSBCRSP,
                                  cancelEvent(RngRspTimeOut);
                                  cancelEvent(RngRegFail);
                                  //scheduleAt(simTime()+2*localMobilestationInfo.DLMAP_interval, SBCRSPTimeOut);
                                  handle_RNG_RSP_Frame(msg);
                                  );

            FSMA_Event_Transition(Scan - next - Channel,
                                  msg == ScanChannelTimer,
                                  WAITDLMAP,
                                  cancelEvent(RngRspTimeOut);
                                  cancelEvent(RngRegFail);
                                  scheduleAt(simTime(), ScanChannelTimer);
                                  );

            FSMA_Event_Transition(Ranging - fail,
                                  msg == RngRegFail,
                                  STARTRANGING,
                                  cancelEvent(RngRspTimeOut);
                                  isRangingIntervall = true;
                                  //rangingStart();
                                  );

            FSMA_Event_Transition(Ranging - time - out,
                                  msg == RngRspTimeOut,
                                  WAITDLMAP,
                                  isRangingIntervall = true;
                                  cancelEvent(RngRegFail);
                                  cancelEvent(ScanChannelTimer); // (mk)
                                  scheduleAt(simTime(), ScanChannelTimer);
                                  );

            FSMA_Event_Transition(DL - MAP,
                                  localMobilestationInfo.scanModus == false && isCorrectDLMAP(msg),
                                  WAITRNGRSP,
                                  handleCorrectDLMAP(msg);
                                  cancelEvent(ScanChannelTimer);
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer);
                                  );

            FSMA_Event_Transition(UL - MAP,
                                  localMobilestationInfo.scanModus == false && isUlMapMsg(msg),
                                  WAITRNGRSP,
                                  handleUL_MAPFrame(msg);
                                  );
        }

        FSMA_State(WAITSBCRSP)
        {
  /** @brief "Kapitel 5.3.4 */
            FSMA_Event_Transition(Find - SBC - RSP,
                                  localMobilestationInfo.scanModus == false && isSbcRspMsg(msg),
                                  WAITREGRSP,
                                  cancelEvent(SBCRSPTimeOut);
                                  handle_SBC_RSP_Frame(msg);
                                  );

            FSMA_Event_Transition(SBC - RSP - time - out,
                                  msg == SBCRSPTimeOut,
                                  WAITDLMAP,
                                  isRangingIntervall = true;
                                  cancelEvent(RngRegFail);
                                  cancelEvent(ScanChannelTimer); // (mk)
                                  scheduleAt(simTime(), ScanChannelTimer);
                                  );

            FSMA_Event_Transition(Scan - next - Channel,
                                  msg == ScanChannelTimer,
                                  WAITDLMAP,
                                  scheduleAt(simTime(), ScanChannelTimer);
                                  );

            FSMA_Event_Transition(UL - MAP,
                                  localMobilestationInfo.scanModus == false && isUlMapMsg(msg),
                                  WAITSBCRSP,
                                  handleUL_MAPFrame(msg);
                                  );

            FSMA_Event_Transition(DL - MAP,
                                  localMobilestationInfo.scanModus == false && isCorrectDLMAP(msg),
                                  WAITSBCRSP,
                                  handleCorrectDLMAP(msg);
                                  cancelEvent(ScanChannelTimer);
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer);
                                  );
        }

        FSMA_State(WAITREGRSP)
        {
  /** @brief "Kapitel 5.3.6 " Regestrierung */
            FSMA_Event_Transition(Find - Ranging - Response,
                                  localMobilestationInfo.scanModus == false && isRegRspMsg(msg),
                                  CONNECT,
                                  handle_REG_RSP_Frame(msg);
                                  cancelEvent(RegRspTimeOut);
                                  );

            FSMA_Event_Transition(Registration - Timeout,
                                  localMobilestationInfo.scanModus == false && msg == RegRspTimeOut,
                                  WAITREGRSP,
                                  buildREG_REQ();
                                  );

            FSMA_Event_Transition(Scan - next - Channel,
                                  msg == ScanChannelTimer,
                                  WAITDLMAP,
                                  scheduleAt(simTime(), ScanChannelTimer);
                                  );

            FSMA_Event_Transition(UL - MAP,
                                  localMobilestationInfo.scanModus == false && isUlMapMsg(msg),
                                  WAITREGRSP,
                                  handleUL_MAPFrame(msg);
                                  );

            FSMA_Event_Transition(DL - MAP,
                                  localMobilestationInfo.scanModus == false && isCorrectDLMAP(msg),
                                  WAITREGRSP,
                                  handleCorrectDLMAP(msg);
                                  cancelEvent(ScanChannelTimer);
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer);
                                  );
        }

        FSMA_State(CONNECT)
        {
            FSMA_Event_Transition(Periodic - DL - MAP,
                                  isCorrectDLMAP(msg),
                                  CONNECT,
                                  handleCorrectDLMAP(msg);
                                  cancelEvent(ScanChannelTimer);
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer);
                                  );

            FSMA_Event_Transition(UL - MAP,
                                  isUlMapMsg(msg),
                                  CONNECT,
                                  handleUL_MAPFrame(msg);
                                  );

            FSMA_Event_Transition(Start - Scan - Modus,
                                  localMobilestationInfo.scanModus == true
                                      && msg == startHandoverScan,
                                  SCAN,
                                  scheduleAt(simTime(), startHandoverScan);
                                  );

            FSMA_Event_Transition(Lost - Connection,
                                  //localMobilestationInfo.scanModus == false && msg == ScanChannelTimer,
                                  msg == ScanChannelTimer, WAITDLMAP,
                                  //clearBSList();//Die List der gescanten Basisstationen
                                  localMobilestationInfo.scanModus = false;
                                  scan_request_sent = false;
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer);
                                  scanNextChannel();
                                  );

            FSMA_Event_Transition(send - PDU,
                                  localMobilestationInfo.scanModus == false && msg == sendPDU,
                                  CONNECT,
                                  sendData();
                                  );

            FSMA_Event_Transition(recieve - scn - rsp,
                                  localMobilestationInfo.scanModus == false && isScnRspMsg(msg),
                                  CONNECT,
                                  //cancelEvent(ScnRegFail);
                                  handle_MOB_SCN_RSP_Frame(msg);
                                  );

            FSMA_Event_Transition(recieve - HO - rsp,
                                  localMobilestationInfo.scanModus == true && isHoRspMsg(msg),
                                  CONNECT,
                                  handle_MOB_BSHO_RSP_Frame(msg);
                                  );

            FSMA_Event_Transition(start HO,
                                  localMobilestationInfo.scanModus == true && msg == startHandover,
                                  HANDOVER,
                                  makeHandover();
                                  );
        }

        FSMA_State(SCAN)
        {
            FSMA_Event_Transition(Next - Channel,
                                  localMobilestationInfo.scanModus == true
                                      && msg == startHandoverScan
                                      && ScanIterationBool == false,
                                  WAITDLMAP,
                                  //targetMobilestationInfo.scanModus = true;
                                  //localMobilestationInfo = targetMobilestationInfo;
                                  cancelEvent(ScanChannelTimer);
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer);
                                  //HO_start_Scn_Time = simTime();
                                  //recordScalar("HO start SCN A", HO_start_Scn_Time-HO_SNR_Time);
                                  //recordScalar("HO start SCN B", HO_start_Scn_Time-HO_get_SCN_RSP_Time);
                                  recordStartScanningMark();
                                  scanNextChannel(); //Tastet den naesten Kanal ab
                                  );

            FSMA_Event_Transition(Next - Channel - Again,
                                  localMobilestationInfo.scanModus == true
                                      && msg == startHandoverScan
                                      && ScanIterationBool == true,
                                  WAITDLMAP,
                                  cancelEvent(ScanChannelTimer);
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer);
                                  localMobilestationInfo.DownlinkChannel =
                                      scanningMobilestationInfo.DownlinkChannel; //uebergibt den zuletzt abgetasteten kanal
                                  recordStartScanningMark();
                                  scanNextChannel();
                                  );

            // MS looses connection to BS
            FSMA_Event_Transition(Scan - next - Channel,
                                  localMobilestationInfo.scanModus == false
                                      && msg == ScanChannelTimer,
                                  WAITDLMAP,
                                  scanNextChannel();
                                  resetStation();
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer);
                                  //++localScanningInfo.scanChannel;
                                  //changeDownlinkChannel(localScanningInfo.scanChannel);
                                  //scheduleAt(simTime()+localMobilestationInfo.ScanTimeInterval, ScanChannelTimer);
                                  );

            //Beendet Scan-Modus
            FSMA_Event_Transition(Back - to - target - BS,
                                  msg == scanDurationTimer,
                                  CONNECT,
                                  stopScanmodus();
                                  );
        }

        FSMA_State(HANDOVER)
        {
            //breakpoint("Handover");
            FSMA_Event_Transition(Handover end,
                                  localMobilestationInfo.scanModus == false && msg == startHandover,
                                  WAITDLMAP,
                                  makeHandoverBool = true;
                                  recordScalar("HO Handover done", startHoTime);
                                  cancelEvent(ScanChannelTimer);
                                  scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval,
                                             ScanChannelTimer);
                                  //breakpoint("Handover");
                                  );
        }
    }

    if (ev.isGUI())
        updateDisplay();
}

/**
* @brief Hilfsfunktionen
*****************************************************************************************/

/**
* @brief Funktionen f�r die Parameterbergabe (Kanal,Bitrate) an die Radiomodule
* @brief und den Kanal-Scan
*****************************************************************************************/
/** Suchfunktion für einen Empfangskanal*/
void ControlPlaneMobilestation::scanNextChannel()
{
    if (localMobilestationInfo.scanModus == true)
    {
        //breakpoint("Scan next Channel");
    }
    EV << "Scan channel " << localMobilestationInfo.DownlinkChannel << "\n";
    EV << "active channel " << activeMobilestationInfo.DownlinkChannel << "\n";
    EV << "Number of channel " << numChannels << "\n";

    ++localMobilestationInfo.DownlinkChannel;

    if (localMobilestationInfo.DownlinkChannel >= numChannels)
    {
        localMobilestationInfo.DownlinkChannel = 0;
    }

    if (localMobilestationInfo.scanModus == true
        && localMobilestationInfo.DownlinkChannel == activeMobilestationInfo.DownlinkChannel)
    {
        ++localMobilestationInfo.DownlinkChannel; //Ueberspringe Kanal der activen BS
        //breakpoint("Kanal ueberspringen");

        if (localMobilestationInfo.DownlinkChannel >= numChannels)
        {
            localMobilestationInfo.DownlinkChannel = 0;
        }
    }

    changeDownlinkChannel(localMobilestationInfo.DownlinkChannel);

    EV << "Scan next channel " << localMobilestationInfo.DownlinkChannel << "\n";
    //breakpoint("scanNextChannel");
    if (localMobilestationInfo.scanModus == false)
    {
        isRangingIntervall = true;
        anzahlVersuche = 0;
    }
}

/**
/Die Methode changDownlinkChannel ändert den Kanal der Donlink Radio Moduls
/Mit der eigenen msg-Nachricht WiMAXPhyControlInfo, mit ihr ist es möglich
/zwischen dem Uplink und Downlink Module zu wählen
*/
void ControlPlaneMobilestation::changeDownlinkChannel(int channelNum) //Einstellen des Kanals
{
    ev << "(in ControlPlaneMobilestation::changeDownlinkChannel) channelNum: " << channelNum <<
        endl;
    PhyControlInfo *phyCtrl = new PhyControlInfo();
    phyCtrl->setChannelNumber(channelNum);
    cMessage *msg = new cMessage("changeChannel", PHY_C_CONFIGURERADIO);
    msg->setControlInfo(phyCtrl);
    sendtoHigherLayer(msg);

}

/**
/Die Methode changUplinkChannel ändert den Kanal der Uplink Radio Moduls
/Mit der eigenen msg-Nachricht WiMAXPhyControlInfo, mit ihr ist es möglich
/zwischen dem Uplink und Downlink Module zu wählen
*/
void ControlPlaneMobilestation::changeUplinkChannel(int channelNum)
{
    PhyControlInfo *phyCtrl = new PhyControlInfo();
    phyCtrl->setChannelNumber(channelNum);
    cMessage *msg = new cMessage("changeChannel", PHY_C_CONFIGURERADIO); //VITA geaendert, davor war cMessage
    msg->setControlInfo(phyCtrl);
    sendtoLowerLayer(msg);
}

void ControlPlaneMobilestation::getRadioConfig()
{
    cModule *module_Radio =
        getParentModule()->getParentModule()->getSubmodule("msReceiver")->getSubmodule("snrEval");

    EV << "Achtung RS: Das Modul " << getName() << " erhält Zeiger des Modules" <<
        module_Radio->getName() << "\n";
    // TODO Verbindung zum Radiomodule aufbauen!!

    //if (snrEval_MS = dynamic_cast<SnrEval80216 *>(module_snrEval))
    // EV <<"RS: Module Name:" << snrEval_MS->name() <<"\n";
}

/**
* @brief Parameter der empfanfenden Basisstation abspeichern
* @brief
*****************************************************************************************/

/**
* Suchfunktion mit der die Basisstation anhand der BasestationID in der Liste identifiziert wird
*/
//ControlPlaneMobilestation::
structBasestationInfo *ControlPlaneMobilestation::lookupBS(MACAddress *BasestationID)
{
    BasestationList::iterator it;
    EV << "Looking for BasestationID: " << *BasestationID << "!";
    for (it = bsList.begin(); it != bsList.end(); ++it)
    {
        if (it->BasestationID == *BasestationID)
        {
            EV << "Found: " << *BasestationID << "\n";
            return &(*it);
        }
    }
    EV << "not found\n";
    return NULL;
}

/**
* Neu Basisstation in die Liste hinzuf�gen
* */
void ControlPlaneMobilestation::storeBSInfo(MACAddress *BasestationID, double frameSNR) //create new Base Station ID in BSInfo list
{
    if (lookupBS(BasestationID) != NULL)
    {
        EV << "Basestation ID=" << *BasestationID << " already in our BS list.\n";
    }
    else
    {
        EV << "Inserting Basestation ID " << *BasestationID << " into our BS list\n";
        structBasestationInfo *new_bs = new structBasestationInfo();
        new_bs->BasestationID = *BasestationID;

        SnrListEntry listEntry; // create a new entry
        listEntry.time = simTime().dbl();
        listEntry.snr = frameSNR;

        new_bs->planeSnrList.push_back(listEntry);
        bsList.push_back(*new_bs);
    }

}

/**
* Den Transceiver Kanal der Basisstation in die Liste eintragen
*/
void ControlPlaneMobilestation::UplinkChannelBS(MACAddress & BasestationID, const int UpChannel)
{
    structBasestationInfo *bs = lookupBS(&BasestationID);
    if (bs)
    {
        bs->UplinkChannel = UpChannel;
    }
}

/**
* Liste Loeschen
*/
void ControlPlaneMobilestation::clearBSList()
{
    BasestationList::iterator it;

    for (it = bsList.begin(); it != bsList.end(); ++it)
        if (it->authTimeoutMsg)
            delete cancelEvent(it->authTimeoutMsg);
    bsList.clear();
}

/**
* @brief Management-Nachrichte erstellen und behandeln von Management Nachrichten
* @brief
*****************************************************************************************/

void ControlPlaneMobilestation::handleDL_MAPFrame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handleDL_MAPFrame) " << msg << endl;
    Ieee80216DL_MAP* frame = check_and_cast<Ieee80216DL_MAP *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " keine DL-Map.");
    EV << "Aus der Nachricht" << frame->getName() << " die BasestationID:" << frame->getBS_ID() <<
        " erhalten.\n";
    EV << "Die Nachricht" << frame->getName() << "besitzt den min. SNR:" << frame->getSNR() << " .\n";
    //receiverSnrVector.record(frame->getMinSNR());

    storeBSInfo(&frame->getBS_ID(), frame->getSNR()); //Basisstation in ein Liste einfügen in der alle Basisstationen enthalten sind
}

/**
 *
 */
void ControlPlaneMobilestation::handleUL_MAPFrame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handleUL_MAPFrame) " << msg << endl;
    stationHasUplinkGrants = false;
    //long tx_data_in_next_ul_subframe = 0;

    Ieee80216UL_MAP* frame = check_and_cast<Ieee80216UL_MAP *>(msg);

    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " keine UL-Map.");

    ++counterRangingUL_MAP_IE;
    //EV << "Zufallszahl: " << getTransmissionOpportunitySlot(4, counterRangingUL_MAP_IE) << ".\n";

    EV << "MS: #UL-MAP_IE in UL-MAP: " << frame->getUlmap_ie_ListArraySize() << "\n";

    for (unsigned int i = 0; i < frame->getUlmap_ie_ListArraySize(); i++)
    {
        EV << "  " << frame->getUlmap_ie_List(i).getCID() << "\n";
    }

    // DEBUG
// EV << "MapConnections.size() = " << map_connections->size() << "\n";
// ConnectionMap::iterator it;
// for (it=map_connections->begin(); it !=map_connections->end(); it++)
//  EV << "CID|SFID = "<< it->first <<"|"<< it->second <<"\n";
    // END DEBUG

    int bits_for_next_uplink_subframe = 0;

    if (isRangingIntervall == true)
    {
        getParentModule()->getParentModule()->getParentModule()->bubble(frame->getName());
        getRangingUL_MAP_IE(frame);
    }
    else
    {
        if (frame->getUlmap_ie_ListArraySize() > 0) //gibt es eine UL-MAP Informationselement
        {
            for (unsigned int i = 0; i < frame->getUlmap_ie_ListArraySize(); ++i)
            {
                // (mk)
                if (frame->getUlmap_ie_List(i).getCID() == localMobilestationInfo.Basic_CID)
                {
                    stationHasUplinkGrants = true;
                }

                // handle uplink interval Request or Basic-CID
                if (frame->getUlmap_ie_List(i).getCID() == 0xFFFF ||
                    frame->getUlmap_ie_List(i).getCID() == localMobilestationInfo.Basic_CID)
                {
                    grants_per_second = grants_per_second + 1;

                    EV << "\n";
                    EV << "******************************************************************\n";
                    EV << "Grants Basic CID | Real CID: " << frame->getUlmap_ie_List(i).getCID() <<
                        " | " << frame->getUlmap_ie_List(i).getRealCID() << "\n";
                    EV << "Achtung RS: Module Name:" <<
                        getParentModule()->getParentModule()->getParentModule()->getName() << "\n";
                    EV << "Achtung RS: Simulationszeit:" << simTime() << " Durchgang i= " << i << "\n";
                    EV << "Achtung RS: Rahmenfolge:" << rahmenfolge << "\n";

                    if (frame->getUlmap_ie_List(i).getCID() == localMobilestationInfo.Basic_CID)
                    {
                        stationHasUplinkGrants = true;
                    }

                    char send_control_name[24];
                    char stop_control_name[24];

                    if (rahmenfolge == true)
                    {
                        sprintf(send_control_name, "sendControlPDU");
                        sprintf(stop_control_name, "stopControlPDU");

                        rahmenfolge = false;
                    }
                    else
                    {
                        sprintf(send_control_name, "sendSecondControlPDU");
                        sprintf(stop_control_name, "stopSecondControlPDU");

                        rahmenfolge = true;
                    }

                    // parameters are helper for decisions in the transceiver
                    cMessage *start_msg = new cMessage(send_control_name);
                    cMsgPar *par = new cMsgPar(); //VITA geaendert: war cPar *par = new cPar();
                    par->setName("CID");
                    par->setLongValue(frame->getUlmap_ie_List(i).getCID());
                    start_msg->addPar(par);

                    cMsgPar *bfb = new cMsgPar(); //VITA geaendert: war cPar *par = new cPar();
                    bfb->setName("bits_for_burst");
                    bfb->setLongValue(frame->getUlmap_ie_List(i).getBits_for_burst());
                    start_msg->addPar(bfb);
                    //EV << "BFB: "<< frame->getUlmap_ie_List(i).getBits_for_burst() << "\n";

                    cMsgPar *real_cid = new cMsgPar(); //VITA geaendert: war cPar *par = new cPar();
                    real_cid->setName("realCID");
                    real_cid->setLongValue(frame->getUlmap_ie_List(i).getRealCID());
                    start_msg->addPar(real_cid);

                    cMessage *stop_msg = new cMessage(stop_control_name);
                    cMsgPar *par2 = new cMsgPar(); //VITA geaendert: war cPar *par = new cPar();
                    par2->setName("CID");
                    par2->setLongValue(frame->getUlmap_ie_List(i).getCID());
                    stop_msg->addPar(par2);

                    EV << "Allocation Start Time  :" << frame->getAllocation_start_time() << "\n";
                    EV << "Achtung RS: Start Timer:" << frame->getAllocation_start_time() +
                        frame->getUlmap_ie_List(i).getStartTime_Offset() << "\n";
                    EV << "Achtung RS: Stop Timer :" << frame->getAllocation_start_time() +
                        frame->getUlmap_ie_List(i).getStopTime_Offset() << "\n";
                    double diff =
                        frame->getUlmap_ie_List(i).getStopTime_Offset() -
                        frame->getUlmap_ie_List(i).getStartTime_Offset();
                    EV << "Differenz: " << diff << " (" << diff * 4E6 << " bit)\n";

                    //startTimerVec.record(frame->getAllocation_start_time()+frame->getUlmap_ie_List(i).getStartTime_Offset());
                    //stopTimerVec.record(frame->getAllocation_start_time()+frame->getUlmap_ie_List(i).getStopTime_Offset());

                    bits_for_next_uplink_subframe += frame->getUlmap_ie_List(i).getBits_for_burst();

                    scheduleAt(frame->getAllocation_start_time() +
                               frame->getUlmap_ie_List(i).getStartTime_Offset(), start_msg);
                    scheduleAt(frame->getAllocation_start_time() +
                               frame->getUlmap_ie_List(i).getStopTime_Offset(), stop_msg);
                }
            }
        }
    }

    if (fsm.getState() == CONNECT)
    {
        EV << "Request packets for " << bits_for_next_uplink_subframe <<
            " bits from scheduler...\n";
        cps_scheduling->sendPacketsDown(bits_for_next_uplink_subframe);
    }
}

void ControlPlaneMobilestation::getRangingUL_MAP_IE(Ieee80216UL_MAP *frame)
{
    ev << "(in ControlPlaneMobilestation::getRangingUL_MAP_IE) " << frame << endl;
    rangingStart();             //Funktion zur Bestimmung des Ranging Intervall Slots

    for (unsigned int i = 0; i < frame->getUlmap_ie_ListArraySize(); ++i)
    {
        if (frame->getUlmap_ie_List(i).getCID() == 0)
        {
            EV << "\n";
            EV << "******************************************************************\n";
            EV << "Initial-Ranging-Interval.\n";
            EV << "Modul Name:" << getParentModule()->getParentModule()->getParentModule()->
                getName() << " \n";
            EV << "Initail-Ranging-Interval CID: " << frame->getUlmap_ie_List(i).
                getCID() << " = 0000\n";
            EV << "Simulationszeit:" << simTime() << "\n";
            //EV << "Control Plane Modul Name:" << name() <<"\n";
            EV << "Allocation start time: " << frame->getAllocation_start_time() << "\n";
            EV << "Initial_Ranging_Interval_Slot: " << Initial_Ranging_Interval_Slot << "\n";
            EV << "Transmission Opportunity Slot startet:" << frame->getAllocation_start_time() +
                Initial_Ranging_Interval_Slot * 48 / 4E6 << "\n";
            EV << "Transmission Opportunity Slot endet:" << frame->getAllocation_start_time() +
                (Initial_Ranging_Interval_Slot + 1) * 48 / 4E6 << "\n";
            EV << "Initial-Ranging-Interval Counter:" << counterRangingUL_MAP_IE << "\n";

            cMessage *start_msg = new cMessage("sendControlPDU");
            cMsgPar *par = new cMsgPar(); // VITA geaendert: war davor  cPar *par = new cPar();
            par->setName("CID");
            par->setLongValue(0);
            start_msg->addPar(par);

            cMsgPar *real_cid = new cMsgPar(); // VITA geaendert: war davor  cPar *real_cid = new cPar();
            real_cid->setName("realCID");
            real_cid->setLongValue(frame->getUlmap_ie_List(i).getRealCID());
            start_msg->addPar(real_cid);

            cMessage *stop_msg = new cMessage("stopControlPDU");
            cMsgPar *par2 = new cMsgPar(); // VITA geaendert: war davor  cPar *par2 = new cPar();
            par2->setName("CID");
            par2->setLongValue(0);
            stop_msg->addPar(par2);

            scheduleAt(frame->getAllocation_start_time() + Initial_Ranging_Interval_Slot * 48 / 4E6, start_msg); // VITA geaendert: war davor statt simtime : frame->getAllocation_start_time()
            scheduleAt(frame->getAllocation_start_time() + (Initial_Ranging_Interval_Slot + 1) * 48 / 4E6, stop_msg); // VITA geaendert: war davor statt simtime : frame->getAllocation_start_time()

            isRangingIntervall = false;
            ++rangingVersuche;

            recordScalar("Start Ranging Time",
                         frame->getAllocation_start_time() +
                         Initial_Ranging_Interval_Slot * 48 / 4E6);
            RangingTimeStats.record();
        }
        else
        {
            EV << "Keine Initial-Ranging-Interval in der UL-MAP.\n";
        }
    }
}

void ControlPlaneMobilestation::handleDCDFrame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handleDCDFrame) " << msg << endl;
    Ieee80216_DCD *frame = check_and_cast<Ieee80216_DCD *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " keine DCD.");
    EV << "Die Nachricht" << frame->getName() << " erhalten.\n";

    structBasestationInfo *bs = lookupBS(&frame->getBS_ID());
    if (!bs)
    {
// cancelEvent(ScanChannelTimer);
//  handleWithFSM(ScanChannelTimer);//neuer Scanvorgang
    }
}

void ControlPlaneMobilestation::handleUCDFrame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handleUCDFrame) " << msg << endl;
    Ieee80216_UCD *frame = check_and_cast<Ieee80216_UCD *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " keine UCD.");
    EV << "Die Nachricht" << frame->getName() << " erhalten.\n";

    structBasestationInfo *bs = lookupBS(&frame->getBS_ID());

    if (bs)
    {
        //EV << "UCD arrived.\n";
        UplinkChannelBS(frame->getBS_ID(), frame->getUploadChannel());
        changeUplinkChannel(frame->getUploadChannel());
        bs->UplinkChannel = frame->getUploadChannel();
        localMobilestationInfo.UplinkChannel = frame->getUploadChannel();
        bs->Ranging_request_opportunity_size = frame->getRanging_request_opportunity_size();
        bs->Bandwidth_request_opportunity_size = frame->getBandwidth_request_opportunity_size();

        //handleWithFSM(startRangingRequest);
    }
    else
    {
        if (!ScanChannelTimer->isScheduled())
            handleWithFSM(ScanChannelTimer); //neuer Scanvorgang
    }
}

void ControlPlaneMobilestation::handle_RNG_RSP_Frame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handle_RNG_RSP_Frame) " << msg << endl;
    Ieee80216_RNG_RSP *frame = check_and_cast<Ieee80216_RNG_RSP *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " kein RNG-RSP.");
    EV << "Die Nachricht" << frame->getName() << " erhalten.\n";

    // TODO die abfrage sollte nich in jeder handle-methode aufs neue
    // gemacht werden sondern möglichst zentral beim eingang der nachricht
    // Die Abfrage wird in der Funktion isRNGRSP durchgeführt
    if (frame->getMSS_MAC_Address() == localMobilestationInfo.MobileMacAddress)
    {
        localMobilestationInfo.Basic_CID = frame->getBasic_CID();
        localMobilestationInfo.Primary_Management_CID = frame->getPrimary_Management_CID();

        // update the msslist object in the CommonPartReceiver (only used in MS)
        cps_Receiver_MS->setSubscriberList(&localMobilestationInfo);
        //cps_Receiver_MS->setSubscriberList(&localMobilestationInfo); verurasach fehler

        EV << "Basic and Primary Management connection established -> " << localMobilestationInfo.
            Basic_CID << "|" << localMobilestationInfo.Primary_Management_CID << "\n";

        // management connection do not have a ServiceFlow
        // so the SFID is set to -1
        (*map_connections)[localMobilestationInfo.Basic_CID] = -1;
        (*map_connections)[localMobilestationInfo.Primary_Management_CID] = -1;

        // TODO einträge in den maps machen.. -> ServiceFlow der Management-Connections muss hierrüberkommen
        getParentModule()->getParentModule()->getParentModule()->bubble(frame->getName());

        isRangingIntervall = true; // Initial Ranging activieren
        Use_ULMap = 0;
        ULMap_Counter = 0;
        //breakpoint("handle RNG RSP");
        buildSBC_REQ();
    }
    else
    {
        EV << "RNG_RSP not for this mobile station.\n";
        delete frame;
    }
}

void ControlPlaneMobilestation::handle_SBC_RSP_Frame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handle_SBC_RSP_Frame) " << msg << endl;
    Ieee80216_SBC_RSP* frame = check_and_cast<Ieee80216_SBC_RSP *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " kein RNG-RSP.");
    EV << "Die Nachricht" << frame->getName() << " erhalten.\n";
    getParentModule()->getParentModule()->getParentModule()->bubble(frame->getName());
    buildREG_REQ();
}

void ControlPlaneMobilestation::handle_REG_RSP_Frame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handle_REG_RSP_Frame) " << msg << endl;
    Ieee80216_REG_RSP* frame = check_and_cast<Ieee80216_REG_RSP *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " kein REG-RSP.");
    EV << "Die Nachricht" << frame->getName() << " erhalten.\n";
    if (frame->getSecondary_Management_CID() != -1)
    {
        EV << "Received new SecondaryManagementCID (" << frame->
            getSecondary_Management_CID() << "\n";
        localMobilestationInfo.Secondary_Management_CID = frame->getSecondary_Management_CID();

        // update the msslist object in the CommonPartReceiver (only used in MS)
        cps_Receiver_MS->setSubscriberList(&localMobilestationInfo);

        // no ServiceFlow for management connections. SFID = -1
        (*map_connections)[localMobilestationInfo.Secondary_Management_CID] = -1;
    }
    //delete frame;
    //parentModule()->parentModule()->parentModule()->bubble(frame->name());
    if (makeHandoverBool == true)
    {
        //makeHandoverBool = false;
        makeHandover_cdTrafficBool = true;
        HO_NetEntryTime = simTime().dbl();
        recordScalar("Mark Net Entry", HO_NetEntryTime);
        recordScalar("HO Net Entry Time", HO_NetEntryTime - HO_makeHandover);
        HandoverTime.record();  //recordScalar();
        recordScalar("HO Hondover Time", HO_NetEntryTime - startHoTime);
        HandoverTime.record();
        stopTimerVec.record(frame->getSNR());
    }

    if (ev.isGUI())
    {
        getParentModule()->getParentModule()->getParentModule()->bubble("Connect");
    }
}

void ControlPlaneMobilestation::handle_MOB_SCN_RSP_Frame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handle_MOB_SCN_RSP_Frame) " << msg << endl;
    Ieee80216_MOB_SCN_RSP *frame = check_and_cast<Ieee80216_MOB_SCN_RSP *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " kein REG-RSP.");

    //breakpoint("Handle SCN-RSP");
    cancelEvent(ScnRegFail);

    EV << "Die Nachricht" << frame->getName() << " erhalten.\n";

    // get scanning parameter from Basestation if Basestation change parameter get new parameter
    if (localMobilestationInfo.scanDuration != frame->getScanDuration())
        localMobilestationInfo.scanDuration = frame->getScanDuration();
    if (localMobilestationInfo.interleavingInterval != frame->getInterleavingInterval())
        localMobilestationInfo.interleavingInterval = frame->getInterleavingInterval();
    if (localMobilestationInfo.scanIteration != frame->getScanIteration())
        localMobilestationInfo.scanIteration = frame->getScanIteration();

    localMobilestationInfo.strartFrame = frame->getStrartFrame();
    EV << "scanDuration:" << frame->getScanDuration() << "\n";
    EV << "interleavingInterval:" << frame->getInterleavingInterval() << "\n";
    EV << "scanIteration:" << frame->getScanIteration() << "\n";
    EV << "strartFrame:" << frame->getStrartFrame() << "\n";

    //ScnRspTime = simTime();
    //recordScalar("HO SCN RSP", ScnRspTime-startHoTime);
    //HO_get_SCN_RSP_Time = simTime();
    //recordScalar("HO SCN RSP", HO_get_SCN_RSP_Time-HO_SNR_Time);
    //HandoverTime.recordScalar();
    // scanning is allowed if Scan Duration > 1
    if (frame->getScanDuration() >= 1)
    {
        EV << "Simulationszeit:" << simTime() << "\n";
        //EV << "Timer:" << simTime()+localMobilestationInfo.strartFrame*0.02  << "\n";  // TODO: Frame duration von BS ÜBERGENEN LASSEN
        EV << "Timer:" << simTime() +
            localMobilestationInfo.strartFrame * localMobilestationInfo.DLMAP_interval << "\n";
        localMobilestationInfo.scanModus = true;
        //scheduleAt(simTime()+localMobilestationInfo.strartFrame*0.02, startFrameTimer);
        scheduleAt(simTime() +
                   localMobilestationInfo.strartFrame * localMobilestationInfo.DLMAP_interval,
                   startFrameTimer);
        EV << "Starte Timer:" << startFrameTimer->getName() << "\n";
    }
    else
    {
        localMobilestationInfo.scanModus = false;
        scan_request_sent = false;
        EV << "Scanning not allowed!\n";
    }
    EV << "scanModus:" << localMobilestationInfo.scanModus << "\n";
}

void ControlPlaneMobilestation::handle_MOB_BSHO_RSP_Frame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handle_MOB_BSHO_RSP_Frame) " << msg << endl;
    Ieee80216_BSHO_RSP* frame = check_and_cast<Ieee80216_BSHO_RSP *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " kein MOB_BSHO-RSP.");

    cancelEvent(MSHO_REQ_Fail);
    //breakpoint("handle BSHO_RSP");
    //stopTimerVec.record(frame->getSNR());
    build_MOB_HO_IND();
}

/**
* Erzeugt Ranging Restpons Nachricht
*
*
*************************************************/
void ControlPlaneMobilestation::buildRNG_REQ()
{
    ev << "(in ControlPlaneMobilestation::buildRNG_REQ) " << endl;
    isRangingIntervall = false;
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;

    /** Erstelle DL-Map
    */
    Ieee80216_RNG_REQ* frame = new Ieee80216_RNG_REQ("RNG_REQ");

    /** Nachrichtenspecifische Informationen
    */
    frame->setTYPE(Type);
    frame->setMSS_MAC_Address(localMobilestationInfo.MobileMacAddress); //copy Mac Adrress of MSS into frame

    /** Sende Nachricht zum Transciever Module
    */
    //EV << "Sende " << frame->name()<< " Nachricht. Die Simulationszeit:" << simTime() <<" .\n";
    sendtoLowerLayer(frame);
    //msgPuffer = frame;
    if (ev.isGUI())
    {
        getParentModule()->getParentModule()->getParentModule()->bubble(frame->getName());
    }
}

/**
* Erzeugt SBC-REQ Nachricht
*
*
*************************************************/
void ControlPlaneMobilestation::buildSBC_REQ()
{
    ev << "(in ControlPlaneMobilestation::buildSBC_REQ) " << endl;
    Ieee80216_SBC_REQ* frame = new Ieee80216_SBC_REQ("SBC_REQ");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);

    // (mk)
    if (localMobilestationInfo.Basic_CID != -1)
    {
        frame->setCID(localMobilestationInfo.Basic_CID);
    }
    else
    {
        error("mk: die Basic_CID der SS wurde nicht richtig erzeugt");
    }
    if (ev.isGUI())
    {
        getParentModule()->getParentModule()->getParentModule()->bubble(frame->getName());
    }
    EV << "Sende " << frame->getName() << " Nachricht. Die Simulationszeit:" << simTime() << " .\n";
    sendtoLowerLayer(frame);
}

/**
* Erzeugt REQ-REQ Nachricht
*
*
*************************************************/
void ControlPlaneMobilestation::buildREG_REQ()
{
    ev << "(in ControlPlaneMobilestation::buildREG_REQ) " << endl;
    Ieee80216_REG_REQ *frame = new Ieee80216_REG_REQ("REG_REQ");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    frame->setMSS_MAC_Address(localMobilestationInfo.MobileMacAddress);

    // (mk)
    frame->setCID(localMobilestationInfo.Primary_Management_CID);

    EV << "Sende " << frame->getName() << " Nachricht. Die Simulationszeit:" << simTime() << " .\n";
    sendtoLowerLayer(frame);

    scheduleAt(simTime() + localMobilestationInfo.RegRspTimeOut, RegRspTimeOut);
}

/**
* Erzeugt MOB_SCN-REQ Nachricht
*
*
*************************************************/
void ControlPlaneMobilestation::build_MOB_SCN_REQ()
{
    ev << "(in ControlPlaneMobilestation::build_MOB_SCN_REQ) " << endl;
    Ieee80216_MOB_SCN_REQ* frame = new Ieee80216_MOB_SCN_REQ("MOB_SCN-REQ");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);

    // (mk)
    frame->setCID(localMobilestationInfo.Primary_Management_CID);

    //set scan parameter
    frame->setScanDuration(localMobilestationInfo.scanDuration);
    frame->setInterleavingInterval(localMobilestationInfo.interleavingInterval);
    frame->setScanIteration(localMobilestationInfo.scanIteration);

    //print inforamtion on display and shell
    EV << "Sende " << frame->getName() << " Nachricht. Die Simulationszeit:" << simTime() << " .\n";
    if (ev.isGUI())
    {
        getParentModule()->getParentModule()->getParentModule()->bubble(frame->getName());
    }

    //breakpoint("Send SCN-REQ");
    scheduleAt(simTime() + 0.05, ScnRegFail);
    sendtoLowerLayer(frame);
}

void ControlPlaneMobilestation::build_MOB_MSHO_REQ()
{
    ev << "(in ControlPlaneMobilestation::build_MOB_MSHO_REQ) " << endl;
    //breakpoint("build MOB-MSHO-REQ");
    Ieee80216_MSHO_REQ* frame = new Ieee80216_MSHO_REQ("MOB_MSHO-REQ");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);
    // (mk)
    frame->setCID(localMobilestationInfo.Primary_Management_CID);

    EV << "Sende " << frame->getName() << " Nachricht. Die Simulationszeit:" << simTime() << " .\n";
    getParentModule()->getParentModule()->getParentModule()->bubble(frame->getName());
    //scheduleAt(simTime() + 2*localMobilestationInfo.DLMAP_interval, MSHO_REQ_Fail);
    sendtoLowerLayer(frame);
}

void ControlPlaneMobilestation::build_MOB_HO_IND()
{
    //breakpoint("HO-IND");
    ev << "(in ControlPlaneMobilestation::build_MOB_HO_IND) " << endl;
    Ieee80216_MOB_HO_IND* frame = new Ieee80216_MOB_HO_IND("MOB_HO-IND");
    SubType Type;               //SubType ist ein Struct und ist in Ieee80216Frame.msg defeniert. Er kenzeichnet ob Subheader vorhanden sind.
    Type.Subheader = 1;
    frame->setTYPE(Type);

    // (mk)
    frame->setCID(localMobilestationInfo.Primary_Management_CID);

    EV << "Sende " << frame->getName() << " Nachricht. Die Simulationszeit:" << simTime() << " .\n";
    getParentModule()->getParentModule()->getParentModule()->bubble(frame->getName());
    sendtoLowerLayer(frame);
    EV << "HO-IND scanModus: " << localMobilestationInfo.scanModus << endl;
    scheduleAt(simTime() + localMobilestationInfo.DLMAP_interval, startHandover);
}

bool ControlPlaneMobilestation::isDlMapMsg(cMessage *msg)
{
    return dynamic_cast<Ieee80216DL_MAP *>(msg);
}

bool ControlPlaneMobilestation::isUlMapMsg(cMessage *msg)
{
    return dynamic_cast<Ieee80216UL_MAP *>(msg);
}

bool ControlPlaneMobilestation::isDCDMsg(cMessage *msg)
{
    return dynamic_cast<Ieee80216_DCD *>(msg);
}

bool ControlPlaneMobilestation::isUCDMsg(cMessage *msg)
{
    return dynamic_cast<Ieee80216_UCD *>(msg);
}

bool ControlPlaneMobilestation::isRngRspMsg(cMessage *msg)
{
    Ieee80216_RNG_RSP* frame = dynamic_cast<Ieee80216_RNG_RSP *>(msg); //Empfangedes Paket ist Ranging  Response

    if (!frame)
    {
        return false;
    }
    else
    {
        if (frame->getMSS_MAC_Address() == localMobilestationInfo.MobileMacAddress) //Besitzt die Nachricht  die richtige MAC Adresse
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool ControlPlaneMobilestation::isSbcRspMsg(cMessage *msg)
{
    return dynamic_cast<Ieee80216_SBC_RSP *>(msg);
}

bool ControlPlaneMobilestation::isRegRspMsg(cMessage *msg)
{
    return dynamic_cast<Ieee80216_REG_RSP *>(msg);
}

bool ControlPlaneMobilestation::isScnRspMsg(cMessage *msg)
{
    return dynamic_cast<Ieee80216_MOB_SCN_RSP *>(msg);
}

bool ControlPlaneMobilestation::isHoRspMsg(cMessage *msg)
{
    return dynamic_cast<Ieee80216_BSHO_RSP *>(msg);
}

bool ControlPlaneMobilestation::isStartRangingRequest(cMessage *msg)
{
    EV << "Achtung RS: Ist es Ranging Request? ";
    if (msg == startRangingRequest)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// (mk)
// copy/paste from 802.11Mac
void ControlPlaneMobilestation::registerInterface()
{
    ev << "(in ControlPlaneMobilestation::registerInterface) " << endl;
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (!ift)
        return;

    InterfaceEntry *e = new InterfaceEntry(this);

    // interface name: NetworkInterface module's name without special characters ([])
    char *interfaceName = new char[strlen(getParentModule()->getFullName()) + 1];
    char *d = interfaceName;
    for (const char *s = getParentModule()->getFullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';

    e->setName(interfaceName);
    delete [] interfaceName;

    // address
    MACAddress mac_add = localMobilestationInfo.MobileMacAddress;
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

/**
* @brief Funktionen fuer die Initial Ranging Intervall Slots
*
*****************************************************************************************/

void ControlPlaneMobilestation::rangingStart()
{
    ev << "(in ControlPlaneMobilestation::rangingStart) " << endl;
    isRangingIntervall = true;
    ++rangingVersuche;
    ++ULMap_Counter;
    Initial_Ranging_Interval_Slot = getTransmissionOpportunitySlot(4, rangingVersuche);
    //isRangingIntervall = false;
    if (ev.isGUI())
    {
        getParentModule()->getParentModule()->getParentModule()->bubble("ranging Start");
    }
    //scheduleAt(simTime(), ScanChannelTimer);
}

/**
* Transmission opportunity slot calculation
*************************************************/
int ControlPlaneMobilestation::getTransmissionOpportunitySlot(int transmissionOpportunitySize,
                                                              int versuche)
{
    if (versuche > transmissionOpportunitySize)
    {
        //rangingVersuche=0;
        //versuche=1;
        Use_ULMap = intrand(versuche) + 1; //Warte eine X Frame mit dem Initial Ranging
        ULMap_Counter = 0;
    }
    else
    {
        ULMap_Counter = 0;
    }

    //anzahlVersuche = pow(2,versuche);
    //int zufallszahl = intrand(anzahlVersuche)+1;
    //int zufallszahl = intrand(transmissionOpportunitySize)+1;
    //EV << "Zufallszahl:" << zufallszahl <<"\n";
    //return (zufallszahl);
    return ((intrand(transmissionOpportunitySize) + 1));
}

void ControlPlaneMobilestation::updateDisplay()
{
    string bs_id = "n/a";

    if (bsList.size() > 0)
    {
        structBasestationInfo con_bs = bsList.front();
        bs_id = con_bs.BasestationID.str();
    }

    //EV << "CONNECTED BS FOR MS "<< localMobilestationInfo.MobileMacAddress <<":  "<< bs_id <<"\n";
    //EV << "Basic CID "<< localMobilestationInfo.Basic_CID <<"\n";
    char buf[80];
    //sprintf(buf, "CID(B|P|S): (%ld|%ld|%ld)", localMobilestationInfo.Basic_CID, localMobilestationInfo.Primary_Management_CID, localMobilestationInfo.Secondary_Management_CID);
    sprintf(buf, "CID(B|P|S): (%d|%d|%d) \nSlot: %d State: %d DLmap: %d\nhasUplinkGrants: %d",
            localMobilestationInfo.Basic_CID, localMobilestationInfo.Primary_Management_CID,
            localMobilestationInfo.Secondary_Management_CID, Initial_Ranging_Interval_Slot + 1,
            fsm.getState(), localMobilestationInfo.dlmapCounter, stationHasUplinkGrants);
    //sprintf(buf, "Slot: %d ", neuerSlot+1);
    getDisplayString().setTagArg("t", 0, buf);
    //EV << "Achtung RS SNR:" << cps_Receiver_MS->receiverSNR <<" Modulename:" << cps_Receiver_MS->name() <<".\n";

    cur_floor = floor(simTime().dbl());
    if ((cur_floor - last_floor) == 1)
    {
        char grants_buf[30];
        sprintf(grants_buf, "%d grants/s", grants_per_second);
        getParentModule()->getDisplayString().setTagArg("t", 0, grants_buf);
        getParentModule()->getDisplayString().setTagArg("t", 1, "t");
        grants_per_second = 0;
    }

    char scanmodus_buf[50];
    sprintf(scanmodus_buf, "BS %d\nHandover: %d", localMobilestationInfo.DownlinkChannel + 1,
            HandoverCounter);
    getParentModule()->getParentModule()->getParentModule()->getDisplayString().setTagArg("t", 0,
                                                                                          scanmodus_buf);

    if (fsm.getState() == CONNECT)
    {
        getParentModule()->getParentModule()->getParentModule()->
            getDisplayString().setTagArg("i2", 0, "x_green");
    }
    else if (fsm.getState() == WAITRNGRSP)
    {
        getParentModule()->getParentModule()->getParentModule()->
            getDisplayString().setTagArg("i2", 0, "x_yellow");
    }
    else if (fsm.getState() == WAITSBCRSP || fsm.getState() == WAITREGRSP)
    {
        getParentModule()->getParentModule()->getParentModule()->
            getDisplayString().setTagArg("i2", 0, "x_excl");
    }
    else if (localMobilestationInfo.scanModus == true)
    {
        getParentModule()->getParentModule()->getParentModule()->
            getDisplayString().setTagArg("i2", 0, "x_noentry");
    }
    else
    {
        getParentModule()->getParentModule()->getParentModule()->
            getDisplayString().setTagArg("i2", 0, "x_red");
    }
}

int ControlPlaneMobilestation::getBitsPerSecond(int duration)
{
// FIXME: wird später dynamisch angepasst (entsprechend der Modulationsart etc.)
    double max_bitrate = 4E+6;  // 4MBit/s

    if (duration != 0)
    {
        double duration_factor = 1000 / duration;

        double bits_per_second = max_bitrate / duration_factor;

        return bits_per_second + .5;
    }
    else
        return 0;
}

/**
 * Checks, if the Primary Management Connection is activated, i.e. an uplink interval is
 * allocated in the current UL-MAP for the next UL-Frame.
 */
bool ControlPlaneMobilestation::hasUplinkGrants()
{
    //return ( getSFIDForCID( localMobilestationInfo.Primary_Management_CID ) == MCID_ACTIVE ) ;
    return stationHasUplinkGrants;
}

bool ControlPlaneMobilestation::isCorrectDLMAP(cMessage *msg)
{
    Ieee80216DL_MAP *frame = dynamic_cast<Ieee80216DL_MAP *>(msg);
    if (frame)
    {
        structBasestationInfo *connectedBasestation = lookupBS(&(frame->getBS_ID()));

        return (connectedBasestation != NULL);
    }
    else
    	return false;
}

void ControlPlaneMobilestation::sendData()
{
    ev << "(in ControlPlaneMobilestation::sendData) " << endl;
    Ieee80216Prim_sendDataRequest *sendDataUnit = new Ieee80216Prim_sendDataRequest();
    sendRequest(sendDataUnit);
}

void ControlPlaneMobilestation::stopData()
{
    ev << "(in ControlPlaneMobilestation::stopData) " << endl;
    Ieee80216Prim_stopDataRequest *stopDataUnit = new Ieee80216Prim_stopDataRequest();
    sendRequest(stopDataUnit);
}

void ControlPlaneMobilestation::sendControl(Ieee80216Prim_sendControlRequest *control_info)
{
    ev << "(in ControlPlaneMobilestation::sendControl) " << endl;
    //Ieee80216Prim_sendControlRequest *sendControlUnit = new Ieee80216Prim_sendControlRequest();
    //sendControlUnit->setPduCID();

    if (control_info != NULL)
    {
        sendRequest(control_info);
    }
    else
    {
        Ieee80216Prim_sendControlRequest *sendControlUnit = new Ieee80216Prim_sendControlRequest();
        sendRequest(sendControlUnit);
    }

    //if (ev.isGUI())
    //   {
    //     parentModule()->parentModule()->parentModule()->bubble("send Conrol");
    //   }
}

void ControlPlaneMobilestation::stopControl(Ieee80216Prim_stopControlRequest *control_info)
{
    ev << "(in ControlPlaneMobilestation::stopControl) " << endl;
    //Ieee80216Prim_stopControlRequest *stopControlUnit = new Ieee80216Prim_stopControlRequest();
    if (control_info != NULL)
    {
        sendRequest(control_info);
    }
    else
    {
        Ieee80216Prim_stopControlRequest *sendControlUnit = new Ieee80216Prim_stopControlRequest();
        sendRequest(sendControlUnit);
    }
}

management_type ControlPlaneMobilestation::getManagementType(int CID)
{
    if (CID == localMobilestationInfo.Basic_CID)
    {
        return BASIC;
    }
    else if (CID == localMobilestationInfo.Primary_Management_CID)
    {
        return PRIMARY;
    }
    else if (CID == localMobilestationInfo.Secondary_Management_CID)
    {
        return SECONDARY;
    }
    else
    {
		return (management_type)-1; // FIXME  is this correct?
    	//    	throw cRuntimeError("Unexpected CID value: %d", CID);
    }
}

station_type ControlPlaneMobilestation::getStationType()
{
    return MOBILESTATION;
}

ControlPlaneMobilestation::State ControlPlaneMobilestation::getFsmState()
{
    return (ControlPlaneMobilestation::State) fsm.getState();
}

void ControlPlaneMobilestation::setDataMsgQueue(Ieee80216MacHeaderFrameList *data_queue)
{
    Enter_Method_Silent();
    // nothing to do here...
}

int ControlPlaneMobilestation::getBasicCID()
{
    Enter_Method_Silent();
    return localMobilestationInfo.Basic_CID;
}

int ControlPlaneMobilestation::getPrimaryCID()
{
    Enter_Method_Silent();
    return localMobilestationInfo.Primary_Management_CID;
}

void ControlPlaneMobilestation::resetStation()
{
    localMobilestationInfo.Basic_CID = -1;
    localMobilestationInfo.Primary_Management_CID = -1;
    localMobilestationInfo.Secondary_Management_CID = -1;

    bsList.clear();

    map_connections->clear();
    map_serviceFlows->clear();

    //RS
    isRangingIntervall = true;
    scan_request_sent = false;
    localMobilestationInfo.scanModus = false;
    startHO_bool = false;
    //breakpoint("Station resetted");
    while (scanSNRQueue.empty() == false)
    {
        SNRQueue.pop_front();   //leert die Queue, ueber die der durchschnittwert fuer die minMargin gebildet wird
    }

    EV << localMobilestationInfo.MobileMacAddress << "\n";
}

/**
* Function for Scan, Handover and SNR
*************************************************/

void ControlPlaneMobilestation::handleScanDL_MAPFrame(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handleScanDL_MAPFrame) " << msg << endl;
    Ieee80216DL_MAP* frame = check_and_cast<Ieee80216DL_MAP *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " keine DL-Map.");

    EV << "Aus der Nachricht" << frame->getName() << " die BasestationID:" << frame->
        getBS_ID() << " erhalten.\n";
    EV << "Die Nachricht" << frame->getName() << "besitzt den min. SNR:" << frame->
        getSNR() << " bei max-margin " << localMobilestationInfo.maxMargin << ".\n";

    //breakpoint("handleScanDL_MAP vor if");
    scanSnrVector.record(frame->getSNR());
    x_posVector.record(frame->getXPos());
    y_posVector.record(frame->getYPos());
    abstandVector.record(frame->getAbstand());

    //recordData(msg);

    scanSNRQueue.push_back(frame->getSNR());
}

void ControlPlaneMobilestation::handleCorrectDLMAP(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::handleCorrectDLMAP) " << msg << endl;
    Ieee80216DL_MAP *frame = check_and_cast<Ieee80216DL_MAP *>(msg);
    //if(frame->getBS_ID() == localMobilestationInfo.activeBasestation)
    getCalculateSNR(msg);
    EV << "Basisstation MAC:" << frame->getBS_ID() << ".\n";
    recordData(msg);

    ++(localMobilestationInfo.dlmapCounter);
    EV << " DL-MAP Counter: " << localMobilestationInfo.dlmapCounter << "\n";
}

void ControlPlaneMobilestation::getCalculateSNR(cMessage *msg)
{
    ev << "(in ControlPlaneMobilestation::getCalculateSNR) " << msg << endl;
    Ieee80216GenericMacHeader *frame = check_and_cast<Ieee80216GenericMacHeader *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Nachricht: ", msg->getName(), " kein Generic MAC.");

    //record parameter
    //recordData(msg);
    receiverSnrVector.record(frame->getSNR());
    abstandVector.record(frame->getAbstand());
    x_posVector.record(frame->getXPos());
    y_posVector.record(frame->getYPos());

    //print parameter
    EV << "Berechne SNR" << "\n";
    EV << " Messure SNR: " << frame->getSNR() << "\n";
    EV << " Max. Margin: " << localMobilestationInfo.maxMargin << "\n";
    EV << " Min. Margin: " << localMobilestationInfo.minMargin << "\n";
    EV << " ScanModus: " << localMobilestationInfo.scanModus << "\n";
    EV << " oldReceivSNR: " << localMobilestationInfo.oldReceivSNR << "\n";
    //EV << " oldReceivSNR: "<< localScanningInfo.oldReceivSNR <<"\n";

    double gemittelterSNR;
    double Summe;
    Summe = 0;
    gemittelterSNR = 0;

    int zaehler;
    zaehler = 0;

    SNRQueue.push_back(frame->getSNR());

    //Es wird der mittlere SNR von "FrameAnzahl" Frames bestimmt
    if ((int)SNRQueue.size() >= FrameAnzahl)
    {
        for (SNRList::const_iterator ci = SNRQueue.begin(); ci != SNRQueue.end(); ++ci)
        {
            Summe = Summe + *ci;
            ++zaehler;
            EV << "SNR " << zaehler << " : " << *ci << "\n";
        }

        gemittelterSNR = Summe / FrameAnzahl;
        SNRQueue.pop_front();

        EV << " Gemittelter SNR: " << gemittelterSNR << " Frame Anzahl. " << FrameAnzahl << "\n";
        gemitelterSnrVector.record(gemittelterSNR);

        //if (frame->getMinSNR() <= localMobilestationInfo.minMargin && scan_request_sent == false && fsm.getState()==CONNECT)
        if (gemittelterSNR <= localMobilestationInfo.minMargin && scan_request_sent == false
            && fsm.getState() == CONNECT)
        {
            //if (localMobilestationInfo.oldReceivSNR >= frame->getSNR())
            if (localMobilestationInfo.oldReceivSNR >= gemittelterSNR)
            {
                EV << " Messure SNR: " << frame->
                    getSNR() << " << oldReceivSNR: " << localMobilestationInfo.oldReceivSNR << "\n";
                //breakpoint("minMargin");
                scan_request_sent = true;
                localMobilestationInfo.activeRecvSNR = frame->getSNR();
                startTimerVec.record(frame->getSNR());
                if (startHO_bool == false)
                {
                    startHO_bool = true;
                    startHoTime = simTime().dbl();
                    recordScalar("Mark minSNR", startHoTime);
                    HO_minMarginTime = simTime().dbl();
                }
                build_MOB_SCN_REQ();
            }

            localMobilestationInfo.oldReceivSNR = frame->getSNR(); //speichere Wert für späteren vergleich mit naechsten Wert
        }
        else
        {
            EV << " Messure SNR: " << frame->
                getSNR() << "< Min. Margin: " << localMobilestationInfo.minMargin << "\n";
        }
    }
}

void ControlPlaneMobilestation::startScanmodus()
{
    //breakpoint("startScanmodus");
    ev << "(in ControlPlaneMobilestation::startScanmodus) " << endl;
    if (ev.isGUI())
    {
        getParentModule()->getParentModule()->getParentModule()->bubble("Start Scan");
    }
    EV << "ActiveMobilestationInfo DownlinkChannel: " << activeMobilestationInfo.
        DownlinkChannel << endl;
    EV << "LocalMobilestationInfo DownlinkChannel: " << localMobilestationInfo.
        DownlinkChannel << endl;

    activeMobilestationInfo = localMobilestationInfo; //Sichern der Parameter der Basisstation mit der die MS verbunden ist!!

    EV << "ActiveMobilestationInfo DownlinkChannel: " << activeMobilestationInfo.
        DownlinkChannel << endl;
    EV << "LocalMobilestationInfo DownlinkChannel: " << localMobilestationInfo.
        DownlinkChannel << endl;

    //localScanningInfo.scanChannel = ++localMobilestationInfo.DownlinkChannel;
    //localMobilestationInfo = targetMobilestationInfo;

    targetMobilestationInfo.scanModus = true;
    scanningMobilestationInfo.scanModus = true;
    //localMobilestationInfo.DownlinkChannel = targetMobilestationInfo.DownlinkChannel;

    localMobilestationInfo.DownlinkChannel = scanningMobilestationInfo.DownlinkChannel;
    scheduleAt(simTime() +
               localMobilestationInfo.scanDuration * localMobilestationInfo.DLMAP_interval,
               scanDurationTimer);

    ++ScanCounter;
    EV << "Mark Scan_Counter: " << ScanCounter << endl;
    //cancelEvent(ScanChannelTimer);
    //scheduleAt(simTime(), ScanChannelTimer);
    scheduleAt(simTime(), startHandoverScan);

}

void ControlPlaneMobilestation::stopScanmodus()
{
    ev << "(in ControlPlaneMobilestation::stopScanmodus) " << endl;
    //breakpoint("stopScan");
    if (ev.isGUI())
    {
        getParentModule()->getParentModule()->getParentModule()->bubble("Stop Scan");
    }

    double gemittelterSNR;
    double Summe;
    int Anzahl;
    Anzahl = 0;
    Summe = 0;
    gemittelterSNR = 0;

    if (scanSNRQueue.empty() == false)
    {
        //breakpoint("stopScan berechene SNR");
        for (scanSNRList::const_iterator ci = scanSNRQueue.begin(); ci != scanSNRQueue.end(); ++ci)
        {
            ++Anzahl;
            Summe = Summe + *ci;
            EV << "SNR " << Anzahl << " : " << *ci << "\n";
        }

        while (scanSNRQueue.empty() == false)
        {
            scanSNRQueue.pop_front();
        }

        gemittelterSNR = Summe / Anzahl;
        gemitelterScanSnrVector.record(gemittelterSNR);

        EV << "gemitteter Scan SNR : " << gemittelterSNR << " = " << Summe << " / " << Anzahl << "\n";

        localMobilestationInfo.targetRecvSNR = gemittelterSNR;

        if (gemittelterSNR >= localMobilestationInfo.maxMargin)
        {
            recordScalar("BS scan true", gemittelterSNR);
            scanBStrue.record();
            recordScalar("BS scan Frame Counter", Anzahl);
            scanBSFrameCounter.record(); //recordScalar();
        }
        else
        {
            recordScalar("BS scan false", gemittelterSNR);
            scanBStrue.record();
            recordScalar("BS scan Frame Counter", Anzahl);
            scanBSFrameCounter.record();
        }
    }
    else
    {
        //breakpoint("stopScan else SNR = 0");
        localMobilestationInfo.targetRecvSNR = 0;
    }

    //cancelEvent(ScanChannelTimer);
    //scheduleAt(simTime(), ScanChannelTimer);
    EV << "localMobilestationInfo DownlinkChannel " << localMobilestationInfo.
        DownlinkChannel << endl;
    EV << "localMobilestationInfo UplinkChannel " << localMobilestationInfo.UplinkChannel << endl;

    //targetMobilestationInfo = localMobilestationInfo;
    //if (localMobilestationInfo.targetRecvSNR > scanningMobilestationInfo.targetRecvSNR)
    scanningMobilestationInfo = localMobilestationInfo;
    localMobilestationInfo = activeMobilestationInfo;
    targetMobilestationInfo = scanningMobilestationInfo;

    //localMobilestationInfo.scanModus = false;

    changeDownlinkChannel(localMobilestationInfo.DownlinkChannel);
    changeUplinkChannel(localMobilestationInfo.UplinkChannel);
    //fsm.setState(CONNECT);
    EV << "activeMobilestationInfo DownlinkChannel: " << activeMobilestationInfo.
        DownlinkChannel << endl;
    EV << "aciveMobilestationInfo UplinkChannel " << activeMobilestationInfo.UplinkChannel << endl;
    EV << "targetMobilestationInfo DownlinkChannel: " << targetMobilestationInfo.
        DownlinkChannel << endl;
    EV << "targetMobilestationInfo UplinkChannel " << targetMobilestationInfo.UplinkChannel << endl;
    EV << "localMobilestationInfo DownlinkChannel " << localMobilestationInfo.
        DownlinkChannel << endl;
    EV << "localMobilestationInfo UplinkChannel " << localMobilestationInfo.UplinkChannel << endl;
    EV << "localMobilestationInfo minMargin " << localMobilestationInfo.minMargin << endl;
    EV << "targetMobilestationInfo targetRecvSNR " << targetMobilestationInfo.targetRecvSNR << endl;
    EV << "ScanIteration Counter: " << localScanIteration << endl;
    EV << "ScanIteration: " << localMobilestationInfo.scanIteration << endl;

    //breakpoint("StopScanmodus");

    ++localScanIteration;       //Zaehlt die Scanning durchlaeufe

    recordStopScanningMark();   //Markierung ende eines Scans

    if (targetMobilestationInfo.targetRecvSNR >= localMobilestationInfo.maxMargin)
    {
        //breakpoint("startHO");
        HO_endScanningTime = simTime().dbl();
        recordScalar("HO Scanning Time", HO_endScanningTime - HO_minMarginTime);
        HandoverTime.record();
        ScanIterationBool = false;
        build_MOB_MSHO_REQ();
    }
    else
    {
        if (localMobilestationInfo.scanIteration >= localScanIteration)
        {
            //Wiederholt Abtastung, wenn zuvor keine BS gefunden wurde
            //breakpoint("Scanning repeat");
            //HO_ScnTime = simTime();
            //recordScalar("HO Scanning Time", HO_ScnTime-HO_minMarginTime);//Dauer des Scanning-Vorgangs
            scheduleAt(simTime() +
                       localMobilestationInfo.interleavingInterval *
                       localMobilestationInfo.DLMAP_interval, scanIterationTimer);
            cancelEvent(ScanChannelTimer);
            scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval, ScanChannelTimer);
        }
        else
        {
            //Beendet Abtastung, wenn keine BS gefunden wurde
            localScanIteration = 0;
            ScanIterationBool = false;
            localMobilestationInfo.scanModus = false;
            scan_request_sent = false;
            cancelEvent(ScanChannelTimer);
            scheduleAt(simTime() + localMobilestationInfo.ScanTimeInterval, ScanChannelTimer);
        }
    }
}

void ControlPlaneMobilestation::makeHandover()
{
    ev << "(in ControlPlaneMobilestation::makeHandover) " << endl;
    //breakpoint("makeHandover");
    HO_makeHandover = simTime().dbl();
    recordScalar("HO make Handover Time", HO_makeHandover - HO_endScanningTime);
    recordScalar("Mark make Handover", simTime());
    HandoverTime.record();
    localMobilestationInfo = targetMobilestationInfo;
    targetMobilestationInfo.recvSNR = 0;
    changeDownlinkChannel(localMobilestationInfo.DownlinkChannel);

    EV << "Make Handover scanModus: " << localMobilestationInfo.scanModus << endl;
    scan_request_sent = false;
    localMobilestationInfo.scanModus = false;
    startHO_bool = false;
    EV << "Make Handover scanModus: " << localMobilestationInfo.scanModus << endl;
    resetStation();

    ScanCounter = 0;

    ++HandoverCounter;
    HandoverCounterVec.record(HandoverCounter);

    //changeUplinkChannel(localMobilestationInfo.UplinkChannel);
    //breakpoint("startHO");
    //localMobilestationInfo.scanModus = false;
    //localScanningInfo.activeScan = false;
    //cancelEvent(ScanChannelTimer);
    //scheduleAt(simTime(), ScanChannelTimer);
    //scheduleAt(simTime()+localMobilestationInfo.ScanTimeInterval, ScanChannelTimer);
    scheduleAt(simTime(), startHandover);
}

bool ControlPlaneMobilestation::isHandoverDone() //Informiert ConvergenceSublayerTrafficClassification ueber einen Handover
{
    return makeHandover_cdTrafficBool;
}

void ControlPlaneMobilestation::setHandoverDone() //Informiert ConvergenceSublayerTrafficClassification ueber einen Handover
{
    makeHandover_cdTrafficBool = false;
}

void ControlPlaneMobilestation::recordData(cMessage *msg)
{
    Ieee80216DL_MAP* frame = check_and_cast<Ieee80216DL_MAP *>(msg);
    if (!frame)
        error("Error im Module", getName(), "Funktion recordData ", msg->getName(),
              " keine DL-Map.");

    MACAddress BS_1;
    MACAddress BS_2;
    MACAddress BS_3;
    MACAddress BS_4;
    MACAddress BS_5;
    MACAddress BS_6;
    MACAddress BS_7;

    BS_1 = MACAddress("0A:00:00:00:00:01");
    BS_2 = MACAddress("0A:00:00:00:00:02");
    BS_3 = MACAddress("0A:00:00:00:00:03");
    BS_4 = MACAddress("0A:00:00:00:00:04");
    BS_5 = MACAddress("0A:00:00:00:00:05");
    BS_6 = MACAddress("0A:00:00:00:00:06");
    BS_7 = MACAddress("0A:00:00:00:00:07");

    if (frame->getBS_ID() == BS_1)
    {
        BS1SNR.record(frame->getSNR());

    }
    else if (frame->getBS_ID() == BS_2)
    {
        BS2SNR.record(frame->getSNR());
    }
    else if (frame->getBS_ID() == BS_3)
    {
        BS3SNR.record(frame->getSNR());
    }
    else if (frame->getBS_ID() == BS_4)
    {
        BS4SNR.record(frame->getSNR());
    }
    else if (frame->getBS_ID() == BS_5)
    {
        BS5SNR.record(frame->getSNR());

    }
    else if (frame->getBS_ID() == BS_6)
    {
        BS6SNR.record(frame->getSNR());
    }
    else if (frame->getBS_ID() == BS_7)
    {
        BS7SNR.record(frame->getSNR());

    }

    x_posVector.record(frame->getXPos());
    y_posVector.record(frame->getYPos());
    abstandVector.record(frame->getAbstand());
    //rcvdPowerVector.record(10*log10(frame->getRcvdPower()));
    //therNoiseVector.record(10*log10(frame->getThermNoise()));
    snrVector.record(10 * log10(frame->getRcvdPower()) - 10 * log10(frame->getThermNoise()));
}

void ControlPlaneMobilestation::recordStartScanningMark()
{
    switch (ScanCounter)
    {
    case 1:
        recordScalar("Mark StartScan_1", simTime());
        break;

    case 2:
        recordScalar("Mark StartScan_2", simTime());
        break;

    case 3:
        recordScalar("Mark StartScan_3", simTime());
        break;

    case 4:
        recordScalar("Mark StartScan_4", simTime());
        break;

    case 5:
        recordScalar("Mark StartScan_5", simTime());
        break;

    case 6:
        recordScalar("Mark StartScan_6", simTime());
        break;
    }
}

void ControlPlaneMobilestation::recordStopScanningMark()
{
    switch (ScanCounter)
    {
    case 1:
        recordScalar("Mark StopScan_1", simTime());
        break;

    case 2:
        recordScalar("Mark StopScan_2", simTime());
        break;

    case 3:
        recordScalar("Mark StopScan_3", simTime());
        break;

    case 4:
        recordScalar("Mark StopScan_4", simTime());
        break;

    case 5:
        recordScalar("Mark StopScan_5", simTime());
        break;

    case 6:
        recordScalar("Mark StopScan_6", simTime());
        break;
    }
}
