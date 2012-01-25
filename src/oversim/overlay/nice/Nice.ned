//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

package inet.oversim.overlay.nice;

import inet.oversim.common.BaseOverlay;
import inet.oversim.common.IOverlay;


@namespace(oversim);

//
// The main module of the NICE implementation
//
// @author Christian Huebsch
//
simple Nice extends BaseOverlay
{
     parameters:
        @class(Nice);
        double heartbeatInterval @unit(s);
        double maintenanceInterval @unit(s);
        double queryInterval @unit(s);
        double peerTimeoutHeartbeats;
        double rpPollTimerInterval @unit(s);
        
        double clusterLeaderBound;
        double clusterLeaderCompareDist;
        double scProcDistance;
        double scMinOffset;
        
        int debug_clusterrefinement; 
        int debug_heartbeats; 
        int debug_visualization;
        int debug_join;
        int debug_peertimeouts;
        int debug_removes;
        int debug_queries;
        
        int enhancedMode;
        
        double startTime;
        
        int k;      
}

//
// Implementation of the NICE overlay as described in
// "Scalable Application Layer Multicast" by
// S. Banerjee and B. Bhattacharjee and C. Kommareddy,
// published at SIGCOMM'02, 2002.
//
// @author Christian Huebsch
//
module NiceModules like IOverlay
{
    parameters:
        @display("i=block/network2");

    gates:
        input udpIn;   // gate from the UDP layer
        output udpOut;    // gate to the UDP layer
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn;   // gate from the application
        output appOut;    // gate to the application

    submodules:
        nice: Nice {
            parameters:
                @display("p=60,60");

        }
    connections allowunconnected:
        udpIn --> nice.udpIn;
        udpOut <-- nice.udpOut;
        appIn --> nice.appIn;
        appOut <-- nice.appOut;
}

