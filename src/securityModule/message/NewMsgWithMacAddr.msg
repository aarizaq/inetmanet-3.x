class noncobject MACAddress;
cplusplus {{
    #include "MACAddress.h"
#include "INETDefs.h"
}}

// For Timeouts
packet newcMessage extends cMessage
{
MACAddress MeshMACAddress_AuthTimeout;
MACAddress senderMACAddr;
}
