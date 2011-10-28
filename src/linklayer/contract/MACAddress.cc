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
#include "MACAddress64.h"
#include "InterfaceToken.h"


unsigned int MACAddress::autoAddressCtr;

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

const MACAddress MACAddress::UNSPECIFIED_ADDRESS;
const MACAddress MACAddress::BROADCAST_ADDRESS("ff:ff:ff:ff:ff:ff");

MACAddress::MACAddress()
{
    address[0]=address[1]=address[2]=address[3]=address[4]=address[5]=0;
}

MACAddress::MACAddress(const char *hexstr)
{
    setAddress(hexstr);
}

MACAddress& MACAddress::operator=(const MACAddress& other)
{
    memcpy(address, other.address, MAC_ADDRESS_BYTES);
    return *this;
}

unsigned int MACAddress::getAddressSize() const
{
    return 6;
}

unsigned char MACAddress::getAddressByte(unsigned int k) const
{
    if (k>=6) throw cRuntimeError("Array of size 6 indexed with %d", k);
    return address[k];
}

void MACAddress::setAddressByte(unsigned int k, unsigned char addrbyte)
{
    if (k>=6) throw cRuntimeError("Array of size 6 indexed with %d", k);
    address[k] = addrbyte;
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
    if (numHexDigits != 2*MAC_ADDRESS_BYTES)
        return false;

    // convert
    hextobin(hexstr, address, MAC_ADDRESS_BYTES);
    return true;
}

void MACAddress::setAddress(const char *hexstr)
{
    if (!tryParse(hexstr))
        throw cRuntimeError("MACAddress: wrong address syntax '%s': 12 hex digits expected, with optional embedded spaces, hyphens or colons", hexstr);
}

void MACAddress::setAddressBytes(unsigned char *addrbytes)
{
    memcpy(address, addrbytes, MAC_ADDRESS_BYTES);
}

void MACAddress::setBroadcast()
{
    address[0]=address[1]=address[2]=address[3]=address[4]=address[5]=0xff;
}

bool MACAddress::isBroadcast() const
{
    return (address[0]&address[1]&address[2]&address[3]&address[4]&address[5])==0xff;
}

bool MACAddress::isUnspecified() const
{
    return !(address[0] || address[1] || address[2] || address[3] || address[4] || address[5]);
}

std::string MACAddress::str() const
{
    char buf[20];
    char *s = buf;
    for (int i=0; i<MAC_ADDRESS_BYTES; i++, s+=3)
        sprintf(s,"%2.2X-",address[i]);
    *(s-1)='\0';
    return std::string(buf);
}

bool MACAddress::equals(const MACAddress& other) const
{
    return memcmp(address, other.address, MAC_ADDRESS_BYTES)==0;
}

int MACAddress::compareTo(const MACAddress& other) const
{
    return memcmp(address, other.address, MAC_ADDRESS_BYTES);
}

InterfaceToken MACAddress::formInterfaceIdentifier() const
{
    const unsigned char *b = address;
    uint32 high = (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | 0xff;
    uint32 low =  (0xfe<<24) | (b[3]<<16) | (b[4]<<8) | b[5];
    return InterfaceToken(low, high, 64);
}

MACAddress MACAddress::generateAutoAddress()
{
    ++autoAddressCtr;

    unsigned char addrbytes[6];
    addrbytes[0] = 0x0A;
    addrbytes[1] = 0xAA;
    addrbytes[2] = (autoAddressCtr>>24)&0xff;
    addrbytes[3] = (autoAddressCtr>>16)&0xff;
    addrbytes[4] = (autoAddressCtr>>8)&0xff;
    addrbytes[5] = (autoAddressCtr)&0xff;

    MACAddress addr;
    addr.setAddressBytes(addrbytes);
    return addr;
}



//
//
//
//
// Methods for EUI-64
/**
    * Returns true if the two addresses are equal.
    */
bool MACAddress::equals(const MACAddress64& other) const
{
	// special case
	if (other.isUnspecified() && this->isUnspecified())
	    return true;
	if (other.isBroadcast() && this->isBroadcast())
		    return true;

	if (other.address[3]!=0xFF && (other.address[4]!=0xFF || other.address[4]!=0xFE))
	    return false;
	MACAddress add=other;
    return memcmp(address, add.address, MAC_ADDRESS_BYTES)==0;
}


   /**
    * Returns -1, 0 or 1 as result of comparison of 2 addresses.
    */

int MACAddress::compare(const MACAddress64& addr) const
{
	if (addr.isUnspecified() && this->isUnspecified())
	    return 0;
	if (addr.isBroadcast() && this->isBroadcast())
		    return 0;

	if (addr.address[3]!=0xFF)
	    return -1; // in other case will consider always bigger EUI-64
	if (addr.address[4]!=0xFF || addr.address[4]!=0xFE)
		return -1; // in other case will consider always bigger EUI-64
    return address[0] < addr.address[0] ? -1 : address[0] > addr.address[0] ? 1 :
           address[1] < addr.address[1] ? -1 : address[1] > addr.address[1] ? 1 :
           address[2] < addr.address[2] ? -1 : address[2] > addr.address[2] ? 1 :
           address[3] < addr.address[5] ? -1 : address[3] > addr.address[5] ? 1 :
           address[4] < addr.address[6] ? -1 : address[4] > addr.address[6] ? 1 :
           address[5] < addr.address[7] ? -1 : address[5] > addr.address[7] ? 1 : 0;
}

   /**
    * Assignment.
    */
MACAddress& MACAddress::operator=(const MACAddress64& other)
{
    if (other.isUnspecified())
    {
        memset(address,0, MAC_ADDRESS_BYTES);
        return *this;
	}

    if (other.isBroadcast())
    {
        memset(address,0xFF, MAC_ADDRESS_BYTES);
        return *this;
	}

    if (!(other.address[3]==0xFF && (other.address[4]==0xFF  || other.address[4]==0xFE)))
    	throw cRuntimeError("Try to convert address EUI-64 %s to EUI-48 and address doesn't match ",str().c_str());
    address[0]=other.address[0];
    address[1]=other.address[1];
    address[2]=other.address[2];
    address[3]=other.address[5];
    address[4]=other.address[6];
    address[5]=other.address[7];
    return *this;
}

MACAddress64 MACAddress::getMacAddress64()
{
    MACAddress64 addr;
	// Special case address null
    if (isUnspecified())
        return MACAddress64::UNSPECIFIED_ADDRESS;
    if (isBroadcast())
        return MACAddress64::BROADCAST_ADDRESS;
    addr.setAddressByte(0,address[0]);
    addr.setAddressByte(1,address[1]);
    addr.setAddressByte(2,address[2]);
    addr.setAddressByte(3,0xFF);
    addr.setAddressByte(4,0xFE);
    addr.setAddressByte(5,address[3]);
	addr.setAddressByte(6,address[4]);
	addr.setAddressByte(7,address[5]);
	return addr;
}


void MACAddress::setAddressUint64(uint64_t val)
{
    setAddressByte(0, (val>>40)&0xff);
    setAddressByte(1, (val>>32)&0xff);
    setAddressByte(2, (val>>24)&0xff);
    setAddressByte(3, (val>>16)&0xff);
    setAddressByte(4, (val>>8)&0xff);
    setAddressByte(5, val&0xff);
}

uint64_t MACAddress::getAddressUint64()
{
    uint64_t aux;
    uint64_t lo=0;
    for (int i=0; i<MAC_ADDRESS_BYTES; i++)
    {
        aux  = address[MAC_ADDRESS_BYTES-i-1];
        aux <<= 8*i;
        lo  |= aux ;
    }
    return lo;
}




