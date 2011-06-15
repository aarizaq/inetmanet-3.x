package inet.linklayer.ieee8021d.ethernet;

//******************************************************
// @brief EthernetIIFrame encapsulation.
// Modified from EtherLLC model.
//  - Removed LLC capabilities.
//  - Just EthernetIIFrames.
//  - Pause frames have also been removed.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple EthernetLink
{
    parameters:
        @display("i=block/fork");
        int outputFrame;  // 0=Ethernet 1=802.1q  2=802.1ad
        bool verbose=default(false); // verbosity
        int defaultVID=default(-1); //Default Vlan. Used when outputFrame is 802.1q or 802.1ad
        int defaultSVID=default(-1); //Default S-Vid. Used when outputFrame is 802.1ad.
        double interFrameTime @unit("s")=default(2s); //Time between two MVRPDUs
    gates:
        input upperLayerIn;   // higher layer protocols or applications
        output upperLayerOut; // higher layer protocols or applications
        input lowerLayerIn @labels(EtherFrame);     // to Ethernet MAC
        output lowerLayerOut @labels(EtherFrame);   // to Ethernet MAC
}

