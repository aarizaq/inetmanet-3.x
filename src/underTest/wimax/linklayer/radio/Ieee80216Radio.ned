package inet.underTest.wimax.linklayer.radio;
import inet.linklayer.radio.IRadio;

simple Ieee80216Radio like IRadio
{
    parameters:
        int channelNumber = default(0); // channel identifier this radio listens. Works only with simlified management module. Otherwise it scans all the channels as specified in ieee 80211 
        double carrierFrequency @unit("Hz") = default(3.5GHz);
        double bitrate @unit("bps");
        double transmitterPower @unit("mW") = default(20mW); // power used for transmission of messages (in mW)
        double thermalNoise @unit("dBm") = default(-105dBm); // base noise level (dBm)
        double sensitivity @unit("dBm") = default(-85dBm); // received signals with power below sensitivity are ignored
        string radioModel = default("Ieee80216RadioModel");  // the radio model implementing the IRadioModel interface (C++). e.g. GenericRadioModel, Ieee80211RadioModel

        double shadowingDeviation @unit("dB") = default(0dB); // used by the shadowing model calculation
        double standardabweichung;
        double snirThreshold @unit("dB") = default(4dB);		// if signal-noise ratio is below this threshold, frame is considered noise (in dB)                
        bool isInput;
        
        string NoiseGenerator = default("");

        // generic FreeSpace model parameters
        double pathLossAlpha = default(2); // used by the path loss calculation
        double TransmissionAntennaGainIndB @unit("dB") = default(0dB);  // Transmission Antenna Gain 
        double ReceiveAntennaGainIndB @unit("dB") = default(0dB);       // Receive Antenna Gain
        double SystemLossFactor @unit("dB") = default(0dB);             // System Loss of Hardware 
        // two ray model paramaeters
        double TransmiterAntennaHigh @unit("m") = default(1m);   // Transmitter Antenna High 
        double ReceiverAntennaHigh @unit("m") = default(1m);   // Receiver Antenna High
        // LogNormalShadowingModel
        double sigma = default(1);
        // NakagamiModel
        double nak_m = default(1);
        // RiceModel 
        double K @unit("dB") = default(8dB);

        // battery module parameters (if any of them is negative, the battery module is disabled)
        double usage_radio_idle @unit(mA) = default(-1mA); // disable battery registration
        double usage_radio_recv @unit(mA) = default(-1mA); // disable battery registration
        double usage_radio_sleep @unit(mA) = default(-1mA); // disable battery registration 
        double usage_radio_send @unit(mA) = default(-1mA); // disable battery registration
        // misc 
        bool drawCoverage = default(true);  // draw the cverage area in Tkenv
        int refreshCoverageInterval @unit("s") = default(0s);  // update period in simulation time for coverage redraw

        @statistic[bitrate](title="Bitrate"; record=timeavg,vector; interpolationmode=sample-hold; );
        @statistic[channelNo](title="Channel no"; record=histogram,vector; interpolationmode=sample-hold; ); //vector
        @statistic[radioState](title="Radio state (enum)"; record=histogram,vector; interpolationmode=sample-hold; ); //vector
        @statistic[lossRate](title="Packet loss rate"; record=timeavg,vector; interpolationmode=sample-hold; );
    gates:
        input upperLayerIn @labels(PhyControlInfo/down); // from higher layer protocol (MAC)
        output upperLayerOut @labels(PhyControlInfo/up); // to higher layer protocol (MAC)
        input radioIn @labels(AirFrame); // to receive frames (AirFrame) on the radio channel
}


