package inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Transceiver;
//
// @author Michael Krücken
//

import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Transceiver.CommonPartSublayerTransceiver;
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.QoS.CommonPartSublayerScheduling;
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.QoS.CommonPartSublayerFragmentation;


module CommonPartSublayerTransceiverCompound
{
    parameters:

        @display("bgb=181,298");
    gates:
        input convergenceGateIn;
        input controlPlaneGateIn;
        input securityGateIn;
        output controlPlaneGateOut;
        output securityGateOut;

    submodules:
        cps_transceiver: CommonPartSublayerTransceiver {
            parameters:
                @display("p=100,152;i=block/switch");
        }
        scheduling: CommonPartSublayerScheduling {
            parameters:
                @display("p=100,64;i=block/segm");
        }
        fragmentation: CommonPartSublayerFragmentation {
            parameters:
                @display("p=100,240;i=block/join");
        }
    connections allowunconnected:

        controlPlaneGateIn --> { @display("m=w"); } --> cps_transceiver.controlPlaneGateIn;
        controlPlaneGateOut <-- { @display("m=w"); } <-- cps_transceiver.controlPlaneGateOut;

        cps_transceiver.schedulingGateIn <-- scheduling.commonPartGateOut;
        cps_transceiver.fragmentationGateOut --> fragmentation.commonPartGateIn;
        cps_transceiver.fragmentationGateIn <-- fragmentation.commonPartGateOut;

        scheduling.upperLayerGateIn <-- convergenceGateIn;

        fragmentation.securityGateOut --> { @display("m=s"); } --> securityGateOut;
        fragmentation.securityGateIn <-- securityGateIn;

}


