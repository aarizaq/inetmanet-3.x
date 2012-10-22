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

#ifndef DIAMETERS6A_H_
#define DIAMETERS6A_H_

#include "DiameterBase.h"
#include "SubscriberTableAccess.h"
#include "NotificationBoard.h"

/*
 * Class for Diameter application over S6a interface. This class is an implementation
 * of Diameter base model (interface). This application will process S6a application
 * specific messages that are forwarded from Diameter base layer. It will be used
 * to transfer subscriber information from the subscriber table of HSS to subscriber
 * table of MME.
 * More info about Diameter S6a application can be found in 3GPP TS 29272.
 */
class DiameterS6a : public DiameterBase, public INotifiable {
private:
	SubscriberTable *subT;
	NotificationBoard *nb;	// only for MME

	/*
	 * Method that processes notifications. This method will be used mainly in MME
	 * node because MME will ask about subscriber's authentication and authorization.
	 */
	virtual void receiveChangeNotification(int category, const cPolymorphic *details);

	/*
	 * Methods for processing and creating S6a application specific messages.
	 */
	DiameterMessage *createULR(Subscriber *sub);
	DiameterMessage *createULA(DiameterMessage *ulr);
	void processULA(DiameterMessage *ula);
public:
	DiameterS6a();
	virtual ~DiameterS6a();

	/*
	 * Method for initializing Diameter S6a model. This method will initialize first
	 * Diameter base protocol and afterwards will get access to subscriber and
	 * notification tables.
	 */
	void initialize(int stage);

	/*
	 * Method for processing only Diameter S6a messages. All messages arrived in
	 * Diameter base protocol, which are destined for S6a model will be processed
	 * by this method.
	 */
	DiameterMessage *processMessage(DiameterMessage *msg);

	/*
	 * Method for handling all messages that arrive in this module. All the messages
	 * will have to go through Diameter base model, which will forward them back
	 * to this module if they are destined for it.
	 */
	void handleMessage(cMessage *msg) { DiameterBase::handleMessage(msg); }
};

#endif /* DIAMETERS6A_H_ */
