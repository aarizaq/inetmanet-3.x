package inet.oversim.applications.i3;

import inet.oversim.common.ITier;


// Container for I3 module and trigger table
// @author Antonio Zea
module OverlayI3 like ITier
{
    parameters:
        bool debugOutput;

    gates:
        input udpIn;    // gate from the UDP layer
        input from_lowerTier;    // gate from the lower tier
        input from_upperTier;    // gate from the upper tier
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        output udpOut;    // gate to the UDP layer
        output to_lowerTier;    // gate to the lower tier
        output to_upperTier;    // gate to the upper tier

    submodules:
        i3: I3;
        triggerTable: TriggerTable;
    connections allowunconnected:
        udpIn --> i3.udpIn;
        udpOut <-- i3.udpOut;
        from_lowerTier --> i3.from_lowerTier;
        to_lowerTier <-- i3.to_lowerTier;
        from_upperTier --> i3.from_upperTier;
        to_upperTier <-- i3.to_upperTier;
}
