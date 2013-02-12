package inet.underTest.wimax.linklayer.Ieee80216.phyLayer.decider;
//
//
//


simple Decider80216
{
    parameters:
        bool debug; // debug switch
        volatile double snrThresholdLevel;
    gates:
        output upperLayerOut;
        input lowerLayerIn;
}
