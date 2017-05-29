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

import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.ieee80211.IIeee80211Mac;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mgmt.Ieee80211AgentSTA;
import inet.physicallayer.contract.packetlevel.IRadio;;

import inet.linklayer.ieee80211mesh.mgmt.Ieee80211Mesh;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.ieee80211mesh.locator.ILocator;
import inet.linklayer.ieee80211mesh.locator.LocatorModule;
import inet.linklayer.common.IIeee8021dQoSClassifier;
import inet.linklayer.contract.IWirelessNic;

//
// This NIC implements an 802.11g network interface card in ad-hoc mode.
//
module Ieee80211NicMeshMulti
{
    parameters:
        string interfaceTableModule;
        string energySourceModule;
         string classifierType = default(""); // needed for EDCA, as the classifier assigns User Priority (UP) to packets; UP determines Access Category via TID
        string mgmtType = default("Ieee80211Mesh"); // name of the management module type (implements IIeee80211Mgmt)
        string radioType = default("Ieee80211ScalarRadio");
        string macType = default("Ieee80211CompatibleMac");   // choose from Ieee80211OldMac, Ieee80211Mac, Ieee80211CompatibleMac
        
        string opMode @enum("a", "b", "g(erp)", "g(mixed)", "n", "p") = default("g(mixed)");
        double bitrate @unit("bps") = default(opMode == "b" ? 11Mbps : opMode == "p" ? 27Mbps : 54Mbps);
        
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        bool _agentNeeded = false;   // internal par. do not use, shows if optional agent module is needed
        int meshChannels = default(1);
        bool hasLocator = default(false);
        **.opMode = opMode;
        **.bitrate = bitrate;       
        @display("i=block/ifcard;bgb=259,357");
        @nic;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.energySourceModule = default(absPath(energySourceModule));        
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        input fromEthernet @labels(Ieee80211Frame);   // to Ethernet MAC
        output toEthernet @labels(Ieee80211Frame); // to Ethernet MAC
        input radioIn[meshChannels] @labels(AirFrame); // to receive AirFrames
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
        mgmt: Ieee80211Mesh {
            parameters:
                locatorActive = hasLocator;
                macModule = "^.mac";
                @display("p=96,136;q=wlanDataQueue;i=block/switch");
             gates:
                macInMulti[meshChannels];
                macOutMulti[meshChannels];
        }
        mac[meshChannels]: <macType> like IIeee80211Mac {
            parameters:
                @display("p=96,222");
        }
        radio[meshChannels]: <radioType> like IRadio {
            parameters:
                @display("p=96,307");
        }
    connections allowunconnected:
        for i=0..meshChannels-1 {
            radioIn[i] --> radio[i].radioIn;
            radio[i].upperLayerIn <-- mac[i].lowerLayerOut;
            radio[i].upperLayerOut --> mac[i].lowerLayerIn;

            mac[i].upperLayerOut --> mgmt.macInMulti[i];
            mac[i].upperLayerIn <-- mgmt.macOutMulti[i];

        }
        mgmt.agentOut --> agent.mgmtIn if _agentNeeded;
        mgmt.agentIn <-- agent.mgmtOut if _agentNeeded;

        mgmt.toEthernet --> toEthernet;
        mgmt.fromEthernet <-- fromEthernet;

        mgmt.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        mgmt.upperLayerIn <-- { @display("m=n"); } <-- upperLayerIn if classifierType == "";
        mgmt.upperLayerIn <-- { @display("m=n"); } <-- classifier.out if classifierType != "";
        classifier.in <-- { @display("m=n"); } <-- upperLayerIn if classifierType != "";
        
        mgmt.locatorIn <-- locator.outGate if hasLocator;
        mgmt.locatorOut --> locator.inGate if hasLocator;
}

