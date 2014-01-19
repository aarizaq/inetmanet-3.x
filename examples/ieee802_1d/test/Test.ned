package inet.examples.ieee802_1d.test;



import ned.DatarateChannel;
import inet.underTest.ieee8021d.nodes.Tester;
import inet.underTest.ieee8021d.nodes.BEB;

import inet.nodes.inet.StandardHost;
import inet.nodes.ethernet.EtherSwitch;
import inet.linklayer.ethernet.EtherHub;
import inet.underTest.ieee8021d.nodes.Tester;
import inet.underTest.ieee8021d.nodes.Ether1QBridge;
import inet.underTest.ieee8021d.nodes.EthernetHost;
import inet.util.ThruputMeteringChannel;
import inet.linklayer.channels.ProgramedFailureDataRateChannel;

network threeSw
{
    @display("bgb=750,630");

    types:
 channel accessCable extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }

        channel coreCable extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 1000Mbps;
        }


        channel throu_etherline1000 extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 1000Mbps;
            thruputDisplayFormat = "B";
        }
        channel failAcc1Server_Sw extends ProgramedFailureDataRateChannel
        {
            datarate = 1000Mbps;
            failureAt = "25s";
            recoveryAt = "70s";
        }

    submodules:
        access1: Ether1QBridge {
            parameters:
                @display("p=155,307");

            gates:
                ethg[3];

        }

        access2: Ether1QBridge {
            parameters:
                @display("p=518,307");

            gates:
                ethg[3];

        }

        server_switch: Ether1QBridge {
            parameters:
                @display("p=327,222");

            gates:
                ethg[3];

        }
        CSsrc1: EthernetHost {
            parameters:
                @display("p=155,392");
            gates:
                ethg;
        }
        CSsrc2: EthernetHost {
            parameters:
                @display("p=518,392");
            gates:
                ethg;
        }

        DBserver: EthernetHost {
            parameters:
                @display("p=327,110");
            gates:
                ethg;
        }
    connections:
        server_switch.ethg[0] <--> failAcc1Server_Sw <--> access1.ethg[0];
        server_switch.ethg[1] <--> coreCable <--> access2.ethg[0];
        server_switch.ethg[2] <--> throu_etherline1000 <--> DBserver.ethg;
        access1.ethg[1] <--> accessCable <--> access2.ethg[1];
        access1.ethg[2] <--> accessCable <--> CSsrc1.ethg;
        access2.ethg[2] <--> accessCable <--> CSsrc2.ethg;
}
