
cplusplus {{
#include "inet/common/INETDefs.h"
}}

import inet.linklayer.common.MACAddress;

namespace inet::ieee80211;
    
// For Timeouts
packet newcMessage
{
    MACAddress MeshMACAddress_AuthTimeout;
    MACAddress senderMACAddr;
}
