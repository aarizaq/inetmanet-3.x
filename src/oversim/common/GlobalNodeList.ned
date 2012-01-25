//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//
// @author Markus Mauch, Robert Palmer
//

package inet.oversim.common;

//
// Global module that supports bootstrap process and key distribution
//
simple GlobalNodeList
{
    parameters:
        double maliciousNodeProbability;              // probability for a node to be malicious on creation
        bool maliciousNodeChange;                     // dynamically change number of malicious nodes
        double maliciousNodeChangeStartTime @unit(s); // simTime at which nodes begin to become malicious
        volatile double maliciousNodeChangeRate;              // number of nodes to change to malicious compared to all nodes
        volatile double maliciousNodeChangeInterval @unit(s); // time between two node changes to malicious
        volatile double maliciousNodeChangeStartValue;        // minimum amount of malicious nodes in the peerSet
        volatile double maliciousNodeChangeStopValue;         // maximum amount of malicious nodes in the peerSet

        int maxNumberOfKeys;             // maximum number of overlay keys the bootstrap oracle handles
        double keyProbability;           // probability of keys to be owned by nodes
        @display("i=block/control");
}
