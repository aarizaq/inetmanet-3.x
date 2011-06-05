package inet.underTest.linklayer.Ieee80216.MACSublayer;
//
// This NIC implements an 802.16e network interface card,
// for Mobile Subscriber Station (MSS)
//

import inet.underTest.linklayer.Ieee80216.MACSublayer.Transceiver.CommonPartSublayerTransceiverCompound;
import inet.underTest.linklayer.Ieee80216.MACSublayer.Transceiver.ConvergenceSublayerTransceiver;
import inet.underTest.linklayer.Ieee80216.MACSublayer.Transceiver.SecuritySublayerTransceiver;
import inet.underTest.linklayer.radio.Ieee80216Radio;


module TransceiverExtended

{
    parameters:
        @display("bgb=548,538");
    gates:

        input macUpperGateIn[]; // from upper layers
        input controlPlaneGateIn;
        output controlPlaneGateOut;
        input not;

        output controlPlaneTrafficClassificationGateOut;

    submodules:
        csTransceiver: ConvergenceSublayerTransceiver {
            parameters:
                @display("p=288,80;i=block/cogwheel");

        }
        cpsTransceiver: CommonPartSublayerTransceiverCompound {
            parameters:
                @display("p=288,235;i=block/layer");

        }
        securityTransceiver: SecuritySublayerTransceiver {
            parameters:
                @display("p=288,352;i=block/encrypt");

        }
        radioTransceiver: Ieee80216Radio {
            parameters:
                @display("p=288,472;i=block/wrxtx");

        }
    connections:

        for i=0..sizeof(macUpperGateIn)-1 {
            macUpperGateIn[i] --> csTransceiver.higherLayerGateIn++;
        }

        csTransceiver.commonPartGateOut --> cpsTransceiver.convergenceGateIn;

        //-------------------------------------------------------------------------------

        controlPlaneGateIn --> { @display("m=w"); } --> cpsTransceiver.controlPlaneGateIn;
        controlPlaneGateOut <-- { @display("m=w,92,92"); } <-- cpsTransceiver.controlPlaneGateOut;

        controlPlaneTrafficClassificationGateOut <-- { @display("m=w"); } <-- csTransceiver.controlPlaneOut;
        //--------------------------------------------------------------------------------

        cpsTransceiver.securityGateOut --> securityTransceiver.commonPartGateIn;
        cpsTransceiver.securityGateIn <-- securityTransceiver.commonPartGateOut;

        securityTransceiver.transceiverRadioGateOut --> radioTransceiver.uppergateIn;
        securityTransceiver.transceiverRadioGateIn <-- radioTransceiver.uppergateOut;

        not --> radioTransceiver.radioIn;

}

