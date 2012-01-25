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
 * @file I3Identifier.h
 * @author Antonio Zea
 */


#ifndef __I3IDENTIFIER_H__
#define __I3IDENTIFIER_H__

#include <omnetpp.h>
#include <OverlayKey.h>

/** Implementation of an Internet Indirection Infrastructure identifier.
* An identifier is an array of bits of size m, containing a prefix of size k.
* An identifier i1 "matches" an identifier i2 if the first k bits are equal,
* and i1 is the longest prefix match of i2.
*/

#define DEFAULT_PREFIX_LENGTH   160
#define DEFAULT_KEY_LENGTH      256

class I3Identifier {
public:
    /** Constructor */
    I3Identifier();

    /** Constructs an identifier filled with a byte
    * @param b Byte to fill with
    */
    I3Identifier(unsigned char b);

    /** Constructor for variable prefix length, key length
     * @param prefixL Prefix length
     * @param keyL Key length
     */
    I3Identifier(int prefixL, int keyL);

    /** Copy constructor
     * @param id Identifier to copy
     */
    I3Identifier(const I3Identifier& id);

    /** Constructs an identifier from the hash of a string
     * @param s String to be hashed
     */
    I3Identifier(std::string s);

    /** Returns the key length (total length) in bits */
    int getKeyLength() const;

    /** Returns the prefix length in bits */
    int getPrefixLength() const;

    /** Sets all bits to 0 */
    void clear();

    /** Comparation function */
    int compareTo(const I3Identifier&) const;

    /** "Less than" comparation function */
    bool operator <(const I3Identifier&) const;

    /** "Greater than" comparation function */
    bool operator >(const I3Identifier&) const;

    /** "Equals" comparation function */
    bool operator ==(const I3Identifier&) const;

    /** Copy operator */
    I3Identifier &operator =(const I3Identifier&);

    /** Checks if this identifier has been cleared */
    bool isClear();

    /** Checks if this identifier's first prefixLength bits equal those of id
    * @param id Identifier to be matched against
    */
    bool isMatch(const I3Identifier& id) const;

    /** Returns the "distance to" a identifier.
     * This is used when many triggers match an identifier, to check which is the biggest prefix match.
     * The distance is defined as the index of the byte (counting from the end) which is
     * the first that is not equal, multiplied by 256, then added the XOR'ing of the differing byte.
     * That way an earlier differing bits are considered "further" from later differing ones
     * (notice that this distance is not a real metric!)
     *
     * @param id Identifier to be compared to
     */
    int distanceTo(const I3Identifier& id) const;

    /** Creates an identifier from the hash of a string
    * @param s String to be hashed to form the prefix
    * @param o String to be hashed to form the remaining bits
    */
    void createFromHash(const std::string &s, const std::string &o = "");

    void createRandomKey();
    void createRandomPrefix();
    void createRandomSuffix();

    int length() const;

    /** Creates an OverlayKey from an identifier, to be used in the overlay underneath.
    * No hashing is done, the first min(OverlayKey::keySize, keyLength) bits are copied directly.
    * @returns An overlay key
    */
    OverlayKey asOverlayKey() const;

    void setName(std::string s);
    std::string getName();

    /** String stream output operator
    * @param os String stream
    * @param id I3Identifier to be output
    * @returns os parameter
    */
    friend std::ostream& operator<<(std::ostream& os, const I3Identifier& id);

    /** Destructor */
    ~I3Identifier();

protected:
    /** Identifier bits */
    unsigned char *key;

    /** Size of prefix in bits */
    unsigned short prefixLength;

    /** Size of identifier in bits */
    unsigned short keyLength;

    std::string name;

    /** Inits the key with a given prefix length and key length
    * @param prefixL Prefix length
    * @param keyL Key (identifier) length
    */
    void initKey(int prefixL, int keyL);
};

#endif

