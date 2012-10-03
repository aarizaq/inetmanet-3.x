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

#include "PerDecoder.h"

PerDecoder::PerDecoder(char *buffer)  {
	this->buffer = buffer;
	this->it = 0;
	leftBits = 8;
}

PerDecoder::~PerDecoder() {

}

int64_t PerDecoder::decodeConstrainedValue(int64_t lowerBound, int64_t upperBound) {
    int64_t range = upperBound - lowerBound + 1;
    int64_t length;
    int64_t value;

    if (range < 2) {
        return 0;
    } else if (range < 256) {
        int64_t numBits = countBits(range - 1, 0);
        value = (decodeBits(numBits) >> (8 - numBits)) + lowerBound;
    } else if (range < 65537) {
        length = (countBits(range - 1, 0) + 7) / 8;
        value = decodeValue(length) + lowerBound;
    } else {
        length = decodeLength((countBits(lowerBound, 0) + 7) / 8, (countBits(upperBound, 0) + 7) / 8);
        value = decodeValue(length) + lowerBound;
    }
    return value;
}

int64_t PerDecoder::decodeUnconstrainedValue() {
    return decodeValue(decodeLength(0, INT_MAX));
}

char *PerDecoder::decodeSmallBitString(unsigned char length) {
    char *buffer = (char*)calloc((length + 7) / 8, sizeof(char));
    if (length < 9) {
        *buffer = decodeBits(length);
    } else {
        *buffer = decodeBits(8);
        *(buffer + 1) = decodeBits(length - 8);
    }
    return buffer;
}

char *PerDecoder::decodeBytes(int64_t length) {
    // gets the buffer from iterator position and moves the iterator
    allignIterator();

    char *buffer = (char *)calloc(sizeof(char), length);
    memcpy(buffer, this->buffer + it, length);
    it += length;

    return buffer;
}

unsigned char PerDecoder::decodeBits(unsigned char length) {
    // returns a maximum of 8 bits from the tail of the buffer
    // the bits are shifted 1100....
    unsigned char value = 0;

    // if it has to read bits also from the next byte in the buffer
    if (leftBits - length < 0) {

        // takes the bits from the current byte and moves the iterator
        value = ((unsigned char)this->buffer[it++] << (8 - leftBits));

        // takes the bits also from the next byte
        value += ((unsigned char)this->buffer[it] >> leftBits);

        // changes the number of left bits in the iterator byte
        leftBits = (8 - (length - leftBits));

    // all the bits are in the same byte
    } else {
        value = (unsigned char)((this->buffer[it] & bitMask(leftBits - length, leftBits)) << (8 - leftBits));
        leftBits -= length;

        // if there are no more bits to read in the iterator byte
        // reset the number of left bits to 8 and move the iterator
        if (!leftBits) {
            leftBits = 8;
            it++;
        }
    }
    return value;
}

int64_t PerDecoder::decodeValue(int64_t length) {
    int64_t val = 0;
    allignIterator();

    for (int i = length - 1; i >= 0; i--) {
        val += ((1 << (8 * i))) * ((int64_t )*(this->buffer + it++));
    }
    return val;
}

int64_t PerDecoder::decodeSmallNumber() {
    int64_t value;

    if ((*(this->buffer + it) << (8 - leftBits)) < 128) {
        value = (decodeBits(7) >> 1);
    } else {
        it++;
        value = decodeLength(0, INT_MAX);
    }
    return value;
}

int64_t PerDecoder::decodeLength(int64_t lowerBound, int64_t upperBound) {
    int64_t length;

    if (upperBound > 65535) {
        allignIterator();
        if ((unsigned char)*(buffer + it) < 128) {
            length = decodeValue(1);
        } else if ((unsigned char)*(buffer + it) < 192) {
            length = decodeValue(2) - 32768;
        } else { /* FIXME */
            while ((unsigned char)*(buffer + it) > 191) {
                length += (decodeValue(1) - 192) * 16384;
            }
            length += decodeLength(0, INT_MAX);
        }
    } else {
        length = decodeConstrainedValue(lowerBound, upperBound);
    }

    return length;

}

void PerDecoder::allignIterator() {
    if (leftBits != 8) {
        it++;
        leftBits = 8;
    }
}


bool PerDecoder::decodeAbstractType(AbstractType& abstractType) {
	switch(abstractType.getTag()) {
		case INTEGER:
			return decodeInteger(static_cast<IntegerBase&>(abstractType));
		case ENUMERATED:
			return decodeEnumerated(static_cast<EnumeratedBase&>(abstractType));
		case BITSTRING:
			return decodeBitString(static_cast<BitStringBase&>(abstractType));
		case OCTETSTRING:
			return decodeOctetString(static_cast<OctetStringBase&>(abstractType));
		case _BOOLEAN:
		    return decodeBoolean(static_cast<Boolean&>(abstractType));
		case _NULL:
		    return decodeNull(static_cast<Null&>(abstractType));
		case SEQUENCE:
			return decodeSequence(static_cast<Sequence&>(abstractType));
		case SEQUENCEOF:
			return decodeSequenceOf(static_cast<SequenceOfBase&>(abstractType));
		case CHOICE:
			return decodeChoice(static_cast<Choice&>(abstractType));
		case PRINTABLESTRING:
			return decodePrintableString(static_cast<PrintableStringBase&>(abstractType));
		case OPENTYPE:
			return decodeOpenType(static_cast<OpenType&>(abstractType));
		default:
			return false;
	}
	return true;
}

bool PerDecoder::decodeOpenType(OpenType& openType) {
	openType.setLength(decodeLength(0, INT_MAX));
	openType.setValue(buffer + it);
	it += openType.getLength();
	return true;
}

bool PerDecoder::decodeBoolean(Boolean& boolean) {
    boolean.setValue(decodeBits(1));
    return true;
}

bool PerDecoder::decodeNull(Null& null) {
    return true;
}

bool PerDecoder::decodeInteger(IntegerBase& integer) {
	if ((integer.isExtendable() && decodeBits(1))
			|| integer.getConstraintType() > EXTCONSTRAINED)
		integer.setValue(decodeUnconstrainedValue());
	else
		integer.setValue(decodeConstrainedValue(integer.getLowerBound(), integer.getUpperBound()));
	return true;
}

bool PerDecoder::decodeEnumerated(EnumeratedBase& enumerated) {
	if ((enumerated.isExtendable() && decodeBits(1)))
		enumerated.setValue(decodeSmallNumber());
	else
		enumerated.setValue(decodeConstrainedValue(0, enumerated.getUpperBound()));
	return true;
}

bool PerDecoder::decodeBitString(BitStringBase& bitString) {
	if ((bitString.isExtendable() && decodeBits(1))
			|| (bitString.getLowerBound() != bitString.getUpperBound())
			|| (bitString.getUpperBound() > 65536)
			|| (bitString.getConstraintType() == UNCONSTRAINED)) {
		int64_t upperBound = bitString.getConstraintType() == UNCONSTRAINED ? INT_MAX : bitString.getUpperBound();
		bitString.setLength(decodeLength(bitString.getLowerBound(), upperBound));
		bitString.setValue(decodeBytes((bitString.getLength() + 7) / 8));
	} else {
		if (!bitString.getLength()) {
			return true;
		} else if (bitString.getLength() < 17) {
			bitString.setValue(decodeSmallBitString(bitString.getLength()));
		} else {
			bitString.setValue(decodeBytes((bitString.getLength() + 7) / 8));
		}
	}
	return true;
}

bool PerDecoder::decodeOctetString(OctetStringBase& octetString) {
	if ((octetString.isExtendable() && decodeBits(1))
			|| (octetString.getLowerBound() != octetString.getUpperBound())
			|| (octetString.getUpperBound() > 65536)
			|| (octetString.getConstraintType() == UNCONSTRAINED)) {
		int64_t upperBound = octetString.getConstraintType() == UNCONSTRAINED ? INT_MAX : octetString.getUpperBound();
		octetString.setLength(decodeLength(octetString.getLowerBound(), upperBound));
		octetString.setValue(decodeBytes(octetString.getLength()));
	} else {
		if (!octetString.getUpperBound()) {
			return true;
		} else if (octetString.getUpperBound() < 3) {
			octetString.setValue(decodeSmallBitString(octetString.getUpperBound() * 8));
		} else {
			octetString.setValue(decodeBytes(octetString.getUpperBound()));
		}
	}
	return true;
}

bool PerDecoder::decodeSequence(Sequence& sequence) {
    // TODO add support for extension
	bool hasExtension = false;
	if (sequence.isExtendable() && decodeBits(1))
		hasExtension = true;

	char *optFlags = sequence.getOptFlags();
	for (int64_t i = 0; i < sequence.getInfo()->sizeOpt; i += 8) {
		if (sequence.getInfo()->sizeOpt - i < 8) {
			optFlags[i / 8] = decodeBits(sequence.getInfo()->sizeOpt - i);
			break;
		} else
			optFlags[i / 8] = decodeBits(8);
	}
	int64_t optIndex = 0;
	for (int64_t i = 0; i < sequence.getInfo()->sizeRoot; i++) {
		if (sequence.isOptional(i)) {
			if (!sequence.getOptFlag(optIndex++))
				continue;
		}
		if (!decodeAbstractType(*(sequence.at(i))))
			return false;
	}

	if (hasExtension) {

	}
///*
//	if ((!sequence.getExtBit().isEmpty()) &&
//			sequence.getExtBit().get(0)) {
//
//		extSize = decodeSmallNumber();
//		extPresent = new BitString(CONSTRAINED, extSize, extSize, extSize);
//
//		if (!decodeBitString(extPresent))
//			return false;
//
//		sequence.setExtPresent(extPresent);
//
//		for (unsigned i = 0; i < extSize; i++)
//			if (!decodeAbstractType(sequence.getExtItem(i))) // trebuie OpenType
//				return false;
//
//	}
//*/
	return true;
}

int64_t PerDecoder::decodeSequenceOfSize(SequenceOfBase& sequenceOf) {
	int64_t size = 0;
	int64_t upperBound = sequenceOf.getUpperBound();

	if ((sequenceOf.getLowerBound() != sequenceOf.getUpperBound()) || (sequenceOf.getUpperBound() > 65536)) {
		if (sequenceOf.getConstraintType() == EXTCONSTRAINED && decodeBits(1))
			upperBound = INT_MAX;
		size = decodeLength(sequenceOf.getLowerBound(), upperBound);
	} else {
		size = sequenceOf.getUpperBound();
	}
	return size;
}

bool PerDecoder::decodeSequenceOf(SequenceOfBase& sequenceOf) {
	int64_t size = 0;

	if ((size = decodeSequenceOfSize(sequenceOf)) == -1)
		return false;

	for (int64_t i = 0; i < size; i++) {
		sequenceOf.push_back(sequenceOf.createItem());
		if (!decodeAbstractType(*(sequenceOf.at(i))))
			sequenceOf.pop_back();
	}
	return true;
}

bool PerDecoder::decodeChoiceValue(Choice& choice) {
	if (choice.isExtendable() && decodeBits(1)) {
		choice.createChoice(decodeSmallNumber());
	} else
		choice.createChoice(decodeConstrainedValue(0, choice.getUpperBound()));
	return true;
}

bool PerDecoder::decodeChoice(Choice& choice) {
	// TODO add support for extension
	if (!decodeChoiceValue(choice))
		return false;

	bool isExtension = choice.getChoice() > choice.getUpperBound() ? true : false;

	if ((choice.getTag() == CONSTRAINED) || (!isExtension)) {
		choice.createValue();
		if (!decodeAbstractType(*(choice.getValue())))
			return false;
	} else {
//		OpenType *openType = new OpenType();
//		openType.setEmpty(0);
//
//		if (!decodeOpenType(openType))
//			return false;
//
//		PerDecoder *perDec = new PerDecoder(openType.getValue());
//
//		if (!perDec.decodeAbstractType(choice.getItem()))
//			return false;
	}
	return true;
}

bool PerDecoder::decodePrintableString(PrintableStringBase& printableString) {
	if ((printableString.isExtendable() && decodeBits(1))
			|| (printableString.getLowerBound() != printableString.getUpperBound())
			|| (printableString.getUpperBound() > 65536)
			|| (printableString.getConstraintType() == UNCONSTRAINED)) {
		int64_t upperBound = printableString.getConstraintType() == UNCONSTRAINED ? INT_MAX : printableString.getUpperBound();
		int64_t len = decodeLength(printableString.getLowerBound(), upperBound);
		printableString.setValue(decodeBytes(len));
	} else {
		printableString.setValue(decodeBytes(printableString.getUpperBound()));
	}

	return true;
}
