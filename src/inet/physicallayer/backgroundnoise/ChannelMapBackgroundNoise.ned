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

package inet.physicallayer.backgroundnoise;

import inet.physicallayer.contract.packetlevel.IRadioBackgroundNoise;

//
// This background noise model describes noise that does not change over space,
// time and frequency. It produces dimensional noise signals that can be further
// used in dimensional computations.
//
module ChannelMapBackgroundNoise extends IsotropicScalarBackgroundNoise
{
    parameters:
        
        xml noiseMap = default(xml("<config><frequency carrier='2.405e9' bandwidth='2e6' power='-70' /> </config>"));
        @class(ChannelMapBackgroundNoise);
        @display("i=block/mac");
}
