
#ifndef Control_Plane_Mobilestation_H
#define Control_Plane_Mobilestation_H

#define FSM_DEBUG

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "MACAddress.h"

#include "Ieee80216Primitives_m.h"

#include "FSMA.h"
#include "ControlPlaneBase.h"

#include "InterfaceTableAccess.h"
#include "InterfaceEntry.h"

#include "CommonPartSublayerServiceFlows_MS.h"
#include "CommonPartSublayerReceiver.h"
#include "CommonPartSublayerScheduling.h"
#include "ChannelControl.h"
//#include "SnrEval80216.h" //Fehlermeldlung mit BasicSnrEval

class ControlPlaneMobilestation : public ControlPlaneBase
{
  public:
    //MobileSubStationInfo
    structMobilestationInfo localMobilestationInfo; // MobileSubStation
    structMobilestationInfo activeMobilestationInfo; // MobileSubStation
    structMobilestationInfo targetMobilestationInfo; // MobileSubStation
    structMobilestationInfo scanningMobilestationInfo; // MobileSubStation
    /**
     * @name BasestationList
     * Die Liste enthälte Parameter von allen gescänten Basisstationen
     */

    BasestationList bsList;

    //Liste zum abspeichern der Signalstärke empfangener Pakete
    typedef std::list<double> SNRList;
    SNRList SNRQueue;

    typedef std::list<double> scanSNRList;
    scanSNRList scanSNRQueue;
    // information about the bandwidth available for the SS
    // in Byte/s
    //int granted_bandwidth;

    /**
     * @name Vector
     * Vectoren zum abspeichern von Messwerten
     */
    cOutVector startTimerVec;
    cOutVector stopTimerVec;
    cOutVector receiverSnrVector;
    cOutVector scanSnrVector;
    cOutVector gemitelterSnrVector;
    cOutVector gemitelterScanSnrVector;
    cOutVector abstandVector;
    cOutVector x_posVector;
    cOutVector y_posVector;

    cOutVector BS1SNR;
    //cOutVector BS1abstandVector;
    //cOutVector BS1x_posVector;
    //cOutVector BS1y_posVector;

    cOutVector BS2SNR;
    //cOutVector BS2abstandVector;
    //cOutVector BS2x_posVector;
    //cOutVector BS2y_posVector;

    cOutVector BS3SNR;
    //cOutVector BS3abstandVector;
    //cOutVector BS3x_posVector;
    //cOutVector BS3y_posVector;

    cOutVector BS4SNR;
    //cOutVector BS4abstandVector;
    //cOutVector BS4x_posVector;
    //cOutVector BS4y_posVector;

    cOutVector BS5SNR;
    //cOutVector BS5abstandVector;
    //cOutVector BS5x_posVector;
    //cOutVector BS5y_posVector;

    cOutVector BS6SNR;
    //cOutVector BS6abstandVector;
    //cOutVector BS6x_posVector;
    //cOutVector BS6y_posVector;

    cOutVector BS7SNR;
    //cOutVector BS7abstandVector;
    //cOutVector BS7x_posVector;
    //cOutVector BS7y_posVector;

    cOutVector HandoverCounterVec;

    cOutVector rcvdPowerVector;
    cOutVector therNoiseVector;
    cOutVector snrVector;
    cStdDev RangingTimeStats;
    cStdDev HandoverTime;
    cStdDev HandoverDone;
    cStdDev scanBStrue;
    cStdDev scanBSFrameCounter;

  private:
    // enum msManagementCIDState {
    //  MCID_ACTIVE,
    //  MCID_INACTIVE
    // };

    CommonPartSublayerServiceFlows_MS *cpsSF_MS;
    CommonPartSublayerScheduling *cps_scheduling; // pointer to the scheduling module

    CommonPartSublayerReceiver *cps_Receiver_MS; //Für die Übertragung der CID-MAP

    //UplinkDatarateMap map_currentDatarates; // current datarates per CID

    bool isPrimaryConnectionActive;

    int cur_floor, last_floor;
    double uplink_rate, downlink_rate;
    int grants_per_second;

    bool scan_request_sent;

  protected:
    /** @name Timer messages */

    cMessage *ScanChannelTimer;
    cMessage *DLMAPTimer;
    cMessage *RngRspTimeOut;
    cMessage *RegRspTimeOut;
    cMessage *RngRegFail;
    cMessage *ScnRegFail;
    cMessage *SBCRSPTimeOut;
    cMessage *setupInitialMACConnection;
    cMessage *sendPDU;
    cMessage *stopPDU;
    cMessage *sendControlPDU;
    cMessage *stopControlPDU;
    cMessage *sendSecondControlPDU;
    cMessage *stopSecondControlPDU;

    cMessage *startScanBasestation;
    cMessage *startRangingRequest;

    //scanmodus Timer
    cMessage *scanDurationTimer; // for x frame go to scan modus
    cMessage *interleavingIntervalTimer; //for x frame back to
    cMessage *scanIterationTimer; // wiederhole den scanmodus
    cMessage *startFrameTimer;  // after x frame start scan
    cMessage *startScanTimer;   //
    cMessage *startHandoverScan; //
    cMessage *startHandover;

    cMessage *MSHO_REQ_Fail;

    cMessage *msgPuffer;

  protected:
//Ranging Variable
    bool isRangingIntervall;
    int counterRangingUL_MAP_IE;
    int Initial_Ranging_Interval_Slot;
    int ULMap_Counter;
    int Use_ULMap;
    int rangingVersuche;
    int anzahlVersuche;

  protected:
//Scanning Variable
    int localScanIteration;
    bool ScanIterationBool;
    int scnChannel;
    bool makeHandoverBool;
    bool makeHandover_cdTrafficBool;
    int ScanCounter;
    int ScanDlMapCounter;
//Varibalen dür scalar record
    double startHoTime;
    double stopHoTime;
    double HO_minMarginTime;
    double HO_endScanningTime;
    double HO_makeHandover;
    double HO_NetEntryTime;
    double HO_StartSendDataTime;

    //double ScnRspTime;
    //double startScnTime;
    //double stopScnTime;
    //double IndRspTime;
    //double startEntryTime;
    //double HO_get_SCN_RSP_Time;
    //double HO_start_Scn_Time;
    //double HO_ScnTime;
    //double HO_HoTime;

//Handover Variavble
    int HandoverCounter;

    int numChannels;            // number of channels in ChannelControl -- used if we're told to scan "all" channels
    bool isAssociated;          // associated Access Point

    bool rahmenfolge;
    int FrameAnzahl;

    bool startHO_bool;

  private:
    ConnectionMap *getConnectionMap();
    int getBitsPerSecond(int duration);

    /**
    * @brief mainfunction
    ***************************************************************************************/
  public:
    ControlPlaneMobilestation();
    virtual ~ControlPlaneMobilestation();

    void addToDownlinkRate(double bits);
    void addToUplinkRate(double bits);

    /** die 80216 ControlPlane state machine */
    enum State {
        WAITDLMAP,
        WAITULMAP,
        WAITDCD,
        WAITUCD,
        STARTRANGING,
        WAITREGRSP,
        WAITRNGRSP,
        WAITSBCRSP,
        CONNECT,
        SCAN,
        HANDOVER,
    };

    cFSM fsm;

    State getFsmState();

  protected:
    void initialize(int);
    void finish();

    virtual void handleClassificationCommand(Ieee80216ClassificationCommand *command);
    virtual void handleServiceFlowMessage(cMessage *msg);

    virtual void handleHigherLayerMsg(cMessage *msg);
    //Bearbeitet die Managementnachrichten die von den Mobilsationen kommen
    virtual void handleLowerLayerMsg(cMessage *msg);

    void handleManagementFrame(Ieee80216GenericMacHeader *genericFrame);
    //Bearbeitet die Managementnachrichten die von den Mobilsationen kommen
    virtual void handleSelfMsg(cMessage *msg);
    //state machine
    void handleWithFSM(cMessage *msg);

    virtual void registerInterface();

    void createInitialServiceFlows();

    virtual void setDataMsgQueue(Ieee80216MacHeaderFrameList *data_queue);

    void resetStation();

    /**
    * @brief helpfunction
    ***************************************************************************************/

    /**
    * @brief Funktionen zum erzeugen von Managementnachrichten
    */

    void buildRNG_REQ();
    void buildSBC_REQ();
    void buildREG_REQ();

    void build_MOB_SCN_REQ();
    void build_MOB_MSHO_REQ();

    void buildDSA_RSP(Ieee80216_DSA_REQ *mframe);
    /**
    * @brief Funktionen zum verarbeiten von empfangenen Managementnachrichten
    */

    void handleDL_MAPFrame(cMessage *msg);
    void handleUL_MAPFrame(cMessage *msg);
    void handleDCDFrame(cMessage *msg);
    void handleUCDFrame(cMessage *msg);
    void handle_RNG_RSP_Frame(cMessage *msg);
    void handle_SBC_RSP_Frame(cMessage *msg);
    void handle_REG_RSP_Frame(cMessage *msg);
    void handle_MOB_SCN_RSP_Frame(cMessage *msg);
    void handle_MOB_BSHO_RSP_Frame(cMessage *msg);
    void build_MOB_HO_IND();

    void handleScan();

    void handleBS_DSA_REQ(Ieee80216_DSA_REQ *mframe);
    void handleDSA_ACK(Ieee80216_DSA_ACK *mframe);

    void handleScanDL_MAPFrame(cMessage *msg);
    void handleCorrectDLMAP(cMessage *msg);
    void getCalculateSNR(cMessage *msg);
    void makeHandover();
    bool isHoRspMsg(cMessage *msg);

    /**
    * @brief Funktionen
    */

    void UplinkChannelBS(MACAddress & BasestationID, const int UpChannel);
    //void storeBSInfo(const MACAddress& BasestationID);
    void storeBSInfo(MACAddress *BasestationID, double frameSNR);
    structBasestationInfo* lookupBS(MACAddress *BasestationID);
    void clearBSList();

    /** Prozeduren für scännen von Kanälen */
    void scanNextChannel();
    void changeDownlinkChannel(int channelNum);
    void changeUplinkChannel(int channelNum);

    bool isDlMapMsg(cMessage *msg);
    bool isUlMapMsg(cMessage *msg);
    bool isDCDMsg(cMessage *msg);
    bool isUCDMsg(cMessage *msg);
    bool isRngRspMsg(cMessage *msg);
    bool isSbcRspMsg(cMessage *msg);
    bool isRegRspMsg(cMessage *msg);
    bool isScnRspMsg(cMessage *msg);
    bool isStartRangingRequest(cMessage *msg);

    void getRadioConfig();

    void getRangingUL_MAP_IE(Ieee80216UL_MAP *frame);

    void rangingStart();
    int getTransmissionOpportunitySlot(int transmissionOpportunitySize, int versuche);
    void updateDisplay();

    bool isCorrectDLMAP(cMessage *msg);

    void startScanmodus();
    void stopScanmodus();

    void sendData();
    void stopData();
    void sendControl(Ieee80216Prim_sendControlRequest *control_info); //Send Management Control message
    void stopControl(Ieee80216Prim_stopControlRequest *control_info);

    void recordStartScanningMark();
    void recordStopScanningMark();

  public:
    //int findMatchingServiceFlow( ip_traffic_types traffic_type );
    virtual management_type getManagementType(int CID);
    virtual station_type getStationType();

    bool hasUplinkGrants();

    virtual int getBasicCID();
    virtual int getPrimaryCID();

    bool isHandoverDone();
    void setHandoverDone();
    void recordData(cMessage *msg);
};

#endif
