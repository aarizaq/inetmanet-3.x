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

#ifndef PDNCONNECTION_H_
#define PDNCONNECTION_H_

#include "BearerContext.h"
#include "GTPMessage_m.h"
#include "IPvXAddress.h"
#include "DiameterUtils.h"
#include "TunnelEndpoint.h"
#include "NASMessage_m.h"

class ESMEntity;

/*
 * Class for subscriber's PDN connection. PDN Connection has the role to offer IP
 * connectivity for the subscriber. This will be done with subscriber IP address.
 * Each PDN connection will be identified uniquely for a subscriber by an id and an
 * APN string. PDN address is the IP address of the serving P-GW for a particular
 * PDN connection.
 * The subscriber will have on S5/S8 interface also a GTP control tunnel, which will
 * be saved in the PDN connection object.
 * IP connectivity is based on bearers, each PDN connection can have multiple bearers
 * from them only one is called the default bearer.
 * PDN connection is not directly allocated to the subscriber, it has an owner, the
 * ESM entity of a subscriber.
 */
class PDNConnection : public cPolymorphic {
private:
	unsigned id;		// for HSS
	unsigned char type;	// pdn type
	std::string apn;
    IPvXAddress pdnAddr;
    IPvXAddress subAddr;

	ESMEntity *ownerp;

	TunnelEndpoint *s5s8Tunn;   // S5/S8 GTP control tunnel

	BearerContext *defBearer;   // is saved also in bearers vector
	typedef std::vector<BearerContext*> BearerContexts;
	BearerContexts bearers;
public:
	PDNConnection();
	PDNConnection(ESMEntity *ownerp);
	virtual ~PDNConnection();

	/*
	 * Method for initializing a PDN connection. Basically all parameters are set to
	 * their default values.
	 */
	void init();

	/*
	 * Setter methods.
	 */
	void setId(unsigned id) { this->id = id; }
	void setAPN(std::string apn) { this->apn = apn; }
	void setOwner(ESMEntity *ownerp);
	void setPDNGwAddress(IPvXAddress pdnAddr) { this->pdnAddr = pdnAddr; }
	void setSubscriberAddress(IPvXAddress subAddr);
	void setType(unsigned char type) { this->type = type; }
	void setS5S8TunnEnd(TunnelEndpoint *s5s8Tunn) { s5s8Tunn->setOwner(this); this->s5s8Tunn = s5s8Tunn; }

	/*
	 * Getter methods.
	 */
	unsigned getId() { return id; }
	std::string getAPN() { return apn; }
	ESMEntity *getOwner();
	IPvXAddress getPDNGwAddress() { return pdnAddr; }
	IPvXAddress getSubscriberAddress() { return subAddr; }
	unsigned char getType() { return type; }
	BearerContext *getDefaultBearer() { return defBearer; }
	TunnelEndpoint *getS5S8TunnEnd() { return s5s8Tunn; }
	Subscriber *getSubscriber();
	virtual const char *getName() const  {return "PDN";}
	bool isDefault();

    /*
     * Method for finding a bearer for a given bearer id. The method returns
     * the bearer, if it is found, or NULL otherwise.
     */
	BearerContext *findBearerContextForId(unsigned char id);

    /*
     * Method for finding a bearer for a given GTP procedure id. The method returns
     * the bearer, if it is found, or NULL otherwise.
     */
	BearerContext *findBearerContextForProcId(unsigned char procId);

	/*
	 * Method for adding a bearer to the vector. It sets the owner of the bearer to
	 * this PDN connection and if the bearer is default it records it accordingly.
	 */
	void addBearerContext(BearerContext *bearer, bool def);

    /*
     * Method for deleting a range of bearers. The method calls first the
     * destructor for the bearers and removes them afterwards from the vector.
     */
	void delBearerContext(unsigned start, unsigned end);

	/*
	 * Wrapper methods.
	 */
	BearerContext *getBearerContext(unsigned it) { return bearers.at(it); }
	unsigned sizeOfBearerContexts() {return bearers.size();}

	/*
	 * Methods for processing and creating of APNconfig AVP. This information will
	 * be used for Diameter S6a application. Process method return true if message
	 * processing was successful or false otherwise.
	 */
	AVP *createAPNConfigAVP();
	bool processAPNConfigAVP(std::vector<AVP*> apnConfigVec);

	/*
	 * Methods for taking and inserting PDN connection related information into GTP
	 * messages. Taking method returns true if the message was correct or false
	 * otherwise.
	 */
	void toGTPMessage(GTPMessage *msg);
	bool fromGTPMessage(GTPMessage *msg, TunnelEndpoint *te);

	/*
	 * Methods for processing and creating of NAS messages, which are related to
	 * PDN connection.
	 */
	NASPlainMessage *createPDNConnectivityRequest();
	NASPlainMessage *createActDefBearerRequest();
	NASPlainMessage *createActDefBearerAccept();
	void processPDNConnectivityRequest(NASPlainMessage *msg);
	void processActDefBearerRequest(NASPlainMessage *msg);
	void processActDefBearerAccept(NASPlainMessage *msg);

	/*
	 * Method for printing information about a PDN connection.
	 */
	std::string info(int tabs) const;
};

#endif /* PDNCONNECTION_H_ */
