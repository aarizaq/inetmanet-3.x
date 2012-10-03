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

#ifndef TUNNELENDPOINT_H_
#define TUNNELENDPOINT_H_

#include "GTPPath.h"

class PDNConnection;

/*
 * Class for GTP tunnel end point. Each tunnel end point is identified uniquely in the
 * GTP node by a local id or local TEID. The remote id is learned from its peer after
 * some message exchanging. For message exchanging a GTP tunnel end point uses a GTP
 * path. GTP tunnel end point also can have three types of owners, Subscriber in case
 * of a S11 tunnel end point, PDN connection, in case of a S5/S8 tunnel end point or
 * a Bearer context, in case of a GTP user tunnel end point.
 */
class TunnelEndpoint : public cPolymorphic {
private:
	unsigned localId;	// TEID
	unsigned remoteId;

	GTPPath *path;

	cPolymorphic *ownerp; // Subscriber, PDNConnection or BearerContext

	GTP	*module;	// same as for path
public:
	// used only in S-GW for user and control GTP message forwarding
	TunnelEndpoint *fwTunnEnd;

	TunnelEndpoint();
	TunnelEndpoint(GTPPath *path);
	virtual ~TunnelEndpoint();

    /*
     * Method for initializing a GTP tunnel end point. Basically all parameters are
     * set to their default values.
     */
	void init();

	/*
	 * Setter methods.
	 */
	void setLocalId(unsigned localId) { this->localId = localId; }
	void setRemoteId(unsigned remoteId) { this->remoteId = remoteId; }
	void setFwTunnelEndpoint(TunnelEndpoint *fwTunnEnd) { fwTunnEnd->fwTunnEnd = this; this->fwTunnEnd = fwTunnEnd; }
	void setOwner(cPolymorphic *ownerp);
	void setPath(GTPPath *path) { module = path->getModule(); this->path = path; }
	void setModule(GTP *module) { this->module = module; }

	/*
	 * Getter methods.
	 */
	unsigned getLocalId() { return localId; }
	unsigned getRemoteId() { return remoteId; }
	TunnelEndpoint *getFwTunnelEndpoint() { return fwTunnEnd; }
	GTPPath *getPath() { return path; }
	cPolymorphic *getOwner();
	GTP *getModule() { return module; }
	IPvXAddress getLocalAddr() { return path->getLocalAddr(); }
	IPvXAddress getRemoteAddr() { return path->getRemoteAddr(); }
	unsigned char getType() { return path->getType(); }

	/*
	 * Methods for sending and processing of GTP tunnel messages. The send methods
	 * create a certain GTP tunnel message and send it via GTP path. The process
	 * methods handle incoming GTP tunnel messages and modify parameters in certain
	 * nodes accordingly.
	 */
	void sendCreateSessionRequest();
	void sendCreateSessionResponse(unsigned seqNr, GTPInfoElem *cause);
	void processTunnelMessage(GTPMessage *msg);
	void processCreateSessionRequest(GTPMessage *msg);
	void processCreateSessionResponse(GTPMessage *msg);
	void sendModifyBearerRequest();
	void processModifyBearerRequest(GTPMessage *msg);
	void sendModifyBearerResponse(unsigned seqNr, GTPInfoElem *cause);
	void processModifyBearerResponse(GTPMessage *msg);

    /*
     * Methods for creating and processing of tunnel end points information element
     * used in certain GTP messages. Process method returns true if the information
     * element was correct or false otherwise.
     */
	GTPInfoElem *createFteidIE(unsigned char instance);
	bool processFteidIE(GTPInfoElem *fteid);

    /*
     * Method for printing information about a GTP tunnel end point.
     */
	std::string info(int tabs) const;
};

#endif /* TUNNELENDPOINT_H_ */
