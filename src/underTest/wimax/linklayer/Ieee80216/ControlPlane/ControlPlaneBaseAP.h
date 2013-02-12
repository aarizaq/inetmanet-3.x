//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "MACAddress.h"
#include "Ieee80216ManagementMessages_m.h"
#include "Ieee80216Primitives_m.h"
#include "PhyControlInfo_m.h"

#include "ControlPlaneAP.h"
/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */

class ControlPlaneBaseAP : public cSimpleModule, public ControlPlaneAP
{
  public:
/**
* @name FlowMaping
*
*/
    struct FlowMap
    {
        MACAddress receiverAddress; // aka address1
        MACAddress transmitterAddress; // aka address2
        int ConnectID;
        int ServiceFlowID;
    };

    /**
    * @name MobileSubscribe
    * Speicher Informationen über
    * Speichert alle empfangenen MobileSubscriber
    */
    struct MSSInfo
    {
        int channel;            //Vrwendeter Kanalnummer
        MACAddress MSS_Address; // MAC Adresse der Mobile Subscriber Station
        cMessage *authTimeoutMsg; // if non-NULL: authentication is in progress

        MSSInfo()
        {
            channel = -1;
            authTimeoutMsg = NULL;
        }
    };

    // APInfo list: we collect scanning results and keep track of ongoing authentications here
    // Note: there can be several ongoing authentications simultaneously
    typedef std::list<MSSInfo>MobileSubscriberStationList;
    MobileSubscriberStationList mssList;

    /**
    * @name Configuration parameters
    * Hier werden Parameter des Modules Basestation defeniert.
    */
  protected:
    BaseStationInfo BSInfo;

  public:
    cGate *gateToWatch;

    cQueue queue;
    cMessage *endTransmissionEvent;
    cMessage *StartsetRadio;
    cMessage *BroadcastTimer;

  public:
    ControlPlaneBaseAP();
    virtual ~ControlPlaneBaseAP();

  protected:
    void startTransmitting(cMessage *msg);
    void initialize();

    void handleMessage(cMessage *msg);

    void handleTimer(cMessage *msg);
    void handleManagmentFrame(Ieee80216GenericMacHeader *GenericFrame);
    void handleUpperMessage(cMessage *msg);

    void sendLowerMessage(cMessage *msg);
    void sendLowerDelayMessage(double delayTime, cMessage *msg);
    void sendRadioUpOut(cMessage *msg);

  protected:
    void sendBroadcast();

    void handle_RNG_REQ_Frame(Ieee80216_RNG_REQ *frame);
    void handle_SBC_REQ_Frame(Ieee80216_SBC_REQ *frame);
    void handle_REG_REQ_Frame(Ieee80216_REG_REQ *frame);

    void clearMSSList();
    void storeMSSInfo(const MACAddress &Address);
    MSSInfo* lookupMSS(const MACAddress &Address);
};
