package inet.underTest.wimax.linklayer.Ieee80216;
//
// This NIC implements an 802.16e network interface card,
// for Base Station (BS)
//

//Das habe ich (Vita) verändert
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Receiver;
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Transceiver;
import inet.underTest.wimax.linklayer.Ieee80216.ControlPlane.ControlPlaneBasestationCompound;
    //"Receiver",
    //"Transceiver",
   // "ControlPlaneBasestationCompound";


module Ieee80216NicBS{

    gates:
        
        input macUpperGateIn[];
        output macUpperGateOut;
        input radioIn; // to receive AirFrames
        input not;


    submodules:
        bsReceiver: Receiver{
            parameters:
            @display("p=104,236;q=queue;i=block/ifcard");
        }
        bsTransceiver: Transceiver{
            parameters:
            @display("p=288,235;q=queue;i=block/ifcard");
        }
        controlPlane: ControlPlaneBasestationCompound{
            parameters:
            @display("p=192,277;i=block/cogwheel");
        }
    
    connections:
        
        for i=0..sizeof(macUpperGateIn)-1 {
             macUpperGateIn[i] --> { @display("m=n"); } --> bsTransceiver.macUpperGateIn++;
        }

         macUpperGateOut <-- { @display("m=n"); } <-- bsReceiver.macUpperGateOut;

        //-------------------------------------------------------------------------------

        controlPlane.receiverCommonGateOut --> bsReceiver.controlPlaneGateIn;
        controlPlane.receiverCommonGateIn <-- bsReceiver.controlPlaneGateOut;

        controlPlane.transceiverCommonGateOut --> bsTransceiver.controlPlaneGateIn;
        controlPlane.transceiverCommonGateIn <-- bsTransceiver.controlPlaneGateOut;

		controlPlane.trafficClassificationGateIn <-- bsTransceiver.controlPlaneTrafficClassificationGateOut;

        //--------------------------------------------------------------------------------

       radioIn --> { @display("m=s,,,36,0"); } --> bsReceiver.radioIn;
        not --> bsTransceiver.not;

}

