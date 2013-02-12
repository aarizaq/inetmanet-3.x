package inet.underTest.wimax.linklayer.Ieee80216.ControlPlane;
//
// @author Michael Krücken
//

import inet.underTest.wimax.linklayer.Ieee80216.ControlPlane.ControlPlaneBasestation;
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.QoS.CommonPartSublayerServiceFlows_BS;


module ControlPlaneBasestationCompound

{
    gates:
        input receiverCommonGateIn;
        input transceiverCommonGateIn;
        input trafficClassificationGateIn;
        output receiverCommonGateOut;
        output transceiverCommonGateOut;


    submodules:
        cp_basestation: ControlPlaneBasestation {
            parameters:
                @display("p=160,80;i=block/switch");

        }
        cp_serviceflows: CommonPartSublayerServiceFlows_BS {
            parameters:
                @display("p=100,160;i=block/classifier");

		//cp_authorization: CommonPartSublayerAuthorizationModule;
		//	display: "p=220,160;i=block/telnet";


        }
    connections:  // nocheck due to authorization modules' dummy gate

        // connections to the outer commonpart sublayer modules
        cp_basestation.receiverCommonGateIn <-- { @display("m=w"); } <-- receiverCommonGateIn;
        cp_basestation.receiverCommonGateOut --> { @display("m=w"); } --> receiverCommonGateOut;

        cp_basestation.transceiverCommonGateIn <-- { @display("m=e"); } <-- transceiverCommonGateIn;
        cp_basestation.transceiverCommonGateOut --> { @display("m=e"); } --> transceiverCommonGateOut;

        cp_basestation.trafficClassificationGateIn <-- { @display("m=n"); } <-- trafficClassificationGateIn;


        // internal connections
        cp_serviceflows.controlPlaneIn <-- cp_basestation.serviceFlowsGateOut;
        cp_serviceflows.controlPlaneOut --> cp_basestation.serviceFlowsGateIn;


}


