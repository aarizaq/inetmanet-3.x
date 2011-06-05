
#ifndef GLOBAL_ENUMS_H
#define GLOBAL_ENUMS_H

#include "MACAddress.h"
#include "SnrList.h"

enum traffic_types {
    BASIC_CID,
    PRIM_MAN_CID
};

enum ip_traffic_types {
    MANAGEMENT = 0,
    UGS = 1,
    RTPS = 2,
    ERTPS = 3,
    NRTPS = 4,
    BE = 5
};

enum priorities {
    pUGS,
    pRTPS,
    pERTPS,
    pNRTPS,
    pBE
};

enum sf_state {
    SF_MANAGEMENT = -1,
    SF_INIT = 0,
    SF_PROVISIONED = 1,
    SF_AUTHORIZED = 2,
    SF_ADMITTED = 3,
    SF_ACTIVE = 4
};

enum management_type {
    BASIC,
    PRIMARY,
    SECONDARY
};

enum station_type {
    MOBILESTATION,
    BASESTATION
};

enum queue_id {
    qREQUEST,
    qCONTROL,
    qBASIC,
    qPRIMARY,
    qDATA
};

/**
enum sf_paramset_state {
    INIT = 0,
    PROVISIONED = 1,
    ADMITTED = 2,
    ACTIVE = 3
};
*/

struct req_tx_policy
{
    bool no_piggyback;
    bool no_fragmentation;
    bool no_payload_header_suppression;
    bool no_packing;

    req_tx_policy()
    {
        no_piggyback = false;
        no_fragmentation = false;
        no_payload_header_suppression = false;
        no_packing = false;
    }
};

struct sf_QoSParamSet
{
// sf_paramset_state state;

// int SFID; // SFID:CID always 1:1
// int CID;
// int active_CID;
// gucken, ob die 3 hier gebraucht werden, ich glaube nicht...

    short traffic_priority;     // 0-7: low to high, default 0
    int max_sustained_traffic_rate; // bit/s - 0 = no explicitly mandatory bandwidth
    int min_reserved_traffic_rate; // bit/s - 0 = no bandwidth reserved for the flow
    req_tx_policy request_transmission_policy;
    int tolerated_jitter;
    int max_latency;            // latency between arrival in CS and arrival at peer device (Table 124c in standard)

    double packetInterval;      // non-standard information to calculate the number of packets per ul-grant inside the basestation
    int granted_traffic_rate;   // non-standard helper field

    sf_QoSParamSet()
    {
        traffic_priority = 0;
        max_sustained_traffic_rate = 0;
        min_reserved_traffic_rate = 0;
        granted_traffic_rate = 0;
    }
};

enum link_direction
{
    ldMANAGEMENT = -1,
    ldUPLINK = 0,
    ldDOWNLINK = 1
};

struct ServiceFlow
{
    int SFID;                   // can be 32bit
    int CID;                    // can be 16bit
    sf_state state;             // initialize with unspecified state

    sf_QoSParamSet *provisioned_parameters;
    sf_QoSParamSet *admitted_parameters;
    sf_QoSParamSet *active_parameters;

    ip_traffic_types traffic_type; // (mk) gedacht als hilfe bei der classification, evt. überflüssig...
    link_direction link_type;

    ServiceFlow()
    {
        SFID = -1;
        CID = -1;
        state = SF_INIT;

        provisioned_parameters = NULL;
        admitted_parameters = NULL;
        active_parameters = NULL;
    }
};

// Holds the mapping of existing CIDs to SFIDs. (unique)
typedef std::map<int, int> ConnectionMap;

// Holds the actual ServiceFlow objects identified by their unique SFID
typedef std::map<int, ServiceFlow> ServiceFlowMap;

// Maps CIDs to their associated datarate.
// This map should be updated every time a new UL-MAP arrives..
typedef std::map<int, int> UplinkDatarateMap;

/**
* @name structMobileSubStationInfo
* in this object it will be save all parameter an information about a mobilstation
*
*/
struct structMobilestationInfo
{
    int channel;                //Vrwendeter Kanalnummer
    //MACAddress MSS_Address; // MAC Adresse der Mobile Subscriber Station
    MACAddress MobileMacAddress; //MacAdrese der Mobilesubscriber Station
    MACAddress activeBSid;      //MacAdrese der Basisstation
    MACAddress activeBasestation;
    MACAddress targetBasestation;

    //CID
    int Basic_CID;
    int Primary_Management_CID;
    int Secondary_Management_CID;

    /**
     * With this map, the station can be identified by any of its connections.
     * Uses map due to its logarithmic find() function instead of a list.
     * Set within ServiceFlow_BS.
     */
    ConnectionMap map_own_connections;

    cMessage *authTimeoutMsg;   // if non-NULL: authentication is in progress
    double time_offset;

    double recvSNR;
    double targetRecvSNR;
    double activeRecvSNR;
    double oldReceivSNR;

    double minMargin;
    double maxMargin;

    double ScanTimeInterval;
    double RngRspTimeOut_B;
    double RngRspTimeOut;
    double RngRspFail;
    double RegRspTimeOut;

    int DownlinkChannel;
    int UplinkChannel;

    int dlmapCounter;
    int dcdCounter;
    int ucdCounter;

    int multiplicator;

    bool connectedMS;
    bool scanModus;

    // variable need for scan
    int scanDuration;           //8Bits
    int interleavingInterval;   //8Bits
    int scanIteration;          //8Bits
    int strartFrame;            //4Bits
    double DLMAP_interval;      // Intervall der DL-MAP Nachricht, gleich Framelänge

    //double uplink_rate, downlink_rate;

    structMobilestationInfo()
    {
        Basic_CID = -1;         //-1 = no connection yet
        Primary_Management_CID = -1; //-1 = no connection yet
        Secondary_Management_CID = -1; //-1 = no connection yet

        channel = -1;           //-1 = no connection yet
        authTimeoutMsg = NULL;
        time_offset = 0.00001;

        recvSNR = -1;           //-1 = no connection yet

        multiplicator = 0;
        dlmapCounter = 0;
        dcdCounter = 0;
        ucdCounter = 0;
        scanModus = false;

        // variable need for scan
        scanDuration = 0;       //8Bits
        interleavingInterval = 0; //8Bits
        scanIteration = 0;      //8Bits
        strartFrame = 0;        //4Bits

        DownlinkChannel = 0;
        UplinkChannel = 0;

        oldReceivSNR = 0;

        //uplink_rate = 0;
        //downlink_rate = 0;
    }
};

/**
* @name structBasestationInfo
* in this object it will be save all parameter an information about a basestation
*
*/
struct structBasestationInfo
{
    int channel;                //Verwendeter Kanalnummer
    MACAddress BasestationID;   // MAC Adresse der Mobile Subscriber Station
    int Basic_CID;
    int Primary_Management_CID;
    int Secondary_Management_CID;

    int radioDatenrate;

    cMessage *authTimeoutMsg;   // if non-NULL: authentication is in progress
    double time_offset;
    int counter;
    double recieverPower;
    bool PollingStatus;
    SnrList planeSnrList;
    double minSNR;
    double receiverSNR;
    double rcvdPower;

    double DLMAP_interval;      // Intervall der DL-MAP Nachricht, gleich Framelänge
    double bitsproFrame;
    int DownlinkChannel;
    int UplinkChannel;

    int Bandwidth_request_opportunity_size;
    int Ranging_request_opportunity_size;

    int dcdCounter;
    int ucdCounter;

    int dlmap_ie_size;
    int ul_dl_ttg;

    bool enablePacking;

    double downlink_per_second; //VITA geaendert: war davor int // fixed downlink per second. if 0, downlink_to_uplink is used to calculate is in BS
    double downlink_to_uplink;

    bool useULGrantWaitingQueue;
    double grant_compensation;

    // variable need for scan
    int scanDuration;           //8Bits
    int interleavingInterval;   //8Bits
    int scanIteration;          //8Bits
    int strartFrame;            //4Bits

    structBasestationInfo()
    {
        Basic_CID = -1;         //-1 = no connection yet
        Primary_Management_CID = -1; //-1 = no connection yet
        Secondary_Management_CID = -1; //-1 = no connection yet

        channel = -1;           //-1 = no connection yet
        authTimeoutMsg = NULL;
        time_offset = 0.00001;
        counter = 0;
        recieverPower = -1;     //-1 = no connection yet
        Bandwidth_request_opportunity_size = 4;
        Ranging_request_opportunity_size = 4;

        downlink_per_second = 0;
    }
};

// Hold the Mobilestation who is connected with th Basestation
typedef std::list<structMobilestationInfo> MobileSubscriberStationList;

// Hold all Basestation who be find by the Mobilstation
typedef std::list<structBasestationInfo> BasestationList;

#endif
