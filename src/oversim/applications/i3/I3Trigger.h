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
 * @file I3Trigger.h
 * @author Antonio Zea
 */

#ifndef __I3TRIGGER_H__
#define __I3TRIGGER_H__

#include "I3Identifier.h"
#include "I3IdentifierStack.h"
#include <omnetpp.h>

/** Implementation of an Internet Indirection Infrastructure trigger.
* An I3Trigger is composed of an I3Identifier and a I3IdentifierStack (a stack of I3SubIdentifier).
* In the most basic case, the subidentifier stack contains a single I3IPAddress.
* When a packet is matched to a trigger, it is sent to the first valid
* subidentifier found - the rest is passed to the application.
* If none is found, the packet is dropped.
*
* @author Antonio Zea
* @see I3, I3Identifier, I3IdentifierStack, I3SubIdentifier
*
*/

class I3Trigger {
public:
    /**
    * Constructor */
    I3Trigger();

    /** Comparison function
    *
    * @param t Trigger to compare to
    *
    */
    int compareTo(const I3Trigger &t) const;

    /** "Less than" comparison function
     *
     * @param t Trigger to compare to
     *
     */
    bool operator <(const I3Trigger &t) const;

    /** "Greater than" comparison function
     *
     * @param t Trigger to compare to
     *
     */
    bool operator >(const I3Trigger &t) const;

    /** "Equals" comparison function
     *
     * @param t Trigger to compare to
     *
     */
    bool operator ==(const I3Trigger &t) const;

    /** Sets the identifier
     *
     * @param id Identifier to set.
     *
     */
    void setIdentifier(const I3Identifier &id);

    /** Sets the insertion time
     *
     * @param time Insertion time
     *
     */
    void setInsertionTime(simtime_t time);

    /** Sets the identifier stack
     *
     * @param stack Insertion time
     *
     */
    void setIdentifierStack(I3IdentifierStack &stack);

    /** Returns the identifier
     *
     * @returns Identifier stack
     *
     */
    I3Identifier &getIdentifier();

    /** Returns the identifier
     *
     * @returns A const reference to the identifier
     *
     */
    const I3Identifier &getIdentifier() const;

    /** Returns the insertion time
     *
     * @returns Insertion time
     *
     */
    simtime_t getInsertionTime() const;

    void clear();

    /** Returns the insertion time
     *
     * @returns Insertion time
     *
     */
    I3IdentifierStack &getIdentifierStack();

    /** Returns the identifier stack
     *
     * @returns A const reference to the identifier stack
     *
     */
    const I3IdentifierStack &getIdentifierStack() const;

    /** String stream operator <<
     *
     * @param os Output string stream
     * @param t Trigger to be output
     * @returns os parameter
     *
     */
    friend std::ostream& operator<<(std::ostream& os, const I3Trigger &t);

    int length() const;

protected:
    /** Identifier to be matched */
    I3Identifier identifier;

    /** Identifier stack */
    I3IdentifierStack identifierStack;

    /** Time in which the trigger was inserted into I3 */
    simtime_t insertionTime;

};

#endif
