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

#ifndef GTPPATH_H_
#define GTPPATH_H_

#include "IPvXAddress.h"
#include "UDPControlInfo_m.h"
#include "GTPMessage.h"
#include "UDPSocket.h"

#include <omnetpp.h>

#define ECHO_TIMER_TIMEOUT	10
#define EPHEMERAL_PORT		61000

enum GTPPathType {
	S1_U_eNodeB_GTP_U 	= 0,
	S1_U_SGW_GTP_U		= 1,
	S5_S8_SGW_GTP_U		= 4,
	S5_S8_PGW_GTP_U		= 5,
	S5_S8_SGW_GTP_C 	= 6,
	S5_S8_PGW_GTP_C 	= 7,
	S11_MME_GTP_C		= 10,
	S11_S4_SGW_GTP_C	= 11
};

class GTP;

/*
 * Class for GTP path. GTP path is the logical connection between two adjacent GTP
 * nodes. GTP path is based upon a UDP connection, it has a local IP address, remote
 * IP address, control IP address for GTP user paths and GTP specific UDP ports.
 * GTP path can be of more than one type, based on the logical interface, on which GTP
 * is used as a tunneling protocol and on the network node type (S11 for MME or S-GW,
 * S5/S8 for S-GW or P-GW etc.).
 * GTP paths are kept alive with echo messages, these are exchanged when a certain
 * timer expires and the messages update the remote and local counter.
 */
class GTPPath {
private:
    unsigned char localCounter;
    unsigned char remoteCounter;
    unsigned char type;

	IPvXAddress localAddr;
	IPvXAddress remoteAddr;
	IPvXAddress ctrlAddr;

	cMessage *echoTimer;

	GTP *module;

	/*
	 * Methods for sending and processing of echo messages. This messages are used to
	 * keep alive a GTP path.
	 */
    void sendEchoResponse(unsigned seqNr, GTPInfoElem *cause);
    void sendEchoRequest();
    void processEchoRequest(GTPMessage *msg);

public:
	GTPPath();
	GTPPath(GTP *module, IPvXAddress localAddr, IPvXAddress remoteAddr, unsigned char type);
	virtual ~GTPPath();

	/*
	 * Setter methods.
	 */
	void setRemoteAddr(IPvXAddress remoteAddr) { this->remoteAddr = remoteAddr; }
	void setLocalAddr(IPvXAddress localAddr) { this->localAddr = localAddr; }
	void setCtrlAddr(IPvXAddress ctrlAddr) { this->ctrlAddr = ctrlAddr; }
	void setModule(GTP *module);
	void setType(unsigned char type) { this->type = type; }

	/*
	 * Getter methods.
	 */
	IPvXAddress getRemoteAddr() { return remoteAddr; }
	IPvXAddress getLocalAddr() { return localAddr; }
	unsigned char getType() { return type; }
	IPvXAddress getCtrlAddr() { return ctrlAddr; }
	bool getPlane();
	GTP *getModule();

	/*
	 * Method for sending GTP messages. The message gets information for UDP protocol
	 * layer attached and is sent to the lower layer. All message from GTP model are
	 * sent this way (path, tunnel etc.).
     */
	void send(GTPMessage *msg);

	/*
	 * Method for processing GTP messages. All messages intended for GTP protocol
	 * layer are processed in this function. Path messages are kept for GTP path
	 * other messages like tunnel message are forwarded to a GTP tunnel end point.
	 */
	void processMessage(cMessage *msg);

    /*
     * Utility methods for managing the echo expire timer.
     */
    void processEchoTimer();
    void resetEchoTimer();

    /*
     * Method for printing information about a GTP path.
     */
	std::string info(int tabs) const;
};

#endif /* GTPPATH_H_ */
