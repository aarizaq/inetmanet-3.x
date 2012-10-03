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

#include "SubscriberTable.h"
#include "LTEUtils.h"

Define_Module(SubscriberTable);

std::ostream& operator<<(std::ostream& os, Subscriber& s) {
    os << s.info();
    return os;
};

SubscriberTable::SubscriberTable() {
	// TODO Auto-generated constructor stub
	enbIds = 0;
	mmeIds = 0;
}

SubscriberTable::~SubscriberTable() {
	// TODO Auto-generated destructor stub
	erase(0, subs.size());

	if (cleanTimer != NULL) {
		if (cleanTimer->getContextPointer() != NULL)
			this->cancelEvent(cleanTimer);
		delete cleanTimer;
	}
}

void SubscriberTable::initialize(int stage) {
	if (findPar("configFile") != -1) {
		const char *fileName = par("configFile");
		if ((fileName != NULL) && (strcmp(fileName, ""))) {
			EV << "SubscriberTable:	Initializing subscribers.\n";

			cXMLElement* config = ev.getXMLDocument(fileName);
			if (config == NULL)
				error("SubscriberTable: Cannot read configuration from file: %s", fileName);

			loadSubscribersFromXML(*config);
		}
	}
	WATCH_PTRVECTOR(subs);

	cleanTimer = new cMessage("INACTIVE-TIMER");
	cleanTimer->setContextPointer(this);
	this->scheduleAt(simTime() + CLEAN_TIMER_TIMEOUT, cleanTimer);
}

void SubscriberTable::handleMessage(cMessage *msg) {
	if (msg->isSelfMessage()) {
		if (msg == cleanTimer) {
			EV << "SubscriberTable: Inactive subscriber timer expired. Cleaning inactive subscribers.\n";
			Subscribers::iterator i = subs.begin();
			Subscribers::iterator last = subs.end();
			for (;i != last; ++i) {
				Subscriber *sub = *i;
				if (sub->getStatus() == SUB_INACTIVE) {
					EV << "SubscriberTable: Found one inactive subscriber. Cleaning it.\n";
					delete *i;
					subs.erase(i);
				}
			}
			this->cancelEvent(cleanTimer);
			this->scheduleAt(simTime() + CLEAN_TIMER_TIMEOUT, cleanTimer);
		}
	} else
		delete msg;
}

Subscriber *SubscriberTable::findSubscriberForId(unsigned enbId, unsigned mmeId) {
	for (unsigned i = 0; i < subs.size(); i++) {
		Subscriber *sub = subs[i];
		if (!sub->getMmeId())
			sub->setMmeId(mmeId);
		if (sub->getEnbId() == enbId && sub->getMmeId() == mmeId)
			return sub;
	}
	return NULL;
}

Subscriber *SubscriberTable::findSubscriberForChannel(int channelNr) {
	for (unsigned i = 0; i < subs.size(); i++) {
		Subscriber *sub = subs[i];
		if (sub->getChannelNr() == channelNr)
			return sub;
	}
	return NULL;
}

Subscriber *SubscriberTable::findSubscriberForIMSI(char *imsi) {
	for (unsigned i = 0; i < subs.size(); i++) {
		Subscriber *sub = subs[i];
		if (!strncmp(sub->getEmmEntity()->getImsi(), imsi, IMSI_CODED_SIZE))
			return sub;
	}
	return NULL;
}

Subscriber *SubscriberTable::findSubscriberForSeqNr(unsigned seqNr) {
	for (unsigned i = 0; i < subs.size(); i++) {
		Subscriber *sub = subs[i];
		if (sub->backSeqNr() == seqNr) {
			sub->popSeqNr();
			return sub;
		}
	}
	return NULL;
}

void SubscriberTable::erase(unsigned it) {
	Subscribers::iterator i = subs.begin() + it;
	delete *i;
	subs.erase(i);
}

void SubscriberTable::erase(unsigned start, unsigned end) {
	Subscribers::iterator first = subs.begin() + start;
	Subscribers::iterator last = subs.begin() + end;
	Subscribers::iterator i = first;
	for (;i != last; ++i) {
		delete *i;
	}
	subs.erase(first, last);
}

void SubscriberTable::loadPDNConnectionsFromXML(const cXMLElement& subElem, Subscriber *sub) {
	cXMLElement* connNode = subElem.getElementByPath("PDNConns");
	if (connNode != NULL) {
		cXMLElementList connList = connNode->getChildren();
		for (cXMLElementList::iterator conIt = connList.begin(); conIt != connList.end(); conIt++) {
			std::string elementName = (*conIt)->getTagName();
			if ((elementName == "PDNConn")) {
				const char *apn = (*conIt)->getAttribute("apn");
				if (!apn)
					error("SubscriberTable: Subscriber has no apn attribute");
				const char *pdnGWAddr = (*conIt)->getAttribute("pdnGwAddr");
				if (!pdnGWAddr)
					error("SubscriberTable: Subscriber has no pdnGwAddr attribute");
				const char *subAddr = (*conIt)->getAttribute("pdnAllocAddr");
				if (!subAddr)
					error("SubscriberTable: Subscriber has no pdnAllocAddr attribute");
				ESMEntity *esm = new ESMEntity();
				PDNConnection *conn = new PDNConnection(esm);
				conn->setAPN(apn);
				conn->setPDNGwAddress(IPvXAddress(pdnGWAddr));
				conn->setSubscriberAddress(IPvXAddress(subAddr));
				esm->setOwner(sub);
				if ((*conIt)->getAttribute("def") && (!strncmp((*conIt)->getAttribute("def"), "true", 4))) {
					esm->addPDNConnection(conn, true);
				} else {
					esm->addPDNConnection(conn, false);
				}
			}
		}
	}
}

void SubscriberTable::loadSubscribersFromXML(const cXMLElement& config) {
	cXMLElement* subssNode = config.getElementByPath("Subscribers");
	if (subssNode != NULL) {
	    cXMLElementList subsList = subssNode->getChildren();
	    for (cXMLElementList::iterator subIt = subsList.begin(); subIt != subsList.end(); subIt++) {

	    	std::string elementName = (*subIt)->getTagName();
	        if ((elementName == "Subscriber")) {
	        	const char *imsi = (*subIt)->getAttribute("imsi");
	        	if (!imsi)
	        		error("SubscriberTable: Subscriber has no imsi attribute");

				const char *msisdn = (*subIt)->getAttribute("msisdn");
				if (!msisdn)
					error("SubscriberTable: Subscriber has no msisdn attribute");

				EMMEntity *emm = new EMMEntity();
	        	Subscriber *sub = new Subscriber();
	        	emm->setOwner(sub);
	        	emm->setImsi(LTEUtils().toIMSI(imsi));
	        	sub->setMsisdn(LTEUtils().toTBCDString(msisdn, MSISDN_UNCODED_SIZE));
	        	sub->setStatus(SUB_ACTIVE);
	        	loadPDNConnectionsFromXML(*(*subIt), sub);
	        	EV << "SubscriberTable: Subscriber nr. " << size() << endl;
	        	push_back(sub);
	        }
	    }
	}
}
