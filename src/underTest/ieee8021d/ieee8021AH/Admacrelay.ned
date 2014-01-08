package inet.underTest.ieee8021d.ieee8021AH;
//******************************************************
// @brief 802.1ah Admacrelay.
// Relay unit. Uses RSTP and MVRP modules and swtable information to determine the correct output gate for the incoming frames.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple Admacrelay
{
    parameters:
        bool verbose= default(false);   // sets the module verbosity     
    gates:
        output ifOut[];	// To the backbone and the IComponents
        input ifIn[];	// From the backbone and the IComponents
        inout RSTPGate;	// To the RSTP module
        inout MVRPGate;	// To the MVRP module
}
