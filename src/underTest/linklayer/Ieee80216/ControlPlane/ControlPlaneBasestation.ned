package inet.underTest.linklayer.Ieee80216.ControlPlane;
//Das Module übernimmt Aufgaben der Control Plane einer Basisstation
//
//
//
//
// @author Roland Siedlczek
//


simple ControlPlaneBasestation{

    parameters:
	string basestationid;   //: string, // Basestation ID as hex string (12 hex digits), or
	                              // "auto". "auto" values will be replaced by
	                              // a generated MAC address in init stage 0.
	
	double DLMapInterval;   	// war davor volatile double
	double DownlinkChannel ; 	//: numeric const,
	double startFrame;		//: numeric const,

	double UplinkChannel;	   //	: numeric const,
	double UplinkDownlinkTTG;	//: numeric const,
	
	bool enablePacking;		//: bool,
	double downlink_per_second;	// : numeric const,
	double downlink_to_uplink;	//	: numeric const,
	
	bool useULGrantWaitingQueue;		//: bool,
	double grant_compensation;			//: numeric const;

    gates:

        input trafficClassificationGateIn;
        input receiverCommonGateIn;
        input transceiverCommonGateIn;
        output receiverCommonGateOut;
        output transceiverCommonGateOut;

	input serviceFlowsGateIn;
        output serviceFlowsGateOut;

}


