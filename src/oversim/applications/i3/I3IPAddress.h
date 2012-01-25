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
 * @file I3IPAddress.h
 * @author Antonio Zea
 */

#ifndef __I3IPADDRESS_H__
#define __I3IPADDRESS_H__

#include <omnetpp.h>
#include <IPvXAddressResolver.h>
#include <TransportAddress.h>

/** A simple wrapper around an IPvXAddress and a port. */
struct I3IPAddress : public TransportAddress {
    /** Constructor */
    I3IPAddress();

    /** Constructor */
    I3IPAddress(IPvXAddress add, int port);

    /** "Less than" operator
    * @param a Address to be compared
    */
    bool operator<(const I3IPAddress &a) const;

    /** "Equals" operator (takes port in account)
     * @param a Address to be compared
     */
    bool operator==(const I3IPAddress &a) const;

    /** "Greater than" operator (takes port in account)
     * @param a Address to be compared
     */
    bool operator>(const I3IPAddress &a) const;

    int length() const;

    /** String stream output operation
    * @param os String stream
    * @param ip Address to be output
    * @returns os parameter
    */
    friend std::ostream& operator<<(std::ostream& os, const I3IPAddress& ip);

};

#endif
