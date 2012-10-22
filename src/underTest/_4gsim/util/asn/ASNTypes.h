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

#ifndef ASNTYPES_H
#define ASNTYPES_H

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omnetpp.h>

enum ConstraintType {
	CONSTRAINED			= 0,
	EXTCONSTRAINED		= 1,
	UNCONSTRAINED		= 2,
	SEMICONSTRAINED		= 3,
};

enum ObjectType {
    _BOOLEAN             = 0,
	INTEGER				= 1,
	ENUMERATED			= 2,
	BITSTRING			= 4,
	OCTETSTRING			= 5,
	_NULL               = 6,
	SEQUENCE			= 7,
	SEQUENCEOF			= 8,
	CHOICE				= 11,
	PRINTABLESTRING		= 17,
	OPENTYPE			= 20,
	ABSTRACTTYPELIST	= 21
};

/*
 * Header file for all ASN.1 type implemented. For more details read X691 specification from ITU-T.
 * ASN implementation is based on IIIASN.1 Tool, http://iiiasn1.sourceforge.net/main.html,
 * but only covers PER encoding/decoding, used for S1AP protocol.
 */

/*
 * Method for counting the bits, needed to represent a certain value.
 */
int64_t countBits(int64_t value, int64_t count);

/*
 * Method for returning a bit mask
 */
unsigned char bitMask(unsigned char start, unsigned char end);

class PerEncoder;

/*
 * Base class for ASN.1 types. All ASN.1 types will be derived from this class and they
 * will be differentiated based on the tag value. Information about the particular ASN.1
 * type will be stored in the Info structure defined in each individual class.
 */
class AbstractType {
public:
    /*
     * Method for creating ASN.1 types. The ASN.1 type will be created
     * based on the information provided to this method. This information
     * cannot be changed afterwards for this particular object.
     */
    inline static AbstractType* create(const void* info) { return info ? static_cast<const Info*>(info)->create(info) : 0; }
    typedef AbstractType* (*CreateAbstractType)(const void*);
protected:
    /*
     * Info structure will hold all the information necessary for
     * each ASN.1 type, including tag, type, flags, constraints,
     * and will change accordingly.
     */
    struct Info {
    	CreateAbstractType create;
    	char tag;
    	const void* parentInfo;
    };
    const void* info;
public:
	AbstractType(const void *info) { this->info = info; }
	virtual ~AbstractType() {}

    /* Getter methods. */
	char getTag() const { return getInfo()->tag; }
	const Info *getInfo() const { return static_cast<const Info*>(info); }

	/* Utility methods. */
	virtual AbstractType *clone() const = 0;
	virtual int64_t compare(const AbstractType& other) const = 0;
};

/*
 * Base class for all constrained ASN.1 types.
 */
class ConstrainedType : public AbstractType {
protected:
    struct Info {
    	CreateAbstractType create;
    	char tag;
    	const void* parentInfo;
    	char type;
    	int64_t lowerBound;
    	int64_t upperBound;
    };
public:
	ConstrainedType(const void *info) : AbstractType(info) {}
	virtual ~ConstrainedType() {}

	/* Getter methods. */
	char getConstraintType() const { return getInfo()->type; }
	int64_t getLowerBound() const { return getInfo()->lowerBound; }
	int64_t getUpperBound() const { return getInfo()->upperBound; }
	bool isExtendable() const { return (getInfo()->type == EXTCONSTRAINED); }
	const Info* getInfo() const { return static_cast<const Info*>(info); }

};

/*
 * Class for ASN.1 Open type field.
 */
class OpenType : public AbstractType {
private:
	int64_t length;
	char *value;
public:
	static const Info theInfo;

	/* Constructors. */
	OpenType(const void *info = &theInfo) : AbstractType(info) {}
	OpenType(char *value, int64_t length, const void *info = &theInfo);
	OpenType(AbstractType *value, const void *info = &theInfo);
	OpenType(const OpenType& other) : AbstractType(other) { operator=(other); }

	virtual ~OpenType() {}

	/* Operator methods. */
	OpenType &operator=(const OpenType& other);

	/* Setter methods. */
	void setValue(char *value) { this->value = value; }
	void setLength(int64_t length) { this->length = length; }

	/* Getter methods. */
	char *getValue() const { return value; }
	int64_t getLength() const { return length; }

	/* Utility methods. */
	virtual AbstractType *clone() const { return new OpenType(*this); }
	virtual int64_t compare(const AbstractType& other) const;
	static AbstractType *create(const void *info) { return new OpenType(info); }

	/* Wrapper methods.  */
	bool decode(char *buffer);
	bool encode(PerEncoder& encoder) const;
};

/*
 * Class for ASN.1 BOOLEAN type
 */
class Boolean : public AbstractType {
private:
    bool value;
public:
    static const Info theInfo;

    /* Constructors. */
    Boolean(const void *info = &theInfo) : AbstractType(info) {}
    Boolean(bool val, const void *info = &theInfo);
    Boolean(const Boolean& other) : AbstractType(other) { operator=(other); }

    virtual ~Boolean() {}

    /* Operator methods. */
    Boolean &operator=(const Boolean& other);

    /* Setter methods. */
    void setValue(bool value) { this->value = value; }

    /* Getter methods. */
    bool getValue() const { return value; }

    /* Utility methods. */
    virtual AbstractType *clone() const { return new Boolean(*this); }
    virtual int64_t compare(const AbstractType& other) const;
    static AbstractType *create(const void *info) { return new Boolean(info); }


    /* Wrapper methods.  */
    bool decode(char *buffer);
    bool encode(PerEncoder& encoder) const;
};

/*
 * Class for ASN.1 NULL type
 */
class Null : public AbstractType {
public:
    static const Info theInfo;

    /* Constructors. */
    Null(const void *info = &theInfo) : AbstractType(info) {}
    Null(const Null& other) : AbstractType(other) { operator=(other); }

    virtual ~Null() {}

    /* Operator methods. */
    Null &operator=(const Null& other);

    /* Utility methods. */
    virtual AbstractType *clone() const { return new Null(*this); }
    virtual int64_t compare(const AbstractType& other) const;
    static AbstractType *create(const void *info) { return new Null(info); }


    /* Wrapper methods.  */
    bool decode(char *buffer);
    bool encode(PerEncoder& encoder) const;
};

/*
 * Class for ASN.1 Integer type.
 */
class IntegerBase : public ConstrainedType {
private:
	int64_t value;
public:
	static const Info theInfo;

	/* Constructors. */
	IntegerBase(const void *info = &theInfo) : ConstrainedType(info) {}
	IntegerBase(int64_t value, const void *info = &theInfo) : ConstrainedType(info) { setValue(value); }
	IntegerBase(const IntegerBase& other) : ConstrainedType(other) { operator=(other); }

	virtual ~IntegerBase() {}

	/* Operator methods. */
	IntegerBase &operator=(const IntegerBase &other);
	IntegerBase &operator=(int64_t value);
//	operator int64_t() const { return value; }

	/* Getter methods. */
	int64_t getValue() const { return value; }

	/* Setter methods. */
	void setValue(int64_t value) { this->value = value; }

	/* Utility methods. */
	virtual AbstractType *clone() const { return new IntegerBase(*this); }
	virtual int64_t compare(const AbstractType& other) const;
	static AbstractType *create(const void *info) { return new IntegerBase(info); }

	/* Wrapper methods. */
	bool decode(char *buffer);
	bool encode(PerEncoder& encoder) const;
};

template <ConstraintType type, int64_t lowerBound, int64_t upperBound>
class Integer : public IntegerBase {
public:
	static const Info theInfo;
	Integer(int64_t value = 0) : IntegerBase(&theInfo) { setValue(value); }
};

template <ConstraintType type, int64_t lowerBound, int64_t upperBound>
const typename Integer<type, lowerBound, upperBound>::Info Integer<type, lowerBound, upperBound>::theInfo = {
    &IntegerBase::create,
    INTEGER,
    0,
    type,
    lowerBound,
    upperBound
};

/*
 * Class for ASN.1 Enumerated type.
 */
class EnumeratedBase : public AbstractType {
private:
	int64_t value;
protected:
    struct Info {
    	CreateAbstractType create;
    	char tag;
    	const void* parentInfo;
    	bool extFlag;
    	int64_t upperBound;
    };
public:
    /* Constructors. */
	EnumeratedBase(const void *info) : AbstractType(info) {}
	EnumeratedBase(const EnumeratedBase& other) : AbstractType(other) { operator=(other); }

	virtual ~EnumeratedBase() {}

	/* Operator methods. */
	EnumeratedBase &operator=(const EnumeratedBase& other);

	/* Getter methods. */
	int64_t getValue() const { return value; }
	bool isExtendable() const { return getInfo()->extFlag; }
	int64_t getUpperBound() const { return getInfo()->upperBound; }
	const Info* getInfo() const { return static_cast<const Info*>(info); }

	/* Setter methods. */
	void setValue(int64_t value) { this->value = value; }

	/* Utility methods. */
	virtual AbstractType *clone() const { return new EnumeratedBase(*this); }
	virtual int64_t compare(const AbstractType& other) const;
	static AbstractType *create(const void *info) { return new EnumeratedBase(info); }

	/* Wrapper methods. */
	bool decode(char *buffer);
	bool encode(PerEncoder& encoder) const;
};

template <bool ext, int64_t upperBound>
class Enumerated : public EnumeratedBase {
public:
	static const Info theInfo;
	Enumerated(int64_t value = 0) : EnumeratedBase(&theInfo) { setValue(value); }

//	Enumerated<ext, upperBound> &operator=(int64_t val) { setValue(val); return *this; }
};

template <bool ext, int64_t upperBound>
const typename Enumerated<ext, upperBound>::Info Enumerated<ext, upperBound>::theInfo = {
    &EnumeratedBase::create,
    ENUMERATED,
    0,
    ext,
    upperBound
};

/*
 * Class for ASN.1 Bitstring type.
 */
class BitStringBase : public ConstrainedType {
protected:
	int64_t length;
	char *value;
public:
	static const Info theInfo;

	/* Constructors. */
	BitStringBase(const void *info = &theInfo);
	BitStringBase(const BitStringBase& other) : ConstrainedType(other) { operator=(other); }

	virtual ~BitStringBase() {}

	/* Operator methods. */
	BitStringBase& operator=(const BitStringBase& other);
//	bool operator==(const BitStringBase& other) const { return compare(other) == 0; }

	/* Getter methods. */
	int64_t getLength() const { return length; }
	char *getValue() const { return value; }
	bool getBit(int64_t index) const;

	/* Setter methods. */
	void setValue(char *value) { this->value = value; }
	void setLength(int64_t length) { this->length = length; }
	void setBit(int64_t index, bool bit);
	int64_t resize(int64_t length);

	/* Utility methods. */
	virtual AbstractType *clone() const { return new BitStringBase(*this); }
	virtual int64_t compare(const AbstractType& other) const;
	static AbstractType *create(const void *info) { return new BitStringBase(info); }
	void print();

	/* Wrapper methods. */
	bool decode(char *buffer);
	bool encode(PerEncoder& encoder) const;
};

template <ConstraintType type, int64_t lowerBound, int64_t upperBound>
class BitString : public BitStringBase {
public:
	static const Info theInfo;
	BitString(char *value = NULL) : BitStringBase(&theInfo) { setValue(value); }
};

template <ConstraintType type, int64_t lowerBound, int64_t upperBound>
const typename BitString<type, lowerBound, upperBound>::Info BitString<type, lowerBound, upperBound>::theInfo = {
    &BitStringBase::create,
    BITSTRING,
    0,
    type,
    lowerBound,
    upperBound
};

/*
 * Class for ASN.1 Octetstring type.
 */
class OctetStringBase : public ConstrainedType {
protected:
	int64_t length;
	char *value;
public:
	static const Info theInfo;

	/* Constructors. */
	OctetStringBase(const void *info = &theInfo);
	OctetStringBase(const OctetStringBase& other) : ConstrainedType(other) { operator=(other); }

	virtual ~OctetStringBase() {}

	/* Operator methods. */
	OctetStringBase& operator=(const OctetStringBase& other);

	/* Getter methods. */
	int64_t getLength() const { return length; }
	char *getValue() const { return value; }

	/* Setter methods. */
	void setValue(char *value) { this->value = value; }
	void setLength(int64_t length) { this->length = length; }

	/* Utility methods. */
	virtual AbstractType *clone() const { return new OctetStringBase(*this); }
	virtual int64_t compare(const AbstractType& other) const;
	static AbstractType *create(const void *info) { return new OctetStringBase(info); }

	/* Wrapper methods. */
	bool decode(char *buffer);
	bool encode(PerEncoder& encoder) const;
};

template <ConstraintType type, int64_t lowerBound, int64_t upperBound>
class OctetString : public OctetStringBase {
public:
	static const Info theInfo;
	OctetString(char *value = NULL) : OctetStringBase(&theInfo) { setValue(value); }
};

template <ConstraintType type, int64_t lowerBound, int64_t upperBound>
const typename OctetString<type, lowerBound, upperBound>::Info OctetString<type, lowerBound, upperBound>::theInfo = {
    &OctetStringBase::create,
    OCTETSTRING,
    0,
    type,
    lowerBound,
    upperBound
};

/*
 * Class for ASN.1 Printablestring type.
 */
class PrintableStringBase : public ConstrainedType {
private:
	std::string value;
public:
	static const Info theInfo;

	/* Constructors. */
	PrintableStringBase(const void *info = &theInfo) : ConstrainedType(info) {}
	PrintableStringBase(std::string value, const void *info = &theInfo) : ConstrainedType(info) { setValue(value); }
	PrintableStringBase(const char *value, const void *info = &theInfo) : ConstrainedType(info) { setValue(value); }
	PrintableStringBase(const PrintableStringBase& other) : ConstrainedType(other) { operator=(other); }

	virtual ~PrintableStringBase() {}

	/* Operator methods. */
	PrintableStringBase& operator=(const PrintableStringBase& other);
	//	bool operator==(const PrintableStringBase& other) const { return compare(other) == 0; }

	/* Getter methods. */
	int64_t getLength() const { return value.size(); }
	std::string getValue() const { return value; }

	/* Setter methods. */
	void setValue(std::string value) { this->value = value; }
	void print() { printf("%s\n", value.c_str()); }

	/* Utility methods. */
	virtual AbstractType *clone() const { return new PrintableStringBase(*this); }
	virtual int64_t compare(const AbstractType& other) const;
	static AbstractType *create(const void *info) { return new PrintableStringBase(info); }

	/* Wrapper methods. */
	bool decode(char *buffer);
	bool encode(PerEncoder& encoder) const;
};

template <ConstraintType type, int64_t lowerBound, int64_t upperBound>
class PrintableString : public PrintableStringBase {
public:
	static const Info theInfo;
	PrintableString(std::string value) : PrintableStringBase(value, &theInfo) {}
	PrintableString(const char *value) : PrintableStringBase(value, &theInfo) {}
};

template <ConstraintType type, int64_t lowerBound, int64_t upperBound>
const typename PrintableString<type, lowerBound, upperBound>::Info PrintableString<type, lowerBound, upperBound>::theInfo = {
    &PrintableStringBase::create,
    PRINTABLESTRING,
    0,
    type,
    lowerBound,
    upperBound
};

/*
 * Class for ASN.1 Sequence type.
 */
class Sequence : public AbstractType {
protected:
	char *optFlags; // holds optional and default bits
	char *extFlags; // holds extension presence bits
	std::vector<AbstractType*> items;
    struct Info {
    	CreateAbstractType create;
    	char tag;
    	const void* parentInfo;
    	bool extFlag;
    	const void** itemsInfo;
    	bool* itemsPres;
    	int64_t sizeRoot;
    	int64_t sizeOpt;
    	int64_t sizeExt;
    };
public:
    /* Constructors */
	Sequence(const void *info);
	Sequence(const Sequence& other);

	virtual ~Sequence() {}

	/* Getter methods. */
	AbstractType *at(int64_t index) const { return items.at(index); }
	int64_t getLength() const { return items.size(); }
	char *getOptFlags() const { return optFlags; }
	char *getExtFlags() const { return extFlags; }
	bool isExtendable() const { return getInfo()->extFlag; }
	bool isOptional(int64_t i) const { return !(getInfo()->itemsPres[i]); }
	bool getOptFlag(int64_t index) const;
	const Info* getInfo() const { return static_cast<const Info*>(info);}

	/* Setter methods. */
	void setOptFlag(int64_t index, bool bit);

	/* Utility methods. */
	virtual AbstractType *clone() const { return new Sequence(*this); }
	virtual int64_t compare(const AbstractType& other) const;
	static AbstractType *create(const void *info) { return new Sequence(info); }

	/* Wrapper methods. */
	bool decode(char *buffer);
	bool encode(PerEncoder& encoder) const;
};

/*
 * Class for ASN.1 Sequenceof type.
 */
class SequenceOfBase : public ConstrainedType {
protected:
	typedef std::vector<AbstractType*> Container;
	Container items;
    struct Info {
    	CreateAbstractType create;
    	char tag;
    	const void* parentInfo;
    	char type;
    	int64_t lowerBound;
    	int64_t upperBound;
    	const void* itemInfo;
    };
public:
    /* Constructors. */
	SequenceOfBase(const void *info) : ConstrainedType(info) {}
	SequenceOfBase(const SequenceOfBase& other);
	virtual ~SequenceOfBase() {}

	/* Getter methods. */
	int64_t size() const { return items.size(); }
	AbstractType *at(int64_t it) const { return items.at(it); }
	void pop_back() { items.pop_back(); }
	const Info* getInfo() const { return static_cast<const Info*>(info); }

	/* Setter methods. */
	void push_back(AbstractType *item) { items.push_back(item); }

    /* Utility methods. */
	virtual AbstractType *clone() const { return new SequenceOfBase(*this); }
	virtual int64_t compare(const AbstractType& other) const;
	static AbstractType *create(const void *info) { return new SequenceOfBase(info); }
    AbstractType * createItem() const;

    /* Wrapper methods. */
	bool decode(char *buffer);
	bool encode(PerEncoder& encoder) const;
};

template <class T, ConstraintType type, int64_t lowerBound, int64_t upperBound>
class SequenceOf : public SequenceOfBase {
public:
	static const Info theInfo;
	SequenceOf(const void *info = &theInfo) : SequenceOfBase(info) {}

    void push_back(const T& x) { items.push_back(x.clone());}
    void push_back(T* x) { items.push_back(x);}

    T& operator[](int64_t n) { return *static_cast<T*>(items.operator[](n));}
    const T&  operator[](int64_t n)  const{ return *static_cast<const T*>(items.operator[](n));}

    T& at(int64_t n) { return *static_cast<T*>(items.at(n));}
    const T& at(int64_t n) const { return *static_cast<const T*>(items.at(n));}

	int64_t size() const { return items.size(); }
};

template <class T, ConstraintType type, int64_t lowerBound, int64_t upperBound>
const typename SequenceOf<T, type, lowerBound, upperBound>::Info SequenceOf<T, type, lowerBound, upperBound>::theInfo = {
	SequenceOfBase::create,
	SEQUENCEOF,
	0,
	type,
	lowerBound,
	upperBound,
	&T::theInfo
};

/*
 * Class for ASN.1 Choice type.
 */
class Choice : public AbstractType {
protected:
	int64_t choice;
	AbstractType *value;
    struct Info {
    	CreateAbstractType create;
    	char tag;
    	const void* parentInfo;
    	bool extFlag;
    	const void** choicesInfo;
    	int64_t upperBound;
    };
public:
    /* Constructors */
	Choice(const void *info, int64_t choice = -1, AbstractType *value = NULL);
	Choice(const Choice& other);

	virtual ~Choice() {}

	/* Getter methods. */
	AbstractType *getValue() const { return value; }
	int64_t getChoice() const { return choice; }
	bool isExtendable() const { return getInfo()->extFlag; }
	int64_t getUpperBound() const { return getInfo()->upperBound; }
	const Info* getInfo() const { return static_cast<const Info*>(info);}

	/* Setter methods. */
	void setValue(AbstractType *value, int64_t choice) { this->choice = choice; this->value = value; }
	void createChoice(int64_t choice) { this->choice = choice; createValue(); }
	void createValue();

	/* Utility methods. */
	virtual AbstractType *clone() const { return new Choice(*this); }
	virtual int64_t compare(const AbstractType& other) const;
	static AbstractType *create(const void *info) { return new Choice(info); }

	/* Wrapper methods. */
	bool decode(char *buffer);
	bool encode(PerEncoder& encoder) const;
};

#endif /* ASNTYPES_H */
