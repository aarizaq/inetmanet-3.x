//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

// @file ChurnGenerator.ned
// @author Ingmar Baumgart
//

package inet.oversim.common;

//
// ChurnGenerator base module
//
simple ChurnGenerator
{
    parameters:
        int targetOverlayTerminalNum; // number of nodes after init phase
        double initPhaseCreationInterval @unit(s); // node creation interval in init phase
        double noChurnThreshold @unit(s); // if the lifetimeMean of a specific
                                          // churn generator exceeds this,
                                          // NoChurn is used instead
        string channelTypes;   // used (tx) access technologies (from common/channels.ned)
        string channelTypesRx; // used (rx) access technologies (from common/channels.ned)
        @display("i=block/timer");
}
