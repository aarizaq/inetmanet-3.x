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

#ifndef PERENCODER_H_
#define PERENCODER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "ASNTypes.h"

#define BIN		0
#define HEX		1

#define ALIGNED		0
#define UNALIGNED	1

/*
 * Class for encoding ASN.1 types. The encoder will produce a buffer according to PER
 * rules. It will start with an empty buffer and add bytes to it according to desired
 * ASN.1 types, which are to be encoded.
 */
class PerEncoder {
private:
    short usedBits;
    int64_t length;
    char *buffer;
    bool alignmentFlag;

    /* Utility methods for encoding */
    bool encodeConstrainedValue(int64_t lowerBound, int64_t upperBound, int64_t value);
    bool encodeUnconstrainedValue(int64_t value);
    void encodeSmallBitString(char *value, unsigned char length);
    void encodeBytes(const char *value, int64_t length);
    bool encodeSmallNumber(int64_t value);
    bool encodeLength(int64_t length, int64_t lowerBound, int64_t upperBound);
    void encodeBits(char value, unsigned char length);
    void encodeValue(int64_t value, int64_t length);

public:
	PerEncoder(bool alignment);
	virtual ~PerEncoder();

	/* Encoding methods */
	bool encodeAbstractType(const AbstractType& abstractType);
	bool encodeOpenType(const OpenType& openType);
	bool encodeBoolean(const Boolean& boolean);
	bool encodeNull(const Null& null);
	bool encodeInteger(const IntegerBase& integer);
	bool encodeEnumerated(const EnumeratedBase& enumerated);
	bool encodeBitString(const BitStringBase& bitString);
	bool encodeOctetString(const OctetStringBase& octetString);
	bool encodeSequence(const Sequence& sequence);
	bool encodeSequenceOf(const SequenceOfBase& sequenceOf);
	bool encodeChoice(const Choice& choice);
	bool encodePrintableString(const PrintableStringBase& printableString);

	void print(bool type);

	/* Getter methods */
    char *getBuffer() { return buffer; }
    char getByteAt(int64_t index) { return buffer[index]; }
    int64_t getLength() { return length; }

	/* Setter methods */
	void setBuffer(char *buffer) { this->buffer = buffer; }
	void setLength(int64_t length) { this->length = length; }

};

#endif /* PERENCODER_H_ */
