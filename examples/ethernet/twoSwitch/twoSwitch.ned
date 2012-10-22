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

package inet.examples.ethernet.twoSwitch;

import inet.nodes.inet.StandardHost;
import inet.nodes.ethernet.EtherSwitch;
import inet.linklayer.ethernet.EtherHub;
import ned.DatarateChannel;

import inet.linklayer.channels.ProgramedFailureDataRateChannel;

channel cable extends DatarateChannel
{
     datarate = 1000Mbps;
}

network twoSwitch
{
    @display("bgb=586,362");
    types:
        channel cableFailure extends ProgramedFailureDataRateChannel
        {
            datarate = 1000Mbps;
            failureAt = "70s";
            recoveryAt = "280s";
        }

    submodules:
        SW_A: EtherSwitch {
            @display("p=278,93");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_B: EtherSwitch {
            @display("p=278,239");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        B: StandardHost {
            @display("p=365,240");
            routingFile = "B.rt";
        }
        A: StandardHost {
            @display("p=188,94");
            routingFile = "A.rt";
        }
    connections:
        A.ethg++ <--> cable <--> SW_A.ethg++;
        B.ethg++ <--> cable <--> SW_B.ethg++;
        
        // Main link between SW_A and SW_B
        // Programed to fail at 70 sec (1m10s) and recover at 280s (4m40s) (210 second of failure)
        SW_A.ethg++  <--> cableFailure <-->  SW_B.ethg++;
        // Backup link between SW_A and SW_B
        SW_A.ethg++  <--> cable <--> SW_B.ethg++;

}

network twoSwitchHub_Down
{
    @display("bgb=586,362");
    types:
        channel cableFailure extends ProgramedFailureDataRateChannel
        {
            datarate = 1000Mbps;
            failureAt = "70s";
            recoveryAt = "280s";
        }

    submodules:
        SW_A: EtherSwitch {
            @display("p=278,93");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_B: EtherSwitch {
            @display("p=278,306");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        B: StandardHost {
            @display("p=365,307");
            routingFile = "B.rt";
        }
        A: StandardHost {
            @display("p=188,94");
            routingFile = "A.rt";
        }
        etherHub: EtherHub {
            @display("p=278,192");
        }
    connections:
        A.ethg++ <--> cable <--> SW_A.ethg++;
        B.ethg++ <--> cable <--> SW_B.ethg++;
        etherHub.ethg++ <--> cable <--> SW_A.ethg++;
        etherHub.ethg++ <--> cableFailure <-->  SW_B.ethg++;
        etherHub.ethg++ <--> cable <--> SW_B.ethg++;
        etherHub.ethg++ <--> cable <--> SW_B.ethg++;
}

network twoSwitchHub_Up
{
    @display("bgb=586,362");
    types:
        channel cableFailure extends ProgramedFailureDataRateChannel
        {
            datarate = 1000Mbps;
            failureAt = "70s";
            recoveryAt = "270s";
        }
    
    submodules:
        SW_A: EtherSwitch {
            @display("p=278,93");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_B: EtherSwitch {
            @display("p=278,306");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        B: StandardHost {
            @display("p=365,307");
            routingFile = "B.rt";
        }
        A: StandardHost {
            @display("p=188,94");
            routingFile = "A.rt";
        }
        etherHub: EtherHub {
            @display("p=278,192");
        }
    connections:
        A.ethg++ <--> cable <--> SW_A.ethg++;
        B.ethg++ <--> cable <--> SW_B.ethg++;
        etherHub.ethg++ <--> cableFailure <--> SW_A.ethg++;
        etherHub.ethg++ <--> cable <--> SW_A.ethg++; 
        etherHub.ethg++ <--> cable <--> SW_B.ethg++;
        
}
