package inet.wpan.networklayer.Ieee802154StarRouting;
simple Ieee802154StarRouting
{
    parameters:
        // debug switch
        bool debug; 				// debug switch
        bool isPANCoor;
    gates:
        input upperLayerIn; 			// from the upper layer
        output upperLayerOut; 		// to the upper layer
        input lowerLayerIn; 			// from the physical layer
        output lowerLayerOut; 			// to the physical layer
}
