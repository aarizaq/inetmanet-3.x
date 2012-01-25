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
 * @file I3IdentifierStack.h
 * @author Antonio Zea
 */

#ifndef __I3IDENTIFIERSTACK_H__
#define __I3IDENTIFIERSTACK_H__

#include "I3Identifier.h"
#include "I3IPAddress.h"
#include "I3SubIdentifier.h"
#include <IPvXAddressResolver.h>

/** Stack of I3SubIdentifier, implementing the "identifier stack" proposed in Internet Indirection Infrastructure */
class I3IdentifierStack {
public:
    /** Pushes an I3Identifier
    * @param id Identifier to be pushed
    */
    void push(const I3Identifier &id);

    /** Pushes an I3IPAddress
     * @param ip IP address to be pushed
     */
    void push(const I3IPAddress &ip);

    /** Pushes an IP address with port
     * @param add IP address to be pushed
     * @param port Address port to be pushed
     */
    void push(const IPvXAddress &add, int port);

    /** Appends an I3IdentifierStack at the end
     * @param stack Identifier stack to be appended
     */
    void push(const I3IdentifierStack &stack);

    /** Returns a reference to the top of the stack */
    I3SubIdentifier &peek();

    /** Returns a const reference to the top of the stack */
    const I3SubIdentifier &peek() const;

    /** Pops a subidentifier from the top of the stack */
    void pop();

    void clear();

    /** Returns the size of the stack */
    uint32_t size() const;

    /** Comparation function
    * @param s Stack to be compared against
    */
    int compareTo(const I3IdentifierStack &s) const;

    int length() const;

    void replaceAddress(const I3IPAddress &source, const I3IPAddress &dest);

    friend std::ostream& operator<<(std::ostream& os, const I3IdentifierStack &t);

protected:
    /** Stack of subidentifiers */
    std::list<I3SubIdentifier> stack;
};

#endif
