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

#ifndef ESMENTITY_H_
#define ESMENTITY_H_

#include <omnetpp.h>
#include "PDNConnection.h"
#include "NASMessage_m.h"

class Subscriber;
class EMMEntity;
class NAS;

/*
 * Class for NAS ESM entity.
 * NAS ESM entity handles user equipment sessions for NAS protocol. Because of
 * that its owner is a subscriber object, but it is also related to NAS module.
 * The role of ESM entity is to hold all the PDN connections belonging to one
 * subscriber, each PDN connection holding all its bearer contexts.
 * NAS ESM entity is also working together with EMM entity for one subscriber.
 */
class ESMEntity {
private:
	unsigned connIds;
	unsigned char bearerIds;

	unsigned char appType;

	Subscriber *ownerp;

	PDNConnection *defConn;
	typedef std::vector<PDNConnection*> PDNConnections;
	PDNConnections conns;

	NAS *module;

	EMMEntity *peer;
public:
	ESMEntity();
	ESMEntity(unsigned char appType);
	virtual ~ESMEntity();

    /*
     * Method for initializing a ESM entity. Basically all parameters are set to
     * their default values.
     */
	void init();

	/*
	 * Setter methods.
	 */
	void setOwner(Subscriber *owernp);
	void setDefPDNConnection(PDNConnection *defConn) { this->defConn = defConn; }
	void setPeer(EMMEntity *peer);
	void setModule(NAS *module);

	/*
	 * Getter methods.
	 */
	Subscriber *getOwner();
	unsigned char getAppType() { return appType; }
	PDNConnection *getDefPDNConnection() { return defConn; }
	EMMEntity *getPeer();

    /*
     * Method for adding a PDN connection to the ESM entity. It sets the owner of the
     * PDN connection to this ESM entity and if the connection is default it records
     * it accordingly.
     */
	void addPDNConnection(PDNConnection *conn, bool def);

    /*
     * Method for deleting a range of bearers. The method calls first the
     * destructor for the bearers and removes them afterwards from the vector.
     */
	void delPDNConnection(unsigned start, unsigned end);

	/*
	 * Methods for generating identifiers.
	 */
	unsigned char genBearerId() { return ++bearerIds; }
	unsigned genPDNConnectionId() { return ++connIds; }

    /*
     * Methods for processing and creating of APNconfigProf AVP. This information
     * will be used for Diameter S6a application. Process method return true if
     * message processing was successful or false otherwise.
     */
	AVP *createAPNConfigProfAVP();
	bool processAPNConfigProfAVP(AVP *apnConfigProf);

	/*
	 * Wrapper methods.
	 */
    unsigned sizeOfPDNConnections() { return conns.size(); }

    /*
     * Method for printing information about ESM entity for a subscriber.
     */
	std::string info(int tabs) const;
};

#endif /* ESMENTITY_H_ */
