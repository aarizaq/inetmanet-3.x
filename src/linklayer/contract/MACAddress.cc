/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <ctype.h>
#include "MACAddress.h"
#include "InterfaceToken.h"


unsigned int MACAddress::autoAddressCtr;

const MACAddress MACAddress::UNSPECIFIED_ADDRESS;
const MACAddress MACAddress::BROADCAST_ADDRESS("ff:ff:ff:ff:ff:ff");
const MACAddress MACAddress::MULTICAST_PAUSE_ADDRESS("01:80:C2:00:00:01");
const MACAddress MACAddress::BROADCAST_ADDRESS64("ff:ff:ff:ff:ff:ff:ff:ff");

unsigned char MACAddress::getAddressByte(unsigned int k) const
{
    if (macAddress64)
    {
        if (k>=MAC_ADDRESS_SIZE64) throw cRuntimeError("Array of size 8 indexed with %d", k);
        int offset = (MAC_ADDRESS_SIZE64-k-1)*8;
        return 0xff&(address>>offset);
    }
    else
    {
        if (k>=MAC_ADDRESS_SIZE) throw cRuntimeError("Array of size 6 indexed with %d", k);
        int offset = (MAC_ADDRESS_SIZE-k-1)*8;
        return 0xff&(address>>offset);
    }
}

void MACAddress::setAddressByte(unsigned int k, unsigned char addrbyte)
{
    if (macAddress64)
    {
        if (k>=MAC_ADDRESS_SIZE64) throw cRuntimeError("Array of size 6 indexed with %d", k);
        int offset = (MAC_ADDRESS_SIZE64-k-1)*8;
        address = (address&(~(((uint64)0xff)<<offset)))|(((uint64)addrbyte)<<offset);
    }
    else
    {
        if (k>=MAC_ADDRESS_SIZE) throw cRuntimeError("Array of size 6 indexed with %d", k);
        int offset = (MAC_ADDRESS_SIZE-k-1)*8;
        address = (address&(~(((uint64)0xff)<<offset)))|(((uint64)addrbyte)<<offset);
    }
}

bool MACAddress::tryParse(const char *hexstr)
{
    if (!hexstr)
        return false;

    // check syntax
    int numHexDigits = 0;
    for (const char *s = hexstr; *s; s++) {
        if (isxdigit(*s))
            numHexDigits++;
        else if (*s!=' ' && *s!=':' && *s!='-')
            return false; // wrong syntax
    }
    if (numHexDigits == 2*MAC_ADDRESS_SIZE)
        macAddress64 = false;
    else if (numHexDigits == 2*MAC_ADDRESS_SIZE64)
        macAddress64 = true;
    else
        return false;

    // Converts hex string into the address
    // if hext string is shorter, address is filled with zeros;
    // Non-hex characters are discarded before conversion.
    address = 0;
    int k = 0;
    const char *s = hexstr;
    if (macAddress64)
    {
        for (int pos=0; pos<MAC_ADDRESS_SIZE64; pos++)
        {
            if (!s || !*s)
            {
                setAddressByte(pos, 0);
            }
            else
            {
                while (*s && !isxdigit(*s)) s++;
                if (!*s) {setAddressByte(pos, 0); continue;}
                unsigned char d = isdigit(*s) ? (*s-'0') : islower(*s) ? (*s-'a'+10) : (*s-'A'+10);
                d = d<<4;
                s++;

                while (*s && !isxdigit(*s)) s++;
                if (!*s) {setAddressByte(pos, 0); continue;}
                d += isdigit(*s) ? (*s-'0') : islower(*s) ? (*s-'a'+10) : (*s-'A'+10);
                s++;

                setAddressByte(pos, d);
                k++;
            }
        }
    }
    else
    {
        for (int pos=0; pos<MAC_ADDRESS_SIZE; pos++)
        {
            if (!s || !*s)
            {
                setAddressByte(pos, 0);
            }
            else
            {
                while (*s && !isxdigit(*s)) s++;
                if (!*s) {setAddressByte(pos, 0); continue;}
                unsigned char d = isdigit(*s) ? (*s-'0') : islower(*s) ? (*s-'a'+10) : (*s-'A'+10);
                d = d<<4;
                s++;

                while (*s && !isxdigit(*s)) s++;
                if (!*s) {setAddressByte(pos, 0); continue;}
                d += isdigit(*s) ? (*s-'0') : islower(*s) ? (*s-'a'+10) : (*s-'A'+10);
                s++;

                setAddressByte(pos, d);
                k++;
            }
        }
    }
    return true;
}

void MACAddress::setAddress(const char *hexstr)
{
    if (!tryParse(hexstr))
        throw cRuntimeError("MACAddress: wrong address syntax '%s': 12 hex digits expected, with optional embedded spaces, hyphens or colons", hexstr);
}

void MACAddress::getAddressBytes(unsigned char *addrbytes) const
{
    if (macAddress64)
    {
        for (int i = 0; i < MAC_ADDRESS_SIZE64; i++)
            addrbytes[i] = getAddressByte(i);
    }
    else
    {
        for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
            addrbytes[i] = getAddressByte(i);
    }
}

void MACAddress::setAddressBytes(unsigned char *addrbytes)
{
    if (macAddress64)
    {
        for (int i = 0; i < MAC_ADDRESS_SIZE64; i++)
            setAddressByte(i, addrbytes[i]);
    }
    else
    {
        address = 0;
        for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
            setAddressByte(i, addrbytes[i]);
    }
}


std::string MACAddress::str() const
{
    char buf[30];
    char *s = buf;
    if (macAddress64)
    {
        for (int i=0; i<MAC_ADDRESS_SIZE64; i++, s += 3)
            sprintf(s, "%2.2X-", getAddressByte(i));
        *(s-1) = '\0';
    }
    else
    {
        for (int i=0; i<MAC_ADDRESS_SIZE; i++, s += 3)
            sprintf(s, "%2.2X-", getAddressByte(i));
        *(s-1) = '\0';
    }
    return std::string(buf);
}

int MACAddress::compareTo(const MACAddress& other) const
{
    return (address < other.address) ? -1 : (address == other.address) ? 0 : 1;
}

InterfaceToken MACAddress::formInterfaceIdentifier() const
{
    if (macAddress64)
    {
        uint32 high = (address>>32);
        uint32 low =  (address&0xffffffff);
        return InterfaceToken(low, high, 64);
    }
    else
    {
        uint32 high = (address>>16)|0xff;
        uint32 low = (0xfe<<24)|(address&0xffffff);
        return InterfaceToken(low, high, 64);
    }
}

MACAddress MACAddress::generateAutoAddress()
{
    ++autoAddressCtr;

    uint64 intAddr = 0x0AAA00000000ULL + (autoAddressCtr & 0xffffffffUL);
    MACAddress addr(intAddr);
    return addr;
}


void MACAddress::convert64()
{
    if (macAddress64)
        return;
    unsigned char addrbytes64[MAC_ADDRESS_SIZE64];
    unsigned char addrbytes[MAC_ADDRESS_SIZE];
    getAddressBytes(addrbytes);
    macAddress64 = true;
    addrbytes64[0] = addrbytes[0];
    addrbytes64[1] = addrbytes[1];
    addrbytes64[2] = addrbytes[2];
    addrbytes64[3] = 0xfe;
    addrbytes64[4] = 0xff;
    addrbytes64[5] = addrbytes[3];
    addrbytes64[6] = addrbytes[4];
    addrbytes64[7] = addrbytes[5];
    setAddressBytes(addrbytes64);
}

MACAddress MACAddress::getEui64()
{
    MACAddress addr=*this;
    addr.convert64();
    return addr;
}

void MACAddress::convert48()
{
    if (!macAddress64)
        return;
    unsigned char addrbytes64[MAC_ADDRESS_SIZE64];
    unsigned char addrbytes[MAC_ADDRESS_SIZE];
    getAddressBytes(addrbytes64);
    if (addrbytes64[3] != 0xfe ||  addrbytes64[4] != 0xff)
        throw cRuntimeError("MACAddress: conversion EUI-64 to EUI-48 impossible");
    macAddress64 = false;
    addrbytes[0] = addrbytes64[0];
    addrbytes[1] = addrbytes64[1];
    addrbytes[2] = addrbytes64[2];

    addrbytes[3] = addrbytes64[5];
    addrbytes[4] = addrbytes64[6];
    addrbytes[5] = addrbytes64[7];
    setAddressBytes(addrbytes);
}

MACAddress MACAddress::getEui48()
{
    MACAddress addr=*this;
    addr.convert48();
    return addr;
}




