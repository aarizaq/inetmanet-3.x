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
// test

#ifndef GTPCONTROL_H_
#define GTPCONTROL_H_

#include "SubscriberTable.h"
#include "GatewayTable.h"
#include "GTP.h"

/*
 * Class for GTP control protocol. This class inherits GTP generic module and adds
 * GTP control functionality.
 */
class GTPControl : public GTP {
protected:
	GatewayTable *gT;
public:
	SubscriberTable *subT;

	GTPControl();
	virtual ~GTPControl();

	/* module */
	virtual void initialize(int stage);
	void processMessage(GTPMessage *msg, GTPPath *path);

    /*
     * Notification methods.
     */
	virtual void receiveChangeNotification(int category, const cPolymorphic *details);
};

#endif /* GTPCONTROL_H_ */
