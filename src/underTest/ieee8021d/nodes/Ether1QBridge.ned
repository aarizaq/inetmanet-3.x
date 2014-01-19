//******************************************************
// @brief IEEE 802.1Q Bridge
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/

package inet.underTest.ieee8021d.nodes;
import inet.underTest.ieee8021d.ieee8021Q.PortFilt;
import inet.underTest.ieee8021d.ieee8021Q.Relay1Q;
import inet.underTest.ieee8021d.ieee8021Q.Cache1Q;
import inet.linklayer.ethernet.EtherMAC;
import inet.underTest.ieee8021d.RSTP.RSTP1Q;
import inet.underTest.ieee8021d.MVRP.MVRP;

//
// Model IEEE 802.1Q Bridge
// This model contains the RSTP (rapid spanning tree) and the MVRP (multiple vlan registration protocol) module
//
module Ether1QBridge
{
    parameters:
        bool withMvrp = default(true);
        @node();
        @display("i=device/switch;bgb=643,442");
    gates:
        inout ethg[];  //To the backbone
    submodules:
        relay: Relay1Q {
            parameters:
                @display("p=171,77");
            gates:
                ifOut[sizeof(ethg)];
                ifIn[sizeof(ethg)];
                RSTPGate;
                MVRPGate;
        }
        PortFilt[sizeof(ethg)]: PortFilt {
            parameters:
                @display("p=171,248,row;q=queue");
            gates:
                ifOut[2];
                ifIn[2];
        }

        cache: Cache1Q {
            parameters:
                @display("p=47,174");
        }
        mac[sizeof(ethg)]: EtherMAC {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=171,343,row;q=queue");
        }

        rstp: RSTP1Q {
            @display("p=47,38");
            gates:
                RSTPPort;
        }
        
        
        mvrp: MVRP if withMvrp {
            @display("p=47,107");
            gates:
                MVRPPort;
        }

    connections:
        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> mac[i].phys;
            PortFilt[i].ifOut[0] --> mac[i].upperLayerIn;
            PortFilt[i].ifIn[0] <-- mac[i].upperLayerOut;
            PortFilt[i].ifOut[1] --> relay.ifIn[i];
            PortFilt[i].ifIn[1] <-- relay.ifOut[i];
        }

        rstp.RSTPPort <--> relay.RSTPGate;
        mvrp.MVRPPort$o --> relay.MVRPGate$i if withMvrp;
        mvrp.MVRPPort$i <-- relay.MVRPGate$o if withMvrp;
}
