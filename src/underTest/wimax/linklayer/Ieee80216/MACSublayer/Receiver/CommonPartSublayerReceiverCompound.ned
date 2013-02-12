package inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Receiver;
//
// @author Michael Kr"ucken
//

import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.QoS.CommonPartSublayerFragmentation;

module CommonPartSublayerReceiverCompound
{
    parameters:

    gates:
        input controlPlaneGateIn;
        input securityGateIn;
        output controlPlaneGateOut;
        output securityGateOut;
        output convergenceGateOut;


    submodules:
        cps_receiver: CommonPartSublayerReceiver {
            parameters:
                @display("p=100,80;i=block/switch");

        }
        fragmentation: CommonPartSublayerFragmentation {
            parameters:
                @display("p=100,160;i=block/join");


        }
    connections:

        controlPlaneGateIn --> { @display("m=e"); } --> cps_receiver.controlPlaneGateIn;
        controlPlaneGateOut <-- { @display("m=e"); } <-- cps_receiver.controlPlaneGateOut;

//        cps_receiver.securityGateOut --> securityGateOut display "m=w";
//        securityGateIn --> cps_receiver.securityGateIn display "m=w";

        cps_receiver.convergenceGateOut --> { @display("m=n"); } --> convergenceGateOut;
        cps_receiver.fragmentationGateOut --> fragmentation.commonPartGateIn;

        securityGateIn --> { @display("m=s"); } --> fragmentation.securityGateIn;
        fragmentation.commonPartGateOut --> cps_receiver.fragmentationGateIn;
        fragmentation.securityGateOut --> securityGateOut;

}


