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

// Definitions required to create a BPDU frame

cplusplus {{
#include "STPDefinitions.h"
}}
class noncobject MACAddress;
class noncobject BPDUType;
class noncobject BridgeID;
class noncobject PortRole;

//
// Bridge Protocol Data Unit (BPDU)
//
packet BPDU {
    BPDUType type;
    BridgeID rootBID;
    int rootPathCost;
    BridgeID senderBID;
    int portId;
    int messageAge;
    int maxAge;
    int helloTime;
    int forwardDelay;
    bool topologyChangeFlag;
    bool ackFlag;
    bool proposal;
    bool agreement;
    PortRole portRole;
    bool learning;
    bool forwarding;
};

packet CBPDU extends BPDU {
    type = CONF_BPDU;
};

packet TCNBPDU extends BPDU {
    type = TCN_BPDU;
};