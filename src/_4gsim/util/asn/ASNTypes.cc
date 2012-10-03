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

#include "ASNTypes.h"
#include "PerEncoder.h"
#include "PerDecoder.h"
#include <map>
#include <assert.h>
#include <iostream>
#include <string>

int64_t countBits(int64_t value, int64_t count) {
	count++;
	value = value / 2;
	if (value)
		return countBits(value, count);
	else
		return count;
}

unsigned char bitMask(unsigned char start, unsigned char end) {
    unsigned char mask = ((1 << (end - start)) - 1);
    return mask << start;
}

const OpenType::Info OpenType::theInfo = {
		&OpenType::create,
		OPENTYPE,
		0,
};

OpenType::OpenType(char *value, int64_t length, const void *info) : AbstractType(info) {
	this->value = value;
	this->length = length;
}

OpenType::OpenType(AbstractType *val, const void *info) : AbstractType(info) {
	PerEncoder enc = PerEncoder(ALIGNED);
	enc.encodeAbstractType(*val);
	this->length = enc.getLength();
	this->value = enc.getBuffer();
}

OpenType& OpenType::operator=(const OpenType& other)  {
	this->length = other.length;
	if (length) {
		value = (char*)calloc(sizeof(char), length);
		memcpy(value, other.value, length);
	} else
		value = NULL;

	return *this;
}

int64_t OpenType::compare(const AbstractType& other) const {
	const OpenType& that = dynamic_cast<const OpenType&>(other);
	int64_t length1 = length;
	int64_t length2 = that.length;
	if (length1 > length2)
		return 1;
	if (length1 < length2)
		return -1;

	int64_t dif;
	char *value1 = value;
	char *value2 = that.value;
	for (; length1; ++value1, --length1, ++value2) {
		dif = *value1 - *value2;
		if (dif != 0) return dif;
	}
	return 0;
}

bool OpenType::decode(char *buffer) {
	return PerDecoder(buffer).decodeOpenType(*this);
}

bool OpenType::encode(PerEncoder &encoder) const {
	return encoder.encodeOpenType(*this);
}

const Boolean::Info Boolean::theInfo = {
        &Boolean::create,
        _BOOLEAN,
        0,
};

Boolean::Boolean(bool value, const void *info) : AbstractType(info) {
    this->value = value;
}

Boolean& Boolean::operator=(const Boolean& other)  {
    this->value = other.value;
    return *this;
}

int64_t Boolean::compare(const AbstractType& other) const {
    const Boolean& that = dynamic_cast<const Boolean&>(other);

    if (value == that.value)
        return 1;
    else
        return -1;

}

bool Boolean::decode(char *buffer) {
    return PerDecoder(buffer).decodeBoolean(*this);
}

bool Boolean::encode(PerEncoder &encoder) const {
    return encoder.encodeBoolean(*this);
}

const Null::Info Null::theInfo = {
        &Null::create,
        _NULL,
        0,
};

Null& Null::operator=(const Null& other)  {
    return *this;
}

int64_t Null::compare(const AbstractType& other) const {
    if (typeid(other) != typeid(Null))
        return -1;
    return 0;
}

bool Null::decode(char *buffer) {
    return PerDecoder(buffer).decodeNull(*this);
}

bool Null::encode(PerEncoder &encoder) const {
    return encoder.encodeNull(*this);
}


const IntegerBase::Info IntegerBase::theInfo = {
		&IntegerBase::create,
		INTEGER,
		0,
		UNCONSTRAINED,
		0,
		INT_MAX
};

IntegerBase& IntegerBase::operator=(const IntegerBase& other) {
	setValue(other.value);
	return *this;
}

IntegerBase& IntegerBase::operator=(int64_t value) {
    setValue(value);
    return *this;
}

int64_t IntegerBase::compare(const AbstractType& other) const {
	const IntegerBase& that = dynamic_cast<const IntegerBase&>(other);
	if (getLowerBound() >= 0)
		return value - that.value;

	return (int64_t)value - (int64_t)that.value;
}

bool IntegerBase::decode(char *buffer) {
	return PerDecoder(buffer).decodeInteger(*this);
}

bool IntegerBase::encode(PerEncoder &encoder) const {
	return encoder.encodeInteger(*this);
}

EnumeratedBase& EnumeratedBase::operator=(const EnumeratedBase& other) {
	assert(getInfo() == other.getInfo());
	setValue(other.value);
	return *this;
}

int64_t EnumeratedBase::compare(const AbstractType& other) const {
	const EnumeratedBase& that = *static_cast<const EnumeratedBase*>(&other);
	assert(info == that.info); // compatible type check
	return value - that.value;
}

bool EnumeratedBase::decode(char *buffer) {
	return PerDecoder(buffer).decodeEnumerated(*this);
}

bool EnumeratedBase::encode(PerEncoder &encoder) const {
	return encoder.encodeEnumerated(*this);
}

const BitStringBase::Info BitStringBase::theInfo = {
	&BitStringBase::create,
	BITSTRING,
	0,
	UNCONSTRAINED,
	0,
	INT_MAX
};

BitStringBase::BitStringBase(const void *info) : ConstrainedType(info) {
	length = getUpperBound() == getLowerBound() ? getUpperBound() : 0;
	value = getUpperBound() == getLowerBound() ? (char*)calloc((getUpperBound() + 7) / 8, sizeof(char)) : NULL;
}

BitStringBase& BitStringBase::operator=(const BitStringBase& other) {
	length = other.length;
	if (length) {
		value = (char*)calloc(sizeof(char), (length + 7) / 8);
		memcpy(value, other.value, (length + 7) / 8);
	} else
		value = NULL;
	return *this;
}

bool BitStringBase::getBit(int64_t index) const {
	int64_t bytePos;
	unsigned char bitPos;

	if (index >= length) {
		printf("Index out of bounds\n");
		return 0;
	} else {
		bytePos = index / 8;
		bitPos = (bytePos + 1) * 8 - index - 1;
		if (!(value[bytePos] & (1 << bitPos)))
			return 0;
		else
			return 1;
	}
}

void BitStringBase::setBit(int64_t index, bool bit) {
	int64_t bytePos;
	unsigned char bitPos;

	if (index >= length) {
		printf("Index out of bounds\n");
	} else {
		bytePos = index / 8;
		bitPos = (bytePos + 1) * 8 - index - 1;
		value[bytePos] |= (1 << bitPos);
	}
}

//unsigned BitString::getShiftedData(char *&shiftedBuffer)
//{
//	int64_t i = 0;
//	int64_t numBytes = (numBits + 7) / 8;
//	short shift = numBytes * 8 - numBits;
//	shiftedBuffer = (char*)calloc(numBytes, sizeof(char*));
//
//	while (i < numBytes) {
//		shiftedBuffer[i] += data[i] >> shift;
//		i++;
//		shiftedBuffer[i] = data[i] & ((2 ^ (8 - shift)) - 1);
//	}
//	return numBytes;
//}

int64_t BitStringBase::resize(int64_t length) {
	int64_t bytesNr = (length + 7) / 8;

	if ((length < getLowerBound()) || ((getConstraintType() == CONSTRAINED) && (length > getUpperBound()))) {
		printf("New size value outside constraints\n");
		return -1;
	}
	this->length = length;
	char *newValue = (char*)calloc(bytesNr, sizeof(char));
	memcpy(newValue, value, bytesNr);
	newValue[bytesNr - 1] &= 0xff - ((2 ^ (bytesNr * 8 - length)) - 1);

	return length;
}

int64_t BitStringBase::compare(const AbstractType& other) const {
	const BitStringBase& that = dynamic_cast<const BitStringBase&>(other);
	int64_t bytesNr = ((length + 7) / 8) < ((that.length + 7) / 8) ? ((length + 7) / 8) : ((that.length + 7) / 8);
	for (int64_t i = 0 ; i < bytesNr; i++) {
		char mask = value[i] ^ that.value[i]; // find the first byte which differs
	    if (mask != 0)
	    	return (value[i] & mask) - (that.value[i] & mask);
	}
	return length - that.length;
}

bool BitStringBase::decode(char *buffer) {
	return PerDecoder(buffer).decodeBitString(*this);
}

bool BitStringBase::encode(PerEncoder &encoder) const {
	return encoder.encodeBitString(*this);
}

void BitStringBase::print() {
	if (value == NULL)
		return;

	for (int64_t i = 0; i < length; i++) {
		int64_t bytePos = i / 8;
		if (value[bytePos] & (1 << (7 - i % 8)))
			printf("1");
		else
			printf("0");
		printf(" ");
	}

	printf("\n");
}

const OctetStringBase::Info OctetStringBase::theInfo = {
	&OctetStringBase::create,
	OCTETSTRING,
	0,
	UNCONSTRAINED,
	0,
	INT_MAX
};


OctetStringBase::OctetStringBase(const void *info) : ConstrainedType(info) {
	length = getUpperBound() == getLowerBound() ? getUpperBound() : 0;
	value = getUpperBound() == getLowerBound() ? (char*)calloc(getUpperBound(), sizeof(char)) : NULL;
}

OctetStringBase& OctetStringBase::operator=(const OctetStringBase& other) {
	this->length = other.length;
	if (length) {
		value = (char*)calloc(sizeof(char), length);
		memcpy(value, other.value, length);
	} else
		value = NULL;
	return *this;
}

bool OctetStringBase::decode(char *buffer) {
	return PerDecoder(buffer).decodeOctetString(*this);
}

bool OctetStringBase::encode(PerEncoder &encoder) const {
	return encoder.encodeOctetString(*this);
}

int64_t OctetStringBase::compare(const AbstractType& other) const {
	const OctetStringBase& that = dynamic_cast<const OctetStringBase&>(other);
	int64_t length1 = length;
	int64_t length2 = that.length;
	if (length1 > length2)
		return 1;
	if (length1 < length2)
		return -1;

	int64_t dif;
	char *value1 = value;
	char *value2 = that.value;
	for (; length1; ++value1, --length1, ++value2) {
		dif = *value1 - *value2;
		if (dif != 0) return dif;
	}
	return 0;
}

const PrintableStringBase::Info PrintableStringBase::theInfo = {
	&PrintableStringBase::create,
	PRINTABLESTRING,
	0,
	UNCONSTRAINED,
	0,
	INT_MAX
};

PrintableStringBase& PrintableStringBase::operator=(const PrintableStringBase& other) {
	value = other.value;
	return *this;
}

bool PrintableStringBase::decode(char *buffer) {
	return PerDecoder(buffer).decodePrintableString(*this);
}

bool PrintableStringBase::encode(PerEncoder &encoder) const {
	return encoder.encodePrintableString(*this);
}

int64_t PrintableStringBase::compare(const AbstractType& other) const {
	const PrintableStringBase& that = dynamic_cast<const PrintableStringBase&>(other);
	return value.compare(that.value);
}

Sequence::Sequence(const void *info) : AbstractType(info) {
	optFlags = (char*)calloc((getInfo()->sizeOpt + 7) / 8, sizeof(char));
	extFlags = (char*)calloc((getInfo()->sizeExt + 7) / 8, sizeof(char));

	for (int64_t i = 0; i < getInfo()->sizeRoot; i++)
		items.push_back(AbstractType::create(getInfo()->itemsInfo[i]));
	for (int64_t i = 0; i < getInfo()->sizeExt; i++)
		items.push_back(AbstractType::create(getInfo()->itemsInfo[i + getInfo()->sizeRoot]));
}

Sequence::Sequence(const Sequence& other) : AbstractType(other) {
	optFlags = (char*)calloc((other.getInfo()->sizeOpt + 7) / 8, sizeof(char));
	memcpy(optFlags, other.optFlags, (other.getInfo()->sizeOpt + 7) / 8);
	extFlags = (char*)calloc((other.getInfo()->sizeExt + 7) / 8, sizeof(char));
	memcpy(extFlags, other.extFlags, (other.getInfo()->sizeExt + 7) / 8);
	for (int64_t i = 0; i < other.getLength(); i++) {
		AbstractType *item = other.at(i);
		items.push_back(item->clone());
	}
//	for (int64_t i = 0; i < sequence.extSize(); i++) {
//		AbstractType *extItem = sequence.getExtItem(i);
//		rootItems.push_back(new AbstractType(*extItem));
//	}
}

bool Sequence::getOptFlag(int64_t index) const {
	int64_t bytePos;
	unsigned char bitPos;

	if (index >= getInfo()->sizeOpt) {
		printf("Index out of bounds\n");
		return 0;
	} else {
		bytePos = index / 8;
		bitPos = (bytePos + 1) * 8 - index - 1;
		if (!(optFlags[bytePos] & (1 << bitPos)))
			return 0;
		else
			return 1;
	}
}

void Sequence::setOptFlag(int64_t index, bool bit) {
	int64_t bytePos;
	unsigned char bitPos;

	if (index >= getInfo()->sizeOpt) {
		printf("Index out of bounds\n");
	} else {
		bytePos = index / 8;
		bitPos = (bytePos + 1) * 8 - index - 1;
		optFlags[bytePos] |= (1 << bitPos);
	}
}

bool Sequence::decode(char *buffer) {
	return PerDecoder(buffer).decodeSequence(*this);
}

bool Sequence::encode(PerEncoder &encoder) const {
	return encoder.encodeSequence(*this);
}

int64_t Sequence::compare(const AbstractType& other) const {
	const Sequence& that = dynamic_cast<const Sequence&>(other);
	assert(info == that.info);

	int64_t optIndex = 0, result = 0;
	int64_t i;
	for (i = 0; i < getInfo()->sizeRoot; i++) {
		bool presence = getInfo()->itemsPres[i];
	    if (presence || (getOptFlag(optIndex) && that.getOptFlag(optIndex)))
	    	result = items[i]->compare(*that.items[i]);
	    else
	    	result = getOptFlag(optIndex) - that.getOptFlag(optIndex);
	    if (!presence)
	    	optIndex++;
	    if (result)
	    	return result;
	}

//	for (; i < items.size(); ++i) {
//		bool presence = getInfo()->itemsPres[i];
//	    if (getOptFlag(optIndex) && that.getOptFlag(optIndex))
//	    	result = items[i]->compare(*that.items[i]);
//	    else
//	    	result = getOptFlag(optIndex) - that.getOptFlag(optIndex);
//	    if (!presence)
//	    	optIndex++;
//	}
	return result;
}

SequenceOfBase::SequenceOfBase(const SequenceOfBase& other) : ConstrainedType(other) {
	for (int64_t i = 0; i < other.size(); i++) {
		AbstractType *item = other.at(i);
		items.push_back(item->clone());
	}
}

AbstractType *SequenceOfBase::createItem() const  {
    const AbstractType::Info* itemInfo =
      static_cast<const AbstractType::Info*>(static_cast<const Info*>(info)->itemInfo);
    return itemInfo->create(itemInfo);
}

bool SequenceOfBase::decode(char *buffer) {
	return PerDecoder(buffer).decodeSequenceOf(*this);
}

bool SequenceOfBase::encode(PerEncoder &encoder) const {
	return encoder.encodeSequenceOf(*this);
}

int64_t SequenceOfBase::compare(const AbstractType& other) const {
	const SequenceOfBase& that = dynamic_cast<const SequenceOfBase&>(other);

	for (int64_t i, j = 0; i != items.size() && j != that.items.size(); i++, j++) {
	    int64_t dif = items[i]->compare(*(that.items[j]));
	    if (dif != 0)
	    	return dif;
	}
	return items.size() - that.items.size();
}

Choice::Choice(const void *info, int64_t choice, AbstractType *val) : AbstractType(info) {
	this->choice = choice;
	this->value = value;
}

Choice::Choice(const Choice& other) : AbstractType(other) {
	this->choice = other.choice;
	value = other.value == NULL ? NULL : other.value->clone();
}

void Choice::createValue() {
	if (choice > -1 && choice <= getUpperBound()) {
		const Info *choiceInfo = static_cast<const Info*>(getInfo()->choicesInfo[choice]);
		value = choiceInfo->create(choiceInfo);
	}
}

bool Choice::decode(char *buffer) {
	return PerDecoder(buffer).decodeChoice(*this);
}

bool Choice::encode(PerEncoder &encoder) const {
	return encoder.encodeChoice(*this);
}

int64_t Choice::compare(const AbstractType& other) const {
	const Choice& that = dynamic_cast<const Choice&>(other);
	if (choice >= 0 && choice == that.choice)
	    return value->compare(*that.value);
	return choice - that.choice;
}
