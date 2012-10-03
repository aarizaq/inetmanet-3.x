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

#ifndef SUBSCRIBERTABLE_H_
#define SUBSCRIBERTABLE_H_

#include "Subscriber.h"

#define CLEAN_TIMER_TIMEOUT	10

/*
 * Class for Subscriber table. This table will hold all the subscribers in a
 * particular network node.
 */
class SubscriberTable : public cSimpleModule {
private:
	unsigned enbIds;
	unsigned mmeIds;
	cMessage *cleanTimer;

	typedef std::vector<Subscriber*> Subscribers;
	Subscribers subs;

	/*
	 * Methods for parsing the XML configuration file. First the subscribers will
	 * be created, filled with info and afterwards their PDN connections will be
	 * stored in ESM entity.
	 * ex.
     *   <Subscriber
     *       imsi="578195505601234"
     *       msisdn="491999792001">
     *       <PDNConns>
     *           <PDNConn def="true" apn="home.test" pdnGwAddr="192.168.4.2" pdnAllocAddr="10.10.10.1"/>
     *           <!-- <PDNConn def="false" apn="work.test" pdnGwAddr="192.168.4.2" pdnAllocAddr="10.10.20.1"/> -->
     *       </PDNConns>
     *   </Subscriber>
	 */
	void loadPDNConnectionsFromXML(const cXMLElement& subElem, Subscriber *sub);
	void loadSubscribersFromXML(const cXMLElement& config);
public:
    SubscriberTable();
    virtual ~SubscriberTable();

    /*
     * Method for initializing the subscriber table. It will read the configuration
     * from the XML file and fill the table with the generated subscribers. Also
     * it schedules the clean timer in order to remove inactive subscribers.
     */
    virtual void initialize(int stage);

    /*
     * Method for handling messages that arrive for the subscriber table. No external
     * messages should arrive, but only the self message when the clean timer expires.
     */
    virtual void handleMessage(cMessage *msg);

    /*
     * Method for finding a subscriber for a given eNB and MME id. The method returns
     * the subscriber, if it is found, or NULL otherwise.
     */
    Subscriber *findSubscriberForId(unsigned enbId, unsigned mmeId);

    /*
     * Method for finding a subscriber for a given channel id. The method returns
     * the subscriber, if it is found, or NULL otherwise.
     */
    Subscriber *findSubscriberForChannel(int channelNr);

    /*
     * Method for finding a subscriber for a given IMSI. The method returns
     * the subscriber, if it is found, or NULL otherwise.
     */
    Subscriber *findSubscriberForIMSI(char *imsi);

    /*
     * Method for finding a subscriber for a message sequence number. The method
     * returns the subscriber, if it is found, or NULL otherwise.
     */
    Subscriber *findSubscriberForSeqNr(unsigned seqNr);

    /*
     * Method for deleting a range of subscribers. The method calls first the
     * destructor for the subscribers and removes them afterwards from the table.
     */
    void erase(unsigned start, unsigned end);

    /*
     * Method for delete a subscriber, at a specific position. The method calls
     * first the destructor for the subscriber and removes it from the table
     * afterwards.
     */
    void erase(unsigned it);

    /*
     * Wrapper methods.
     */
    unsigned int size() {return subs.size();}
    void push_back(Subscriber *sub) { subs.push_back(sub); }
    Subscriber *at(unsigned i) { return subs.at(i); }

    /*
     * Utility methods.
     */
    unsigned genEnbId() { return ++enbIds; }
    unsigned genMmeId() { return ++mmeIds; }

};

#endif /* SUBSCRIBERTABLE_H_ */
