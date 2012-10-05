//
// Copyright (C) 2012 Calin Cerchez
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

package inet.underTest._4gsim.linklayer.lte;

import inet.linklayer.radio.IRadio;

simple LTERadio like IRadio
{
    parameters:
        @display("i=block/wrxtx");
        double transmitterPower @unit("mW") = default(20mW);
        double bitrate @unit("bps") = default(2Mbps);
        int channelNumber = default(0);
        double snirThreshold @unit("dB") = default(4dB);
        double thermalNoise @unit("dBm") = default(-110dBm);
        string modulation = default("16-QAM");
        double bandwidth @unit("Hz") = default(2MHz);
        int headerLengthBits @unit(B) = default(2B);
        double sensitivity @unit("mW") = default(-85mW);
        double pathLossAlpha = default(2);
        double shadowingDeviation @unit("dB") = default(0dB);
        
        string radioModel;  // the radio model implementing the IRadioModel interface (C++). e.g. GenericRadioModel, Ieee80211RadioModel

        // generic FreeSpace model parameters
        double TransmissionAntennaGainIndB @unit("dB") = default(0dB);  // Transmission Antenna Gain
        double ReceiveAntennaGainIndB @unit("dB") = default(0dB);       // Receive Antenna Gain
        double SystemLossFactor @unit("dB") = default(0dB);             // System Loss of Hardware
        // two ray model paramaeters
        double TransmiterAntennaHigh @unit("m") = default(1m);   // Transmitter Antenna High
        double ReceiverAntennaHigh @unit("m") = default(1m);   // Receiver Antenna High
        // LogNormalShadowingModel
        double sigma = default(1);
        // NakagamiModel
        double nak_m = default(1);
        // RiceModel
        double K @unit("dB") = default(8dB);

        // battery module parameters (if any of them is negative, the battery module is disabled)
        double usage_radio_idle @unit(mA) = default(-1mA); // disable battery registration
        double usage_radio_recv @unit(mA) = default(-1mA); // disable battery registration
        double usage_radio_sleep @unit(mA) = default(-1mA); // disable battery registration
        double usage_radio_send @unit(mA) = default(-1mA); // disable battery registration
        
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input uppergateIn @labels(PhyControlInfo/down); // from higher layer protocol (NAS)
        output uppergateOut @labels(PhyControlInfo/up); // to decider (decider connects to NAS)
        input radioIn @labels(AirFrame); // to receive frames (AirFrame) on the radio channel
}