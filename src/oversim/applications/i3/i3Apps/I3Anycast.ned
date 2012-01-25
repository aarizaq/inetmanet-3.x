package inet.oversim.applications.i3.i3Apps;

import inet.oversim.common.ITier;


simple I3Anycast like ITier
{
    parameters:
        int clientPort;
        int serverPort;
        double triggerRefreshTime @unit(s);
        double sampleRefreshTime @unit(s);
        double bootstrapTime @unit(s);
        double initTime @unit(s);
        int cacheSize;
        double idStoreTime @unit(s);
        volatile double serverTimeout @unit(s);
    gates:
        input udpIn;
        input from_lowerTier;
        input from_upperTier;
        input tcpIn;
        output tcpOut;
        output udpOut;
        output to_lowerTier;
        output to_upperTier;
}
