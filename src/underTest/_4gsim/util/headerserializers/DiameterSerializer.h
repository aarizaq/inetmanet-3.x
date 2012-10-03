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

#ifndef DIAMETERSERIALIZER_H_
#define DIAMETERSERIALIZER_H_

#include "DiameterMessage.h"
#include "SCTPMessage_m.h"

/*
 * Class for serializing and parsing of Diameter messages.
 * This class will be used to serialize a Diameter message before sending it
 * to SCTP layer and parsing the message when it comes from SCTP layer.
 */
class DiameterSerializer {
private:
    /*
     * Method returns the length of the Diameter message. The length includes
     * the header, all the AVPs (header + payload) and offset bytes, if it is
     * needed.
     */
	unsigned getMessageLength(DiameterMessage *msg);

	/*
	 * Methods for serializing and parsing of Diameter header. They return the
	 * serialized or parsed Diameter header length.
	 */
	unsigned serializeHeader(DiameterHeader hdr, unsigned len, char *p);
	unsigned parseHeader(DiameterHeader *hdr, char *p);
public:
	DiameterSerializer();
	virtual ~DiameterSerializer();

	/*
	 * Methods for serializing and parsing of Diameter AVP. They return the
	 * serialized or parsed AVP length (header + payload).
	 */
	unsigned serializeAVP(AVP *avp, char *p);
	unsigned parseAVP(AVP *avp, char *p);

	/*
	 * Methods for serializing and parsing of Diameter Message. Serialize method
	 * will generate directly a SCTP message with the resulted bytes and parse
	 * method will process a SCTP message and generate the appropriate Diameter message.
	 */
    SCTPSimpleMessage *serialize(DiameterMessage *msg);
    DiameterMessage *parse(const SCTPSimpleMessage *msg);
};

#endif /* DIAMETERSERIALIZER_H_ */
