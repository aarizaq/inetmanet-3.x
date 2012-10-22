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

#include "GTPPath.h"
#include "GTP.h"
#include "GTPUtils.h"
#include "GTPControl.h"

GTPPath::GTPPath() {
	echoTimer = NULL;
	module = NULL;
}

GTPPath::GTPPath(GTP *module, IPvXAddress localAddr, IPvXAddress remoteAddr, unsigned char type) {
	// TODO Auto-generated constructor stub
	this->localAddr = localAddr;
	this->remoteAddr = remoteAddr;
	this->module = module;
	this->remoteCounter = 0;
	this->type = type;

	echoTimer = new cMessage("ECHO-TIMER");
	module->scheduleAt(simTime() + ECHO_TIMER_TIMEOUT + uniform(-2, 2), echoTimer);
	echoTimer->setContextPointer(this);

	EV <<"GTP: Initialized listen address = " << localAddr.str() << endl;

	int localPort;
	if (module->getPlane() == GTP_CONTROL)
	    localPort = GTP_CONTROL_PORT;
	else if (module->getPlane() == GTP_USER)
	    localPort = GTP_USER_PORT;
	module->socket.setOutputGate(module->gate("udpOut"));
	module->socket.bind(localAddr,localPort);

	if (module->getPlane() == GTP_USER)
		localCounter = 0;
	else
		localCounter = 1;
}

GTPPath::~GTPPath() {
	// TODO Auto-generated destructor stub
	if (echoTimer != NULL) {
		if (echoTimer->getContextPointer() != NULL)
			module->cancelEvent(echoTimer);
		delete echoTimer;
	}
}

void GTPPath::resetEchoTimer() {
	module->cancelEvent(echoTimer);
	module->scheduleAt(simTime() + ECHO_TIMER_TIMEOUT + uniform(-2, 2), echoTimer);
}

void GTPPath::processMessage(cMessage *msg) {
	GTPMessage *gmsg = check_and_cast<GTPMessage*>(msg);
	gmsg->print();

	switch(gmsg->getHeader()->getType()) {
		case EchoRequest:
			processEchoRequest(gmsg);
			break;
		case EchoResponse:
			break;
		default:
			dynamic_cast<GTPControl*>(module)->processMessage(gmsg, this);
			break;
	}
}

void GTPPath::processEchoRequest(GTPMessage *msg) {
	if (module->getPlane() == GTP_USER) {
		resetEchoTimer();
		sendEchoResponse(msg->getHeader()->getSeqNr(), NULL);
	} else if (module->getPlane() == GTP_CONTROL) {
		GTPInfoElem *recovCount = msg->findIe(GTPV2_Recovery, 0);
		if (recovCount != NULL) {
			unsigned char remoteCounter = recovCount->getValue(0);
			if (remoteCounter > this->remoteCounter)
				this->remoteCounter = remoteCounter;
			resetEchoTimer();
			if (msg->getHeader()->getType() == EchoRequest)
				sendEchoResponse(msg->getHeader()->getSeqNr(), NULL);
		} else if (msg->getHeader()->getVersion() == 2) {
			if (msg->getHeader()->getType() == EchoRequest)
				sendEchoResponse(msg->getHeader()->getSeqNr(), GTPUtils().createCauseIE(GTPV2_CAUSE_IE_MAX_SIZE, 0, Man_IE_Missing, 0, 0, 1, GTPV2_Recovery, 0));
		}
	}
}

void GTPPath::processEchoTimer() {
	sendEchoRequest();
	resetEchoTimer();
}

void GTPPath::send(GTPMessage *msg) {



    int remotePort;
    int localPort = EPHEMERAL_PORT;
    if (module->getPlane() == GTP_CONTROL)
        remotePort = GTP_CONTROL_PORT;
    else if (module->getPlane() == GTP_USER)
        remotePort = GTP_USER_PORT;
            /*
    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSrcAddr(localAddr);
    ctrl->setSrcPort(EPHEMERAL_PORT);
    ctrl->setDestAddr(remoteAddr);
	if (module->getPlane() == GTP_CONTROL)
		ctrl->setDestPort(GTP_CONTROL_PORT);
	else if (module->getPlane() == GTP_USER)
		ctrl->setDestPort(GTP_USER_PORT);
    msg->setControlInfo(ctrl);

    module->send(msg, "udpOut");
    */
    module->socket.sendTo(msg,remoteAddr, remotePort);
}

void GTPPath::sendEchoRequest() {
	GTPMessage *echoReq = new GTPMessage("Echo-Request");

	if (module->getPlane() == GTP_CONTROL) {
		GTPv2Header *hdr = GTPUtils().createHeader(EchoRequest, 0, 0, 0, uniform(0, 1000));
		echoReq->setHeader(hdr);
		echoReq->pushIe(GTPUtils().createIE(GTP_V2, GTPV2_Recovery, 0, localCounter));
	} else if (module->getPlane() == GTP_USER) {
		std::vector<GTPv1Extension> exts;
		GTPv1Header *hdr = GTPUtils().createHeader(EchoRequest, 1, 0, 1, 0, 0, uniform(0, 1000), 0, 0, exts);
		echoReq->setHeader(hdr);
	}
	send(echoReq);
}

void GTPPath::sendEchoResponse(unsigned seqNr, GTPInfoElem *cause) {
	GTPMessage *echoRes = new GTPMessage("Echo-Response");
	if (module->getPlane() == GTP_CONTROL) {
		GTPv2Header *hdr = GTPUtils().createHeader(EchoResponse, 0, 0, 0, seqNr);
		echoRes->setHeader(hdr);
		echoRes->pushIe(GTPUtils().createIE(GTP_V2, GTPV2_Recovery, 0, localCounter));
		if (cause != NULL) {
			echoRes->pushIe(cause);
		}
	} else if (module->getPlane() == GTP_USER) {
		std::vector<GTPv1Extension> exts;
		GTPv1Header *hdr = GTPUtils().createHeader(EchoResponse, 1, 0, 1, 0, 0, seqNr, 0, 0, exts);
		echoRes->setHeader(hdr);
		echoRes->pushIe(GTPUtils().createIE(GTP_V1, GTPV1_Recovery, 0, localCounter));
	}

	send(echoRes);
}

std::string GTPPath::info(int tabs) const {

    std::stringstream out;
    if (module != NULL)
    	out << "pl:" << (module->getPlane() == GTP_USER ? "u" : "c") << "  ";
    out << "type:" << (unsigned)type << "  ";
    out << "lAddr:" << localAddr.str() << "  ";
    out << "rAddr:" << remoteAddr.str();

    return out.str();
}

bool GTPPath::getPlane() {
	return module->getPlane();
}

void GTPPath::setModule(GTP *module) {
	this->module = module;
}

GTP *GTPPath::getModule() {
	return module;
}
