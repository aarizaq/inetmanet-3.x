//
// Autor Roland Siedlaczek
//
// In dem Module "CommonPartSublayerTransceiver" ist der Teil von Mechanismen des IEEE802.16e Sublayer implementiert,
// die fuer die Bearbeitung von MAC.Packeten benoetigt die vom Hoeheren Layer (Layer-3) ankommen und an den Untereren Layer (Layer-1)
// weiter gesendet werden       |
// Managementnachrichten des Protokolls IEEE802.16e, die von dem "ControlPlane**" Module gesendet werden weitergeleitet
// bzw. abgefangen und an das "ControlPlane**" Module weitergeleitet.
// Bei Managementnachrichten die von Hoeheren Layern verschickt worden.sind in diesem Fall von Basisstationen ueber das Netz verschickt.
//
//
//
#include "CommonPartSublayerTransceiver.h"

Define_Module(CommonPartSublayerTransceiver);

CommonPartSublayerTransceiver::CommonPartSublayerTransceiver()
{
    nextControlQueueElement = NULL;
    nextDataQueueElement = NULL;
    mediumStateChange = NULL;
    endTimeout = NULL;
    pendingRadioConfigMsg = NULL;
    stopTransmision = NULL;
    allowedToSend = false;
    transmitControlPDU = false;
    transmitDataPDU = false;

    last_floor = -1;
    uplink_rate = 0;
    missed_cqe_counter = 0;
}

CommonPartSublayerTransceiver::~CommonPartSublayerTransceiver()
{
    cancelAndDelete(nextControlQueueElement);
    cancelAndDelete(nextDataQueueElement);
    cancelAndDelete(mediumStateChange);
    cancelAndDelete(endTimeout);
    cancelAndDelete(pendingRadioConfigMsg);
    cancelAndDelete(stopTransmision);
    while (!requestMsgQueue.empty())
    {
        delete requestMsgQueue.back();
        requestMsgQueue.pop_back();
    }
    while (!dataMsgQueue.empty())
    {
        delete dataMsgQueue.back();
        dataMsgQueue.pop_back();
    }
}

void CommonPartSublayerTransceiver::initialize()
{
    /**
    * Hilfs-Nachrichten zum ausloesen von Ereignissen (Events)
    *
    ******************************************************************************************/
    nextControlQueueElement = new cMessage("nextControlQueueElement");

    mediumStateChange = new cMessage("mediumStateChange");
    endTimeout = new cMessage("Timeout");
    //stopTransmision = new cMessage("stopTransmision");

    /**
    * Hier werden die GateID der Gates abgefragt und in den Int. Variablen abgespeichert.
    * Das Abfragen der Ein- und Ausgaenge erfolgt dannach nur ueber die GateId. Damit wird der
    * Maschienencode optimiert.
    ******************************************************************************************/
    controlPlaneGateIn = findGate("controlPlaneGateIn");
    controlPlaneGateOut = findGate("controlPlaneGateOut");
    schedulingGateIn = findGate("schedulingGateIn");
    fragmentationGateOut = findGate("fragmentationGateOut");

    sfOut = findGate("sfOut");
    sfIn = findGate("sfIn");

    // state variable
    fsmb.setName("TransmitState");

    nb = NotificationBoardAccess().get(); // Erhoeht den Zeiger vom NotificationBoard Modul
    nb->subscribe(this, NF_RADIOSTATE_CHANGED); // Soll  auf Aenderungen des Radio Modul reagieren.
    //initializeQueueModule();
    cModule *module_transceiver_Radio =
        getParentModule()->getParentModule()->getSubmodule("radioTransceiver");
    radioId = module_transceiver_Radio->getId();

    /**
     * get the control plane module for the correct station type
     */
    cModule *tmp_controlplane =
        getParentModule()->getParentModule()->getParentModule()->getSubmodule("controlPlane")->
        getSubmodule("cp_mobilestation");
    if (tmp_controlplane != 0)
    {
        controlplane = dynamic_cast<ControlPlaneMobilestation *>(tmp_controlplane);
    }
    else
    {
        tmp_controlplane =
            getParentModule()->getParentModule()->getParentModule()->getSubmodule("controlPlane")->
            getSubmodule("cp_basestation");
        if (tmp_controlplane != 0)
        {
            controlplane = dynamic_cast<ControlPlaneBasestation *>(tmp_controlplane);

            controlplane->setDataMsgQueue(&dataMsgQueue);
        }
    }

    per_second_timer = new cMessage("per_second_timer");
    scheduleAt(simTime(), per_second_timer);

    cvec_ugs_perSecond.setName("ugs traffic rate");
    cvec_rtps_perSecond.setName("rtps traffic rate");
    cvec_ertps_perSecond.setName("ertps traffic rate");
    cvec_nrtps_perSecond.setName("nrtps traffic rate");
    cvec_be_perSecond.setName("be traffic rate");
    cvec_throughput_uplink.setName("thoughput uplink");

    cvec_sum_ugs.setName("ugs traffic thhoughtput");
    updateDisplay();
}

/**
* Nachrichten (Messages) die das Module erreichen, loesen als erstes die handleMessage() Funktion aus.
* Die handleMessage() Funktion ist teil des cSimpleModule. In dieser Funktion wird ueberprueft an welchem Gate Eingang die
* Nachricht das Module erreicht hat.
*
* Das Module "CommonPartSublayerTransceiver" besitzt folgende Eingaenge:
* convergenceGateIn -  hier werden Nachrichten empfangen die vom Convergence Module (Hoehere Schicht) versendet wurden
* controlGateIn- hier werden Nachrichten empfangen die vom ControlPlane Module versendet wurden.
* Es kann sich dabei um die Control Plane der Basisstation oder der Mobilestation handeln.
* Als Letztes koenne die Nachrichten von diesem Module an selbst gesendet worden sein (selfMessage)
*
*/
void CommonPartSublayerTransceiver::handleMessage(cMessage *msg)
{
    //if (dataMsgQueue.size() == 20)
    // breakpoint("Danger: Queue overflow!");

    EV << "(in CommonPartSublayerTransceiver::handleMessage) Message  " << msg->getName() << endl; // <<", Laenge der Nachricht: " << tempPtr->getByteLength() << endl;

    if (msg->getArrivalGateId() == controlPlaneGateIn) //Nachricht vom Conrol Plane
    {
        EV << "von controlPlaneGateIn " << msg <<
            " an die Funktion handleControlPlaneMsg() gesendet.\n";
        handleControlPlaneMsg(msg);
        return;
    }

    if (msg->getArrivalGateId() == schedulingGateIn) // Nachricht vom CS
    {
        EV << "von schedulingGateIn " << msg <<
            " an die Funktion handleUpperSublayerMsg() gesendet.\n";
        handleUpperSublayerMsg(msg);
        return;
    }

    if (msg->isSelfMessage())
    {
        EV << "is Selfmessage: " << msg->getName() << endl;
        if (msg == per_second_timer)
        {
            recordDatarates();
            scheduleAt(simTime() + 1, per_second_timer);
            return;
        }

        {
            cPolymorphic *c_info = msg->getControlInfo();
            Ieee80216Prim_sendControlRequest *sc_req;
            int cid = -1;
            if (c_info)
            {
                sc_req = dynamic_cast<Ieee80216Prim_sendControlRequest *>(c_info);
                cid = sc_req->getPduCID();
                EV << "sendControlRequest : PDU-CID:" << cid << "(realCID:" << sc_req->getRealCID() <<
                    "), Bits for burst: " << sc_req->getBits_for_burst() << " bits\n";

                if (!managementQueuesEmpty(sc_req))
                {
                    handleWithFSM(msg);
                    return;
                }
            }
        }
        return;
    }

	error("Unhandled message: %s (type: %s)",msg->getName(),msg->getClassName());
	delete msg;
}

/**
* Funktion zum Abarbeiten von Nachrichten, die vom
* Service Specific Convergence Sublayer Modul gesendet wurden.
*
*
****************************************************************************/
void CommonPartSublayerTransceiver::handleUpperSublayerMsg(cMessage *msg)
{
    ev << "(in CommonPartSublayerTransceiver::handleUpperSublayerMsg) Message " <<
        msg->getName() << " eingetroffen" << endl;
    Ieee80216GenericMacHeader *frame = check_and_cast<Ieee80216GenericMacHeader *>(msg);
// if ( frame->controlInfo() != NULL )
//  breakpoint("control info noch da");

    dataMsgQueue.push_back(frame);

    updateDisplay();
}

/**
* Funktion zum Abarbeiten von Nachrichten, die vom Control Plane Modul
* gesendet wurde. Bei diesen Nachrichten kann es sich um zwei Typen von Nachrichten Handeln.
* Es koennen einerseits Managementnachrichten des IEEE802.16e Standart sein die in Richtung
* Funkkanal versand werden.
* Oder es handelt sich um Befehle (Kontrol-Nachrichten) fuer das Transceiver Radio Module.
* Diese sind nicht Bestandteil des Ieee802.16e Standards.
*
*********************************************************************************************/
void CommonPartSublayerTransceiver::handleControlPlaneMsg(cMessage *msg)
{
    // Kontrol-Nachrichten haben keine Laenge aber ein "kind()"
    if (!msg->isPacket() && msg->getKind() != 0) //VITA geaendert: statt !msg->isPacket() war zuvor msg->getByteLength==0
    {
        EV << "(in CommonPartSublayerTransceiver::handleControlPlaneMsg) eingegangene Nachricht is eine Kontroll-Nachricht. Sende an handleCommand().\n";
        handleCommand(msg);
        return;
    }
    else if (msg->isPacket())
    {
        EV << "(in CommonPartSublayerTransceiver::handleControlPlaneMsg) eingegangene Nachricht is ein Paket.\n";
        Ieee80216MacHeader *frame = check_and_cast<Ieee80216MacHeader *>(msg);
        // EV << "\n\n\nCONTROLPLANE VON: " << controlplane->getStationType() << "\n\n";
        // mk: hier entsprechend nach der management CID in die entsprechenden queues leiten
        if (controlplane->getStationType() == MOBILESTATION)
        {
            EV << "\n\n\nCONTROLPLANE VON: " << controlplane->getStationType() << "MOBILESTATION.\n\n";
            // TODO BW-REQ ueber normales interval verschicken, wenn vorhanden, sonst request-interval
            if (dynamic_cast<Ieee80216BandwidthMacHeader *>(msg))
            {
                EV << "ist ein Ieee80216BandwidthMacHeader";
                //primaryCIDQueue.push_back(frame);
                // FIXME: should not be in primaryCIDQueue. is a workaround until
                // broadcast contention really works...
                if (!controlplane->stationHasUplinkGrants)
                {
                    EV << " , keine UplinkGrants -> in die Warteschlange requestMsgQueue.\n";
                    requestMsgQueue.push_back(frame);
                }
                else
                {
                    EV << " , UplinkGrants vorhanden -> in die Warteschlange basicCIDQueue.\n";
                    basicCIDQueue.push_back(frame);
                }
            }
            else
            {
                EV << msg << " ist KEIN Ieee80216BandwidthMacHeader.  ";
                switch (controlplane->getManagementType(frame->getCID()))
                {
                case BASIC:
                    EV << " in die basicCIDQueue gepushed";
                    basicCIDQueue.push_back(frame);
                    break;

                case PRIMARY:
                    EV << " in die primaryCIDQueue gepushed";
                    primaryCIDQueue.push_back(frame);
                    break;

                case SECONDARY:
                    // sollte nicht von der controlplane kommen. secondary cids sind daten cids und kommen vom scheduler oben.
                    EV << "\n\nDIESE NACHRICHT SOLLTE NICHT AN DIE SECONDARY-CID ADRESSIERT SEIN!! -> "
                        << frame << "\n\n";
                    break;

                default:
                    //fuer alles andere (ranging nachrichten)
                    EV << " -> in die controlMsgQueue gepushed" << endl;
                    controlMsgQueue.push_back(frame);
                    break;
                }
            }
        }
        else if (controlplane->getStationType() == BASESTATION)
        {
            EV << "\n\n\nCONTROLPLANE VON: " << controlplane->getStationType() << " BASESTATION.\n\n";
            if (dynamic_cast<Ieee80216DL_MAP *>(frame)
                || dynamic_cast<Ieee80216UL_MAP *>(frame)
                || dynamic_cast<Ieee80216_DCD *>(frame)
                || dynamic_cast<Ieee80216_UCD *>(frame))
            {
                requestMsgQueue.push_back(frame);
            }
            else
                dataMsgQueue.push_back(frame);
        }

        updateDisplay();
    }
    else if (!msg->isPacket() && msg->getKind() == 0) //VITA geaendert: war davor else if ( msg->getByteLength==0 && msg->getKind() == 0)
    {
        EV << "Message " << msg->getName() <<
            " mit Kind=0 an die Funktion CommonPartSublayerTransceiver::handleControlRequest() gesendet.\n";
        handleControlRequest(msg);
    }
    else
    {
        error
            ("CommonPartSublayerTransceiver::handleControlPlaneMsg: Nicht erkannte Nachricht erhalten: (%s)%s msgkind=%d",
             msg->getClassName(), msg->getName(), msg->getKind());
    }

}

/**
*
* Mit dieser Funktion werden Hilfsnachrichten der Control Plane bearbeitet.
*
*********************************************************************************************/
void CommonPartSublayerTransceiver::handleControlRequest(cMessage *msg)
{
    //int msgkind = msg->kind();
    EV << "(in CommonPartSublayerTransceiver::handleControlRequest) message " << msg->getName() <<
        " eingetroffen" << endl;
    cPolymorphic *ctrl = msg->removeControlInfo();
    delete msg;

    if (dynamic_cast<Ieee80216Prim_sendControlRequest *>(ctrl))
    {
        allowedToSend = true;
        updateDisplay();

        Ieee80216Prim_sendControlRequest *send_ctrl =
            dynamic_cast<Ieee80216Prim_sendControlRequest *>(ctrl);

        // debug
        cPolymorphic *bla = nextControlQueueElement->removeControlInfo();
        if (bla)
        {
            Ieee80216Prim_sendControlRequest *blub =
                check_and_cast<Ieee80216Prim_sendControlRequest *>(bla);
            EV << "nextControlQueueElement ALT CID: " << blub->getPduCID() << "\n";
            delete bla;
        }
        else
            EV << "nextControlQueueElement ALT CID: keine CONTROLINFO in der Nachricht vorhanden\n";
        // end debug

        nextControlQueueElement->setControlInfo(send_ctrl);
        EV << "nextControlQueueElement fuer: PDU-CID:" << send_ctrl->getPduCID() << "(realCID:" <<
            send_ctrl->getRealCID() << ") scheduled at " << simTime() << "\n";

        if (nextControlQueueElement->isScheduled())
            cancelEvent(nextControlQueueElement);

        scheduleAt(simTime(), nextControlQueueElement);
    }
    else if (dynamic_cast<Ieee80216Prim_stopControlRequest *>(ctrl))
    {
        //Ieee80216Prim_stopControlRequest *stopControlRequest = dynamic_cast<Ieee80216Prim_stopControlRequest *>(ctrl);
        // scheduleAt(simTime()+stopControlRequest->getTimeInterval(),stopTransmision);
        allowedToSend = false;
        cancelEvent(nextControlQueueElement);

//     if ( dynamic_cast<Ieee80216Prim_stopControlRequest *>(ctrl)->getPduCID() != 0xFFFF )
//      flushTransmissionBuffer();

        updateDisplay();
        delete ctrl;
    }
/*
    else if (dynamic_cast<Ieee80216Prim_sendDataRequest *>(ctrl))
    {
        allowedToSend = true;
        handleWithFSM(nextDataQueueElement);
    }
    else if (dynamic_cast<Ieee80216Prim_stopDataRequest *>(ctrl))
    {
        allowedToSend = false;
    }
*/
    else
    {
        error("In Control Request nicht enthalten: (%s)%s msgkind=%d", msg->getClassName(),
              msg->getName(), msg->getKind());
        delete ctrl;
    }
}

/**
*
* Mit dieser Funktion werden Kontrol-Nachrichten an das Radio Transceiver Module bearbeitet.
*
*********************************************************************************************/
void CommonPartSublayerTransceiver::handleCommand(cMessage *msg)
{
    EV << "(in CommonPartSublayerTransceiver::handleCommand) message " << msg->getName() <<
        " eingetroffen.\n";
    if (msg->getKind() == PHY_C_CONFIGURERADIO)
    {
        EV << "Empfange Kontrolnachricht " << msg->getName() << " fuer das Transceiver-Radio-Modul.\n";
        if (pendingRadioConfigMsg != NULL)
        {
            EV << "pendingRadioConfigMsg != NULL, d.h. es gibt noch alte Kommands f�r das RadioModul.\n";
            // wenn es alte Befehle fuer das Radio Module gibt die nicht versendet werden konnten. Werden die mit den neuen Werten abgeglichen
            PhyControlInfo *pOld =
                check_and_cast<PhyControlInfo *>(pendingRadioConfigMsg->getControlInfo());
            PhyControlInfo *pNew = check_and_cast<PhyControlInfo *>(msg->getControlInfo());
            if (pNew->getChannelNumber() == -1 && pOld->getChannelNumber() != -1)
                pNew->setChannelNumber(pOld->getChannelNumber());
            if (pNew->getBitrate() == -1 && pOld->getBitrate() != -1)
                pNew->setBitrate(pOld->getBitrate());
            delete pendingRadioConfigMsg;
            pendingRadioConfigMsg = NULL;
        }

        if (fsmb.getState() == START)
        {
            EV << "Sende Kontrolnachricht sofort an das Radio-Module\n";

            send(msg, "fragmentationGateOut");
        }
        else
        {
            EV << "Sende Kontrolnachricht " << msg->getName() << " spaeter an das Radio Modul.\n";
            pendingRadioConfigMsg = msg;
        }
    }
    else
    {
        error("Nicht erkannte Nachricht erhalten: (%s)%s msgkind=%d", msg->getClassName(),
              msg->getName(), msg->getKind());
    }
}

/**
*
* Mit dieser "state machine" wird der Zugriff auf das Radio Module gesteuert.
*
* Anmerkung MK:  die state machine flushed die queues gnadenlos,
* daher muss im scheduling modul ein puffer eingebaut sein, der immer genau
* so viele pakete zum transceiver schiebt, wie im naechsten DL-Burst platz ist.
*/

void CommonPartSublayerTransceiver::handleWithFSM(cMessage *msg)
{
    FSM_Switch(fsmb)
    {
    case FSM_Enter(START):
        sendDownPendingRadioConfigMsg();
        break;

    case FSM_Exit(START):
        EV << "name:" << msg->getName() << "  free:" << isMediumFree() << "\n";
        if (isNextControlQueueElement(msg) && isMediumFree() && isAllowedToSend())
        {
            sendDown(currentManagementMsgQueue());

            FSM_Goto(fsmb, CONTROLSEND);
        }
        break;

    case FSM_Enter(CONTROLSEND):
        // scheduleAt( simTime(), nextControlQueueElement );  //VITA geaendert: davor war diese Zeile auskommentiert
        break;

        // wait until previous message is sent and medium is free again
        // before popping the queue and scheduling the next element in line
    case FSM_Exit(CONTROLSEND):
        if (isMediumStateChange(msg) && isMediumFree())
        {
            popManagementMsgQueue();
            EV << "Nochens datt nCQE...\n";
            if (controlplane->getStationType() == BASESTATION)
                scheduleAt(simTime(), nextControlQueueElement);
            else if (!nextControlQueueElement->isScheduled())
            {
                scheduleAt(simTime(), nextControlQueueElement);
            }

            FSM_Goto(fsmb, START);
        }
        break;
    }
}

/*
void CommonPartSublayerTransceiver::handleWithFSM(cMessage *msg)
{
    EV << "FSM STATUS: "<< fsmb.getStateName() <<"\n";
    FSMA_Switch(fsmb)
    {

        EV << "Die Station " << parentModule()->parentModule()->name() << " startet die State Machine zum senden von Daten.\n";
        EV << "Control Queue:" << isNextControlQueueElement(msg) <<"name " << msg->name() << " Empty" << "N/A Medium" << isMediumFree() << " aloowed to send: " << isAllowedToSend()<<"\n";
        FSMA_State(START)
        {
            FSMA_Enter(sendDownPendingRadioConfigMsg());//Vorhandene Radioeinstellungen

            FSMA_Event_Transition(Start-Send-Management-Msg,
                    isNextControlQueueElement(msg) && isMediumFree() && isAllowedToSend(),
                          CONTROLSEND,
                          EV << "FSMA: send A\n";
                          sendDown(currentManagementMsgQueue());
            );

            FSMA_No_Event_Transition(Start-Send-Management-Msg-b,
                    isNextControlQueueElement(msg)  && isMediumFree(),
                          CONTROLSEND,
                          EV << "FSMA: send B\n";
                          sendDown( currentManagementMsgQueue() );
            );
        }

        FSMA_State(CONTROLSEND)
        {
            FSMA_Event_Transition(Radio-Change,
                          isMediumStateChange(msg) && isMediumFree(),
                          CONTROLQUEUE,
                          popManagementMsgQueue();
                          EV << "nextControlQueueElement scheduled in FSM(CONTROLSEND) at " << simTime() << "\n";
                          scheduleAt(simTime(),nextControlQueueElement);
                        );
//            FSMA_Event_Transition(Radio-Change-Send,
//                             isMediumStateChange(msg) && isMediumFree(),
//                              CONTROLQUEUE,
//                              popManagementMsgQueue();
//                              scheduleAt(simTime(),nextControlQueueElement);
//                  );
        }

        FSMA_State(CONTROLQUEUE)
        {
            FSMA_Event_Transition(Check-Queue-Empty,
                             isNextControlQueueElement(msg)  &&  isMediumFree(),
                          START,
                          EV << "FSMA: switch to START\n";
                          EV << "nextControlQueueElement scheduled in FSM(CONTROLQUEUE) at " << simTime() << "\n";
                          scheduleAt(simTime() ,nextControlQueueElement);;
            );
//            FSMA_Event_Transition(Check-ControlQueue,
//                             isNextControlQueueElement(msg) && isMediumFree(),
//                          CONTROLSEND,
//                          EV << "FSMA: switch to CONTROLSEND\n";
//                 sendDown(currentManagementMsgQueue());
//            );
        }
    }
}
*/

/*
// FSM von (mk)
void CommonPartSublayerTransceiver::handleWithFSM(cMessage *msg)
{
    EV << "FSM STATUS: "<< fsmb.getStateName() <<"\n";
    FSMA_Switch(fsmb)
    {
        EV << "Die Station " << parentModule()->parentModule()->name() << " startet die State Machine zum senden von Daten.\n";
        EV << "Control Queue:" << isNextControlQueueElement(msg) <<"name " << msg->name() << " Empty" << "N/A Medium" << isMediumFree() << " aloowed to send: " << isAllowedToSend()<<"\n";

        FSMA_State(START)
        {
            FSMA_Enter(sendDownPendingRadioConfigMsg());//Vorhandene Radioeinstellungen

            FSMA_Event_Transition(Start-Send-Broadcast-Management-Msg,
                        isNextControlQueueElement(msg) && isMediumFree() && isAllowedToSend() && isBroadcastSendRequest(msg),
                        BROADCASTSEND,
                        sendDown(currentManagementMsgQueue());
            );

            FSMA_Event_Transition(Start-Send-Data-Management-Msg,
                        isNextControlQueueElement(msg) && isMediumFree() && isAllowedToSend() && !isBroadcastSendRequest(msg),
                          DATASEND,
                          sendDown(currentManagementMsgQueue());
            );
        }

        FSMA_State(BROADCASTSEND)
        {
            FSMA_Event_Transition(Radio-Change,
                            isMediumStateChange(msg) && isMediumFree(),
                            START,
                            popManagementMsgQueue();
                            scheduleAt(simTime(),nextControlQueueElement);
            );
        }

        FSMA_State(DATASEND)
        {
            FSMA_Event_Transition(Data-Sent,
                            isMediumStateChange(msg) && isMediumFree(),
                            START,
                            popManagementMsgQueue();
            );
        }
    }
}
*/

/**
*
* Diese Funktion wird aufgerufen, wenn sich der Zustand eines Modules innerhalb des uebergeordneten Modules
* aendert.
*
****************************************************************************/
void CommonPartSublayerTransceiver::receiveChangeNotification(int category,
                                                              const cPolymorphic *details)
{
    EV << "(in CommonPartSublayerTransceiver::receiveChangeNotification)" << endl;
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category == NF_RADIOSTATE_CHANGED) // Hiermit werden nur Aenderungen des Radio Modules beruecksichtigt
    {
        //RadioState
        RadioState::State newRadioState = check_and_cast<RadioState *>(details)->getState();
        RadioState *myRadioState = check_and_cast<RadioState *>(details);

        if (myRadioState->getRadioId() == radioId)
        {
            EV << "Debug RS Radio Id: " << myRadioState->getRadioId() << "\n";
            EV << "Debug RS Modul Id: " << radioId << "\n";
            radioState = newRadioState;

            handleWithFSM(mediumStateChange);
        }
    }
}

void CommonPartSublayerTransceiver::sendDown(cMessage *msg)
{
    EV << "(in CommonPartSublayerTransceiver::sendDown)" << endl;

    cPacket *tempPtr = PK(msg);
    if (tempPtr != NULL)
    {
        uplink_rate += tempPtr->getByteLength();

        //double guard_time = msg->length()/8*4E6 ;  //0.000005; // if messages are not sent slightly delayed, sooner or later a collision will occur on the remote side
        double guard_time = 0.000005;
        EV << "Sending " << msg << " to transceiver Radio.\n";

        sendDelayed(msg, guard_time, fragmentationGateOut);
        //send( msg, fragmentationGateOut );

        updateDisplay();
    }
    else
        error("Cannot send NULL down! Check your queues!");
}

/**
 * This method returns the next message from the queues by strict
 * priorities: ranging->basic->primary->data
 */
Ieee80216MacHeader *CommonPartSublayerTransceiver::currentManagementMsgQueue()
{
    EV << "ENTER METHOD currentManagementMSGQueue()\n";

    Ieee80216Prim_sendControlRequest* send_ctrl =
        dynamic_cast<Ieee80216Prim_sendControlRequest *>(nextControlQueueElement->getControlInfo());

    if (controlplane->getStationType() == BASESTATION && send_ctrl)
    {
        EV << "FOR BASESTATION\n";
        if (send_ctrl->getPduCID() != 1025 && !requestMsgQueue.empty())
        {
            queue_to_pop = qREQUEST;
            return (Ieee80216MacHeader*)requestMsgQueue.front();
        }
        else if (send_ctrl->getPduCID() == 1025 && !dataMsgQueue.empty())
        {
            queue_to_pop = qDATA;
            return (Ieee80216MacHeader*)dataMsgQueue.front();
        }
    }

    if (controlplane->getStationType() == MOBILESTATION)
    {
        EV << "FOR MOBILESTATION\n";

        if (send_ctrl != NULL)
        {

            // explicitly pop the request queue, if a sendControlRequest for a broadcast interval has arrived
            // --> but ONLY if the station has no Uplink Grants
            // FIXME: should only be sent if station has no grants
            if (controlplane->stationHasUplinkGrants &&
                send_ctrl->getPduCID() == 0xFFFF && !requestMsgQueue.empty())
            {
                EV << "REQUEST QUEUE\n";
                queue_to_pop = qREQUEST;
                return (Ieee80216MacHeader *) requestMsgQueue.front();
            }
            else if (send_ctrl->getPduCID() == 0x0000 && !controlMsgQueue.empty())
            {
                // otherwise pop the other queues in order of their priority
                EV << "CONTROLMSG QUEUE\n";
                queue_to_pop = qCONTROL;
                return (Ieee80216MacHeader *) controlMsgQueue.front();
            }
            else if (send_ctrl->getPduCID() != 0x0000 && send_ctrl->getPduCID() != 0xFFFF)
            {                   // && send_ctrl->getPduCID() != 0xFFFF ){
                EV << "\n\n\n\n\nREALCID: " << send_ctrl->getRealCID() << "\n\n\n\n\n\n";

                if (!basicCIDQueue.empty()
                    && send_ctrl->getRealCID() == controlplane->getBasicCID())
                {
                    EV << "BASIC QUEUE\n";
                    queue_to_pop = qBASIC;
                    return (Ieee80216MacHeader *) basicCIDQueue.front();
                }
                else if (!primaryCIDQueue.empty()
                         && send_ctrl->getRealCID() == controlplane->getPrimaryCID())
                {
                    EV << "PRIMARY QUEUE\n";
                    queue_to_pop = qPRIMARY;
                    return (Ieee80216MacHeader *) primaryCIDQueue.front();
                }
                else if (!dataMsgQueue.empty() &&
                         (send_ctrl->getRealCID() != controlplane->getBasicCID() &&
                          send_ctrl->getRealCID() != controlplane->getPrimaryCID()))
                {

                    EV << "\n\nREALCID2: " << send_ctrl->getRealCID() << "\n\n";
                    //
                    EV << "DATA QUEUE\n";
                    queue_to_pop = qDATA;
                    //return (Ieee80216MacHeader *)dataMsgQueue.front();
                    return (Ieee80216MacHeader *) packedDataForCID(send_ctrl->getRealCID(),
                                                                   send_ctrl->getBits_for_burst());
                }
            }

//   delete send_ctrl;
        }
        else
            EV << "nextControlQueueElement had no ControlInfo attached!\n";
    }

    EV << "RETURN NULL\n";
    return NULL;
}

void CommonPartSublayerTransceiver::popManagementMsgQueue()
{
    EV << "(in CommonPartSublayerTransceiver::popManagementMsgQueue) " << endl;
    switch (queue_to_pop)
    {
    case qREQUEST:
        if (!requestMsgQueue.empty())
            requestMsgQueue.pop_front();

    case qCONTROL:
        if (!controlMsgQueue.empty())
            controlMsgQueue.pop_front(); // Das vordere Element der Warteschlange wird geloescht
        break;

    case qBASIC:
        if (!basicCIDQueue.empty())
            basicCIDQueue.pop_front();
        break;

    case qPRIMARY:
        if (!primaryCIDQueue.empty())
            primaryCIDQueue.pop_front();
        break;

    case qDATA:
        if (controlplane->getStationType() == BASESTATION)
        {
            if (!dataMsgQueue.empty())
                dataMsgQueue.pop_front();
        }
        else
        {
        }                       //do nothing, because the elements have already been removed in the Mobilestation due to packing
        break;
    }
    updateDisplay();
}

/**
* Mit den folgenden Funktionen wird abgefragt ueber welche Gates die
* Nachrichten empfangen wurden und um welche Nachrichten es sich
* handelt.
*/

bool CommonPartSublayerTransceiver::isUpperSublayerMsg(cMessage *msg)
{
    EV << "(in CommonPartSublayerTransceiver::isUpperSublayerMsg)" << endl;
    return msg->getArrivalGateId() == schedulingGateIn;
}

bool CommonPartSublayerTransceiver::isControlPlaneMsg(cMessage *msg)
{
    EV << "(in CommonPartSublayerTransceiver::isControlPlaneMsg)" << endl;
    return msg->getArrivalGateId() == controlPlaneGateIn;
}

bool CommonPartSublayerTransceiver::isMediumStateChange(cMessage *msg)
{
    EV << "(in CommonPartSublayerTransceiver::isMediumStateChange)" << endl;
    return msg == mediumStateChange;
}

bool CommonPartSublayerTransceiver::isNextControlQueueElement(cMessage *msg)
{
    return msg == nextControlQueueElement;
    //return strcmp(msg->name(), "nextQueueElement") == 0;
}

bool CommonPartSublayerTransceiver::isTimeout(cMessage *msg)
{
    return msg == endTimeout;
}

bool CommonPartSublayerTransceiver::isStopTransmision(cMessage *msg)
{
    return msg == stopTransmision;
}

bool CommonPartSublayerTransceiver::isMagMsg(cMessage *msg)
{
    return dynamic_cast<Ieee80216GenericMacHeader *>(msg);
}

/**
* Ueber diese Funktionen werden die Zustaende des Radio Moduls abgefragt.
*/
bool CommonPartSublayerTransceiver::isMediumFree()
{
    return radioState == RadioState::IDLE;
}

bool CommonPartSublayerTransceiver::isMediumTransmit()
{
    return radioState == RadioState::TRANSMIT;
}

/**
* @brief Funktionen
*/
void CommonPartSublayerTransceiver::sendDownPendingRadioConfigMsg()
{
    if (pendingRadioConfigMsg != NULL)
    {
        sendDown(pendingRadioConfigMsg);
        pendingRadioConfigMsg = NULL;
    }
}

void CommonPartSublayerTransceiver::setAllowedToSend(bool send)
{
    allowedToSend = send;
}

bool CommonPartSublayerTransceiver::getAllowedToSend()
{
    return allowedToSend;
}

bool CommonPartSublayerTransceiver::isAllowedToSend()
{
    if (allowedToSend)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void CommonPartSublayerTransceiver::updateDisplay()
{
    /**
    * Displays the current queue sizes beside the transceiver module, separated by the station type
    */
    char buf[90];
    if (controlplane->getStationType() == BASESTATION)
    {
        sprintf(buf,
                "Broadcast Queue: %d\nControl Queue: %d\nBasic Queue: %d\nPrimary Queue: %d\nData Queue: %d\nAllowed to send: %d",
                (int) (requestMsgQueue.size()), (int) (controlMsgQueue.size()),
                (int) (basicCIDQueue.size()), (int) (primaryCIDQueue.size()),
                (int) (dataMsgQueue.size()), (int) (allowedToSend));
    }
    else if (controlplane->getStationType() == MOBILESTATION)
    {
        sprintf(buf,
                "Request Queue: %d\nControl Queue: %d\nBasic Queue: %d\nPrimary Queue: %d\nData Queue: %d\n",
                (int) (requestMsgQueue.size()), (int) (controlMsgQueue.size()),
                (int) (basicCIDQueue.size()), (int) (primaryCIDQueue.size()),
                (int) (dataMsgQueue.size()));
    }

    getDisplayString().setTagArg("t", 0, buf);
    getDisplayString().setTagArg("t", 1, "r");

    /**
     * The transmitted datarate is displayed beside the Transceiver-Compound-Module.
     * Update the displayed string every second -> datarate per second
     */
    cur_floor = floor(simTime().dbl());
    if ((cur_floor - last_floor) == 1)
    {
        char rates_buf[50];
        sprintf(rates_buf, "\n\nTx: %d bit/s", uplink_rate);

        getParentModule()->getParentModule()->getDisplayString().setTagArg("t", 0, rates_buf);
        getParentModule()->getParentModule()->getDisplayString().setTagArg("t", 1, "r");

        uplink_rate = 0;
        last_floor = cur_floor;
    }
}

/**
 * Checks if the queue, that the CID is associated with, is empty or not.
 * This way, if a send request for ranging messages arrives only messages
 * from the controlMsgQueue are sent and NOT from queues later in the
 * priority ordering.
 */
bool CommonPartSublayerTransceiver::managementQueuesEmpty(Ieee80216Prim_sendControlRequest *sc_req)
{
    EV << "(in CommonPartSublayerTransceiver::managementQueuesEmpty) " << endl;
    bool status = true;
    int CID = sc_req->getPduCID();
    EV << "managementQueuesEmpty(CID: " << CID << ") in ";

    if (controlplane->getStationType() == MOBILESTATION)
    {
        EV << "MS = ";
        if (CID == 0xFFFF && !requestMsgQueue.empty())
            status = false;
        else if (CID == 0x0000 && !controlMsgQueue.empty())
            status = false;
        else if (CID != 0x0000 && CID != 0xFFFF)
        {
            if (!basicCIDQueue.empty() && sc_req->getRealCID() == controlplane->getBasicCID())
                status = false;
            else if (!primaryCIDQueue.empty()
                     && sc_req->getRealCID() == controlplane->getPrimaryCID())
                status = false;
            else if (!dataMsgQueue.empty() &&
                     (sc_req->getRealCID() != controlplane->getBasicCID() &&
                      sc_req->getRealCID() != controlplane->getPrimaryCID()))
            {

                status = false;

                // (mk): die nächsten zeilen sind mein eigener blödsinn... AARRRGGHH
                // grants für cids müssen auch gefüllt werden, wenn kein explizites paket für diese CID in der queue ist..
                //  den rest macht packedData()...

                //    Ieee80216MacHeaderFrameList::iterator it;
                //    for ( it = dataMsgQueue.begin(); it != dataMsgQueue.end(); it++ ) {
                //     if ( (dynamic_cast<Ieee80216MacHeader *>(*it))->getCID() == sc_req->getRealCID() )
                //      status=false;
                //    }

            }
        }
    }

    if (controlplane->getStationType() == BASESTATION)
    {
        EV << "BS = ";
        if (!requestMsgQueue.empty() && CID == 0xFFFF)
            status = false;
        else if (CID == 1025 && !dataMsgQueue.empty())
            status = false;
    }

    EV << status << "\n";
    return status;
}

/**
 * Returns a packed MacHeader with as many pending packets for the given CID
 * in the dataQueue as possible (bounded by the number of bits possible for
 * the current uplink burst). The traffic generators should not send more
 * packets down than allowed for the corresponding ServiceFlow (maybe future TODO). Their number
 * is calculated by the packet size and the inter-departure time
 * -> see application "TrafficGenerator_Erlangen"
 */
Ieee80216GenericMacHeader *CommonPartSublayerTransceiver::packedDataForCID(int CID,
                                                                           int bits_for_burst)
{
    Ieee80216MacHeaderFrameList packed_list;
    int bits_left = bits_for_burst;

    Ieee80216MacHeaderFrameList *tmp_list = new Ieee80216MacHeaderFrameList(dataMsgQueue);
    Ieee80216MacHeaderFrameList cleaned_dataMsgQueue;

// EV << "qdataa   = ? : "  << dataMsgQueue.size() << "\n";
// EV << "tmp_list = 0? : "  << tmp_list->size() << "\n";
//
    EV << "bits_for_burst: " << bits_left << "\n";

    // first, look for packets with the corresponding CID in the dataQueue
    while (!tmp_list->empty() && bits_left > 0)
    {
        Ieee80216GenericMacHeader *gmh = (Ieee80216GenericMacHeader *) tmp_list->front();
        if (gmh->getCID() == CID)
        {

            if (bits_left - gmh->getByteLength() >= 0)
            {
                bits_left -= gmh->getByteLength();
                packed_list.push_back(gmh);

                int index = gmh->findPar("priority");
                int prio = gmh->par(index).doubleValue();

                sums_perSecond[prio] += gmh->getByteLength() - 48;

                sums_Data[prio] += gmh->getByteLength() - 48; //Die gesamte Nutzdatenmenge aufzeichnen
                recordDataoutput();
            }
        }
        else
            cleaned_dataMsgQueue.push_back(gmh);

        tmp_list->pop_front();
    }

    // add potentially left messages from the tmp_list to the cleaned_dataMsgQueue
    // which will later replace the old dataMsgQueue
    while (tmp_list->size() > 0)
    {
        cleaned_dataMsgQueue.push_back(tmp_list->front());
        tmp_list->pop_front();
    }

    EV << "bits left for burst with CID " << CID << ": " << bits_left << "\n";
    // if the burst is not full, yet, look for other packets
    // that still fit in, so that a miniumum of burst capacity is wasted
    if (bits_left > 0)
    {
        EV << "trying to fill them up with any pending packets...\n";
        tmp_list = new Ieee80216MacHeaderFrameList(cleaned_dataMsgQueue);
        cleaned_dataMsgQueue.clear();

        while (!tmp_list->empty() && bits_left > 0)
        {
            Ieee80216GenericMacHeader *gmh = (Ieee80216GenericMacHeader *) tmp_list->front();
            if (bits_left - gmh->getByteLength() >= 0)
            {
                bits_left -= gmh->getByteLength();
                packed_list.push_back(gmh);

                // add bits to the sum of the packets priority
                int index = gmh->findPar("priority");
                int prio = gmh->par(index).doubleValue();
                sums_perSecond[prio] += gmh->getByteLength() - 48;
            }
            else
                cleaned_dataMsgQueue.push_back(gmh);

            tmp_list->pop_front();
        }
    }

    // add potentially left messages from the tmp_list to the cleaned_dataMsgQueue
    // which will later replace the old dataMsgQueue
    while (tmp_list->size() > 0)
    {
        cleaned_dataMsgQueue.push_back(tmp_list->front());
        tmp_list->pop_front();
    }

    // if there are packets in the list, encapsulate them in a PackingSubHeader
    if (packed_list.size() > 0)
    {
        Ieee80216PackingSubHeader *psh = new Ieee80216PackingSubHeader("PackingSubHeader");
        psh->setByteLength(psh->getBit_length());
        psh->setIncluded_datagramsArraySize(packed_list.size());

        Ieee80216MacHeaderFrameList::iterator pit;
        int index = 0;
        int new_packet_size = 0;
        while (!packed_list.empty())
        {
            Ieee80216GenericMacHeader *gm =
                check_and_cast<Ieee80216GenericMacHeader *>(packed_list.front());
            new_packet_size += gm->getByteLength();

            psh->setIncluded_datagrams(index++, *gm);
            packed_list.pop_front();

            delete gm;
        }

        Ieee80216GenericMacHeader *packed_data =
            new Ieee80216GenericMacHeader(getParentModule()->getParentModule()->getParentModule()->
                                          getParentModule()->getName());
        packed_data->setCID(CID);
        packed_data->setByteLength(new_packet_size);
        packed_data->encapsulate(psh);

        // replace the original data queue with the cleaned one
        dataMsgQueue = cleaned_dataMsgQueue;

//  EV << "qdataa   = ? : "  << dataMsgQueue.size() << "\n";
//  EV << "tmp_list = 0? : "  << tmp_list->size() << "\n";

        return packed_data;
    }
    else
        return NULL;
}

bool CommonPartSublayerTransceiver::isBroadcastSendRequest(cMessage *msg)
{
    if (msg == nextControlQueueElement)
    {
        cPolymorphic *c_info = nextControlQueueElement->getControlInfo();
        if (c_info)
        {
            Ieee80216Prim_sendControlRequest *sc_req =
                check_and_cast<Ieee80216Prim_sendControlRequest *>(c_info);
            if (sc_req->getPduCID() == 0xFFFF)
                return true;
            else
                return false;
        }
        else
            error("ControlRequest enthält keine Kontrollinformationen!");
    }
    return false;
}

void CommonPartSublayerTransceiver::recordDatarates()
{
    //cvec_fifo_perSecond.record( sum_fifo );

    cvec_ugs_perSecond.record(sums_perSecond[pUGS]);
    cvec_rtps_perSecond.record(sums_perSecond[pRTPS]);
    cvec_ertps_perSecond.record(sums_perSecond[pERTPS]);
    cvec_nrtps_perSecond.record(sums_perSecond[pNRTPS]);
    cvec_be_perSecond.record(sums_perSecond[pBE]);
    cvec_throughput_uplink.record(sums_perSecond[pUGS] + sums_perSecond[pRTPS] +
                                  sums_perSecond[pERTPS] + sums_perSecond[pNRTPS] +
                                  sums_perSecond[pBE]);

    sums_perSecond[pUGS] = sums_perSecond[pRTPS] = sums_perSecond[pERTPS] = sums_perSecond[pNRTPS] =
        sums_perSecond[pBE] = 0;
}

void CommonPartSublayerTransceiver::recordDataoutput()
{
    cvec_sum_ugs.record(sums_Data[pUGS]);
}
