class noncobject MACAddress;
cplusplus {{
    #include "MACAddress.h"
#include "inet/common/INETDefs.h"
}}

namespace inet::ieee80211;
    
// For Timeouts
packet newcMessage extends cMessage
{
MACAddress MeshMACAddress_AuthTimeout;
MACAddress senderMACAddr;
}
