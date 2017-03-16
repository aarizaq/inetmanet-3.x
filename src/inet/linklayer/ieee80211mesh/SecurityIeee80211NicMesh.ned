//
// Copyright (C) 2008 Alfonso Ariza
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//

package inet.linklayer.ieee80211mesh;

import inet.linklayer.common.IIeee8021dQoSClassifier;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.ieee80211.oldmac.Ieee80211OldMac;
import inet.linklayer.ieee80211.mgmt.Ieee80211AgentSTA;
import inet.physicallayer.contract.packetlevel.IRadio;
import inet.linklayer.ieee80211mesh.locator.ILocator;
import inet.securityModule.Security;
import inet.linklayer.ieee80211mesh.mgmt.SecurityIeee80211Mesh;


//
// This NIC implements an 802.11g network interface card in ad-hoc mode.
//
module SecurityIeee80211NicMesh like IWirelessNic
{
    parameters:
        string interfaceTableModule;
        string energySourceModule;
        string classifierType = default(""); // needed for EDCA, as the classifier assigns User Priority (UP) to packets; UP determines Access Category via TID
        string mgmtType = default("SecurityIeee80211Mesh"); // name of the management module type (implements IIeee80211Mgmt)
        string radioType = default("Ieee80211Radio");
        string opMode @enum("b","g","a","p") = default("g");
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        double bitrate @unit("bps") = default(opMode == "b" ? 11Mbps : opMode == "p" ? 27Mbps : 54Mbps);
        bool _agentNeeded = false;   // internal par. do not use, shows if optional agent module is needed        
        bool hasLocator = default(false);
        bool securityNeeded = true;
        bool hasSecurity = default(false);        
        @display("i=block/ifcard;bgb=259,357");
        @nic;        
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.energySourceModule = default(absPath(energySourceModule));
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        input fromEthernet @labels(Ieee80211Frame);   // to Ethernet MAC
        output toEthernet @labels(Ieee80211Frame); // to Ethernet MAC
        input radioIn @labels(AirFrame); // to receive AirFrames
    submodules:
        classifier: <classifierType> like IIeee8021dQoSClassifier if classifierType != "" {
            @display("p=110,59;i=block/classifier");
        }
        locator: <default("LocatorModule")> like ILocator if hasLocator {
            @display("i=abstract/people;p=164,182");
        }
        // optional agent module (can be either 0 or 1 sized vector)
        agent: Ieee80211AgentSTA if _agentNeeded {
            parameters:
                @display("p=202,136");
        }
        security: Security if hasSecurity {
            // security: Security if securityNeeded {
            parameters:
                securityActive = hasSecurity;
                @display("p=234,43;i=block/encrypt");
        }

        mgmt: SecurityIeee80211Mesh {
            parameters:
                locatorActive = hasLocator;
                @display("p=96,136;q=wlanDataQueue;i=block/switch");
            gates:
                macIn[1];
                macOut[1];
        }
        mac: Ieee80211OldMac {
            parameters:
                opMode = opMode;
                bitrate = bitrate;
                @display("p=96,222");
        }
        radio: <radioType> like IRadio {
            parameters:
                @display("p=157,302");
        }
    connections allowunconnected:
        radioIn --> radio.radioIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radio.upperLayerOut --> mac.lowerLayerIn;

        mac.upperLayerOut --> mgmt.macIn[0];
        mac.upperLayerIn <-- mgmt.macOut[0];

        mgmt.agentOut --> agent.mgmtIn if _agentNeeded;
        mgmt.agentIn <-- agent.mgmtOut if _agentNeeded;
        
        mgmt.securityOut --> security.mgmtIn if hasSecurity;
        mgmt.securityIn <-- security.mgmtOut if hasSecurity;
        

        mgmt.toEthernet --> toEthernet;
        mgmt.fromEthernet <-- fromEthernet;

        mgmt.upperLayerOut --> upperLayerOut if numInputHooks == 0;
        mgmt.upperLayerIn <-- upperLayerIn if numOutputHooks == 0;

        mgmt.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        mgmt.upperLayerIn <-- { @display("m=n"); } <-- upperLayerIn if classifierType == "";
        mgmt.upperLayerIn <-- { @display("m=n"); } <-- classifier.out if classifierType != "";
        classifier.in <-- { @display("m=n"); } <-- upperLayerIn if classifierType != "";
        mgmt.locatorIn <-- locator.outGate if hasLocator;
        mgmt.locatorOut --> locator.inGate if hasLocator;
}




