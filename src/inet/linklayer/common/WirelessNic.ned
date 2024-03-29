//
// Copyright (C) 2006 Andras Varga
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

package inet.linklayer.common;

import inet.linklayer.contract.IMACProtocol;
import inet.linklayer.contract.IWirelessNic;
import inet.physicallayer.contract.packetlevel.IRadio;

module WirelessNic like IWirelessNic
{
    parameters:
        string classifierType = default("");
        string macType;
        string radioType;
        string interfaceTableModule;
        string energySourceModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.energySourceModule = default(absPath(this.energySourceModule));
        @display("i=block/ifcard;bgb=250,300");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IRadioFrame);
    submodules:
        classifier: <classifierType> like IIeee8021dQoSClassifier if classifierType != "" {
                @display("p=80,70");
        }
        mac: <macType> like IMACProtocol {
                @display("p=150,100");
        }
        radio: <radioType> like IRadio {
                @display("p=150,200");
        }
    connections:
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radio.upperLayerOut --> mac.lowerLayerIn;

        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        mac.upperLayerIn <-- { @display("m=n"); } <-- upperLayerIn if classifierType == "";
        mac.upperLayerIn <-- { @display("m=n"); } <-- classifier.out if classifierType != "";
        classifier.in <-- { @display("m=n"); } <-- upperLayerIn if classifierType != "";
}
