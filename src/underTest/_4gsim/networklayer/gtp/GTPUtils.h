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

#ifndef GTPUTILS_H_
#define GTPUTILS_H_

#include "GTPMessage_m.h"
#include "IPvXAddress.h"
#include "TunnelEndpoint.h"
#include "GTP.h"

/*
 * GTP utility class. It is used for creating and processing GTP header and GTP
 * information elements.
 */
class GTPUtils {
public:
	GTPUtils();
	virtual ~GTPUtils();

    /*
     * Methods for GTP header and GTP information element creation.
     */
	GTPv2Header *createHeader(unsigned char type, bool p, bool t, unsigned int teid, unsigned int seqNr);
	GTPv1Header *createHeader(unsigned char type, bool pt, bool e, bool s, bool pn, unsigned int teid, unsigned int seqNr, unsigned char npduNr, unsigned char extNextType, std::vector<GTPv1Extension> exts);
	GTPInfoElem *createIE(unsigned char type, unsigned short length, unsigned char instance, char *value);
	GTPInfoElem *createIE(unsigned char type, unsigned char instance, std::string value);
	GTPInfoElem *createIE(unsigned char version, unsigned char type, unsigned char instance, char value);
	GTPInfoElem *createIntegerIE(unsigned char type, unsigned char instance, unsigned value);
	GTPInfoElem *createCauseIE(unsigned short length, unsigned char instance, unsigned char cause, bool pce = false, bool bce = false, bool cs = false, unsigned char ieType = 0, unsigned char ieInstance = 0);
    GTPInfoElem *createAddrIE(unsigned char type, unsigned char instance, IPvXAddress addr);
    GTPInfoElem *createFteidIE(unsigned char instance, unsigned teid, char type, IPvXAddress addr);
    GTPInfoElem *createGroupedIE(unsigned char type, unsigned char instance, unsigned length, std::vector<GTPInfoElem*>ies);

    /*
     * Methods for GTP header and GTP information element processing. They return the
     * payload taken from the GTP information element (unsigned, integer, char etc
     * values).
     */
	unsigned char processCauseIE(GTPInfoElem *cause);
    char *processIE(GTPInfoElem *ie);
	IPvXAddress processAddrIE(GTPInfoElem *addr);
	TunnelEndpoint *processFteidIE(GTPInfoElem *fteid);
	std::vector<GTPInfoElem*> processGroupedIE(GTPInfoElem *ie);

	/*
	 * Method for returning the length of a GTPv1 information element based on its type.
	 */
	unsigned getGTPv1InfoElemLen(unsigned char type);
};

#endif /* GTPUTILS_H_ */
