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
 * @file TransportAddress.h
 * @author Markus Mauch
 * @author Sebastian Mies
 * @author Ingmar Baumgart
 */

#ifndef __TRANSPORTADDRESS_H_
#define __TRANSPORTADDRESS_H_

//#include <stdint.h>
#include <oversim_mapset.h>

#include <omnetpp.h>
#include <IPvXAddress.h>

class TransportAddress;
typedef std::vector<TransportAddress> TransportAddressVector;

/**
 * This class implements a common transport address.<br>
 *
 * It covers the complete node information, like IP-Address,
 * and port. The information can be sparse, so parts can be
 * omited by setting the property to an unspecified value.
 *
 * @author Markus Mauch
 * @author Sebastian Mies
 * @author Ingmar Baumgart
 */
class TransportAddress
{
public:

    /**
     * defines a hash function for TransportAddress
     */
    class hashFcn
    {
    public:
        size_t operator()( const TransportAddress& h1 ) const
        {
            return h1.hash();
        }
    };

    // TODO: maybe we should use const uint8_t instead to save memory
    enum NatType {
        UNKNOWN_NAT = 0,
        NO_NAT = 1,
        FULL_CONE_NAT = 2,
        PORT_RESTRICTED_NAT = 3,
        SYMMETRIC_NAT = 4
    };

public://collection typedefs

    typedef UNORDERED_SET<TransportAddress, hashFcn> Set; /**< a hashed set of TransportAddresses */

protected://fields

    IPvXAddress ip; /**< the ip of this TransportAddress object */
    int port; /**< the port of this TransportAddress object */

private:
    NatType natType; /**< the assumed type of a NAT this node is behind (work in progress and currently not used */
    // TODO: as soon as sourceRoute is used, we need to calculate the correct TransportAddress length for statistics */
    TransportAddressVector sourceRoute; /**< source route for NAT traversal */

public://construction

    /**
     * Constructs a unspecified TransportAddress
     */
    TransportAddress();

    /**
     * Standard destructor
     */
    virtual ~TransportAddress() {};

    /**
     * Copy constructor.
     *
     * @param handle The TransportAddress to copy
     */
    TransportAddress(const TransportAddress& handle);

    /**
     * Complete constructor.
     *
     * @param ip The IPvXAddress
     * @param port The UDP-Port
     * @param natType the type of NAT this node is behind
     */
    TransportAddress(const IPvXAddress& ip, int port = -1,
                     NatType natType = UNKNOWN_NAT);

public://static fields

    static const TransportAddress UNSPECIFIED_NODE; /**< TransportAddress without specified ip and port */
    static const TransportAddressVector UNSPECIFIED_NODES;

public://methods: delegates to OverlayKey and IPvXAddress

    /**
     * compares this to a given TransportAddress
     *
     * @param rhs the TransportAddress this is compared to
     * @return true if both IPvXAddress and port are equal, false otherwise
     */
    bool operator==(const TransportAddress& rhs) const;

    /**
     * compares this to a given TransportAddress
     *
     * @param rhs the TransportAddress this is compared to
     * @return true if both IPvXAddress and port are not equal, false otherwise
     */
    bool operator!=(const TransportAddress& rhs) const;

    /**
     * compares this to a given TransportAddress
     *
     * @param rhs the TransportAddress this is compared to
     * @return true if this->ip is smaller than rhs->ip, false otherwise
     */
    bool operator< (const TransportAddress& rhs) const;

    /**
     * compares this to a given TransportAddress
     *
     * @param rhs the TransportAddress this is compared to
     * @return true if this->ip is greater than rhs->ip, false otherwise
     */
    bool operator> (const TransportAddress& rhs) const;

    /**
     * compares this to a given TransportAddress
     *
     * @param rhs the TransportAddress this is compared to
     * @return true if this->ip is smaller than or equal to rhs->ip, false otherwise
     */
    bool operator<=(const TransportAddress& rhs) const;

    /**
     * compares this to a given TransportAddress
     *
     * @param rhs the TransportAddress this is compared to
     * @return true if this->ip is greater than or equal to rhs->ip, false otherwise
     */
    bool operator>=(const TransportAddress& rhs) const;

public://methods: operators

    /**
     * assigns ip and port of rhs to this->ip and this->port
     *
     * @param rhs the TransportAddress with the defined ip and port
     * @return this TransportAddress object
     */
    TransportAddress& operator=(const TransportAddress& rhs);

public://methods: setters and getters

    /**
     * Sets the ip address, port and NAT type
     *
     * @param ip the new IPvXAddress
     * @param port the new port
     * @param natType the type of NAT this node is behind
     */
    void setIp(const IPvXAddress& ip, int port = -1,
               NatType natType = UNKNOWN_NAT);

    /**
     * Sets the ip address, port and NAT type. DEPRECATED: Use setIp() instead!
     *
     * @param ip the new IPvXAddress
     * @param port the new port
     * @param natType the type of NAT this node is behind
     */
    void setAddress(const IPvXAddress& ip, int port = -1,
                    NatType natType = UNKNOWN_NAT) __attribute ((deprecated))
                    { setIp(ip, port, natType); };

    /**
     * Appends a source route to this TransportAddress
     *
     * @param sourceRoute the source route to append
     */
    void appendSourceRoute(const TransportAddress& sourceRoute);

    /**
     * Clears the source route of this TransportAddress
     *
     */
    void clearSourceRoute() { sourceRoute.clear(); };

    /**
     * sets this->port to the given port
     *
     * @param port the new port
     * */
    void setPort( int port );

    /**
     * returns ip address
     *
     * @return this->ip
     */
    const IPvXAddress& getIp() const;

    /**
     * returns ip address. DEPRECATED: Use getIp() instead!
     *
     * @return this->ip
     */
    const IPvXAddress& getAddress() const __attribute ((deprecated))
            { return getIp(); };

    /**
     * returns port
     *
     * @return this->port
     */
    int getPort() const;

    /**
     * returns the type of NAT this node is behind
     *
     * @return The type of NAT this node is behind
     */
    NatType getNatType() const;

    /**
     * Returns the length of the source route to reach this node
     *
     * @return The length of the source route
     */
    size_t getSourceRouteSize() const;

    /**
     * Returns source route used to reach this node
     *
     * @return The source route
     */
    const TransportAddressVector& getSourceRoute() const;

    /**
     * indicates if TransportAddress is specified
     *
     * @return true, if TransportAddress has no ip or port specified, false otherwise
     */
    bool isUnspecified() const;

public://methods: hashing

    /**
     * creates a hash value of ip and port
     *
     * @return the hash value
     */
    size_t hash() const;

public://methods: c++ streaming

    /**
     * standard output stream for TransportAddress,
     * gives out ip and port
     *
     * @param os the ostream
     * @param n the TransportAddress
     * @return the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const TransportAddress& n);

private://methods:

    /**
     * throws an opp_error if this or handle is unspecified
     *
     * @param handle the TransportAddress to check
     */
    void assertUnspecified( const TransportAddress& handle ) const;

public:

    /**
     * returns a copy of the TransportAddress
     *
     * @return a copy of the TransportAddress
     */
    virtual TransportAddress* dup() const;
};

/**
 * netPack for TransportAddress
 *
 * @param buf the buffer
 * @param addr the TransportAddress to serialise
 */
inline void doPacking(cCommBuffer *buf, TransportAddress& addr)
{
    doPacking(buf, addr.getIp());
    doPacking(buf, addr.getPort());
    doPacking(buf, static_cast<uint8_t>(addr.getNatType()));
    doPacking(buf, static_cast<uint8_t>(addr.getSourceRouteSize()));
    for (size_t i = 0; i < addr.getSourceRouteSize(); i++) {
        // TODO: ugly const_cast should be avoided
        doPacking(buf, const_cast<TransportAddress&>(addr.getSourceRoute()[i]));
    }
}

/**
 * netUnpack for TransportAddress
 *
 * @param buf the buffer
 * @param addr the TransportAddress to unserialise
 */
inline void doUnpacking(cCommBuffer *buf, TransportAddress& addr)
{
    IPvXAddress ip;
    int port;
    uint8_t natType = 0;
    doUnpacking(buf, ip);
    doUnpacking(buf, port);
    doUnpacking(buf, natType);
    addr.setIp(ip, port, static_cast<TransportAddress::NatType>(natType));

    uint8_t sourceRouteSize;
    TransportAddress sourceRouteEntry;
    doUnpacking(buf, sourceRouteSize);
    for (size_t i = 0; i < sourceRouteSize; i++) {
        doUnpacking(buf, sourceRouteEntry);
        addr.appendSourceRoute(sourceRouteEntry);
    }
}


#endif
