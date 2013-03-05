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

import inet.linklayer.ieee80211.radio.Ieee80211Radio;
import inet.linklayer.ieee80211.mgmt.Ieee80211AgentSTA;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mac.Ieee80211Mac;
import inet.linklayer.ieee80211mesh.mgmt.Ieee80211Mesh;
import inet.base.IHook;
import inet.linklayer.ieee80211mesh.locator.ILocator;


//
// This NIC implements an 802.11g network interface card in ad-hoc mode.
//
module Ieee80211NicMeshMulti
{
    parameters:
        string mgmtType = default("Ieee80211Mesh"); // name of the management module type (implements IIeee80211Mgmt)
        // string opMode @enum("b","g","a","p") = default("g");
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        bool _agentNeeded = false;   // internal par. do not use, shows if optional agent module is needed
        int meshChannels = default(1);
        bool hasLocator = default(false);
        @display("i=block/ifcard;bgb=259,357");
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        input fromEthernet @labels(Ieee80211Frame);   // to Ethernet MAC
        output toEthernet @labels(Ieee80211Frame); // to Ethernet MAC
        input radioIn[meshChannels] @labels(AirFrame); // to receive AirFrames
    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0;
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0;
        locator: <default("Locator")> like ILocator if hasLocator {
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
                @display("p=96,136;q=wlanDataQueue;i=block/switch");
             gates:
                macIn[meshChannels];
                macOut[meshChannels];
        }
        mac[meshChannels]: Ieee80211Mac {
            parameters:
                queueModule = "mgmt";
                @display("p=96,222");
        }
        radio[meshChannels]: Ieee80211Radio {
            parameters:
                @display("p=96,307");
        }
    connections allowunconnected:
        for i=0..meshChannels-1 {
            radioIn[i] --> radio[i].radioIn;
            radio[i].upperLayerIn <-- mac[i].lowerLayerOut;
            radio[i].upperLayerOut --> mac[i].lowerLayerIn;

            mac[i].upperLayerOut --> mgmt.macIn[i];
            mac[i].upperLayerIn <-- mgmt.macOut[i];

        }
        mgmt.agentOut --> agent.mgmtIn if _agentNeeded;
        mgmt.agentIn <-- agent.mgmtOut if _agentNeeded;

        mgmt.toEthernet --> toEthernet;
        mgmt.fromEthernet <-- fromEthernet;

        mgmt.upperLayerOut --> upperLayerOut if numInputHooks == 0;
        mgmt.upperLayerIn <-- upperLayerIn if numOutputHooks == 0;

        mgmt.upperLayerOut --> inputHook[0].in if numInputHooks > 0;
        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }
        inputHook[numInputHooks-1].out --> upperLayerOut if numInputHooks > 0;
        outputHook[0].in <-- upperLayerIn if numOutputHooks > 0;
        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }
        mgmt.upperLayerIn <-- outputHook[numOutputHooks-1].out if numOutputHooks > 0;
        mgmt.locatorIn <-- locator.outGate if hasLocator;
        mgmt.locatorOut --> locator.inGate if hasLocator;
}

