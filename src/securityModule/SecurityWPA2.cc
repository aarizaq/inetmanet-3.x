/**
*Authors Mohamad.Nehme | Mohamad.Sbeiti \@tu-dortmund.de
*
*copyright (C) 2013 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
* at Technische Universitaet Dortmund, Germany
* http://www.kn.e-technik.tu-dortmund.de/
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later
* version.
* For further information see file COPYING
* in the top level directory
********************************************************************************
* This work is part of the secure wireless mesh networks framework, which is currently under development by CNI */

#include "SecurityWPA2.h"
#include "Ieee80211MgmtAP.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "securityPkt_m.h"
#include "NewMsgWithMacAddr_m.h"
#include "UDP.h"
#include "IPv4InterfaceData.h"
#include "NotificationBoard.h"
#include "Ieee802Ctrl_m.h"
#include "Ieee80211Frame_m.h"
#include "Ieee80211Primitives_m.h"
#include "NotifierConsts.h"
#include "InterfaceTableAccess.h"


#include "ByteArrayMessage.h"
#include "TCPByteStreamRcvQueue.h"

#include <sstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "Ieee80211Frame_m.h"
#include "IPv4Datagram_m.h"
#include "UDPPacket_m.h"

using namespace std;

// delays
//#define c 1.6e-19    // 1.6 x 10^-19
#define enc 89.2873043e-6
#define dec 72.1017e-6
#define mic_add 1.622e-6
#define mic_verify 1.7379e-6
#define rand_gen 60.4234e-6

// message kind values for timers
#define MK_AUTH_TIMEOUT         1
#define GTK_TIMEOUT             3
#define PTK_TIMEOUT             4
#define GK_AUTH_TIMEOUT         5
#define MK_BEACON_TIMEOUT       6

#define MAX_BEACONS_MISSED 600  // beacon lost timeout, in beacon intervals (doesn't need to be integer)


std::ostream& operator<<(std::ostream& os, const SecurityWPA2::LocEntry& e)
{
    os << " Mac Address " << e.macAddr << "\n";
    return os;
}


std::ostream& operator<<(std::ostream& os, const SecurityWPA2::MeshInfo& mesh)
{
    //  SecurityWPA2::MeshStatus status;
    if(mesh.status==0)        os << "state:" << " NOT_AUTHENTICATED ";
    else if(mesh.status==1)   os << "state:" << " AUTHENTICATED ";
    else if(mesh.status==2)   os << "state:" << " WaitingForAck ";
    os << "Mesh addr=" << mesh.address
            << " ssid=" << mesh.ssid
            //    << " beaconIntvl=" << mesh.beaconInterval
            << " beaconTimeoutMsg=" << mesh.beaconTimeoutMsg
            << " authTimeoutMsg_a =" << mesh.authTimeoutMsg_a
            << " authTimeoutMsg_b =" << mesh.authTimeoutMsg_b
            //   << " PTKTimerMsg=" << mesh.PTKTimerMsg
            << " groupAuthTimeoutMsg=" << mesh.groupAuthTimeoutMsg
            << " authSeqExpected=" << mesh.authSeqExpected
            << "  isAuthenticated="  << mesh.isAuthenticated
            << " isCandidate=" << mesh.isCandidate;
    return os;
}

#define MK_STARTUP  1

Define_Module(SecurityWPA2);

simsignal_t SecurityWPA2::AuthTimeoutsNrSignal= SIMSIGNAL_NULL;
simsignal_t SecurityWPA2::BeaconsTimeoutNrSignal= SIMSIGNAL_NULL;
simsignal_t SecurityWPA2::deletedFramesSignal= SIMSIGNAL_NULL;
bool SecurityWPA2::statsAlreadyRecorded;
SecurityWPA2::SecurityWPA2()
{
    // TODO Auto-generated constructor stub
    rt = NULL;
    itable = NULL;
}

SecurityWPA2::~SecurityWPA2()
{
    /*  if(PTKTimer)
        PTKTimer =NULL;
    if(GTKTimer)
        GTKTimer =NULL;*/
}


void SecurityWPA2::initialize(int stage)
{
    if (stage==0)
    {
        statsAlreadyRecorded = false;
        totalAuthTimeout = 0;
        totalBeaconTimeout = 0;
        NrdeletedFrames =0;
        // read parameters
        authenticationTimeout_a = par("authenticationTimeout_a");
        authenticationTimeout_b = par("authenticationTimeout_b");
        groupAuthenticationTimeout = par("groupAuthenticationTimeout");
        ssid = par("ssid").stringValue();
        beaconInterval = par("beaconInterval");
        PTKTimeout = par("PTKTimeout");
        GTKTimeout = par("GTKTimeout");
        PSK =par("PSK").stringValue();

        numAuthSteps = par("numAuthSteps");
        nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_L2_BEACON_LOST);

        activeHandshake = par("activeHandshake");

        counter=0;
        InterfaceTable *ift = (InterfaceTable*)InterfaceTableAccess().getIfExists();
        myIface = NULL;
        if (!ift)
        {
            myIface = ift->getInterfaceByName(getParentModule()->getFullName());
        }

        // subscribe for notifications
        nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_RADIO_CHANNEL_CHANGED);


        EV << "stage \n" << stage << "\n";
        EV << "Init mesh proccess ** mn * \n";
        // read params and init vars
        channelNumber = -1;  // value will arrive from physical layer in receiveChangeNotification()

        //  WATCH(ssid);
        //  WATCH(channelNumber);
        //  WATCH(beaconInterval);
        WATCH(numAuthSteps);
        WATCH_LIST(meshList);
        //  WATCH(PMK);
        // WATCH_LIST(simpleMeshList);
        WATCH(counter);
        // start beacon timer (randomize startup time)
        beaconTimer = new cMessage("beaconTimer");
        scheduleAt(simTime()+uniform(0, beaconInterval), beaconTimer);
        AuthTime=0;
        this->AuthTimeoutsNrSignal=   registerSignal("AuthTimeoutsNr");
        this->BeaconsTimeoutNrSignal=   registerSignal("BeaconTimeoutsNr");
        this->deletedFramesSignal=   registerSignal("deletedFramesNr");
    }

    if (stage==1)
    {
        // obtain our address from MAC
        unsigned int numMac=0;
        cModule *mac = getParentModule()->getSubmodule("mac");
        if (!mac)
        {
            // search for vector of mac:
            do
            {
                mac = getParentModule()->getSubmodule("mac",numMac);
                if (mac)
                    numMac++;
            }
            while (mac);
            if (numMac == 0)
                error("MAC module not found; it is expected to be next to this submodule and called 'mac'");
            else
                mac = getParentModule()->getSubmodule("mac",0);
        }
        myAddress.setAddress(mac->par("address").stringValue());
        EV << "My MAcAddress is: " << myAddress <<endl;
    }


    /** mn */
    else if (stage!=3)
        return;

    rt = RoutingTableAccess().getIfExists();
    itable = InterfaceTableAccess().get();
}

void SecurityWPA2::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else
        handleResponse(msg);
}

void SecurityWPA2::handleResponse(cMessage *msg)
{
    // EV << "handleResponse() " <<msg->getName() << "\n";
    if(strstr(msg->getName() ,"Beacon")!=NULL)
    {
        Ieee80211BeaconFrame *frame= (check_and_cast<Ieee80211BeaconFrame *>(msg));
        handleBeaconFrame(frame);
    }

    else if(strstr(msg->getName() ,"Open Auth-Req")!=NULL || strstr(msg->getName() ,"Open Auth-Resp")!=NULL)
    {
        Ieee80211AuthenticationFrame *frame= (check_and_cast<Ieee80211AuthenticationFrame *>(msg));
        handleOpenAuthenticationFrame(frame);
    }
    else if(strstr(msg->getName() ,"Auth")!=NULL || strstr(msg->getName() ,"Auth-OK")!=NULL || strstr(msg->getName() ,"Auth-ERROR")!=NULL
            || strstr(msg->getName() ,"Auth msg 1/4")!=NULL || strstr(msg->getName() ,"Auth msg 2/4")!=NULL ||
            strstr(msg->getName() ,"Auth msg 3/4")!=NULL || strstr(msg->getName() ,"Auth msg 4/4")!=NULL)
    {
        Ieee80211AuthenticationFrame *frame= (check_and_cast<Ieee80211AuthenticationFrame *>(msg));
        handleAuthenticationFrame(frame);
    }
    else if(strstr(msg->getName() ,"Deauth")!=NULL)
    {
        Ieee80211AuthenticationFrame *frame= (check_and_cast<Ieee80211AuthenticationFrame *>(msg));
        handleDeauthenticationFrame(frame);
    }

    else if(strstr(msg->getName() , "Group msg 1/2")!=NULL || strstr(msg->getName() ,"Group msg 2/2")!=NULL)
    {
        Ieee80211AuthenticationFrame *frame= (check_and_cast<Ieee80211AuthenticationFrame *>(msg));
        handleGroupHandshakeFrame(frame);
    }

    //HWMP
     else if (dynamic_cast<Ieee80211ActionHWMPFrame *>(msg))
    {
       handleIeee80211ActionHWMPFrame(msg);
    }


    else if (dynamic_cast<Ieee80211MeshFrame *>(msg))
    {
        if(!activeHandshake)
        {
            send(msg,"mgmtOut");
            return;
        }
        Ieee80211MeshFrame *frame = (check_and_cast<Ieee80211MeshFrame *>(msg));
        double delay=0;
        if(strstr(msg->getName() ,"EncBrodcast"))
        {
            delay= (double) dec + mic_verify;
            //  send(msg,"mgmtOut");
            MeshInfo *mesh = lookupMesh(frame->getTransmitterAddress());
            if(mesh)
            {
                if(mesh->status==NOT_AUTHENTICATED)
                {
                    EV << frame->getTransmitterAddress() <<"is NOT_AUTHENTICATED"<<endl;
                    return;
                }
            }
            if(!mesh)
            {
                EV << frame->getTransmitterAddress() <<"is unknown"<<endl;
                return;
            }

            sendDelayed(msg, delay,"mgmtOut");

        }
        else if(frame->getReceiverAddress().isBroadcast() || frame->getTransmitterAddress().isBroadcast())
        {
            msg->setName("EncBrodcast");
            // send(msg,"mgmtOut");
            delay= (double) enc + mic_add;
            Ieee80211MeshFrame *frame = (check_and_cast<Ieee80211MeshFrame *>(msg));
                       frame->setByteLength(frame->getByteLength()+16);
                               sendDelayed(frame, delay,"mgmtOut");
        }
        else
        {
            //   send(msg,"mgmtOut");
            handleIeee80211DataFrameWithSNAP(msg);
            //handleIeee80211MeshFrame(msg);

        }

    }
    else if (dynamic_cast<Ieee80211DataFrameWithSNAP *>(msg))
    {
        if(!activeHandshake)
        {
            send(msg,"mgmtOut");
            return;
        }
        double delay=0;
        EV << msg << "..................................................." <<endl;


        Ieee80211DataFrameWithSNAP *frame = (check_and_cast<Ieee80211DataFrameWithSNAP *>(msg));

        if(strstr(msg->getName() ,"EncBrodcast"))
        {
            delay= (double) dec + mic_verify;
            //  send(msg,"mgmtOut");
            MeshInfo *mesh = lookupMesh(frame->getTransmitterAddress());
                  if(mesh)
                  {
                      if(mesh->status==NOT_AUTHENTICATED)
                      {
                          EV << frame->getTransmitterAddress() <<"is NOT_AUTHENTICATED"<<endl;
                          return;
                      }
                  }
                  if(!mesh)
                  {
                      EV << frame->getTransmitterAddress() <<"is unknown"<<endl;
                      return;
                  }


            sendDelayed(msg, delay,"mgmtOut");

        }
        else if(frame->getReceiverAddress().isBroadcast() || frame->getTransmitterAddress().isBroadcast())
        {
            msg->setName("EncBrodcast");
            // send(msg,"mgmtOut");
            EV << "increasing size of broadcast message" << endl;
            delay= (double) enc + mic_add;
            Ieee80211DataFrameWithSNAP *frame = (check_and_cast<Ieee80211DataFrameWithSNAP *>(msg));
              frame->setByteLength(frame->getByteLength()+16);
                      sendDelayed(frame, delay,"mgmtOut");
        }
        else
        {
            //   send(msg,"mgmtOut");
            handleIeee80211DataFrameWithSNAP(msg);
        }


    }

    else
    {
        //   EV <<"SecurityWPA2::handleResponse: msg: " <<msg<<endl;
        EV << msg << "..................................................." <<endl;
        send(msg,"mgmtOut");
    }

}

void SecurityWPA2::handleTimer(cMessage *msg)
{
    EV << "HandleTimer " << msg << endl;
    if(strstr(msg->getName() ,"beaconTimer"))
    {
        sendBeacon();
        scheduleAt(simTime()+beaconInterval, beaconTimer);
    }

    else if (dynamic_cast<newcMessage *>(msg) != NULL)
    {
        newcMessage *newmsg = (check_and_cast<newcMessage *>(msg));
        if(newmsg->getKind())
        {
            if (newmsg->getKind()==MK_BEACON_TIMEOUT)
            {
                EV<<"missed a few consecutive beacons, delete entry" <<endl;
                MeshInfo *mesh = lookupMesh(newmsg->getMeshMACAddress_AuthTimeout());
                if(mesh)
                {
                    totalBeaconTimeout++;
                    emit(BeaconsTimeoutNrSignal, totalBeaconTimeout);
                    EV << "BeaconInterval: " <<mesh->beaconInterval<<endl;
                    EV << "delete mesh " << mesh->address  << " entry from our List" <<endl;
                    clearMeshNode(mesh->address);
                    updateGroupKey();
                }

            }
            else if (newmsg->getKind() == PTK_TIMEOUT)
            {
                EV << "PTK Timeout" << endl;
                MeshInfo *mesh = lookupMesh(newmsg->getMeshMACAddress_AuthTimeout());
                if(mesh)
                {
                    EV << "PTK time out, Mesh address = " << mesh->address << endl;
                    clearMeshNode( mesh->address);
                }
            }
            //Timeout for 4-Way-Handshake
            else if ( newmsg->getKind()==MK_AUTH_TIMEOUT)
            {

                EV << "Authentication time out for 4 WHS" <<endl;
                MeshInfo *mesh = lookupMesh(newmsg->getMeshMACAddress_AuthTimeout());
                if(mesh)
                {
                    totalAuthTimeout++;
                    emit(AuthTimeoutsNrSignal, totalAuthTimeout);
                    EV << "Authentication time out, Mesh address = " << mesh->address << endl;
                    clearMeshNode( mesh->address);
                    // keep listening to Beacons, maybe we'll have better luck next time
                }
            }
            //Timeout for Group key update/Handshake
            else  if ( newmsg->getKind()==GK_AUTH_TIMEOUT)
            {
                EV << "Authentication time out for 2 WHS" <<endl;
                MeshInfo *mesh = lookupMesh(newmsg->getMeshMACAddress_AuthTimeout());
                if(mesh)
                {
                    EV << "Authentication time out, Mesh address = " << mesh->address << endl;
                }
                // keep listening to Beacons, maybe we'll have better luck next time
            }
            else if (newmsg->getKind() == GTK_TIMEOUT)
            {
                EV << "GTK Timeout" << endl;
                updateGroupKey();

                if(newmsg) delete newmsg;
            }
        }
    }


    else
    {
        error("unknown msg");
    }
}

void SecurityWPA2::sendBeacon()
{
    EV << "Sending beacon"<<endl;
    Ieee80211BeaconFrame *frame = new Ieee80211BeaconFrame("Beacon");
    Ieee80211BeaconFrameBody& body = frame->getBody();
    body.setSSID(ssid.c_str());
    body.setBeaconInterval(beaconInterval);
    body.setChannelNumber(channelNumber);
    frame->setReceiverAddress(MACAddress::BROADCAST_ADDRESS);
    frame->setFromDS(true);
    frame->setByteLength(108);
    send(frame,"mgmtOut");
}

void SecurityWPA2::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    //drop all beacons while Authentication
    EV << "Received Beacon frame"<<endl;

    if (!activeHandshake)
        return;

    storeMeshInfo(frame->getTransmitterAddress(), frame->getBody());
    MeshInfo *mesh = lookupMesh(frame->getTransmitterAddress());
    if(mesh)
    {
        // just to avoid undefined states
        if(mesh->authTimeoutMsg_a!=NULL && mesh->authTimeoutMsg_b!=NULL)
        {
            EV << "Authentication in Progress, ignore Beacon"<<endl;
            counter ++;
            if(counter==2)
            {
                clearMeshNode(mesh->address);
                counter=0;
            }
            delete frame;
            return;
        }
        else
        {
            // no Authentication in progress, start negotiation
            if (mesh->status==NOT_AUTHENTICATED && mesh->authTimeoutMsg_a==NULL &&mesh->authTimeoutMsg_b==NULL)
            {
                sendOpenAuthenticateRequest(mesh);
            }
            // if authenticated Mesh, restart beacon timeout
            else if (mesh->status==AUTHENTICATED && mesh->authTimeoutMsg_a==NULL &&mesh->authTimeoutMsg_b==NULL && mesh->sideA==1)
            {
                EV << "Beacon is from authenticated Mesh, restarting beacon timeout timer"<<endl;
                EV << "++++++++++++++++++++++++++++++++++++++++++++++++++" <<endl;
                ASSERT(mesh->beaconTimeoutMsg!=NULL);
                cancelEvent(mesh->beaconTimeoutMsg);
                scheduleAt(simTime()+MAX_BEACONS_MISSED*mesh->beaconInterval, mesh->beaconTimeoutMsg);

            }

        }
    }
    delete frame;
}

void SecurityWPA2::storeMeshInfo(const MACAddress& address, const Ieee80211BeaconFrameBody& body)
{
    MeshInfo *mesh = lookupMesh(address);

    if (mesh)
    {
        EV << "Mesh address=" << address << ", SSID=" << body.getSSID() << " already in our Mesh list, refreshing the info\n";
    }
    else
    {
        EV << "Inserting Mesh address=" << address << ", SSID=" << body.getSSID() << " into our Mesh list\n";
        meshList.push_back(MeshInfo());
        mesh = &meshList.back();
        mesh->status = NOT_AUTHENTICATED;
        mesh->authTimeoutMsg_a=NULL;
        mesh->authTimeoutMsg_b=NULL;

        mesh->authSeqExpected = 1;
        mesh->isCandidate=0;
        mesh->groupAuthTimeoutMsg=NULL;
        mesh->PTKTimerMsg=NULL;
        mesh->sideA=0;
    }
    //mesh->channel = body.getChannelNumber();
    mesh->address = address;
    mesh->ssid = body.getSSID();
    mesh->beaconInterval = body.getBeaconInterval();
}




void SecurityWPA2::clearMeshNode(const MACAddress& address)
{
    EV << "clearMeshNode" <<endl;
    EV << "Node: " << address<<endl;

    MeshInfo *mesh = lookupMesh(address);

    if(mesh)
    {
        if(mesh->beaconTimeoutMsg!=NULL)
        {
            delete cancelEvent(mesh->beaconTimeoutMsg);
            mesh->beaconTimeoutMsg=NULL;
        }
        if(mesh->authTimeoutMsg_a!=NULL)
        {
            delete cancelEvent(mesh->authTimeoutMsg_a);
            mesh->authTimeoutMsg_a=NULL;
        }
        if(mesh->authTimeoutMsg_b!=NULL)
        {
            delete cancelEvent(mesh->authTimeoutMsg_b);
            mesh->authTimeoutMsg_b=NULL;
        }

        if(mesh->groupAuthTimeoutMsg!=NULL)
        {
            delete cancelEvent(mesh->groupAuthTimeoutMsg);
            mesh->groupAuthTimeoutMsg=NULL;
        }
        if(mesh->PTKTimerMsg!=NULL)
        {
            delete cancelEvent(mesh->PTKTimerMsg);
            mesh->PTKTimerMsg=NULL;
        }

        mesh->status=NOT_AUTHENTICATED;
        mesh->isAuthenticated=0;
        mesh->isCandidate=0;
        mesh->authTimeoutMsg_a=NULL;
        mesh->authTimeoutMsg_b=NULL;
        mesh->sideA=0;
        /*for(MeshList::iterator it=meshList.begin(); it != meshList.end();)
        {
            if (it->address == address)
                it = meshList.erase(it);
            else
                ++it;
        }*/
    }
}

int SecurityWPA2::checkAuthState(const MACAddress& address)
{
    for (MeshList::iterator it=meshList.begin(); it!=meshList.end(); ++it)
    {
        if (it->address == address)
            if(it->status==AUTHENTICATED)
                return 1;
    }
    return 0;
}
int SecurityWPA2::checkMac(const MACAddress& address)
{
    if (
            address.str().compare("10:00:00:00:00:00") == 0 ||
            address.str().compare("10:00:00:00:00:01") == 0 ||
            address.str().compare("10:00:00:00:00:02") == 0 ||
            address.str().compare("10-00-00-00-00-00") == 0 ||
            address.str().compare("10-00-00-00-00-01") == 0 ||
            address.str().compare("10-00-00-00-00-02") == 0
    )
        return 1;
    else
    {
        error("unknown Mac Address");
        return 0;
    }
}



SecurityWPA2::MeshInfo *SecurityWPA2::lookupMesh(const MACAddress& address)
{
    for (MeshList::iterator it=meshList.begin(); it!=meshList.end(); ++it)
        if (it->address == address)
            return &(*it);
    return NULL;
}


void SecurityWPA2::sendManagementFrame(Ieee80211ManagementFrame *frame, const MACAddress& address)
{
    frame->setToDS(true);
    frame->setReceiverAddress(address);
    send(frame,"mgmtOut");
}

void SecurityWPA2::sendDelayedManagementFrame(Ieee80211ManagementFrame *frame, const MACAddress& address,  simtime_t delay)
{
    frame->setToDS(true);
    frame->setReceiverAddress(address);
    sendDelayed(frame, delay,"mgmtOut");
}


void SecurityWPA2::sendMeshFrame(Ieee80211MeshFrame *frame, const MACAddress& address)
{
    frame->setToDS(true);
    frame->setReceiverAddress(address);
    send(frame,"mgmtOut");
}
void SecurityWPA2::sendDelayedMeshFrame(Ieee80211MeshFrame *frame, const MACAddress& address, simtime_t delay)
{
    frame->setToDS(true);
    frame->setReceiverAddress(address);
    sendDelayed(frame, delay,"mgmtOut");
}

void SecurityWPA2::sendDataOrMgmtFrame(Ieee80211DataOrMgmtFrame *frame, const MACAddress& address)
{
    frame->setToDS(true);
    frame->setReceiverAddress(address);
    send(frame,"mgmtOut");
}
void SecurityWPA2::sendDelayedDataOrMgmtFrame(Ieee80211DataOrMgmtFrame *frame, const MACAddress& address, simtime_t delay)
{
    frame->setToDS(true);
    frame->setReceiverAddress(address);
    sendDelayed(frame, delay,"mgmtOut");
}

/*------------------------------      Open Authentication   --------------------------------*/


void SecurityWPA2::sendOpenAuthenticateRequest(MeshInfo *mesh)//, simtime_t timeout)
{
    AuthTime =  simTime();
    EV << "Sending Open Auth-Req" <<endl;
    // create and send first authentication frame
    Ieee80211AuthenticationFrame *frame = new Ieee80211AuthenticationFrame("Open Auth-Req");
    frame->getBody().setSequenceNumber(100);
    frame->setByteLength(34);

    mesh->isCandidate=0;
    sendManagementFrame(frame, mesh->address);

    // schedule timeout for side A
    ASSERT(mesh->authTimeoutMsg_a==NULL);
    mesh->authTimeoutMsg_a = new newcMessage("authTimeout", MK_AUTH_TIMEOUT);
    mesh->authTimeoutMsg_a->setMeshMACAddress_AuthTimeout(mesh->address);
    scheduleAt(simTime()+authenticationTimeout_a, mesh->authTimeoutMsg_a);
}

void SecurityWPA2::handleOpenAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    EV <<frame->getTransmitterAddress() <<endl;
    int frameAuthSeq = frame->getBody().getSequenceNumber();

    // create Mesh entry if needed
    MeshInfo *mesh = lookupMesh(frame->getTransmitterAddress());
    if (!mesh)
    {
        EV<<"create Entry" <<endl;
        MACAddress meshAddress = frame->getTransmitterAddress();
        // Candidate For Authentication
        meshList.push_back(MeshInfo());// this implicitly creates a new entry
        mesh = &meshList.back();
        mesh->address = meshAddress;
        mesh->status = NOT_AUTHENTICATED;
        mesh->authSeqExpected = 1;
        mesh->isCandidate=0;
        mesh->beaconInterval=beaconInterval;
        mesh->authTimeoutMsg_a=NULL;
        mesh->authTimeoutMsg_b=NULL;
        mesh->groupAuthTimeoutMsg=NULL;
        mesh->sideA=0;
    }
    if(strstr(frame->getName() ,"Open Auth-Req") || strstr(frame->getName() ,"Open Auth-Resp"))
    {
        if(frameAuthSeq ==100)
        {
            if (mesh->authTimeoutMsg_b!=NULL)
            {
                EV << "previous authentication attempt was broken" <<endl;;
                clearMeshNode(mesh->address);
            }

            EV << " <<< Open Auth 1/2 >>>"<<endl;
            EV << "Open Auth-Req arrived, send Response"<<endl;

            Ieee80211AuthenticationFrame *resp = new Ieee80211AuthenticationFrame("Open Auth-Resp");
            resp->getBody().setSequenceNumber(frameAuthSeq+100);
            resp->getBody().setStatusCode(SC_SUCCESSFUL);
            resp->setByteLength(34);

            ASSERT(mesh->authTimeoutMsg_b==NULL);
            mesh->authTimeoutMsg_b = new newcMessage("authTimeout", MK_AUTH_TIMEOUT);
            mesh->authTimeoutMsg_b->setMeshMACAddress_AuthTimeout(mesh->address);
            scheduleAt(simTime()+authenticationTimeout_b, mesh->authTimeoutMsg_b);

            sendManagementFrame(resp, frame->getTransmitterAddress());

            delete frame;
            return;
        }

        else if(frameAuthSeq ==200)
        {


            if (mesh->authTimeoutMsg_a==NULL)
            {
                EV << "No Authentication in progress, ignoring frame\n";
                EV << mesh->authTimeoutMsg_a <<endl;
                // error("HandleAuth. frameAuthSeq == 2");
                clearMeshNode(mesh->address);
                delete frame;
                return;
            }
            EV << "<<< Open Auth 2/2 >>>" <<endl;
            EV << "Open Auth-Resp arrived, start authentication \n";
            checkForAuthenticationStart(mesh);
            delete frame;
            return;
        }
        else
            error( "Error in Open Authentication");
        return;
    }
    else
        error( "Error in Open Authentication_");
}

/*------------------------------       Authentication   --------------------------------*/

void SecurityWPA2::checkForAuthenticationStart(MeshInfo *mesh)
{
    if (mesh)
    {
        if (mesh->authTimeoutMsg_a!=NULL && mesh->authTimeoutMsg_b==NULL)
        {
            EV <<"Start Authentication"<<endl;
            Ieee80211Prim_AuthenticateRequest *ctrl = new  Ieee80211Prim_AuthenticateRequest();
            ctrl->setTimeout(authenticationTimeout_a);
            startAuthentication(mesh, ctrl->getTimeout());
        }
    }
}


void SecurityWPA2::startAuthentication(MeshInfo *mesh, simtime_t timeout)
{
    /************************************************************************************************
     *
     * 1. |NonceA|                                               (length = 95)
     * 2. |NonceB| Mic_KCK(msg2)|                              (length = 117)
     * 3. |ok1| Enc_KEK(GTK)| Mic_KCK(msg3)|                     (length = 151)
     * 4. |ok2 - Mic(msg4)|                                       (length = 95)
     *
     *
     * Delay: 4-WHS:
     * msg 1/4: rand_gen
     * msg 2/4: mic_add + rand_gen
     * msg 3/4: mic_verify + mic_add + enc
     * msg 4/4: mic_verify + decr + mic_add + mic_verify
     *
     * **********************************************************************************************/

    double  delay=0;

    // generate & save PMK(256) = (PSK XOR SSID)
    clearKey256(PMK);
    PMK=computePMK(PSK,ssid);

    // generate NonceA
    SecurityPkt *msg = new SecurityPkt();
    clearNonce(mesh->NonceA);
    mesh->NonceA = generateNonce();
    mesh->NonceA.len=256;
    // pass NonceA
    for(int i=0;i<12;i++)
        msg->setDescriptor_Type(i,'0');

    msg->setKey_Nonce(mesh->NonceA);
    msg->setDescriptor_Type(4,'1');

    EV << "Sending initial Authentication frame with seqNum=1"<<endl;
    EV << "Sending NonceA" <<endl;
    // create and send first authentication frame
    Ieee80211AuthenticationFrame *frame = new Ieee80211AuthenticationFrame("Auth msg 1/4");
    frame->getBody().setSequenceNumber(1);

    //Set Nonce A in Frame
    msg->setByteLength(103);
    frame->encapsulate(msg);

    delay= (double) rand_gen;
    //XXX frame length could be increased to account for challenge text length etc.
    sendDelayedManagementFrame(frame, mesh->address,delay);
    mesh->authSeqExpected = 2;
}



void SecurityWPA2::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    EV <<"handleAuthenticationFrame" <<endl;
    int frameAuthSeq = frame->getBody().getSequenceNumber();
    MeshInfo *mesh = lookupMesh(frame->getTransmitterAddress());
    if(mesh)
        EV << mesh->address<<endl;
    SecurityPkt * msg= (SecurityPkt *)frame->decapsulate();
    double delay=0;
    if(!msg)
    {
        EV << "Frame decapsulation failed!"<<endl;
        return;
    }

    if(mesh)
    {
        if(mesh->authTimeoutMsg_a!=NULL && mesh->authTimeoutMsg_b!=NULL)
        {
            EV << "illegal state!" <<endl;
            return;
        }
    }

    // reset authentication status, when starting a new auth sequence
    if(strstr(frame->getName() ,"Auth-OK msg 4/4") && frameAuthSeq==4)
    {
        EV << "<<<<< 4 >>>>>"<<endl;

        if (mesh->authTimeoutMsg_a==NULL)
        {
            EV << "No Authentication in progress, ignoring frame\n";
            EV << mesh->authTimeoutMsg_a <<endl;
            //error("HandleAuth. frameAuthSeq == 2");
            clearMeshNode(mesh->address);
            delete frame;
            return;
        }

        // authentication completed
        mesh->authSeqExpected =1 ;

        EV << "Initiator: Authentication with Mesh-Peer completed"<<endl;

        ASSERT(mesh->authTimeoutMsg_a!=NULL);
        delete cancelEvent(mesh->authTimeoutMsg_a);
        mesh->authTimeoutMsg_a = NULL;

        if(mesh->beaconTimeoutMsg!=NULL)
        {
            delete cancelEvent(mesh->beaconTimeoutMsg);
            mesh->beaconTimeoutMsg = NULL;
        }

        mesh->beaconTimeoutMsg = new newcMessage("beaconTimeout");
        mesh->beaconTimeoutMsg->setMeshMACAddress_AuthTimeout(mesh->address);
        mesh->beaconTimeoutMsg->setKind(MK_BEACON_TIMEOUT);
        scheduleAt(simTime()+MAX_BEACONS_MISSED*mesh->beaconInterval, mesh->beaconTimeoutMsg);
        mesh->sideA=1;

        //verify Mic (NonceB)
        for(int i=0;i<12;i++)
            msg->setDescriptor_Type(i,'0');
        msg->setDescriptor_Type(1,'1');
        clearKey128(mesh->TempMIC);
        mesh->TempMIC = computeMic128(mesh->KCK, msg);

        if( mesh->TempMIC.buf.at(0) == msg->getMic().buf.at(0) &&  mesh->TempMIC.buf.at(1) == msg->getMic().buf.at(1))
        {
            EV << "Mic( ok1) verifying was successful" << endl;
            EV << "ok2 ! Install keys" << endl;
            mesh->TempMIC.buf.clear();

            // The pairwise key shall be from the 4-Way Handshake initiated by the STA with the highest MAC address.
            if(frame->getReceiverAddress().getInt()>mesh->address.getInt())
            {
                clearKey384(mesh->PTK_final);
                mesh->PTK_final= mesh->PTK_a;
            }
            else
            {
                clearKey384(mesh->PTK_final);
                mesh->PTK_final= mesh->PTK_b;
            }
            derivePTKKeys(mesh, mesh->PTK_final);

        }
        else
        {
            EV << "Mic verifying failed" << endl;
         //   error("msg 4/4: Mic verifying failed");
            //discards silently Message 2
            clearMeshNode(mesh->address);
            return;
        }
        GTKTimer = new newcMessage("GTKTimer");
        GTKTimer->setKind(GTK_TIMEOUT);
        scheduleAt(simTime()+GTKTimeout, GTKTimer);

        AuthTime=  simTime()-AuthTime;

        if (mesh->authTimeoutMsg_b!=NULL)
        {

            delete cancelEvent(mesh->authTimeoutMsg_b);
            mesh->authTimeoutMsg_b = NULL;

        }

        EV <<".............................. AuthTime: " << AuthTime <<endl;
        delete frame;
        delete msg;
        return;
    }

    if (frameAuthSeq == 1)
    {
        if (mesh->authTimeoutMsg_b==NULL)
        {
            EV << "No Authentication in progress, ignoring frame\n";
            EV << mesh->authTimeoutMsg_b <<endl;
            //error("HandleAuth. frameAuthSeq == 2");
            clearMeshNode(mesh->address);
            delete frame;
            return;
        }
        mesh->authSeqExpected = 1;
        mesh->isAuthenticated =0;
        EV << "<<<<< 1 >>>>>"<<endl;

        // extract NonceA
        clearNonce(mesh->NonceA);
        mesh->NonceA=msg->getKey_Nonce();

        //generate NonceB
        clearNonce(mesh->NonceB);
        mesh->NonceB=generateNonce();

        //compute PTK
        if(!PMK.buf.empty())
            PMK.buf.clear();
        PMK=computePMK(PSK,ssid);
        if(!mesh->PTK.buf.empty())
            mesh->PTK.buf.clear();
        mesh->PTK= computePTK(PMK, mesh->NonceA, mesh->NonceB, frame->getReceiverAddress(), mesh->address);
        // PTK = KEC | KEK | TK
        derivePTKKeys(mesh, mesh->PTK);

        clearKey384(mesh->PTK_a);
        mesh->PTK_a=mesh->PTK;
        EV << "Sending NonceB, Mic(NonceB)" <<endl;
        // pass NonceB and MIC_NonceB
        for(int i=0;i<12;i++)
            msg->setDescriptor_Type(i,'0');

        msg->setKey_Nonce(mesh->NonceB);
        msg->setDescriptor_Type(4,'1');
        //compute Mic(NonceB)
        msg->setMic(computeMic128(mesh->KCK, msg));
        msg->setByteLength(125);

        delay=(double) mic_add + rand_gen;

        EV   << msg->getMic().buf.at(0) <<   " " <<msg->getMic().buf.at(1)<<endl;

    }

    if (frameAuthSeq == 2)
    {
        if (mesh->authTimeoutMsg_a==NULL)
        {
            EV << "No Authentication in progress, ignoring frame\n";
            EV << mesh->authTimeoutMsg_a <<endl;
            //error("HandleAuth. frameAuthSeq == 2");
            clearMeshNode(mesh->address);
            delete frame;
            return;
        }


        EV << "<<<<< 2 >>>>>"<<endl;
        // extract NonceB & MIC(NonceB)
        clearNonce(mesh->NonceB);
        mesh->NonceB=msg->getKey_Nonce();

        //compute PTK(256)= KEK XOR (Z1 XOR Z2 XOR MACA XOR MACB)
        clearKey384(mesh->PTK);
        mesh->PTK = computePTK(PMK, mesh->NonceA, mesh->NonceB, frame->getReceiverAddress(), mesh->address);
        derivePTKKeys(mesh, mesh->PTK);

        clearKey384(mesh->PTK_b);
        mesh->PTK_b=mesh->PTK;
        //verify Mic (NonceB)

        for(int i=0;i<12;i++)
            msg->setDescriptor_Type(i,'0');
        msg->setKey_Nonce(mesh->NonceB);
        msg->setDescriptor_Type(4,'1');
        clearKey128(mesh->TempMIC);
        mesh->TempMIC = computeMic128(mesh->KCK, msg);

        EV <<  mesh->TempMIC.buf.at(0) << "==" << msg->getMic().buf.at(0)  <<"&&"<<  mesh->TempMIC.buf.at(1)<< "==" <<msg->getMic().buf.at(1)<<endl;

        if( mesh->TempMIC.buf.at(0) == msg->getMic().buf.at(0) &&  mesh->TempMIC.buf.at(1) == msg->getMic().buf.at(1))
        {
            EV << "Mic verifying was successful" << endl;
            //Pick Random GTK(128)
            if(GTK.buf.empty())
            {
                GTK.buf.push_back(genk_intrand(1,1073741823));
                GTK.buf.push_back(genk_intrand(1,1073741823));
                GTK.len=128;
            }
            EV<< "GTK vor decrypt:" << endl;EV<< GTK.buf.at(0) << " ";EV<< GTK.buf.at(1) <<  endl;
            //Encrypt GTK with KEK
            EV << "Encrypted GTK: " << endl;
            msg->setKey_Data128( encrypt128(mesh->KEK , GTK));

            //ok1 "is sent by the authenticator to tell the supplicant that it is ready to start using the new keys for encryption.
            // This Message includes a MIC check so the supplicant can verify that the authenticator has a matching PMK
            // set enabled_fields to 0
            for(int i=0;i<12;i++)
                msg->setDescriptor_Type(i,'0');
            msg->setDescriptor_Type(1,'1');
            msg->setDescriptor_Type(10,'1');
            //ok1
            msg->setKey_Info(1);
            msg->setMic(computeMic128(mesh->KCK, msg));
            msg->setByteLength(159);

            EV<< " " << msg->getMic().buf.size() << " " << msg->getMic().buf.at(0)  << "  "<< msg->getMic().buf.at(1)<<endl;

            EV << "ok1, Enk(GTK), Mic(ok1, Enk(GTK))," <<endl;
            // mic.buf.empty();
            delay= (double)mic_verify +mic_add+enc;



        }
        else
        {
            EV << "Mic verifying failed" << endl;
           // error("msg 2/4: Mic verifying failed");
            clearMeshNode(mesh->address);

            //discards silently Message 2
            return;
        }
    }



    if (frameAuthSeq == 3)
    {
        if (mesh->authTimeoutMsg_b==NULL)
        {
            EV << "No Authentication in progress, ignoring frame\n";
            EV << mesh->authTimeoutMsg_b <<endl;
            //error("HandleAuth. frameAuthSeq == 2");
            clearMeshNode(mesh->address);
            delete frame;
            return;
        }
        EV << "<<<<< 3 >>>>>"<<endl;
        EV << "ok1 !" << endl;
        //Decrypt GTK
        EV<< "GTK nach decrypt:" <<endl;
        clearKey128(mesh->GTK);
        mesh->GTK = decrypt128(mesh->KEK, msg->getKey_Data128());

        //verify Mic (NonceB)
        for(int i=0;i<12;i++)
            msg->setDescriptor_Type(i,'0');
        msg->setDescriptor_Type(1,'1');
        msg->setDescriptor_Type(10,'1');
        clearKey128(mesh->TempMIC);
        mesh->TempMIC = computeMic128(mesh->KCK, msg);

        if(msg->getMic().buf.size()>=2 &&  mesh->TempMIC.buf.at(0) == msg->getMic().buf.at(0) &&  mesh->TempMIC.buf.at(1) == msg->getMic().buf.at(1))
        {
            EV << "Mic( ok1, Enc(GTK) ) verifying was successful" << endl;
            for(int i=0;i<12;i++)
                msg->setDescriptor_Type(i,'0');
            msg->setKey_Info(1);
            msg->setDescriptor_Type(1,'1');
            msg->setMic(computeMic128(mesh->KCK, msg));
            msg->setByteLength(103);
            EV << "Sending ok2, Mic(ok2)" <<endl;
            delay= (double)mic_verify+ mic_add + dec + mic_verify;

            EV <<"statusCode : MHN : "<<frame->getBody().getStatusCode()<< endl;

            int statusCode = frame->getBody().getStatusCode();
            if (statusCode==SC_TBTT_REQUEST)
            {
                EV <<" mesh->isCandidate=1 wurde gesetzt!" <<endl;
                mesh->isCandidate=1;
            }
        }

        else
        {
            EV << "Mic(ok2) verifying failed"<<endl;
           // error("msg 3/4: Mic verifying failed");
            clearMeshNode(mesh->address);
            //discards silently Message 2
            return;
        }
    }
    // station is authenticated if it made it through the required number of steps
    bool isLast = (frameAuthSeq+1 == numAuthSteps);
    // check authentication sequence number is OK
    if (frameAuthSeq > mesh->authSeqExpected)
    {
        EV << "frameAuthSeq: " << frameAuthSeq<<endl;
        EV << "Wrong sequence number, " << mesh->authSeqExpected << " expected\n";
        Ieee80211AuthenticationFrame *resp = new Ieee80211AuthenticationFrame("Auth-ERROR");
        resp->getBody().setStatusCode(SC_AUTH_OUT_OF_SEQ);
        sendManagementFrame(resp, frame->getTransmitterAddress());
        delete frame;
        delete msg;
        mesh->authSeqExpected = 1; // go back to start square
        mesh->isAuthenticated=0;
        mesh->isCandidate=0;
      //  error("Auth-ERROR");
        clearMeshNode(mesh->address);
        return;
    }



    // send OK response (we don't model the cryptography part, just assume successful authentication every time)
    EV << "Sending Authentication frame, seqNum=" << (frameAuthSeq+1) << "\n";
    std::stringstream buffer;
    buffer << "Auth msg " <<frameAuthSeq +1<< "/4" << std::endl;

    const char* p  = buffer.str().c_str();
    Ieee80211AuthenticationFrame *resp = new Ieee80211AuthenticationFrame(isLast ? "Auth-OK msg 4/4" : p);
    resp->getBody().setSequenceNumber(frameAuthSeq+1);

    if(frameAuthSeq +1== 3)
    {  // pass status to other party
        EV <<"MHN"<<endl;
        if(mesh->status==NOT_AUTHENTICATED)
            resp->getBody().setStatusCode(SC_TBTT_REQUEST);

    }
    resp->getBody().setIsLast(isLast);
    resp->encapsulate(msg);

    EV << "Delay: " <<delay <<endl;
    sendDelayedManagementFrame(resp, frame->getTransmitterAddress(),delay);

    // update status
    if (isLast)
    {
        EV << "Authentication successful\n";
        mesh->authSeqExpected = 1;

        //wait for Ack
        nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_TX_ACKED);
        mesh->status=WaitingForAck;
        delete frame;
        mesh->isAuthenticated=3;
    }

    else
    {
        mesh->authSeqExpected += 2;
        EV << "Expecting Authentication frame " << mesh->authSeqExpected << endl;
        delete frame;
    }

}



/*----------------------------------------       Group Handshake           ---------------------------------------------*/

void SecurityWPA2::updateGroupKey()
{
    EV << "Group key update\n";
    for (MeshList::iterator it=meshList.begin(); it!=meshList.end(); ++it)
    {
        if (it->status == AUTHENTICATED)
        {
            EV<< "GTK Update for:" << it->address << endl;
            MeshInfo *mesh = lookupMesh(it->address);
            if(mesh)
            {
                if(mesh->PTK_final.buf.size()>=2)
                {
                    Ieee80211Prim_AuthenticateRequest *ctrl = new  Ieee80211Prim_AuthenticateRequest();
                    ctrl->setTimeout(groupAuthenticationTimeout);
                    if(mesh->groupAuthTimeoutMsg==NULL)
                        sendGroupHandshakeMsg(mesh,ctrl->getTimeout());
                }
            }
            EV << "---------------------------------------" <<endl;
        }
    }
}



void SecurityWPA2::sendGroupHandshakeMsg(MeshInfo *mesh,  simtime_t timeout)
{

    EV << "Initialize Group key Handshake\n";
    if(mesh->groupAuthTimeoutMsg!=NULL)
        error("startAuthentication: already authenticated with Mesh address=", mesh->address.str().c_str());
    double delay=0;

    EV << " Final: " << mesh->PTK_final.buf.at(0) << ""<< mesh->PTK_final.buf.at(1) <<endl;

    // create and send first authentication frame
    Ieee80211AuthenticationFrame *frame = new Ieee80211AuthenticationFrame("Group msg 1/2");
    frame->getBody().setSequenceNumber(1);

    SecurityPkt *msg = new SecurityPkt();
    //Pick Random GTK(128)
    clearKey128(GTK);
    GTK.buf.push_back(genk_intrand(1,1073741823));
    GTK.buf.push_back(genk_intrand(1,1073741823));
    GTK.len=128;
    //Encrypt GTK with KEK
    EV << "Encrypted GTK: " << endl;
    if(mesh->KEK.buf.size()<2)
        error("sendGroupHandshakeMsg: No KEK key found!");
    if(mesh->KCK.buf.size()<2)
        error("sendGroupHandshakeMsg: No KCK key found!");
    msg->setKey_Data128( encrypt128(mesh->KEK, GTK) );
    for(int i=0;i<12;i++)
        msg->setDescriptor_Type(i,'0');
    //compute mic
    msg->setDescriptor_Type(1,'1');
    msg->setKey_Info(1);
    msg->setMic(computeMic128(mesh->KCK, msg));
    EV << "Sending OK1, Enk(GTK), Mic( OK1,  Enk(GTK))" <<endl;
    frame->encapsulate(msg);

    //   sendManagementFrame(frame, mesh->address);
    delay = (double) enc + mic_add + rand_gen;
    EV <<mesh->address<<endl;
    frame->setByteLength(125);
    sendDelayedManagementFrame(frame, mesh->address,delay);
    // schedule timeout
    ASSERT(mesh->groupAuthTimeoutMsg==NULL);
    mesh->groupAuthTimeoutMsg = new newcMessage("groupauthTimeout", GK_AUTH_TIMEOUT);
    mesh->groupAuthTimeoutMsg->setMeshMACAddress_AuthTimeout(mesh->address);
    scheduleAt(simTime()+timeout, mesh->groupAuthTimeoutMsg);

}

void SecurityWPA2::handleGroupHandshakeFrame(Ieee80211AuthenticationFrame *frame)
{
    int frameAuthSeq = frame->getBody().getSequenceNumber();
    SecurityPkt * msg= (SecurityPkt *)frame->decapsulate();
    double delay=0;
    if(!msg)
        error("handleGroupHandshakeFrame: frame->decapsulate()");

    MeshInfo *mesh = lookupMesh(frame->getTransmitterAddress());

    if(mesh)
    {
        if(strstr(frame->getName() ,"Group msg 1/2") || strstr(frame->getName() ,"Group msg 2/2"))
        {
            if(frameAuthSeq ==1)
            {
                if(checkAuthState(frame->getTransmitterAddress())==0)
                {
                    return;
                }

                EV << "Group msg 1/2 arrived"<<endl;
                Ieee80211AuthenticationFrame *resp = new Ieee80211AuthenticationFrame("Group msg 2/2");
                resp->getBody().setSequenceNumber(frameAuthSeq+1);
                resp->getBody().setStatusCode(SC_SUCCESSFUL);

                //verify Mic (ok1)
                for(int i=0;i<12;i++)
                    msg->setDescriptor_Type(i,'0');
                msg->setDescriptor_Type(1,'1');

                if(mesh->KCK.buf.size()<2)
                    error("mesh->KCK is empty: '%d'", mesh->KCK.buf.size());
                if(mesh->KEK.buf.size()<2)
                    error("mesh->KEK is empty: '%d'", mesh->KEK.buf.size());
                clearKey128(mesh->TempMIC);
                mesh->TempMIC = computeMic128(mesh->KCK, msg);

                if( ( mesh->TempMIC.buf.at(0) == msg->getMic().buf.at(0)) && ( mesh->TempMIC.buf.at(1) == msg->getMic().buf.at(1)))
                {
                    EV << "Mic(ok1) verifying was successful" << endl;
                    //Install key
                    if(msg->getKey_Info()==1)
                    {
                        EV << "Install keys" << endl;
                        clearKey128(mesh->GTK);
                        mesh->GTK = decrypt128(mesh->KEK, msg->getKey_Data128());
                        EV << "Sending OK2, Mic( OK2 )" <<endl;
                        msg->setKey_Info(1);
                    }
                    else
                        msg->setKey_Info(0);

                    for(int i=0;i<12;i++)
                        msg->setDescriptor_Type(i,'0');
                    msg->setDescriptor_Type(1,'1');
                    msg->setMic( mesh->TempMIC);
                    msg->setKey_Info(1);

                    resp->encapsulate(msg);
                    resp->setByteLength(103);
                    //sendManagementFrame(resp, frame->getTransmitterAddress());
                    delay= (double)dec + mic_verify;
                    sendDelayedManagementFrame(resp, frame->getTransmitterAddress(),delay);
                    delete frame;

                    return;
                }
                else
                   // error("Mic verifying failed msg1/2");
                    return;
            }


            else if(frameAuthSeq ==2)
            {
                if(checkAuthState(frame->getTransmitterAddress())==0)
                {
                    if(mesh->groupAuthTimeoutMsg!=NULL)
                    {
                        delete cancelEvent(mesh->groupAuthTimeoutMsg);
                        mesh->groupAuthTimeoutMsg = NULL;
                        return;
                    }
                }

                EV << "Group msg 2/2 arrived";
                //verify Mic (ok2)
                for(int i=0;i<12;i++)
                    msg->setDescriptor_Type(i,'0');
                msg->setDescriptor_Type(1,'1');

                if(mesh->KCK.buf.size()<2)
                    error("mesh->KCK is empty: '%d'", mesh->KCK.buf.size());
                clearKey128(mesh->TempMIC);
                mesh->TempMIC = computeMic128(mesh->KCK, msg);
                if(msg->getKey_Info()==0)
                {
                    //Dont't install keys
                    return;
                }

                if( mesh->TempMIC.buf.at(0) == msg->getMic().buf.at(0) &&  mesh->TempMIC.buf.at(1) == msg->getMic().buf.at(1))
                {
                    EV << "Mic verifying was successful" << endl;
                }
                else
                {
                    //error("Mic verifying failed msg 2/2");
                    EV << "Mic verifying failed " <<endl;
                    mesh->GTK.buf.clear();
                    mesh->GTK.len=0;
                    //   clearMeshNode(mesh->address);
                    return;
                }

                //    ASSERT(mesh->groupAuthTimeoutMsg!=NULL);
                if(mesh->groupAuthTimeoutMsg!=NULL)
                    delete cancelEvent(mesh->groupAuthTimeoutMsg);
                mesh->groupAuthTimeoutMsg = NULL;
                //   mic.buf.clear();

                GTKTimer = new newcMessage("GTKTimer");
                GTKTimer->setKind(GTK_TIMEOUT);
                scheduleAt(simTime()+GTKTimeout, GTKTimer);
                delete frame;
                delete msg;
            }
            //   delete msg;
        }
        else{
            EV << "Error in Group Key msg \n";
            error("Group Key msg");
            return;
        }
    }
}

/*-------------------------------------------            ---------------------------------------------*/

IPv4Datagram*  SecurityWPA2::handleIPv4Datagram(IPv4Datagram* IP, MeshInfo *mesh)
{
    /*
     short version_var;
     short headerLength_var;
     IPv4Address srcAddress_var;
     IPv4Address destAddress_var;
     int transportProtocol_var;
     short timeToLive_var;
     int identification_var;
     bool moreFragments_var;
     bool dontFragment_var;
     int fragmentOffset_var;
     unsigned char typeOfService_var;
     int optionCode_var;
     IPv4RecordRouteOption recordRoute_var;
     IPv4TimestampOption timestampOption_var;
     IPv4SourceRoutingOption sourceRoutingOption_var;
     unsigned int totalPayloadLength_var;
     */

    if(mesh){
        /*   EV << "Before: " <<endl;
        EV <<IP->getHeaderLength() <<endl;
        EV << "" << IP->getSrcAddress() <<endl;
        EV << "" << IP->getDestAddress() <<endl;
        EV << "" << IP->getTransportProtocol() <<endl;
        EV << "" << IP->getTimeToLive() <<endl;
        EV << "" << IP->getIdentification()<<endl;
        //    EV << "" << IP->getMoreFragments() <<endl;
        //EV << "" << IP->getDontFragment() <<endl;
        //EV << "" << IP->getFragmentOffset() <<endl;
        EV << "" << IP->getTypeOfService() <<endl;
        EV << "" << IP->getOptionCode() <<endl;
        EV << "" << IP->getTotalPayloadLength() <<endl;*/
        if(!mesh->KEK.buf.size()<2)
        {

            IP->setHeaderLength(IP->getHeaderLength()^mesh->KEK.buf.at(0));
            IP->setSrcAddress(IPv4Address((IP->getSrcAddress().getInt())^mesh->KEK.buf.at(0)));
            IP->setDestAddress(IPv4Address((IP->getDestAddress().getInt())^mesh->KEK.buf.at(0)));
            IP->setTransportProtocol(IP->getTransportProtocol()^mesh->KEK.buf.at(0));
            IP->setTimeToLive(IP->getTimeToLive()^mesh->KEK.buf.at(0));
            IP->setIdentification(IP->getIdentification()^mesh->KEK.buf.at(0));
            //IP->setMoreFragments(IP->getMoreFragments()^mesh->KEK.buf.at(0));
            //IP->setDontFragment(IP->getDontFragment()^mesh->KEK.buf.at(0));
            //IP->setFragmentOffset(IP->getFragmentOffset()^mesh->KEK.buf.at(0));
            IP->setTypeOfService(IP->getTypeOfService()^mesh->KEK.buf.at(0));
            IP->setOptionCode(IP->getOptionCode()^mesh->KEK.buf.at(0));
            IP->setTotalPayloadLength(IP->getTotalPayloadLength()^mesh->KEK.buf.at(0));

            /*     IPv4RecordRouteOption recordRoute_var;
               IPv4TimestampOption timestampOption_var;
               IPv4SourceRoutingOption sourceRoutingOption_var;*/

        }
        else
            error("KEK is too short");

        /*     EV << "Later: " <<endl;
        EV <<IP->getHeaderLength() <<endl;
        EV << "" << IP->getSrcAddress() <<endl;
        EV << "" << IP->getDestAddress() <<endl;
        EV << "" << IP->getTransportProtocol() <<endl;
        EV << "" << IP->getTimeToLive() <<endl;
        EV << "" << IP->getIdentification()<<endl;
        //    EV << "" << IP->getMoreFragments() <<endl;
        //EV << "" << IP->getDontFragment() <<endl;
        //EV << "" << IP->getFragmentOffset() <<endl;
        EV << "" << IP->getTypeOfService() <<endl;
        EV << "" << IP->getOptionCode() <<endl;
        EV << "" << IP->getTotalPayloadLength() <<endl;*/
    }
    return IP;
}

Ieee80211ActionHWMPFrame *  SecurityWPA2::encryptActionHWMPFrame(Ieee80211ActionHWMPFrame* frame, MeshInfo *mesh)
{
    /* PREQ:
     *      bodyLength = 26;
     *      id = IE11S_PREQ;
     *      unsigned int pathDiscoveryId;
     *      MACAddress originator;
     *      unsigned int originatorSeqNumber;
     *      MACAddress originatorExternalAddr;
     *      unsigned int lifeTime;
     *      unsigned int metric = 0;
     *      unsigned char targetCount;
     */

    /* PREP:
     *      bodyLength = 37;
     *      id = IE11S_PREP;
     *      MACAddress target;
     *      unsigned int targetSeqNumber;
     *      MACAddress tagetExternalAddr;
     *      unsigned int lifeTime;
     *      unsigned int metric = 0;
     *      MACAddress originator;
     *      unsigned int originatorSeqNumber;
     */

    if(strstr(frame->getClassName() ,"Ieee80211ActionPREQFrame")!=NULL)
    {
        Ieee80211ActionPREQFrame *preq = (check_and_cast<Ieee80211ActionPREQFrame *>(frame));
        if(preq)
        {
            Ieee80211ActionPREQFrameBody body = preq->getBody();
            EV << "vorher: "<<endl;
            EV <<  body.getBodyLength()<<endl;
            EV <<  body.getId()<<endl;
            EV <<  body.getPathDiscoveryId()<<endl;
            EV <<  body.getOriginator()<<endl;
            EV << body.getOriginatorSeqNumber()<<endl;
            EV <<  body.getOriginatorExternalAddr()<<endl;
            EV << body.getLifeTime()<<endl;
            EV << body.getMetric()<<endl;

            body.setBodyLength(body.getBodyLength()^4);
            body.setId(body.getId()^4);
            body.setPathDiscoveryId(body.getPathDiscoveryId()^4);
            body.setOriginator(MACAddress(body.getOriginator().getInt()^4));
            body.setOriginatorSeqNumber(body.getOriginatorSeqNumber()^4);
            body.setOriginatorExternalAddr(MACAddress(body.getOriginatorExternalAddr().getInt()^4));
            body.setLifeTime(body.getLifeTime()^4);
            body.setMetric(body.getMetric()^4);

            EV << "nachher: "<<endl;
            EV <<  body.getBodyLength()<<endl;
            EV <<  body.getId()<<endl;
            EV <<  body.getPathDiscoveryId()<<endl;
            EV <<  body.getOriginator()<<endl;
            EV << body.getOriginatorSeqNumber()<<endl;
            EV <<  body.getOriginatorExternalAddr()<<endl;
            EV << body.getLifeTime()<<endl;
            EV << body.getMetric()<<endl;
            preq->setBody(body);
            return preq;
        }
    }


    else if(strstr(frame->getClassName() ,"Ieee80211ActionPREPFrame")!=NULL){
        Ieee80211ActionPREPFrame *prep = (check_and_cast<Ieee80211ActionPREPFrame *>(frame));
        if(prep)
        {
            Ieee80211ActionPREPFrameBody body = prep->getBody();
            EV << "vorher: "<<endl;
            EV <<  body.getBodyLength()<<endl;
            EV <<  body.getId()<<endl;
            EV << body.getTarget()<<endl;
            EV << body.getTargetSeqNumber()<<endl;
            EV << body.getTagetExternalAddr()<<endl;
            EV << body.getLifeTime()<<endl;
            EV <<  body.getOriginator()<<endl;
            EV << body.getOriginatorSeqNumber()<<endl;

            body.setBodyLength(body.getBodyLength()^4);
            body.setId(body.getId()^4);
            body.setTarget(MACAddress(body.getTarget().getInt()^4));
            body.setTargetSeqNumber(body.getTargetSeqNumber()^4);
            body.setTagetExternalAddr(MACAddress(body.getTagetExternalAddr().getInt()^4));
            body.setLifeTime(body.getLifeTime()^4);
            body.setMetric(body.getMetric()^4);
            body.setOriginator(MACAddress(body.getOriginator().getInt()^4));
            body.setOriginatorSeqNumber(body.getOriginatorSeqNumber()^4);

            EV << "nachher: "<<endl;
            EV <<  body.getBodyLength()<<endl;
            EV <<  body.getId()<<endl;
            EV << body.getTarget()<<endl;
            EV << body.getTargetSeqNumber()<<endl;
            EV << body.getTagetExternalAddr()<<endl;
            EV << body.getLifeTime()<<endl;
            EV <<  body.getOriginator()<<endl;
            EV << body.getOriginatorSeqNumber()<<endl;
            prep->setBody(body);
            return prep;
        }
    }


    return 0;
}

void SecurityWPA2::handleIeee80211MeshFrame(cMessage *msg)
{
    double delay=0;
    //Decryption
    if(strstr(msg->getName() ,"CCMPFrame")!=NULL)
    {
        EV << "msg from Mac >>> decrypt msg ...." <<endl;
        //  cPacket* payloadMsg=NULL;
        Ieee80211MeshFrame *frame = (check_and_cast<Ieee80211MeshFrame *>(msg));
        MeshInfo *mesh = lookupMesh(frame->getTransmitterAddress());
        EV<< frame->getTransmitterAddress()<<endl;
        if(mesh)
        {
            checkAuthState(mesh->address);
            CCMPFrame *ccmpFrame = dynamic_cast<CCMPFrame*> ( frame->decapsulate());
            IPv4Datagram *IP = dynamic_cast<IPv4Datagram *>(ccmpFrame->decapsulate());// payloadMsg= ccmpFrame->decapsulate();
            if(IP)
            {
                frame->setName("DecCCMP");
                //frame->encapsulate(payloadMsg);
                if(mesh->status==AUTHENTICATED){
                    frame->encapsulate(handleIPv4Datagram(IP, mesh));
                    //   else
                    //     error("frame->encapsulate(handleIPv4Datagram(IP, mesh))");
                    delay = (double)  dec + mic_verify;
                    sendDelayed(frame, delay,"mgmtOut");
                    //   send(frame,"mgmtOut");
                }
                else return;
            }
            else
                error("Not IPv4");

            delete ccmpFrame;

        }
        // else{
        //   error("mesh not found");
        //   }
    }

    //Encryption
    else
    {
        // New Frame = Old MAC Header | CCMP Header | ENC (Encapsulated Old MSG)
        EV << "encrypt msg ...."<<msg->getClassName() <<" " << msg->getName() <<endl;
        //  cPacket* payloadMsg=NULL;
        Ieee80211MeshFrame *frame = (check_and_cast<Ieee80211MeshFrame *>(msg));
        MeshInfo *mesh = lookupMesh(frame->getReceiverAddress());


        CCMPFrame *ccmpFrame = new CCMPFrame();

        // payloadMsg = frame->decapsulate();
        IPv4Datagram *IP = dynamic_cast<IPv4Datagram *>(frame->decapsulate());
        if(IP)
        {
            //        payloadMsg->encapsulate(IP);
            //   if (payloadMsg)
            if(mesh){
                if(mesh->status==AUTHENTICATED)
                    ccmpFrame->encapsulate(handleIPv4Datagram(IP, mesh));

            }
            // else
            //   error("Enc: frame2 ERROR");

            ccmpFrame->setCCMP_Header(1);
            ccmpFrame->setCCMP_Mic(1);

            if(checkAuthState(frame->getReceiverAddress())==0)
            {
                delete frame;
                delete ccmpFrame;
                return;
            }
            else
            {
                if (frame==NULL)
                {
                    frame = new Ieee80211MeshFrame(msg->getName());
                    frame->setTimestamp(msg->getCreationTime());
                    frame->setTTL(32);
                }

                if (msg->getControlInfo())
                    delete msg->removeControlInfo();
                frame->setReceiverAddress(mesh->address);
                frame->setTransmitterAddress(frame->getAddress3());

                if (frame->getReceiverAddress().isUnspecified())
                {
                    char name[50];
                    strcpy(name,msg->getName());
                    opp_error ("Ieee80211Mesh::encapsulate Bad Address");
                }
                if (frame->getReceiverAddress().isBroadcast())
                    frame->setTTL(1);
                frame->setName("CCMPFrame");
                frame->encapsulate(ccmpFrame);
            }
            delay = (double) enc + mic_add;
            sendDelayedMeshFrame(frame, mesh->address, delay);
            // sendMeshFrame(frame, mesh->address);
        }
        else
            error("NOT IPv4");
    }
}


void SecurityWPA2::handleIeee80211ActionHWMPFrame(cMessage *msg)
{
    //Decryption
    if(strstr(msg->getName() ,"CCMPFrame")!=NULL)
    {
        Ieee80211ActionHWMPFrame *hwmpFrame = dynamic_cast<Ieee80211ActionHWMPFrame *>(msg);
        MeshInfo *mesh = lookupMesh(hwmpFrame->getTransmitterAddress());
        EV <<hwmpFrame->getTransmitterAddress()<<endl;
        EV << "Encrypted Ieee80211ActionHWMPFrame from Mac >>> decrypt frame ...."<< endl;

        //  if(mesh)checkAuthState(mesh->address);;
        // if(mesh)
        // {
        //   if(mesh->status==AUTHENTICATED && mesh->KEK.buf.size()>1)
        // {
        Ieee80211ActionHWMPFrame *hwmpFrame2 = encryptActionHWMPFrame(hwmpFrame,mesh);
        hwmpFrame2->setName("DecCCMP");
        send(hwmpFrame2,"mgmtOut");
        // }
        //  }
        //   else
        //     EV << "Mesh node not found or not authenticated!"<< endl;
    }
    //Encryption
    else
    {
        Ieee80211ActionHWMPFrame *hwmpFrame = dynamic_cast<Ieee80211ActionHWMPFrame *>(msg);
        MeshInfo *mesh = lookupMesh(hwmpFrame->getReceiverAddress());
        EV <<hwmpFrame->getReceiverAddress()<<endl;
        EV << "Ieee80211ActionHWMPFrame from Mgmt >>> encrypt frame ...."<< endl;

        //    if(mesh)checkAuthState(mesh->address);
        //if(mesh)
        // {
        //  EV <<mesh->KEK.buf.size()<<endl;
        //   if(mesh->status==AUTHENTICATED && mesh->KEK.buf.size()>1)
        // {

        Ieee80211ActionHWMPFrame *hwmpFrame2 = encryptActionHWMPFrame(hwmpFrame,mesh);
        hwmpFrame2->setName("CCMPFrame");
        send(hwmpFrame2,"mgmtOut");
        // }
        //}
        // else
        //   EV << "Mesh node not found or not authenticated!"<< endl;
    }
}




void SecurityWPA2::handleIeee80211DataFrameWithSNAP(cMessage *msg)
{
    double delay=0;
    //Decryption
    if(strstr(msg->getName() ,"CCMPFrame")!=NULL)
    {
        EV << "msg from Mac >>> decrypt msg ...." <<endl;
        //  cPacket* payloadMsg=NULL;
        Ieee80211DataFrameWithSNAP *frame = (check_and_cast<Ieee80211DataFrameWithSNAP *>(msg));
        MeshInfo *mesh = lookupMesh(frame->getTransmitterAddress());
        //  EV << "transmitter address before lookup"<< frame->getTransmitterAddress()<<endl;
        frame->setName("DecCCMP");
        delay= (double) dec + mic_verify;
        if(mesh)
        {
            if(mesh->status==AUTHENTICATED)
                sendDelayedDataOrMgmtFrame(frame,frame->getReceiverAddress(),delay);
        }
        /*     if(mesh)
        {
            checkAuthState(mesh->address);
            CCMPFrame *ccmpFrame = dynamic_cast<CCMPFrame*> ( frame->decapsulate());
            IPv4Datagram *IP = dynamic_cast<IPv4Datagram *>(ccmpFrame->decapsulate());// payloadMsg= ccmpFrame->decapsulate();
            if(IP)
            {
                frame->setName("DecCCMP");
                // frame->encapsulate(payloadMsg);
                if(mesh->status==AUTHENTICATED){
                    frame->encapsulate(handleIPv4Datagram(IP, mesh));
                    //   else
                    //   error("frame->encapsulate(handleIPv4Datagram(IP, mesh))");

                    sendDataOrMgmtFrame(frame,frame->getReceiverAddress()); //? send(frame,"mgmtOut");
                }
                else{
                    delete frame;
                    delete msg;
                    return;
                }
            }
            else
                error("Not IPv4");

            delete ccmpFrame;

        }
        // else
        // error("mesh not found");*/

    }

    //Encryption
    else
    {
        // New Frame = Old MAC Header | CCMP Header | ENC (Encapsulated Old MSG)
        EV << "encrypt msg ...."<<msg->getClassName() <<" " << msg->getName() <<endl;
        //  cPacket* payloadMsg=NULL;
        Ieee80211DataFrameWithSNAP *frame = (check_and_cast<Ieee80211DataFrameWithSNAP *>(msg));
        // EV << "receiver address before lookup"<<frame->getReceiverAddress()<<endl;

        MeshInfo *mesh = lookupMesh(frame->getReceiverAddress());
        frame->setName("CCMPFrame");
        delay = (double) enc + mic_add;
        /*   IPv4Datagram *IP = check_and_cast<IPv4Datagram *>(frame->getEncapsulatedPacket);
       if(IP)
            EV << "IP "<<endl;*/
        if(mesh)
        {

            if(mesh->status==AUTHENTICATED)
            {
                frame->setByteLength(frame->getByteLength()+16);
                sendDelayedDataOrMgmtFrame(frame, mesh->address,delay);}
else
            {
                NrdeletedFrames ++;
            emit(deletedFramesSignal, NrdeletedFrames);

            }

        }


        /*    CCMPFrame *ccmpFrame = new CCMPFrame();

        // payloadMsg = frame->decapsulate();
        IPv4Datagram *IP = dynamic_cast<IPv4Datagram *>(frame->decapsulate());
       if(IP)
        {
            //        payloadMsg->encapsulate(IP);
            //   if (payloadMsg)
            if(mesh)
            {
                if(mesh->status==AUTHENTICATED)
                    ccmpFrame->encapsulate(handleIPv4Datagram(IP, mesh));
            }
            // else
            //   error("Enc: frame2 ERROR");

            ccmpFrame->setCCMP_Header(1);
            ccmpFrame->setCCMP_Mic(1);

            if(checkAuthState(frame->getReceiverAddress())==0)
            {
                delete frame;
                delete ccmpFrame;
                return;
            }
            else
            {
                if (frame==NULL)
                {
                    frame = new Ieee80211DataFrameWithSNAP(msg->getName());
                    frame->setTimestamp(msg->getCreationTime());

                }

                if (msg->getControlInfo())
                    delete msg->removeControlInfo();
                frame->setReceiverAddress(mesh->address);
                frame->setTransmitterAddress(frame->getAddress3());

                if (frame->getReceiverAddress().isUnspecified())
                {
                    char name[50];
                    strcpy(name,msg->getName());
                    opp_error ("Ieee80211Mesh::encapsulate Bad Address");
                }
                if (frame->getReceiverAddress().isBroadcast())

                frame->setName("CCMPFrame");
                frame->encapsulate(ccmpFrame);
            }
            sendDataOrMgmtFrame(frame, mesh->address);
        }
        else
            error("NOT IPv4");*/
    }
}






void SecurityWPA2::sendDeauthentication(const MACAddress& address)
{
    EV << "Send Deauthentication to Mesh " << address<< endl;

    Ieee80211AuthenticationFrame *resp = new Ieee80211AuthenticationFrame("Deauth");
    sendManagementFrame(resp, address);

}
void SecurityWPA2::handleDeauthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    EV << "Processing Deauthentication frame\n";

    MeshInfo *mesh = lookupMesh(frame->getTransmitterAddress());
    delete frame;
    if(mesh){
        sendDeauthentication(mesh->address);
        if(mesh->beaconTimeoutMsg!=NULL)
        {
            delete cancelEvent(mesh->beaconTimeoutMsg);
            //  mesh->beaconTimeoutMsg=NULL;
        }
        if(mesh->authTimeoutMsg_a!=NULL)
        {
            delete cancelEvent(mesh->authTimeoutMsg_a);
            mesh->authTimeoutMsg_a=NULL;
        }
        if(mesh->authTimeoutMsg_b!=NULL)
        {
            delete cancelEvent(mesh->authTimeoutMsg_b);
            mesh->authTimeoutMsg_b=NULL;
        }
        if(mesh->groupAuthTimeoutMsg!=NULL)
        {
            delete cancelEvent(mesh->groupAuthTimeoutMsg);
            mesh->groupAuthTimeoutMsg=NULL;
        }
        if(mesh->PTKTimerMsg!=NULL)
        {
            delete cancelEvent(mesh->PTKTimerMsg);
            mesh->PTKTimerMsg=NULL;
        }
        /*   for(MeshList::iterator it=meshList.begin(); it != meshList.end();)
            {
                if (it->address == address)
                    it = meshList.erase(it);
                else
                    ++it;
            }
         */
    }
}




void SecurityWPA2::handleAck()
{

    EV<<"handleAck" <<endl;
    for (MeshList::iterator it=meshList.begin(); it!=meshList.end(); ++it)
        if (it->status == WaitingForAck)
        {
            MeshInfo *mesh = lookupMesh(it->address);
            if(mesh)
            {
                if (mesh->authTimeoutMsg_b==NULL)
                {
                    EV << "No Authentication in progress, ignoring frame\n";
                    EV << mesh->authTimeoutMsg_b <<endl;
                    //error("HandleAuth. frameAuthSeq == 2");
                    clearMeshNode(mesh->address);
                    return;
                }

                mesh->status=AUTHENTICATED;

                EV << "Schedule next Key refresh \n";
                mesh->PTKTimerMsg = new newcMessage("PTKTimer");
                mesh->PTKTimerMsg->setKind(PTK_TIMEOUT);
                mesh->PTKTimerMsg->setMeshMACAddress_AuthTimeout(mesh->address);
                scheduleAt(simTime()+PTKTimeout, mesh->PTKTimerMsg);

                //start Authentication with other party
                ASSERT(mesh->authTimeoutMsg_b!=NULL);
                delete cancelEvent(mesh->authTimeoutMsg_b);
                mesh->authTimeoutMsg_b = NULL;

                if(mesh->isAuthenticated==3)
                {
                    mesh->isAuthenticated=4;
                    if(mesh->authTimeoutMsg_a!=NULL)
                    {
                        delete cancelEvent(mesh->authTimeoutMsg_a);
                        mesh->authTimeoutMsg_a = NULL;
                    }
                    if(mesh->isCandidate==1)
                        sendOpenAuthenticateRequest(mesh);
                }
                mesh->isCandidate=0;
                EV << "Install keys"<<endl;
                if(!mesh->PTK_final.buf.empty())
                    EV << "Final: " << mesh->PTK_final.buf.at(0) << ""<< mesh->PTK_final.buf.at(1) <<endl;

            }
        }
}


uint64_t SecurityWPA2::stringToUint64_t(std::string s) {
    using namespace std;
    char * a=new char[s.size()+1];
    a[s.size()]=0;
    memcpy(a,s.c_str(),s.size());
    //std::copy(s.c_str(),s.c_str()+s.size(),a);
    uint64_t zahl = 0;
    for(int len = strlen(a), i = len - 1; i != -1; --i){
        zahl = zahl * 10 + s[i] - '0';}
    return zahl;
}
uint32_t SecurityWPA2::stringToUint32_t(std::string s) {
    using namespace std;
    char * a=new char[s.size()+1];
    a[s.size()]=0;
    memcpy(a,s.c_str(),s.size());
    //std::copy(s.c_str(),s.c_str()+s.size(),a);
    uint32_t zahl = 0;
    for(int len = strlen(a), i = len - 1; i != -1; --i){
        zahl = zahl * 10 + s[i] - '0';}
    return zahl;
}


SecurityWPA2::key256 SecurityWPA2::computePMK(std::string s, std::string s1) {

    //PMK(256) = (PSK XOR SSID)
    uint64_t a = stringToUint64_t(PSK);
    uint64_t b = stringToUint64_t(ssid);
    uint64_t c = a ^ b;

    key256 vec;
    vec.len=256;
    vec.buf.push_back(c);
    //PMK ist zu kurz. Rest einfach mit gleichen Inhalt f�llen
    vec.buf.push_back(c+1);
    vec.buf.push_back(c+2);
    vec.buf.push_back(c+3);
    return vec;
}

SecurityWPA2::key384 SecurityWPA2::computePTK(SecurityWPA2::key256 PMK, SecurityWPA2::nonce NonceA, SecurityWPA2::nonce NonceB, const MACAddress & addressA, const MACAddress & addressB) {
    key384 PTK;

    // PTK(384) = PMK Xor (Z1 xor Z2 xor MACA xor MACB)
    PTK.len=384;
    uint64_t a  = stringToUint64_t(addressA.str());
    uint64_t b = stringToUint64_t(addressB.str());
    for (int i = 0; i < 4; i++)
        PTK.buf.push_back(PMK.buf.at(i) ^ NonceA.buf.at(i) ^ NonceB.buf.at(i) ^ a ^ b);

    for (int i = 4; i < 6; i++)
        PTK.buf.push_back(PMK.buf.at(i-4) ^ NonceA.buf.at(i-3) ^ NonceB.buf.at(i-2) ^ a ^ b);

    return PTK;
}



SecurityWPA2::nonce SecurityWPA2::generateNonce() {

    nonce Nonce;
    Nonce.len=256;
    Nonce.buf.push_back( genk_intrand(1,1073741823));
    Nonce.buf.push_back( genk_intrand(1,1073741823));
    Nonce.buf.push_back( genk_intrand(1,1073741823));
    Nonce.buf.push_back( genk_intrand(1,1073741823));

    Nonce.len=256;
    return Nonce;
}



SecurityWPA2::key128 SecurityWPA2::encrypt128(SecurityWPA2::key128 a, SecurityWPA2::key128 b){
    key128 c;
    if(a.buf.size()<2)
        error("encrypt128:a is empty: '%d'", a.buf.size());
    if( b.buf.size()<2)
        error("encrypt128:b is empty: '%d'", b.buf.size());
    for(int i=0;i<2;i++){
        c.buf.push_back(a.buf.at(i)^b.buf.at(i));
        EV<<a.buf.at(i); EV<< " XOR "; EV<< b.buf.at(i);EV<< " = "; EV<< c.buf.at(i)<<endl;
    }
    c.len=128;
    return c;

}
SecurityWPA2::key128 SecurityWPA2::decrypt128(SecurityWPA2::key128 a, SecurityWPA2::key128 b){
    key128 c;
    for(int i=0; i<2;i++){
        c.buf.push_back(a.buf.at(i)^b.buf.at(i));
        EV<< c.buf.at(i)<<endl;
    }
    return c;

}

SecurityWPA2::key128 SecurityWPA2::computeMic128(key128 KCK, cMessage *msg){
    uint64_t mic=0;
    key128 mic_;
    SecurityPkt *pkt = dynamic_cast<SecurityPkt*> (msg);

    for(int i=0;i<12;i++){
        if(pkt->getDescriptor_Type(i)-'0' == 1)
        {
            switch(i)
            {
                case 0: mic^=pkt->getDescriptor_Type(0);  break;
                case 1: mic^=pkt->getKey_Info();  break;
                case 2: mic^=pkt->getKey_Length();  break;
                case 3: mic^=pkt->getKey_RC();  break;
                case 4: for(int i=0; i<(pkt->getKey_Nonce().len/64);i++){mic^=pkt->getKey_Nonce().buf.at(i);}      break;//32 octets
                case 5: for(int i=0; i<(pkt->getEAPOL_KeyIV().len/64);i++) mic^=pkt->getEAPOL_KeyIV().buf.at(i);  break;//16 octets
                case 6: mic^=pkt->getKey_RSC();  break;
                case 7: mic^=pkt->getReserved();  break;
                case 8:   break;//MIC!
                case 9: mic^=pkt->getKey_Data_Length();  break;
                case 10: for(int i=0; i<(pkt->getKey_Data128().len/64);i++) mic^=pkt->getKey_Data128().buf.at(i);  break;//16 octets
                case 11: for(int i=0; i<(pkt->getKey_Data256().len/64);i++) mic^=pkt->getKey_Data256().buf.at(i);  break;//32 octets
                default: mic^=0; break;
            }
        }
    }

    mic_.buf.push_back(mic^KCK.buf.at(0));
    mic_.buf.push_back(mic^KCK.buf.at(1));
    mic_.len=128;
    return mic_;
}

void SecurityWPA2::clearKey128(key128 key){
    if(!key.buf.empty())
    {
        key.buf.clear();
    }
}
void SecurityWPA2::clearKey256(key256 key){
    if(!key.buf.empty())
    {
        key.buf.clear();
    }
}
void SecurityWPA2::clearKey384(key384 key){
    if(!key.buf.empty())
    {
        key.buf.clear();
    }
}
void SecurityWPA2::clearNonce(nonce key){
    if(!key.buf.empty())
    {
        key.buf.clear();
    }
}


void SecurityWPA2::derivePTKKeys(MeshInfo *mesh, key384 key)
{
    if(!mesh->KCK.buf.empty() && !mesh->KEK.buf.empty() && !mesh->TK.buf.empty() ){
        mesh->KCK.buf.clear();
        mesh->KEK.buf.clear();
        mesh->TK.buf.clear();
    }
    // PTK = KEC | KEK | TK
    for(int i=0;i<2;i++){
        mesh->KCK.buf.push_back( mesh->PTK.buf.at(i) );
        mesh->KEK.buf.push_back( mesh->PTK.buf.at(i+2) );
        mesh->TK.buf.push_back( mesh->PTK.buf.at(i+4) );
    }
    mesh->KCK.len=128;
    mesh->KEK.len=128;
    mesh->TK.len=128;
}


/*char* SecurityWPA2::encapsulate(cPacket *msg, unsigned int* length)
{


    unsigned int payloadlen;
    static unsigned int iplen = 20; // we don't generate IP options
    static unsigned int udplen = 8;
    cPacket* payloadMsg = NULL;
    char* buf = NULL, *payload = NULL;
    uint32_t saddr, daddr;
    volatile iphdr* ip_buf;
    struct newiphdr {
               short version_var;
               short headerLength_var;
               IPv4Address srcAddress_var;
               IPv4Address destAddress_var;
               int transportProtocol_var;
               short timeToLive_var;
               int identification_var;
               bool moreFragments_var;
               bool dontFragment_var;
               int fragmentOffset_var;
               unsigned char typeOfService_var;
               int optionCode_var;
               IPv4RecordRouteOption recordRoute_var;
               IPv4TimestampOption timestampOption_var;
               IPv4SourceRoutingOption sourceRoutingOption_var;
               unsigned int totalPayloadLength_var;
       }*iphdr;





    IPv4Datagram* IP = check_and_cast<IPv4Datagram*>(msg);

    // FIXME: Cast ICMP-Messages
    UDPPacket* UDP = dynamic_cast<UDPPacket*>(IP->decapsulate());
    if (!UDP) {
        EV << "    Can't parse non-UDP packets (e.g. ICMP) (yet)" << endl;
    }
    payloadMsg = UDP->decapsulate();


 *length = payloadlen + iplen + udplen;
    buf = new char[*length];

    // We use the buffer to build an ip packet.
    // To minimise unnecessary copying, we start with the payload
    // and write it to the end of the buffer
    memcpy( (buf + iplen + udplen), payload, payloadlen);

    // write ip header to begin of buffer
    ip_buf = (iphdr*) buf;
    ip_buf->version = 4; // IPv4
    ip_buf->ihl = iplen / 4;
    ip_buf->saddr = IP->getSrcAddress();
    ip_buf->daddr = IP->getDestAddress();
    ip_buf->protocol = IPPROTO_UDP;
    ip_buf->ttl = IP->getTimeToLive();
    ip_buf->tot_len = htons(*length);
    ip_buf->id = htons(IP->getIdentification());
    ip_buf->frag_off = htons(IP_DF); // DF, no fragments
    ip_buf->check = 0;
//    ip_buf->check = cksum((uint16_t*) ip_buf, iplen);

    delete IP;
    delete UDP;
    delete payloadMsg;
    delete payload;

    return buf;


}*/

void SecurityWPA2::receiveChangeNotification(int category, const cObject *details)
{
    EV << "receiveChangeNotification()" <<endl;
    Enter_Method_Silent();

    if (category == NF_TX_ACKED)
    {
        handleAck();
    }
}

void SecurityWPA2::finish()
 {
     if (!statsAlreadyRecorded) {
        recordScalar("Total AuthTimeout Me", totalAuthTimeout);
        recordScalar("Total BeaconTimeout Me", totalBeaconTimeout);
        statsAlreadyRecorded = true;
     }
 }
