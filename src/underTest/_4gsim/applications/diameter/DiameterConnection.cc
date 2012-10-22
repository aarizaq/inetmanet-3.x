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

#include "DiameterConnection.h"
#include "DiameterPeer.h"
#include "DiameterBase.h"
#include "DiameterUtils.h"
#include "DiameterSerializer.h"
#include "SCTPCommand_m.h"

DiameterConnection::DiameterConnection(DiameterBase *module) {
	// TODO Auto-generated constructor stub
	this->module = module;
	peer = NULL;
	ignore = true;
	ordered = true;
}

DiameterConnection::~DiameterConnection() {
	// TODO Auto-generated destructor stub
}

void DiameterConnection::socketEstablished(int32 connId, void *yourPtr, uint64 buffer) {
	if (type == INITIATOR) {
		PeerEvent event = I_RCV_CONN_ACK;
		peer->performStateTransition(event, NULL);
	}
}

void DiameterConnection::socketDataArrived(int32 connId, void *yourPtr, cPacket *msg, bool urgent) {

	EV << "Received message on Assoc Id = " << socket.getConnectionId() << endl;
	SCTPSimpleMessage* smsg = check_and_cast<SCTPSimpleMessage*>(msg);
	DiameterMessage *dmsg = DiameterSerializer().parse(smsg);
	DiameterHeader hdr = dmsg->getHdr();
	dmsg->print();
	DiameterPeer *newPeer = NULL;
	unsigned resCode = processOrigin(newPeer, dmsg);

	if (peer == NULL) {
	    // enters only when the connection has no owner
		if ((hdr.getCommandCode() == CapabilitiesExchange) && (hdr.getReqFlag() == true)) {
			PeerEvent event = R_CONN_CER;
			if (resCode == DIAMETER_MISSING_AVP) {
			    /* TODO */
			} else {
				if ((newPeer->getState() == R_OPEN)
						|| (newPeer->getState() == I_OPEN)
						|| (newPeer->getState() == WAIT_RETURNS)
						|| (newPeer->getState() == WAIT_CONN_ACK_ELECT)) {
				    // if a peer with the same address info was found and it is one
				    // of this states reject the connection
					shutdown();
					newPeer->performStateTransition(event, dmsg);
					return;
				}
				// new peer and if available old peer becomes the owner of this
				// connection, election process will follow if necessary
				newPeer->rConn = this;
				this->setPeer(newPeer);
				newPeer->performStateTransition(event, dmsg);
			}
		}
		return;
	}

	if (peer->getState() == WAIT_I_CEA) {
		if ((hdr.getCommandCode() != CapabilitiesExchange) || (hdr.getReqFlag() != false)) {
			PeerEvent event = I_RCV_NON_CEA;
			peer->performStateTransition(event, NULL);
			return;
		}
	}
	if (type == RESPONDER) {
		switch (hdr.getCommandCode()) {
		case CapabilitiesExchange: {
			PeerEvent event;
			event = hdr.getReqFlag() == true ? R_RCV_CER : R_RCV_CEA;
			peer->performStateTransition(event, dmsg);
			break;
		}
		case DeviceWatchdog: {
			PeerEvent event = hdr.getReqFlag() == true ? R_RCV_DWR : R_RCV_DWA;
			peer->performStateTransition(event, dmsg);
			break;
		}
		case DisconnectPeer: {
			PeerEvent event = hdr.getReqFlag() == true ? R_RCV_DPR : R_RCV_DPA;
			peer->performStateTransition(event, dmsg);
			break;
		}
		default:
			PeerEvent event = R_RCV_MESSAGE;
			peer->performStateTransition(event, dmsg);
			break;
		}
	} else {
		switch (hdr.getCommandCode()) {
		case CapabilitiesExchange: {
			PeerEvent event = hdr.getReqFlag() == true ? I_RCV_CER : I_RCV_CEA;
			peer->performStateTransition(event, dmsg);
			break;
		}
		case DeviceWatchdog: {
			PeerEvent event = hdr.getReqFlag() == true ? I_RCV_DWR : I_RCV_DWA;
			peer->performStateTransition(event, dmsg);
			break;
		}
		case DisconnectPeer: {
			PeerEvent event = hdr.getReqFlag() == true ? I_RCV_DPR : I_RCV_DPA;
			peer->performStateTransition(event, dmsg);
			break;
		}
		default:
			PeerEvent event = I_RCV_MESSAGE;
			peer->performStateTransition(event, dmsg);
			break;
		}
	}
}

void DiameterConnection::socketDataNotificationArrived(int32 connId, void *yourPtr, cPacket *msg) {
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
    cPacket* cmsg = new cPacket();
    SCTPSendCommand *cmd = new SCTPSendCommand();
    cmd->setAssocId(ind->getAssocId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    delete ind;
    socket.sendNotification(cmsg);
}

void DiameterConnection::socketPeerClosed(int32 connId, void *yourPtr) {
	EV << "Peer socket closed (Assoc Id = " << socket.getConnectionId() << ")\n";
	if (peer != NULL) {
		PeerEvent event;
		// removes the connection from the peer
		if (type == INITIATOR) {
			event = I_PEER_DISC;
			peer->iConn = NULL;
		} else {
			event = R_PEER_DISC;
			peer->rConn = NULL;
		}
		if (peer->getState() != CLOSED) {
		    // peer will be deleted during the state transition
			peer->performStateTransition(event, NULL);
		}
		// removes the owner
		peer = NULL;
	}
	// removes the connection
	module->removeConnection(this);
}

void DiameterConnection::socketClosed(int32 connId, void *yourPtr) {
	EV << "Socket closed (Assoc Id = " << socket.getConnectionId() << ")\n";
	if (!ignore) {
	    // ignore is used because socketClosed is called twice
		if (peer != NULL) {
			PeerEvent event;
			if ((peer->getState() == WAIT_CONN_ACK) || (peer->getState() == WAIT_CONN_ACK_ELECT))
				event = I_RCV_CONN_NACK;
			else
				event = type == INITIATOR ? I_PEER_DISC : R_PEER_DISC;
			if ((type == peer->getConnectionType()) && (peer->getState() != CLOSED)) {
			    // peer will be removed during state transition
				peer->performStateTransition(event, NULL);
			}
			// removes the owner
			peer = NULL;
		}
		// connection cleanup
		module->removeConnection(this);
	}
	ignore = false;
}

void DiameterConnection::socketFailure(int32 connId, void *yourPtr, int32 code) {
//	EV << "failed\n";
	//peer->removeMe();
}

void DiameterConnection::send(DiameterMessage *msg, unsigned fqdnPos, unsigned realmPos) {

    cPacket* cmsg = new cPacket(msg->getName());
	msg->insertAvp(fqdnPos, DiameterUtils().createOctetStringAVP(AVP_OriginHost, 0, 1, 0, 0, peer->oFQDN.size(), peer->oFQDN.data()));
	msg->insertAvp(realmPos, DiameterUtils().createOctetStringAVP(AVP_OriginRealm, 0, 1, 0, 0, peer->oRealm.size(), peer->oRealm.data()));

    SCTPSimpleMessage* smsg = DiameterSerializer().serialize(msg);

    cmsg->encapsulate(smsg);

    cmsg->setKind(ordered ? SCTP_C_SEND_ORDERED : SCTP_C_SEND_UNORDERED);

    EV << "Sending message "<< msg->getName() << " on Assoc with Id = " << socket.getConnectionId() << endl;
    delete msg;		// all its content is in SCTPSimpleMessage
    socket.send(cmsg, true, true);

}

void DiameterConnection::connect() {
   	socket.connectx(addresses, port, 0);
}

void DiameterConnection::shutdown() {
    EV << "Shutting down Assoc Id = " << socket.getConnectionId() << endl;
    socket.shutdown();
}

void DiameterConnection::close() {
    EV << "Closing Assoc Id = " << socket.getConnectionId() << endl;
    socket.close();
}

unsigned DiameterConnection::processOrigin(DiameterPeer *&peer, DiameterMessage *msg) {
	AVP *fqdn = msg->findAvp(AVP_OriginHost);
	AVP *realm = msg->findAvp(AVP_OriginRealm);
	//DiameterPeer *peer = NULL;
	if ((fqdn == NULL) || (realm == NULL))
		return DIAMETER_MISSING_AVP;

	std::string dFQDN = DiameterUtils().processOctetStringAVP(fqdn);
	std::string dRealm = DiameterUtils().processOctetStringAVP(realm);

	//module->printPeerList();
	if ((peer = module->findPeer(dFQDN, dRealm)) != NULL) {
	    // enters if the peers are already open and the connection has owner
	    EV << "DiameterPeer with fqdn: " << dFQDN << " realm: " << dRealm << " found.\n";
		return DIAMETER_PEER_FOUND;
	}
	if (this->peer == NULL)	{
	    // only add another peer if this connection has no peer
        EV << "DiameterPeer with fqdn: " << dFQDN << " realm: " << dRealm << " not found. Creating peer.\n";
		type = RESPONDER;
		peer = module->createPeer(dFQDN, dRealm, this);
	}
	return DIAMETER_SUCCESS;
}

DiameterConnectionMap::DiameterConnectionMap() {
    // TODO Auto-generated constructor stub

}

DiameterConnectionMap::~DiameterConnectionMap() {
    // TODO Auto-generated destructor stub
}

DiameterConnection *DiameterConnectionMap::findConnection(cMessage *msg) {
    SCTPCommand *ind = dynamic_cast<SCTPCommand *>(msg->getControlInfo());
    if (!ind)
        opp_error("DiameterConnectionMap: findConnection(): no SCTPCommand control info in message (not from SCTP?)");
    int assocId = ind->getAssocId();
    DiameterConnections::iterator i = conns.find(assocId);
    ASSERT(i == conns.end() || i->first == i->second->getConnectionId());
    return (i == conns.end()) ? NULL : i->second;
}

void DiameterConnectionMap::insert(DiameterConnection *conn) {
    ASSERT(conns.find(conn->getConnectionId()) == conns.end());
    conns[conn->getConnectionId()] = conn;
}

void DiameterConnectionMap::erase(DiameterConnection *conn) {
    int connId = conn->getConnectionId();
    ASSERT(conns.find(connId) != conns.end());
    delete conns[connId];
    conns.erase(connId);
}

void DiameterConnectionMap::print() {
    EV << "=====================================================================\n"
       << "Connection Map:\n"
       << "-------------------------------------------------------------------------------\n"
       << "Conn Id\t| Local Addr\t| Local Port\t| Remote Addr\t| Remote Port\n"
       << "---------------------------------------------------------------------\n";
    for (DiameterConnections::iterator i = conns.begin(); i != conns.end(); ++i)
        EV << (i->second)->getConnectionId() << endl;// << "\t| " << (i->second)->getLocalAddresses().at(0) << "\t| " << "" << "\t| " << "" << "\t| " << "" << endl;
    EV << "=====================================================================\n";
}

