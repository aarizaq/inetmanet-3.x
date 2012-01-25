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
 * @file I3IdentifierStack.cc
 * @author Antonio Zea
 */


#include "I3IdentifierStack.h"

using namespace std;

void I3IdentifierStack::push(const I3Identifier &identifier)
{
    I3SubIdentifier id;

    id.setIdentifier(identifier);
    stack.push_back(id);
}

void I3IdentifierStack::push(const IPvXAddress &ip, int port)
{
    I3IPAddress ipAddress;

    ipAddress.setIp(ip);
    ipAddress.setPort(port);
    push(ipAddress);
}

void I3IdentifierStack::push(const I3IPAddress &ip)
{
    I3SubIdentifier id;

    id.setIPAddress(ip);
    stack.push_back(id);
}

void I3IdentifierStack::push(const I3IdentifierStack &s)
{
    list<I3SubIdentifier>::const_iterator it;

    for (it = s.stack.begin(); it != s.stack.end(); it++) {
        stack.push_back(*it);
    }
}

I3SubIdentifier &I3IdentifierStack::peek()
{
    return stack.back();
}

const I3SubIdentifier &I3IdentifierStack::peek() const
{
    return stack.back();
}

void I3IdentifierStack::pop()
{
    stack.pop_back();
}


int I3IdentifierStack::compareTo(const I3IdentifierStack &s) const
{
    int cmp;

    if (stack.size() != s.size()) {
        return stack.size() - s.size();
    }

    list<I3SubIdentifier>::const_iterator it0 = stack.begin();
    list<I3SubIdentifier>::const_iterator it1 = s.stack.begin();

    for (; it0 != stack.end(); it0++, it1++) {
        //for (uint i = 0; i < stack.size(); i++) {
        cmp = it0->compareTo(*it1);
        //cmp = stack[i].compareTo(s.stack[i]);
        if (cmp != 0) return cmp;
    }
    return 0;
}


void I3IdentifierStack::clear()
{
    stack.clear();
}

uint32_t I3IdentifierStack::size() const
{
    return stack.size();
}

int I3IdentifierStack::length() const {
    int len = 0;
    list<I3SubIdentifier>::const_iterator it;

    for (it = stack.begin(); it != stack.end(); it++) {
        len += it->length();
    }
    return len + 16; /* the size variable */
}

void I3IdentifierStack::replaceAddress(const I3IPAddress &source, const I3IPAddress &dest) {
    list<I3SubIdentifier>::iterator it;

    for (it = stack.begin(); it != stack.end(); it++) {
        if (it->getType() == I3SubIdentifier::IPAddress && it->getIPAddress() == source) {
            it->setIPAddress(dest);
        }
    }

}

std::ostream& operator<<(std::ostream& os, const I3IdentifierStack &s) {
    list<I3SubIdentifier>::const_iterator it;

    for (it = s.stack.begin(); it != s.stack.end(); it++) {
        os << *it << ", ";
    }
    return os;
}

