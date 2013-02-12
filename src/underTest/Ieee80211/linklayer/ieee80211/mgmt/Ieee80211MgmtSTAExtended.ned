//
// Station (managed mode) 802.11 managmenet implementation
//
// @author Juan-Carlos Maureira
//
package inet.underTest.Ieee80211.linklayer.ieee80211.mgmt;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
simple Ieee80211MgmtSTAExtended like IIeee80211Mgmt
{
    parameters:
        int frameCapacity = default(100); // maximum queue length
        double max_beacons_missed = default(5); // maximum number of beacon missed to notify agent about beacon lost event
        @display("i=block/cogwheel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input macIn;
        output macOut;
        input agentIn;
        output agentOut;
}
