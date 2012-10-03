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

#ifndef GTPSERIALIZER_H_
#define GTPSERIALIZER_H_

#include "GTPMessage.h"

/*
 * Class for serializing and parsing of GTP messages. This class covers both
 * GTPv1 and GTPv2 messages. It creates byte strings from GTPMessage objects
 * or GTPMessage objects from byte strings.
 */
class GTPSerializer {
private:
    /*
     * Method for serializing the GTP header. It can serialize both GTPv1 and
     * GTPv2 headers. This class is used only internally by the general serialize
     * method. It return the serialized GTP header length. The method returns
     * the GTP header length.
     */
	unsigned serializeHeader(GTPMessage *msg, unsigned char *buf);

public:
	GTPSerializer();
	virtual ~GTPSerializer();

	/*
	 * Methods for serializing and parsing of GTP information elements. They can
	 * parse and serialize both GTPv1 and GTPv2 information elements. The methods
	 * return the serialized or parsed GTP information element length (header +
	 * payload).
	 */
    unsigned serializeIE(unsigned char version, GTPInfoElem *ie, unsigned char *buf);
    unsigned parseIE(unsigned char version, unsigned char *buf, GTPInfoElem *ie);

    /*
     * Method for serializing a GTP message. The serialize method returns the
     * serialized GTP message length.
     */
	unsigned serialize(GTPMessage *msg, unsigned char *buf, unsigned bufsize);

};

#endif /* GTPSERIALIZER_H_ */
