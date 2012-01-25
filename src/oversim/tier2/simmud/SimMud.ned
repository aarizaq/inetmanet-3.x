package inet.oversim.tier2.simmud;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


simple SimMud extends BaseApp
{
    parameters:
        @class(SimMud);
        double playerTimeout @unit(s); // time for checking for dead nodes
        double areaDimension; // size of the play filed
        double AOIWidth;     // size of the area of interest
        double maxMoveDelay @unit(s); // time after which a move message is considered to be too late
        int numSubspaces; // number of subspaces per direction
}

module SimMudModules like ITier
{
    gates:
        input udpIn;
        input from_lowerTier;
        input from_upperTier;
        input tcpIn;
        output tcpOut;
        output udpOut;
        output to_lowerTier;
        output to_upperTier;

    submodules:
        simMud: SimMud;
    connections allowunconnected:
        from_lowerTier --> simMud.from_lowerTier;
        to_lowerTier <-- simMud.to_lowerTier;
        from_upperTier --> simMud.from_upperTier;
        to_upperTier <-- simMud.to_upperTier;
        udpIn --> simMud.udpIn;
        udpOut <-- simMud.udpOut;

}
