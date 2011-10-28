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

#ifndef MACADDRESS_64_H_
#define MACADDRESS_64_H_

#include <string>
#include <omnetpp.h>
#include "INETDefs.h"
#include "MACAddress.h"


#define MAC_ADDRESS_BYTES_64 8

class InterfaceToken;


/**
 * Stores an IEEE 802 MAC address (6 octets = 48 bits).
 */
class INET_API MACAddress64
{
  private:
    unsigned char address[8];   // 8*8=64 bit address
    bool forceException; // try to convert 64 to 48 and desn't match, throw exception
    bool removeUnnecessary; // remove unnecessary bytes if the orignal address was 48 bits
    friend class MACAddress;
  public:
    /** Returns the unspecified (null) MAC address */
    static const MACAddress64 UNSPECIFIED_ADDRESS;

    /** Returns the broadcast (ff:ff:ff:ff:ff:ff:ff:ff) MAC address */
    static const MACAddress64 BROADCAST_ADDRESS;

    /**
     * Default constructor initializes address bytes to zero.
     */
    MACAddress64();

    /**
     * Constructor which accepts a hex string (12 hex digits, may also
     * contain spaces, hyphens and colons)
     */
    MACAddress64(const char *hexstr);

    /**
     * Copy constructor.
     */
    MACAddress64(const MACAddress64& other) {operator=(other);}


    MACAddress64(uint64_t other);

    /**
     * Assignment.
     */
    MACAddress64& operator=(const MACAddress64& other);

    /**
     * Returns 6.
     */
    unsigned int getAddressSize() const;

    /**
     * Returns the kth byte of the address.
     */
    unsigned char getAddressByte(unsigned int k) const;

    /**
     * Sets the kth byte of the address.
     */
    void setAddressByte(unsigned int k, unsigned char addrbyte);

    /**
     * Sets the address and returns true if the syntax of the string
     * is correct. (See setAddress() for the syntax.)
     */
    bool tryParse(const char *hexstr);

    /**
     * Converts address value from hex string (12 hex digits, may also
     * contain spaces, hyphens and colons)
     */
    void setAddress(const char *hexstr);

    /**
     * Returns pointer to internal binary representation of address
     * (array of 6 unsigned chars).
     */
    unsigned char *getAddressBytes() {return address;}

    /**
     * Sets address bytes. The argument should point to an array of 6 unsigned chars.
     */
    void setAddressBytes(unsigned char *addrbytes);

    /**
     * Sets the address to the broadcast address (hex ff:ff:ff:ff:ff:ff).
     */
    void setBroadcast();

    /**
     * Returns true this is the broadcast address (hex ff:ff:ff:ff:ff:ff).
     */
    bool isBroadcast() const;

    /**
     * Returns true this is a multicast logical address (starts with bit 1).
     */
    bool isMulticast() const  {return address[0]&0x80;};

    /**
     * Returns true if all address bytes are zero.
     */
    bool isUnspecified() const;

    /**
     * Converts address to a hex string.
     */
    std::string str() const;

    /**
     * Returns true if the two addresses are equal.
     */
    bool equals(const MACAddress64& other) const;


    /**
     * Returns true if the two addresses are equal.
     */
    bool operator==(const MACAddress64& other) const {return (*this).equals(other);}

    /**
     * Returns true if the two addresses are not equal.
     */
    bool operator!=(const MACAddress64& other) const {return !(*this).equals(other);}

    /**
     * Returns -1, 0 or 1 as result of comparison of 2 addresses.
     */
    int compareTo(const MACAddress64& other) const;

    /**
     * Create interface identifier (IEEE EUI-64) which can be used by IPv6
     * stateless address autoconfiguration.
     */
    InterfaceToken formInterfaceIdentifier() const;

    /**
     * Generates a unique address which begins with 0a:aa and ends in a unique
     * suffix.
     */
    static MACAddress64 generateAutoAddress();

    bool operator<(const MACAddress64& addr) const {return compare (addr)<0;}

    bool operator>(const MACAddress64& addr) const {return compare (addr)>0;}

    /**
     * Compares two MAC addresses.
     * Returns -1, 0 or 1.
     */
    int compare(const MACAddress64& addr) const
    {
        return address[0] < addr.address[0] ? -1 : address[0] > addr.address[0] ? 1 :
               address[1] < addr.address[1] ? -1 : address[1] > addr.address[1] ? 1 :
               address[2] < addr.address[2] ? -1 : address[2] > addr.address[2] ? 1 :
               address[3] < addr.address[3] ? -1 : address[3] > addr.address[3] ? 1 :
               address[4] < addr.address[4] ? -1 : address[4] > addr.address[4] ? 1 :
               address[5] < addr.address[5] ? -1 : address[5] > addr.address[5] ? 1 :
               address[6] < addr.address[6] ? -1 : address[6] > addr.address[6] ? 1 :
               address[7] < addr.address[7] ? -1 : address[7] > addr.address[7] ? 1 : 0;
    }

    bool getForceException() {return forceException;}
    void setForceException(bool e) {forceException=e;}
    void activeException() {forceException=true;}
    void desActiveException() {forceException=false;}

    bool getRemoveUnnecessary() {return removeUnnecessary;}
    void setRemoveUnnecessary(bool e) {removeUnnecessary=e;}
    void activeRemoveUnnecessary() {removeUnnecessary=true;}
    void desRemoveUnnecessary() {removeUnnecessary=false;}


    // works with MACaddress (48 bits)

    /**
         * Copy constructor.
         */
     MACAddress64(const MACAddress& other) {operator=(other);}

    /**
        * Returns true if the two addresses are equal.
        */
    bool equals(const MACAddress& other) const;


       /**
        * Returns true if the two addresses are equal.
        */
    bool operator==(const MACAddress& other) const {return (*this).equals(other);}

       /**
        * Returns true if the two addresses are not equal.
        */
    bool operator!=(const MACAddress& other) const {return !(*this).equals(other);}

       /**
        * Returns -1, 0 or 1 as result of comparison of 2 addresses.
        */
    int compareTo(const MACAddress& other) const;


    int compare(const MACAddress& addr) const
    {
    	if (address[3]!=0xFF)
    	    return 1; // in other case will consider always bigger EUI-64
    	if (address[4]!=0xFF || address[4]!=0xFE)
    		return 1; // in other case will consider always bigger EUI-64
        return address[0] < addr.address[0] ? -1 : address[0] > addr.address[0] ? 1 :
               address[1] < addr.address[1] ? -1 : address[1] > addr.address[1] ? 1 :
               address[2] < addr.address[2] ? -1 : address[2] > addr.address[2] ? 1 :
               //address[3] < addr.address[3] ? -1 : address[3] > addr.address[3] ? 1 :
               //address[4] < addr.address[4] ? -1 : address[4] > addr.address[4] ? 1 :
               address[5] < addr.address[3] ? -1 : address[5] > addr.address[3] ? 1 :
               address[6] < addr.address[4] ? -1 : address[6] > addr.address[4] ? 1 :
               address[7] < addr.address[5] ? -1 : address[7] > addr.address[5] ? 1 : 0;
    }

    bool operator<(const MACAddress& addr) const {return compare (addr)<0;}

    bool operator>(const MACAddress& addr) const {return compare (addr)>0;}


       /**
        * Assignment.
        */
    MACAddress64& operator=(const MACAddress& other);

    MACAddress getMacAddress48();
    void setAddressUint64(uint64_t val);
    uint64_t getAddressUint64();
};

inline std::ostream& operator<<(std::ostream& os, const MACAddress64& mac)
{
    return os << mac.str();
}

#endif


