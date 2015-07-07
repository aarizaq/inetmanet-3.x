//
//  Copyright (C) 2012 Nikolaos Vastardis
//  Copyright (C) 2012 University of Essex
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package inet.routing.extras.saors.base;


import inet.routing.extras.DYMOFau;


//
// <b> The Socially-Aware Opportunistic Routing System Base </b>
//
// The Socially-Aware Opportunistic Routing System (SAORS) enables 
// the easy developement of opportunistic routing schemes in OMNET++.
// It extends the DYMO MANET routing protocol implementation of
// Christoph Sommer (DYMOFAU) that is found in the inetment framework.
// By using a MANET protocol as its base, it provide instant communication
// where a path can be found between tw nodes. On the other hand, if not,
// it has the ability to store the messages until the destination
// of a better carrier is found. To use SAORSBase in order to create
// a new opportunistic protocol, all that needs to be done is the
// extention of the hooks provided. In the C++ code of the module
// these are the following functions:
//
// * sendBeacon()
// * handleBeacon(SAORS_BEACON* my_beacon)
// * findEncounterProb(const SAORSBase_RoutingEntry* routeToNode)
// * compareEncounterProb(const SAORSBase_RoutingEntry* dtEntry, const SAORS_RREP* rrep)
// * sendEncounterProb(SAORS_RREQ* rreq, const SAORSBase_RoutingEntry* dtEntry)
// 
// These five function will provide all the opportunistic functionality
// needed for the creation of almost any probabilistic or epidemic
// routing scheme. 
//
// @see DTDYMO, DSDYMO
//
// @author: Nikolaos Vastardis
//
simple SAORSBase extends DYMOFau
{
    parameters:
        double BEACON_TIMEOUT @unit("s") = default(30s);
        double DTRT_DELETE_TIMEOUT @unit("s") = default(5000s);
        double DLT_OUTSTANDINGRREQ_TIMEOUT @unit("s") = default(3s);
        int NUM_COPIES = default(1);
        int SEND_COPIES = default(0);
        int SEND_COPIES_PC = default(0);
        bool EPIDEMIC = default(false);
        @statistic[RcvdDTMsgs](title="RcvdDTMsgs"; source="RcvdDTMsgs"; record=vector; interpolationmode=none);
        
    gates:
}

