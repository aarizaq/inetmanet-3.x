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

#ifndef NASSERIALIZER_H_
#define NASSERIALIZER_H_

#include "NASMessage_m.h"

/*
 * Class for serializing and parsing of NAS messages.
 * A NAS message is built from one header and multiple information elements
 * which can of multiple types. More information about this types can be
 * found in the .msg file.
 */
class NASSerializer {
private:
    // only used for NAS IE_V type (total of 4 bits of information) 1001....
	unsigned char shift;

    /*
     * Method returns the length of the NAS message. The length includes
     * the header and all the IEs.
     */
	unsigned getMessageLength(NASPlainMessage *msg);
public:
	 // only used for NAS IE_V type (total of 4 bits of information) ....1001
	unsigned short skip;

	NASSerializer();
	virtual ~NASSerializer();

    /*
     * Methods for serializing and parsing of NAS header. Serialize method
     * returns the header length and parse method return the header.
     */
    unsigned serializeHeader(NASHeader hdr, char *p);
	NASHeader parseHeader(char *p);

    /*
     * Methods for serializing and parsing of NAS IE. Serialize methods
     * returns the IE length and parse method returns the IE depending on
     * some input parameters.
     */
    unsigned serializeIE(NASInfoElem ie, char *p);
	NASInfoElem parseIE(char *p, unsigned char format, unsigned char ieType, unsigned short length = 0);

    /*
     * Method for serializing a NAS message. The serialize method returns the
     * serialized NAS message length.
     */
    unsigned serialize(NASPlainMessage *msg, char *&buffer);

    /*
     * Methods for parsing different types of NAS messages. Parsing methods are needed
     * for each message type because each message has a different form depending
     * on its information elements.
     */
    bool parseAttachRequest(NASPlainMessage *msg, char *buf);
    bool parseAttachAccept(NASPlainMessage *msg, char *buf);
    bool parseAttachReject(NASPlainMessage *msg, char *buf);
    bool parseAttachComplete(NASPlainMessage *msg, char *buf);
    void parsePDNConnectivityRequest(NASPlainMessage *msg, char *buf);
    void parseActDefBearerRequest(NASPlainMessage *msg, char *buf);
    void parseActDefBearerAccept(NASPlainMessage *msg, char *buf) {}
};

#endif /* NASSERIALIZER_H_ */
