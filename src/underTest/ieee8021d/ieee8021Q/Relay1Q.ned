package inet.underTest.ieee8021d.ieee8021Q;
//******************************************************
// @brief 802.1Q relay
// Relaying functions.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple Relay1Q
{
    parameters:
        @display("i=block/broadcast;bgb=425,114");
        bool verbose = default(false);
        
 
    gates: 
        input ifIn[] @labels(EtherFrame);
        output ifOut[] @labels(EtherFrame);
        inout RSTPGate;	// To the RSTP module
        inout MVRPGate @loose;	// To the MVRP module
}
