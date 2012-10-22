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

#include "ESMEntity.h"
#include "EMMEntity.h"
#include "NAS.h"
#include "NASUtils.h"
#include "DiameterApplication.h"
#include "GTPMessage_m.h"
#include "NASSerializer.h"

ESMEntity::ESMEntity() {
	init();
}

ESMEntity::ESMEntity(unsigned char appType) {
	// TODO Auto-generated constructor stub
	init();
	this->appType = appType;
}

ESMEntity::~ESMEntity() {
	// TODO Auto-generated destructor stub
	delPDNConnection(0, conns.size());
}

void ESMEntity::init() {
	ownerp = NULL;
	appType = 0;
	defConn = NULL;
	connIds = 0;
	bearerIds = 0;
	peer = NULL;
	module = NULL;
}

void ESMEntity::setOwner(Subscriber *ownerp) {
	ownerp->setEsmEntity(this);
	this->ownerp = ownerp;
}

Subscriber *ESMEntity::getOwner() {
	return ownerp;
}

void ESMEntity::addPDNConnection(PDNConnection *conn, bool def) {
	conn->setOwner(this);
	if (def == true)
		defConn = conn;
	conns.push_back(conn);
}

void ESMEntity::delPDNConnection(unsigned start, unsigned end) {
	PDNConnections::iterator first = conns.begin() + start;
	PDNConnections::iterator last = conns.begin() + end;
	PDNConnections::iterator i = first;
	for (;i != last; ++i) {
		if (*i == defConn)
			defConn = NULL;
		delete *i;
	}
	conns.erase(first, last);
}

AVP *ESMEntity::createAPNConfigProfAVP() {
	if (conns.size() > 0) {
		std::vector<AVP*> apnConfigProfVec;
		apnConfigProfVec.push_back(DiameterUtils().createUnsigned32AVP(AVP_ContextIdentifier, 1, 0, 0, TGPP, defConn->getId()));
		apnConfigProfVec.push_back(DiameterUtils().createInteger32AVP(AVP_AllAPNConfigInclInd, 1, 0, 0, TGPP, All_APN_CONFIGURATIONS_INCLUDED));
		for (unsigned i = 0; i < conns.size(); i++) {
			PDNConnection *conn = conns.at(i);
			apnConfigProfVec.push_back(conn->createAPNConfigAVP());
		}
		AVP *apnConfigProf = DiameterUtils().createGroupedAVP(AVP_APNConfigProfile, 1, 0, 0, TGPP, apnConfigProfVec);
		DiameterUtils().deleteGroupedAVP(apnConfigProfVec);
		return apnConfigProf;
	}
	return NULL;
}

bool ESMEntity::processAPNConfigProfAVP(AVP *apnConfigProf) {
	std::vector<AVP*> apnConfigProfVec = DiameterUtils().processGroupedAVP(apnConfigProf);
	AVP *ctxtId = DiameterUtils().findAVP(AVP_ContextIdentifier, apnConfigProfVec);
	if (ctxtId == NULL) {
		EV << "DiameterS6a: Missing ContextIdentifier AVP.\n";
		return false;
	}
	unsigned defId = DiameterUtils().processUnsigned32AVP(ctxtId);
	std::vector<AVP*> apnConfigs = DiameterUtils().findAVPs(AVP_APNConfig, apnConfigProfVec);
	for (unsigned i = 0; i < apnConfigs.size(); i++) {
		std::vector<AVP*> apnConfig = DiameterUtils().processGroupedAVP(apnConfigs[i]);
		if (i < conns.size()) {
			PDNConnection *conn = conns.at(i);
			if (!conn->processAPNConfigAVP(apnConfig))
				return false;
			if (conn->getId() == defId)
				this->defConn = conn;
		} else {
			/* TODO */
			// add additional pdn connections received from HSS
		}
	}
	DiameterUtils().deleteGroupedAVP(apnConfigProfVec);
	return true;
}

void ESMEntity::setPeer(EMMEntity *peer) {
	this->peer = peer;
}

EMMEntity *ESMEntity::getPeer() {
	return peer;
}

void ESMEntity::setModule(NAS *module) {
	this->module = module;
}

std::string ESMEntity::info(int tabs) const {
	std::stringstream out;
	if (conns.size() > 0) {
		for (int i = 0; i < tabs; i++) out << "\t";
		out << "pdnConns:{\n";
		for (unsigned j = 0; j < conns.size(); j++) {
			for (int i = 0; i < tabs + 1; i++) out << "\t";
			out << "pdnConn:{\n" << conns.at(j)->info(tabs + 2);
			for (int i = 0; i < tabs + 1; i++) out << "\t";
			out << "}\n";
		}
		for (int i = 0; i < tabs; i++) out << "\t";
		out << "}\n";
	}
	return out.str();
}
