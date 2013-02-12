package inet.underTest.wimax.linklayer.Ieee80216.MACSublayer.QoS;
//
//@author Michael Kr"uecken
//

simple CommonPartSublayerControlModule
{
    gates:
        input cpsUpIn;
        input controlIn;
        input cpsDownIn;
        input schedIn;
        input fragIn;
        input serviceflowIn;
        input authIn;
        output cpsUpOut;
        output controlOut;
        output cpsDownOut;
        output schedOut;
        output fragOut;
        output serviceflowOut;
        output authOut;

}
