//**************************************************************************
// * file:        DMAMAC NIC descriptor file
// *
// * author:      A. Ajith Kumar S.
// * copyright:   (c) A. Ajith Kumar S. 
// * homepage:    www.hib.no/ansatte/aaks
// * email:       aji3003 @ gmail.com
// **************************************************************************
// * part of:     A dual mode adaptive MAC (DMAMAC) protocol for WSAN.
// * Refined on:  25-Apr-2015
// **************************************************************************
// *This file is part of DMAMAC (DMAMAC Protocol Implementation on MiXiM-OMNeT).
// *
// *DMAMAC is free software: you can redistribute it and/or modify
// *it under the terms of the GNU General Public License as published by
// *the Free Software Foundation, either version 3 of the License, or
// *(at your option) any later version.
// *
// *DMAMAC is distributed in the hope that it will be useful,
// *but WITHOUT ANY WARRANTY; without even the implied warranty of
// *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *GNU General Public License for more details.
// *
// *You should have received a copy of the GNU General Public License
// *along with DMAMAC.  If not, see <http://www.gnu.org/licenses/>./
// **************************************************************************
package inet.node.dmamac;

import inet.linklayer.contract.ITunNic;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.power.contract.IEnergyStorage;
import inet.power.contract.IEnergyGenerator;
import inet.common.lifecycle.NodeStatus;

import inet.applications.generic.IIPvXTrafficGenerator;


module DMAMACNodeNic
{
	parameters:
	    @networkNode();
        @labels(node,ethernet-node,wireless-node);
        
        int numApps = default(0);
        bool hasStatus = default(false);
        int numRadios = default(1);
        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");
        string networkLayerType = default("GenericNetworkLayer");
        string routingTableType = default("GenericRoutingTable");         
        string energyStorageType = default("");
        string energyGeneratorType = default("");
        routingTable.forwarding = true;
        routingTable.multicastForwarding = false;   // for IPv4, IPv6, Generic
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.networkProtocolType = default("GenericNetworkProtocol");
         *.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");
        *.energySourceModule = default(energyStorageType != "" ? absPath(".energyStorage") : "");
        *.mobilityModule = default(mobilityType != "" ? absPath(".mobility") : "");
     gates:
        input radioIn[1] @directIn;
    submodules:
        
        status: NodeStatus if hasStatus {
            @display("p=50,50;is=s");
        }
        
        app[numApps]: <> like IIPvXTrafficGenerator {
            parameters:
                @display("p=147,54,row,60");
        }
        
        energyStorage: <energyStorageType> like IEnergyStorage if energyStorageType != "" {
            parameters:
                @display("p=50,100;i=block/plug;is=s");
        }
        energyGenerator: <energyGeneratorType> like IEnergyGenerator if energyGeneratorType != "" {
            parameters:
                @display("p=50,150;i=block/plug;is=s");
        }
        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=53,200");
        }
        // network layer
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=329,287;q=queue");
        }
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=53,250;is=s");
        }
        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,300;is=s");
        }
        wlan: <default("DMAMACNic")> like IWirelessNic {
            parameters:
                @display("p=216,406,row,60;q=queue");
        }
    connections allowunconnected:
        // connections to network outside
        for i=0..numApps-1 {
            app[i].ipOut --> networkLayer.transportIn++;
            app[i].ipIn <-- networkLayer.transportOut++;
        }


       radioIn[0] --> wlan.radioIn;
       wlan.upperLayerOut --> networkLayer.ifIn++;
       wlan.upperLayerIn <-- networkLayer.ifOut++; 
}