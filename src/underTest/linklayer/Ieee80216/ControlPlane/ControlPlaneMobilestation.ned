package inet.underTest.linklayer.Ieee80216.ControlPlane;

simple ControlPlaneMobilestation
{
    parameters:
        string address;             	 	// MAC address as hex string (12 hex digits), or
                                       		// "auto". "auto" values will be replaced by
                                       		// a generated MAC address in init stage 0.

        double scanintervall;		//
        double numChannels; 		// number of radio channels (frequencies)

        //Initial-Ranging parameter
        // Ranging Response timeout (Table 342 timer T3)
        double InitialRangingFail;	// Ranging Response timeout (Table 342 timer T3)
        double InitialRangingTimeOut;	// Ranging Response timeout (Table 342 timer T3)	

        double rangingintervall;	// Ranging Response timeout (Table 342 timer T3)	
        double registrationResponseTimeout; //mk: added


        //Scanning parameter        
        //
        double scanDuration; //
        double interleavingInterval; // Ranging Response timeout (Table 342 timer T3)
        // number of radio channels (frequencies)       
        double scanIteration; // number of radio channels (frequencies)       
        double DLMapInterval;

        //Handover parameter        
        // Handover Margin
        double maxMargin; // Handover Margin
        // Handover Margin
        double minMargin; // Handover Margin
        // Anzahl der Frames ueber die der SNR gemitelt wird
        double FrameAnzahl; // Anzahl der Frames ueber die der SNR gemitelt wird

    gates:
        input receiverCommonGateIn;
        input transceiverCommonGateIn;
        output receiverCommonGateOut;
        output transceiverCommonGateOut;

        input trafficClassificationGateIn;
        input serviceFlowsGateIn;
        output serviceFlowsGateOut;

}


