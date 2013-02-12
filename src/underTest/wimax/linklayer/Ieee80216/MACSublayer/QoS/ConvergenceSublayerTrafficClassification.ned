package inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.QoS;
//
//@author Michael Krücken
//

simple ConvergenceSublayerTrafficClassification
{
    parameters:
        double voip_max_latency;
        double voip_tolerated_jitter;
        double sf_request_timeout;

    gates:
        input higherLayerGateIn[];
        input headerCompressionGateIn;
        output higherLayerGateOut;
        output headerCompressionGateOut;
        output controlPlaneOut;
}
