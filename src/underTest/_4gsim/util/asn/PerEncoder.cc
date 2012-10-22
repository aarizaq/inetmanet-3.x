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

#include "PerEncoder.h"
#include <omnetpp.h>

PerEncoder::PerEncoder(bool alignment) {
	usedBits = 8;
	length = 0;
	buffer = NULL;
	alignmentFlag = alignment;
}

PerEncoder::~PerEncoder() {

}

bool PerEncoder::encodeConstrainedValue(int64_t lowerBound, int64_t upperBound, int64_t value) {

    int64_t range = upperBound - lowerBound + 1;
    value -= lowerBound;
    int64_t bitCount;

    if (range < 2) {
        return true;
    } else if (range < 256) {
        bitCount = countBits(range - 1, 0);
        value <<= (8 - bitCount);
        encodeBits(value, bitCount);
    } else if (range < 65537) {
        bitCount = (countBits(range - 1, 0) + 7) / 8;
        encodeValue(value, bitCount);
    } else {
        bitCount = (countBits(value, 0) + 7) / 8;
        if (!encodeLength(bitCount, (countBits(lowerBound, 0) + 7) / 8, (countBits(upperBound, 0) + 7) / 8))
            return false;
        encodeValue(value, bitCount);
    }
    return true;
}

bool PerEncoder::encodeUnconstrainedValue(int64_t value) {

    int64_t bitCount = (countBits(value, 0) + 7) / 8;

    if (!encodeLength(bitCount, 0, INT_MAX))
        return false;

    encodeValue(value, bitCount);

    return true;
}

void PerEncoder::encodeSmallBitString(char *value, unsigned char length) {

    if (length < 9) {
        encodeBits(*value, length);
    } else {
        encodeBits(*value, 8);
        encodeBits(*(value + 1), length - 8);
    }
}

void PerEncoder::print(bool type) {
    if (type == BIN) {
//      for (int64_t i = 0; i < len; i++) {
//          for (short j = 7; j >= 0; j--) {
//              if ((data[i] & (1 << j)) == 0)
//                  printf("0");
//              else
//                  printf("1");
//          }
//          printf(" ");
//      }
    } else {
        char buffer[length * 3 + 1];
        int j = 0;
        for (int i = 0; i < length; i++, j += 3)
            sprintf(&buffer[j], " %02x ", (unsigned char)this->buffer[i]);
        buffer[length * 3] = '\0';
        EV << buffer;
    }
    EV << endl;
}

void PerEncoder::encodeBytes(const char *value, int64_t length) {
    // appends the bytes to the buffer and increases the size of it
	if (alignmentFlag == ALIGNED) {
		this->buffer = (char *)realloc(this->buffer, this->length + length);
		memcpy(this->buffer + this->length, value, length);
		this->length += length;
		usedBits = 8;
	} else {
		for (int64_t i = 0; i < length; i++) {
			encodeBits(value[i], 8);
			this->length++;
		}
	}
}

bool PerEncoder::encodeSmallNumber(int64_t value) {
    if (value) {
        value = value << 1;
        encodeBits((unsigned char )value, 7);
    } else {
        encodeBits(128, 1);
        if (encodeLength(value, 0, INT_MAX) < 0)
            return false;
    }
    return true;
}

bool PerEncoder::encodeLength(int64_t length, int64_t lowerBound, int64_t upperBound) {
    if (upperBound > 65535) {
        if (length < 128) {
            encodeValue(length, 1);
        } else if (length < 16384) {
            length += 32768;
            encodeValue(length, 2);
        } else {
            for (unsigned char f = 4; f > 0; f--) {
                while((length - f * 16384) >= 0) {
                    length -= f * 16384;
                    char tmp = (char )(192 + f);
                    encodeBytes(&tmp, 1);
                }
            }
            encodeLength(length, lowerBound, upperBound);
        }
    } else {
        return encodeConstrainedValue(lowerBound, upperBound, length);
    }
    return true;
}

void PerEncoder::encodeBits(char value, unsigned char length) {

    // if there are 8 bits available in the last byte copy the
    // value in this byte and set the number of used bits
    if (usedBits == 8) {
        this->buffer = (char *)realloc(this->buffer, this->length + 1);
        memcpy(this->buffer + this->length++, &value, 1);
        usedBits = length;
        return;

    // if there are not enough bits in the last byte split the value
    } else if (usedBits + length > 8) {
        // put the first part in the last byte
        this->buffer[this->length - 1] += ((unsigned char)value >> usedBits);

        // put the remaining part in the next byte after the buffer is increased
        // and set the number of used bits according to this part
        unsigned char tmpBits = length - (8 - usedBits);
        value = (unsigned char)value << (length - tmpBits);
        this->buffer = (char *)realloc(this->buffer, this->length + 1);
        memcpy(this->buffer + this->length++, &value, 1);
        usedBits = tmpBits;

    // if the value fits in the last byte put it and increase
    // the number of used bits in the last byte
    } else {
        this->buffer[this->length - 1] += ((unsigned char)value >> usedBits);
        usedBits += length;
    }
}

void PerEncoder::encodeValue(int64_t value, int64_t size) {
    for (int i = size - 1; i >= 0; i--) {
        char tmp = (char)(value >> i * 8);
        encodeBytes(&tmp, 1);
    }
}

bool PerEncoder::encodeAbstractType(const AbstractType& abstractType) {

	switch(abstractType.getTag()) {
		case INTEGER:
			return encodeInteger(static_cast<const IntegerBase&>(abstractType));
		case ENUMERATED:
			return encodeEnumerated(static_cast<const EnumeratedBase&>(abstractType));
		case BITSTRING:
			return encodeBitString(static_cast<const BitStringBase&>(abstractType));
		case OCTETSTRING:
			return encodeOctetString(static_cast<const OctetStringBase&>(abstractType));
		case _BOOLEAN:
		    return encodeBoolean(static_cast<const Boolean&>(abstractType));
		case _NULL:
		    return encodeNull(static_cast<const Null&>(abstractType));
		case SEQUENCE:
			return encodeSequence(static_cast<const Sequence&>(abstractType));
		case SEQUENCEOF:
			return encodeSequenceOf(static_cast<const SequenceOfBase&>(abstractType));
		case CHOICE:
			return encodeChoice(static_cast<const Choice&>(abstractType));
		case PRINTABLESTRING:
			return encodePrintableString(static_cast<const PrintableStringBase&>(abstractType));
		case OPENTYPE:
			return encodeOpenType(static_cast<const OpenType&>(abstractType));
		default:
			return false;
	}
	return true;
}

bool PerEncoder::encodeOpenType(const OpenType& openType) {

	if (!encodeLength(openType.getLength(), 0, INT_MAX))
		return false;

	encodeBytes(openType.getValue(), openType.getLength());

	return true;
}

bool PerEncoder::encodeBoolean(const Boolean& boolean) {
    encodeBits(boolean.getValue(), 1);
    return true;
}

bool PerEncoder::encodeNull(const Null& null) {
    return true;
}

bool PerEncoder::encodeInteger(const IntegerBase& integer) {

	bool isExtension = (integer.getValue() > integer.getUpperBound());
	if (integer.isExtendable()) {
		encodeBits(isExtension << 7, 1);
	}

	if ((integer.isExtendable() && isExtension))
		return encodeUnconstrainedValue(integer.getValue());
	else
		return encodeConstrainedValue(integer.getLowerBound(), integer.getUpperBound(), integer.getValue());
}

bool PerEncoder::encodeEnumerated(const EnumeratedBase& enumerated) {

	bool isExtension = (enumerated.getValue() > enumerated.getUpperBound());
	if (enumerated.isExtendable()) {
		encodeBits(isExtension << 7, 1);
	}

	if (enumerated.isExtendable() && enumerated.getValue() > enumerated.getUpperBound())
		return encodeSmallNumber(enumerated.getValue());
	else
		return encodeConstrainedValue(0, enumerated.getUpperBound(), enumerated.getValue());
}

bool PerEncoder::encodeBitString(const BitStringBase& bitString) {

	int64_t bytesNr = 0;
	bool isExtension = (bitString.getLength() > bitString.getUpperBound());
	if (bitString.isExtendable()) {
		encodeBits(isExtension << 7, 1);
	}

	if ((bitString.getLowerBound() != bitString.getUpperBound())
			|| (bitString.getUpperBound() > 65536)
			|| (bitString.isExtendable() && (isExtension))
			|| (bitString.getConstraintType() == UNCONSTRAINED)) {
		if (!encodeLength(bitString.getLength(), bitString.getLowerBound(), bitString.getTag() != UNCONSTRAINED ? bitString.getUpperBound() : INT_MAX))
			return false;
		bytesNr = (bitString.getLength() + 7) / 8;
		encodeBytes(bitString.getValue(), bytesNr);
		usedBits = bytesNr * 8 - bitString.getLength();
	} else {
		if (!bitString.getLength()) {
			return true;
		} else if (bitString.getLength() < 17) {
			encodeSmallBitString(bitString.getValue(), bitString.getLength());
		} else {
			bytesNr = (bitString.getLength() + 7) / 8;
			encodeBytes(bitString.getValue(), bytesNr);
			usedBits = bytesNr * 8 - bitString.getLength();
		}
	}

	return true;
}

bool PerEncoder::encodeOctetString(const OctetStringBase& octetString) {

	bool isExtension = (octetString.getLength() > octetString.getUpperBound());
	if (octetString.isExtendable()) {
		encodeBits(isExtension << 7, 1);
	}

	if ((octetString.getLowerBound() != octetString.getUpperBound())
			|| (octetString.getUpperBound() > 65536)
			|| (octetString.isExtendable() && (isExtension))
			|| (octetString.getConstraintType() == UNCONSTRAINED)) {
		if (!encodeLength(octetString.getLength(), octetString.getLowerBound(), octetString.getTag() != UNCONSTRAINED ? octetString.getUpperBound() : INT_MAX)) {
			return false;
		}
		encodeBytes(octetString.getValue(), octetString.getLength());
	} else {
		if (!octetString.getLength()) {
			return true;
		} else if (octetString.getLength() < 3) {
			encodeSmallBitString(octetString.getValue(), octetString.getLength() * 8);
		} else {
			encodeBytes(octetString.getValue(), octetString.getLength());
		}

	}
	return true;
}

bool PerEncoder::encodeSequence(const Sequence& sequence) {
    // TODO add support for extension
	if (sequence.isExtendable()) {
		char extFlag = 0x00;
		char *extFlags = sequence.getExtFlags();
		for (int64_t i = 0; i < sequence.getInfo()->sizeExt; i += 8)
			if (extFlags[i / 8])
				extFlag = 0x80;
		encodeBits(extFlag, 1);
	}

	char *optFlags = sequence.getOptFlags();
	for (int64_t i = 0; i < sequence.getInfo()->sizeOpt; i += 8) {
		if (sequence.getInfo()->sizeOpt - i < 8) {
			encodeBits(optFlags[i / 8], sequence.getInfo()->sizeOpt - i);
			break;
		} else
			encodeBits(optFlags[i / 8], 8);
	}
	int64_t optIndex = 0;
	for (int64_t i = 0; i < sequence.getInfo()->sizeRoot; i++) {
		if (sequence.isOptional(i)) {
			if (!sequence.getOptFlag(optIndex++))
				continue;
		}
		if (!encodeAbstractType(*(sequence.at(i))))
			return false;
	}
/*
	if (!sequence.getExtPresent().isEmpty()) {

		if (!encodeSmallNumber(sequence.sizeExt()))
				return false;

		if (!encodeBitString(sequence.getExtPresent()))
			return false;

		for (unsigned i = 0; i < sequence.sizeExt(); i++)
			if (!encodeAbstractType(sequence.getExtItem(i))) // trebuie OpenType
				return false;
	}
*/
	return true;
}

bool PerEncoder::encodeSequenceOf(const SequenceOfBase& sequenceOf) {

	bool isExtension = (sequenceOf.size() > sequenceOf.getUpperBound());
	if (sequenceOf.isExtendable()) {
		encodeBits(isExtension << 7, 1);
	}

	if ((sequenceOf.getLowerBound() != sequenceOf.getUpperBound())
			|| (sequenceOf.getUpperBound() > 65536)) {
		if (!encodeLength(sequenceOf.size(), sequenceOf.getLowerBound(), isExtension ? INT_MAX : sequenceOf.getUpperBound()))
			return false;
	}

	for (int64_t i = 0; i < sequenceOf.size(); i++)
		if (!encodeAbstractType(*(sequenceOf.at(i))))
			return false;

	return true;
}

bool PerEncoder::encodeChoice(const Choice& choice) {

	bool isExtension = (choice.getChoice() > choice.getUpperBound());
	if (choice.isExtendable()) {
		encodeBits(isExtension << 7, 1);
	}

	if (choice.isExtendable() && isExtension) {

		if (!encodeSmallNumber(choice.getChoice()))
			return false;

		OpenType openType = (choice.getValue());

		if (!encodeOpenType(openType))
			return false;
	} else {
		if (!encodeConstrainedValue(0, choice.getUpperBound(), choice.getChoice()))
			return false;

		if (!encodeAbstractType(*(choice.getValue())))
			return false;
	}

	return true;
}

bool PerEncoder::encodePrintableString(const PrintableStringBase& printableString) {

	bool isExtension = (printableString.getLength() > printableString.getUpperBound());
	if (printableString.isExtendable()) {
		encodeBits(isExtension << 7, 1);
	}

	if ((printableString.getLowerBound() != printableString.getUpperBound())
			|| (printableString.getUpperBound() > 65536)) {
		if (!encodeLength(printableString.getLength(), printableString.getLowerBound(), isExtension ? INT_MAX : printableString.getUpperBound()))
			return false;
	}

	encodeBytes(printableString.getValue().c_str(), printableString.getLength());

	return true;
}


