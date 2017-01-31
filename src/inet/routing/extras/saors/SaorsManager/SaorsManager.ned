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

package inet.routing.extras.saors.SaorsManager;


//
// Dispatches the packets to the selected SAORS routing algorithm
// (creates the requested routing protocol dynamically)
//
// NOTE: you should not use this module directly because this is 
// internal to the DTRouting module.
//
// @see DTRouting, SAORSBase
//
// @author: Nikolaos Vastardis
//
simple SaorsManager
{
    parameters:
        // Manager parameters
        @class(SaorsManager);
		string routingProtocol @enum("DT-DYMO","EP-DYMO","R-DYMO","SimBetTS","SAMPhO") = default("");
        bool manetActive;
        bool manetPurgeRoutingTables = default (true);
        string interfaceTableModule;
        string routingTableModule;
        bool AUTOASSIGN_ADDRESS = default (false);
        string AUTOASSIGN_ADDRESS_BASE = default("10.0.0.0");
    gates:
        input ipIn;
        output ipOut;
        input from_dtdymo;
        output to_dtdymo;
        input from_sampho;
        output to_sampho;  
        input from_rdymo;
        output to_rdymo;
        input from_epdymo;
        output to_epdymo;
        input from_simbet;
        output to_simbet;    
}

