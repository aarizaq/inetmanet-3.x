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

#ifndef EMMENTITY_H_
#define EMMENTITY_H_

#include <omnetpp.h>
#include "NASMessage_m.h"
#include "PDNConnection.h"

#define T3410_TIMEOUT	20

class NAS;
class Subscriber;

enum EMMSublayerState {
	/* for UE */
	EMM_NULL								= FSM_Steady(0),
	EMM_DEREGISTERED_U						= FSM_Steady(1),
	EMM_DEREGISTERED_NORMAL_SERVICE			= FSM_Steady(2),
	EMM_DEREGISTERED_LIMITED_SERVICE		= FSM_Steady(3),
	EMM_DEREGISTERED_ATTEMPTING_TO_ATTACH	= FSM_Steady(4),
	EMM_DEREGISTERED_PLMN_SEARCH			= FSM_Steady(5),
	EMM_DEREGISTERED_NO_IMSI				= FSM_Steady(6),
	EMM_DEREGISTERED_ATTACH_NEEDED			= FSM_Steady(7),
	EMM_DEREGISTERED_NO_CELL_AVAILABLE		= FSM_Steady(8),
	EMM_REGISTERED_U						= FSM_Steady(9),
	EMM_REGISTERED_INITIATED 				= FSM_Steady(10),
	EMM_REGISTERED_NORMAL_SERVICE			= FSM_Steady(11),
	EMM_REGISTERED_ATTEMPTING_TO_UPDATE		= FSM_Steady(12),
	EMM_REGISTERED_LIMITED_SERVICE			= FSM_Steady(13),
	EMM_REGISTERED_PLMN_SEARCH				= FSM_Steady(14),
	EMM_REGISTERED_UPDATE_NEEDED			= FSM_Steady(15),
	EMM_REGISTERED_NO_CELL_AVAILABLE		= FSM_Steady(16),
	EMM_REGISTERED_ATTEMPTING_TO_UPDATE_MM	= FSM_Steady(17),
	EMM_REGISTERED_IMSI_DETACH_INITIATED	= FSM_Steady(18),
	EMM_DEREGISTERED_INITIATED_U 			= FSM_Steady(19),
	EMM_TRACKING_AREA_UPDATING_INITIATED 	= FSM_Steady(20),
	EMM_SERVICE_REQUEST_INITIATED 			= FSM_Steady(21),
	/* for MME */
	EMM_DEREGISTERED_N						= FSM_Steady(22),
	EMM_COMMON_PROCEDURE_INITIATED			= FSM_Steady(23),
	EMM_REGISTERED_N						= FSM_Steady(24),
	EMM_DEREGISTERED_INITIATED_M			= FSM_Steady(25)
};

enum EMMSublayerEvent {
	/* for UE */
	EnableS1Mode,
	SwitchOn,
	NoUSIM,
	CellFoundPermittedPLMN,
	AttachRequested,
	/* for MME and UE */
	AttachAccepted,
	AttachRejected,
	/* for MME */
	AttachCompleted,
};

enum EPSUpdateStatus {
	EU1_UPDATED,
	EU2_NOT_UPDATED,
	EU3_ROAMING_NOT_ALLOWED
};

/*
 * Class for NAS EMM entity.
 * NAS EMM entity handles user equipment mobility for NAS protocol. Because of
 * that its owner is a subscriber object, but it is also related to NAS module.
 * For NAS mobility, EMM entity implements the state machine described in the
 * specification, together with the necessary timers.
 * All mobility related NAS messages are created and processed by such an
 * EMM entity.
 * EMM entity has to be also linked to ESM entity, because the specification
 * describes certain interworking such as message piggybacking.
 */
class EMMEntity : public cPolymorphic {
private:
	unsigned char attCounter;
	unsigned char tracAreaUpdCounter;

	// used for message creating and processing
	unsigned char appType;
	unsigned char nasKey;
	char emmCause;
	char *imsi;
	unsigned tmsi;
	char *ueNetworkCapability;

	cFSM *fsm;

	char epsUpdSt;

	cMessage *t3402;
	cMessage *t3410;
	cMessage *t3411;
	cMessage *t3450;

	NAS *module;

	Subscriber *ownerp;

	ESMEntity *peer;
public:
	EMMEntity();
	EMMEntity(unsigned char appType);
	virtual ~EMMEntity();

    /*
     * Method for initializing a EMM entity. Basically all parameters are set to
     * their default values.
     */
	void init();

	/*
	 * Setter methods.
	 */
	void setOwner(Subscriber *ownerp);
	void setModule(NAS *module);
	void setImsi(char *imsi) { this->imsi = imsi; }
	void setTmsi(unsigned tmsi) { this->tmsi = tmsi; }
	void setPeer(ESMEntity *peer);
	void setEmmCause(int emmCause) { this->emmCause = emmCause; }

	/*
	 * Getter methods.
	 */
	unsigned char getAppType() { return appType; }
	NAS *getModule();
	Subscriber *getOwner();
	char *getImsi() { return imsi; }
	unsigned getTmsi() { return tmsi; }
	ESMEntity *getPeer();
    int getState() { return fsm->getState(); }
    cFSM *getFSM() { return fsm; }

    /*
     * Utility methods for state processing and printing.
     */
	void performStateTransition(EMMSublayerEvent event);
	void stateEntered();
	const char *stateName(int state) const;
	const char *statusName() const;
	const char *eventName(int event);


    /*
     * Methods for processing and creating of NAS messages, which are related to
     * EMM entity. These messages are related to user equipment mobility.
     */
	NASPlainMessage *createAttachRequest();
	NASPlainMessage *createAttachAccept();
	NASPlainMessage *createAttachReject();
	NASPlainMessage *createAttachComplete();
	void processAttachRequest(NASPlainMessage *msg);
	void processAttachAccept(NASPlainMessage *msg);
	void processAttachReject(NASPlainMessage *msg);
	void processAttachComplete(NASPlainMessage *msg);

	/*
	 * Method for starting T3410 timer. This will cancel timers T3402 and T3411.
	 */
	void startT3410();

    /*
     * Method for printing information about EMM entity for a subscriber.
     */
	std::string info(int tabs) const;
};

#endif /* EMMENTITY_H_ */
