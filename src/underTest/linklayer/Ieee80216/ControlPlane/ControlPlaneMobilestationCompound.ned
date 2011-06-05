package inet.underTest.linklayer.Ieee80216.ControlPlane;
//
// @author Michael Krücken
//

import inet.underTest.linklayer.Ieee80216.MACSublayer.QoS.CommonPartSublayerServiceFlows_MS;


module ControlPlaneMobilestationCompound

{
    gates:
        input receiverCommonGateIn;
        input transceiverCommonGateIn;
        input trafficClassificationGateIn;
        output receiverCommonGateOut;
        output transceiverCommonGateOut;


    submodules:
        cp_mobilestation: ControlPlaneMobilestation {
            parameters:
                @display("p=160,80;i=block/switch");

        }
        cp_serviceflows: CommonPartSublayerServiceFlows_MS {
            parameters:
                @display("p=100,160;i=block/classifier");


        }
    connections:
        // connections to the outer commonpart sublayer modules
        cp_mobilestation.receiverCommonGateIn <-- { @display("m=w"); } <-- receiverCommonGateIn;
        cp_mobilestation.receiverCommonGateOut --> { @display("m=w"); } --> receiverCommonGateOut;

        cp_mobilestation.transceiverCommonGateIn <-- { @display("m=e"); } <-- transceiverCommonGateIn;
        cp_mobilestation.transceiverCommonGateOut --> { @display("m=e"); } --> transceiverCommonGateOut;

        cp_mobilestation.trafficClassificationGateIn <-- { @display("m=n"); } <-- trafficClassificationGateIn;

        // internal connections
        cp_serviceflows.controlPlaneIn <-- cp_mobilestation.serviceFlowsGateOut;
        cp_serviceflows.controlPlaneOut --> cp_mobilestation.serviceFlowsGateIn;


}


