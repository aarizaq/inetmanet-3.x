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

#ifndef SUBSCRIBERTABLEACCESS_H_
#define SUBSCRIBERTABLEACCESS_H_

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "SubscriberTable.h"

/**
 * Gives access to the SubscriberTable.
 */
class SubscriberTableAccess : public ModuleAccess<SubscriberTable> {
public:
	SubscriberTableAccess() : ModuleAccess<SubscriberTable>("subscriberTable") {}
};

#endif /* SUBSCRIBERTABLEACCESS_H_ */
