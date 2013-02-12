package inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.Transceiver;
//
// @author Michael Krücken
//

//	"ConvergenceSublayerControlModule",
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.QoS.ConvergenceSublayerTrafficClassification;
import inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.QoS.ConvergenceSublayerHeaderCompression;



module ConvergenceSublayerTransceiver
{
    parameters:

    gates:
        input higherLayerGateIn[];
        output commonPartGateOut;
        output controlPlaneOut;

    submodules:
//		control: ConvergenceSublayerControlModule;
//			display: "p=80,130;i=block/switch";

        traffic_classification: ConvergenceSublayerTrafficClassification {
            parameters:
                @display("p=100,80;i=block/classifier");

        }
        header_comp: ConvergenceSublayerHeaderCompression {
            parameters:
                @display("p=100,180;i=block/join");

        }
    connections allowunconnected:
//        higherLayerGateIn --> control.outerIn;
//        commonPartGateOut <-- control.outerOut;

        for i=0..sizeof(higherLayerGateIn)-1 {
            higherLayerGateIn[i] --> traffic_classification.higherLayerGateIn++;
        }

        traffic_classification.headerCompressionGateOut --> header_comp.trafficClassificationGateIn;

        header_comp.commonPartGateOut --> commonPartGateOut;

        traffic_classification.controlPlaneOut --> { @display("m=w"); } --> controlPlaneOut;
}


