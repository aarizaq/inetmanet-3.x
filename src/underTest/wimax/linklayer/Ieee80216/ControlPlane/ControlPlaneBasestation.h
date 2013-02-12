//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//
#ifndef Control_Plane_Basestation_H
#define Control_Plane_Basestation_H

#define FSM_DEBUG

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "MACAddress.h"

#include "FSMA.h"
#include "ControlPlaneBase.h"

#include "InterfaceTableAccess.h"
#include "InterfaceEntry.h"

#include <map>

using namespace std;

#include "CommonPartSublayerServiceFlows_BS.h"
#include "CommonPartSublayerScheduling.h"

#include "CommonPartSublayerReceiver.h"
#include "Ieee80216Primitives_m.h"
/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */

class ControlPlaneBasestation : public ControlPlaneBase
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

    //struct burstProfile
    //{
    //    double burstTime;
    //    int byteLength;
    //};

    //burstProfile burst;

    // APInfo list: we collect scanning results and keep track of ongoing authentications here
    // Note: there can be several ongoing authentications simultaneously
    // typedef std::list<MSSInfo> MobileSubscriberStationList;  // --> global_enums.h

    MobileSubscriberStationList localeMobilestationList; //List of all suported Mobilestation
    structBasestationInfo localBasestationInfo;

    // pointer to CPSServiceFlow module
    CommonPartSublayerServiceFlows_BS *cpsSF_BS;
    /**
    * @name Configuration parameters
    * Hier werden Parameter des Modules Basestation defeniert.
    */
  protected:
    /** @name Timer messages */

    cMessage *systemStartTimer;
    cMessage *broadcastTimer;
    cMessage *endTransmissionEvent;
    cMessage *preambleTimer;
    cMessage *startTransmissionEvent;
    cMessage *scheduleTimer;

    Ieee80216Prim_ScanMS *setMStoScanmodusTimer;

    //cPar *DlMapInterval;
    cParImpl *BWREQGrants;

   /**
    * Keeps track of the currently available datarate,
    * either fixed and configured through omnetpp.ini
    * or set with caluculated values from RPS.
    */
    //double available_datarate;   localBasestationInfo.radioDatenrate hat das schon...

  protected:
    /**
     * @name Ieee80216 Control Plane state variables
     *
     */
    //@{
    /** die 80216 ControlPlane state machine */
    enum State {
        DLMAP,
        ULMAP,
        DCD,
        UCD,
        STARTBURST,
        STOPBURST,
        PREAMBLE,
    };
    cFSM fsm;

  private:
    cOutVector cvec_grant_capacity[BE + 1];

    int scantimer;

    // keeps track of the next frame start and is set after each DL-MAP build
    double next_frame_start;

    typedef map<MACAddress, map<traffic_types, vector<ServiceFlow> > > StationServiceFlowMap;
    StationServiceFlowMap *stationSFMap;

    map<int, int> map_waitingGrants, map_servedGrants;
    list<ServiceFlow *> serviceFlowsSortedByPriority[6];

    CommonPartSublayerReceiver *cps_Receiver_BS; //Für die Übertragung der CID-MAP
    CommonPartSublayerScheduling *cps_scheduling; // pointer to the scheduling module
    // CommonPartSublayerAuthorizationModule *cps_auth;

    Ieee80216MacHeaderFrameList *tx_dataMsgQueue;

    // keep track of the ul-map size for offset calculations for dlmap_ies
    // in bits
    double cur_ulmap_size;

    // provides the offset for the uplink subframe
    // wird erstmal statisch auf das gleiche interval wie der downlink subframe (2ms) gesetzt...
    // kann dann später verschoben werden, um das verhältnis dl zu ul zu beeinflussen.
    double uplink_subframe_starttime, uplink_subframe_starttime_offset;

    // set during dlmap creation. contains the exact time, when the broadcast must be stopped
    // and is used in the FSM to schedule the stopControlQueueElement message
    double broadcast_stop_time;

    double downlink_to_uplink;

    /**
    * @brief mainfunction
    ***************************************************************************************/
  public:
    ControlPlaneBasestation();
    virtual ~ControlPlaneBasestation();

    virtual station_type getStationType();
    virtual management_type getManagementType(int CID);
    virtual void setDataMsgQueue(Ieee80216MacHeaderFrameList *data_queue);

    // only used in mobilestation, but needed due to castings from transceiver...
    virtual int getBasicCID();
    virtual int getPrimaryCID();

    void setTransceiverFlushBuffer(bool *flush_tx_buffer);

  protected:
    void finish();
    void initialize(int);
    //void startTransmitting(cMessage *msg);
    virtual void handleMessage(cMessage *msg);

    //Bearbeitet die Managementnachrichten die vom Netz kommen
    virtual void handleHigherLayerMsg(cMessage *msg);
    //Bearbeitet die Managementnachrichten die von den Mobilsationen kommen
    virtual void handleLowerLayerMsg(cMessage *msg);
    //Bearbeitet die Managementnachrichten die von den Mobilsationen kommen
    void handleManagementFrame(Ieee80216GenericMacHeader *GenericFrame);
    //Bearbeitet die Managementnachrichten die von den Mobilsationen kommen
    virtual void handleSelfMsg(cMessage *msg);
    //Timer
    // void handleTimer(cMessage *msg);
    //State machine
    void handleWithFSM(cMessage *msg);

    virtual void registerInterface();
    virtual void handleClassificationCommand(Ieee80216ClassificationCommand *msg);
    virtual void handleServiceFlowMessage(cMessage *msg);

    // virtual void handleScheduleTimer();

    /**
    * @brief helpfunction
    ***************************************************************************************/

    /**
    * @brief Funktionen zum erzeugen von Managementnachrichten
    */

    void buildPreamble();
    void buildDL_MAP();
    void buildUL_MAP();

    void buildRangingUL_MAP_IE();
    void buildBandwithRequestUL_MAP_IE();

    void buildUL_MAP_IE();
    void buildUL_MAP_IE(ServiceFlow *sf, double *accumulated_offset,
                        long *granted_data_in_next_ul_subframe, double compensation);

    void buildDCD();
    void buildUCD();

    void buildRNG_RSP(MACAddress recieveMobileStation);
    void buildRNG_RSP(structMobilestationInfo *new_ss);

    void buildSBC_RSP(int basic_cid);
    void buildREG_RSP(int primary_cid);

    void build_MOB_SCN_RSP(int cid);
    void build_MOB_BSHO_RSP(int CID);

    void buildScheduledIEs();
    void provideStationWithServiceFlows(int primary_cid);

    /**
    * @brief Funktionen zum verarbeiten von empfangenen Managementnachrichten
    */

    // management message for net entry
    void handle_RNG_REQ_Frame(Ieee80216_RNG_REQ *frame);
    void handle_SBC_REQ_Frame(Ieee80216_SBC_REQ *frame);
    void handle_REG_REQ_Frame(Ieee80216_REG_REQ *frame);
    // management message for handover
    void handle_MOB_SCN_REQ_Frame(Ieee80216_MOB_SCN_REQ *frame);
    void handle_MOB_MSHO_REQ_Frame(Ieee80216_MSHO_REQ *frame);
    void handle_MOB_HO_IND_Frame(Ieee80216_MOB_HO_IND *frame);

    virtual void handleBandwidthRequest(Ieee80216BandwidthMacHeader *bw_req);

    /**
    * @brief Funktionen für die Bearbeitung der localeMobilestationList
    */
    void clearMSSList();
    void storeMSSInfo(const MACAddress & Address);
    structMobilestationInfo *lookupLocaleMobilestationListMAC(const MACAddress & Address);
    structMobilestationInfo *lookupLocaleMobilestationListCID(int cid);

    /**
    * @brief Funktionen für die Radiomodule
    */
    void setRadio();
    void setRadioDownlink();
    void setRadioUplink();

    void startControlBurst();
    void stopControlBurst();
    double computePacketDuration(int bits);

    bool cidIsInConnectionMap(int cid);

    void setMobilestationScanmodus(cMessage *msg);

    void sortServiceFlowsByPriority();

    void updateDisplay();
};

#endif
