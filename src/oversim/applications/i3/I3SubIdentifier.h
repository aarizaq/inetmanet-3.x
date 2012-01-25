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
 * @file I3SubIdentifier.h
 * @author Antonio Zea
 */

#ifndef __I3SUBIDENTIFIER_H__
#define __I3SUBIDENTIFIER_H__

#include "I3IPAddress.h"
#include "I3Identifier.h"

/** A wrapper around either an I3IPAddress or an I3Identifier.
* The I3 paper proposes an identifier stack that is "a list of identifiers
* that takes the form (id[1], id[2], ..., id[k]) where
* where id[i] is either an identifier or an address." To avoid confusion, the
* former identifier id[i] is called an I3SubIdentifier and the latter is the normal I3Identifier.
*/

class I3SubIdentifier {
public:
    /** @enum IdentifierType Identifier type */
    enum IdentifierType {
        Invalid,       /** Type has not been set yet */
        Identifier,    /** Subidentifier refers to an I3Identifier */
        IPAddress      /** Subidentifier refers to an I3IPAddress */
    };

    /** Constructor */
    I3SubIdentifier();

    /** Sets this identifier as an I3IPAddress
    * @param address IP address to set to
    */
    void setIPAddress(const I3IPAddress &address);

    /** Sets this identifier as an I3Identifier
     * @param id Identifer to set to
     */
    void setIdentifier(const I3Identifier &id);

    /** Returns the subidentifier type
     * @returns Subidentifier type
     */
    IdentifierType getType() const;

    /** Returns the IP address
    * @returns IP address referred to
    */
    I3IPAddress &getIPAddress();

    /** Returns the identifier
     * @returns Identifier referred to
     */
    I3Identifier &getIdentifier();


    /** Comparison function */
    int compareTo(const I3SubIdentifier &) const;

    int length() const;

    friend std::ostream& operator<<(std::ostream& os, const I3SubIdentifier &s);

protected:
    /** Type of subidentifier */
    IdentifierType type;

    /** IP address referred to */
    I3IPAddress ipAddress;

    /** Identifier referred to */
    I3Identifier identifier;
};

#endif
