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

#include "TunnelEndpointTable.h"
#include <algorithm>

Define_Module(TunnelEndpointTable);

std::ostream& operator<<(std::ostream& os, TunnelEndpoint& te) {
    os << te.info(0);
    return os;
};

TunnelEndpointTable::TunnelEndpointTable() {
	// TODO Auto-generated constructor stub

}

TunnelEndpointTable::~TunnelEndpointTable() {
	// TODO Auto-generated destructor stub
	erase(0, tunnEnds.size());
}

void TunnelEndpointTable::initialize(int stage) {
	WATCH_PTRVECTOR(tunnEnds);
}

TunnelEndpoint *TunnelEndpointTable::findTunnelEndpoint(unsigned localId, GTPPath *path) {
	for (unsigned i = 0; i < tunnEnds.size(); i++) {
		TunnelEndpoint *te = tunnEnds[i];
		if (te->getLocalId() == localId && te->getPath() == path)
			return te;
	}
	return NULL;
}

TunnelEndpoint *TunnelEndpointTable::findTunnelEndpoint(TunnelEndpoint *sender) {
	for (unsigned i = 0; i < tunnEnds.size(); i++) {
		TunnelEndpoint *te = tunnEnds[i];
		if (te->getRemoteId() == sender->getLocalId())
			return te;
	}
	return NULL;
}

void TunnelEndpointTable::erase(unsigned start, unsigned end) {
	TunnelEndpoints::iterator first = tunnEnds.begin() + start;
	TunnelEndpoints::iterator last = tunnEnds.begin() + end;
	TunnelEndpoints::iterator i = first;
	for (;i != last; ++i)
		delete *i;
	tunnEnds.erase(first, last);
}
