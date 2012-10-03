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
#include "NASSerializer.h"
#include "NASUtils.h"

NASSerializer::NASSerializer() {
	// TODO Auto-generated constructor stub
	shift = 0;
}

NASSerializer::~NASSerializer() {
	// TODO Auto-generated destructor stub
}

unsigned NASSerializer::getMessageLength(NASPlainMessage *msg) {
	unsigned len = msg->getHdr().getMsgType() & 0x80 ? NAS_ESM_HEADER_SIZE : NAS_EMM_HEADER_SIZE;
	for (unsigned i = 0; i < msg->getIesArraySize(); i++) {
		NASInfoElem ie = msg->getIes(i);
		switch(ie.getIeType()) {
			case IEType1:
				if (ie.getFormat() == IE_TV)
					len++;
				else if (ie.getFormat() == IE_V) {
					len += (shift + 1) % 2;
					shift = (shift + 1) % 2;
				}
				break;
			case IEType2:
				len++;
				break;
			case IEType3:
				len += ie.getValueArraySize();
				if (ie.getFormat() == IE_TV)
					len++;
				break;
			case IEType4:
				len += ie.getValueArraySize() + 1;
				if (ie.getFormat() == IE_TLV)
					len++;
				break;
			case IEType6:
				len += ie.getValueArraySize() + 2;
				if (ie.getFormat() == IE_TLVE)
					len++;
				break;
			default:
			    break;
		}
	}
	return len;
}

unsigned NASSerializer::serializeHeader(NASHeader hdr, char *p) {

	if (hdr.getMsgType() & 0x80) {	// is ESM message
		*((unsigned char*)(p++)) = (hdr.getEpsBearerId() << 4) | (hdr.getProtDiscr());
		*((unsigned char*)p++) = hdr.getProcTransId();
	} else	{						// for EMM message
		*((unsigned char*)(p++)) = (hdr.getSecHdrType() << 4) | (hdr.getProtDiscr());
	}
	*((unsigned char*)p++) = hdr.getMsgType();
	return hdr.getLength();
}

NASHeader NASSerializer::parseHeader(char *p) {
	NASHeader hdr = NASHeader();
	hdr.setProtDiscr(*((unsigned char*)(p)) & 0x0f);
	if (hdr.getProtDiscr() == ESMMessage) {	// is ESM message
		hdr.setEpsBearerId((*((unsigned char*)(p++)) >> 4) & 0x0f);
		hdr.setLength(NAS_ESM_HEADER_SIZE);
		hdr.setProcTransId(*((unsigned char*)p++));
	} else if (hdr.getProtDiscr() == EMMMessage) {	// for EMM message
		hdr.setSecHdrType((*((unsigned char*)(p++)) >> 4) & 0x0f);
		hdr.setLength(NAS_EMM_HEADER_SIZE);
	}
	hdr.setMsgType(*((unsigned char*)p++));
	return hdr;
}

unsigned NASSerializer::serializeIE(NASInfoElem ie, char *p) {
	char *begin = p;
	switch(ie.getIeType()) {
		case IEType1:
			if (ie.getFormat() == IE_TV) {
				*((unsigned char*)(p++)) = (ie.getType() << 4) | (ie.getValue(0) & 0x0f);
			} else if (ie.getFormat() == IE_V) { // shifting only for type 1 (after one IE type 1 follows another with type 1)
				*((unsigned char*)(p)) += ((unsigned char)ie.getValue(0)) << (((shift + 1) % 2) * 4);
				p += shift % 2;
				shift = (shift + 1) % 2;
			}
			break;
		case IEType2:
			*((unsigned char*)(p++)) = ie.getType();
			break;
		case IEType3:
			if (ie.getFormat() == IE_TV)
				*((unsigned char*)(p++)) = ie.getType();
			for (unsigned i = 0; i < ie.getValueArraySize(); i++)
				*((char*)(p++)) = ie.getValue(i);
			break;
		case IEType4:
			if (ie.getFormat() == IE_TLV)
				*((unsigned char*)(p++)) = ie.getType();
			*((unsigned char*)(p++)) = ie.getValueArraySize();
			for (unsigned i = 0; i < ie.getValueArraySize(); i++)
				*((char*)(p++)) = ie.getValue(i);
			break;
		case IEType6:
			if (ie.getFormat() == IE_TLVE)
				*((unsigned char*)(p++)) = ie.getType();
			*((unsigned short*)p) = ntohs(ie.getValueArraySize());
			p += 2;
			for (unsigned i = 0; i < ie.getValueArraySize(); i++)
				*((char*)(p++)) = ie.getValue(i);
			break;
		default:
		    break;
	}
	return p - begin;
}

NASInfoElem NASSerializer::parseIE(char *p, unsigned char format, unsigned char ieType, unsigned short length) {
	NASInfoElem ie = NASInfoElem();
	ie.setIeType(ieType);
	ie.setFormat(format);
	ie.setValueArraySize(length);
	char *begin = p;
	switch(ie.getIeType()) {
		case IEType1:
			ie.setValueArraySize(1);
			if (ie.getFormat() == IE_TV) {
				ie.setType((*((unsigned char*)(p)) >> 4) & 0x0f);
				ie.setValue(0, *((unsigned char*)(p++)) & 0x0f);
			} else if (ie.getFormat() == IE_V) { // shifting only for type 1 (after one IE type 1 follows another with type 1)
				ie.setValue(0, *((unsigned char*)(p)) >> (((shift + 1) % 2) * 4) & 0x0f);
				p += shift % 2;
				shift = (shift + 1) % 2;
			}
			break;
		case IEType2:
			ie.setType(*((unsigned char*)(p++)));
			break;
		case IEType3:
			if (ie.getFormat() == IE_TV)
				ie.setType(*((unsigned char*)(p++)));
			for (unsigned i = 0; i < ie.getValueArraySize(); i++)
				ie.setValue(i, *((char*)(p++)));
			break;
		case IEType4:
			if (ie.getFormat() == IE_TLV)
				ie.setType(*((unsigned char*)(p++)));
			ie.setValueArraySize(*((unsigned char*)(p++)));
			for (unsigned i = 0; i < ie.getValueArraySize(); i++)
				ie.setValue(i, *((char*)(p++)));
			break;
		case IEType6:
			if (ie.getFormat() == IE_TLVE)
				ie.setType(*((unsigned char*)(p++)));
			ie.setValueArraySize(ntohs(*((unsigned short*)p)));
			p += 2;
			for (unsigned i = 0; i < ie.getValueArraySize(); i++)
				ie.setValue(i, *((char*)(p++)));
			break;
		default:
		    break;
	}
	skip = p - begin;
	return ie;
}

unsigned NASSerializer::serialize(NASPlainMessage *msg, char *&buffer) {
	NASPlainMessage *encMsg = NULL;
	if (msg->getEncapsulatedPacket() != NULL)
		encMsg = check_and_cast<NASPlainMessage*>(msg->decapsulate());
	unsigned len = getMessageLength(msg);
	int encPos = -1;
	if (encMsg != NULL) {
		len += getMessageLength(encMsg) + 2;
		encPos = msg->getEncapPos();
	}
	buffer = (char*)calloc(len, sizeof(char));
	char *p = buffer;

	p += serializeHeader(msg->getHdr(), p);
	for (unsigned i = 0; i < msg->getIesArraySize(); i++) {
		if ((i == (unsigned)encPos) && (encPos != -1)) {
			char *esmCont;
			unsigned esmContLen = NASSerializer().serialize(encMsg, esmCont);
			NASInfoElem ie = NASUtils().createIE(IE_LVE, IEType6, 0, esmContLen, esmCont);
			p += serializeIE(ie, p);
		}
		p += serializeIE(msg->getIes(i), p);
	}
	delete encMsg;
	return len;
}

bool NASSerializer::parseAttachRequest(NASPlainMessage *msg, char *buf) {
    char *p = buf;
    msg->setIesArraySize(5);
    NASSerializer parser = NASSerializer();

    /* NAS key set identifier */
    msg->setIes(0, parser.parseIE(p, IE_V, IEType1));
    p += parser.skip;

    /* EPS attach type */
    msg->setIes(1, parser.parseIE(p, IE_V, IEType1));
    p += parser.skip;

    /* EPS mobile identity */
    msg->setIes(2, parser.parseIE(p, IE_LV, IEType4));
    p += parser.skip;

    /* UE network capability */
    msg->setIes(3, parser.parseIE(p, IE_LV, IEType4));
    p += parser.skip;

    /* ESM message container */
    msg->setIes(4, parser.parseIE(p, IE_LVE, IEType6));
    p += parser.skip;

    buf = NASUtils().processIE(msg->getIes(4));
    p = buf;
    NASPlainMessage *smsg = new NASPlainMessage();
    NASHeader hdr = NASSerializer().parseHeader(p);
    if (!hdr.getLength()) {
        EV << "NAS: Message decoding error. Dropping message.\n";
        return false;
    }
    p += hdr.getLength();
    smsg->setHdr(hdr);

    if (smsg->getHdr().getMsgType() == PDNConnectivityRequest) {
        smsg->setName("PDN-Connectivity-Request");
        parsePDNConnectivityRequest(smsg, p);
        msg->encapsulate(smsg);
    } else {
        delete smsg;
    }
    return true;
}

void NASSerializer::parsePDNConnectivityRequest(NASPlainMessage *msg, char *buf) {
    char *p = buf;
    msg->setIesArraySize(2);
    NASSerializer parser = NASSerializer();

    /* PDN type */
    msg->setIes(0, parser.parseIE(p, IE_V, IEType1));
    p += parser.skip;

    /* Request type */
    msg->setIes(1, parser.parseIE(p, IE_V, IEType1));
    p += parser.skip;
}

bool NASSerializer::parseAttachAccept(NASPlainMessage *msg, char *buf) {
    char *p = buf;
    msg->setIesArraySize(5);
    NASSerializer parser = NASSerializer();

    /* EPS attach result */
    msg->setIes(0, parser.parseIE(p, IE_V, IEType1));
    p += parser.skip;

    /* Spare half octet */
    msg->setIes(1, parser.parseIE(p, IE_V, IEType1));
    p += parser.skip;

    /* T3412 value */
    msg->setIes(2, parser.parseIE(p, IE_V, IEType3, 1));
    p += parser.skip;

    /* TAI list */
    msg->setIes(3, parser.parseIE(p, IE_LV, IEType4));
    p += parser.skip;

    /* ESM message container */
    msg->setIes(4, parser.parseIE(p, IE_LVE, IEType6));
    p += parser.skip;

    buf = NASUtils().processIE(msg->getIes(4));
    p = buf;
    NASPlainMessage *smsg = new NASPlainMessage();
    NASHeader hdr = NASSerializer().parseHeader(p);
    if (!hdr.getLength()) {
        EV << "NAS: Message decoding error. Dropping message.\n";
        return false;
    }
    p += hdr.getLength();
    smsg->setHdr(hdr);

    if (smsg->getHdr().getMsgType() == ActDefEPSBearerCtxtReq) {
        smsg->setName("Activate-Default-Bearer-Request");
        parseActDefBearerRequest(smsg, p);
        msg->encapsulate(smsg);
    } else {
        delete smsg;
    }

    return true;
}

void NASSerializer::parseActDefBearerRequest(NASPlainMessage *msg, char *buf) {
    char *p = buf;
    msg->setIesArraySize(3);
    NASSerializer parser = NASSerializer();

    /* EPS QoS */
    msg->setIes(0, parser.parseIE(p, IE_LV, IEType4));
    p += parser.skip;

    /* APN */
    msg->setIes(1, parser.parseIE(p, IE_LV, IEType4));
    p += parser.skip;

    /* PDN address */
    msg->setIes(2, parser.parseIE(p, IE_LV, IEType4));
}

bool NASSerializer::parseAttachReject(NASPlainMessage *msg, char *buf) {
    char *p = buf;
    msg->setIesArraySize(1);
    NASSerializer parser = NASSerializer();

    /* T3412 value */
    msg->setIes(0, parser.parseIE(p, IE_V, IEType3, 1));
    p += parser.skip;

    /* ESM message container */
//  msg->setIes(0, parser.parseIE(p, IE_LVE, IEType6));
//  p += parser.skip;
//
//  buf = NASUtils().processIE(msg->getIes(0));
//  p = buf;
//  NASPlainMessage *smsg = new NASPlainMessage();
//  NASHeader hdr = NASSerializer().parseHeader(p);
//  if (!hdr.getLength()) {
//      EV << "NAS: Message decoding error. Dropping message.\n";
//      return false;
//  }
//  p += hdr.getLength();
//  smsg->setHdr(hdr);
//
//  if (smsg->getHdr().getMsgType() == ActDefEPSBearerCtxtAcc) {
//      smsg->setName("Activate-Default-Bearer-Request");
//      NASUtils().parseActDefBearerAccept(smsg, p);
//      msg->encapsulate(smsg);
//  } else {
//      delete smsg;
//  }

    return true;
}

bool NASSerializer::parseAttachComplete(NASPlainMessage *msg, char *buf) {
    char *p = buf;
    msg->setIesArraySize(1);
    NASSerializer parser = NASSerializer();

    /* ESM message container */
    msg->setIes(0, parser.parseIE(p, IE_LVE, IEType6));
    p += parser.skip;

    buf = NASUtils().processIE(msg->getIes(0));
    p = buf;
    NASPlainMessage *smsg = new NASPlainMessage();
    NASHeader hdr = NASSerializer().parseHeader(p);
    if (!hdr.getLength()) {
        EV << "NAS: Message decoding error. Dropping message.\n";
        return false;
    }
    p += hdr.getLength();
    smsg->setHdr(hdr);

    if (smsg->getHdr().getMsgType() == ActDefEPSBearerCtxtAcc) {
        smsg->setName("Activate-Default-Bearer-Request");
        parseActDefBearerAccept(smsg, p);
        msg->encapsulate(smsg);
    } else {
        delete smsg;
    }

    return true;
}
