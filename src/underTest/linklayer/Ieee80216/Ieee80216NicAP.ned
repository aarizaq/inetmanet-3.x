package inet.underTest.linklayer.Ieee80216;
// This NIC implements an 802.16e network interface card,
// for Base Station (BS)
//

import inet.underTest.linklayer.Ieee80216.Mac.ConvergenceSublayerUp;
import inet.underTest.linklayer.Ieee80216.Mac.ConvergenceSublayerDown;
import inet.underTest.linklayer.Ieee80216.Mac.CommonPartSublayerUp;
import inet.underTest.linklayer.Ieee80216.Mac.CommonPartSublayerDown;
import inet.underTest.linklayer.Ieee80216.Mac.SecuritySublayerUp;
import inet.underTest.linklayer.Ieee80216.Mac.SecuritySublayerDown;
import inet.linklayer.radio.GenericRadio;
import inet.underTest.linklayer.Ieee80216.ControlPlane.ControlPlaneBaseAP;


module Ieee80216NicAP
{
    parameters:
        @display("bgb=349,530");
    gates:
        input uppergateIn; // to upper layers
        output uppergateOut; // from upper layers
        input radioIn; // to receive AirFrames
        input not;
    submodules:
        csUp: ConvergenceSublayerUp {
            parameters:
                @display("p=128,76;i=block/cogwheel");
        }
        csDown: ConvergenceSublayerDown {
            parameters:
                @display("p=288,80;i=block/cogwheel");
        }
        cpsUp: CommonPartSublayerUp {
            parameters:
                @display("p=128,235;i=block/layer");
        }
        cpsDown: CommonPartSublayerDown {
            parameters:
                @display("p=288,235;i=block/layer");
        }
        securityUp: SecuritySublayerUp {
            parameters:
                @display("p=128,352;i=block/decrypt");
        }
        securityDown: SecuritySublayerDown {
            parameters:
                @display("p=288,352;i=block/encrypt");
        }
        radioUp: GenericRadio {
            parameters:
                @display("p=128,472;i=block/wrxtx");
        }
        radioDown: GenericRadio {
            parameters:
                @display("p=288,472;i=block/wrxtx");
        }
        controlplane: ControlPlaneBaseAP {
            parameters:
                @display("p=208,237;i=block/cogwheel");
        }
    connections:
        csUp.uppergateOut --> uppergateOut;
        csDown.uppergateIn <-- uppergateIn;

        cpsUp.uppergateOut --> csUp.lowergateIn;
        cpsDown.uppergateIn <-- csDown.lowergateOut;

        securityUp.uppergateOut --> cpsUp.lowergateIn;
        securityDown.uppergateIn <-- cpsDown.lowergateOut;

        radioUp.uppergateOut --> securityUp.lowergateIn;
        radioDown.uppergateIn <-- securityDown.lowergateOut;

        radioIn --> radioUp.radioIn;

        not --> radioDown.radioIn;


        controlplane.cpsDownOut --> cpsDown.controlplaneIn;
        cpsDown.controlplaneOut --> controlplane.cpsDownIn;
        cpsUp.controlplaneOut --> controlplane.cpsUpIn;
        controlplane.cpsUpOut --> cpsUp.controlplaneIn;

        controlplane.radioUpOut --> radioUp.uppergateIn;
        radioDown.uppergateOut --> controlplane.radioDownIn;
}

