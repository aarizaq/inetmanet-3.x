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

#include "GTPMessage.h"

Register_Class(GTPMessage);

GTPMessage& GTPMessage::operator=(const GTPMessage& other) {
	GTPMessage_Base::operator=(other);

	GTPHeader *hdr = other.getHeader()->dup();
	this->setHeader(hdr);

	for (unsigned i = 0; i < other.getIesArraySize(); i++) {
		this->pushIe(other.ies.at(i)->dup());
	}

    return *this;
}

GTPMessage::~GTPMessage() {
	delete header_var;
    if (this->getIesArraySize() > 0) {
    	while(!ies.empty()) {
    		GTPInfoElem *ie = ies.back();
    		ies.pop_back();
    		delete ie;
    	}
    }
}

void GTPMessage::setIesArraySize(unsigned int size) {
     throw new cException(this, "setIesArraySize() not supported, use pushAVP()");
}

unsigned int GTPMessage::getIesArraySize() const {
     return ies.size();
}

void GTPMessage::setIes(unsigned int k, const GTPInfoElemPtr& avps_var) {
     throw new cException(this, "setIes() not supported, use use pushAVP()");
}

GTPInfoElemPtr& GTPMessage::getIes(unsigned int k) {
	GTPInfoElems::iterator i = ies.begin();
    while (k > 0 && i != ies.end())
         (++i, --k);
    return *i;
}

GTPInfoElemPtr GTPMessage::findIe(unsigned char type, unsigned char instance) {
	if (this->getHeader()->getVersion() == 2) {
		for (unsigned i = 0; i < this->getIesArraySize(); i++) {
			GTPInfoElemPtr ie = this->getIes(i);
			if ((ie->getType() == type) && (ie->getInstance() == instance))
				return ie;
		}
	}
	return NULL;
}

std::vector<GTPInfoElemPtr> GTPMessage::findIes(unsigned char type, unsigned char instance) {
	std::vector<GTPInfoElemPtr> ies;
	if (this->getHeader()->getVersion() == 2) {
		for (unsigned i = 0; i < this->getIesArraySize(); i++) {
			GTPInfoElemPtr ie = this->getIes(i);
			if ((ie->getType() == type) && (ie->getInstance() == instance))
				ies.push_back(ie);
		}
	}
	return ies;
}

void GTPMessage::print() {
	EV << "===================================================\n";
	EV << "Version: " << (unsigned)this->getHeader()->getVersion() << endl;
	EV << "Flags:\n";
	if (this->getHeader()->getVersion() == GTP_V1) {
		GTPv1Header *hdr = dynamic_cast<GTPv1Header*>(this->getHeader());
		EV << "\tProt. Type Flag: " << hdr->getPt() << endl;
		EV << "\tExt. Flag: " << hdr->getE() << endl;
		EV << "\tSeq. Nr. Flag: " << hdr->getS() << endl;
		EV << "\tNPDU Flag: " << hdr->getPn() << endl;
		EV << "Message Type: " << (unsigned)hdr->getType() << endl;
		EV << "TEID: " << hdr->getTeid() << endl;
		if (hdr->getE() || hdr->getE() || hdr->getS()) {
			EV << "Seq. Nr.: " << hdr->getSeqNr() << endl;
			EV << "NPDU Nr.: " << (unsigned)hdr->getNpduNr() << endl;
			EV << "Next Ext. Hdr. Type: " << (unsigned)hdr->getExtNextType() << endl;
		}
	} else if (this->getHeader()->getVersion() == GTP_V2) {
		GTPv2Header *hdr = dynamic_cast<GTPv2Header*>(this->getHeader());
		EV << "\tPiggyback. Flag: " << hdr->getP() << endl;
		EV << "\tTEID Flag: " << hdr->getT() << endl;
		EV << "Message Type: " << (unsigned)hdr->getType() << endl;
		if (hdr->getT())
			EV << "TEID: " << hdr->getTeid() << endl;
		EV << "Seq. Nr.: " << hdr->getSeqNr() << endl;
	}
	EV << "===================================================\n";
}
