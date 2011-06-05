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

package inet.examples.ethernet.redundant;

import inet.nodes.inet.StandardHost;
import inet.nodes.ethernet.EtherSwitch;

import inet.linklayer.ethernet.EtherBus;
import inet.linklayer.ethernet.EtherHub;

import inet.linklayer.channels.ProgramedFailureChannel;
import inet.linklayer.channels.LinkFailureManager;

network Redundant
{
    @display("bgb=798,483");
    submodules:
        SW_A: EtherSwitch {
            @display("p=189,96");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_B: EtherSwitch {
            @display("p=556,96");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_C: EtherSwitch {
            @display("p=189,259");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        C: StandardHost {
            @display("p=361,435");
            routingFile = "C.rt";
        }
        A: StandardHost {
            @display("p=75,37");
            routingFile = "A.rt";
        }
        B: StandardHost {
            @display("p=731,97");
            routingFile = "B.rt";
        }
        SW_D: EtherSwitch {
            @display("p=556,259");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        etherHub: EtherHub {
            @display("p=556,182");
        }
        etherHub1: EtherHub {
            @display("p=361,354");
        }
        D: StandardHost {
            @display("p=731,182");
            routingFile = "D.rt";
        }
    connections:
        B.ethg++ <--> SW_B.ethg++;
        A.ethg++ <--> SW_A.ethg++;
        SW_A.ethg++ <--> SW_C.ethg++;
        SW_C.ethg++ <--> etherHub1.ethg++;
        SW_D.ethg++ <--> etherHub1.ethg++;
        SW_B.ethg++ <--> etherHub.ethg++;
        etherHub.ethg++ <--> SW_D.ethg++;
        etherHub1.ethg++ <--> C.ethg++;
        D.ethg++ <--> etherHub.ethg++;

        // link between SW A and SW B programed to failed at 70s
        SW_A.ethg++ <--> ProgramedFailureChannel { failureAt="70s"; recoveryAt="280s"; } <--> SW_B.ethg++;

}

network Redundant2
{
    @display("bgb=798,483");
    submodules:
        SW_A: EtherSwitch {
            @display("p=189,96");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_B: EtherSwitch {
            @display("p=556,96");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_C: EtherSwitch {
            @display("p=189,259");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        C: StandardHost {
            @display("p=361,435");
            routingFile = "C.rt";
        }
        A: StandardHost {
            @display("p=75,97");
            routingFile = "A.rt";
        }
        B: StandardHost {
            @display("p=731,37");
            routingFile = "B.rt";
        }
        SW_D: EtherSwitch {
            @display("p=556,259");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        etherHub: EtherHub {
            @display("p=556,182");
        }
        etherHub1: EtherHub {
            @display("p=361,354");
        }
        D: StandardHost {
            @display("p=731,182");
            routingFile = "D.rt";
        }
    connections:
        B.ethg++ <--> SW_B.ethg++;
        A.ethg++ <--> SW_A.ethg++;
        C.ethg++ <--> etherHub1.ethg++;
        D.ethg++ <--> etherHub.ethg++;

        SW_A.ethg++ <--> SW_C.ethg++;
        SW_C.ethg++ <--> etherHub1.ethg++;

        // redundant link to the segment 1 from SW_D
        SW_D.ethg++ <--> etherHub1.ethg++;
        SW_D.ethg++ <--> etherHub1.ethg++;

        // redundant link to the segment from SW_B
        SW_B.ethg++ <--> etherHub.ethg++;
        SW_B.ethg++ <--> etherHub.ethg++;

        // redundant link to the segment 2 from SW_D
        etherHub.ethg++ <--> SW_D.ethg++;
        etherHub.ethg++ <--> SW_D.ethg++;

        // link between SW A and SW B programed to failed at 70s
        SW_A.ethg++ <--> ProgramedFailureChannel { failureAt="70s"; recoveryAt="280s"; } <--> SW_B.ethg++;
}

