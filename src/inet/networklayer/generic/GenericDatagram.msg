//
// Copyright (C) 2012 Opensim Ltd.
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

import inet.common.TLVOption;
import inet.networklayer.common.L3Address;
import inet.networklayer.common.IPProtocolId;

namespace inet;

//
// Generic protocol TLV option types
//
enum GenericTLVOptionTypes
{
    GENERIC_TLVOPTION_NOP1 = 0;
    GENERIC_TLVOPTION_NOPN = 1;
    GENERIC_TLVOPTION_TLV_GPSR = 47;
};



//
// Represents a generic network datagram with generic network addresses. 
//
packet GenericDatagram
{
    @customize(true);
    L3Address sourceAddress @getter(_getSrcAddr);
    L3Address destinationAddress @getter(_getDestAddr);
    int transportProtocol @enum(IPProtocolId) = IP_PROT_NONE;
    short hopLimit;
    TLVOptions tlvOptions;
}
