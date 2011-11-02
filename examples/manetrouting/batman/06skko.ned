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

package inet.examples.manetrouting.batman;

import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;
import inet.nodes.inet.AdhocHost;
import inet.world.radio.ChannelControl;
import inet.nodes.ethernet.EtherHost;


//
// TODO auto-generated type
//
network _06skko
{
    @display("bgb=696,637");
    submodules:
        host1: AdhocHost {
            @display("p=300,300");
        }
        host2: AdhocHost {
            @display("p=300,350");
        }
        channelControl: ChannelControl {
            @display("p=464,52");
        }
        flatNetworkConfigurator: FlatNetworkConfigurator {
            networkAddress = "145.236.0.0";
            netmask = "255.255.0.0";
            @display("p=277,48");
        }
        host3: AdhocHost {
            @display("p=300,250");
        }
        host4: AdhocHost {
            @display("p=257,275");
        }
        host5: AdhocHost {
            @display("p=343,275");
        }
        host6: AdhocHost {
            @display("p=256,325");
        }
        host7: AdhocHost {
            @display("p=343,325");
        }
        host8: AdhocHost {
            @display("p=257,375");
        }
        host9: AdhocHost {
            @display("p=300,400");
        }
        host10: AdhocHost {
            @display("p=343,375");
        }
        host11: AdhocHost {
            @display("p=213,300");
        }
        ahost12: AdhocHost {
            @display("p=213,350");
        }
        host13: AdhocHost {
            @display("p=213,250");
        }

        host14: AdhocHost {
            @display("p=257,225");
        }
        host15: AdhocHost {
            @display("p=300,200");
        }

        ahost16: AdhocHost {
            @display("p=387,250");
        }

        host17: AdhocHost {
            @display("p=343,225");
        }

        host18: AdhocHost {
            @display("p=387,300");
        }
        host19: AdhocHost {
            @display("p=387,350");
        }


    connections:
//allowunconnected:
}
