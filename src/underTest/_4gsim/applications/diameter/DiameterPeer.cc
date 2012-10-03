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

#include "DiameterPeer.h"
#include "DiameterBase.h"
#include "DiameterUtils.h"
#include "DiameterSerializer.h"

DiameterPeer::DiameterPeer(DiameterBase *module) {
	// TODO Auto-generated constructor stub
	this->module = module;
	this->dFQDN = "";
	this->dFQDN = "";
	this->oFQDN = module->getFqdn();
	this->oRealm = module->getRealm();

	char fsmname[24];
	sprintf(fsmname, "fsm-%s", oFQDN.data());
	fsm.setName(fsmname);
	fsm.setState(CLOSED);

	iConn = NULL;
	rConn = NULL;
	tcTimer = NULL;
	startTimer(tcTimer, "TC-TIMER", TC_TIMER_TIMEOUT);
	tsTimer = NULL;
	twTimer = NULL;
	teTimer = NULL;
	appl = NULL;
	msg = NULL;
}

DiameterPeer::~DiameterPeer() {
	// TODO Auto-generated destructor stub

    // msg will be deleted at receiver end

	if (twTimer != NULL) {
		if (twTimer->getContextPointer() != NULL)
			module->cancelEvent(twTimer);
		delete twTimer;
	}
	if (tcTimer != NULL) {
		if (tcTimer->getContextPointer() != NULL)
			module->cancelEvent(tcTimer);
		delete tcTimer;
	}
	if (tsTimer != NULL) {
		if (tsTimer->getContextPointer() != NULL)
			module->cancelEvent(tsTimer);
		delete tsTimer;
	}
	if (teTimer != NULL) {
		if (teTimer->getContextPointer() != NULL)
			module->cancelEvent(teTimer);
		delete teTimer;
	}
	if (appl != NULL)
		delete appl;

	// iConn and rConn will be deleted in connection map
}

void DiameterPeer::processTimer(cMessage *msg) {

	if (msg == tcTimer) {
	   	PeerEvent event = START;
	   	performStateTransition(event, NULL);
	} else if (msg == twTimer) {
		rConn == NULL ? sendDWR(iConn) : sendDWR(rConn);
		startTimer(twTimer, "TW-TIMER", TW_TIMER_TIMEOUT + uniform(-2, 2));
	} else if (msg == tsTimer) {
		EV << "Test timer timeout\n";
		PeerEvent event = STOP;
		performStateTransition(event, NULL);
/*		AddressVector addresses;
		IPvXAddress address = IPvXAddress("192.168.1.1");
		addresses.push_back(address);
    	DiameterConnection *conn = module->createConnection(addresses, 3868);
    	DiameterPeer *peer = module->createPeer("hss.lte.test", "lte.test", conn);
		peer->startTimer(peer->tcTimer, "TC-TIMER", TC_TIMER_TIMEOUT);
    	conn->setPeer(peer);
    	*/
	} else if (msg == teTimer) {
		PeerEvent event = TIMEOUT;
		performStateTransition(event, NULL);
	} else {

	}
}

void DiameterPeer::performStateTransition(PeerEvent &event, DiameterMessage *msg) {

	int oldState = fsm.getState();

	switch(oldState) {
	case CLOSED:
		switch(event) {
			case START:
				iConn->connect();
				FSM_Goto(fsm, WAIT_CONN_ACK);
				break;
			case R_CONN_CER: {
				unsigned resCode = processCER(msg);
				sendCEA(rConn, msg, resCode);
				if ((resCode / 1000) != 2) {
					//rConn->disconnect();
					break;
				}
				FSM_Goto(fsm, R_OPEN);
				break;
			}
			default:
				EV << "DiameterPeer: Received unexpected event\n";
				if (msg != NULL)
					delete msg;
				break;
		}
		break;
	case WAIT_CONN_ACK:
		switch(event) {
			case I_RCV_CONN_ACK:
				module->cancelEvent(tcTimer);
				sendCER(iConn);
				FSM_Goto(fsm, WAIT_I_CEA);
				break;
			case I_RCV_CONN_NACK:
				cleanup();
				FSM_Goto(fsm, CLOSED);
				break;
			case R_CONN_CER: {
				unsigned resCode = processCER(msg);
				if ((resCode / 1000) != 2) {
					break;
				}
				this->msg = msg;
				FSM_Goto(fsm, WAIT_CONN_ACK_ELECT);
				break;
			}
			case TIMEOUT:
				error();
				FSM_Goto(fsm, CLOSED);
				break;
			default:
				EV << "DiameterBase: Received unexpected event\n";
				if (msg != NULL)
					delete msg;
				break;
		}
		break;
	case WAIT_I_CEA:
		switch(event) {
			case I_RCV_CEA:
				if ((processCEA(msg) / 1000) != 2)
					break;
				FSM_Goto(fsm, I_OPEN);
				break;
			case R_CONN_CER: {
				unsigned resCode = processCER(msg);
				if ((resCode / 1000) != 2) {
					break;
				}
				this->msg = msg;
				FSM_Goto(fsm, WAIT_RETURNS);
				break;
			}
			case I_PEER_DISC:
				iDisconnect();
				FSM_Goto(fsm, CLOSED);
				break;
			case I_RCV_NON_CEA: // same as timeout
			case TIMEOUT:
				error();
				FSM_Goto(fsm, CLOSED);
				break;
			default:
				EV << "DiameterPeer: Received unexpected event\n";
				if (msg != NULL)
					delete msg;
				break;
		}
		break;
	case WAIT_CONN_ACK_ELECT:
		switch(event) {
			case I_RCV_CONN_ACK:
				sendCER(iConn);
				FSM_Goto(fsm, WAIT_RETURNS);
				break;
			case I_RCV_CONN_NACK:
				sendCEA(rConn, this->msg, DIAMETER_SUCCESS);
				FSM_Goto(fsm, R_OPEN);
				break;
			case R_PEER_DISC:
				rDisconnect();
				FSM_Goto(fsm, WAIT_CONN_ACK);
				break;
			case R_CONN_CER:
				//already rejected
				FSM_Goto(fsm, WAIT_CONN_ACK);
				break;
			case TIMEOUT:
				error();
				FSM_Goto(fsm, CLOSED);
				break;
			default:
				EV << "DiameterPeer: Received unexpected event\n";
				if (msg != NULL)
					delete msg;
				break;
		}
		break;
	case WAIT_RETURNS:
		switch(event) {
			case WIN_ELECTION:
				iDisconnect();
				sendCEA(rConn, this->msg, DIAMETER_SUCCESS);
				FSM_Goto(fsm, R_OPEN);
				break;
			case I_PEER_DISC:
				iDisconnect();
				FSM_Goto(fsm, R_OPEN);
				break;
			case I_RCV_CEA:
			    rDisconnect();
                if ((processCEA(msg) / 1000) != 2)
                    break;
                delete this->msg;
                this->msg = NULL;
//				startTimer(tsTimer, "TS-TIMER", TS_TIMER_TIMEOUT);
				FSM_Goto(fsm, I_OPEN);
				break;
			case R_PEER_DISC:
				rDisconnect();
				FSM_Goto(fsm, WAIT_I_CEA);
				break;
			case R_CONN_CER:
				// already rejected
				break;
			case TIMEOUT:
				error();
				FSM_Goto(fsm, CLOSED);
				break;
			default:
				EV << "DiameterPeer: Received unexpected event\n";
				if (msg != NULL)
					delete msg;
				break;
		}
		break;
	case R_OPEN:
		switch(event) {
			case SEND_MESSAGE:
				sendApplMessage(rConn, msg);
				//stay in this state
				break;
			case R_RCV_MESSAGE:
				processApplMessage(rConn, msg);
				//remain in this state
				break;
			case R_RCV_DWR: {
				unsigned resCode = processDWR(msg);
				sendDWA(rConn, msg, resCode);
				if ((resCode / 1000) != 2)
					break;
				//resetTwTimer();
				break;
			}
			case R_RCV_DWA:
				processDWA(msg);
				break;
			case R_CONN_CER:
				// already rejected
				break;
			case STOP:
				sendDPR(rConn);
				FSM_Goto(fsm, CLOSING);
				break;
			case R_RCV_DPR:
				sendDPA(rConn, msg);
				rDisconnect();
				FSM_Goto(fsm, CLOSED);
				break;
			case R_PEER_DISC:
				rDisconnect();
				FSM_Goto(fsm, CLOSED);
				break;
			case R_RCV_CER:
				sendCEA(rConn, msg, 2000);
				break;
			case R_RCV_CEA:
				processCEA(msg);
				break;
			default:
				EV << "DiameterPeer: Received unexpected event\n";
				if (msg != NULL)
					delete msg;
				break;
		}
		break;
	case I_OPEN:
		switch(event) {
			case SEND_MESSAGE:
				sendApplMessage(iConn, msg);
				//stay in this state
				break;
			case I_RCV_MESSAGE:
				processApplMessage(iConn, msg);
				//stay in this state
				break;
			case I_RCV_DWR: {
				unsigned resCode = processDWR(msg);
				sendDWA(iConn, msg, resCode);
				if ((resCode / 1000) != 2)
					break;
				break;
			}
			case I_RCV_DWA:
				processDWA(msg);
				break;
			case R_CONN_CER:
				// already rejected
				break;
			case STOP:
				sendDPR(iConn);
				FSM_Goto(fsm, CLOSING);
				break;
			case I_RCV_DPR:
				sendDPA(iConn, msg);
				iDisconnect();
				FSM_Goto(fsm, CLOSED);
				break;
			case I_PEER_DISC:
				iDisconnect();
				FSM_Goto(fsm, CLOSED);
				break;
			case I_RCV_CER:
				sendCEA(iConn, msg, 2000);
				break;
			case I_RCV_CEA:
				processCEA(msg);
				break;
			default:
				EV << "DiameterPeer: Received unexpected event\n";
				if (msg != NULL)
					delete msg;
				break;
		}
		break;
	case CLOSING:
		switch(event) {
			case I_RCV_DPA:
				iDisconnect();
				FSM_Goto(fsm, CLOSED);
				break;
			case R_RCV_DPA:
				rDisconnect();
				FSM_Goto(fsm, CLOSED);
				break;
			case TIMEOUT:
				error();
				FSM_Goto(fsm, CLOSED);
				break;
			case I_PEER_DISC:
				iDisconnect();
				FSM_Goto(fsm, CLOSED);
				break;
			case R_PEER_DISC:
				rDisconnect();
				FSM_Goto(fsm, CLOSED);
				break;
			default:
				EV << "DiameterPeer: Received unexpected event\n";
				if (msg != NULL)
					delete msg;
				break;
		}
		break;
	default:
		EV << "DiameterPeer: Unknown state\n";
		if (msg != NULL)
			delete msg;
		break;
	}

    if (oldState != fsm.getState())
        EV << "DiameterPeer: PSM-Transition: " << stateName(oldState) << " --> " << stateName(fsm.getState()) << "  (event was: " << eventName(event) << ")\n";
    else
        EV << "DiameterPeer: Staying in state: " << stateName(fsm.getState()) << " (event was: " << eventName(event) << ")\n";

    stateEntered();
}

void DiameterPeer::sendApplMessage(DiameterConnection *conn, DiameterMessage *msg) {
	if (msg->getHdr().getReqFlag()) {
		msg->insertAvp(2, DiameterUtils().createOctetStringAVP(AVP_DestinationHost, 0, 1, 0, 0, dFQDN.size(), dFQDN.data()));
		msg->insertAvp(3, DiameterUtils().createOctetStringAVP(AVP_DestinationRealm, 0, 1, 0, 0, dRealm.size(), dRealm.data()));
	}
	conn->send(msg, 2, 3);

}
void DiameterPeer::processApplMessage(DiameterConnection *conn, DiameterMessage *msg) {
	AVP *authStMaint = msg->findAvp(AVP_AuthSessionState);
	if (authStMaint == NULL) {
		DiameterMessage *err = new DiameterMessage();
		DiameterHeader hdr = msg->getHdr();
		hdr.setReqFlag(false);
		err->setHdr(hdr);
		err->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_ResultCode, 0, 1, 0, 0, DIAMETER_MISSING_AVP));
		conn->send(err, 1, 2);
		delete msg;
		return;
	}
	int state = DiameterUtils().processInteger32AVP(authStMaint);
	if (state == NO_STATE_MAINTAINED) {
		DiameterSession *session = module->createSession(state);
		SessionEvent event = RequestReceived;
		session->performStateTransition(event, appl->applId, this, msg);
	}
}

void DiameterPeer::stateEntered() {

	switch(fsm.getState()) {
		case CLOSED:
			if (teTimer != NULL)
				module->cancelEvent(teTimer);
			if (twTimer != NULL)
				module->cancelEvent(twTimer);
			break;
		case WAIT_CONN_ACK:
		case WAIT_I_CEA:
		case WAIT_CONN_ACK_ELECT:
		case CLOSING:
			startTimer(teTimer, "TE-TIMER", TE_TIMER_TIMEOUT);
			break;
		case WAIT_RETURNS:
			startTimer(teTimer, "TE-TIMER", TE_TIMER_TIMEOUT);
			elect();
			break;
		case R_OPEN:
		case I_OPEN:
			if (teTimer != NULL)
				module->cancelEvent(teTimer);
			if (tcTimer != NULL)
			    module->cancelEvent(tcTimer);
			startTimer(twTimer, "TW-TIMER", TW_TIMER_TIMEOUT + uniform(-2, 2));
			break;
		default:;
		break;
	}
}

void DiameterPeer::sendCER(DiameterConnection *conn) {
	std::vector<AVP*> avps;
	DiameterMessage *cer = new DiameterMessage("Capabilities-Exchange-Request");
	cer->setHdr(DiameterUtils().createHeader(CapabilitiesExchange, 1, 0, 0, 0, appl->applId, uniform(0, 10000), uniform(0, 10000)));

	cer->pushAvp(DiameterUtils().createAddressAVP(AVP_HostIPAddress, 0, 1, 0, 0, module->getServerSocket().getLocalAddresses().at(0)));
	cer->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_VendorId, 0, 1, 0, 0, module->getVendorId()));
	cer->pushAvp(DiameterUtils().createUTF8StringAVP(AVP_ProductName, 0, 1, 0, 0, module->getProductName()));

	if (appl) {
		AVP *authApplId = DiameterUtils().createUnsigned32AVP(AVP_AuthApplId, 0, 1, 0, 0, appl->applId);
		avps.push_back(authApplId);
		AVP *suppVendId = DiameterUtils().createUnsigned32AVP(AVP_SuppVendorId, 0, 1, 0, 0, appl->vendorId);
		avps.push_back(suppVendId);
		AVP *vendSpecApplId = DiameterUtils().createGroupedAVP(AVP_VendorSpecApplId, 0, 1, 0, 0, avps);
		cer->pushAvp(authApplId);
		cer->pushAvp(suppVendId);
		cer->pushAvp(vendSpecApplId);
	}
    conn->send(cer, 0, 1);
}

void DiameterPeer::sendCEA(DiameterConnection *conn, DiameterMessage *cer, unsigned resCode) {
	DiameterMessage *cea = new DiameterMessage("Capabilities-Exchange-Answer");
	cea->setHdr(DiameterUtils().createHeader(CapabilitiesExchange, 0, 0, 0, 0, appl->applId, cer->getHdr().getHopByHopId(), cer->getHdr().getEndToEndId()));

	cea->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_ResultCode, 0, 1, 0, 0, resCode));
	cea->pushAvp(DiameterUtils().createAddressAVP(AVP_HostIPAddress, 0, 1, 0, 0, module->getServerSocket().getLocalAddresses().at(0)));
	cea->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_VendorId, 0, 1, 0, 0, module->getVendorId()));
	cea->pushAvp(DiameterUtils().createUTF8StringAVP(AVP_ProductName, 0, 1, 0, 0, module->getProductName()));

	if (appl) {
		std::vector<AVP*> avps;
		AVP *authApplId = DiameterUtils().createUnsigned32AVP(AVP_AuthApplId, 0, 1, 0, 0, appl->applId);
		avps.push_back(authApplId);
		AVP *suppVendId = DiameterUtils().createUnsigned32AVP(AVP_SuppVendorId, 0, 1, 0, 0, appl->vendorId);
		avps.push_back(suppVendId);
		AVP *vendSpecApplId = DiameterUtils().createGroupedAVP(AVP_VendorSpecApplId, 0, 1, 0, 0, avps);
		cea->pushAvp(authApplId);
		cea->pushAvp(suppVendId);
		cea->pushAvp(vendSpecApplId);
	}

	delete cer;
	conn->send(cea, 1, 2);
}

void DiameterPeer::sendDWR(DiameterConnection *conn) {
	DiameterMessage *dwr = new DiameterMessage("Device-Watchdog-Request");
	dwr->setHdr(DiameterUtils().createHeader(DeviceWatchdog, 1, 0, 0, 0, appl->applId, uniform(0, 10000), uniform(0, 10000)));

	conn->send(dwr, 0, 1);
}

void DiameterPeer::sendDWA(DiameterConnection *conn, DiameterMessage *dwr, unsigned resCode) {
	DiameterMessage *dwa = new DiameterMessage("Device-Watchdog-Answer");
	dwa->setHdr(DiameterUtils().createHeader(DeviceWatchdog, 0, 0, 0, 0, appl->applId, dwr->getHdr().getHopByHopId(), dwr->getHdr().getEndToEndId()));

	dwa->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_ResultCode, 0, 1, 0, 0, resCode));

	delete dwr;
	conn->send(dwa, 1, 2);
}

void DiameterPeer::sendDPR(DiameterConnection *conn) {
	DiameterMessage *dpr = new DiameterMessage("Disconnect-Peer-Request");
	dpr->setHdr(DiameterUtils().createHeader(DisconnectPeer, 1, 0, 0, 0, appl->applId, uniform(0, 10000), uniform(0, 10000)));

	dpr->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_DisconnectCause, 0, 1, 0, 0, DO_NOT_WANT_TO_TALK_TO_YOU));

	conn->send(dpr, 0, 1);
}

void DiameterPeer::sendDPA(DiameterConnection *conn, DiameterMessage *dpr) {
	DiameterMessage *dpa = new DiameterMessage("Disconnect-Peer-Answer");
	dpa->setHdr(DiameterUtils().createHeader(DisconnectPeer, 0, 0, 0, 0, appl->applId, dpr->getHdr().getHopByHopId(), dpr->getHdr().getEndToEndId()));

	dpa->pushAvp(DiameterUtils().createUnsigned32AVP(AVP_ResultCode, 0, 1, 0, 0, 2000));

	delete dpr;
	conn->send(dpa, 1, 2);
}

unsigned DiameterPeer::processCER(DiameterMessage *msg) {
	AVP *vendSpecApplId = msg->findAvp(AVP_VendorSpecApplId);
	if (vendSpecApplId != NULL) {
		AVP *authApplId = msg->findAvp(AVP_AuthApplId);
		AVP *suppVendorId = msg->findAvp(AVP_SuppVendorId);

		if ((authApplId == NULL) || (authApplId == NULL))
			return DIAMETER_MISSING_AVP;
		unsigned applId = DiameterUtils().processUnsigned32AVP(authApplId);
		unsigned vendorId = DiameterUtils().processUnsigned32AVP(suppVendorId);
		appl = new DiameterApplication(applId, vendorId);
	}
	return DIAMETER_SUCCESS;
}

unsigned DiameterPeer::processCEA(DiameterMessage *msg) {
	delete msg;
	return DIAMETER_SUCCESS;
}

unsigned DiameterPeer::processDWR(DiameterMessage *msg) {
	return DIAMETER_SUCCESS;
}

unsigned DiameterPeer::processDWA(DiameterMessage *msg) {
	delete msg;
	return DIAMETER_SUCCESS;
}

const char *DiameterPeer::stateName(int state) {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (state) {
        CASE(CLOSED);
        CASE(WAIT_CONN_ACK);
        CASE(WAIT_I_CEA);
        CASE(WAIT_CONN_ACK_ELECT);
        CASE(WAIT_RETURNS);
        CASE(R_OPEN);
        CASE(I_OPEN);
        CASE(CLOSING);
    }
    return s;
#undef CASE
}

const char *DiameterPeer::eventName(int event) {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (event) {
        CASE(START);
        CASE(R_CONN_CER);
        CASE(I_RCV_CONN_ACK);
        CASE(I_RCV_CONN_NACK);
        CASE(TIMEOUT);
        CASE(R_RCV_CER);
        CASE(I_RCV_CER);
        CASE(I_RCV_CEA);
        CASE(R_RCV_CEA);
        CASE(I_RCV_NON_CEA);
        CASE(I_PEER_DISC);
        CASE(R_PEER_DISC);
        CASE(R_RCV_DPR);
        CASE(I_RCV_DPR);
        CASE(R_RCV_DPA);
        CASE(I_RCV_DPA);
        CASE(WIN_ELECTION);
        CASE(SEND_MESSAGE);
        CASE(R_RCV_MESSAGE);
        CASE(I_RCV_MESSAGE);
        CASE(R_RCV_DWR);
        CASE(I_RCV_DWR);
        CASE(I_RCV_DWA);
        CASE(R_RCV_DWA);
        CASE(STOP);
    }
    return s;
#undef CASE
}

void DiameterPeer::startTimer(cMessage *&timer, const char *name, simtime_t timeout) {
	if (timer == NULL)
		timer = new cMessage(name);
	else
		module->cancelEvent(timer);
	module->scheduleAt(simTime() + timeout, timer);
	timer->setContextPointer(this);
}

void DiameterPeer::iDisconnect() {
	if (iConn != NULL) {
		iConn->shutdown();
		iConn = NULL;
	}
}

void DiameterPeer::rDisconnect() {
	if (rConn != NULL) {
		rConn->shutdown();
		rConn = NULL;
	}
}

void DiameterPeer::cleanup() {
	if (iConn != NULL) {
		iConn->shutdown();
		iConn->setPeer(NULL);
		iConn = NULL;
	}
	if (rConn != NULL) {
		rConn->shutdown();
		rConn->setPeer(NULL);
		rConn = NULL;
	}
}

void DiameterPeer::elect() {

	PeerEvent event;
	for (unsigned i = 0; (i < oFQDN.size()) && (i < dFQDN.size()); i++) {
		unsigned orig = ((unsigned)oFQDN[i] & 0xff);
		unsigned dest = ((unsigned)dFQDN[i] & 0xff);
		if (orig > dest) {
			event = WIN_ELECTION;
			performStateTransition(event, NULL);
			return;
		} else {
			return;
		}
	}
	if (oFQDN.size() > dFQDN.size()) {
		event = WIN_ELECTION;
		performStateTransition(event, NULL);
	}
}

const char *DiameterPeer::getTypeName(){
    if (type == STATIC)
        return "STATIC";
    else
        return "DYNAMIC";
}

int DiameterPeer::getConnectionId() {
    if (rConn == NULL)
        return iConn->getConnectionId();
    else
        return rConn->getConnectionId();
}

int DiameterPeer::getConnectionType() {
	if (iConn != NULL)
		return INITIATOR;
	else if (rConn != NULL)
		return RESPONDER;
	else
		return -1;
}

DiameterPeerTable::DiameterPeerTable() {
    // TODO Auto-generated constructor stub

}

DiameterPeerTable::~DiameterPeerTable() {
    // TODO Auto-generated destructor stub
}

DiameterPeer *DiameterPeerTable::findPeer(std::string dFQDN, std::string dRealm) {
    if (dRealm == "") {
        for (unsigned i = 0; i < peers.size(); i++) {
            DiameterPeer *peer = peers[i];
            if ((peer->dFQDN.find(dFQDN)) != std::string::npos)
                return peer;
        }
    } else {
        for (unsigned i = 0; i < peers.size(); i++) {
            DiameterPeer *peer = peers[i];
            if ((peer->dFQDN == dFQDN) && (peer->dRealm == dRealm))
                return peer;
        }
    }
    return NULL;
}

DiameterPeer *DiameterPeerTable::findPeer(unsigned applId) {
    for (unsigned i = 0; i < peers.size(); i++) {
        DiameterPeer *peer = peers[i];
        if (peer->appl->applId == applId)
            return peer;
    }
    return NULL;
}

void DiameterPeerTable::erase(unsigned start, unsigned end) {
    DiameterPeers::iterator first = peers.begin() + start;
    DiameterPeers::iterator last = peers.begin() + end;
    DiameterPeers::iterator i = first;
    for (;i != last; ++i)
        delete *i;
    peers.erase(first, last);
}

void DiameterPeerTable::print() {
    EV << "=====================================================================\n"
       << "Diameter Peer Table:\n";
    for (unsigned int i = 0; i < peers.size(); i++) {
        EV << "DiameterPeer nr. " << i << ":{\n";
        DiameterPeer *peer = peers.at(i);
        EV << "\torigFQDN:" << peer->oFQDN << endl;
        EV << "\tdestFQDN:" << peer->dFQDN << endl;
        EV << "\tstate:" << peer->stateName(peer->getState()) << endl;
        if (peer->appl) {
            EV << "\tappl:{\n";
            EV << "\t\tapplId:" << peer->appl->applId << endl;
            EV << "\t\tvendorId:" << peer->appl->vendorId << endl;
            EV << "}\n";
        }
        EV << "}\n";
    }
    EV << "=====================================================================\n";
}

