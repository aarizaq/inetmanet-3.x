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

#ifndef LTEUTILS_H_
#define LTEUTILS_H_

#define PLMNID_CODED_SIZE			3
#define PLMNID_UNCODED_SIZE			6
#define CELLID_UNCODED_SIZE			7
#define CELLID_CODED_SIZE			4		// only half of last byte
#define TAC_UNCODED_SIZE			4
#define TAC_CODED_SIZE				2
#define IMSI_UNCODED_MIN_SIZE		15
#define IMSI_UNCODED_MAX_SIZE		16
#define IMSI_CODED_SIZE				8
#define MSISDN_UNCODED_SIZE			12
#define MSISDN_CODED_SIZE			6

#define ENB_CHANNEL					10

#define TBCD_TYPE	0
#define ASCII_TYPE	1

#include <omnetpp.h>

enum RATType {
	UTRAN = 1,
	GERAN = 2,
	WLAN = 3,
	GAN = 4,
	HSPAEvolution = 5,
	EUTRAN = 6
};

class LTEUtils {
public:
	LTEUtils();
	virtual ~LTEUtils();

	char *toIMSI(std::string imsi);
	char *toPLMNId(std::string mcc, std::string mnc);
	char *toByteString(std::string buf, unsigned size);
	char *toTBCDString(std::string buf, unsigned size);
	std::string toASCIIString(const char *buf, unsigned size, bool type = TBCD_TYPE);
	void printBits(const char *buf, unsigned size);
	void printBytes(const char *buf, unsigned size);
};

#endif /* LTEUTILS_H_ */
