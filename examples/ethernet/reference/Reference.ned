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

package inet.examples.ethernet.reference;


import inet.nodes.inet.StandardHost;
import inet.nodes.ethernet.EtherSwitch;
import inet.linklayer.ethernet.EtherHub;

import inet.linklayer.channels.ProgramedFailureChannel;
import ned.DatarateChannel;

import inet.linklayer.channels.ProgramedFailureDataRateChannel;

channel cable extends DatarateChannel
{
     datarate = 1000Mbps;
}

//
// Reference Topology according IEEE 802.1w document. Page 140
//
network Reference
{
    @display("bgb=835,509");
    submodules:
        SW_111: EtherSwitch {
            @display("p=269,98");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_222: EtherSwitch {
            @display("p=505,98");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_333: EtherSwitch {
            @display("p=269,194");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_444: EtherSwitch {
            @display("p=505,194");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_555: EtherSwitch {
            @display("p=145,329");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_666: EtherSwitch {
            @display("p=294,329");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_777: EtherSwitch {
            @display("p=454,329");
            relayUnitType = "MACRelayUnitSTPNP";
        }
        SW_888: EtherSwitch {
            @display("p=616,329");
            relayUnitType = "MACRelayUnitSTPNP";
        }
    connections:
        SW_111.ethg++ <--> cable <--> SW_222.ethg++;
        SW_111.ethg++ <--> cable <--> SW_222.ethg++;
        SW_111.ethg++ <--> cable <--> SW_444.ethg++;
        SW_111.ethg++ <--> cable <--> SW_333.ethg++;
        SW_222.ethg++ <--> cable <--> SW_333.ethg++;
        SW_222.ethg++ <--> cable <--> SW_444.ethg++;
        SW_333.ethg++ <--> cable <--> SW_888.ethg++;
        SW_333.ethg++ <--> cable <--> SW_777.ethg++;
        SW_333.ethg++ <--> cable <--> SW_666.ethg++;
        SW_333.ethg++ <--> cable <--> SW_555.ethg++;
        SW_444.ethg++ <--> cable <--> SW_888.ethg++;
        SW_444.ethg++ <--> cable <--> SW_777.ethg++;
        SW_444.ethg++ <--> cable <--> SW_666.ethg++;
        SW_444.ethg++ <--> cable <--> SW_555.ethg++;
}
