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
 * @file NodeHandle.h
 * @author Markus Mauch
 * @author Sebastian Mies
 */

#ifndef __NODEHANDLE_H_
#define __NODEHANDLE_H_

//#include <stdint.h>
#include <oversim_mapset.h>

#include <omnetpp.h>

#include <OverlayKey.h>
#include <TransportAddress.h>

class hashFcn;
class IPvXAddress;

/**
 * This class implements a node handle.<br>
 *
 * It covers the complete node information, like IP-Address,
 * port, NodeKey and some additional flags for Simulation
 * behaviour. The information can be sparse, so parts can be
 * omited by setting the property to an unspecified value.
 *
 * @author Markus Mauch
 * @author Sebastian Mies
 */
class NodeHandle : public TransportAddress
{
public://collection typedefs
    typedef UNORDERED_SET<NodeHandle, hashFcn> Set; /**< a hash_set of NodeHandles */

protected://fields
    OverlayKey key; /**< the OverlayKey of this NodeHandle */

public://construction

    /**
     * Constructs an unspecified NodeHandle
     */
    NodeHandle( );

    /**
     * Standard destructor
     */
    virtual ~NodeHandle( ) {};

    /**
     * Copy constructor.
     *
     * @param handle The NodeHandle to copy
     */
    NodeHandle( const NodeHandle& handle );

    /**
     * Complete constructor.
     *
     * @param key The OverlayKey
     * @param ip The IPvXAddress
     * @param port The UDP-Port
     */
    NodeHandle( const OverlayKey& key,
                const IPvXAddress& ip,
                int port);

    /**
     * Constructor to generate a NodeHandle from a TransportAddress.
     * The key will be unspecified.
     *
     * @param ta transport address
     */
    NodeHandle( const TransportAddress& ta );

    /**
     * Constructor to generate a NodeHandle from an existing
     * OverlayKey and TransportAddress.
     *
     * @param key the overlay key
     * @param ta transport address
     */
    NodeHandle( const OverlayKey& key, const TransportAddress& ta );

public://static fields

    static const NodeHandle UNSPECIFIED_NODE; /**< the unspecified NodeHandle */

public://methods: delegates to OverlayKey and IPvXAddress

    /**
     * compares this NodeHandle to another given NodeHandle
     *
     * @param rhs the NodeHandle this is compared to
     * @return true if OverlayKey, IPvXAddress and port is equal, false otherwise
     */
    bool operator==(const NodeHandle& rhs) const;

    /**
     * compares this NodeHandle to another given NodeHandle
     *
     * @param rhs the NodeHandle this is compared to
     * @return true if OverlayKey, IPvXAddress and port is not equal, false otherwise
     */
    bool operator!=(const NodeHandle& rhs) const;

     /**
     * compares this to a given NodeHandle
     *
     * @param rhs the NodeHandle this is compared to
     * @return true if this->key is smaller than rhs->key, false otherwise
     */
    bool operator< (const NodeHandle& rhs) const;

    /**
     * compares this to a given NodeHandle
     *
     * @param rhs the NodeHandle this is compared to
     * @return true if this->key is greater than rhs->key, false otherwise
     */
    bool operator> (const NodeHandle& rhs) const;

    /**
     * compares this to a given NodeHandle
     *
     * @param rhs the NodeHandle this is compared to
     * @return true if this->key is smaller than or equal to rhs->key, false otherwise
     */
    bool operator<=(const NodeHandle& rhs) const;

    /**
     * compares this to a given NodeHandle
     *
     * @param rhs the NodeHandle this is compared to
     * @return true if this->key is greater than or equal to rhs->key, false otherwise
     */
    bool operator>=(const NodeHandle& rhs) const;

public://methods: operators

    /**
     * assigns key, ip and port of rhs to this->key, this->ip and this->port
     *
     * @param rhs the NodeHandle with the defined key, ip and port
     * @return this NodeHandle object
     */
    NodeHandle& operator=(const NodeHandle& rhs);

public://methods: setters and getters

    /**
     * saves given key to NodeHandle::key
     *
     * @param key the given key
     */
    void setKey( const OverlayKey& key );

    /**
     * returns key of this NodeHandle
     *
     * @return the key of this NodeHandle
     */
    const OverlayKey& getKey() const;

    /**
     * indicates if this NodeHandle is specified
     *
     * @return true, if NodeHandle has no ip or key specified, false otherwise
     */
    bool isUnspecified() const;

public://methods: c++ streaming

    /**
     * standard output stream for NodeHandle,
     * gives out ip, port and key
     *
     * @param os the ostream
     * @param n the NodeHandle
     * @return the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const NodeHandle& n);

private://methods:

    /**
     * throws an opp_error if this or handle is unspecified
     *
     * @param handle the NodeHandle to check
     */
    void assertUnspecified( const NodeHandle& handle ) const;

public:

    /**
     * serializes the object into a buffer
     *
     * @param b the buffer
     */
    virtual void netPack(cCommBuffer *b);

    /**
     * deserializes the object from a buffer
     *
     * @param b the buffer
     */
    virtual void netUnpack(cCommBuffer *b);

    /**
     * returns a copy of the NodeHandle
     *
     * @return a copy of the NodeHandle
     */
    virtual TransportAddress* dup() const;
};

/**
 * netPack for NodeHandles
 *
 * @param b the buffer
 * @param obj the NodeHandle to serialise
 */
inline void doPacking(cCommBuffer *b, NodeHandle& obj) {obj.netPack(b);}

/**
 * netUnpack for NodeHandles
 *
 * @param b the buffer
 * @param obj the NodeHandles to unserialise
 */
inline void doUnpacking(cCommBuffer *b, NodeHandle& obj) {obj.netUnpack(b);}

#endif
