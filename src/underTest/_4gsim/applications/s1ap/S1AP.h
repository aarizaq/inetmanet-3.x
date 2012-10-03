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

#ifndef S1AP_H_
#define S1AP_H_

#include <omnetpp.h>
#include "SCTPSocket.h"
#include "S1APUtils.h"
#include "S1APConnection.h"
#include "SubscriberTableAccess.h"
#include "IRoutingTable.h"
#include "NotificationBoard.h"

#define LISTENER	0
#define CONNECTOR	1

/*
 * Module for S1AP protocol. This module is an implementation of S1AP
 * protocol used for establishing control connections between the
 * user equipment and the network. S1AP implementation is based mainly
 * on S1APConnection class, which manages a connection between eNB and MME.
 *
 * All the information needed for S1AP connections are loaded from
 * XML files, information such as Cell id, PLMN id, Served GUMMEIs
 * and Supported TAs will be used in the message exchange.
 *
 * Also the module holds the table with all S1AP connections configured
 * in the simulation.
 *
 * S1AP is also a transition module, it will get message from the
 * upper layer, NAS and send them to SCTP transport socket.
 *
 * S1AP is implemented according to 3GPP TS 36413.
 */
class S1AP : public cSimpleModule, public INotifiable {
private:
	bool type;      // connector or listener
	char *plmnId;
	char *cellId;
	std::vector<SupportedTaItem> suppTas;
	std::vector<ServedGummeiItem> servGummeis;
	std::string name;
	unsigned char pagDrx;
	unsigned char relMmeCapac;

	int outboundStreams;
	SCTPSocket serverSocket;

    cQueue sendQueue;
    cMessage *retryTimer;

    IRoutingTable *rT;
    NotificationBoard *nb;
    S1APConnectionTable conns;

    /*
     * Methods for loading the configuration from the XML file.
     * ex. for eNB
     *   <S1AP   type="1"
     *           mcc="591"
     *           mnc="78"
     *           cellId="1"
     *           name="enb1">    <!-- 1 = connector -->
     *       <Connectors>
     *           <Connector address="192.168.2.2" port="30000"/>
     *       </Connectors>
     *       <SuppTas>
     *           <SuppTa tac="7712">
     *               <Bplmns>
     *                   <Bplmn mcc="591" mnc="78"/>
     *               </Bplmns>
     *           </SuppTa>
     *       </SuppTas>
     *   </S1AP>
     * ex. for MME
     *   <S1AP   type="0"
     *           name="mme1">        <!-- 0 = listener -->
     *       <Listeners port="30000">
     *           <Listener address="192.168.2.2"/>
     *       </Listeners>
     *       <ServGummeis>
     *           <ServGummei>
     *               <ServPlmns>
     *                   <ServPlmn mcc="558" mnc="71"/>
     *               </ServPlmns>
     *               <ServGroupIds>
     *                   <ServGroup id="4444"/>
     *               </ServGroupIds>
     *               <ServMmecs>
     *                   <ServMme code="55"/>
     *               </ServMmecs>
     *           </ServGummei>
     *       </ServGummeis>
     *   </S1AP>
     */
    void loadOwnConfigFromXML(const cXMLElement &s1apNode);
    void loadSuppTasFromXML(const cXMLElement &suppTasNode);
    void loadServGummeisFromXML(const cXMLElement &servGummeisNode);
    void loadConnectorsFromXML(const cXMLElement &s1apNode);
    void loadListenersFromXML(const cXMLElement &s1apNode);
    void loadConfigFromXML(const char *filename);
public:
    SubscriberTable *subT;

	S1AP();
	virtual ~S1AP();

	virtual int numInitStages() const  { return 5; }

	/*
	 * Method for initializing the module. During initialization
	 * the XML configuration file will be read, and the module
	 * will try to gain access to needed table and boards.
	 */
    virtual void initialize(int stage);

    /*
     * Method for message handling. This is just a decision method,
     * the actual message processing is done in handleLowerMessage,
     * handleUpperMessage and processTimer according to the source
     * of the message.
     */
    virtual void handleMessage(cMessage *msg);

    /*
     * Method for lower message handling. This method will get lower
     * messages sent from SCTP layer and redirect them to the
     * appropriate SCTP connection or create a new SCTP connection
     * if none is found.
     */
    void handleLowerMessage(cMessage *msg);

    /*
     * Method for upper message handling. This method will get upper
     * messages sent from NAS layer and redirect them to the appropriate
     * S1AP connection. The connection and the lower message type will
     * be picked based on S1APControlInfo parameters.
     */
    void handleUpperMessage(cMessage *msg);

    /*
     * Method for queue timer handling. When the queue timer expires,
     * the last message of the queue is popped out and the module tries
     * to send it to its destination.
     */
    void processTimer(cMessage *msg);

    /*
     * Getter methods.
     */
    std::string getName() { return name; }
    bool getType() { return type; }
    char *getPlmnId() { return plmnId; }
    std::vector<ServedGummeiItem> getServedGummeis() { return servGummeis; }
    unsigned char getPagingDrx() { return pagDrx; }
    unsigned char getRelMmeCapac() { return relMmeCapac; }

    /*
     * Method for sending messages to the upper layer. Besides the serialized
     * bytes of NAS messages, the method will send also subscriber information
     * stored in S1APControlInfo.
     */
    void sendMessageUp(Subscriber *sub, NasPdu *nasPdu);

	/*
	 * Notification methods.
	 */
	virtual void receiveChangeNotification(int category, const cPolymorphic *details);
	void fireChangeNotification(int category, const cPolymorphic *details) { nb->fireChangeNotification(category, details); }
};

#endif /* S1AP_H_ */
