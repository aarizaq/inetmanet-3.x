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
 * @file I3Identifier.cc
 * @author Antonio Zea
 */


#include "I3Identifier.h"
#include "SHA1.h"

using namespace std;

void I3Identifier::initKey(int prefixL, int keyL)
{
    prefixLength = prefixL;
    keyLength = keyL;
    key = new unsigned char[keyLength / 8];
}

I3Identifier::I3Identifier()
{
    initKey(DEFAULT_PREFIX_LENGTH, DEFAULT_KEY_LENGTH);
}

I3Identifier::I3Identifier(int prefixL, int keyL)
{
    initKey(prefixL, keyL);
}

I3Identifier::I3Identifier(const I3Identifier &id)
{
    initKey(DEFAULT_PREFIX_LENGTH, DEFAULT_KEY_LENGTH);
    *this = id;
}

I3Identifier::I3Identifier(std::string s)
{
    initKey(DEFAULT_PREFIX_LENGTH, DEFAULT_KEY_LENGTH);
    createFromHash(s);
}

void I3Identifier::clear()
{
    memset(key, 0, keyLength / 8);
}

bool I3Identifier::isClear()
{
    for (int i = 0; i < keyLength / 8; i++) {
        if (key[i] != 0) return true;
    }
    return false;
}

int I3Identifier::getPrefixLength() const
{
    return prefixLength;
}

int I3Identifier::getKeyLength() const
{
    return keyLength;
}

int I3Identifier::compareTo(const I3Identifier &id) const
{
    return memcmp(key, id.key, keyLength / 8);
}

bool I3Identifier::operator <(const I3Identifier &id) const
{
    return compareTo(id) < 0;
}

bool I3Identifier::operator >(const I3Identifier &id) const
{
    return compareTo(id) > 0;
}

bool I3Identifier::operator ==(const I3Identifier &id) const
{
    return compareTo(id) == 0;
}

I3Identifier &I3Identifier::operator =(const I3Identifier &id)
{
    memcpy(key, id.key, keyLength / 8);
    name = id.name;
    return *this;
}

bool I3Identifier::isMatch(const I3Identifier &id) const
{
    return memcmp(key, id.key, prefixLength / 8) == 0;
}

int I3Identifier::distanceTo(const I3Identifier &id) const
{
    int index;

    for (index = 0; index < keyLength; index++) {
        if (key[index] != id.key[index]) break;
    }

    return (keyLength - index) * 256 + (key[index] ^ id.key[index]);
}

OverlayKey I3Identifier::asOverlayKey() const
{
    return OverlayKey(key, prefixLength / 8);
}

void I3Identifier::createFromHash(const std::string &p, const std::string &o)
{
    uint8_t temp[20];
    CSHA1 sha1;
    int size1, size2;

    sha1.Reset();
    sha1.Update((uint8_t*)p.c_str(), p.size());
    sha1.Final();
    sha1.GetHash(temp);

    clear();
    size1 = prefixLength / 8;
    if (size1 > 20) size1 = 20;
    memcpy(key, temp, size1);

    name = p + ":0";

    if (o.size() == 0) return;

    name = p + ":" + o;

    sha1.Reset();
    sha1.Update((uint8_t*)o.c_str(), o.size());
    sha1.Final();
    sha1.GetHash(temp);

    clear();
    size2 = (keyLength - prefixLength) / 8;
    if (size2 > 20) size2 = 20;
    memcpy(key + size1, temp, size2);
}

void I3Identifier::createRandomKey()
{
    createRandomPrefix();
    createRandomSuffix();
}

void I3Identifier::createRandomPrefix()
{
    for (int i = 0; i < prefixLength / 8; i++) {
        key[i] = intrand(256);
    }
}

void I3Identifier::createRandomSuffix()
{
    for (int i = prefixLength / 8; i < keyLength / 8; i++) {
        key[i] = intrand(256);
    }
}

int I3Identifier::length() const {
    return 16 + keyLength;
}

void I3Identifier::setName(std::string s) {
    name = s;
}

std::string I3Identifier::getName() {
    return name;
}


std::ostream& operator<<(std::ostream& os, const I3Identifier& id)
{
    bool allzeros;
    const char hex[] = "0123456789abcdef";
    string s0, s1;

    if (id.name.length() != 0) {
        os << "(" << id.name << ") ";
    }

    for (int i = 0; i < id.prefixLength / 8; i++) {
        os << hex[id.key[i] >> 4];
        os << hex[id.key[i] & 0xf];
    }
    os << ':';

    allzeros = true;
    for (int i = id.prefixLength / 8; i < id.keyLength / 8; i++) {
        if (id.key[i] != 0) {
            allzeros = false;
            break;
        }
    }
    if (allzeros) {
        os << "0...";
    } else {
        for (int i = id.prefixLength / 8; i < id.keyLength / 8; i++) {
            os << hex[id.key[i] >> 4];
            os << hex[id.key[i] & 0xf];
        }
    }
    return os;
}


I3Identifier::~I3Identifier()
{
    if (key == 0) {
        cout << "Warning: key already deleted." << endl;
    }
    delete[] key;
    key = 0;
}
