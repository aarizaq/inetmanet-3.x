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

#ifndef GTP_H_
#define GTP_H_

#include <omnetpp.h>
#include "GTPPathTable.h"
#include "TunnelEndpointTable.h"
#include "NotificationBoard.h"

#define GTP_CONTROL		0
#define GTP_USER		1

/*
 * Module for generic GTP protocol. This class will be inherited by GTP control and
 * GTP user protocol implementations (plane). GTP control is based on GTPv2 control
 * protocol and GTP user is based on GTPv1 user protocol, both described for 4G
 * networks.
 *
 * The GTP protocol implementation is composed of several classes
 * (discussion follows below):
 *   - GTP: the generic module class
 *   - GTPControl: the GTP control class
 *   - GTPUser: the GTP user class
 *   - GTPPath: manages a GTP path
 *   - TunnelEndpoint: manages a GTP tunnel end point
 *
 * GTP tries to get access to 3 tables, GTPPathtable, where it manages GTP paths,
 * TunnelEndpointTable, where it manages the GTP tunnel end points and
 * NotificationBoard, from where it gets requests regarding specific GTP procedures.
 *
 * More info about GTPv2 control protocol can be found in 3GPP TS 29274 and about
 * GTPv1 user protocol can be found in 3GPP TS 29281.
 */
class GTP : public cSimpleModule, public INotifiable {
protected:
	bool plane;
	char *plmnId;
	unsigned char localCounter;
	unsigned tunnIds;

	GTPPathTable *pT;
	TunnelEndpointTable *teT;
	NotificationBoard *nb;

    /*
     * Method for loading the configuration from the XML file.
     * ex.
     * <GTP mcc="558" mnc="71">
     *   <Paths>
     *       <Path localAddr="192.168.5.2" remoteAddr="192.168.5.1" type="10"/> <!-- 10 = S11 MME GTP-C interface -->
     *   </Paths>
     * </GTP>
     */
	virtual void loadPathsFromXML(const cXMLElement &gtpNode);
public:
	UDPSocket socket;
	GTP();
	virtual ~GTP();

	/*
	 * Method for initializing GTP module. This method will be inherited by GTP
	 * control and user implementations. The method tries to gain access to the
	 * tables required by this module and parses the configuration file.
	 */
	virtual void initialize(int stage);

	/*
	 * Method for handling of messages. Message can be of two types, GTP messages or
	 * timers, the role of this method is to redirect this messages to one GTP path
	 * for further processing.
	 */
	virtual void handleMessage(cMessage *msg);

	/*
	 * Getter methods.
	 */
	unsigned char getLocalCounter() { return localCounter; }
	char *getPLMNId() { return plmnId; }
	unsigned genTeids() { return ++tunnIds; }
	bool getPlane() { return plane; }

    /*
     * Notification methods.
     */
	virtual void receiveChangeNotification(int category, const cPolymorphic *details) {}
	void fireChangeNotification(int category, const cPolymorphic *details) { nb->fireChangeNotification(category, details); }

	/*
	 * Wrapper methods.
	 */
    GTPPath *findPath(IPvXAddress addr, unsigned char type) { return pT->findPath(addr, type); }
    GTPPath *findPath(IPvXAddress ctrlAddr) { return pT->findPath(ctrlAddr); }
    void addTunnelEndpoint(TunnelEndpoint *te) { teT->push_back(te); }

};

#endif /* GTP_H_ */
