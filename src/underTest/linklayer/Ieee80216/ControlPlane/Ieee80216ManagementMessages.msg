//
//
// Copyright (C) 2007 Roland Siedlaczek
// Copyright (C) 2007 Universität Dortmund, Germany
//
// MAC-Paket Data Unit Header
// Defenition des Managementnachrichten
//
//


cplusplus {{
#include "MACAddress.h"
#include "Ieee80216MacHeader_m.h"
#include "global_enums.h"
#include <vector>
#include "Struct.h"//Type/Length/Value TLV coding
}}

class noncobject MACAddress;

message Ieee80216GenericMacHeader;

//Management-Nachrichten definition gemaess Standard
enum Ieee80216ManagementType
{
    ST_UCD = 0x00;
    ST_DCD = 0x01;
    ST_DL_MAP = 0x02;
    ST_UL_MAP = 0x03;
    ST_RNG_REQ = 0x04;
    ST_RNG_RSP = 0x05;
    ST_REG_REQ = 0x06;
    ST_REG_RSP = 0x07;

    ST_MOB_NBR_ADV = 0x53;
    ST_MOB_SCN_REQ = 0x54;
    ST_MOB_SCN_RSP = 0x55;
    ST_MSHO_REQ = 0x57;
    ST_BSHO_RSP = 0x58;
    ST_MOB_HO_IND = 0x59;

    ST_DSA_REQ = 0x0B;
    ST_DSA_RSP = 0x0C;
    ST_DSA_ACK = 0x0D;
    ST_DSC_REQ = 0x0E;
    ST_DSC_RSP = 0x0F;
    ST_DSC_ACK = 0x10;
    ST_DSD_REQ = 0x11;
    ST_DSD_RSP = 0x12;
    ST_DSX_RVD = 0x1E;

    //new management messages for provisioned service flows
    ST_PSF_REQ = 0x40;
    ST_PSF_RSP = 0x41;
 //---

    ST_SBC_REQ = 0x1A;
    ST_SBC_RSP = 0x1B;
}

struct TLV_LEN8;

struct TLV_LEN9;

struct TLV_LEN13;

struct TLV_LEN16;

struct TLV_LEN24;

struct TLV_LEN32;

struct TLV_LEN40;

struct TLV_LEN48;

struct ServiceFlow;

enum ip_traffic_types;

packet Ieee80216_Preambel
{
    byteLength = 48;
}

//
// DL_MAP frame body format.
//
packet Ieee80216ManagementFrame extends Ieee80216GenericMacHeader
{
    short Management_Message_Type enum(Ieee80216ManagementType);
}

//
// DL-MAP-IE Frmat aus der Tabele 275 im 802.16e Standard für OFDMA
//
packet Ieee80216DL_MAP_IE
{
    int bitlength = 62;

    short DIUC;                                 //  4 bits
    short N_CID;                                //  8 bits table 276
    int CID;                                    // 16 bits
    short OFDMA_Symbol_offset;                  //  8 bits
    short Subchannel_offset;                    //  8 bits
    short Boosting;                             //  3 bits
    short No_OFDMA_Symbols;                     //  7 bits
    short No_Subchannels;                       //  6 bits
    short Repetition_Coding_Indication;         //  2 bits

    double StartTime_Offset;
}

//DL-MAP Format aus der Tabele 16 im 802.16e Standard
packet Ieee80216DL_MAP extends Ieee80216ManagementFrame
{
    CID = 0xFFFF;//Broadcast Frame

    //table 16
    Management_Message_Type = ST_DL_MAP;        // 8 Bit
    short DCD_Count;                            // 8 bit
    MACAddress BS_ID;                           //48 bit
    int No_OFDMA_Symbols;                       //8 Bit
    Ieee80216DL_MAP_IE dlmap_ie_List[]; // array for DL-MapIE

    //table 273
    int Frame_duration_code;                    // 8 Bit
    double Frame_number;                        // 24 Bit
}

//UL-MAP-IE Format aus der Tabele 287 im 802.16e Standard für OFDMA
packet Ieee80216UL_MAP_IE
{
    int bitlength = 32;

    //table 287
    int CID; // 16 bits
    int uiuc; // 4Bits table 288

    //if uiuc = 12
    //int symbol_Offset;                // 8Bits
    int OFDMA_Symbol_offset;            //8 Bit
    int Subchannel_offset;            //7 Bit
    int No_OFDMA_Symbols;             //7 Bit
    int No_Subchannels;                // 7 Bit
    int Ranging_Method;                 // 2 Bit
    double Duration;                    //    10 Bit see 8.4.3.1 for simulation: in milliseconds  (originally in #OFDM-Symbols)
    int Repetition_Coding_Indication;//2 Bit

    double StartTime_Offset;//symbol_Offset;//8bit
    double StopTime_Offset;//time_offset;

    int bits_for_burst;    // non-standard information transfered to the MobileStation
    int realCID; // non-standard information to help the transceiver select the right packets for the uplink burst 
}

//UL-MAP Format aus der Tabele 18 im 802.16e Standard
packet Ieee80216UL_MAP extends Ieee80216ManagementFrame
{
    CID = 0xFFFF;//Broadcast Frame
    int bitlength_without_ielist = 72;

    //Informationen der UL-MAP Nachricht
    //table 18
    Management_Message_Type = ST_UL_MAP;    // 8 bits
    int Reserved;                        // 8 Bit
    int uplinkChannelId;                // 8 bits
    int UCD_Count;                        // 8 bit
    double allocation_start_time;        // 32bit Startpunkt an dem der uplink Unterrahmen begint
    int No_OFDMA_Symbols;                // 8 Bit
    Ieee80216UL_MAP_IE ulmap_ie_List[]; //
}


packet Ieee80216_DCD extends Ieee80216ManagementFrame
{
    //MAC Header Einstellungen
    CID = 0xFFFF;//Broadcast Frame

    //table 15
    Management_Message_Type = ST_DCD;
    int Reserved;
    int Config_Change_Count;

    //table 358
    TLV_LEN16 BS_EIRP;
    TLV_LEN8 Channel_Nr; //8.5.
    TLV_LEN8 TTG;    //only for TDD
    TLV_LEN8 RTG;    //only for TDD
    TLV_LEN16 RSS;
    TLV_LEN24 Channel_Switch_Frame_Number;
    TLV_LEN32 Frequency;
    //TLV_LEN48 BS_ID;
    TLV_LEN8 Size_of_CQICH_ID_field;
    TLV_LEN8 HARQ_ACK_delay_for_DL_burst;
    TLV_LEN8 MAC_version; //11.1.3

    //int BS_EIRP;
    //int Channel_Nr;
    //int Channel_Switch_Frame_Nummber;
    //int Frequency;
    MACAddress BS_ID;
    //int HARQ_ACK_delay_for_DL_burst;
    //int MAC_version;
}

packet Ieee80216_UCD extends Ieee80216ManagementFrame
{
    CID = 0xFFFF;//Broadcast Frame

    //table 17
    Management_Message_Type = ST_UCD;
    int Config_Change_Count;                    //8
    int Rng_Backoff_Start;                        //8
    int Rng_Backoff_End;                         //8
    int Req_Backoff_Start;                        //8
    int Req_Backoff_End;                        //8
    MACAddress BS_ID;
    int UploadChannel;
    //table 349
    TLV_LEN8 Contention_based_reservation_timeout;
    int Bandwidth_request_opportunity_size;//16Bit
    int Ranging_request_opportunity_size;//16Bit
    TLV_LEN32 Frequency;
    //table 353
    TLV_LEN8 Initial_ranging_codes;
    TLV_LEN8 Periodic_ranging_codes;
    TLV_LEN8 Bandwidth_request_codes;
    TLV_LEN8 Periodic_ranging_backoff_start;
    TLV_LEN8 Periodic_ranging_backoff_end;
    TLV_LEN8 Start_of_ranging_codes_group;
    TLV_LEN8 Permutation_base;
    TLV_LEN9 UL_allocated_subchannels_bitmap;
    TLV_LEN13 Optional_permutation_UL_Allocated_subchannels_bitmap;
    TLV_LEN8 Band_AMC_Allocation_Threshold;
    TLV_LEN8 Band_AMC_Release_Threshold;
    TLV_LEN8 Band_AMC_Allocation_Timer;
    TLV_LEN8 Band_AMC_Release_Timer;
    TLV_LEN8 Band_Status_Reporting_MAX_Period;
    TLV_LEN8 Band_AMC_Retry_Timer;
    TLV_LEN8 Safety_Channel_Allocation_Threshold;
    TLV_LEN8 Safety_Channel_Release_Threshold;
    TLV_LEN8 Safety_Channel_Allocation_Timer;
    TLV_LEN8 Safety_Channel_Release_Timer;
    TLV_LEN8 Bin_Status_Reporting_MAX_Period;
    TLV_LEN8 Safety_Channel_Retry_Timer;
    TLV_LEN8 HARQ_ACK_delay_for_UL_burst;
    TLV_LEN8 CQICH_Band_AMCTransition_Delay;
}

//
//    Tabele 19 Ranging request message
//
packet Ieee80216_RNG_REQ extends Ieee80216ManagementFrame
{
    CID = 0x0000;// Initial Ranging oder Basic CID
    //table 19
    Management_Message_Type = ST_RNG_REQ;        // 8 Bit
    int Reserved;                            // 8 Bit

    //table 364
    int Requested_Downlink_Burst_Profile;    // 8 Bit
    MACAddress MSS_MAC_Address;                // 48 Bit
    int Ranging_Anomalies;                     // 8 Bit
    int AAS_broadcast_capability;            // 8 Bit
}

packet Ieee80216_RNG_RSP extends Ieee80216ManagementFrame
{
    CID = 0x0000;// Initial Ranging oder Basic CID
    //table 20
    Management_Message_Type = ST_RNG_RSP;        // 8 Bit
    int Reserved;                            // 8 Bit

    //table 367
    double Timing_Adjust;                        // 32 Bit
    int Power_Level_Adjust;                    // 8 Bit
    double Offset_Frequency_Adjust;                // 32 Bit
    int Ranging_Status;                        // 8 Bit
    double Downlink_frequency_override;            // 32 Bit
    int Uplink_channel_ID_override;            // 8 Bit 
    double Downlink_Operational_Burst_Profile;    // 16 Bit
    MACAddress MSS_MAC_Address;                    // 48 Bit 
    int Basic_CID;                                 // 16 Bit 
    int Primary_Management_CID;                    // 16 Bit
    int AAS_broadcast_permission;                // 8 Bit
}

packet Ieee80216_SBC_REQ extends Ieee80216ManagementFrame
{
    CID = 0xFFFF;
    Management_Message_Type = ST_SBC_REQ;
}

packet Ieee80216_SBC_RSP extends Ieee80216ManagementFrame
{
    CID = 0xFFFF;
    Management_Message_Type = ST_SBC_RSP;
}

packet Ieee80216_REG_REQ extends Ieee80216ManagementFrame
{
    CID = 0xFFFF;
    Management_Message_Type = ST_REG_REQ;
    MACAddress MSS_MAC_Address;
}

packet Ieee80216_REG_RSP extends Ieee80216ManagementFrame
{
    CID = 0xFFFF;
    Management_Message_Type = ST_REG_RSP;

    int Secondary_Management_CID;
}

packet Ieee80216_MOB_NBR_ADV extends Ieee80216ManagementFrame
{
    Management_Message_Type = ST_MOB_NBR_ADV;
}

packet Ieee80216_MSHO_REQ extends Ieee80216ManagementFrame
{
    Management_Message_Type = ST_MSHO_REQ;
    int reportMetric;//8Bit
    int nNewBSindex;//8Bits
    int cahngeCounterForMOB_NBR_ADV;//8Bits
    int neighborBSindex;//8Bits
}

packet Ieee80216_BSHO_RSP extends Ieee80216ManagementFrame
{
    Management_Message_Type = ST_BSHO_RSP;
    int mode;//3Bits and 5Bits reserve
}

packet Ieee80216_MOB_HO_IND extends Ieee80216ManagementFrame
{
    Management_Message_Type = ST_MOB_HO_IND;
    int mode;//3Bits and 5Bits reserve
}

packet Ieee80216_MOB_SCN_REQ extends Ieee80216ManagementFrame
{
    Management_Message_Type = ST_MOB_SCN_REQ;
    int scanDuration;//8Bits
    int interleavingInterval;//8Bits
    int scanIteration;//8Bits
    int changeCountMOBNBR;//8Bits
}

packet Ieee80216_MOB_SCN_RSP extends Ieee80216ManagementFrame
{
    Management_Message_Type = ST_MOB_SCN_RSP;
    int strartFrame; //4Bits
    int scanDuration;//8Bits
    int interleavingInterval;//8Bits
    int repordeMode;//8Bits
    int repordPeriiod;//8Bits
    int repordMetric;//8Bits
    int scanIteration;//8Bits
}

packet Ieee80216ServiceFlowFrame extends Ieee80216ManagementFrame
{
    CID = 0xFFFF;
}

packet Ieee80216_DSA_REQ extends Ieee80216ServiceFlowFrame
{
    Management_Message_Type = ST_DSA_REQ;

    short traffic_type enum(ip_traffic_types);
    ServiceFlow newServiceFlow;
}

packet Ieee80216_DSA_RSP extends Ieee80216ServiceFlowFrame
{
    MACAddress mac_address;    // not in standard, helper field

    Management_Message_Type = ST_DSA_RSP;

    bool rejected = false;
    short traffic_type enum(ip_traffic_types);
    ServiceFlow newServiceFlow;
}

packet Ieee80216_DSA_ACK extends Ieee80216ServiceFlowFrame
{
    MACAddress mac_address;    // not in standard, helper field

    Management_Message_Type = ST_DSA_ACK;
    //short traffic_type enum(ip_traffic_types);
    ServiceFlow newServiceFlow;
}

packet Ieee80216_DSC_REQ extends Ieee80216ServiceFlowFrame
{
    Management_Message_Type = ST_DSC_REQ;
}

packet Ieee80216_DSC_RSP extends Ieee80216ServiceFlowFrame
{
    Management_Message_Type = ST_DSC_RSP;
}

packet Ieee80216_DSC_ACK extends Ieee80216ServiceFlowFrame
{
    Management_Message_Type = ST_DSC_ACK;
}

packet Ieee80216_DSD_REQ extends Ieee80216ServiceFlowFrame
{
    Management_Message_Type = ST_DSD_REQ;
}

packet Ieee80216_DSD_RSP extends Ieee80216ServiceFlowFrame
{
    Management_Message_Type = ST_DSD_RSP;
}

packet Ieee80216_DSX_RVD extends Ieee80216ServiceFlowFrame
{
    Management_Message_Type = ST_DSX_RVD;

    string request_id;
}
