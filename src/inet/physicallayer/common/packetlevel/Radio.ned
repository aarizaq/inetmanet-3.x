//
// Copyright (C) 2013 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.physicallayer.common.packetlevel;

import inet.physicallayer.contract.packetlevel.IRadio;
import inet.physicallayer.contract.packetlevel.IAntenna;
import inet.physicallayer.contract.packetlevel.IReceiver;
import inet.physicallayer.contract.packetlevel.ITransmitter;
import inet.power.contract.IEnergyConsumer;

//
// The radio model describes the physical device that is capable of transmitting
// and receiving signals on the medium. It contains an antenna model, a
// transmitter model, a receiver model, and an energy consumer model.
//
// The radio model supports changing radio mode, transmission power, or bitrate
// via ~ConfigureRadioCommand, or with direct C++ function calls.
//
// switchingTimes parameter should be regarded as a 5x5 matrix:
//
//               OFF SLEEP RECEIVER TRANSMITTER TRANSCEIVER
// OFF           0 0 0 0 0
// SLEEP         0 0 0 0 0
// RECEIVER      0 0 0 0 0
// TRANSMITTER   0 0 0 0 0
// TRANSCEIVER   0 0 0 0 0
//
// The corresponding RadioMode pairs contain the time needed to
// switch from one state to another.
// For example: the 6th number identifies the time needed to switch
// from SLEEP to OFF.
//
// The first entry of the switchingTimes string indicates the
// time-related metric prefix and it MUST be s, ms or ns.
//
module Radio like IRadio
{
    parameters:
        string antennaType;                      // NED type of the antenna model
        string transmitterType;                  // NED type of the transmitter model
        string receiverType;                     // NED type of the receiver model
        string energyConsumerType = default(""); // NED type of the energy consumer model

        string radioMediumModule = default("radioMedium"); // module path of the medium module where this radio communicates
        string energySourceModule = default("");           // module path of the energy source module which provides energy to the radio

        string switchingTimes = default("ms 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"); // time parameters to switch between radio modes

        bool separateTransmissionParts = default(false); // when enabled the transmission of preamble, header and data part are simulated separately
        bool separateReceptionParts = default(false);    // when enabled the reception of preamble, header and data part are simulated separately 

        bool displayCommunicationRange = default(false); // if true communication range is displayed as a blue circle around the node
        bool displayInterferenceRange = default(false);  // if true interference range is displayed as a gray circle around the node

        *.energySourceModule = default(absPath(this.energySourceModule));

        @class(Radio);
        @display("i=block/wrxtx");

        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[radioModeChanged](type=long);
        @signal[listeningChanged];
        @signal[receptionStateChanged](type=long);
        @signal[transmissionStateChanged](type=long);
        @signal[receivedSignalPartChanged](type=long);
        @signal[transmittedSignalPartChanged](type=long);
        @signal[minSNIR];
        @signal[packetErrorRate];
        @signal[bitErrorRate];
        @signal[symbolErrorRate];

        @statistic[radioMode](title="Radio mode"; source=radioModeChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[receptionState](title="Radio reception state"; source=receptionStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[transmissionState](title="Radio transmission state"; source=transmissionStateChanged; record=count,vector; interpolationmode=sample-hold);

        @statistic[minSNIR](title="Min SNIR"; source=minSNIR; record=histogram);
        @statistic[packetErrorRate](title="Packet error rate"; source=packetErrorRate; record=histogram);
        @statistic[bitErrorRate](title="Bit error rate"; source=bitErrorRate; record=histogram);
        @statistic[symbolErrorRate](title="Symbol error rate"; source=symbolErrorRate; record=histogram);

    gates:
        input upperLayerIn @labels(ILinkLayerFrame/down);
        output upperLayerOut @labels(ILinkLayerFrame/up);
        input radioIn @labels(IRadioFrame);

    submodules:
        antenna: <antennaType> like IAntenna {
            parameters:
                @display("p=100,50");
        }
        transmitter: <transmitterType> like ITransmitter {
            parameters:
                @display("p=100,150");
        }
        receiver: <receiverType> like IReceiver {
            parameters:
                @display("p=100,250");
        }
        energyConsumer: <energyConsumerType> like IEnergyConsumer if energyConsumerType != "" {
            parameters:
                @display("p=100,350");
        }
}
