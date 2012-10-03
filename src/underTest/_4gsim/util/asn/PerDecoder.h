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

#ifndef PERDECODER_H_
#define PERDECODER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "ASNTypes.h"

/*
 * Class for decoding an ASN.1 PER encoded byte string. The class will parse the
 * buffer and decode each required ASN.1 type, moving the iterator and changing
 * the number of left bits in the iterator byte if necessary, accordingly. During the
 * decoding the class will store the information in the desired ASN.1 object.
 * More information about decoding rules can be found in X691 specification.
 */
class PerDecoder {
private:
    char *buffer;
    int64_t it;
    short leftBits;

    /* Utility methods for decoding. */
    int64_t decodeConstrainedValue(int64_t lowerBound, int64_t upperBound);
    int64_t decodeUnconstrainedValue();
    int64_t decodeValue(int64_t length);
    int64_t decodeSmallNumber();
    int64_t decodeLength(int64_t lowerBound, int64_t upperBound);
    char *decodeSmallBitString(unsigned char length);
    char *decodeBytes(int64_t length);
    unsigned char decodeBits(unsigned char length);

    /* Utility methods. */
    void allignIterator();

public:
	PerDecoder(char *val);
	virtual ~PerDecoder();

	/* Decoding methods. */
	bool decodeAbstractType(AbstractType& abstractType);
	bool decodeOpenType(OpenType& openType);
	bool decodeBoolean(Boolean& boolean);
	bool decodeNull(Null& null);
	bool decodeInteger(IntegerBase& integer);
	bool decodeEnumerated(EnumeratedBase& enumerated);
	bool decodeBitString(BitStringBase& bitString);
	bool decodeOctetString(OctetStringBase& octetString);
	bool decodeSequence(Sequence& sequence);
	bool decodeSequenceOf(SequenceOfBase& sequenceOf);
	bool decodeChoice(Choice& choice);
	bool decodePrintableString(PrintableStringBase& printableString);

	int64_t decodeSequenceOfSize(SequenceOfBase& sequenceOf);
	bool decodeChoiceValue(Choice& choice);

};

#endif /* PERDECODER_H_ */
