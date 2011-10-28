/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 * Copyright (C) 2011 Alfonso Ariza; Universidad de Malaga, Spain
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
#include "MACAddress64.h"
#include "InterfaceToken.h"


//
// Converts hex string into a byte array 'destbuf'. Destbuf is 'size'
// chars long -- if hext string is shorter, destbuf is filled with zeros;
// if destbuf is longer, it is truncated. Non-hex characters are
// discarded before conversion. Returns number of bytes converted from hex.
//
static int hextobin(const char *hexstr, unsigned char *destbuf, int size)
{
    int k=0;
    const char *s = hexstr;
    for (int pos=0; pos<size; pos++)
    {
        if (!s || !*s)
        {
            destbuf[pos] = 0;
        }
        else
        {
            while (*s && !isxdigit(*s)) s++;
            if (!*s) {destbuf[pos]=0; continue;}
            unsigned char d = isdigit(*s) ? (*s-'0') : islower(*s) ? (*s-'a'+10) : (*s-'A'+10);
            d = d<<4;
            s++;

            while (*s && !isxdigit(*s)) s++;
            if (!*s) {destbuf[pos]=0; continue;}
            d += isdigit(*s) ? (*s-'0') : islower(*s) ? (*s-'a'+10) : (*s-'A'+10);
            s++;

            destbuf[pos] = d;
            k++;
        }
    }
    return k;
}

const MACAddress64 MACAddress64::UNSPECIFIED_ADDRESS;
const MACAddress64 MACAddress64::BROADCAST_ADDRESS("ff:ff:ff:ff:ff:ff:ff:ff");

MACAddress64::MACAddress64()
{
	forceException = true;
	removeUnnecessary=true;
    address[0]=address[1]=address[2]=address[3]=address[4]=address[5]=address[6]=address[7]=0;
}

MACAddress64::MACAddress64(const char *hexstr)
{
	forceException = true;
	removeUnnecessary=true;
    setAddress(hexstr);
}


MACAddress64::MACAddress64(uint64_t val)
{
	forceException = true;
	removeUnnecessary=true;
	setAddressUint64(val);
}


MACAddress64& MACAddress64::operator=(const MACAddress64& other)
{
	forceException = other.forceException;
	removeUnnecessary = other.removeUnnecessary;
    memcpy(address, other.address, MAC_ADDRESS_BYTES_64);
    return *this;
}


MACAddress64& MACAddress64::operator=(const MACAddress& other)
{
	forceException = true;
	removeUnnecessary=true;
	address[0]=other.getAddressByte(0);
	address[1]=other.getAddressByte(1);
	address[2]=other.getAddressByte(2);
	address[3]=0xFF;
	address[4]=0xFF;
	address[5]=other.getAddressByte(3);
	address[6]=other.getAddressByte(4);
	address[7]=other.getAddressByte(5);
    return *this;
}

unsigned int MACAddress64::getAddressSize() const
{
    return 8;
}

unsigned char MACAddress64::getAddressByte(unsigned int k) const
{
    if (k>=8) throw cRuntimeError("Array of size 6 indexed with %d", k);
    return address[k];
}

void MACAddress64::setAddressByte(unsigned int k, unsigned char addrbyte)
{
    if (k>=8) throw cRuntimeError("Array of size 6 indexed with %d", k);
    address[k] = addrbyte;
}

bool MACAddress64::tryParse(const char *hexstr)
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

    if (numHexDigits == 2*MAC_ADDRESS_BYTES)
    {
        unsigned char address48[6];
        hextobin(hexstr, address48, MAC_ADDRESS_BYTES);
        // insert extensions
        address[0]=address48[0];
        address[1]=address48[1];
        address[2]=address48[2];
        address[3]=0xFF;
        address[4]=0XFF;
        address[5]=address48[3];
        address[6]=address48[4];
        address[7]=address48[5];
        return true;
    }
    if (numHexDigits == 2*MAC_ADDRESS_BYTES_64)
    {
        hextobin(hexstr, address, MAC_ADDRESS_BYTES_64);
        return true;
    }
    return false;
}

void MACAddress64::setAddress(const char *hexstr)
{
    if (!tryParse(hexstr))
        throw cRuntimeError("MACAddress64: wrong address syntax '%s': 12 hex digits expected, with optional embedded spaces, hyphens or colons", hexstr);
}

void MACAddress64::setAddressBytes(unsigned char *addrbytes)
{
    memcpy(address, addrbytes, MAC_ADDRESS_BYTES_64);
}

void MACAddress64::setBroadcast()
{
    address[0]=address[1]=address[2]=address[3]=address[4]=address[5]=address[6]=address[7]=0xff;
}

bool MACAddress64::isBroadcast() const
{
    return (address[0]&address[1]&address[2]&address[3]&address[4]&address[5]&address[6]&address[6])==0xff;
}

bool MACAddress64::isUnspecified() const
{
    return !(address[0] || address[1] || address[2] || address[3] || address[4] || address[5]);
}

std::string MACAddress64::str() const
{
    char buf[30];
    char *s = buf;
    bool jump=false;
    if (removeUnnecessary && address[3]==0xFF && address[4]==0xFF)
        jump=true;
    for (int i=0; i<MAC_ADDRESS_BYTES_64; i++, s+=3)
    {
    	if ((i==3 || i==4) && jump) continue;
        sprintf(s,"%2.2X-",address[i]);
    }
    *(s-1)='\0';
    return std::string(buf);
}

bool MACAddress64::equals(const MACAddress64& other) const
{
    return memcmp(address, other.address, MAC_ADDRESS_BYTES_64)==0;
}

int MACAddress64::compareTo(const MACAddress64& other) const
{
    return memcmp(address, other.address, MAC_ADDRESS_BYTES_64);
}


bool MACAddress64::equals(const MACAddress& other) const
{
	MACAddress64 add=other;
    return memcmp(address, add.address, MAC_ADDRESS_BYTES_64)==0;
}

int MACAddress64::compareTo(const MACAddress& other) const
{
	MACAddress64 add=other;
    return memcmp(address, add.address, MAC_ADDRESS_BYTES_64);
}


InterfaceToken MACAddress64::formInterfaceIdentifier() const
{
    const unsigned char *b = address;
    uint32 high = (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | b[3];
    uint32 low =  (b[4]<<24) | (b[5]<<16) | (b[6]<<8) | b[7];
    return InterfaceToken(low, high, 64);
}

MACAddress64 MACAddress64::generateAutoAddress()
{
    ++ MACAddress::autoAddressCtr;

    unsigned char addrbytes[8];
    addrbytes[0] = 0x0A;
    addrbytes[1] = 0xAA;
    addrbytes[2] = (MACAddress::autoAddressCtr>>24)&0xff;
    addrbytes[3] = 0xFF;
    addrbytes[4] = 0xFF;
    addrbytes[5] = (MACAddress::autoAddressCtr>>16)&0xff;
    addrbytes[6] = (MACAddress::autoAddressCtr>>8)&0xff;
    addrbytes[7] = (MACAddress::autoAddressCtr)&0xff;

    MACAddress64 addr;
    addr.setAddressBytes(addrbytes);
    return addr;
}


MACAddress MACAddress64::getMacAddress48()
{
    if (forceException && address[3]!=0xFF && address[4]!=0xFF)
    	throw cRuntimeError("Try to convert address EUI-64 %s to Mac-48 and address doesn't match ",str().c_str());
    MACAddress addr;
	addr.setAddressByte(0,address[0]);
	addr.setAddressByte(1,address[1]);
	addr.setAddressByte(2,address[2]);
	addr.setAddressByte(3,address[5]);
	addr.setAddressByte(4,address[6]);
	addr.setAddressByte(5,address[7]);
	return addr;
}


void MACAddress64::setAddressUint64(uint64_t val)
{
	setAddressByte(0, (val>>56)&0xff);
    setAddressByte(1, (val>>48)&0xff);
    setAddressByte(2, (val>>40)&0xff);
    setAddressByte(3, (val>>32)&0xff);
    setAddressByte(4, (val>>24)&0xff);
    setAddressByte(5, (val>>16)&0xff);
    setAddressByte(6, (val>>8)&0xff);
    setAddressByte(7, val&0xff);
}

uint64_t MACAddress64::getAddressUint64()
{
    uint64_t aux;
    uint64_t lo=0;
    for (int i=0; i<MAC_ADDRESS_BYTES; i++)
    {
        aux  = address[MAC_ADDRESS_BYTES_64-i-1];
        aux <<= 8*i;
        lo  |= aux ;
    }
    return lo;
}


