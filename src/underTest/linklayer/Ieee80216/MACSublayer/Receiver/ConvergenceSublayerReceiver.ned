package inet.underTest.linklayer.Ieee80216.MACSublayer.Receiver;
//
// @author Michael Kr"ucken
//

//	"ConvergenceSublayerControlModule",
import inet.underTest.linklayer.Ieee80216.MACSublayer.QoS.ConvergenceSublayerTrafficClassification;
import inet.underTest.linklayer.Ieee80216.MACSublayer.QoS.ConvergenceSublayerHeaderCompression;


module ConvergenceSublayerReceiver
{
    parameters:

    gates:
        input commonPartGateIn;
        output higherLayerGateOut;

    submodules:
//		control: ConvergenceSublayerControlModule;
//			display: "p=80,130;i=block/switch";

        traffic_classification: ConvergenceSublayerTrafficClassification {
            parameters:
                @display("p=200,80;i=block/classifier");

        }
        header_comp: ConvergenceSublayerHeaderCompression {
            parameters:
                @display("p=200,180;i=block/join");

        }
    connections allowunconnected:
//        commonPartGateIn --> control.outerIn;
//        higherLayerGateOut <-- control.outerOut;

        header_comp.trafficClassificationGateOut --> traffic_classification.headerCompressionGateIn;
        traffic_classification.higherLayerGateOut --> higherLayerGateOut;

        header_comp.commonPartGateIn <-- commonPartGateIn;

}
