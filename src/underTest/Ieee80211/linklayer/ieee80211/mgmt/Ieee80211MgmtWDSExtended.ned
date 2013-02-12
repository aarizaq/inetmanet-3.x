//
// Copyright (C) 2006 Juan-Carlos Maureira
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Wireless Distribution System for 802.11 
// it's only distributing data frames (using 4 address frames)
// among the clients specified in the WDSClients list. This list
// can be the module name (full path) or the MAC Address.
//
// TODO: There is no authentication between APs. So. it must implement
// an authentication round between senders and receivers. 
//
// @author Juan-Carlos Maureira
//
package inet.underTest.Ieee80211.linklayer.ieee80211.mgmt;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
simple Ieee80211MgmtWDSExtended like IIeee80211Mgmt
{
    parameters:
        string ssid = default("SSID");
        int frameCapacity = default(100); // maximum queue length
        string WDSClients = default(""); // modules's names or MAC address of next-hop WDS clients. separated by comma (,)
        @display("i=block/cogwheel");
    gates:
        input upperLayerIn; // from upper layers; can be unconnected
        output upperLayerOut; // to upper layers; can be unconnected
        input macIn; // from Ieee80211Mac
        output macOut; // to Ieee80211Mac
        
        input agentIn; // null in WDS
        output agentOut; // null in WDS
}

