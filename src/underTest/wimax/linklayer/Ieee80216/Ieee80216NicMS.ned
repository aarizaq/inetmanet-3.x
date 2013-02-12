package inet.underTest.wimax.linklayer.Ieee80216;
//
// This NIC implements an 802.16e network interface card,
// for Mobile Subscriber Station (MSS)
//

import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Receiver;
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Transceiver;
import inet.underTest.wimax.linklayer.Ieee80216.ControlPlane.ControlPlaneMobilestationCompound; //ControlPlaneMobilestation das war davor (VITA)

module Ieee80216NicMS

{
    gates:

        input macUpperGateIn[]; // to upper layers
        output macUpperGateOut; // from upper layers
        input radioIn; // to receive AirFrames
        input not;

    submodules:
        msReceiver: Receiver {
            parameters:
                @display("p=112,236;q=queue;i=block/ifcard");
        }
        msTransceiver: Transceiver {
            parameters:
                @display("p=288,235;q=queue;i=block/ifcard");
        }
        controlPlane: ControlPlaneMobilestationCompound {
            parameters:
                @display("p=200,237;i=block/cogwheel");
        }
    connections:

        for i=0..sizeof(macUpperGateIn)-1 {
            macUpperGateIn[i] --> { @display("m=n"); } --> msTransceiver.macUpperGateIn++;
        }

        macUpperGateOut <-- { @display("m=n"); } <-- msReceiver.macUpperGateOut;

       //-------------------------------------------------------------------------------

        controlPlane.receiverCommonGateOut --> msReceiver.controlPlaneGateIn;
        controlPlane.receiverCommonGateIn <-- msReceiver.controlPlaneGateOut;

        controlPlane.transceiverCommonGateOut --> msTransceiver.controlPlaneGateIn;
        controlPlane.transceiverCommonGateIn <-- msTransceiver.controlPlaneGateOut;

//		controlPlane.transceiverQoSGateOut --> msTransceiver.controlPlaneQoSGateIn;
//		controlPlane.transceiverQoSGateIn <-- msTransceiver.controlPlaneQoSGateOut;

//		controlPlane.receiverQoSGateOut --> msReceiver.controlPlaneQoSGateIn;
//		controlPlane.receiverQoSGateIn <-- msReceiver.controlPlaneQoSGateOut;

        controlPlane.trafficClassificationGateIn <-- msTransceiver.controlPlaneTrafficClassificationGateOut;


        //--------------------------------------------------------------------------------

        radioIn --> { @display("m=s,,,36,0"); } --> msReceiver.radioIn;
        not --> msTransceiver.not;
}

