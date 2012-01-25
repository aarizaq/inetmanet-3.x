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
 * @file I3Trigger.cc
 * @author Antonio Zea
 */

#include "I3Trigger.h"

I3Trigger::I3Trigger() :
        insertionTime(0)
{
}

int I3Trigger::compareTo(const I3Trigger &t) const
{
    int cmp = identifier.compareTo(t.identifier);
    return (cmp != 0) ? cmp : identifierStack.compareTo(t.identifierStack);
}

bool I3Trigger::operator <(const I3Trigger &t) const
{
    return compareTo(t) < 0;
}

bool I3Trigger::operator >(const I3Trigger &t) const
{
    return compareTo(t) > 0;
}

bool I3Trigger::operator ==(const I3Trigger &t) const
{
    return compareTo(t) == 0;
}


void I3Trigger::setIdentifier(const I3Identifier &id)
{
    identifier = id;
}

void I3Trigger::setInsertionTime(simtime_t time)
{
    insertionTime = time;
}

void I3Trigger::setIdentifierStack(I3IdentifierStack &stack)
{
    identifierStack = stack;
}

I3Identifier &I3Trigger::getIdentifier()
{
    return identifier;
}

const I3Identifier &I3Trigger::getIdentifier() const
{
    return identifier;
}

simtime_t I3Trigger::getInsertionTime() const
{
    return insertionTime;
}

void I3Trigger::clear()
{
    identifier.clear();
    identifierStack.clear();
}

I3IdentifierStack &I3Trigger::getIdentifierStack()
{
    return identifierStack;
}

const I3IdentifierStack &I3Trigger::getIdentifierStack() const
{
    return identifierStack;
}

int I3Trigger::length() const {
    /* insertionTime is an internal variable and doesn't count as part of the message */
    return identifier.length() + identifierStack.length();
}

std::ostream& operator<<(std::ostream& os, const I3Trigger &t)
{
    os << "(" << t.identifier << ", {" << t.identifierStack << "})";
    return os;
}

