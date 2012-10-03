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
#include "DiameterUtils.h"
#include "DiameterSerializer.h"
#include <stdio.h>
#include <stdlib.h>

DiameterUtils::DiameterUtils() {
	// TODO Auto-generated constructor stub

}

DiameterUtils::~DiameterUtils() {
	// TODO Auto-generated destructor stub
}

DiameterHeader DiameterUtils::createHeader(unsigned commandCode, bool reqFlag, bool prxyFlag, bool errFlag, bool retrFlag, unsigned applId, unsigned hopByHopId, unsigned endToEndId) {
	DiameterHeader hdr = DiameterHeader();
	hdr.setCommandCode(commandCode);
	hdr.setReqFlag(reqFlag);
	hdr.setPrxyFlag(prxyFlag);
	hdr.setErrFlag(errFlag);
	hdr.setRetrFlag(retrFlag);
	hdr.setApplId(applId);
	hdr.setHopByHopId(hopByHopId);
	hdr.setEndToEndId(endToEndId);
	return hdr;
}

AVP *DiameterUtils::createBaseAVP(unsigned avpCode, bool vendFlag, bool manFlag, bool privFlag, unsigned vendorId) {
	AVP *ret = new AVP(avpName(avpCode));
	ret->setAvpCode(avpCode);
	ret->setManFlag(manFlag);
	ret->setVendFlag(vendFlag);
	ret->setPrivFlag(privFlag);
	ret->setVendorId(vendorId);
	return ret;
}

AVP *DiameterUtils::createOctetStringAVP(unsigned avpCode, bool vendFlag, bool manFlag, bool privFlag, unsigned vendorId, unsigned len, const char *str) {
	AVP *ret = createBaseAVP(avpCode, vendFlag, manFlag, privFlag, vendorId);
	ret->setValueArraySize(len);
	for (unsigned i = 0; i < ret->getValueArraySize(); i++) ret->setValue(i, str[i]);
	return ret;
}
AVP *DiameterUtils::createUTF8StringAVP(unsigned avpCode, bool vendFlag, bool manFlag, bool privFlag, unsigned vendorId, std::string str) {
	return createOctetStringAVP(avpCode, vendFlag, manFlag, privFlag, vendorId, str.size(), str.data());
}

AVP *DiameterUtils::createAddressAVP(unsigned avpCode, bool vendFlag, bool manFlag, bool privFlag, unsigned vendorId, IPvXAddress addr) {
	AVP *ret = createBaseAVP(avpCode, vendFlag, manFlag, privFlag, vendorId);
	if (addr.isIPv6()) {

	} else {
		unsigned short type = IPV4_ADDRESS_TYPE;
		unsigned len = IPV4_ADDRESS_SIZE + sizeof(unsigned short);
		char *buf = (char*)calloc(len, sizeof(char));
		char *p = buf;
		ret->setValueArraySize(len);
		*((unsigned short*)p) = ntohs(type);
		p += 2;
		*((unsigned*)p) = ntohl(addr.get4().getInt());
		p += 4;
		for (unsigned i = 0; i < ret->getValueArraySize(); i++) ret->setValue(i, buf[i]);

	}
	return ret;
}

AVP *DiameterUtils::createUnsigned32AVP(unsigned avpCode, bool vendFlag, bool manFlag, bool privFlag, unsigned vendorId, unsigned val) {
	AVP *ret = createBaseAVP(avpCode, vendFlag, manFlag, privFlag, vendorId);
	char* buf = (char*)malloc(sizeof(unsigned));
	char *p = buf;
	*((unsigned*)p) = ntohl(val);
	p += 4;
	ret->setValueArraySize(sizeof(unsigned));
	for (unsigned i = 0; i < ret->getValueArraySize(); i++) ret->setValue(i, buf[i]);
	return ret;
}

AVP *DiameterUtils::createInteger32AVP(unsigned avpCode, bool vendFlag, bool manFlag, bool privFlag, unsigned vendorId, int val) {
	AVP *ret = createBaseAVP(avpCode, vendFlag, manFlag, privFlag, vendorId);
	char* buf = (char*)malloc(sizeof(unsigned));
	char *p = buf;
	*((int*)p) = ntohl(val);
	p += 4;
	ret->setValueArraySize(sizeof(int));
	for (unsigned i = 0; i < ret->getValueArraySize(); i++) ret->setValue(i, buf[i]);
	return ret;
}

AVP *DiameterUtils::createGroupedAVP(unsigned avpCode, bool vendFlag, bool manFlag, bool privFlag, unsigned vendorId, std::vector<AVP*> avps) {
	AVP *ret = createBaseAVP(avpCode, vendFlag, manFlag, privFlag, vendorId);
	unsigned len = 0;
	for (unsigned i = 0; i < avps.size(); i++) {
		len = (((len - 1) >> 2) + 1) << 2;
		len += (avps.at(i))->getValueArraySize() + ((avps.at(i))->getVendFlag() ? AVP_MAX_HEADER_SIZE : AVP_MIN_HEADER_SIZE);
	}
	char *buf = (char*)calloc(len, sizeof(char));
	char *p = buf;
	for (unsigned i = 0; i < avps.size(); i++) {
		unsigned tmp = p - buf;
		p += ((((tmp - 1) >> 2) + 1) << 2) - tmp;	// skip padding bytes
		AVP *avp = avps[i];
		p += DiameterSerializer().serializeAVP(avp, p);
	}
	ret->setValueArraySize(len);
	for (unsigned i = 0; i < ret->getValueArraySize(); i++) ret->setValue(i, buf[i]);
	return ret;
}

unsigned DiameterUtils::processUnsigned32AVP(AVP *unsigned32AVP) {
	char *buf = processOctetStringAVP(unsigned32AVP);
	return ntohl(*((unsigned*)buf));
}

int DiameterUtils::processInteger32AVP(AVP *integer32AVP) {
	char *buf = processOctetStringAVP(integer32AVP);
	return ntohl(*((int*)buf));
}

char *DiameterUtils::processOctetStringAVP(AVP *octetStringAVP) {
	char *buf = (char*)calloc(octetStringAVP->getValueArraySize() + 1, sizeof(char));
	for (unsigned i = 0; i < octetStringAVP->getValueArraySize(); i++)
		buf[i] = octetStringAVP->getValue(i);
	return buf;
}

std::string DiameterUtils::processUTF8String(AVP *utf8StringAVP) {
	return processOctetStringAVP(utf8StringAVP);
}

IPvXAddress DiameterUtils::processAddressAVP(AVP *addressAVP) {
	char *buf = processOctetStringAVP(addressAVP);
	char *p = buf;
	IPvXAddress address;
	unsigned short type = ntohs(*((unsigned short*)(p)));
	p += 2;
	if (type == IPV4_ADDRESS_TYPE) {
		address = IPvXAddress(IPv4Address(ntohl(*((unsigned*)(p)))));
//		p += 4;
	} else {

	}
	return address;
}

std::vector<AVP*> DiameterUtils::processGroupedAVP(AVP *groupedAVP) {
	char *buf = processOctetStringAVP(groupedAVP);
	char *p = buf;
	std::vector<AVP*> avps;
	char *end = p + groupedAVP->getValueArraySize();
	while (p < end) {
		unsigned tmp = p - buf;
		p += ((((tmp - 1) >> 2) + 1) << 2) - tmp;	// skip padding bytes
		AVP *avp = new AVP();
		p += DiameterSerializer().parseAVP(avp, p);
		avps.push_back(avp);
	}

	return avps;
}

AVP *DiameterUtils::findAVP(unsigned avpCode, std::vector<AVP*>avps) {
	for (unsigned i = 0; i < avps.size(); i++) {
		if (avps[i]->getAvpCode() == avpCode)
			return avps[i];
	}
	return NULL;
}

std::vector<AVP*> DiameterUtils::findAVPs(unsigned avpCode, std::vector<AVP*>avps) {
	std::vector<AVP*> ret;
	for (unsigned i = 0; i < avps.size(); i++) {
		if (avps[i]->getAvpCode() == avpCode)
			ret.push_back(avps[i]);
	}
	return ret;
}

const char *DiameterUtils::avpName(unsigned avpCode) {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (avpCode) {
        CASE(AVP_UserName);
        CASE(AVP_HostIPAddress);
        CASE(AVP_AuthApplId);
        CASE(AVP_VendorSpecApplId);
        CASE(AVP_SessionId);
        CASE(AVP_OriginHost);
        CASE(AVP_SuppVendorId);
        CASE(AVP_ResultCode);
        CASE(AVP_ProductName);
        CASE(AVP_DisconnectCause);
        CASE(AVP_AuthSessionState);
        CASE(AVP_DestinationRealm);
        CASE(AVP_DestinationHost);
        CASE(AVP_OriginRealm);
        CASE(AVP_ExpResultCode);
        CASE(AVP_MIPHomeAgentAddr);
        CASE(AVP_MIP6AgentInfo);
        CASE(AVP_ServiceSelection);
        CASE(AVP_MSISDN);
        CASE(AVP_ServPartyIPAddr);
        CASE(AVP_RATType);
        CASE(AVP_SubscriptionData);
        CASE(AVP_ULRFlags);
        CASE(AVP_ULAFlags);
        CASE(AVP_VisitedPLMNId);
        CASE(AVP_ContextIdentifier);
        CASE(AVP_AllAPNConfigInclInd);
        CASE(AVP_APNConfigProfile);
        CASE(AVP_APNConfig);
        CASE(AVP_PDNGWAllocType);
        CASE(AVP_PDNType);
    }
    return s;
#undef CASE
}

void DiameterUtils::deleteGroupedAVP(std::vector<AVP*> avps) {
	std::vector<AVP*>::iterator i = avps.begin();
	for (;i != avps.begin() + avps.size(); ++i) {
		delete *i;
	}
	avps.erase(avps.begin(), avps.begin() + avps.size());
}
