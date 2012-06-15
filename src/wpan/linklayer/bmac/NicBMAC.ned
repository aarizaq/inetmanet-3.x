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

//***************************************************************************
//* file:        NicBMAC.ned
//*
//* author:      Anna Foerster
//* author:      Alfonso Ariza
//* 
//* copyright:   (C) 22008- 2010, NetLab, SUPSI, Switzerland.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//*
//**************************************************************************/


package inet.wpan.linklayer.bmac;

import inet.wpan.linklayer.bmac.BMacLayer;
import inet.wpan.phyLayer.ieee802154.Ieee802154Phy;

module NicBMAC
{
    parameters:
        // name of the interface queue type
        string ifqType; // name of the interface queue type
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        input radioIn; // to receive AirFrames
    submodules:
        mac: BMacLayer {
            parameters:
                @display("p=70,180;i=block/layer");
        }
        phy: Ieee802154Phy {
            parameters:
                @display("p=70,280;i=block/wrxtx");
        }
    connections:
        radioIn --> phy.radioIn;
        phy.upperLayerOut --> { @display("m=m,50,50,75,0"); } --> mac.lowerLayerIn;
        phy.upperLayerIn <-- { @display("m=m,25,0,25,0"); } <-- mac.lowerLayerOut;
        mac.upperLayerOut --> upperLayerOut;
        mac.upperLayerIn <-- upperLayerIn;
}

