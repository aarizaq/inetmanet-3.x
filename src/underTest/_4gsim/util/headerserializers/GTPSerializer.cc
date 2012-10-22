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
#include "GTPSerializer.h"
#include "GTPUtils.h"

GTPSerializer::GTPSerializer() {
	// TODO Auto-generated constructor stub

}

GTPSerializer::~GTPSerializer() {
	// TODO Auto-generated destructor stub
}

unsigned GTPSerializer::serializeHeader(GTPMessage *msg, unsigned char *buf) {
	unsigned short hdrLen;
	unsigned short msgLen;
	unsigned char *p = buf;

	if (msg->getHeader()->getVersion() == GTP_V2) {
		const GTPv2Header *hdr = dynamic_cast<const GTPv2Header*>(msg->getHeader());
		msgLen = hdrLen = (hdr->getT() == true ? GTP_HEADER_MAX_SIZE : GTP_HEADER_MIN_SIZE);
		for (unsigned i = 0; i < msg->getIesArraySize(); i++) {
			GTPInfoElem *ie = msg->getIes(i);
			msgLen += ie->getValueArraySize() + GTPV2_IE_HEADER_SIZE;
		}

		buf = (unsigned char*)calloc(hdrLen, sizeof(unsigned char));
		*((unsigned char*)(p)) =
				((hdr->getVersion() << 5) & 0xe0) | ((hdr->getP() << 4) & 0x10)  | ((hdr->getT() << 3) & 0x08);
		p += 1;
		*((unsigned char*)p) = hdr->getType();
		p += 1;
		*((unsigned short*)p) = ntohs(msgLen);
		p += 2;
		if (hdr->getT()) {
			*((unsigned*)p) = ntohl(hdr->getTeid());
			p += 4;
		}
		*((unsigned*)p) = htonl(hdr->getSeqNr() & 0x00ffffff) >> 8;
		p += 3;
		*((unsigned char*)p) |= 0;
		p += 1;
	} else if (msg->getHeader()->getVersion() == GTP_V1) {
		const GTPv1Header *hdr = dynamic_cast<const GTPv1Header*>(msg->getHeader());
		msgLen = hdrLen = GTP_HEADER_MIN_SIZE;
		if (hdr->getE() || hdr->getE() || hdr->getS())
			hdrLen = GTP_HEADER_MAX_SIZE;
		for (unsigned i = 0; i < msg->getIesArraySize(); i++) {
			msgLen += msg->getIes(i)->getValueArraySize() + GTPV1_IE_HEADER_MAX_SIZE;
		}
		buf = (unsigned char*)calloc(hdrLen, sizeof(unsigned char));
		*((unsigned char*)(p)) =
				((hdr->getVersion() << 5) & 0xe0) | ((hdr->getPt() << 4) & 0x10)  | ((hdr->getE() << 2) & 0x04) | ((hdr->getS() << 1) & 0x02) | ((hdr->getPn()) & 0x01);
		p += 1;
		*((unsigned char*)p) = hdr->getType();
		p += 1;
		*((unsigned short*)p) = ntohs(msgLen);
		p += 2;
		*((unsigned*)p) = ntohl(hdr->getTeid());
		p += 4;
		if (hdrLen > GTP_HEADER_MIN_SIZE) {
			*((unsigned short*)p) = ntohs(hdr->getSeqNr());
			p += 2;
			*((unsigned char*)p) = hdr->getNpduNr();
			p += 1;
			*((unsigned char*)p) = hdr->getExtNextType();
			p += 1;
		}
	}

	return hdrLen;
}

unsigned GTPSerializer::serializeIE(unsigned char version, GTPInfoElem *ie, unsigned char *buf) {
	unsigned ieLen;
	unsigned char *p = buf;

	if (version == GTP_V2) {
		ieLen = ie->getValueArraySize() + GTPV2_IE_HEADER_SIZE;

		buf = (unsigned char*)calloc(ieLen, sizeof(unsigned char));

		*((unsigned char*)(p)) = ie->getType();
		p += 1;
		*((unsigned short*)p) = ntohs(ie->getValueArraySize());
		p += 2;
		*((unsigned char*)p) = ie->getInstance();
		p += 1;

		for (unsigned i = 0; i < ie->getValueArraySize(); i++)
			p[i] = ie->getValue(i);
	} else if (version == GTP_V1) {
		ieLen = ie->getValueArraySize() + ((ie->getType() & 0x80) ? GTPV1_IE_HEADER_MAX_SIZE : GTPV1_IE_HEADER_MIN_SIZE);

		buf = (unsigned char*)calloc(ieLen, sizeof(unsigned char));

		*((unsigned char*)(p)) = ie->getType();
		p += 1;
		if (ie->getType() & 0x80) {
			if (ie->getType() == GTPV1_ExtHdrTypeList) {
				*((unsigned char*)p) = ie->getValueArraySize();
				p += 1;
			} else {
				*((unsigned short*)p) = ntohs(ie->getValueArraySize());
				p += 2;
			}
		}
		for (unsigned i = 0; i < ie->getValueArraySize(); i++)
			p[i] = ie->getValue(i);
	}

	return ieLen;
}

unsigned GTPSerializer::serialize(GTPMessage *msg, unsigned char *buf, unsigned bufsize) {
	unsigned hdrLen = serializeHeader(msg, buf);
	unsigned msgLen = hdrLen;

	for (unsigned i = 0; i < msg->getIesArraySize(); i++) {
		GTPInfoElem *ie = msg->getIes(i);
		msgLen += serializeIE(msg->getHeader()->getVersion(), ie, buf + msgLen);
	}

	return msgLen;
}

unsigned GTPSerializer::parseIE(unsigned char version, unsigned char *buf, GTPInfoElem *ie) {
	unsigned char *p = buf;

	if (version == GTP_V2) {
		ie->setType(*((unsigned char*)(p)));
		p += 1;
		ie->setValueArraySize(ntohs(*((unsigned short*)(p))));
		p += 2;
		ie->setInstance(*((unsigned char*)(p)));
		p += 1;

		for (unsigned i = 0; i < ie->getValueArraySize(); i++)
			ie->setValue(i, *((char*)(p++)));
	}
	return p - buf;
}
