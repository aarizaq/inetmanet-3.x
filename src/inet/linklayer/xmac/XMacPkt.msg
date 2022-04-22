// 
// @author Alfonso Ariza


import inet.linklayer.base.MACFrameBase;
import inet.linklayer.common.MACAddress;

namespace inet;

packet XMacPkt extends MACFrameBase
{
    int preambleCont;
    MACAddress destinationAddress[]; // currently known occupied slots
}
