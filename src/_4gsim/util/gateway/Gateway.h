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

#ifndef GATEWAY_H_
#define GATEWAY_H_

#include "IPvXAddress.h"

/*
 * Utility class for 4Gsim gateways. At the moment this class is only used on
 * MME to select the correct S-GW to serve a user equipment. This selection
 * is made during attach procedure and is based on TAC of the UE.
 * But this class can be extended to include other gateway types and selections.
 */
class Gateway {
private:
	char *plmnId;
	char *tac;
	unsigned pathId;

public:
	Gateway(char *plmnId, char *tac, unsigned pathId);
	virtual ~Gateway();

	/*
	 * Getter methods.
	 */
	char *getTAC() { return tac; }
	char *getPLMNId() { return plmnId; }
	unsigned getPathId() { return pathId; }

    /*
     * Method for printing information about a gateway.
     */
	std::string info() const;
};

#endif /* GATEWAY_H_ */
