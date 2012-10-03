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

#ifndef BEARERCONTEXT_H_
#define BEARERCONTEXT_H_

#include <omnetpp.h>
#include "GTPMessage_m.h"
#include "TunnelEndpoint.h"
#include "NASMessage_m.h"

enum ESMSublayerState {
	/* for UE */
	BEARER_CONTEXT_INACTIVE_U 			= FSM_Steady(0),
	BEARER_CONTEXT_ACTIVE_U				= FSM_Steady(1),
	PROCEDURE_TRANSACTION_INACTIVE_U	= FSM_Steady(2),
	PROCEDURE_TRANSACTION_PENDING_U		= FSM_Steady(3),
	/* for MME */
	BEARER_CONTEXT_INACTIVE_N 			= FSM_Steady(4),
	BEARER_CONTEXT_ACTIVE_PENDING_N		= FSM_Steady(5),
	BEARER_CONTEXT_ACTIVE_N				= FSM_Steady(6),
	BEARER_CONTEXT_INACTIVE_PENDING_N	= FSM_Steady(7),
	BEARER_CONTEXT_MODIFY_PENDING_N		= FSM_Steady(8),
	PROCEDURE_TRANSACTION_INACTIVE_N	= FSM_Steady(9),
	PROCEDURE_TRANSACTION_PENDING_N		= FSM_Steady(10)
};

enum ESMSublayerEvent {
	/* for UE */
	UEInitESMProcReq,
	ActBearerCtxtAcc,
	/* for MME and UE */
	ActBearerCtxtReq,
};

class PDNConnection;
class Subscriber;
class NasPdu;

/*
 * Class for PDN connection's bearer contexts. The bearer context will be used in
 * a number of 4G models.
 * For GTP, bearer context has an id (ebi), a QoS number (at the moment not used) and
 * GTP user tunnel end points, for each node using GTP user protocol.
 * For NAS, bearer context implements the state machine described in the specification,
 * and has also the timers and procedure ids, mentioned in the specification.
 * The bearer context will be linked to its PDN connection, which has multiple such
 * bearer contexts (relation one-to-many).
 *
 */
class BearerContext : public cPolymorphic {
private:
	unsigned char id;
	unsigned char qci; // TODO QoS

    // only user plane tunnels
    TunnelEndpoint *enbTunn;
    TunnelEndpoint *sgwTunn;
    TunnelEndpoint *pgwTunn;

	PDNConnection *ownerp;

	unsigned char procId;

	cFSM *fsm;

	cMessage *t3485;

	NasPdu *pdu;		// used for S1AP initialContextSetup
public:
	BearerContext();
	BearerContext(PDNConnection *ownerp);
	virtual ~BearerContext();

    /*
     * Method for initializing a Bearer context. Basically all parameters are set to
     * their default values.
     */
	void init();

	/*
	 * Setter methods.
	 */
	void setId(unsigned char id) { this->id = id; }
	void setENBTunnEnd(TunnelEndpoint *enbTunn) { this->enbTunn = enbTunn; }
	void setSGWTunnEnd(TunnelEndpoint *sgwTunn) { sgwTunn->setOwner(this); this->sgwTunn = sgwTunn; }
	void setPGWTunnEnd(TunnelEndpoint *pgwTunn) { this->pgwTunn = pgwTunn; }
	void setOwner(PDNConnection *esm);
	void setProcId(unsigned char procId) { this->procId = procId; }
	void setQci(unsigned char qci) { this->qci = qci; }
	void setNasPdu(NasPdu *pdu);

	/*
	 * Getter methods.
	 */
	unsigned char getId() { return id; }
	unsigned char getProcId() { return procId; }
	TunnelEndpoint *getENBTunnEnd() { return enbTunn; }
	TunnelEndpoint *getPGWTunnEnd() { return pgwTunn; }
	TunnelEndpoint *getSGWTunnEnd() { return sgwTunn; }
	PDNConnection *getOwner();
	unsigned char getQci() { return qci; }
	NasPdu *getNasPdu();
	Subscriber *getSubscriber();

    /*
     * Utility methods for state processing and printing.
     */
	void performStateTransition(ESMSublayerEvent event);
	const char *stateName(int state) const;
	const char *eventName(int event);
	void stateEntered();

	/*
	 * Methods for creating and processing of Bearer Context information element
	 * used in certain GTP messages. Process method returns true if the information
	 * element was correct or false otherwise.
	 */
	std::map<unsigned char, TunnelEndpoint*> tunnEnds;	// for message parsing
	GTPInfoElem *createBearerContextIE(unsigned char instance);
	bool processBearerContextIE(GTPInfoElem *bearerContext);

    /*
     * Method for printing information about a bearer context.
     */
	std::string info(int tabs) const;
};

#endif /* BEARERCONTEXT_H_ */
