package inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Receiver;

simple CommonPartSublayerReceiver
{
    parameters:

    gates:
        input controlPlaneGateIn;
        input fragmentationGateIn;
        output controlPlaneGateOut;
        output fragmentationGateOut;
        output convergenceGateOut;
}


