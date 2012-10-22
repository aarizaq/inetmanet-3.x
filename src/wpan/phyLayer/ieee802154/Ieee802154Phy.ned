package inet.wpan.phyLayer.ieee802154;
simple Ieee802154Phy
{
    parameters:

        bool debug;
        double channelNumber = default(0);
        bool NoBitError =default(true);

        // power used for transmission of messages (mW)
        double transmitterPower @unit("mW") = default(1.0mW);

        // received signals with power below sensitivity are ignored (dBm)
        double sensitivity @unit("dBm")=default(-85dBm);

        // base noise level (dBm)
        double thermalNoise @unit("dBm") = default(-110dBm);

        // used by PathLossReceptionModel
        double pathLossAlpha = default(2);
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
        
        // used by Ieee802154RadioModel
        double snirThreshold @unit("dB") = default(4dB); 
        double carrierFrequency @unit("Hz") = default(2.4GHz); 
                // parameters for energy usage
        bool drawCoverage =default(true);
        double refresCoverageInterval @unit("s")=default(5s);
                      
        string radioModel @enum("Ieee802154RadioModel") = default("Ieee802154RadioModel");
        double usage_radio_idle @unit("mA");
        double usage_radio_recv @unit("mA");
        double usage_radio_sleep @unit("mA");
        
        int aMaxPHYPacketSize  = default(127);      //max PSDU size (in bytes) the PHY shall be able to receive
        int aTurnaroundTime    = default(12);       //Rx-to-Tx or Tx-to-Rx max turnaround time (in symbol period)
        int aMaxBeaconOverhead = default(75);       //max # of octets added by the MAC sublayer to the payload of its beacon frame
        int aMaxFrameOverhead  = default(25);       //max # of octets added by the MAC sublayer to its payload w/o security.//max # of octets that can be transmitted in the MAC frame payload field
        bool forceIdle = default(true);

        @signal[bitrate](type="double";unit=bps); // current transmit bitrate
        @signal[channelNo](type=long);
        @signal[radioState](type=long);  // enum
        @signal[lossRate](type="double"); // 0..1; percentage of frames received incorrectly in the last period
        @statistic[bitrate](title="tx bitrate"; record=min,max,timeavg,vector; interpolationmode=sample-hold;unit=bps);
        @statistic[channelNo](title="channel no"; record=histogram,vector; interpolationmode=sample-hold);
        @statistic[radioState](title="radio state (enum)"; record=count,vector; interpolationmode=sample-hold);
        @statistic[lossRate](title="packet loss rate"; record=min,max,timeavg,vector; interpolationmode=backward-sample-hold);

    gates:
        input upperLayerIn;			// from Mac
        output upperLayerOut;			// to Mac
        input radioIn; 					// to receive frames (AirFrame) on the radio channel
}
