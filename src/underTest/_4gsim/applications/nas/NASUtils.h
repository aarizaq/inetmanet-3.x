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

#ifndef NASUTILS_H_
#define NASUTILS_H_

#include "NASMessage_m.h"

/*
 * NAS utility class. It is used for creating and processing NAS header
 * and IEs.
 */
class NASUtils {
public:
	NASUtils();
	virtual ~NASUtils();

    /*
     * Methods for NAS header and IE creation.
     */
	NASHeader createHeader(unsigned char epsBearerId, unsigned char secHdrType, unsigned char protDiscr, unsigned char procTransId, unsigned char msgType);
	NASInfoElem createIE(unsigned char format, unsigned char ieType, unsigned char type, char value);
	NASInfoElem createIE(unsigned char format, unsigned char ieType, unsigned char type, unsigned short length, const char *value);
	NASInfoElem createEPSMobileIdIE(unsigned char format, unsigned char typeOfId, char *id);

    /*
     * Methods for NAS header and IE processing. They return the payload taken
     * from the IE (unsigned, integer, char etc.).
     */
	char *processIE(NASInfoElem ie);
	unsigned processEPSMobileIdIE(NASInfoElem ie, char *&id);

    /*
     * Method for printing the message contents. Currently it prints info
     * only for the header.
     */
	void printMessage(NASPlainMessage *msg);
};

#endif /* NASUTILS_H_ */
