package inet.physicallayer.antenna;

import inet.physicallayer.base.packetlevel.AntennaBase;
import inet.power.base.EnergySourceBase;


//
// This antenna model describes the well-known dipole antenna or doublet. It
// consists of two identical conductive elements, which are bilaterally
// symmetrical. 
// 
module PhasedArray extends AntennaBase
{
    parameters:
        double length @unit(m); // the length of the dipole antenna
        double distance;
        double freq;
        double phiz;
        int sectorWidth;
        string energySourceModule;


        @class(PhasedArray);
        @signal[radioModeChangedSignal](type="long");
        @signal[receptionStateChangedSignal](type="long");
        @signal[transmissionStateChangedSignal](type="long");
        @signal[receivedSignalPartChangedSignal](type="long");
        @statistic[guadagno](title="guadagno"; source="gain"; record=vector; interpolationmode=none);
    submodules:
}
