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

#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

#include "NotificationBoard.h"
#include "TunnelEndpoint.h"
#include "EMMEntity.h"
#include "ESMEntity.h"

enum SubscriberStatus {
	SUB_INACTIVE,
	SUB_PENDING,
	SUB_ACTIVE
};

enum SubscriberGTPProcedure {
	NoProcedure = 0,
	EUTRANInitAttachReq = 2,
	EUTRANInitAttachRes = 3,
};

/*
 * Class that describes a 4G subscriber.
 *
 * The subscriber object stores information about the channel number for the
 * particular UE. This channel number will be used for LTE radio model.
 *
 * From the information regarding S1AP protocol, the most important are eNB id
 * and MME id, based on them the subscriber will be identified in eNB and MME
 * subscriber tables. Other parameters serve mainly for message creation purposes.
 * More info about S1AP protocol implementation can be found in the source files
 * of s1ap directory.
 *
 * S11 tunnel endpoint is the endpoint for the GTP control tunnel in one network
 * entity for this subscriber. All GTP control messages between MME and S-GW will
 * be exchanged through this tunnel. More info about GTP protocol and tunnels can be
 * found in the source files of gtp directory.
 *
 * NAS protocol from subscriber point of view is divided in two entities, ESM and EMM.
 * EMM entity will manage the subscriber mobility and ESM entity will manage its
 * IP sessions. Both entities are linked to NAS model. EMM entity implements the mobility
 * state machine and ESM entity holds all the PDN connections for the subscriber. More
 * info about the entities can be found in the source files of nas directory.
 *
 * Sequence number list holds message sequence numbers, arrived in different modules,
 * the last message in the list being the latest. The list can hold sequence numbers
 * from GTP messages, Diameter messages etc.
 *
 * GTP procedure number holds an identifier of the last GTP procedure initiated in the
 * GTP module. This number is important because message processing in this module is
 * done based on this number.
 *
 * Status of the subscriber is a mini implementation of a state machine. Initially the
 * subscriber will be in INACTIVE state, in which it has no information stored, it will
 * go to PENDING if some operations are done upon it and will become ACTIVE if some
 * conditions are fulfilled. The ACTIVE transition is done in various modules, for more
 * info consult the sources for GTP, S1AP and SubscriberTable. If the subscriber stays
 * too much outside ACTIVE state it will be cleaned.
 */
class Subscriber : public cPolymorphic {
private:
    // radio info
    int channelNr;

    // s1ap info
	unsigned enbId;
	unsigned mmeId;
	char *plmnId;
	char *cellId;
	char *tac;
	char *mmeCode;
	char *mmeGrId;

	// diameter/gtp info
	char *msisdn;
	char ratType;

	// control tunnel over S11 interface
	TunnelEndpoint *s11Tunn;

	// nas entities
	EMMEntity *emm;
	ESMEntity *esm;

    // message sequence numbers (multiple protocols)
    std::list<unsigned> seqNrs;

	// for gtp message processing
	unsigned char gtpProc;

    // for cleaning the subscriber
    char status;
public:
	Subscriber();
	virtual ~Subscriber();

	/*
	 * Method for initiating NAS entities for the subscriber.
	 */
	void initEntities(unsigned char appType);

	/*
	 * Setter methods.
	 */
	void setEnbId(unsigned enbId) { this->enbId = enbId; }
	void setMmeId(unsigned mmeId) { this->mmeId = mmeId; }
	void setMmeGroupId(char *mmeGrId) { this->mmeGrId = mmeGrId; }
	void setMmeCode(char *mmeCode) { this->mmeCode = mmeCode; }
	void setChannelNr(unsigned channelNr) { this->channelNr = channelNr; }
	void setImsi(char *imsi) { emm->setImsi(imsi); }
	void setCellId(char *cellId) { this->cellId = cellId; }
	void setPlmnId(char *plmnId) { this->plmnId = plmnId; }
	void setTac(char *tac) { this->tac = tac; }
	void setMsisdn(char *msisdn) { this->msisdn = msisdn; }
	void setS11TunnEnd(TunnelEndpoint *s11Tunn) { s11Tunn->setOwner(this); this->s11Tunn = s11Tunn; }
	void setEmmEntity(EMMEntity *emm) { this->emm = emm; }
	void setEsmEntity(ESMEntity *esm) { this->esm = esm; }
	void setStatus(char status) { this->status = status; }
	void setGTPProcedure(unsigned gtpProc) { this->gtpProc = gtpProc; }

	/*
	 * Getter methods.
	 */
	unsigned getEnbId() { return enbId; }
	unsigned getMmeId() { return mmeId; }
	char *getMmeCode() { return mmeCode; }
	char *getMmeGroupId() { return mmeGrId; }
	int getChannelNr() { return channelNr; }
	char *getCellId() { return cellId; }
	char *getPlmnId() { return plmnId; }
	char *getTac() { return tac; }
	char *getMsisdn() { return msisdn; }
	char getRatType() { return ratType; }
	TunnelEndpoint *getS11TunnEnd() { return s11Tunn; }
	EMMEntity *getEmmEntity() { return emm; }
	ESMEntity *getEsmEntity() { return esm; }
	virtual const char *getName() const  {return "Sub";}
	PDNConnection *getDefaultPDNConn() { return esm->getDefPDNConnection(); }
	char getStatus() { return status; }
	const char *statusName() const;
	unsigned char getGTPProcedure() { return gtpProc; }

	/*
	 * Methods for managing the message sequence numbers stored for the subscriber.
	 */
	void pushSeqNr(unsigned seqNr) { seqNrs.push_back(seqNr); }
	unsigned popSeqNr();
	unsigned backSeqNr() { return seqNrs.back(); }

	/*
	 * Method for printing information about a specific subscriber. This method
	 * will be called in the graphical user interface if the user wants to get
	 * details about the subscriber. For different entities this method will
	 * generate different output.
	 */
	std::string info() const;

};

#endif /* SUBSCRIBER_H_ */
