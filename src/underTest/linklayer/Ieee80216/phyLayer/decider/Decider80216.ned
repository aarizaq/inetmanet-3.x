package inet.underTest.linklayer.Ieee80216.phyLayer.decider;
//
//
//


simple Decider80216
{
    parameters:
        bool debug; // debug switch
        volatile double snrThresholdLevel;
    gates:
        output uppergateOut;
        input lowergateIn;
}
