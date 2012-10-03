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

#ifndef NAS_H_
#define NAS_H_

#include <omnetpp.h>
#include "NASMessage_m.h"
#include "SubscriberTableAccess.h"
#include "NotificationBoard.h"
#include "IInterfaceTable.h"
#include "RoutingTable.h"

#define UE_APPL_TYPE		0
#define MME_APPL_TYPE		1
#define RELAY_APPL_TYPE		2
#define SEND_RETRY_TIMER	2

/*
 * Module class that implements NAS protocol. NAS protocol can be found in 3 different
 * network nodes, application type, UE, MME and eNB (only relay for NAS messages).
 * NAS messages can come from S1AP layer or from radio layer and can be sent likewise
 * to S1AP layer or radio layer, depending on the application type.
 * NAS module offers message handling support for subscribers EMM and ESM entities
 * described in more detail in the class files and in the specification.
 *
 * More information about NAS can be found in 3GPP TS 24301
 */
class NAS : public cSimpleModule, public INotifiable {
private:
	unsigned char appType;

	int channelNumber;

	SubscriberTable *subT;
	IInterfaceTable *ift;
	IRoutingTable *rt;
    NotificationBoard *nb;  // only for MME

    /*
     * Notification methods.
     */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

    /*
     * Methods for loading the configuration from the XML file
     * ex.
     *  <NAS appType="0"
     *       imsi="578195505601234">     <!-- appType - 0 = UE -->
     *  </NAS>
     */
    void loadConfigFromXML(const char *filename);
    void loadEMMConfigFromXML(const cXMLElement &nasNode);
    void loadESMConfigFromXML(const cXMLElement &nasNode);
public:
	NAS();
	virtual ~NAS();

    /*
     * Method for initializing the module. During initialization the XML
     * configuration file will be read, and the module will try to gain
     * access to needed table and boards.
     */
	virtual void initialize(int stage);

    /*
     * Method for message handling. This is just a decision method, the actual
     * message processing is done in handleMessageFromS1AP and handleMessageFromS1AP
     * according to the source of the message and the application type.
     */
	virtual void handleMessage(cMessage *msg);

	/*
	 * Getter methods.
	 */
	int getChannelNumber() { return channelNumber; }

	/*
	 * Method for handling messages coming from S1AP layer. This method will get
	 * the subscriber for the message based on S1APControlInfo and process the
	 * message based on its type, modifying the subscribers parameters.
	 */
	void handleMessageFromS1AP(cMessage *msg);

	/*
	 * Method for handling messages coming from radio layer. If the message comes
	 * to eNB relay it will be forwarded to the appropriate S1AP connection, else
	 * it will be processed based on its type.
	 */
	void handleMessageFromRadio(cMessage *msg);

	/*
	 * Method for sending messages to S1AP layer. The method adds S1AP specific
	 * information in S1APControlInfo additional to the NAS message.
	 */
	void sendToS1AP(NASPlainMessage *nmsg, unsigned subEnbId, unsigned subMmeId);

    /*
     * Method for sending messages to radio layer. The method adds radio specific
     * information in PhyControlInfo additional to the NAS message.
     */
	void sendToRadio(NASPlainMessage *nmsg, int channelNr);

	/*
	 * Wrapper methods.
	 */
	void dropTimer(cMessage *timer) { drop(timer); }
	void takeTimer(cMessage *timer) { take(timer); }

	/*
	 * Utility methods.
	 */
	virtual int numInitStages() const  { return 5; }

};

#endif /* NAS_H_ */
