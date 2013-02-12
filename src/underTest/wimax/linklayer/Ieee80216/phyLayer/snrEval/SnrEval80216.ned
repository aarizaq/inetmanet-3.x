package inet.underTest.wimax.linklayer.Ieee80216.phyLayer.snrEval;

simple SnrEval80216
{
    parameters:
        double channelNumber; // channel identifier
        bool debug; // debug switch
        double transmitterPower;
        double bitrate;
        double headerLength;
        double thermalNoise;
        double pathLossAlpha;
        double sensitivity;
    gates:
        input uppergateIn;   // from higher layer protocol (MAC)
        output uppergateOut; // to decider (decider connects to higher layer protocol, i.e. the MAC)
        input radioIn; // to receive frames (AirFrame) on the radio channel
}

