package inet.underTest.wimax.linklayer.Ieee80216.MACSublayer;
//
// This NIC implements an 802.16e network interface card,
// for Mobile Subscriber Station (MSS)
//

import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Receiver.CommonPartSublayerReceiverCompound;
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Receiver.ConvergenceSublayerReceiver;
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Receiver.SecuritySublayerReceiver;
import inet.underTest.wimax.linklayer.radio.Ieee80216Radio;


module ReceiverExtended

{
    gates:

        output macUpperGateOut; // from upper layers
        input controlPlaneGateIn;
        output controlPlaneGateOut;
        input radioIn; // to receive AirFrames

    submodules:

        csReceiver: ConvergenceSublayerReceiver {
            parameters:
                @display("p=128,76;i=block/cogwheel");
        }
        cpsReceiver: CommonPartSublayerReceiverCompound {
            parameters:
                @display("p=128,235;i=block/layer");
        }
        securityReceiver: SecuritySublayerReceiver {
            parameters:
                @display("p=128,352;i=block/decrypt");
        }
        radioReceiver: Ieee80216Radio {
            parameters:
                @display("p=124,486;i=prot3");
        }
    connections:

        macUpperGateOut <-- csReceiver.higherLayerGateOut;

        csReceiver.commonPartGateIn <-- cpsReceiver.convergenceGateOut;

//-------------------------------------------------------------------------------

        controlPlaneGateIn --> { @display("m=e"); } --> cpsReceiver.controlPlaneGateIn;
        controlPlaneGateOut <-- { @display("m=e"); } <-- cpsReceiver.controlPlaneGateOut;

//--------------------------------------------------------------------------------

        cpsReceiver.securityGateOut --> securityReceiver.commonPartGateIn;
        cpsReceiver.securityGateIn <-- securityReceiver.commonPartGateOut;

        radioReceiver.upperLayerIn <-- securityReceiver.receiverRadioGateOut;
        radioReceiver.upperLayerOut --> securityReceiver.receiverRadioGateIn;

        radioIn --> radioReceiver.radioIn;

}

