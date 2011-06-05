package inet.underTest.linklayer.Ieee80216.ControlPlane;
//
// Used in 802.11 infrastructure mode: handles management frames for a station (STA).
//
// Relies on the MAC layer (Ieee80211Mac) for reception and transmission of frames.
//
// @author Roland Siedlczek
//
simple ControlPlaneBaseAP
{
    parameters:
        string basestationid; // Basestation ID as hex string (12 hex digits), or
                                // "auto". "auto" values will be replaced by
                                // a generated MAC address in init stage 0.
        double DLMapInterval;
        double DownlinkChannel;
        double UplinkChannel;
    gates:
        input cpsDownIn;
        input cpsUpIn;
        input radioDownIn;
        output cpsDownOut;
        output cpsUpOut;
        output radioUpOut;
}


