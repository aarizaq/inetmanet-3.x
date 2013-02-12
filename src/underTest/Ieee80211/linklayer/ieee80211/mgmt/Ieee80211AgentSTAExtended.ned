
//
// Used in 802.11 infrastructure mode: in a station (STA), this module
// controls channel scanning, association and handovers, by sending commands
// (e.g. Ieee80211Prim_ScanRequest) to the management module (Ieee80211MgmtSTA).
//
// @author Andras Varga
//
// Modified by Juan-Carlos Maureira. INRIA 2009
// - correct the dissasociation and back to scanning behavior
// - add a fixed ssid to connect with.
// - Agent status logging vector.
// - Agent starting time random. (avoid all the STAs starts to scan at the same time)
// - Agent Interface Implementation to allow polymorphism and dynamic agent loading
//
package inet.underTest.Ieee80211.linklayer.ieee80211.mgmt;
import  inet.underTest.Ieee80211.linklayer.ieee80211.mgmt.Ieee80211AgentInterface;

simple Ieee80211AgentSTAExtended like Ieee80211AgentInterface
{
    parameters:
        bool activeScan = default(true);  // selects between active and passive scanning
        string channelsToScan = default("");  // list of channel numbers to scan (space delimited); empty means all channels
        double probeDelay @unit("s") = default(0.1s); // delay before sending a probe request during active scanning
        double minChannelTime @unit("s") = default(0.15s); // min interval to spend on a channel during active scanning
        double maxChannelTime @unit("s") = default(0.3s); // channel time for passive scanning, and max channel time for active scanning
        double authenticationTimeout @unit("s") = default(5s); // timeout for the authentication procedure
        double associationTimeout @unit("s") = default(5s); // timeout for the association procedure
        double startingTime @unit("s") = default(uniform(0s,5s)); // agent starting time. 0 to disable the agent
        string default_ssid            = default(""); // default ssid
 		@display("i=block/control");
    gates:
        input mgmtIn;
        output mgmtOut;
}
