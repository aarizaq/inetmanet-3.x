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
package inet.underTest.linklayer.ieee80211.mgmt;
import inet.linklayer.ieee80211.radio.Ieee80211NewRadio;
import inet.linklayer.ieee80211.mgmt.Ieee80211AgentSTA;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mac.Ieee80211NewMac;
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
        string mgmtType; // name of the management module type
        @display("bgb=220,326;i=block/ifcard");
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        input radioIn; // to receive AirFrames
        input agentIn; // to Agent
        output agentOut; // from Agent

    submodules:

        mgmt: <mgmtType> like IIeee80211Mgmt {
            parameters:
                @display("p=96,69;q=wlanDataQueue");
        }
        mac: Ieee80211NewMac {
            parameters:
                queueModule = "mgmt";
                @display("p=96,155");
        }
        radio: Ieee80211NewRadio {
            parameters:
                @display("p=96,240");
        }
    connections:
        radioIn --> radio.radioIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radio.upperLayerOut --> mac.lowerLayerIn;

        mac.upperLayerOut --> mgmt.macIn;
        mac.upperLayerIn <-- mgmt.macOut;

        mgmt.upperLayerOut --> upperLayerOut;
        mgmt.upperLayerIn <-- upperLayerIn;

        mgmt.agentOut --> agentOut;
        agentIn --> mgmt.agentIn;
}
