//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// based on Andras Varga Ieee80211NIC 
// @author: Juan-Carlos Maureira
//
package inet.underTest.Ieee80211.linklayer.ieee80211.mgmt;
import inet.linklayer.ieee80211.radio.Ieee80211Radio;
import inet.underTest.Ieee80211.linklayer.ieee80211.mgmt.Ieee80211AgentSTAExtended;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mac.Ieee80211Mac;
import inet.linklayer.IWirelessNic;
import inet.base.IHook;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;

//
// An Abstract Wireless Network Card Interface. Mgmgt module is given by
// the user and agent is external to the NIC.
// 
module Ieee80211NicExtended like IWirelessNic
{
    parameters:
        string mgmtType = default("Ieee80211MgmtSTAExtended"); // name of the management module type (implements IIeee80211Mgmt)
        string opMode @enum("b","g","a","p") = default("g");
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        bool _agentNeeded = (mgmtType == "Ieee80211MgmtSTAExtended");   // internal par. do not use, shows if optional agent module is needed 
        @display("i=block/ifcard;bgb=259,357");
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        input radioIn; // to receive AirFrames
        input agentIn; // to Agent
        output agentOut; // from Agent

    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0 {
            @display("p=96,59");
        }
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0 {
            @display("p=42,103");
        }
        // optional agent module (can be either 0 or 1 sized vector)
        agent: Ieee80211AgentSTAExtended if _agentNeeded {
            parameters:
                @display("p=202,136");
        }
        mgmt: <mgmtType> like IIeee80211Mgmt {
            parameters:
                @display("p=96,136;q=wlanDataQueue");
        }
        mac: Ieee80211Mac {
            parameters:
                opMode = opMode;
                queueModule = "mgmt";
                @display("p=96,222");
        }
        radio: Ieee80211Radio {
            parameters:
                @display("p=96,307");
        }
    connections:
        radioIn --> radio.radioIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radio.upperLayerOut --> mac.lowerLayerIn;

        mac.upperLayerOut --> mgmt.macIn;
        mac.upperLayerIn <-- mgmt.macOut;

        mgmt.agentOut --> agent.mgmtIn if _agentNeeded;
        mgmt.agentIn <-- agent.mgmtOut if _agentNeeded;

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
}
