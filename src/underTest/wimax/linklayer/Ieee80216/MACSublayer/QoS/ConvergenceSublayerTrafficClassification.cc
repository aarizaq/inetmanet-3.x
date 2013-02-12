#include "ConvergenceSublayerTrafficClassification.h"

Define_Module(ConvergenceSublayerTrafficClassification);

ConvergenceSublayerTrafficClassification::ConvergenceSublayerTrafficClassification()
{
}

ConvergenceSublayerTrafficClassification::~ConvergenceSublayerTrafficClassification()
{
}

void ConvergenceSublayerTrafficClassification::initialize()
{
    // KD: Assign the Gate-IDs to certain variables (integer). By doing this, Gates don't have to be called with ID directly.
    higherLayerGateIn_ftp = findGate("higherLayerGateIn", 0);
    higherLayerGateIn_voice_no_supr = findGate("higherLayerGateIn", 1);
    higherLayerGateIn_voice_supr = findGate("higherLayerGateIn", 2);
    higherLayerGateIn_video_stream = findGate("higherLayerGateIn", 3);
    higherLayerGateIn_guaranteed_minbw_web_access = findGate("higherLayerGateIn", 4);

    higherLayerGateOut = findGate("higherLayerGateOut");
    headerCompressionGateIn = findGate("headerCompressionGateIn");
    headerCompressionGateOut = findGate("headerCompressionGateOut");

    controlPlaneOut = findGate("controlPlaneOut");

    module_name = new string(getParentModule()->getParentModule()->getParentModule()->getParentModule()->getName());

    // TODO: Not used yet
    // for measurements of latency and jitter -> no further implementation.
    voip_max_latency = par("voip_max_latency");
    voip_tolerated_jitter = par("voip_tolerated_jitter");

    // Wenn die BS nicht rechtzeitig auf SF-Request der SS reagiert. Wenn der Request verloren geht zB.
    sf_request_timeout = par("sf_request_timeout");

    /**
     * get the control plane module for the correct station type
     * Im Weiteren wird nur noch "Controlplane" als Variable genutzt. Hier also eine Zuweisung der jew. Module aus BS und SS
     */
    cModule *tmp_controlplane =
        getParentModule()->getParentModule()->getParentModule()->getSubmodule("controlPlane")->
        getSubmodule("cp_mobilestation");
    if (tmp_controlplane != 0)
    {
        controlplane = dynamic_cast<ControlPlaneMobilestation *>(tmp_controlplane);
        controlPlaneMobilstation = dynamic_cast<ControlPlaneMobilestation *>(tmp_controlplane); //RS
    }
    else
    {
        tmp_controlplane =
            getParentModule()->getParentModule()->getParentModule()->getSubmodule("controlPlane")->
            getSubmodule("cp_basestation");
        if (tmp_controlplane != 0)
        {
            controlplane = dynamic_cast<ControlPlaneBasestation *>(tmp_controlplane);
        }
    }

    //test_timer = new cMessage("testpacketTimer");
    //scheduleAt( 0.002, test_timer );
}

void ConvergenceSublayerTrafficClassification::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage())
    {
//  if ( this->parentModule()->parentModule()->parentModule()->parentModule()->name().compare(ms) == 0 ) {
        EV << "(in handleMessage) Message " << msg->getName() <<
            " ist eine SelfMessage und wird an die Funktion handleSelfMessage() weitergeleitet.\n";

        handleSelfMessage(msg);
//  }
    }
    else if (msg->isPacket())
    {
        EV << "(in handleMessage) Message " << msg->getName() << " ist ein Paket.\n";
        // || -> XOR, alternative ist eine Switch-Konstruktion dazu.
        if (msg->getArrivalGateId() == higherLayerGateIn_ftp ||
            msg->getArrivalGateId() == higherLayerGateIn_voice_no_supr ||
            msg->getArrivalGateId() == higherLayerGateIn_voice_supr ||
            msg->getArrivalGateId() == higherLayerGateIn_video_stream ||
            msg->getArrivalGateId() == higherLayerGateIn_guaranteed_minbw_web_access)
        {
            cPacket *tempPtr = PK(msg);
            if (tempPtr->getByteLength() != 0)
            {
                if (dynamic_cast<IPv4Datagram *>(tempPtr))
                {
                    EV << "Incoming packet: " << tempPtr->getName() << "(" <<
                        tempPtr->getByteLength() << " byte(s))\n";
                    handleUnclassifiedMessage(tempPtr);
                }               // Da ist was vom TG reingekommen -> Debug-Output mit ev auf der Konsole.
            }
            else
            {
                EV << "Incoming message has length 0\n";
            }
        }
    }
}

//requesttimeout: startNewRetryTimer: Wird also aufgerufen, wenn Timer abgelaufen ist.
// - für welchen TRaffic Type wurde Timer gestartet?
// - wenn in RequestMap ( current_tt_requests) nicht gefunden wurde, dann löscht er Timer, weil dann SF bereits eingerichtet
// - wenn in RequestMap ( current_tt_requests) gefunden wurde, dann greift Timer und neuer Reuest wird implizit durch Löschen des Eintrages erzugt. HandleUnclassifiedMessage erzeugt den Request dann tatsächlich
void ConvergenceSublayerTrafficClassification::handleSelfMessage(cMessage *msg)
{

    int index = msg->findPar("traffic_type");
    int traffic_type = msg->par(index).longValue();

    if (current_tt_requests.find(traffic_type) == current_tt_requests.end())
    {
        delete msg;
    }
    else
    {
        EV << "Retry timer for traffic type " << traffic_type << " activated\n";
        current_tt_requests.erase(traffic_type);

        delete msg;
    }
}

/**
 * Handle messages from upper layers:
 *  - find ServiceFlow for messages
 *  - trigger new requests via ControlPlane
 */
void ConvergenceSublayerTrafficClassification::handleUnclassifiedMessage(cMessage *msg)
{
    // extract control information --> contains the type of incoming traffic
	IPv4Datagram *ip_d = check_and_cast<IPv4Datagram *>(msg); // check_and_cast erspart Fehlerabfrage, NULL-Pointer (Omnet-code, siehe Doku).
    Ieee80216TGControlInformation *ci =
        check_and_cast<Ieee80216TGControlInformation *>(ip_d->getControlInfo());

    EV << "Traffic type of arriving packet is: " << ci->getTraffic_type() << "\n";

    //cModule *cplane = parentModule()->parentModule()->parentModule()->submodule("controlPlane")->submodule("cp_mobilestation");
    //ControlPlaneMobilestation *cpms = dynamic_cast<ControlPlaneMobilestation*>(cplane);

    // link direction wird abgefragt für UL und DL, da für UL und DL unterschiedliche Service Flows eingerichtet werden, die sonst miteinander in Konflikt kommen.
    // UL und DL-SF müssen getrennt werden.
    link_direction link_type;
    if (controlplane->getStationType() == MOBILESTATION)
    {
        link_type = ldUPLINK;
    }
    else if (controlplane->getStationType() == BASESTATION)
    {
        link_type = ldDOWNLINK;
    }

    std::list<int> list_cids =
        controlplane->findMatchingServiceFlow((ip_traffic_types) ci->getTraffic_type(), link_type);
    map<int, int>::iterator map_it = current_tt_requests.find(ci->getTraffic_type());

    if (controlplane->getStationType() == MOBILESTATION && controlPlaneMobilstation->isHandoverDone() == true) //wenn ein Handover durchgefuehrt wurde neue Datenverbind eerstellen
    {
        //breakpoint("Handover Done");
        controlPlaneMobilstation->setHandoverDone();
        startHORetryTimer((ip_traffic_types) ci->getTraffic_type());
    }

    // a suitable connection has been found
    if (list_cids.size() > 0)
    {
        EV << "Found " << list_cids.size() << " matching CIDs for incoming packets\n";

        string packet_name;
        std::list<int>::iterator it;

        switch (ci->getTraffic_type())
        {
        case UGS:
            packet_name = *module_name + "-UGS";
            break;
        case RTPS:
            packet_name = *module_name + "-RTPS";
            break;
        case ERTPS:
            packet_name = *module_name + "-ERTPS";
            break;
        case NRTPS:
            packet_name = *module_name + "-NRTPS";
            break;
        case BE:
            packet_name = *module_name + "-BE";
            break;
        }

        // Es gibt an der BS EINEN TG pro Traffic-Typ und NICHT (!!!!) für jede Mobilstation ein eigenes Set.
        // Damit jede SS auch im DL etwas empfängt, müssen entsprechend viele Duplikate erzeugt werden.
        // Erübrigt sich nach Integration des IP-Layers (Kommunikation zw. 2 Nodes)
        // send a duplicate of the incoming traffic packet to each MS
        // with an established ServiceFlow for this type of traffic (BS).
        // MS only gets one result returned
        for (it = list_cids.begin(); it != list_cids.end(); it++)
        {
            // Erzeugen, Duplizieren
        	IPv4Datagram *duplicate = check_and_cast<IPv4Datagram *>(check_and_cast<cMessage *>(msg->dup()));
            duplicate->setControlInfo(new Ieee80216TGControlInformation(*ci));
            EV << "has controlinformation: " << duplicate->getControlInfo() << "\n";
            // Header Erzeugen
            Ieee80216GenericMacHeader *mac = new Ieee80216GenericMacHeader(packet_name.c_str());
            mac->setCID(*it);
            // Paket mit Header ausstatten
            mac->encapsulate(duplicate);

            //mac->setTraffic_type( ci->getTraffic_type() );
//    Ieee80216TGControlInformation *cinfo = new Ieee80216TGControlInformation(*ci);
//    mac->setControlInfo( cinfo );

            EV << "duplicate: " << mac->getName();

            send(mac, headerCompressionGateOut);

        }

        // remove the traffic type from pending requests map
        if (map_it != current_tt_requests.end())
        {
            current_tt_requests.erase(map_it);
        }

        delete ip_d;
    }

    // Es kommt Traffic vom TG runter, aber es noch keine Verbindung aufgebaut. Das muss dann hier "nachgeholt" werden.
    // a request for a new connection or a change of a current connection has to be built
    else
    {
        if (map_it == current_tt_requests.end())
        {

            EV << "Found no matching CID for incoming packets - sending classification command to ControlPlane..\n";
            // TODO: DSD, DSC fehlen noch!
            // CC = Classification Command
            Ieee80216ClassificationCommand *cc = new Ieee80216ClassificationCommand("CC_unknown");
            cc->setRequest_type(DSA);
            cc->setTraffic_type(ci->getTraffic_type()); // <-- wahrscheinlich obsolet, da eigentlich nur für QoSParamSet-Werte benötigt..
            cc->setRequested_sf_state(SF_ACTIVE);

            // values set according to standard. these settings help
            // to identify the type of traffic by the given parameter sets
            sf_QoSParamSet *qos = new sf_QoSParamSet();
            qos->packetInterval = ci->getPacketInterval();

            //so in etwa gem. Standard :-), Nomenklatur gem. Standard.
            switch (ci->getTraffic_type())
            {
            case BE:
                qos->min_reserved_traffic_rate = 0;
                qos->max_sustained_traffic_rate = 0;
                qos->max_latency = 0;
                cc->setName("CC_BE");
                break;

            case UGS:
                qos->max_sustained_traffic_rate = ci->getBitrate(); //sustained = reserved gem. Standard
                qos->min_reserved_traffic_rate = ci->getBitrate();
                qos->max_latency = voip_max_latency;
                qos->tolerated_jitter = voip_tolerated_jitter;
                cc->setName("CC_UGS");
                break;

            case RTPS:
                qos->max_sustained_traffic_rate = ci->getBitrate();
                qos->min_reserved_traffic_rate = ci->getBitrate() / 2; // nicht gem. Standard, ggf. unspecified, nichts gefunden
                qos->max_latency = voip_max_latency;
                cc->setName("CC_RTPS");
                break;

            case ERTPS:        // erweitert für MobileWiMAX. Siehe Standard.
                qos->max_sustained_traffic_rate = ci->getBitrate();
                qos->min_reserved_traffic_rate = ci->getBitrate() / 2;
                qos->max_latency = voip_max_latency;
                cc->setName("CC_ERTPS");
                break;

            case NRTPS:
                qos->min_reserved_traffic_rate = ci->getBitrate();
                qos->traffic_priority = 1;
                cc->setName("CC_NRTPS");
                break;
            }

            // Anhängen der QoS-Parameter an die Classification Command zur weiteren Nutzung in der BS
            cc->setRequested_qos_params(*qos); //

            send(cc, controlPlaneOut);

            // the second parameter (0) is the initial lost packet counter for the request until the ServiceFlow
            // is acknowledged
            current_tt_requests[ci->getTraffic_type()] = 0;

           /**
             * Start a retry timer after the classification command is sent.
             * If the request has been rejected for the moment or the request got lost,
             * a new connection attempt is ensured.
             */
            EV << "Starting retry timer for traffic type " << ci->getTraffic_type() << "\n";
            // Timer started after send-method. Abchecken ob SF erfolgreich eingerichtet. läuft innerhalb einer Station ab. Also hier gehts nicht um erfolgreich gesendete Pakete. Auch nicht um ARQ
            startNewRetryTimer((ip_traffic_types) ci->getTraffic_type());

            delete msg;
        }
        else
        {
            EV << "DSA-REQ for traffic type [" << ci->getTraffic_type() <<
                "] already sent. Still waiting for DSX-RVD...Retries: " <<
                current_tt_requests[ci->getTraffic_type()] << "\n";
            current_tt_requests[ci->getTraffic_type()]++;
            // req
            delete msg;
        }

    }

    return;
}

void ConvergenceSublayerTrafficClassification::startNewRetryTimer(ip_traffic_types traffic_type)
{
    cMessage *retry_msg = new cMessage("retryTimer");
    cMsgPar *tt_par = new cMsgPar();
    tt_par->setName("traffic_type");
    tt_par->setDoubleValue(traffic_type);
    retry_msg->addPar(tt_par);

    scheduleAt(simTime() + sf_request_timeout, retry_msg);
}

void ConvergenceSublayerTrafficClassification::startHORetryTimer(ip_traffic_types traffic_type)
{
    cMessage *retry_msg = new cMessage("retryTimer");
    cMsgPar *tt_par = new cMsgPar();
    tt_par->setName("traffic_type");
    tt_par->setDoubleValue(traffic_type);
    retry_msg->addPar(tt_par);

    scheduleAt(simTime(), retry_msg);
}
