package inet.underTest.linklayer.Ieee80216.MACSublayer.QoS;

simple CommonPartSublayerScheduling
{
    parameters:
        bool equal_weights_for_wrr;

        string scheduler;
        volatile double weight_ugs;
        volatile double weight_rtps;
        volatile double weight_ertps;
        volatile double weight_nrtps;
        double weight_be;

    gates:
        input upperLayerGateIn;
        input controlPlaneGateIn;
        output commonPartGateOut;
}
