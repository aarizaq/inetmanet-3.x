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

#include "S1APConnection.h"
#include "S1AP.h"
#include "PerEncoder.h"
#include "LTEUtils.h"
#include "SCTPCommand_m.h"
#include "SCTPMessage_m.h"

S1APConnection::S1APConnection(S1AP *module) {
	// TODO Auto-generated constructor stub
	this->module = module;
	socket = NULL;
	cellId = NULL;
	plmnId = NULL;
	fsm = new cFSM();
	fsm->setName("fsm-S1AP");
	fsm->setState(S1AP_DISCONNECTED);
}

S1APConnection::~S1APConnection() {
	// TODO Auto-generated destructor stub
	if (fsm)
		delete fsm;
	// plmnId, cellId, suppTas and servGummeis deleted in S1AP
	if (socket)
		delete socket;
}

void S1APConnection::socketEstablished(int32 connId, void *yourPtr, uint64 buffer) {
	if (module->getType() == CONNECTOR) {
		S1APConnectionEvent event = SCTPEstablished;
		performStateTransition(event);
	}
}

void S1APConnection::socketDataArrived(int32 connId, void *yourPtr, cPacket *msg, bool urgent) {
	EV << "Received message on Assoc Id = " << socket->getConnectionId() << endl;
	SCTPSimpleMessage* smsg = check_and_cast<SCTPSimpleMessage*>(msg);
	char* buffer = (char*)calloc(smsg->getByteLength(), sizeof(char));
	for (unsigned i = 0; i < smsg->getByteLength(); i++)
		buffer[i] = smsg->getData(i);

	S1APPdu pdu = S1APPdu();
	pdu.decode(buffer);

	switch(pdu.getChoice()) {
		case initiatingMessage: {
			InitiatingMessage *initMsg = static_cast<InitiatingMessage*>(pdu.getValue());
			switch (initMsg->getProcedureCode()) {
				case id_S1Setup: {
					S1APConnectionEvent event = S1APRcvSetupRequest;
					performStateTransition(event, initMsg->getValue());
					break;
				}
				case id_initialUEMessage: {
//					EV << "S1AP-" << moduleName << ": Received S1AP - InitialUEMessage message\n";
					processInitialUeMessage(initMsg->getValue());
					break;
				}
				case id_InitialContextSetup: {
					processInitialContextSetupRequest(initMsg->getValue());
					break;
				}
				case id_downlinkNASTransport: {
					processDownlinkNasTransport(initMsg->getValue());
					break;
				}
				case id_uplinkNASTransport: {
					processUplinkNasTransport(initMsg->getValue());
					break;
				}
				default:
//					EV << "S1AP-" << moduleName << ": Unknown InitiatingMessage type\n";
					break;
			}
			break;
		}
		case successfulOutcome: {
			SuccessfulOutcome *succOut = static_cast<SuccessfulOutcome*>(pdu.getValue());
			switch(succOut->getProcedureCode()) {
				case id_S1Setup: {
					S1APConnectionEvent event = S1APRcvSetupResponse;
					performStateTransition(event, succOut->getValue());
					break;
				}
				case id_InitialContextSetup: {
					processInitialContextSetupResponse(succOut->getValue());
					break;
				}
				default:
//					EV << "S1AP-" << moduleName << ": Unknown InitiatingMessage type\n";
					break;
			}
			break;
		}
		case unsuccessfulOutcome: {
			UnsuccessfulOutcome *unsuccOut = static_cast<UnsuccessfulOutcome*>(pdu.getValue());
			switch(unsuccOut->getProcedureCode()) {
				case id_S1Setup: {
					S1APConnectionEvent event = S1APRcvSetupFailure;
					performStateTransition(event);
					break;
				}
				default:
//				EV << "S1AP-" << moduleName << ": Unknown InitiatingMessage type\n";
					break;
				}
			break;
		}
		default:
			EV << "Unknown S1AP-PDU message type.\n";
			break;
	}
}

void S1APConnection::socketDataNotificationArrived(int32 connId, void *yourPtr, cPacket *msg) {
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
    cPacket* cmsg = new cPacket();
    SCTPSendCommand *cmd = new SCTPSendCommand();
    cmd->setAssocId(ind->getAssocId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    delete ind;
    socket->sendNotification(cmsg);
}

void S1APConnection::socketPeerClosed(int32 connId, void *yourPtr) {

}

void S1APConnection::socketClosed(int32 connId, void *yourPtr) {

}

void S1APConnection::socketFailure(int32 connId, void *yourPtr, int32 code) {

}

void S1APConnection::performStateTransition(S1APConnectionEvent &event, OpenType *val) {
	int oldState = fsm->getState();
	switch(oldState) {
		case S1AP_DISCONNECTED:
			switch(event) {
				case SCTPEstablished:
					if (module->getType() == CONNECTOR)
						sendS1SetupRequest();
					FSM_Goto(*fsm, S1AP_PENDING);
					break;
				case S1APRcvSetupRequest: {
					ProtocolIeField *cause = processS1SetupRequest(val);
					if (cause == NULL) {
						sendS1SetupResponse();
						FSM_Goto(*fsm, S1AP_CONNECTED);
					} else {
						sendS1SetupFailure(cause);
						// waiting for other node to cancel this connection
					}
					break;
				}
				default:
					EV << "S1AP: Received unexpected event\n";
					break;
			}
			break;
		case S1AP_PENDING:
			switch(event) {
				case S1APRcvSetupResponse: {
					ProtocolIeField *cause = processS1SetupResponse(val);
					if (cause == NULL) {
						FSM_Goto(*fsm, S1AP_CONNECTED);
					} else {
						/* TODO */
						// close socket and erase itself
						FSM_Goto(*fsm, S1AP_DISCONNECTED);
					}
					break;
				}
				case S1APRcvSetupFailure:
					/* TODO */
					// close socket and erase itself
					FSM_Goto(*fsm, S1AP_DISCONNECTED);
					break;
				default:
					EV << "S1AP: Received unexpected event\n";
					break;
			}
			break;
		default:
			EV << "S1AP: Unknown state\n";
			break;
	}

    if (oldState != fsm->getState())
        EV << "S1AP: PSM-Transition: " << stateName(oldState) << " --> " << stateName(fsm->getState()) << "  (event was: " << eventName(event) << ")\n";
    else
        EV << "S1AP: Staying in state: " << stateName(fsm->getState()) << " (event was: " << eventName(event) << ")\n";

}


void S1APConnection::send(S1APPdu *msg) {
	cPacket* cmsg = new cPacket();
	SCTPSimpleMessage* smsg = new SCTPSimpleMessage();
	PerEncoder perEnc = PerEncoder(ALIGNED);
	msg->encode(perEnc);

	smsg->setDataArraySize(perEnc.getLength());

    for (unsigned i = 0; i < perEnc.getLength(); i++)
    	smsg->setData(i, perEnc.getByteAt(i));

    smsg->setDataLen(perEnc.getLength());
    smsg->setByteLength(perEnc.getLength());

    cmsg->encapsulate(smsg);
    cmsg->setKind(SCTP_C_SEND);

    //EV << "Diameter-" << moduleName << ": Sending Diameter Packet '" << cmsg->getName() << "' to lower layer\n";
    delete msg;		// all its content is in SCTPSimpleMessage
    socket->send(cmsg, true, true, 18);
}

void S1APConnection::sendS1SetupRequest() {
	S1APPdu *pdu = new S1APPdu();
	ProtocolIeContainer *container = new ProtocolIeContainer();

	/* Global-ENB-ID */
	container->push_back(new ProtocolIeField(id_Global_ENB_ID, reject, new GlobalEnbId(module->getPlmnId(), homeEnbId, cellId)));

	/* ENBname */
	if (!module->getName().empty())
		container->push_back(new ProtocolIeField(id_eNBname, ignore, new EnbName(module->getName())));

	/* SupportedTAs */
	container->push_back(new ProtocolIeField(id_SupportedTAs, reject, toSupportedTas(suppTas)));

	/* DefaultPagingDRX */
	container->push_back(new ProtocolIeField(id_DefaultPagingDRX, ignore, new PagingDrx(module->getPagingDrx())));

	InitiatingMessage *initMsg = new InitiatingMessage(id_S1Setup, reject, new S1APProcedure(container));
	pdu->setValue(initMsg, initiatingMessage);
	send(pdu);
}

ProtocolIeField *S1APConnection::processS1SetupRequest(OpenType *val) {
	if (val->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	S1APProcedure s1setupReq = S1APProcedure();
	s1setupReq.decode(val->getValue());
	ProtocolIeContainer *container = s1setupReq.getContainer();
	OpenType *tmp;

	/* Global-ENB-ID */
	tmp = findValue(container, id_Global_ENB_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	GlobalEnbId globalEnbId = GlobalEnbId();
	globalEnbId.decode(tmp->getValue());
	plmnId = globalEnbId.getPlmnIdentity();
	cellId = globalEnbId.getEnbId()->getId();
	/* TODO  must add some chars for HomeEnbId */

	/* SupportedTAs */
	tmp = findValue(container, id_SupportedTAs);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	SupportedTas supportedTas = SupportedTas();
	supportedTas.decode(tmp->getValue());
	suppTas = fromSupportedTas(&supportedTas);

	return NULL;
}

void S1APConnection::sendS1SetupResponse() {
	S1APPdu *pdu = new S1APPdu();
	ProtocolIeContainer *container = new ProtocolIeContainer();

	/* MMEname */
	if (!module->getName().empty())
		container->push_back(new ProtocolIeField(id_MMEname, ignore, new MmeName(module->getName())));

	/* ServedGUMMEIs */
	container->push_back(new ProtocolIeField(id_ServedGUMMEIs, reject, toServedGummeis(servGummeis)));

	/* RelativeMMECapacity */
	container->push_back(new ProtocolIeField(id_RelativeMMECapacity, ignore, new RelativeMmeCapacity(module->getRelMmeCapac())));

	SuccessfulOutcome *succOut = new InitiatingMessage(id_S1Setup, reject, new S1APProcedure(container));
	pdu->setValue(succOut, successfulOutcome);
	send(pdu);
}

ProtocolIeField *S1APConnection::processS1SetupResponse(OpenType *val) {
	if (val->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	S1APProcedure s1setupRes = S1APProcedure();
	s1setupRes.decode(val->getValue());
	ProtocolIeContainer *container = s1setupRes.getContainer();
	OpenType *tmp;

	/* ServedGUMMEIs */
	tmp = findValue(container, id_ServedGUMMEIs);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	ServedGummeis servedGummeis = ServedGummeis();
	servedGummeis.decode(tmp->getValue());
	servGummeis = fromServedGummeis(&servedGummeis);

	return NULL;
}

void S1APConnection::sendS1SetupFailure(ProtocolIeField *cause) {
	S1APPdu *pdu = new S1APPdu();
	ProtocolIeContainer *container = new ProtocolIeContainer();

	/* Cause */
	container->push_back(cause);

	UnsuccessfulOutcome *unsuccessfulOut = new UnsuccessfulOutcome(id_S1Setup, reject, new S1APProcedure(container));
	pdu->setValue(unsuccessfulOut, unsuccessfulOutcome);
	send(pdu);
}

void S1APConnection::sendInitialUeMessage(Subscriber *sub, NasPdu *nasPdu) {
	S1APPdu *pdu = new S1APPdu();
	ProtocolIeContainer *container = new ProtocolIeContainer();

	/* ENB-UE-S1AP-ID */
	container->push_back(new ProtocolIeField(id_eNB_UE_S1AP_ID, reject, new EnbUeS1apId(sub->getEnbId())));

	/* NAS-PDU */
	container->push_back(new ProtocolIeField(id_NAS_PDU, reject, nasPdu));

	/* EUTRAN-CGI */
	container->push_back(new ProtocolIeField(id_EUTRAN_CGI, ignore, new EutranCgi(plmnId, cellId)));
//	sub->setPlMNId(plmnId);
	sub->setCellId(cellId);

	/* TAI */
	container->push_back(new ProtocolIeField(id_TAI, reject, new Tai(plmnId, suppTas[0].tac)));
	sub->setTac(suppTas[0].tac);

	/* RRC-Establishment-Cause */
	container->push_back(new ProtocolIeField(id_RRC_Establishment_Cause, ignore, new RrcEstablishmentCause(mo_Signalling)));

	InitiatingMessage *initMsg = new InitiatingMessage(id_initialUEMessage, reject, new S1APProcedure(container));
	pdu->setValue(initMsg, initiatingMessage);
	send(pdu);
}

ProtocolIeField *S1APConnection::processInitialUeMessage(OpenType *val) {
	if (val->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	S1APProcedure initUeMsg = S1APProcedure();
	initUeMsg.decode(val->getValue());
	Subscriber *sub = new Subscriber();
	ProtocolIeContainer *container = initUeMsg.getContainer();
	OpenType *tmp;

	/* ENB-UE-S1AP-ID */
	tmp = findValue(container, id_eNB_UE_S1AP_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		delete sub;
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	EnbUeS1apId enbUeS1apId = EnbUeS1apId();
	enbUeS1apId.decode(tmp->getValue());
	sub->setEnbId(enbUeS1apId.getValue());

	/* NAS-PDU */
	tmp = findValue(container, id_NAS_PDU);
	if (tmp == NULL || tmp->getValue() == NULL) {
		delete sub;
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	NasPdu *nasPdu = new NasPdu();
	nasPdu->decode(tmp->getValue());

	/* EUTRAN-CGI */
	tmp = findValue(container, id_EUTRAN_CGI);
	if (tmp == NULL || tmp->getValue() == NULL) {
		delete sub;
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	EutranCgi eutranCgi = EutranCgi();
	eutranCgi.decode(tmp->getValue());
	sub->setPlmnId(eutranCgi.getPlmnIdentity());
	sub->setCellId(eutranCgi.getCellIdentity());

	/* TAI */
	tmp = findValue(container, id_TAI);
	if (tmp == NULL || tmp->getValue() == NULL) {
		delete sub;
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	Tai tai = Tai();
	tai.decode(tmp->getValue());
	sub->setTac(tai.getTac());

	module->subT->push_back(sub);
	module->sendMessageUp(sub, nasPdu);
	return NULL;
}

void S1APConnection::sendInitialContextSetupRequest(Subscriber *sub, NasPdu *nasPdu) {
	S1APPdu *pdu = new S1APPdu();
	ProtocolIeContainer *container = new ProtocolIeContainer();

	/* MME-UE-S1AP-ID */
	container->push_back(new ProtocolIeField(id_MME_UE_S1AP_ID, reject, new MmeUeS1apId(sub->getMmeId())));

	/* ENB-UE-S1AP-ID */
	container->push_back(new ProtocolIeField(id_eNB_UE_S1AP_ID, reject, new EnbUeS1apId(sub->getEnbId())));

	/* TODO UEAggregateMaximumBitrate */
	container->push_back(new ProtocolIeField(id_uEaggregateMaximumBitrate, reject, new UEAggregateMaximumBitrate(100, 100)));

	/* E-RABToBeSetupListCtxtSUReq */
	std::vector<BearerContext*>bearers;
	BearerContext *defBearer = sub->getEsmEntity()->getDefPDNConnection()->getDefaultBearer();
	defBearer->setNasPdu(nasPdu);
	bearers.push_back(defBearer);
	container->push_back(new ProtocolIeField(id_E_RABToBeSetupListCtxtSUReq, reject, toERabToBeSetupListCtxtSuReq(bearers)));

	/* TODO UESecurityCapabilities */
	container->push_back(new ProtocolIeField(id_UESecurityCapabilities, reject, new UESecurityCapabilities((char*)calloc(2, sizeof(char)), 16, (char*)calloc(2, sizeof(char)), 16)));

	/* TODO SecurityKey */
	container->push_back(new ProtocolIeField(id_SecurityKey, reject, new SecurityKey((char*)calloc(32, sizeof(char)))));

	InitiatingMessage *initMsg = new InitiatingMessage(id_InitialContextSetup, reject, new S1APProcedure(container));
	pdu->setValue(initMsg, initiatingMessage);
	send(pdu);
}

ProtocolIeField *S1APConnection::processInitialContextSetupRequest(OpenType *val) {
	if (val->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	S1APProcedure initCtxtSet = S1APProcedure();
	initCtxtSet.decode(val->getValue());
	ProtocolIeContainer *container = initCtxtSet.getContainer();
	OpenType *tmp;

	/* ENB-UE-S1AP-ID */
	tmp = findValue(container, id_eNB_UE_S1AP_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	EnbUeS1apId enbUeS1apId = EnbUeS1apId();
	enbUeS1apId.decode(tmp->getValue());

	/* MME-UE-S1AP-ID */
	tmp = findValue(container, id_MME_UE_S1AP_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	MmeUeS1apId mmeUeS1apId = MmeUeS1apId();
	mmeUeS1apId.decode(tmp->getValue());
	Subscriber *sub = module->subT->findSubscriberForId(enbUeS1apId.getValue(), mmeUeS1apId.getValue());
	if (sub == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeRadioNetwork, unknown_enb_ue_s1ap_id));
	}

	/* E-RABToBeSetupListCtxtSUReq */
	tmp = findValue(container, id_E_RABToBeSetupListCtxtSUReq);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	ERabToBeSetupListCtxtSuReq *eRabToBeSetupListCtxtSuReq = new ERabToBeSetupListCtxtSuReq();
	eRabToBeSetupListCtxtSuReq->decode(tmp->getValue());
	PDNConnection *conn = sub->getDefaultPDNConn();
	std::vector<BearerContext*> bearers = fromERabToBeSetupListCtxtSuReq(eRabToBeSetupListCtxtSuReq);
	conn->addBearerContext(bearers[0], true);
	module->fireChangeNotification(NF_SUB_NEEDS_TUNN, bearers[0]);

	if (bearers[0]->getNasPdu() != NULL) {
		module->sendMessageUp(sub, bearers[0]->getNasPdu());
	}

	return NULL;
}

void S1APConnection::sendInitialContextSetupResponse(Subscriber *sub) {
	S1APPdu *pdu = new S1APPdu();
	ProtocolIeContainer *container = new ProtocolIeContainer();

	/* MME-UE-S1AP-ID */
	container->push_back(new ProtocolIeField(id_MME_UE_S1AP_ID, reject, new MmeUeS1apId(sub->getMmeId())));

	/* ENB-UE-S1AP-ID */
	container->push_back(new ProtocolIeField(id_eNB_UE_S1AP_ID, reject, new EnbUeS1apId(sub->getEnbId())));

	/* E-RABSetupListCtxtSURes */
	std::vector<BearerContext*>bearers;
	BearerContext *defBearer = sub->getEsmEntity()->getDefPDNConnection()->getDefaultBearer();
	bearers.push_back(defBearer);
	container->push_back(new ProtocolIeField(id_E_RABSetupListCtxtSURes, reject, toERABSetupListCtxtSURes(bearers)));

	SuccessfulOutcome *succOut = new SuccessfulOutcome(id_InitialContextSetup, reject, new S1APProcedure(container));
	pdu->setValue(succOut, successfulOutcome);
	send(pdu);
}

ProtocolIeField *S1APConnection::processInitialContextSetupResponse(OpenType *val) {
	if (val->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	S1APProcedure initCtxtSet = S1APProcedure();
	initCtxtSet.decode(val->getValue());
	ProtocolIeContainer *container = initCtxtSet.getContainer();
	OpenType *tmp;

	/* ENB-UE-S1AP-ID */
	tmp = findValue(container, id_eNB_UE_S1AP_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	EnbUeS1apId enbUeS1apId = EnbUeS1apId();
	enbUeS1apId.decode(tmp->getValue());

	/* MME-UE-S1AP-ID */
	tmp = findValue(container, id_MME_UE_S1AP_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	MmeUeS1apId mmeUeS1apId = MmeUeS1apId();
	mmeUeS1apId.decode(tmp->getValue());
	Subscriber *sub = module->subT->findSubscriberForId(enbUeS1apId.getValue(), mmeUeS1apId.getValue());
	if (sub == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeRadioNetwork, unknown_enb_ue_s1ap_id));
	}

	/* E-RABSetupListCtxtSURes */
	tmp = findValue(container, id_E_RABSetupListCtxtSURes);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	ERABSetupListCtxtSURes *eRABSetupListCtxtSURes = new ERABSetupListCtxtSURes();
	PDNConnection *conn = sub->getDefaultPDNConn();
	eRABSetupListCtxtSURes->decode(tmp->getValue());
	std::vector<BearerContext*> bearers = fromERABSetupListCtxtSURes(eRABSetupListCtxtSURes);
	TunnelEndpoint *enbTe = conn->getDefaultBearer()->getENBTunnEnd();
	enbTe->setLocalId(bearers[0]->getENBTunnEnd()->getLocalId());
	enbTe->getPath()->setLocalAddr(bearers[0]->getENBTunnEnd()->getLocalAddr());

	module->fireChangeNotification(NF_SUB_MODIF_TUNN, conn->getDefaultBearer());

	return NULL;
}

void S1APConnection::sendDownlinkNasTransport(Subscriber *sub, NasPdu *nasPdu) {
	S1APPdu *pdu = new S1APPdu();
	ProtocolIeContainer *container = new ProtocolIeContainer();

	/* MME-UE-S1AP-ID */
	container->push_back(new ProtocolIeField(id_MME_UE_S1AP_ID, reject, new MmeUeS1apId(sub->getMmeId())));

	/* ENB-UE-S1AP-ID */
	container->push_back(new ProtocolIeField(id_eNB_UE_S1AP_ID, reject, new EnbUeS1apId(sub->getEnbId())));

	/* NAS-PDU */
	container->push_back(new ProtocolIeField(id_NAS_PDU, reject, nasPdu));

	InitiatingMessage *initMsg = new InitiatingMessage(id_downlinkNASTransport, reject, new S1APProcedure(container));
	pdu->setValue(initMsg, initiatingMessage);
	send(pdu);
}

ProtocolIeField *S1APConnection::processDownlinkNasTransport(OpenType *val) {
	if (val->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	S1APProcedure uplNasTrans = S1APProcedure();
	uplNasTrans.decode(val->getValue());
	ProtocolIeContainer *container = uplNasTrans.getContainer();
	OpenType *tmp;

	/* ENB-UE-S1AP-ID */
	tmp = findValue(container, id_eNB_UE_S1AP_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	EnbUeS1apId enbUeS1apId = EnbUeS1apId();
	enbUeS1apId.decode(tmp->getValue());

	/* MME-UE-S1AP-ID */
	tmp = findValue(container, id_MME_UE_S1AP_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	MmeUeS1apId mmeUeS1apId = MmeUeS1apId();
	mmeUeS1apId.decode(tmp->getValue());
	Subscriber *sub = module->subT->findSubscriberForId(enbUeS1apId.getValue(), mmeUeS1apId.getValue());
	if (sub == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeRadioNetwork, unknown_enb_ue_s1ap_id));
	}

	/* NAS-PDU */
	tmp = findValue(container, id_NAS_PDU);
	if (tmp == NULL || tmp->getValue() == NULL) {
//		delete sub;
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	NasPdu *nasPdu = new NasPdu();
	nasPdu->decode(tmp->getValue());
	module->sendMessageUp(sub, nasPdu);

	return NULL;
}

void S1APConnection::sendUplinkNasTransport(Subscriber *sub, NasPdu *nasPdu) {
	S1APPdu *pdu = new S1APPdu();
	ProtocolIeContainer *container = new ProtocolIeContainer();

	/* MME-UE-S1AP-ID */
	container->push_back(new ProtocolIeField(id_MME_UE_S1AP_ID, reject, new MmeUeS1apId(sub->getMmeId())));

	/* ENB-UE-S1AP-ID */
	container->push_back(new ProtocolIeField(id_eNB_UE_S1AP_ID, reject, new EnbUeS1apId(sub->getEnbId())));

	/* NAS-PDU */
	container->push_back(new ProtocolIeField(id_NAS_PDU, ignore, nasPdu));

	/* EUTRAN-CGI */
	container->push_back(new ProtocolIeField(id_EUTRAN_CGI, ignore, new EutranCgi(plmnId, cellId)));

	/* TAI */
	container->push_back(new ProtocolIeField(id_TAI, reject, new Tai(plmnId, sub->getTac())));

	InitiatingMessage *initMsg = new InitiatingMessage(id_uplinkNASTransport, reject, new S1APProcedure(container));
	pdu->setValue(initMsg, initiatingMessage);
	send(pdu);
}

ProtocolIeField *S1APConnection::processUplinkNasTransport(OpenType *val) {
	if (val->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	S1APProcedure uplNasTrans = S1APProcedure();
	uplNasTrans.decode(val->getValue());
	ProtocolIeContainer *container = uplNasTrans.getContainer();
	OpenType *tmp;

	/* ENB-UE-S1AP-ID */
	tmp = findValue(container, id_eNB_UE_S1AP_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	EnbUeS1apId enbUeS1apId = EnbUeS1apId();
	enbUeS1apId.decode(tmp->getValue());

	/* MME-UE-S1AP-ID */
	tmp = findValue(container, id_MME_UE_S1AP_ID);
	if (tmp == NULL || tmp->getValue() == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	MmeUeS1apId mmeUeS1apId = MmeUeS1apId();
	mmeUeS1apId.decode(tmp->getValue());
	Subscriber *sub = module->subT->findSubscriberForId(enbUeS1apId.getValue(), mmeUeS1apId.getValue());
	if (sub == NULL) {
		return new ProtocolIeField(id_Cause, reject, new Cause(causeRadioNetwork, unknown_enb_ue_s1ap_id));
	}

	/* NAS-PDU */
	tmp = findValue(container, id_NAS_PDU);
	if (tmp == NULL || tmp->getValue() == NULL) {
//		delete sub;
		return new ProtocolIeField(id_Cause, reject, new Cause(causeProtocol, abstract_syntax_error_reject));
	}
	NasPdu *nasPdu = new NasPdu();
	nasPdu->decode(tmp->getValue());
	module->sendMessageUp(sub, nasPdu);

	return NULL;
}

const char *S1APConnection::stateName(int state) {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (state) {
        CASE(S1AP_CONNECTED);
        CASE(S1AP_PENDING);
        CASE(S1AP_DISCONNECTED);
    }
    return s;
#undef CASE
}

const char *S1APConnection::eventName(int event) {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (event) {
        CASE(SCTPEstablished);
        CASE(S1APRcvSetupRequest);
        CASE(S1APRcvSetupResponse);
        CASE(S1APRcvSetupFailure);
    }
    return s;
#undef CASE
}

S1APConnectionTable::S1APConnectionTable() {
    // TODO Auto-generated constructor stub

}

S1APConnectionTable::~S1APConnectionTable() {
    // TODO Auto-generated destructor stub
    erase(0, conns.size());
}

S1APConnection *S1APConnectionTable::findConnectionForId(int connId) {
    for (unsigned i = 0; i < conns.size(); i++) {
        S1APConnection *conn = conns[i];
        if (conn->getConnectionId() == connId)
            return conn;
    }
    return NULL;
}

S1APConnection *S1APConnectionTable::findConnectionForCellId(char *cellId) {
    if (cellId == NULL)
        return NULL;
    for (unsigned i = 0; i < conns.size(); i++) {
        S1APConnection *conn = conns[i];
        if (!strncmp(conn->getCellId(), cellId, CELLID_CODED_SIZE))
            return conn;
    }
    return NULL;
}

S1APConnection *S1APConnectionTable::findConnectionForState(int state) {
    for (unsigned i = 0; i < conns.size(); i++) {
        S1APConnection *conn = conns[i];
        if (conn->getState() == state)
            return conn;
    }
    return NULL;
}

void S1APConnectionTable::erase(unsigned start, unsigned end) {
    S1APConnections::iterator first = conns.begin() + start;
    S1APConnections::iterator last = conns.begin() + end;
    S1APConnections::iterator i = first;
    for (;i != last; ++i)
        delete *i;
    conns.erase(first, last);
}
