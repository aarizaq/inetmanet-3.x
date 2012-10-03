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
#include "DiameterSerializer.h"
#include "DiameterUtils.h"

DiameterSerializer::DiameterSerializer() {
	// TODO Auto-generated constructor stub

}

DiameterSerializer::~DiameterSerializer() {
	// TODO Auto-generated destructor stub
}
unsigned DiameterSerializer::getMessageLength(DiameterMessage *msg) {
	unsigned len = DIAMETER_HEADER_SIZE;
	for (unsigned i = 0; i < msg->getAvpsArraySize(); i++) {
		len = (((len - 1) >> 2) + 1) << 2;
		AVP *avp = msg->getAvps(i);
		len += avp->getValueArraySize() + (avp->getVendFlag() ? AVP_MAX_HEADER_SIZE : AVP_MIN_HEADER_SIZE);
	}
	return len;
}

unsigned DiameterSerializer::serializeAVP(AVP *avp, char *p) {

	unsigned len = avp->getValueArraySize() + (avp->getVendFlag() ? AVP_MAX_HEADER_SIZE : AVP_MIN_HEADER_SIZE);
	*((unsigned*)p) = ntohl(avp->getAvpCode());
	p += 4;
	*((unsigned char*)(p)) =
			(avp->getVendFlag() << 7) | (avp->getManFlag() << 6)  | (avp->getPrivFlag() << 5);
	*((unsigned*)p) += htonl(len & 0x00ffffff);
	p += 4;

	if (avp->getVendFlag()) {
		*((unsigned*)p) = ntohl(avp->getVendorId());
	    p += 4;
	}
	for (unsigned i = 0; i < avp->getValueArraySize(); i++)
	 	p[i] = avp->getValue(i);

	return len;
}

unsigned DiameterSerializer::serializeHeader(DiameterHeader hdr, unsigned len, char *p) {
	*((char*)p) = hdr.getVersion();
	*((unsigned*)p) += htonl(len & 0x00ffffff);
	p += 4;
	*((unsigned char*)(p)) =
			(hdr.getReqFlag() << 7) | (hdr.getPrxyFlag() << 6)  | (hdr.getErrFlag() << 5) | (hdr.getRetrFlag() << 4);
	*((unsigned*)p) |= ntohl(hdr.getCommandCode() & 0x00ffffff);
	p += 4;

	*((unsigned*)p) = ntohl(hdr.getApplId());
	p += 4;
	*((unsigned*)p) = ntohl(hdr.getHopByHopId());
	p += 4;
	*((unsigned*)p) = ntohl(hdr.getEndToEndId());
	p += 4;

	return DIAMETER_HEADER_SIZE;
}

SCTPSimpleMessage *DiameterSerializer::serialize(DiameterMessage *msg) {
	unsigned len = getMessageLength(msg);
	char *buf = (char*)calloc(len, sizeof(char));
	char *p = buf;

	p += serializeHeader(msg->getHdr(), len, p);

	for (unsigned i = 0; i < msg->getAvpsArraySize(); i++) {
		unsigned tmp = p - buf;
		p += ((((tmp - 1) >> 2) + 1) << 2) - tmp;	// skip padding bytes
		p += serializeAVP(msg->getAvps(i), p);
	}

	SCTPSimpleMessage *ret = new SCTPSimpleMessage(msg->getName());
    ret->setDataArraySize(len);

    for (unsigned i = 0; i < len; i++)
    {
        ret->setData(i, buf[i]);
    }
    ret->setDataLen(len);
    ret->setByteLength(len);

	return ret;
}

unsigned DiameterSerializer::parseAVP(AVP *avp, char *p) {
	unsigned len;

	avp->setAvpCode(ntohl(*((unsigned*)(p))));
	p += 4;
	avp->setName(DiameterUtils().avpName(avp->getAvpCode()));
	avp->setVendFlag(*((unsigned char*)(p)) & 0x80);
	avp->setManFlag(*((unsigned char*)(p)) & 0x40);
	avp->setPrivFlag(*((unsigned char*)(p)) & 0x20);
	len = ntohl(*((unsigned*)(p))) & 0x00ffffff;
	p += 4;
	if (avp->getVendFlag()) {
		avp->setVendorId(ntohl(*((unsigned*)(p))));
	    p += 4;
	}

	avp->setValueArraySize(len - (avp->getVendFlag() ? AVP_MAX_HEADER_SIZE : AVP_MIN_HEADER_SIZE));
	for (unsigned i = 0; i < avp->getValueArraySize(); i++)
		avp->setValue(i, p[i]);
	return len;
}

unsigned DiameterSerializer::parseHeader(DiameterHeader *hdr, char *p) {

	hdr->setVersion(*((char*)(p)));
	p += 4;
	hdr->setReqFlag(*((unsigned char*)(p)) & 0x80);
	hdr->setPrxyFlag(*((unsigned char*)(p)) & 0x40);
	hdr->setErrFlag(*((unsigned char*)(p)) & 0x20);
	hdr->setRetrFlag(*((unsigned char*)(p)) & 0x10);
	hdr->setCommandCode(ntohl(*((unsigned*)(p))) & 0x00ffffff);
	p += 4;
	hdr->setApplId(ntohl(*((unsigned*)(p))));
	p += 4;
	hdr->setHopByHopId(ntohl(*((unsigned*)(p))));
	p += 4;
	hdr->setEndToEndId(ntohl(*((unsigned*)(p))));
	p += 4;

	return DIAMETER_HEADER_SIZE;
}

DiameterMessage *DiameterSerializer::parse(const SCTPSimpleMessage *msg) {
	char *buf = (char*)calloc(msg->getDataArraySize(), sizeof(char));
	for (unsigned i = 0; i < msg->getDataArraySize(); i++)
		buf[i] = msg->getData(i);
	char *p = buf;
	char *end;
	DiameterMessage *ret = new DiameterMessage();
	DiameterHeader hdr = DiameterHeader();
	p += parseHeader(&hdr, p);
	ret->setHdr(hdr);

	end = buf + msg->getDataArraySize();

	while (p < end) {
		unsigned tmp = p - buf;
		p += ((((tmp - 1) >> 2) + 1) << 2) - tmp;	// skip padding bytes
		AVP *avp = new AVP();
		p += parseAVP(avp, p);
		ret->pushAvp(avp);
	}

	return ret;
}
