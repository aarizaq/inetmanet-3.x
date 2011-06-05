package inet.underTest.linklayer.Ieee80216.ControlPlane;

simple ControlPlaneBaseMS
{
    parameters:
        string address;              // MAC address as hex string (12 hex digits), or
                                       // "auto". "auto" values will be replaced by
                                       // a generated MAC address in init stage 0.
        double scanintervall;
    gates:
        input cpsDownIn;
        input cpsUpIn;
        input radioDownIn;
        output cpsDownOut;
        output cpsUpOut;
        output radioUpOut;
}


