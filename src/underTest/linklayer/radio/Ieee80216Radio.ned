package inet.underTest.linklayer.radio;

simple Ieee80216Radio
{
    parameters:
        int channelNumber = default(0); 								// channel identifier
        double carrierFrequency @unit("Hz") = default(3.5GHz);
        double transmitterPower @unit("mW") = default(20mW); 		// power used for transmission of messages (in mW)
        double bitrate @unit("bps"); 						   // (in bits/s)
        double thermalNoise @unit("dBm") = default(-105dBm); 	// base noise level (dBm)
        double shadowingDeviation @unit("dB") = default(0dB); // used by the shadowing model calculation
        double standardabweichung;
        double snirThreshold @unit("dB") = default(4dB);		// if signal-noise ratio is below this threshold, frame is considered noise (in dB)
        double sensitivity @unit("dBm") = default(-85dBm); 		// received signals with power below sensitivity are ignored
        string attenuationModel = default("WiMAXPathLossReceptionModel");
        bool isInput;
        double pathLossAlpha = default(2); // used by the path loss calculation
    gates:
        input uppergateIn;   // from higher layer protocol (MAC)
        output uppergateOut; // to decider (decider connects to higher layer protocol, i.e. the MAC)
        input radioIn; // to receive frames (AirFrame) on the radio channel
}


