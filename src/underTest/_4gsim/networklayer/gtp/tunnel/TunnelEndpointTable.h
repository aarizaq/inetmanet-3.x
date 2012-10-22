//
// Copyright (C) 2012 Calin Cerchez
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

#ifndef TUNNELENDPOINTTABLE_H_
#define TUNNELENDPOINTTABLE_H_

#include "TunnelEndpoint.h"

/*
 * Class for GTP tunnel end point table. This table will hold all the tunnel end points
 * owned by the GTP model implementation for both version, GTP control and GTP user.
 * The tunnel end points for GTP control and GTP user are stored in a single tunnel end
 * point table.
 */
class TunnelEndpointTable : public cSimpleModule {
private:
	typedef std::vector<TunnelEndpoint*> TunnelEndpoints;
	TunnelEndpoints tunnEnds;
public:
	TunnelEndpointTable();
	virtual ~TunnelEndpointTable();

    /*
     * Method for initializing a GTP path table.
     */
	void initialize(int stage);

    /*
     * Method for finding a GTP tunnel end point for a given local TEID and path.
     * The method returns the tunnel end point, if it is found, or NULL otherwise.
     */
	TunnelEndpoint *findTunnelEndpoint(unsigned localId, GTPPath *path);

    /*
     * Method searches the table for a specific tunnel end point. The method returns
     * the tunnel end point, if it is found, or NULL otherwise.
     */
	TunnelEndpoint *findTunnelEndpoint(TunnelEndpoint *sender);

    /*
     * Wrapper methods.
     */
	void push_back(TunnelEndpoint *te) { tunnEnds.push_back(te); }
	void erase(unsigned start, unsigned end);
	unsigned size() {return tunnEnds.size();}
};

#endif /* TUNNELENDPOINTTABLE_H_ */
