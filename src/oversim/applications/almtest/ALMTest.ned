package inet.oversim.applications.almtest;

import inet.oversim.common.BaseApp;
import inet.oversim.common.ITier;


simple ALMTest extends BaseApp
{
    parameters:
        @class(ALMTest);
        bool joinGroups = default(true);
        int messageLength;
        bool sendMessages = default(true);
}

module ALMTestModule like ITier
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
        almTest: ALMTest;
    connections allowunconnected:
        from_lowerTier --> almTest.from_lowerTier;
        to_lowerTier <-- almTest.to_lowerTier;
        from_upperTier --> almTest.from_upperTier;
        to_upperTier <-- almTest.to_upperTier;
        udpIn --> almTest.udpIn;
        udpOut <-- almTest.udpOut;

}

