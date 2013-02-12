//
// Required for MACAddress declarations
//

cplusplus {{
#include "MACAddress.h"
#include "Ieee80216MacHeader_m.h"
#include "global_enums.h"
}}

class noncobject MACAddress;

enum ip_traffic_types;

enum sf_state;

struct sf_QoSParamSet;

struct structMobilestationInfo;

// ControlInfo for WiMAXQoSTrafficGenerator
message Ieee80216TGControlInformation
{
    int traffic_type enum(ip_traffic_types);
    int bitrate;
    double packetInterval;
}

// Command message sent by the TrafficClassificationModule to the ControlPlane
enum request_types
{
    DSA = 0;
    DSC = 1;
    DSD = 2;
}

message Ieee80216ClassificationCommand
{
    int request_type enum(request_types);
    int traffic_type enum(ip_traffic_types);
    sf_QoSParamSet requested_qos_params;
    int requested_sf_state enum(sf_state);
}

message Ieee80216FlushCommand
{
}

//
// IEEE 802.16 command codes, sent by the agent to the management entity.
// These constants should be set as message kind on the messages.

enum Ieee80216PrimRequestCode
{
    RC_DATA_REQUEST = 1;
}

//
// 
// 
//
// 
//
enum Ieee80216PrimResultCode
{
    PRC_SUCCESS = 0;
}

message Ieee80216_FCH_Preambel
{
}

//
// Base class for request primitives
//
message Ieee80216PrimRequest
{
}

//
// Base class for confirm primitives
//
message Ieee80216PrimConfirm
{
    int resultCode enum(Ieee80216PrimResultCode);
}
//
//
// Losely based on MLME-SCAN.request.
//
message Ieee80216Prim_ScanRequest extends Ieee80216PrimRequest
{
    MACAddress BSSID; // specific BSSID to scan for (default: any)
    int channelList[]; // list of channels to scan (default: all channels)
    double minChannelTime; // minimum time to spend on each channel when scanning
    double maxChannelTime; // maximum time to spend on each channel when scanning
}

message Ieee80216Prim_BSSDescription
{
    int channelNumber;
    MACAddress BSSID;
    double DLmapInterval;
    double rxPower;
}

//
message Ieee80216Prim_sendControlRequest extends Ieee80216PrimRequest
{
    //bool is_broadcast;
    int pduCID;         // CID for uplink grant (according to standard)
    double timeInterval;
    int bits_for_burst;
    int realCID;        // helper variable for packing mechanism (non-standard)
}

message Ieee80216Prim_stopControlRequest extends Ieee80216PrimRequest
{
    //bool is_broadcast;
    int pduCID = -1;
    double timeInterval;
}
//
//
//
message Ieee80216Prim_sendDataRequest extends Ieee80216PrimRequest
{
    int pduCID;
    double timeInterval;
}

message Ieee80216Prim_stopDataRequest extends Ieee80216PrimRequest
{
    int pduCID;
    double timeInterval;
}

//
// 
//
message Ieee80216Prim_BasicCID extends Ieee80216PrimRequest
{
    int BasicCID; // BasicCID
}

message Ieee80216Prim_ScanMS
{
    MACAddress scanMobilestationAddress;
    //structMobilestationInfo scanMobilestationInfo;
}
