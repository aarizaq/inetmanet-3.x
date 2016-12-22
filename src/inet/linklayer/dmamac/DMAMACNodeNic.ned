//**************************************************************************
// * file:        DMAMAC NIC descriptor file
// *
// * author:      A. Ajith Kumar S.
// * copyright:   (c) A. Ajith Kumar S. 
// * homepage:    www.hib.no/ansatte/aaks
// * email:       aji3003 @ gmail.com
// **************************************************************************
// * part of:     A dual mode adaptive MAC (DMAMAC) protocol for WSAN.
// * Refined on:  25-Apr-2015
// **************************************************************************
// *This file is part of DMAMAC (DMAMAC Protocol Implementation on MiXiM-OMNeT).
// *
// *DMAMAC is free software: you can redistribute it and/or modify
// *it under the terms of the GNU General Public License as published by
// *the Free Software Foundation, either version 3 of the License, or
// *(at your option) any later version.
// *
// *DMAMAC is distributed in the hope that it will be useful,
// *but WITHOUT ANY WARRANTY; without even the implied warranty of
// *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *GNU General Public License for more details.
// *
// *You should have received a copy of the GNU General Public License
// *along with DMAMAC.  If not, see <http://www.gnu.org/licenses/>./
// **************************************************************************

package inet.linklayer.dmamac;

import inet.linklayer.contract.IWirelessNic;
import inet.physicallayer.contract.packetlevel.IRadio;

module DMAMACNic like IWirelessNic
{
    parameters:
        string interfaceTableModule;
        string energySourceModule = default("");
        string radioType = default("Ieee802154NarrowbandScalarRadio");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.energySourceModule = default(absPath(energySourceModule));
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IRadioFrame);
    submodules:
        mac: DMAMAC {
            parameters:
                @display("p=100,150");
        }
        radio: <radioType> like IRadio {
            parameters:
                @display("p=100,250");
        }
    connections:
        upperLayerIn --> mac.upperLayerIn;
        mac.lowerLayerOut --> radio.upperLayerIn;
        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerOut --> mac.lowerLayerIn;
}
