// 
// @author Alfonso Ariza


cplusplus {{
#include "inet/linklayer/base/MACFrameBase_m.h"
}}

namespace inet;

class noncobject MACAddress;
class MACFrameBase;

packet XMacPkt extends MACFrameBase
{
    int preambleCont;
    MACAddress destinationAddress[]; // currently known occupied slots
}
