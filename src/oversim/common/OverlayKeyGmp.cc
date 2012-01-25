//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file OverlayKey.cc
 * @author Sebastian Mies, Ingmar Baumgart
 */



#include <omnetpp.h>
#include "OverlayKey.h"

#ifdef USEGMP
#include "Comparator.h"

#include <BinaryValue.h>
#include "SHA1.h"

using namespace std;

uint32_t OverlayKey::keyLength = MAX_KEYLENGTH;

// TODO: replace both commands below with simpler macros without conditionals

//#define ROUND_NEXT(n, div) (((n) + (div) - 1) / (div))
//uint OverlayKey::aSize = ROUND_NEXT(OverlayKey::keyLength, 8*sizeof(mp_limb_t));

uint32_t OverlayKey::aSize = OverlayKey::keyLength / (8*sizeof(mp_limb_t)) +
    (OverlayKey::keyLength % (8*sizeof(mp_limb_t))
     != 0 ? 1 : 0);

mp_limb_t OverlayKey::GMP_MSB_MASK = (OverlayKey::keyLength % GMP_LIMB_BITS)
    != 0 ? (((mp_limb_t)1 << (OverlayKey::keyLength % GMP_LIMB_BITS))-1)
    : (mp_limb_t) - 1;

//--------------------------------------------------------------------
// constants
//--------------------------------------------------------------------

// predefined keys
const OverlayKey OverlayKey::UNSPECIFIED_KEY;
const OverlayKey OverlayKey::ZERO((uint32_t)0);
const OverlayKey OverlayKey::ONE((uint32_t)1);


// hex crap
const char* HEX = "0123456789abcdef";

//--------------------------------------------------------------------
// construction and destruction
//--------------------------------------------------------------------

// default construction: create a unspecified node key
OverlayKey::OverlayKey()
{
    isUnspec = true;
}

// create a key out of an normal integer
OverlayKey::OverlayKey(uint32_t num)
{
    clear();
    key [0] = num;
    trim();
}

// create a key out of a buffer
OverlayKey::OverlayKey(const unsigned char* buf, uint32_t size)
{
    int trimSize, offset;
    clear();
    trimSize = (int)min((uint32_t) (aSize * sizeof(mp_limb_t)), size);
    offset = aSize * sizeof(mp_limb_t) - trimSize;
    memcpy(((char*)key) + offset, buf, trimSize);
    trim();
}

// create a key out of an string with the given base
OverlayKey::OverlayKey(const std::string& str, uint32_t base)
{
    if ((base < 2) || (base > 16)) {
        throw cRuntimeError("OverlayKey::OverlayKey(): Invalid base!");
    }

    if (base != 8 && base != 16 && base != 10)
        throw cRuntimeError("OverlayKey::OverlayKey(): Invalid base!");

    string s(str);
    clear();

    for (uint32_t i=0; i<s.size(); i++) {
        if ((s[i] >= '0') && (s[i] <= '9')) {
            s[i] -= '0';
        } else if ((s[i] >= 'a') && (s[i] <= 'f')) {
            s[i] -= ('a' - 10);
        } else if ((s[i] >= 'A') & (s[i] <= 'F')) {
            s[i] -= ('A' - 10);
        } else {
            throw cRuntimeError("OverlayKey::OverlayKey(): "
                                    "Invalid character in string!");
        }
    }

    int i = 0;
    string val;
    int radix = 0;;

    if (s [i] == '-')
    {
        ++i;
    }

    if (s [i] == '0')
    {
        radix = 8;
        ++i;
        if (s [i] == 'x')
        {
            radix = 16;
            ++i;
       }
    }

    if (radix != 8  && base == 8)
        val = "0" + s;
    else if (radix != 16 && base == 16)
        val = "0x" + s;
    else
        val = s;

    key.set(s.c_str());

    //mpn_set_str ((mp_limb_t*)this->key, (const unsigned char*)s.c_str(),
	//	 str.size(), base);
    // trim();
}

// copy constructor
OverlayKey::OverlayKey(const OverlayKey& rhs)
{
    (*this) = rhs;
}

// default destructur
OverlayKey::~OverlayKey()
{}

//--------------------------------------------------------------------
// string representations & node key attributes
//--------------------------------------------------------------------

void OverlayKey::setKeyLength(uint32_t length)
{
    if ((length < 1) || (length > OverlayKey::keyLength)) {
        opp_error("OverlayKey::setKeyLength(): length must be <= %i "
                  "and setKeyLength() must not be called twice "
                  "with different length!", MAX_KEYLENGTH);
    }

    keyLength = length;

    aSize = keyLength / (8*sizeof(mp_limb_t)) +
        (keyLength % (8*sizeof(mp_limb_t))!=0 ? 1 : 0);

    GMP_MSB_MASK = (keyLength % GMP_LIMB_BITS)
	!= 0 ? (((mp_limb_t)1 << (keyLength % GMP_LIMB_BITS))-1)
	: (mp_limb_t)-1;
}


// returns the length in bits
uint32_t OverlayKey::getLength()
{
    return OverlayKey::keyLength;
}

bool OverlayKey::isUnspecified() const
{
    return isUnspec;
}

std::string OverlayKey::toString(uint32_t base) const
{
    if ((base != 2) && (base != 16)) {
        throw cRuntimeError("OverlayKey::OverlayKey(): Invalid base!");
    }

    if (isUnspec)
        return std::string("<unspec>");
    else {
        char temp[MAX_KEYLENGTH+1];

        if (base==16) {
            int k=0;
            for (int i=(keyLength-1)/4; i>=0; i--, k++)
                temp[k] = HEX[this->getBitRange
                              (4*i,4)];

            temp[k] = 0;
            return std::string((const char*)temp);
        } else if (base==2) {
            int k=0;
            for (int i=keyLength-1; i>=0; i-=1, k++)
                temp[k] = HEX[this->getBit(i)];
            temp[k] = 0;
            return std::string((const char*)temp);
        } else {
            throw cRuntimeError("OverlayKey::OverlayKey(): Invalid base!");
        }

// the following native libgmp code doesn't work with leading zeros
#if 0
        mp_size_t last = mpn_get_str((unsigned char*)temp, base,
                                     (mp_limb_t*)this->key, aSize);
        for (int i=0; i<last; i++) {
            temp[i] = HEX[temp[i]];
        }
        temp[last] = 0;
        return std::string((const char*)temp);
#endif

    }
}

//--------------------------------------------------------------------
// operators
//--------------------------------------------------------------------

// assignment operator
OverlayKey& OverlayKey::operator=(const OverlayKey& rhs)
{
    isUnspec = rhs.isUnspec;
    memcpy( key, rhs.key, aSize*sizeof(mp_limb_t) );
    return *this;
}

// sub one prefix operator
OverlayKey& OverlayKey::operator--()
{
    return (*this -= ONE);
}

// sub one postfix operator
OverlayKey OverlayKey::operator--(int)
{
    OverlayKey clone = *this;
    *this -= ONE;
    return clone;
}

// add one prefix operator
OverlayKey& OverlayKey::operator++()
{
    return (*this += ONE);
}

// sub one postfix operator
OverlayKey OverlayKey::operator++(int)
{
    OverlayKey clone = *this;
    *this += ONE;
    return clone;
}

// add assign operator
OverlayKey& OverlayKey::operator+=( const OverlayKey& rhs )
{
    mpn_add_n((mp_limb_t*)key, (mp_limb_t*)key, (mp_limb_t*)rhs.key, aSize);
    trim();
    isUnspec = false;
    return *this;
}

// sub assign operator
OverlayKey& OverlayKey::operator-=( const OverlayKey& rhs )
{
    mpn_sub_n((mp_limb_t*)key, (mp_limb_t*)key, (mp_limb_t*)rhs.key, aSize);
    trim();
    isUnspec = false;
    return *this;
}

// add operator
OverlayKey OverlayKey::operator+(const OverlayKey& rhs) const
{
    OverlayKey result = *this;
    result += rhs;
    return result;
}

// sub operator
OverlayKey OverlayKey::operator-(const OverlayKey& rhs) const
{
    OverlayKey result = *this;
    result -= rhs;
    return result;
}

// compare operators
bool OverlayKey::operator<(const OverlayKey& compKey) const
{
    return compareTo(compKey) < 0;
}
bool OverlayKey::operator>(const OverlayKey& compKey) const
{
    return compareTo(compKey) > 0;
}
bool OverlayKey::operator<=(const OverlayKey& compKey) const
{
    return compareTo(compKey) <=0;
}
bool OverlayKey::operator>=(const OverlayKey& compKey) const
{
    return compareTo(compKey) >=0;
}
bool OverlayKey::operator==(const OverlayKey& compKey) const
{
    return compareTo(compKey) ==0;
}
bool OverlayKey::operator!=(const OverlayKey& compKey) const
{
    return compareTo(compKey) !=0;
}

// bitwise xor
OverlayKey OverlayKey::operator^ (const OverlayKey& rhs) const
{
    OverlayKey result = *this;
    for (uint32_t i=0; i<aSize; i++) {
        result.key[i] ^= rhs.key[i];
    }

    return result;
}

// bitwise or
OverlayKey OverlayKey::operator| (const OverlayKey& rhs) const
{
    OverlayKey result = *this;
    for (uint32_t i=0; i<aSize; i++) {
        result.key[i] |= rhs.key[i];
    }

    return result;
}

// bitwise and
OverlayKey OverlayKey::operator& (const OverlayKey& rhs) const
{
    OverlayKey result = *this;
    for (uint32_t i=0; i<aSize; i++) {
        result.key[i] &= rhs.key[i];
    }

    return result;
}

// complement
OverlayKey OverlayKey::operator~ () const
{
    OverlayKey result = *this;
    for (uint32_t i=0; i<aSize; i++) {
        result.key[i] = ~key[i];
    }
    result.trim();

    return result;
}

// bitwise shift right
OverlayKey OverlayKey::operator>>(uint32_t num) const
{
    OverlayKey result = ZERO;
    int i = num/GMP_LIMB_BITS;

    num %= GMP_LIMB_BITS;

    if (i>=(int)aSize)
        return result;

    for (int j=0; j<(int)aSize-i; j++) {
        result.key[j] = key[j+i];
    }
    mpn_rshift(result.key,result.key,aSize,num);
    result.isUnspec = false;
    result.trim();

    return result;
}

// bitwise shift left
OverlayKey OverlayKey::operator<<(uint32_t num) const
{
    OverlayKey result = ZERO;
    int i = num/GMP_LIMB_BITS;

    num %= GMP_LIMB_BITS;

    if (i>=(int)aSize)
        return result;

    for (int j=0; j<(int)aSize-i; j++) {
        result.key[j+i] = key[j];
    }
    mpn_lshift(result.key,result.key,aSize,num);
    result.isUnspec = false;
    result.trim();

    return result;
}

// get bit
OverlayKeyBit OverlayKey::operator[](uint32_t n)
{
    return OverlayKeyBit(getBit(n), n, this);
}

OverlayKey& OverlayKey::setBit(uint32_t pos, bool value)
{
    if (pos >= keyLength) {
        throw cRuntimeError("OverlayKey::setBitAt(): "
                                "pos >= keyLength!");
    }

    mp_limb_t digit = 1;
    digit = digit << (pos % GMP_LIMB_BITS);

    if (value) {
        key[pos / GMP_LIMB_BITS] |= digit;
    } else {
        //key[pos / GMP_LIMB_BITS] = key[pos / GMP_LIMB_BITS] & ~digit;
        key[pos / GMP_LIMB_BITS] &= ~digit;
    }

    return *this;
};

//--------------------------------------------------------------------
// additional math
//--------------------------------------------------------------------

// returns a sub integer
uint32_t OverlayKey::getBitRange(uint32_t p, uint32_t n) const
{
    int i = p / GMP_LIMB_BITS,      // index of starting bit
        f = p % GMP_LIMB_BITS,      // position of starting bit
        f2 = f + n - GMP_LIMB_BITS; // how many bits to take from next index

    if ((p + n > OverlayKey::keyLength) || (n > 32)) {
        throw cRuntimeError("OverlayKey::get:  Invalid range");
    }
    if (GMP_LIMB_BITS < 32) {
        throw cRuntimeError("OverlayKey::get:  GMP_LIMB_BITS too small!");
    }

    return ((key[i] >> f) |                                     // get the bits of key[i]
            (f2 > 0 ? (key[i+1] << (GMP_LIMB_BITS - f)) : 0)) & // the extra bits from key[i+1]
        (((uint32_t)(~0)) >> (GMP_LIMB_BITS - n));              // delete unused bits
}

double OverlayKey::toDouble() const
{
    double result = 0;
    uint8_t range = 32, length = getLength();
    for (uint8_t i = 0; i < length; i += 32) {
        if ((length - i) < 32) range = length - i;
        result += (getBitRange(i, range) * pow(2.0, i));
    }

    return result;
}

// fill suffix with random bits
OverlayKey OverlayKey::randomSuffix( uint32_t pos ) const
{
    OverlayKey newKey = *this;
    int i = pos/GMP_LIMB_BITS, j = pos%GMP_LIMB_BITS;
    mp_limb_t m = ((mp_limb_t)1 << j)-1;
    mp_limb_t rnd;

    //  mpn_random(&rnd,1);
    omnet_random(&rnd,1);
    newKey.key[i] &= ~m;
    newKey.key[i] |= (rnd&m);
    //  mpn_random(newKey.key,i);
    omnet_random(newKey.key,i);
    newKey.trim();

    return newKey;
}

// fill prefix with random bits
OverlayKey OverlayKey::randomPrefix( uint32_t pos ) const
{
    OverlayKey newKey = *this;
    int i = pos/GMP_LIMB_BITS, j = pos%GMP_LIMB_BITS;
    mp_limb_t m = ((mp_limb_t)1 << j)-1;
    mp_limb_t rnd;

    //  mpn_random(&rnd,1);
    omnet_random(&rnd,1);

    newKey.key[i] &= m;
    newKey.key[i] |= (rnd&~m);
    for (int k=aSize-1; k!=i; k--) {
        //        mpn_random( &newKey.key[k], 1 );
        omnet_random( &newKey.key[k], 1 );
    }
    newKey.trim();

    return newKey;
}

// calculate shared prefix length
uint32_t OverlayKey::sharedPrefixLength(const OverlayKey& compKey,
                                        uint32_t bitsPerDigit) const
{
    if (compareTo(compKey) == 0) return keyLength;

    uint32_t length = 0;
    int i;
    uint32_t j;
    bool msb = true;

    // count equal limbs first:
    for (i=aSize-1; i>=0; --i) {
        if (this->key[i] != compKey.key[i]) {
            // XOR first differing limb for easy counting of the bits:
            mp_limb_t d = this->key[i] ^ compKey.key[i];
            if (msb) d <<= ( GMP_LIMB_BITS - (keyLength % GMP_LIMB_BITS) );
            for (j = GMP_LIMB_BITS-1; d >>= 1; --j);
            length += j;
            break;
        }
        length += GMP_LIMB_BITS;
        msb = false;
    }

    return length / bitsPerDigit;
}

// calculate log of base 2
int OverlayKey::log_2() const
{
    int16_t i = aSize-1;

    while (i>=0 && key[i]==0) {
        i--;
    }

    if (i<0) {
        return -1;
    }

    mp_limb_t j = key[i];
    i *= GMP_LIMB_BITS;
    while (j!=0) {
        j >>= 1;
        i++;
    }

    return i-1;
}

// returns a simple hash of the key
size_t OverlayKey::hash() const
{
    return (size_t)key[0];
}

// returns true, if this key is element of the interval (keyA, keyB)
bool OverlayKey::isBetween(const OverlayKey& keyA,
                           const OverlayKey& keyB) const
{
    if (isUnspec || keyA.isUnspec || keyB.isUnspec)
        return false;

    if (*this == keyA)
        return false;
    else if (keyA < keyB)
        return ((*this > keyA) && (*this < keyB));
    else
        return ((*this > keyA) || (*this < keyB));
}

// returns true, if this key is element of the interval (keyA, keyB]
bool OverlayKey::isBetweenR(const OverlayKey& keyA,
                            const OverlayKey& keyB) const
{
    if (isUnspec || keyA.isUnspec || keyB.isUnspec)
        return false;

    if ((keyA == keyB) && (*this == keyA))
        return true;
    else if (keyA <= keyB)
        return ((*this > keyA) && (*this <= keyB));
    else
        return ((*this > keyA) || (*this <= keyB));
}

// returns true, if this key is element of the interval [keyA, keyB)
bool OverlayKey::isBetweenL(const OverlayKey& keyA,
                            const OverlayKey& keyB) const
{
    if (isUnspec || keyA.isUnspec || keyB.isUnspec)
        return false;

    if ((keyA == keyB) && (*this == keyA))
        return true;
    else if (keyA <= keyB)
        return ((*this >= keyA) && (*this < keyB));
    else
        return ((*this >= keyA) || (*this < keyB));
}

// returns true, if this key is element of the interval [keyA, keyB]
bool OverlayKey::isBetweenLR(const OverlayKey& keyA,
                             const OverlayKey& keyB) const
{
    if (isUnspec || keyA.isUnspec || keyB.isUnspec)
        return false;

    if ((keyA == keyB) && (*this == keyA))
        return true;
    else if (keyA <= keyB)
        return ((*this >= keyA) && (*this <= keyB));
    else
        return ((*this >= keyA) || (*this <= keyB));
}


//----------------------------------------------------------------------
// statics and globals
//----------------------------------------------------------------------

// default console output
std::ostream& operator<<(std::ostream& os, const OverlayKey& c)
{
    os << c.toString(16);
    return os;
};

// returns a key filled with bit 1
OverlayKey OverlayKey::getMax()
{
    OverlayKey newKey;

    for (uint32_t i=0; i<aSize; i++) {
        newKey.key[i] = ~0;
    }
    newKey.isUnspec = false;
    newKey.trim();

    return newKey;
}

// generate random number
OverlayKey OverlayKey::random()
{
    OverlayKey newKey = ZERO;

    omnet_random(newKey.key,aSize);

    newKey.trim();

    return newKey;
}

// generate sha1 hash
OverlayKey OverlayKey::sha1(const BinaryValue& input)
{
    OverlayKey newKey = OverlayKey();
    uint8_t temp[20];
    CSHA1 sha1;

    sha1.Reset();
    sha1.Update((uint8_t*)(&(*input.begin())), input.size());
    sha1.Final();
    sha1.GetHash(temp);
    mpn_set_str(newKey.key, (const uint8_t*)temp,
                (int)std::min((uint32_t)(aSize * sizeof(mp_limb_t)), 20U), 256);
    newKey.isUnspec = false;
    newKey.trim();

    return newKey;
}

// generate a key=2**exponent
OverlayKey OverlayKey::pow2( uint32_t exponent )
{
    if (exponent >= keyLength) {
        throw cRuntimeError("OverlayKey::pow2(): "
                                "exponent >= keyLength!");
    }

    OverlayKey newKey = ZERO;

    newKey.key[exponent/GMP_LIMB_BITS] =
        (mp_limb_t)1 << (exponent % GMP_LIMB_BITS);

    return newKey;
}

// pseudo-regression test
void OverlayKey::test()
{
    // add test
    cout << endl << "--- Add test ..." << endl;
    OverlayKey key = 123456789;
    cout << "    key=" << key << endl;
    cout << "    key += 987654321 = " << (key+=987654321) << endl;
    cout << "    prefix++  : " << (++key) << endl;
    cout << "    postfix++ : " << (key++) << endl;
    cout << "    key=" << key << endl;

    OverlayKey k1 = 256, k2 = 10, k3 = 3;

    // compare test
    cout << endl << "--- Compare test ..." << endl;
    cout << "    256 < 10 = "<< (k1 < k2) << " k1="<<k1<<endl;
    cout << "    256 > 10 = "<< (k1 > k2) << " k2="<<k2<<endl;

    cout << "    10 isBetween(3, 256)=" << k2.isBetween(k3, k1) << endl;
    cout << "    3 isBetween(10, 256)=" << k3.isBetween(k2, k1) << endl;
    cout << "    256 isBetween(10, 256)=" << k1.isBetween(k2, k1) << endl;
    cout << "    256 isBetweenR(10, 256)=" << k1.isBetweenR(k2, k1) << endl;
    cout << "    max isBetween(max-1,0)=" << OverlayKey::getMax().isBetween(
                                                                         OverlayKey::getMax()-1, OverlayKey::ZERO) << endl;
    cout << "    max-1 isBetween(max,1)=" << (OverlayKey::getMax()-1).isBetween(
                                                                             OverlayKey::getMax(), OverlayKey::ONE) << endl;
    cout << "    max-1 isBetweenL(max-1,1)=" << (OverlayKey::getMax()-1).
        isBetweenL(OverlayKey::getMax()-1, OverlayKey::ONE) << endl;
    cout << "    1 isBetweenL(max-1,1)=" << (OverlayKey::ONE).isBetweenL(
                                                                         OverlayKey::getMax()-1, OverlayKey::ONE) << endl;
    cout << "    1 isBetweenR(max-1,1)=" << OverlayKey::ONE.isBetweenR(
                                                                       OverlayKey::getMax()-1, OverlayKey::ONE) << endl;
    cout << "    1 isBetween(max-1,1)=" << OverlayKey::ONE.isBetween(
                                                                     OverlayKey::getMax()-1, OverlayKey::ONE) << endl;
    cout << "    1 isBetween(max-1,0)=" << OverlayKey::ONE.isBetween(
                                                                     OverlayKey::getMax()-1, OverlayKey::ZERO) << endl;
    cout << "    256 sharedPrefixLength(3)=" << k1.sharedPrefixLength(k3)
         << endl;
    cout << "    256 sharedPrefixLength(256)=" << k1.sharedPrefixLength(k1)
         << endl;

    // wrap around test
    cout << endl << "--- Warp around test ..." << endl;

    k1 = OverlayKey::getMax();
    cout << "k1=max= " << k1.toString(16) << endl;
    cout << "k1+1 = " << (k1 + 1).toString(16) << endl;
    cout << "k1+2 = " << (k1 + 2).toString(16) << endl;

    k1 = OverlayKey::ZERO;
    cout << "k1=0= " << k1.toString(16) << endl;
    cout << "k1-1 = " << (k1 - 1).toString(16) << endl;
    cout << "k1-2 = " << (k1 - 2).toString(16) << endl;

    cout << "max > ONE=" << (OverlayKey::getMax() > OverlayKey::ONE) << endl;
    cout << "max < ONE=" << (OverlayKey::getMax() < OverlayKey::ONE) << endl;

    // distance test
    cout << endl << "--- Distance test ..." << endl;

    cout << "KeyRingMetric::distance(1, max)="
         <<  KeyRingMetric().distance(OverlayKey::ONE, OverlayKey::getMax()) << endl;
    cout << "KeyUniRingMetric::distance(1, max)="
         <<  KeyUniRingMetric().distance(OverlayKey::ONE, OverlayKey::getMax()) << endl;
    cout << "KeyRingMetric::distance(max, 1)="
         <<  KeyRingMetric().distance(OverlayKey::getMax(), OverlayKey::ONE) << endl;
    cout << "KeyUniRingMetric::distance(max, 1)="
         <<  KeyUniRingMetric().distance(OverlayKey::getMax(), OverlayKey::ONE) << endl;

    // suffix and log2 test
    cout << endl << "--- RandomSuffix and log2 test ..." << endl;
    k1 = OverlayKey::ZERO;
    for (uint32_t i=0; i<k1.getLength(); i++) {
        k2=k1.randomSuffix(i);
        cout << "    " << k2.toString(16) << " log2=" << k2.log_2() << endl;
    }
    cout << endl << "--- RandomPrefix and log2 test ..." << endl;
    k1 = OverlayKey::getMax();
    for (uint32_t i=0; i<k1.getLength(); i++) {
        k2=k1.randomPrefix(i);
        cout << "    " << k2.toString(16) << " log2=" << k2.log_2() << endl;
    }

    cout << endl << "--- pow2 test..." << endl;
    for (uint32_t i=0; i<k1.getLength(); i++) {
        k2=pow2(i);
        cout << " 2^" << i << " = " << k2.toString(16) << "   log2="
             << k2.log_2() << endl;
    }

    cout << endl << "--- Bits test ..." << endl << "    ";
    const char* BITS[] = { "000","001","010","011","100","101","110","111" };
    k1 = OverlayKey::random();
    for (int i=k1.getLength()-1; i>=0; i--)
        cout << k1[i];
    cout << " = " << endl << "    ";
    for (int i=k1.getLength()-3; i>=0; i-=3)
        cout << BITS[k1.getBitRange(i,3)];
    cout << endl;

    cout << endl << "--- SHA1 test ... (verified with test vectors)" << endl;
    cout << "    Empty string: " << OverlayKey::sha1("").toString(16)
         << " = da39a3ee5e6b4b0d3255bfef95601890afd80709" << endl;
    cout << "    'Hello World' string: "
         << OverlayKey::sha1("Hello World").toString(16)
         << " = 0a4d55a8d778e5022fab701977c5d840bbc486d0" << endl;
}

//--------------------------------------------------------------------
// private methods (mostly inlines)
//--------------------------------------------------------------------

// trims a key after each operation
inline void OverlayKey::trim()
{
    key[aSize-1] &= GMP_MSB_MASK;
}


// compares this key to any other
int OverlayKey::compareTo( const OverlayKey& compKey ) const
{
    if (compKey.isUnspec || isUnspec)
        opp_error("OverlayKey::compareTo(): key is unspecified!");
    return mpn_cmp(key,compKey.key,aSize);
}

// sets key to zero and clears unspecified bit
inline void OverlayKey::clear()
{
    memset( key, 0, aSize * sizeof(mp_limb_t) );
    isUnspec = false;
}

// replacement function for mpn_random() using omnet's rng
inline void omnet_random(mp_limb_t *r1p, mp_size_t r1n)
{
    // fill in 32 bit chunks
    uint32_t* chunkPtr = (uint32_t*)r1p;

    for (uint32_t i=0; i < ((r1n*sizeof(mp_limb_t) + 3) / 4); i++) {
        chunkPtr[i] = intuniform(0, 0xFFFFFFFF);
    }
}

#ifdef __GMP_SHORT_LIMB
#define GMP_TYPE unsigned int
#else
#ifdef _LONG_LONG_LIMB
#define GMP_TYPE unsigned long long int
#else
#define GMP_TYPE unsigned long int
#endif
#endif

void OverlayKey::netPack(cCommBuffer *b)
{
    //doPacking(b,(GMP_TYPE*)this->key, MAX_KEYLENGTH / (8*sizeof(mp_limb_t)) +
              //(MAX_KEYLENGTH % (8*sizeof(mp_limb_t))!=0 ? 1 : 0));
    // Pack an OverlayKey as uint32_t array and hope for the best
    // FIXME: This is probably not exactly portable
    doPacking(b,(uint32_t*)this->key, MAX_KEYLENGTH / (8*sizeof(uint32_t)) +
              (MAX_KEYLENGTH % (8*sizeof(uint32_t))!=0 ? 1 : 0));
    doPacking(b,this->isUnspec);
}

void OverlayKey::netUnpack(cCommBuffer *b)
{
    //doUnpacking(b,(GMP_TYPE*)this->key, MAX_KEYLENGTH / (8*sizeof(mp_limb_t)) +
                //(MAX_KEYLENGTH % (8*sizeof(mp_limb_t))!=0 ? 1 : 0));
    doUnpacking(b,(uint32_t*)this->key, MAX_KEYLENGTH / (8*sizeof(uint32_t)) +
                (MAX_KEYLENGTH % (8*sizeof(uint32_t))!=0 ? 1 : 0));
    doUnpacking(b,this->isUnspec);

}
#endif
