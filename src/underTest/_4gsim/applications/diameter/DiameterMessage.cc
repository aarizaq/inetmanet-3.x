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

#include "DiameterMessage.h"

Register_Class(DiameterMessage);

DiameterMessage& DiameterMessage::operator=(const DiameterMessage& other) {

	DiameterMessage_Base::operator=(other);

	DiameterHeader hdr = other.getHdr();
	this->setHdr(hdr);

	for (unsigned i = 0; i < other.getAvpsArraySize(); i++) {
		this->pushAvp(other.avps.at(i)->dup());
	}
    return *this;
}

DiameterMessage::~DiameterMessage() {
    if (this->getAvpsArraySize() > 0) {
    	while(!avps.empty()) {
    		AVP *avp = avps.back();
    		avps.pop_back();
    		delete avp;
    	}
    }
}

void DiameterMessage::setAvpsArraySize(unsigned int size) {
     throw new cException(this, "setAvpsArraySize() not supported, use pushAVP() or insertAVP()");
}

unsigned int DiameterMessage::getAvpsArraySize() const {
     return avps.size();
}

void DiameterMessage::setAvps(unsigned int k, const AVPPtr& avps_var) {
     throw new cException(this, "setAvps() not supported, use use pushAVP() or insertAVP()");
}

AVPPtr& DiameterMessage::getAvps(unsigned int k) {
    AVPVector::iterator i = avps.begin();
    while (k > 0 && i != avps.end())
         (++i, --k);
    return *i;
}

void DiameterMessage::insertAvp(unsigned pos, AVPPtr avp) {
	AVPVector::iterator it = avps.begin();
	avps.insert(it + pos, avp);
}

AVPPtr DiameterMessage::findAvp(unsigned avpCode) {
	for (unsigned i = 0; i < this->getAvpsArraySize(); i++) {
		if (this->getAvps(i)->getAvpCode() == avpCode)
			return this->getAvps(i);
	}
	return NULL;
}

void DiameterMessage::print() {
	DiameterHeader hdr = this->getHdr();
	EV << "===================================================\n";
	EV << "DiameterMessage:\n";
	EV << "Version: " << (unsigned)hdr.getVersion() << endl;
	EV << "Flags:\n";
	EV << "\tRequest: " << hdr.getReqFlag() << endl;
	EV << "\tProxyable: " << hdr.getPrxyFlag() << endl;
	EV << "\tError: " << hdr.getErrFlag() << endl;
	EV << "\tRetransmit: " << hdr.getRetrFlag() << endl;
	EV << "Command Code: " << hdr.getCommandCode() << endl;
	EV << "ApplicationId: " << hdr.getApplId() << endl;
	EV << "Hop-by-Hop Identifier: " << hdr.getHopByHopId() << endl;
	EV << "End-to-End Identifier: " << hdr.getEndToEndId() << endl;
	EV << "===================================================\n";
}
