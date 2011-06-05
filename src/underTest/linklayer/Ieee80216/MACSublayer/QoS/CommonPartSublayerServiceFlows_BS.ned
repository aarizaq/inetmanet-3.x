package inet.underTest.linklayer.Ieee80216.MACSublayer.QoS;

simple CommonPartSublayerServiceFlows_BS
{
    parameters:
        double lower_bound_for_BE_traffic;
        double upper_bound_for_BE_grant;

    gates:
        input controlPlaneIn;
        output controlPlaneOut;
}
