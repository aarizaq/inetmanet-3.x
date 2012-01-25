package inet.oversim.applications.scribe;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


simple Scribe extends BaseApp
{
    parameters:
        @class(Scribe);
        double childTimeout @unit(s); //seconds until a node assumes that a particular child has failed
        double parentTimeout @unit(s); //seconds until a node assumes that a particular parent has failed
}

module MulticastScribe like ITier
{
    gates:
        input udpIn;
        input from_lowerTier;
        input from_upperTier;
        input tcpIn;
        output tcpOut;
        output udpOut;
        output to_lowerTier;
        output to_upperTier;

    submodules:
        scribe: Scribe;
    connections allowunconnected:
        from_lowerTier --> scribe.from_lowerTier;
        to_lowerTier <-- scribe.to_lowerTier;
        from_upperTier --> scribe.from_upperTier;
        to_upperTier <-- scribe.to_upperTier;
        udpIn --> scribe.udpIn;
        udpOut <-- scribe.udpOut;

}
