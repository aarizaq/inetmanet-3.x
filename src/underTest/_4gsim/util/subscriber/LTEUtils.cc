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

#include "LTEUtils.h"

LTEUtils::LTEUtils() {
	// TODO Auto-generated constructor stub

}

LTEUtils::~LTEUtils() {
	// TODO Auto-generated destructor stub
}

char *LTEUtils::toIMSI(std::string imsi) {
	if (imsi.size() != IMSI_UNCODED_MIN_SIZE)
		return toTBCDString(imsi, IMSI_UNCODED_MAX_SIZE);
	else {
		char *plmnId = toPLMNId(imsi.substr(0, 3), imsi.substr(3, 2));
		char *msin = toTBCDString(imsi.substr(5, 10), 10);
		char *imsi = (char*)calloc(IMSI_CODED_SIZE, sizeof(char));
		memcpy(imsi, plmnId, PLMNID_CODED_SIZE);
		memcpy(imsi + PLMNID_CODED_SIZE, msin, 5);
		return imsi;
	}
}

char *LTEUtils::toPLMNId(std::string mcc, std::string mnc) {
	char *mcc_ = toTBCDString(mcc, 3);
	char *mnc_ = toTBCDString(mnc, 3);
	char *ret = (char *)calloc(PLMNID_CODED_SIZE, sizeof(char));
	ret[0] = mcc_[0];
	ret[1] = mcc_[1];
	ret[1] += (mnc_[0] & 0x0f) ? ((mnc_[0] << 4) & 0xf0) : 0xf0;
	ret[2] = ((mnc_[0] >> 4) & 0x0f) + ((mnc_[1] << 4) & 0xf0);
	return ret;
}

char *LTEUtils::toByteString(std::string buf, unsigned size) {
	if (buf.size() > size)
		buf = buf.substr(0, size);
	char *ret = (char *)calloc((size + 1) / 2, sizeof(char));
	unsigned j = (size - buf.size()) / 2;
	for (unsigned i = 0; i < buf.size(); i++) {
		ret[j] += (buf[i] - 48) << (4 * ((size - buf.size() + i + 1) % 2));
		j = j + ((size - buf.size() + i) % 2);
	}
	return ret;
}

char *LTEUtils::toTBCDString(std::string buf, unsigned size) {
	char *ret = toByteString(buf, size);
	for (unsigned i = 0; i < (size + 1) / 2; i++)
		ret[i] = ((ret[i] >> 4) & 0x0f) + ((ret[i] << 4) & 0xf0);
	return ret;
}

std::string LTEUtils::toASCIIString(const char *buf, unsigned size, bool type) {
	if (type == TBCD_TYPE) {
		if (buf == NULL)
			return "";
		unsigned len = buf[size - 1] == 0xf0 ? size * 2 - 1: size * 2;
		std::string ret = "";
		unsigned j = 0;
		for (unsigned i = 0; i < len; i++) {
			if (((buf[j]) >> (4 * (i % 2)) & 0x0f) < 10)
				ret += ((buf[j]) >> (4 * (i % 2)) & 0x0f) + 48;
			j = j + i % 2;
		}
		return ret;
	} else {
		if (buf == NULL)
			return "";
		unsigned len = buf[size - 1] == 0xf0 ? size * 2 - 1: size * 2;
		std::string ret = "";
		unsigned j = 0;
		for (unsigned i = 0; i < len; i++) {
			ret += ((buf[j]) >> (4 * ((i + 1) % 2)) & 0x0f) + 48;
			j = j + i % 2;
		}
		return ret;
	}
}

void LTEUtils::printBits(const char *buf, unsigned size) {
	if (buf == NULL)
		return;
	for (unsigned i = 0; i < size; i++) {
		for (short j = 7; j >= 0; j--) {
			if ((buf[i] & (1 << j)) == 0) EV << "0";
			else EV << "1";
		}
		EV << endl;
	}
	EV << endl;
}

void LTEUtils::printBytes(const char *buf, unsigned size) {
	if (buf == NULL)
		return;
    char str[size * 3 + 1];
    int j = 0;
    for (unsigned i = 0; i < size; i++, j += 3)
        sprintf(&str[j], " %02x ", (unsigned char)buf[i]);
    str[size * 3] = '\0';
    EV << str << endl;
}
