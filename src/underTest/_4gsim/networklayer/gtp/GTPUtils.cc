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

#include <platdep/sockets.h>
#include "GTPUtils.h"
#include "GTPSerializer.h"

GTPUtils::GTPUtils() {
	// TODO Auto-generated constructor stub

}

GTPUtils::~GTPUtils() {
	// TODO Auto-generated destructor stub
}

GTPv2Header *GTPUtils::createHeader(unsigned char type, bool p, bool t, unsigned teid, unsigned seqNr) {
	GTPv2Header *hdr = new GTPv2Header();
	hdr->setType(type);
	hdr->setP(p);
	hdr->setT(t);
	hdr->setTeid(teid);
	hdr->setSeqNr(seqNr);
	return hdr;
}

GTPv1Header *GTPUtils::createHeader(unsigned char type, bool pt, bool e, bool s, bool pn, unsigned int teid, unsigned int seqNr, unsigned char npduNr, unsigned char extNextType, std::vector<GTPv1Extension> exts) {
	GTPv1Header *hdr = new GTPv1Header();
	hdr->setType(type);
	hdr->setPt(pt);
	hdr->setE(e);
	hdr->setS(s);
	hdr->setPn(pn);
	hdr->setTeid(teid);
	hdr->setSeqNr(seqNr);
	hdr->setNpduNr(npduNr);
	hdr->setExtNextType(extNextType);
	hdr->setExtsArraySize(exts.size());
	for (unsigned i = 0; i < exts.size(); i++)
		hdr->setExts(i, exts.at(i));
	return hdr;
}

GTPInfoElem *GTPUtils::createIE(unsigned char type, unsigned short length, unsigned char instance, char *value) {
	GTPInfoElem *ie = new GTPInfoElem();
	ie->setType(type);
	ie->setInstance(instance);
	ie->setValueArraySize(length);
	for (unsigned i = 0; i < length; i++) ie->setValue(i, value[i]);
	return ie;
}

GTPInfoElem *GTPUtils::createIE(unsigned char type,unsigned char instance, std::string value) {
	GTPInfoElem *ie = new GTPInfoElem();
	ie->setType(type);
	ie->setInstance(instance);
	ie->setValueArraySize(value.size());
	for (unsigned i = 0; i < value.size(); i++) ie->setValue(i, value[i]);
	return ie;
}

GTPInfoElem *GTPUtils::createIE(unsigned char version, unsigned char type, unsigned char instance, char value) {
	GTPInfoElem *ie = new GTPInfoElem();
	if (version == GTP_V2) {
		GTPInfoElem *ie = new GTPInfoElem();
		ie->setInstance(instance);
	}
	ie->setType(type);
	ie->setValueArraySize(1);
	ie->setValue(0, value);
	return ie;
}

GTPInfoElem *GTPUtils::createIntegerIE(unsigned char type, unsigned char instance, unsigned value) {
	char *val = (char *)calloc(sizeof(unsigned), sizeof(char));
	char *p = val;
	*((unsigned*)p) = ntohl(value);
	p += 4;
	return createIE(type, sizeof(unsigned), instance, val);
}

GTPInfoElem *GTPUtils::createCauseIE(unsigned short length,
											unsigned char instance,
											unsigned char cause,
											bool pce,
											bool bce,
											bool cs,
											unsigned char ieType,
											unsigned char ieInstance) {

	GTPInfoElem *ie = new GTPInfoElem();
	ie->setType(GTPV2_Cause);
	ie->setInstance(instance);
	ie->setValueArraySize(length);
	ie->setValue(0, cause);
	ie->setValue(1, ((pce << 2) & 0x04)  | ((bce << 1) & 0x02) | (cs & 0x01));
	if (length == GTPV2_CAUSE_IE_MAX_SIZE) {
		ie->setValue(2, ieType);
		ie->setValue(3, instance);
	}
	return ie;
}

GTPInfoElem *GTPUtils::createAddrIE(unsigned char type, unsigned char instance, IPvXAddress addr) {
	unsigned len = addr.isIPv6() ? 17 : 5;
	char *val = (char*)calloc(len, sizeof(char));
	char *p = val;

	*((char*)p) = addr.isIPv6() ? 0x02 : 0x01;
	p += 1;
	if (addr.isIPv6()) {	// not supported
	} else {
		*((unsigned*)p) = ntohl(addr.get4().getInt());
		p += 4;
	}
	return GTPUtils().createIE(type, len, instance, val);
}

GTPInfoElem *GTPUtils::createGroupedIE(unsigned char type, unsigned char instance, unsigned length, std::vector<GTPInfoElem*>ies) {
	unsigned pos = 0;
	unsigned char *buf = (unsigned char*)calloc(length, sizeof(unsigned char));
	for (unsigned i = 0; i < ies.size(); i++) {
		pos += GTPSerializer().serializeIE(GTP_V2, ies.at(i), buf + pos);
	}
	return createIE(type, pos, instance, (char*)buf);
}

GTPInfoElem *GTPUtils::createFteidIE(unsigned char instance, unsigned teid, char type, IPvXAddress addr) {
	unsigned len = addr.isIPv6() ? 21 : 9;
	char *val = (char*)calloc(len, sizeof(char));
	char *p = val;

	*((char*)p) = (addr.isIPv6() ? 0x40 : 0x80) + type;
	p += 1;
	*((unsigned*)p) = ntohl(teid);
	p += 4;
	if (addr.isIPv6()) {	// not supported
	} else {
		*((unsigned*)p) = ntohl(addr.get4().getInt());
		p += 4;
	}
	return GTPUtils().createIE(GTPV2_F_TEID, len, instance, val);
}

char *GTPUtils::processIE(GTPInfoElem *ie) {
	char *val = (char*)calloc(ie->getValueArraySize(), sizeof(char));
	for (unsigned i = 0; i < ie->getValueArraySize(); i++)
		val[i] = ie->getValue(i);
	return val;
}

IPvXAddress GTPUtils::processAddrIE(GTPInfoElem *addr) {
	char *val = (char*)calloc(addr->getValueArraySize(), sizeof(char));
	for (unsigned i = 0; i < addr->getValueArraySize(); i++)
		val[i] = addr->getValue(i);
	char *p = val;

	if ((*p & 0x01)) {
//		char type = *p & 31;
		p++;
		unsigned addr = ntohl(*((unsigned*)(p)));
		p += 4;
		return IPvXAddress(IPv4Address(addr));
	} else if ((*p & 0x02)) {	// ipv6 not supported

	}
	return IPvXAddress("0.0.0.0");
}

TunnelEndpoint *GTPUtils::processFteidIE(GTPInfoElem *fteid) {
	if (fteid->getType() != GTPV2_F_TEID) {
		return NULL;
	}
	char *val = processIE(fteid);
	char *p = val;
	if ((*p & 0x80)) {
		char type = *p & 31;
		p++;
		unsigned teid = ntohl(*((unsigned*)(p)));
		p += 4;
		unsigned addr = ntohl(*((unsigned*)(p)));
		p += 4;
		GTPPath *path = new GTPPath();
		path->setRemoteAddr(IPv4Address(addr));
		path->setType(type);
		TunnelEndpoint *te = new TunnelEndpoint();
		te->setPath(path);
		te->setRemoteId(teid);
		return te;
	} else if ((*p & 0x40)) {	// ipv6 not supported

	}

	return NULL;
}

unsigned char GTPUtils::processCauseIE(GTPInfoElem *cause) {
	EV << "Request cause:\n";
	char *val = processIE(cause);
	EV << "\tCause value: " << (unsigned)*((unsigned char*)val) << endl;
	return *((unsigned char*)val);
}

unsigned GTPUtils::getGTPv1InfoElemLen(unsigned char type) {
	switch(type) {
		case GTPV1_Recovery: return 1;
		case GTPV1_TEIDData1: return 4;
		default:
		    break;
	}
	return 0;
}

std::vector<GTPInfoElem*> GTPUtils::processGroupedIE(GTPInfoElem *ie) {
	std::vector<GTPInfoElem*> group;

	char *val = processIE(ie);
	unsigned char *p = (unsigned char*)val;
	unsigned char *end = (unsigned char*)val + ie->getValueArraySize();
	while(p < end) {
		GTPInfoElem *groupIE = new GTPInfoElem();
		GTPSerializer().parseIE(GTP_V2, p, groupIE);
		group.push_back(groupIE);
		p += groupIE->getValueArraySize() + GTPV2_IE_HEADER_SIZE;
	}
	return group;
}
