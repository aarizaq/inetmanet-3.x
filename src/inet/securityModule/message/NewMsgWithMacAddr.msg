class noncobject MACAddress;
cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"

}}

namespace inet::ieee80211;
    
// For Timeouts
packet newcMessage extends cMessage
{
MACAddress MeshMACAddress_AuthTimeout;
MACAddress senderMACAddr;
}
