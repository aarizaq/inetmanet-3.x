/*
 *\class       PASER_Socket
 *@brief       Class comprises the <b>handleMessage</b>
 * function that is called upon receipt of a message by the PASER module.
 *
 *\authors     Eugen.Paul | Mohamad.Sbeiti \@paser.info
 *
 *\copyright   (C) 2012 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
 *                  at Technische Universitaet Dortmund, Germany
 *                  http://www.kn.e-technik.tu-dortmund.de/
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ********************************************************************************
 * This work is part of the secure wireless mesh networks framework, which is currently under development by CNI
 ********************************************************************************/
#include "Configuration.h"
#ifdef OPENSSL_IS_LINKED
#include <openssl/err.h>
#include "PASER_Socket.h"
#include "IPv4InterfaceData.h"


#include "ControlManetRouting_m.h"
#include "IPv4ControlInfo.h"
#include "UDPPacket.h"
#include "Ieee802Ctrl_m.h"
#include "InterfaceEntry.h"

#include <list>
#include "compatibility.h"

Define_Module(PASER_Socket);

PASER_Socket::PASER_Socket() {
    timer_queue = NULL;
    routing_table = NULL;
    neighbor_table = NULL;
    message_queue = NULL;
    root = NULL;
    crypto_sign = NULL;
    crypto_hash = NULL;
    paket_processing = NULL;
    route_findung = NULL;
    route_maintenance = NULL;
    paser_global = NULL;
    paser_configuration = NULL;

    rreq_list = NULL;///< List of IP addresses to which a route discovery is started.

    rrep_list = NULL; ///< List of IP addresses from which a TU-RREP-ACK is expected.
#ifdef OMNETPP
    startMessage = NULL;
    sendMessageEvent = NULL;
    genRootEvent = NULL;
#endif


}

PASER_Socket::~PASER_Socket() {
    delete paser_global;
    delete paser_configuration;

    delete paket_processing;
    delete route_findung;
    delete route_maintenance;

#ifdef OMNETPP
    if (sendMessageEvent->isScheduled()) {
        cancelEvent(sendMessageEvent);
    }
    delete sendMessageEvent;
    if (startMessage->isScheduled()) {
        cancelEvent(startMessage);
    }
    delete startMessage;
    if (genRootEvent->isScheduled()) {
        cancelEvent(genRootEvent);
    }
    delete genRootEvent;
#endif
}

void PASER_Socket::initialize(int stage) {
    if (stage == 4) {
        paketProcessingDelay = 0;
        data_message_send_total_delay = 0;
        registerRoutingModule();
        firstRoot = true;

        //Init OPENSSL
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        ENGINE_load_builtin_engines();
        ENGINE_register_all_complete();

        if (getParentModule()->par("isGW")) {
            char temp[] = "1";
            EV << "GATEWAY\n";
            paser_configuration = new PASER_Configurations(temp, this);
        } else {
            char temp[] = "0";
            EV << "CLIENT\n";
            paser_configuration = new PASER_Configurations(temp, this);
        }
        paser_global = new PASER_Global(paser_configuration, this);
        bool setGWsearch = paser_configuration->isGWsearch();

        timer_queue = paser_global->getTimer_queue();
        routing_table = paser_global->getRouting_table();
        neighbor_table = paser_global->getNeighbor_table();
        message_queue = paser_global->getMessage_queue();

        rreq_list = paser_global->getRreq_list();
        rrep_list = paser_global->getRrep_list();

        root = paser_global->getRoot();

        if (paser_configuration->isSetLinkLayerFeeback()) {
            linkLayerFeeback();
        }

        registerPosition();

//        isGW = paser_configuration->getIsGW();
        EV << "isGW = " << paser_configuration->getIsGW() << "\n";
//        isRegistered = paser_global->getIsRegistered();
//        wasRegistered = paser_global->getWasRegistered();

        sendMessageEvent = new cMessage("sendMessageEvent");
        startMessage = new cMessage("startMessage");
        genRootEvent = new cMessage("genRootEvent");
        simtime_t timer;
        if (paser_configuration->getIsGW()) {
            timer = simTime();
        } else {
            timer = simTime() + par("startRegTime");
        }
        scheduleAt(timer, startMessage);

        crypto_sign = paser_global->getCrypto_sign();
        crypto_hash = paser_global->getCrypto_hash();

        route_findung = new PASER_Route_Discovery(paser_global,
                paser_configuration, this, setGWsearch);
        paser_global->setRoute_findung(route_findung);
        paket_processing = new PASER_Message_Processing(paser_global,
                paser_configuration, this);
        paser_global->setPaket_processing(paket_processing);
        route_maintenance = new PASER_Route_Maintenance(paser_global,
                paser_configuration, this);
        paser_global->setRoute_maintenance(route_maintenance);

        EV << "NumWlanInterfaces = " << getNumWlanInterfaces() << "\n";
        EV << "getWlanInterfaceIndexByAddress = "
                << getWlanInterfaceIndexByAddress() << "\n";

        netDevice = paser_global->getNetDevice();

        WATCH_PTRLIST(timer_queue->timer_queue);
        WATCH_PTRMAP(routing_table->route_table);
        WATCH_PTRMAP(neighbor_table->neighbor_table_map);
        WATCH_PTRMAP(rrep_list->rreq_list);
        WATCH_PTRMAP(rreq_list->rreq_list);
//        WATCH(seqNr);

        if (ev.isGUI() && paser_global->getIsRegistered()) {
            cDisplayString& connDispStr = getParentModule()->getDisplayString();
            connDispStr.setTagArg("i", 1, "green");
        }
        startGenNewRoot();
        scheduleNextEvent();
    }
}

void PASER_Socket::startGenNewRoot() {
    GenRootTimer = simTime() + par("auth_tree_gen_delay");
    scheduleAt(GenRootTimer, genRootEvent);
    paser_configuration->isRootReady = false;
}

void PASER_Socket::GenNewRoot() {
    paser_global->getRoot()->root_regenerate();
    paser_configuration->isRootReady = true;
    if (!firstRoot) {
        //Set timeout and send ROOT
        struct timeval now;
        MYgettimeofday(&now, NULL);

        for (u_int32_t i = 0; i < paser_configuration->getRootRepetitions();
                i++) {
            PASER_Timer_Message *timer = new PASER_Timer_Message();
            timer->handler = PASER_ROOT;
            timer->data = NULL;
            timer->destAddr.S_addr = PASER_BROADCAST;
            timer->timeout = timeval_add(now,
                    paser_configuration->getRootRepetitionsTimeout() * (i + 1));
            paser_global->getTimer_queue()->timer_add(timer);
            paser_global->getTimer_queue()->timer_sort();
        }
        paser_global->getPaket_processing()->send_root();
    }
    firstRoot = false;
}
//double PASER_Socket::calcDistFreeSpace()
//{
//    double SPEED_OF_LIGHT = 300000000.0;
//    double interfDistance;
//
//    //the carrier frequency used
//    double carrierFrequency = cc->par("carrierFrequency");
//    EV << "standart carrierFrequency:" << carrierFrequency << "\n";
//    //signal attenuation threshold
//    //path loss coefficient
//    double alpha = cc->par("alpha");
//
//    double waveLength = (SPEED_OF_LIGHT / carrierFrequency);
//    //minimum power level to be able to physically receive a signal
//    double minReceivePower = sensitivity;
//
//    interfDistance = pow(waveLength * waveLength * transmitterPower /
//                         (16.0 * M_PI * M_PI * minReceivePower), 1.0 / alpha);
//    return interfDistance;
//}

InterfaceEntry * PASER_Socket::PUBLIC_getInterfaceEntry(int index) {
    EV << "gPUBLIC_getInterfaceEntry "
             << index << "\n";
    return getInterfaceEntry(index);
}

//TIMEOUT
std::ostream& operator<<(std::ostream& os, const PASER_Timer_Message& ob2) {
    os
            << /*"src: " << ob2.srcAddr.S_addr.getIPv4().str() << "forw: " << ob2.forwAddr.S_addr.getIPv4().str() <<*/" dest: "
            << ob2.destAddr.S_addr.getIPv4().str() << " type: ";
    switch ((int) ob2.handler) {
    case KDC_REQUEST:
        os << "KDC_REQUEST";
        break;
    case ROUTE_DISCOVERY_UB:
        os << "ROUTE_DISCOVERY_UB";
        break;
    case ROUTINGTABLE_DELETE_ENTRY:
        os << "ROUTINGTABLE_DELETE_ENTRY";
        break;
    case ROUTINGTABLE_VALID_ENTRY:
        os << "ROUTINGTABLE_VALID_ENTRY";
        break;
    case NEIGHBORTABLE_DELETE_ENTRY:
        os << "NEIGHBORTABLE_DELETE_ENTRY";
        break;
    case NEIGHBORTABLE_VALID_ENTRY:
        os << "NEIGHBORTABLE_VALID_ENTRY";
        break;
    case TU_RREP_ACK_TIMEOUT:
        os << "TU_RREP_ACK_TIMEOUT";
        break;
    case HELLO_SEND_TIMEOUT:
        os << "HELLO_SEND_TIMEOUT";
        break;
    }
    os << " sec: " << ob2.timeout.tv_sec;
    os << " usec: " << ob2.timeout.tv_usec;
    return os;
}
;

//NEIGHBOR TABLE
std::ostream& operator<<(std::ostream& os, const PASER_Neighbor_Entry& ob2) {
    os << "addr: " << ob2.neighbor_addr.S_addr.getIPv4().str() << " IV: "
            << ob2.IV << " Flag: " << ob2.neighFlag << " positionX: "
            << ob2.position.lat << " positionY: " << ob2.position.lon
            << " isValid: " << (int) ob2.isValid << " if: " << ob2.ifIndex;
    if (ob2.validTimer) {
        os << "valid: " << ob2.validTimer->timeout.tv_sec << "."
                << ob2.validTimer->timeout.tv_usec;
    }
    return os;
}
;

//ROUTING TABLE
std::ostream& operator<<(std::ostream& os, const PASER_Routing_Entry& ob2) {
    os << "destAddr.S_addr: " << ob2.dest_addr.S_addr.getIPv4().str()
            << " Metric: " << (int) ob2.hopcnt << " nextHop: "
            << ob2.nxthop_addr.S_addr.getIPv4().str() << " isGW: "
            << (int) ob2.is_gw << " SeqNr: " << ob2.seqnum << " isValid: "
            << (int) ob2.isValid;
    return os;
}
;

//ROUTING TABLE
std::ostream& operator<<(std::ostream& os, const message_rreq_entry& ob2) {
    os << "dest : " << ob2.dest_addr.S_addr.getIPv4().str() << " ";
    switch (ob2.tPack->handler) {
    case ROUTE_DISCOVERY_UB:
        os << "ROUTE_DISCOVERY_UB";
        break;
    case ROUTINGTABLE_DELETE_ENTRY:
        os << "ROUTINGTABLE_DELETE_ENTRY";
        break;
    case ROUTINGTABLE_VALID_ENTRY:
        os << "ROUTINGTABLE_VALID_ENTRY";
        break;
    case NEIGHBORTABLE_DELETE_ENTRY:
        os << "NEIGHBORTABLE_DELETE_ENTRY";
        break;
    case NEIGHBORTABLE_VALID_ENTRY:
        os << "NEIGHBORTABLE_VALID_ENTRY";
        break;
    case TU_RREP_ACK_TIMEOUT:
        os << "TU_RREP_ACK_TIMEOUT";
        break;
    default:
        break;
    }
    return os;
}
;

void PASER_Socket::editNodeColor() {
    if (!paser_configuration->getIsGW()) {
        if (paser_global->getRouting_table()->findBestGW() != NULL) {
            paser_global->setIsRegistered(true);
        } else {
            paser_global->setIsRegistered(false);
        }
    }
    if (ev.isGUI() && paser_global->getIsRegistered()) {
        cDisplayString& connDispStr = getParentModule()->getDisplayString();
        connDispStr.setTagArg("i", 1, "green");
    } else if (ev.isGUI() && !paser_global->getIsRegistered()
            && paser_global->getWasRegistered()) {
        cDisplayString& connDispStr = getParentModule()->getDisplayString();
        connDispStr.setTagArg("i", 1, "yellow");
    } else if (ev.isGUI()) {
        cDisplayString& connDispStr = getParentModule()->getDisplayString();
        connDispStr.setTagArg("i", 1, "");
    }
}

/**
 * This function is called whenever a message is received by manetrouting module
 *
 */
void PASER_Socket::handleMessage(cMessage *msg) {
    EV << "handle Message\n";
    paketProcessingDelay = 0;
    data_message_send_total_delay = 0;
    IPv4Datagram * ipDgram = NULL;
    cMessage * msg_aux = NULL;
    UDPPacket* udpPacket = NULL;
    struct in_addr src_addr;

    //Falls genRootEvent => dann soll eine neue AuthTree generiert werden
    // In case genRootEvent is true => generate a new AuthTree
    if (msg == genRootEvent) {
        GenNewRoot();
        scheduleNextEvent();
        return;
    }
    //Falls kein AuthTree vorhanden ist, dann werden alle Pakete geloescht
    // In case AuthTree exists, delete all messages
    if (!paser_configuration->isRootReady) {
        if (msg == startMessage || msg->isSelfMessage()) {
            EV << "genRootTimer: " << GenRootTimer.str() << "\n";
            simtime_t temp = GenRootTimer + 0.000001;
            EV << "newRootTimer: " << temp.str() << "\n";
            scheduleAt(GenRootTimer + 0.000001, msg);
            return;
        }
        delete msg;
        return;
    }
    EV<<"getMessageInterface1"<<endl;
    //Falls startMessage, dann registriere sich am GW oder hole GTK
    //In case startMessage is true, start a registration at a gateway and get GTK
    if (msg == startMessage && paser_configuration->getIsGW()) {
        EV<<"getMessageInterface2"<<endl;
//        paket_processing->sendGTKRequest();
        lv_block cert;
        if (!crypto_sign->getCert(&cert)) {
            EV << "cert ERROR\n";
            return;
        }
        paser_global->generateGwSearchNonce();
        struct in_addr addr1 = paser_configuration->getNetDevice()->ipaddr;
        paket_processing->sendKDCRequest(
                addr1,
                addr1, cert,
                paser_global->getLastGwSearchNonce());
        free(cert.buf);
        editNodeColor();

        PASER_Timer_Message *timeMessage = new PASER_Timer_Message();
        struct timeval now;
        MYgettimeofday(&now, NULL);
        timeMessage->handler = KDC_REQUEST;
//        timeMessage->timeout = timeval_add(now, PASER_KDC_REQUEST_TIME);
        timeMessage->timeout = timeval_add(now,
                par("PASER_KDCWaitTime").doubleValue() / (double) 1000);
        timeMessage->destAddr = paser_configuration->getAddressOfKDC();
        timer_queue->timer_add(timeMessage);
        scheduleNextEvent();
        return;

    }
    if (msg == startMessage) {
        /*
         * just 4 test*/
        //''''''''''''''''''''''''''''''''
//        EV << getParentModule()->getDisplayString().getNumTags() << "\n";
//        for(int i = 0; i< getParentModule()->getDisplayString().getNumTags(); i++){
//            EV << "name: " << getParentModule()->getDisplayString().getTagName(i);
//            for(int j=0; j<getParentModule()->getDisplayString().getNumArgs(i); j++){
//                EV << " = " << getParentModule()->getDisplayString().getTagArg(i,j) << " ";
//            }
//            EV << "\n";
//        }
//        getParentModule()->getDisplayString().setTagArg("p", 0, 700);
//        getParentModule()->getDisplayString().setTagArg("p", 1, 200);
//        getParentModule()->getSubmodule("mobility")->par("x")=700;
//        getParentModule()->getSubmodule("mobility")->par("y")=200;
//        return;
        //''''''''''''''''''''''''''''''''
        EV<<"getMessageInterface3"<<endl;
        route_findung->tryToRegister();
        editNodeColor();
        return;
    }

    // ein Timer ist abgelaufen, bearbeite TimerEvent in RouteMaintenance
    // A timer is expired, process TimerEvent in RouteMaintenance
    if (msg->isSelfMessage()) { // self msg
        route_maintenance->handleSelfMsg(msg);
        scheduleNextEvent();
        editNodeColor();
        return;
    }
    // Falls keine Route vorhanden ist oder auf eine Route ein Paket verschickt wird,
    //dann ist msg ein Objekt der Klasse ControlManetRouting
    // In case no route exists or a message is already sent on a route, msg is an object of class ControlManetRouting
    /* Control Message decapsulate */
    if (dynamic_cast<ControlManetRouting *>(msg)) {
        ControlManetRouting * control = check_and_cast<ControlManetRouting *>(
                msg);
        //Keine Route ist vorhanden
        if (control->getOptionCode() == MANET_ROUTE_NOROUTE) {
            ipDgram = (IPv4Datagram*) control->decapsulate();
            EV << "PASER rec datagram  " << ipDgram->getName() << " with dest="
                    << ipDgram->getDestAddress().str() << "\n";
            //Starte Suche nach der Route
            route_findung->processMessage(ipDgram);
        }
        //aktualisiere den Timer fuer eine Route
        // Update Timer of a route
        else if (control->getOptionCode() == MANET_ROUTE_UPDATE) {
            EV << "Update Message. src: "
                    << control->getSrcAddress().getIPv4().str()
                    << ", dest: "
                    << control->getDestAddress().getIPv4().str() << "\n";
            struct in_addr tempAddr;
            if (paser_global->isHelloActive()) {
                tempAddr.S_addr = control->getSrcAddress();
                EV << "controlSrc: " << tempAddr.S_addr.getIPv4().str()
                        << "\n";
                if (paser_configuration->isAddInMySubnetz(tempAddr)
                        || isLocalAddress(tempAddr.S_addr)) {
                    EV << "I am a src.\n";
                } else {
                    routing_table->updateRouteLifetimes(tempAddr);
                }
            }
            tempAddr.S_addr = control->getDestAddress();
            EV << "controlDest: " << tempAddr.S_addr.getIPv4().str()
                    << "\n";
            if (tempAddr.S_addr != PASER_BROADCAST) {
                if (paser_configuration->isAddInMySubnetz(tempAddr)
                        || isLocalAddress(tempAddr.S_addr)) {
                    EV << "I am a dest.\n";
                } else {
                    routing_table->updateRouteLifetimes(tempAddr);
                }
            } else {
                EV << "is broadcast!\n";
            }

//            updateRouteLifetimes(control->getSrcAddress());
//            updateRouteLifetimes(control->getDestAddress());
        }
        delete msg;
        scheduleNextEvent();
        editNodeColor();
        return;
    }
    // teste ob msg ein PASER Paket ist(Transport Protokoll = UDP, Port = PASER_PORT)
    // Check if msg is a PASER message
    else if (dynamic_cast<UDPPacket *>(msg)) {

        udpPacket = check_and_cast<UDPPacket*>(msg);
        if (udpPacket->getDestinationPort() != PASER_PORT) {
            delete msg;
            return;
        }
        msg_aux = udpPacket->decapsulate();
        IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(
                udpPacket->removeControlInfo());
        if (isIpLocalAddress(controlInfo->getSrcAddr())
                || controlInfo->getSrcAddr().isUnspecified()) {
            // local address delete message
            delete msg_aux;
            delete controlInfo;
            return;
        }
        msg_aux->setControlInfo(controlInfo);
    } else {
        EV << "!!!!!!!!!!!!! Unknown Message type !!!!!!!!!! \n";
        delete msg;
        return;
    }

    if (udpPacket) {
        delete udpPacket;
        udpPacket = NULL;
    }

    if (!dynamic_cast<PASER_MSG *>(msg_aux)
            && !dynamic_cast<crl_message *>(msg_aux)) {
        EV << "dynamic_cast<PASER_MSG *> or dynamic_cast<crl_message *>\n";
        delete msg_aux;
        return;
    }
    cPacket* apPkt = check_and_cast<cPacket *>(msg_aux);

    //bearbeite ein PASER Paket
    // Process a PASER message
    EV<<"getMessageInterface"<<endl;
    paket_processing->handleLowerMsg(apPkt, getMessageInterface(apPkt));
    scheduleNextEvent();

    editNodeColor();
}

// Funktion liefert die Interface auf den die Nachricht ankamm
// Function returns the interface on which the message arrived
u_int32_t PASER_Socket::getMessageInterface(cPacket * msg) {
    int32_t interfaceId;

    EV<<"getMessageInterface"<<endl;
    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo *>(
            msg->removeControlInfo());
    interfaceId = ctrl->getInterfaceId();

    u_int32_t ifIndex = -1;
    bool found = false;
    EV << "if = " << interfaceId << "\n";

    InterfaceEntry * ie;
    for (int i = 0; i < getNumWlanInterfaces(); i++) {
        ie = getWlanInterfaceEntry(i);
        if (ie && interfaceId == ie->getInterfaceId()) {
            ifIndex = getWlanInterfaceIndex(i);
            EV << "found WlanInterfaceIndex\n";
            break;
        }
    }
    for (u_int32_t i = 0; i < paser_configuration->getNetDeviceNumber(); i++) {
        if (netDevice[i].ifindex == ifIndex) {
            found = true;
            break;
        }
    }
//if(isGW && !found){
//    ifIndex = -1;
//    return ifIndex;
//}
    if (!found) {
        EV << "wrong ifIndex\n";
        delete ctrl;
        return 0;
    }
    EV << "Incomming msg on " << ifIndex << "\n";
    delete ctrl;
    EV<<"getMessageInterface"<<ifIndex<<endl;
    return ifIndex;
}

//Funktion holt den naechsten TimerEvent
// Get the next TimerEvent
void PASER_Socket::scheduleNextEvent() {
    //Zuesrst wird TimerQueue sortiert
    timer_queue->timer_sort();
    struct timeval now;
    struct timeval remaining;
    //Der naechste Event wird geholt
    PASER_Timer_Message *next_timer_message = timer_queue->timer_get_next_timer();
    //Falls kein Event vorhanden => return
    if (next_timer_message == NULL) {
        if (sendMessageEvent->isScheduled()) {
            cancelEvent(sendMessageEvent);
        }
        return;
    }
//    EV << "nextEventTime: " << next_timer_message->timeout.tv_sec << "." << next_timer_message->timeout.tv_usec << "\n";
    //Berechne in wieviel Sekunden der Event auftritt
    struct timeval nextTimeout = next_timer_message->timeout;
    MYgettimeofday(&now, NULL);
//    EV << "now: " << now.tv_sec << "." << now.tv_usec << "\n";
    remaining.tv_usec = nextTimeout.tv_usec - now.tv_usec;
    remaining.tv_sec = nextTimeout.tv_sec - now.tv_sec;
    if (remaining.tv_usec < 0) {
//        EV << "remaining.tv_usec < 0\n";
        remaining.tv_usec += 1000000;
        remaining.tv_sec -= 1;
    }

//    EV << "remaining: " << remaining.tv_sec << "." << remaining.tv_usec << "\n";
    simtime_t timer;
    double delay;
    delay = (double) (((double) remaining.tv_usec / (double) 1000000.0)
            + (double) remaining.tv_sec);
    if (delay < 0.0)
        delay = 0.0;
    timer = simTime() + delay;
    if (sendMessageEvent->isScheduled()) {
        if (timer < sendMessageEvent->getArrivalTime()) {
            EV << "reset Timeout\n";
            cancelEvent(sendMessageEvent);
            scheduleAt(timer, sendMessageEvent);
        }
    } else {
        EV << "set new Timeout  " << timer << "... ";
        scheduleAt(timer, sendMessageEvent);
        EV << "OK\n";
    }
}

void PASER_Socket::send_message(cPacket * msg, struct in_addr dest_addr,
        u_int32_t ifIndex) {
    EV << "ifindex " << ifIndex<< "... ";
    if (dynamic_cast<PASER_MSG *>(msg)) {
        PASER_MSG *paser_msg = check_and_cast<PASER_MSG *>(msg);
        switch (paser_msg->type) {
        case 0:
            msg->setName("UB_RREQ");
            break;
        case 1:
            msg->setName("UU_RREP");
            break;
        case 2:
            msg->setName("TU_RREQ");
            break;
        case 3:
            msg->setName("TU_RREP");
            break;
        case 4:
            msg->setName("TU_RREP_ACK");
            break;
        case 5:
            msg->setName("B_RERR");
            break;
        case 6:
            msg->setName("B_HELLO");
            break;
        case 7:
            msg->setName("B_ROOT");
            break;
        case 8:
            msg->setName("B_RESET");
            break;
        }
//        if(paser_msg->type == 5){
//            opp_error("aaaaa");
//        }
        msg->setByteLength(addPaketLaengeZuStat(msg));
//        msg->setByteLength(64);
    } else {
        //Send CRL request
        msg->setName("CRL_REQUEST");
        EV << "paketProcessingDelay = " << paketProcessingDelay << "\n";
        EV << "ifIndex = " << paketProcessingDelay << "\n";
        sendUDPToIp(msg, PASER_PORT_CRL, dest_addr.S_addr, PASER_PORT_CRL, 30,
                paketProcessingDelay, ifIndex);
        return;
    }
    if (dest_addr.s_addr == PASER_BROADCAST) {
        double tempDelay = par("broadCastDelay");
        tempDelay += paketProcessingDelay;
        EV << "paketProcessingDelay = " << paketProcessingDelay << "\n";
        sendToIp(msg, PASER_PORT, dest_addr.S_addr, PASER_PORT, 1, tempDelay,
                ifIndex);
    } else {
        EV << "paketProcessingDelay = " << paketProcessingDelay << "\n";
        sendToIp(msg, PASER_PORT, dest_addr.S_addr, PASER_PORT, 1,
                paketProcessingDelay, ifIndex);
    }
}

int PASER_Socket::addPaketLaengeZuStat(cPacket * msg) {
    PASER_MSG *paser_msg = check_and_cast<PASER_MSG *>(msg);
    int leng = 0;
    if (paser_msg->type == 0) {
        EV << "stat PASER_UB_RREQ\n";
        PASER_UB_RREQ *paket = check_and_cast<PASER_UB_RREQ *>(msg);
        u_int8_t* data = paket->getCompleteByteArray(&leng);
        free(data);
    }
    if (paser_msg->type == 1) {
        EV << "stat PASER_UU_RREP\n";
        PASER_UU_RREP *paket = check_and_cast<PASER_UU_RREP *>(msg);
        u_int8_t* data = paket->getCompleteByteArray(&leng);
        free(data);
    }
    if (paser_msg->type == 2) {
        EV << "stat PASER_TU_RREQ\n";
        PASER_TU_RREQ *paket = check_and_cast<PASER_TU_RREQ *>(msg);
        u_int8_t* data = paket->getCompleteByteArray(&leng);
        free(data);
    }
    if (paser_msg->type == 3) {
        EV << "stat PASER_TU_RREP\n";
        PASER_TU_RREP *paket = check_and_cast<PASER_TU_RREP *>(msg);
        u_int8_t* data = paket->getCompleteByteArray(&leng);
        free(data);
    }
    if (paser_msg->type == 4) {
        EV << "stat PASER_TU_RREP_ACK\n";
        PASER_TU_RREP_ACK *paket = check_and_cast<PASER_TU_RREP_ACK *>(msg);
        u_int8_t* data = paket->getCompleteByteArray(&leng);
        free(data);
    }
    if (paser_msg->type == 5) {
        EV << "stat PASER_TB_RERR\n";
        PASER_TB_RERR *paket = check_and_cast<PASER_TB_RERR *>(msg);
        u_int8_t* data = paket->getCompleteByteArray(&leng);
        free(data);
    }
    if (paser_msg->type == 6) {
        EV << "stat PASER_TB_Hello\n";
        PASER_TB_Hello *paket = check_and_cast<PASER_TB_Hello *>(msg);
        u_int8_t* data = paket->getCompleteByteArray(&leng);
        free(data);
    }
    if (paser_msg->type == 7) {
        EV << "stat PASER_TB_Hello\n";
        PASER_UB_Root_Refresh *paket = check_and_cast<PASER_UB_Root_Refresh *>(msg);
        u_int8_t* data = paket->getCompleteByteArray(&leng);
        free(data);
    }
    if (paser_msg->type == 8) {
        EV << "stat PASER_UB_Key_Refresh\n";
        PASER_UB_Key_Refresh *paket = check_and_cast<PASER_UB_Key_Refresh *>(msg);
        u_int8_t* data = paket->getCompleteByteArray(&leng);
        free(data);
    }

    return leng;
}

int PASER_Socket::getIfIdFromIfIndex(int ifIndex) {
    EV<<"getIfIdFromIfIndex"<<ifIndex<<"\n";
    for (u_int32_t i = 0; i < paser_configuration->getNetDeviceNumber(); i++) {
        if (netDevice[i].enabled == 1 && (int) netDevice[i].ifindex == ifIndex) {
            return i;
        }
    }
    return -1;
}

void PASER_Socket::processLinkBreak(const cPolymorphic *details) {
    IPv4Datagram *dgram = NULL;
    if (paser_configuration->isSetLinkLayerFeeback()) {
        if (dynamic_cast<IPv4Datagram *>(const_cast<cPolymorphic*>(details))) {
            dgram = check_and_cast<IPv4Datagram *>(details);
            struct in_addr dest_addr, src_addr;
            src_addr.s_addr.set(dgram->getSrcAddress());
            dest_addr.s_addr.set(dgram->getDestAddress());

            PASER_Routing_Entry *rEntry = routing_table->findDest(dest_addr);
            if (rEntry) {
                EV << "alte Route wurde gefunden\n";
            } else {
                EV << "keine Route wurde gefunden\n";
            }
            if (rEntry && paser_configuration->isSetLocalRepair()
                    && paser_configuration->getMaxLocalRepairHopCount()
                            >= rEntry->hopcnt) {
                route_maintenance->messageFailed(src_addr, dest_addr, false);
                route_findung->processMessage(dgram->dup());
            } else {
                route_maintenance->messageFailed(src_addr, dest_addr, true);

                struct timeval now;
                MYgettimeofday(&now, NULL);
                paser_global->getBlacklist()->setRerrTime(dest_addr, now);
            }

            scheduleNextEvent();
            return;
        }
//        else if (dynamic_cast<Ieee80211DataFrame *>(const_cast<cPolymorphic*> (details)))
//        {
//            Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame *>(const_cast<cPolymorphic*>(details));
//            messageFailedMac(frame);
//        }
    }
}

bool PASER_Socket::isMyLocalAddress(struct in_addr addr) {
    EV << " Entering isMyLocalAddress \n";
    EV << addr.S_addr.getIPv4().getInt()<<" \n";
    bool result = isLocalAddress(addr.S_addr);
    EV <<"result"<< result<<" \n";
    return result;
}

int PASER_Socket::MYgettimeofday(struct timeval *tv, struct timezone *tz) {
//    return gettimeofday(tv, tz);
    double current_time;
    double tmp;
    /* Timeval is required, timezone is ignored */
    if (!tv)
        return -1;
    current_time = simTime().dbl();
//    EV << "current_time: " << current_time << "\n";
    tv->tv_sec = (long) current_time; /* Remove decimal part */
//    EV << "tv->tv_sec: " << tv->tv_sec << "\n";
    tmp = (current_time - tv->tv_sec) * 1000000;
//    EV << "current_time - tv->tv_sec = " << current_time - tv->tv_sec << "\n";
//    EV << "tmp: " << tmp << "\n";
    tv->tv_usec = (long) (tmp + 0.5);
//    EV << "tv->tv_usec: " << tv->tv_usec << "\n";
    if (tv->tv_usec >= 1000000) {
        tv->tv_sec++;
        tv->tv_usec -= 1000000;
    }
    return 0;
}

int PASER_Socket::MYgetWlanInterfaceIndexByAddress(ManetAddress add) {
    EV<<"MYgetWlanInterfaceIndexByAddress"<<endl;
    return getWlanInterfaceIndexByAddress(add);
}

void PASER_Socket::MY_omnet_chg_rte(const ManetAddress &destination, const ManetAddress &nextHop,
        const ManetAddress &mask, short int hops, bool del_entry, int ifaceIndex) {


    if (!getIsRegistered())
         opp_error("Manet routing protocol is not register");

    if (getIsMacLayer())
        opp_error("PASER can not work in the MAC layer");
     /* Add route to kernel routing table ... */
     IPv4Address desAddress(destination.getIPv4());
     EV<<"destination is"<<destination.getIPv4()<<endl;
     EV<<"MASK"<<mask.getIPv4()<<endl;
     EV<<"getNumInterfaces()"<<getNumInterfaces()<<endl;
     EV<<"ifaceIndex"<<ifaceIndex<<endl;

     if (ifaceIndex>=getNumInterfaces())
         return;

     bool found = false;
     IPv4Route *oldentry = NULL;

     //TODO the entries with ALLONES netmasks stored at the begin of inet route entry vector,
     // let optimise next search!
     for (int i=getInetRoutingTable()->getNumRoutes(); i>0; --i)
     {
         IPv4Route *e = getInetRoutingTable()->getRoute(i-1);
         if (desAddress == e->getDestination())     // FIXME netmask checking?
         {
             if (del_entry && !found)    // FIXME The 'found' never set to true when 'del_entry' is true
             {
                 if (!getInetRoutingTable()->deleteRoute(e))
                     opp_error("ManetRoutingBase::setRoute can't delete route entry");
             }
             else
             {
                 found = true;
                 oldentry = e;
             }
         }
     }


     if (del_entry)
          return;

      IPv4Address netmask(mask.getIPv4());
      IPv4Address gateway(nextHop.getIPv4());
      //if (mask.isUnspecified())
        //  netmask = IPv4Address::ALLONES_ADDRESS;
      InterfaceEntry *ie = getInterfaceEntry(ifaceIndex);
     EV<< ifaceIndex<<endl;

      if (found)
      {
          if (oldentry->getDestination() == desAddress
                  && oldentry->getNetmask() == netmask
                  && oldentry->getGateway() == gateway
                  && oldentry->getMetric() == hops
                  && oldentry->getInterface() == ie
                  && oldentry->getSource() == IPv4Route::MANUAL)
              return;
          getInetRoutingTable()->deleteRoute(oldentry);
      }

      IPv4Route *entry = new IPv4Route();

      /// Destination
      entry->setDestination(desAddress);
      EV<<"destination is"<<desAddress<<endl;
      EV<<"destination is"<< entry->getDestination()<<endl;
      /// Route mask
      entry->setNetmask(netmask);
      EV<<"netmask is"<<netmask<<endl;
      EV<<"netmask is"<< entry->getNetmask()<<endl;
      /// Next hop
      entry->setGateway(gateway);
      EV<<"gateway is"<<gateway<<endl;
      EV<<"gateway is"<< entry->getGateway()<<endl;
      /// Metric ("cost" to reach the destination)
      entry->setMetric(hops);
      /// Interface name and pointer
      EV<<"hops"<<hops<<endl;
      EV<<"hops is"<< entry->getMetric()<<endl;
      entry->setInterface(ie);
      EV<<"ie is"<<ie<<endl;
      EV<<"ie is"<< entry->getInterface()<<endl;

      /// Source of route, MANUAL by reading a file,
      /// routing protocol name otherwise
      IPv4Route::RouteSource routeSource = getUseManetLabelRouting() ? IPv4Route::MANET : IPv4Route::MANET2;
      entry->setSource(routeSource);
      EV<<"routeSource is"<<routeSource<<endl;
      EV<<"routeSource is"<<entry->getSource()<<endl;

      getInetRoutingTable()->addRoute(entry);

      //omnet_chg_rte(dst, gtwy, netm, hops, del_entry, index);
      if (!del_entry) {
          EV << "update  Routing Table: " << destination.getIPv4().str() << "\n";
      } else {
          EV << "loesche Routing Table: " << destination.getIPv4().str() << "\n";
      }
      return;
}

int PASER_Socket::MYgetNumWlanInterfaces() {
    return getNumWlanInterfaces();
}

int PASER_Socket::MYgetNumInterfaces() {
    EV<<"getNumInterfaces()"<<getNumInterfaces()<<endl;
    return getNumInterfaces();
}

double PASER_Socket::MY_getXPos() {
    double xPos;
    //parseIntTo(getParentModule()->getDisplayString().getTagArg("p", 0), xPos);
    xPos = getPosition().x;
    EV << "**********xPos****************" << xPos <<"\n";
    return xPos;
}

double PASER_Socket::MY_getYPos() {
    double yPos;
    //parseIntTo(getParentModule()->getDisplayString().getTagArg("p", 1), yPos);
    yPos = getPosition().y;
    return yPos;
}

bool PASER_Socket::parseIntTo(const char *s, double& destValue) {
    if (!s || !*s)
        return false;

    char *endptr;
    int value = strtol(s, &endptr, 10);

    if (*endptr)
        return false;

    destValue = value;
    return true;
}

void PASER_Socket::sendUDPToIp(cPacket *msg, int srcPort, const ManetAddress& destAddr,
        int destPort, int ttl, double delay, int index) {
    InterfaceEntry *ie = NULL;

    UDPPacket *udpPacket = new UDPPacket(msg->getName());
    //TODO set UDPSIZE
    udpPacket->setByteLength(600);
    udpPacket->encapsulate(msg);
    //IPvXAddress srcAddr = interfaceWlanptr->ipv4Data()->getIPv4();

    if (ttl == 0) {
        // delete and return
        delete msg;
        return;
    }
    // set source and destination port
    udpPacket->setSourcePort(srcPort);
    udpPacket->setDestinationPort(destPort);

    if (index != -1) {
        ie = getInterfaceEntry(index); // The user want to use a pre-defined interface
        EV<<"getInterfaceEntry"<<index<<endl;
    }

    //if (!destAddr.isIPv6())
    if (true) {
        // Send to IPv4
        IPv4Address add = destAddr.getIPv4();
        IPv4Address srcadd;

// If the interface on which the message arrived, use the address of that interface
        if (ie)
            srcadd = ie->ipv4Data()->getIPAddress();
        else {
//            srcadd = hostAddress.getIPv4();
            srcadd =
                    netDevice[getIfIdFromIfIndex(index)].ipaddr.S_addr.getIPv4();
        }

        EV << "Sending app message " << msg->getName() << " over IPv4."
                << " from " << srcadd.str() << " to " << add.str() << "\n";
        IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
        ipControlInfo->setDestAddr(add);
        ipControlInfo->setProtocol(IP_PROT_UDP);
//        ipControlInfo->setProtocol(IP_PROT_MANET);

        ipControlInfo->setTimeToLive(ttl);
        udpPacket->setControlInfo(ipControlInfo);

        if (ie != NULL)
            ipControlInfo->setInterfaceId(ie->getInterfaceId());

        ipControlInfo->setSrcAddr(srcadd);
        sendDelayed(udpPacket, delay, "to_ip");
    }

    // totalSend++;
}

void PASER_Socket::MY_sendICMP(cPacket* pkt) {
    sendICMP(pkt);
}

void PASER_Socket::resetKernelRoutingTable() {
    omnet_clean_rte();
}

#endif
